/* Vikram Adiga
 * System Call Fuzzer
 * Main control here
 */
#include "common-header.h"
#include <syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>
#include "random.h"
#include "databuffers.h"
#include "syscalls.h"
#define COVERAGE_DATA "code_coverage.txt"

void init(void);
void abrthandler(int sig, siginfo_t *info, void *uap);
void segfaulthandler(int sig, siginfo_t *info, void *uap);
void sighandler(int sig);
void greedy_algorithm(int isbetter);
void modify_syscalllist(void);
void execute_syscalllist(void);
void generate_syscalllist(void);
void reset_coverage(void);
long run_coverage(void);

static sigjmp_buf env;
char pwd[100] = {0};
int main (int argc, char *argv[])
{
        if(access(COVERAGE_DATA, F_OK) == 0 || access(FILE_DIR, F_OK) == 0){
                printf("Remove %s and run clean.sh then run again\n", COVERAGE_DATA);
                exit(1);
        }
        /*Set up for system fuzzing*/
        init();  
        /* Initializing random generator */
        unsigned int seed = rand_init();

        int fd = open(COVERAGE_DATA, O_RDWR | O_CREAT, 0755);
        if(fd == -1) {
                printf("ERROR: Could not open: %s\n", COVERAGE_DATA);
                exit(1);
        }
        int j = 0, better = 0;
        long curr_result = 0, prev_result = 0;
        char databuf[sizeof(int) * 2 + sizeof(long) + 4] = {0};
        //write seed
        sprintf(databuf, "%u\n", seed);
        write(fd, databuf, strlen(databuf)); 
        for(j = 0; j < NUM_RUNS; j++){
                if(CODE_COVERAGE){
                        reset_coverage();
                        better = curr_result > prev_result;
                }
                else{
                        if(j)  better ^= 1;
                }
                if(better){
                        prev_result = curr_result;
                        greedy_algorithm(1);
                }else{
                        if(sigsetjmp(env,1)){
                                printf("MEMORY CORRUPTION DETECTED. IGNORING...\n");
                                force_clearaddresslist();
                        }else{
                                if(j > 0) clearaddresslist();

                        }
                        greedy_algorithm(0);
                }
                if(CODE_COVERAGE) {
                        curr_result = run_coverage();
                        //write results
                        sprintf(databuf, "%d,%d,%ld\n", j+1, better, curr_result);
                        write(fd, databuf, strlen(databuf)); 
                }
        }
        close(fd);
        free_resources();

        return 0;
}

/* blocks signals,turn off buffering,  initializes random generator
 *  and generates file */
void init(void){
        struct sigaction sa, sa2, sa3;

        //Ignore signal generated due broken pipes (EPIPE)
        assert(signal(SIGPIPE, SIG_IGN) != SIG_ERR);
        assert(signal(SIGINT, sighandler) != SIG_ERR);
        /* turn off buffering */
        assert(setvbuf(stdout,NULL,_IONBF,0) == 0);
        assert(setvbuf(stderr,NULL,_IONBF,0) == 0);

        /* Handle SIGSEGV, SIGILL, SIGABRT */
        sa.sa_sigaction = segfaulthandler;
        assert(sigemptyset(&sa.sa_mask) != -1);
        sa.sa_flags = SA_SIGINFO;
        assert(sigaction(SIGSEGV,&sa,0)!= -1);
        sa2.sa_sigaction = segfaulthandler;
        assert(sigemptyset(&sa2.sa_mask) != -1);
        sa2.sa_flags = SA_SIGINFO;
        assert(sigaction(SIGILL,&sa2,0)!= -1);
        sa3.sa_sigaction = abrthandler;
        assert(sigemptyset(&sa3.sa_mask) != -1);
        sa3.sa_flags = SA_SIGINFO;
        assert(sigaction(SIGABRT,&sa3,0)!= -1);

        if(DEBUG) printf("Debug mode: on\n");
        else printf("Normal mode: on\n");

        if(CODE_COVERAGE) printf("Code coverage: on\n");
        else printf("Code coverage: off\n");

        getcwd(pwd,(sizeof(pwd)/sizeof(pwd[0])));
        printf("Present working dir: %s\n",pwd);
        

}

