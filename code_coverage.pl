#!/usr/bin/perl -w
#Gets the kernel code coverage data (functions, branches and lines)
#Usage: 
#Reset coverage: sudo perl $(ABS_PATH)/code_coverage.pl -z -p $(FINAL_DATA_PATH)
#Run coverage: sudo perl $(ABS_PATH)/code_coverage.pl -r -p $(FINAL_DATA_PATH)
#
#OPTIONS: -z   zero kernel code coverage counter
#         -r   get the code coverage data
#         -p  $(FINAL_DATA_PATH)   $(FINAL_DATA_PATH)
#                                  is the path to store the 
#                                  coverage.txt file
use strict;
use warnings;
use File::Spec::Functions qw(splitpath);
use File::Copy::Recursive qw /dircopy/;
use File::Temp qw /tempdir/;
use Getopt::Long;

#Globals
my $reset_file = "/sys/kernel/debug/gcov/reset";
my $base_dir = "/sys/kernel/debug/gcov/home";
my $tmp_dir = "/tmp/";
my $coverage_file = "/coverage.txt";
my $path_name = "";
my $curr_data_dir;
my $zero_flag = 0;
my $run_flag = 0;
my $result = 0;
my $gcda_extension = ".gcda";
my $nfunc = 0; #total function count
my $nfunc_exec = 0; #Executed function count
my $func_flag = 0; #Indicates function string encountered line expected next
my $nlines = 0; #total lines count
my $nlines_exec = 0; #Executed lines count
my $nbranches = 0; #total branches count
my $nbranches_exec = 0; #Executed branches count
my @gcda_files; #Store list of  gcda files
my $file_flag = 0;


#Functions
sub code_coverage($);
sub reset_kernel_coverage();
sub copy_data_into_temp();
sub printme(@); #wrapper for print
sub scanforfiles($);
sub run_gcov($$); 
sub print_coverage_data();
sub get_coverage($);
sub get_result();
sub clearglobals();


#Code starts here
GetOptions("z"=> \$zero_flag, "r" =>\$run_flag, "p=s" =>\$path_name) or die "Error";
if(!($zero_flag ^ $run_flag)){
        print "Usage: Options\n\t-z\tzero kernel counters\n\t-r\trun code coverage\nRerun with only one of these options\n";
        exit(-1);
}
if($zero_flag){ 
        reset_kernel_coverage();
}
elsif($run_flag){
        printme("Code coverage\n");
        copy_data_into_temp();
        $result = get_coverage($curr_data_dir);
        local *HDL;
        my $data_file = $path_name.$coverage_file;
        open(HDL, ">$data_file") or die "Error: could not open $data_file file\n";
        print(HDL $result);
        close(HDL);
        chmod(0777,$data_file);
}
exit(0); 


#Copys the kernel .gcda/.gcno data 
#into a temp dir for further processing
sub copy_data_into_temp(){
        #creates temp dir in $tmp_dir, clears when exiting the script.
        $curr_data_dir = tempdir(DIR => $tmp_dir, CLEANUP => 1);
        dircopy($base_dir, $curr_data_dir);

}
#Reset the kernel counter
sub reset_kernel_coverage(){

        local *HANDLE;
        open(HANDLE, ">$reset_file") or die "Error: Could not open $reset_file file\n";
        print(HANDLE "0");
        close(HANDLE);
        printme("\nKernel counters reset\n");

}

#Gets coverage data using GCOV
#returns the magic number
sub get_coverage($){

        printme("Getting coverage data ...\n");
        clearglobals();
        scanforfiles($_[0]);

        foreach my $file (@gcda_files){
                my ($vol, $dir, $file) = splitpath($file);
                chomp($file);
                chomp($dir);
                #printme("Processing $dir$file ...\n"); 
                run_gcov($file, $dir);
        }

        print_coverage_data();

        return get_result();
}
1;

#clear all globals
sub clearglobals(){

        #clear globals
        $nfunc = 0;
        $nbranches = 0;
        $nlines = 0;
        $nfunc_exec = 0;
        $nbranches_exec = 0;
        $nlines_exec = 0;

}

# Search for $gcda_extension files 
# in the given path
sub scanforfiles($){
        
        my $gcda_base_dir = $_[0];
        @gcda_files = `find $gcda_base_dir -name *$gcda_extension -type f 2>/dev/null` or die "Error: Could not find $gcda_extension files\n"; 
        printme("Found ".($#gcda_files+1) . " $gcda_extension files.\n");

}
# Runs gcov, parse result and stores data
# Takes name
sub run_gcov($$){
        my $gcda_file = $_[0] ; 
        my $object_path = $_[1];
        my @lines = `gcov $gcda_file -o $object_path -b -f -n` or die "Error: Filename expected\n";


#print "This is gcov report\n@lines\n";
        foreach my $line (@lines){

                chomp($line);

                if($line =~ /^Function/){
                        $nfunc++; 
                        $func_flag = 1;
                        next;
                }

                #To get the next line after Function
                #probably crappy way but works.

                if($func_flag == 1){ 
                        if( $line !~ /^Lines\sexecuted\W0\W\d\d\W/){
                                $nfunc_exec++;
                        }
                        $func_flag = 0;
                        next; #very important else false values recorded
                        #by next if block  
                }  

                if($file_flag && $line =~ /^Lines/){
			my ($percent, $tlines) =  ($line =~ /(\d+.\d+)% of (\d+)/) or die "Error";
                        $nlines_exec += ($percent * $tlines) / 100;
                        $nlines += $tlines;
                }
                elsif($file_flag && $line =~ /^Taken/){
			my ($percent, $tbranches) =  ($line =~ /(\d+.\d+)% of (\d+)/) or die "Error";
                        $nbranches_exec += ($percent * $tbranches) / 100;
                        $nbranches += $tbranches;
                }elsif(!$file_flag && $line =~ /^File/){
			$file_flag = 1;
		}

        }
}

#Returns a magic number
sub get_result(){
        return ($nfunc_exec + $nlines_exec + $nbranches_exec);
}

#Prints the lines, branches, functions coverage details
sub print_coverage_data(){

        printme("\nLines executed.....: $nlines_exec of $nlines lines\n");
        printme("Branches executed..: $nbranches_exec of $nbranches branches\n");
        printme("Function executed..: $nfunc_exec of $nfunc functions\n");
        printme("Our magic number...: ".($nfunc_exec + $nlines_exec + $nbranches_exec)."\n");
}

#Wrapper for print
#Additional functionality like quiet can be added
sub printme(@){

        print "@_"; 

}