/*
 * SEG FAULT HANDLER
 * */
void segfaulthandler(int sig, siginfo_t *info, void *uap){

        //safe printing
        char a[] = "\n******SEGMENTATION FAULT OCCURRED******\n";
        write(1,a,sizeof(a));
        siglongjmp(env, 1);

}
/*
 * ABRT HANDLER
 * */
void abrthandler(int sig, siginfo_t *info, void *uap){

        //safe printing
        char a[] = "\n******ABORT SIGNAL RECIEVED******\n";
        write(1,a,sizeof(a));
        if(sig != SIGINT)
                siglongjmp(env, 1);
}
/*
 * SIG INT HANDLER
 * */
void sighandler(int sig){

        //safe printing
        char a[] = "\n*******SIGINT RECD EXITING*******\n";   
        write(1, a, sizeof(a));
        //exit on CTRL-C after freeing resources
        free_resources();
        exit(1);
}

/*
 * Reset coverage data
 * */
void reset_coverage(void){
        char cmd[200] = {0};
        sprintf(cmd,"sudo perl %s/code_coverage.pl -z -p %s",pwd,pwd);
        system(cmd);
        return;
}

/*
 * Run and capture coverage data
 * */
long run_coverage(void){
        char cmd[200] = {0};
        sprintf(cmd,"sudo perl %s/code_coverage.pl -r -p %s",pwd,pwd);
        system(cmd);
        //open the data file
        char filename[130] = {0};
        sprintf(filename,"%s/coverage.txt",pwd);
        int fd = open(filename,O_RDONLY);
        //check the number of files open for 
        //this process
        if(fd == -1) {
                pid_t pid = getpid();
                char tbuf[20];
                sprintf(tbuf,"lsof -p %d",pid);
                printf("No of open files: \n");
                system(tbuf);       
                exit(1);

        }        
        //read value
        char buf[sizeof(long) + 1] = {0};
        long value = 0;
        read(fd,buf,sizeof(buf));
        sscanf(buf,"%ld", &value);
        //close file and remove
        close(fd);
        remove(filename);
        //another file value test
        if(value == 0) exit(1); 
        return value;
}       


/*
 * Greedy algorithm: 
 * if better makes a 5% change to existing data, else generates
 * a new set of data
 * isbetter: external input based on lcov results 
 * */
void greedy_algorithm(int isbetter){

        if(isbetter){
                modify_syscalllist();          
        }else{
                generate_syscalllist();
        }      
        execute_syscalllist();    
}
/*
 * Modify and make a new list of calls
 * */
void modify_syscalllist(void){

        printf("Modifying existing data...\n");
        syscalldata *data = NULL;
        //Gets the syscall list pointer and clear the syscall_list ptr
        linkedlist *temp = (linkedlist *)getsyscalllist();
        if(temp == NULL) {
                printf("Error!  Run with 0 as input and then retry with 1\n");
                exit(1);
        }
        //read list and call mysycall which modifies syscall and update syscall list
        int i = 0;
        for(i = NUM_RANDCALLS; i > 0; i--){
                data = getnodedata(&temp, i);
                mysyscall(data->number, data->args[0],data->args[1],
                                data->args[2],data->args[3],data->args[4],
                                data->args[5],data->args[6]);
        }   
        clearlist(&temp);

}
/*
 * Generates a random list of syscall
 * */
void generate_syscalllist(void){
        printf("Generating new data...\n");
        /*populate all functions to be called */
#ifndef OLDER_LINUX
        void (*func_ptr[NUM_SYSCALLS]) () = {&myopen, &myclose, &myread, &mywrite, &myfsync, &myfdatasync, &myfstatfs, 
                &myfstatfs64, &myfstat, &myfstat64, &mypipe2, &mydup, &mydup2, &mydup3, &myfchdir, &myfchmod,
                &myreadahead, &myreadv, &mywritev, &myfallocate, &myftruncate, &myftruncate64, &myfadvise64_64,
                &mysendfile, &mylseek, &myllseek, &mypread64, &mypwrite64, &mypreadv, &mypwritev, &myopenat, &myfchown, 
                &mymkdirat, &mymkdir, &myunlinkat, &myunlink,&myrenameat, &myrename, &myfchmodat, &myfchownat, 
                &myfstatat64, &myfaccessat, &myaccess, mysymlinkat, &mysymlink,&mylinkat, &mylink, &myreadlink,
                &myreadlinkat, &myutimensat, &myioctl};
#else
        void (*func_ptr[NUM_SYSCALLS]) () = {&myopen, &myclose, &myread, &mywrite, &myfsync, &myfdatasync, &myfstatfs, 
                &myfstatfs64, &myfstat, &myfstat64, &mydup, &mydup2, &myfchdir, &myfchmod,
                &myreadahead, &myreadv, &mywritev, &myftruncate, &myftruncate64, &myfadvise64_64,
                &mysendfile, &mylseek, &myllseek, &mypread64, &mypwrite64, &myopenat, &myfchown, 
                &mymkdirat, &mymkdir, &myunlinkat, &myunlink,&myrenameat, &myrename, &myfchmodat, &myfchownat, 
                &myfstatat64, &myfaccessat, &myaccess, mysymlinkat, &mysymlink,&mylinkat, &mylink, &myreadlink,
                &myreadlinkat, &myioctl};
#endif

        int offset = 0;
        unsigned long i = 0;
        for (i = 0 ; i < NUM_RANDCALLS;i++){
                //get a random syscall offset
                offset = rand_gen(NUM_SYSCALLS);
                //syscall
                func_ptr[offset]();
        }
}

/*
 * Execute each syscall in the list
 * */
void execute_syscalllist(void){

        printf("Executing syscall...\n");
        syscalldata *data = NULL;
        int i = 0,ret = 0;
        int *pfd = NULL;
        //syscall
        for(i = 0; i < NUM_RANDCALLS; i++){
                //saving env for safe return point.
                while(sigsetjmp(env,1)){
                        if(i < 0 || i > NUM_RANDCALLS){
                                printf("something really bad happened\n");
                                exit(1);
                        }
                        i++;
                        //if it is the last random call exit
                        if(i == NUM_RANDCALLS)  return;
                        printf("%d: FAULT\n", i);
                }
                //get a syscall
                data = (syscalldata *)getsyscall(i);
                //execute
                //printf("SYSCALL: %d\n", data->number);
                ret = syscall(data->number, data->args[0],data->args[1],
                                data->args[2],data->args[3],data->args[4],
                                data->args[5],data->args[6]);
                //add opened fds for future use
                if(ret != -1){
                        switch(data->number){
                                case SYS_open  :
                                case SYS_dup   :
                                case SYS_dup2  :
#ifndef OLDER_LINUX
                                case SYS_dup3  : 
#endif
                                        if(rand_gen(10)) close(ret);        
                                        else addfd(ret, 'f');         
                                        break;
#ifndef OLDER_LINUX
                                case SYS_pipe2 : pfd = (int *)data->args[0];
                                                 if(rand_gen(20)) { close(pfd[0]); close(pfd[1]);}
                                                 else { addfd(pfd[0],'z'); addfd(pfd[1],'p'); }
                                                 break;
#endif
                                case SYS_openat: if(rand_gen(10)) close(ret);
                                                         else addfd(ret,'d');                
                                                         break;
                                default        : break;

                        }
                }
        }   
}

