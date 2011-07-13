/* Random generation of data specific to system call inputs
 **/ 
#include "common-header.h"
#include "lib/xmalloc.h"
#include "randomdata.h"
#include "random.h"
#include "databuffers.h"

/* Returns a 32 bit value */
unsigned int rand32(void){
        return rand_gen(RANDOM_MAX);
}

/* Returns a 64 bit value */
long long rand64(void){
        return ((long long)rand_gen(RANDOM_MAX) << 32) | rand_gen(RANDOM_MAX);
}

/* generate a random string */
void rand_string_gen(char *buf, int buf_len){
        // 40 chars       
        const char alphabet[] = "abcdefghijklmnopqrstuvwxyz1234567890.,_ ";
        if(buf_len != 0){
                int i;
                for(i = 0; i < buf_len -1; i++){
                        buf[i] = alphabet[rand_gen(sizeof(alphabet)-1)];
                }
                buf[i] = '\0';
        }	
}

/* generate a random number if linked fd_list is empty
 * or every random 10% of the time */
int rand_fd(void){
        int fd = 0;
        //num of fds zero or 10% of time
        if((getnfd() == 0) || (rand_gen(10) < 1)){
                while((fd == 0) || (fd == 1) || (fd == 2)){
                        fd = rand_gen(FD_MAX);
                }
        }else{
                //if there is fd in the fd_list
                fd = getfd();
        }
        return fd;
}

/* Returns a random name file */
char *rand_pathname(void){
        char file[32] ={0};
        char *filename = file;
        if(getnfiles() == 0){
               if(mkdir(FILE_DIR,0777) == -1){
                       printf("Cannot create %s\n", FILE_DIR);
                       printf("Run ./clean.sh and then rerun the code\n");
                       exit(1);
               }
                //create files
                int i, fd;
                for(i = 0 ; i < MAX_FILES; i++){
                        sprintf(filename,"%s%s%d", FILE_DIR,"f", i);
                        fd = open(filename, O_CREAT, 0666);
                        assert(fd != -1);
                        addfilename(filename, sizeof(file)); 
                        close(fd);
                }
        }
        //TODO return files from /dev /proc
        if(DEBUG || rand_gen(10)){
                filename = (char *)getfilename();
        }else{
                filename = (char *)rand_gen(RANDOM_MAX);
        }

        return filename; 
}

/*
 * Generates buf of the specified size
 * Sometimes returns invalid random address
 */
void *rand_buf(char rw, unsigned int size){
        void *p = NULL;
        if(DEBUG || rand_gen(10)){
                p = (void *)xmalloc(size);
                switch(rw){
                        case 'r': memset(p, 0, size);
                                  break;
                        case 'w': rand_string_gen(p, size);
                                  break;
                        default : printf("ERROR IN RAND_BUF\n");
                                  break;
                }
                addaddress(p, size);
                xfree(p);
                p = getaddress();
        }else{
                p = (void *)rand_gen(RANDOM_MAX);  
        }    
        return p;
}

/*Generate randomly filled iovec struct */
struct iovec *rand_iovec(int size){
        int i = 0;
        struct iovec *iov = NULL;
                if(DEBUG || rand_gen(10)){
                        iov = (struct iovec *)xmalloc(sizeof(struct iovec) * size);
                        //memset(iov, 0, sizeof(struct iovec) * size);
                        for(i = 0; i < size; i++){
                                unsigned int bsize = rand_gen(PAGE_SIZE);
                                while(bsize == 0) bsize = rand_gen(PAGE_SIZE);
                                char *buf = rand_buf('r', bsize);
                                //10% bit flipping
                                if(DEBUG || rand_gen(10)){ 
                                        iov[i].iov_base = buf;
                                        iov[i].iov_len = bsize;
                                }else{
                                        iov[i].iov_base = (char *)rand_bitflip((unsigned long)(buf));
                                        iov[i].iov_len = rand_bitflip(bsize);
                                }
                        }
                        addaddress(iov,(sizeof(struct iovec) * size));
                        xfree(iov);
                        iov = getaddress();
                }else{
                        iov = (struct iovec *) rand_gen(RANDOM_MAX);
                }

        return iov;
}


/* Generates a combination of status mode*/
mode_t rand_statusmode(void){
        //status
        mode_t m1[] = {0, S_IRWXU, S_IRUSR, S_IWUSR, S_IXUSR};
        mode_t m2[] = {0, S_IRWXG, S_IRGRP, S_IWGRP, S_IXGRP};
        mode_t m3[] = {0, S_IRWXO, S_IROTH, S_IWOTH, S_IXOTH};
        mode_t mode =  m1[rand_gen(sizeof(m1)/sizeof(mode_t))] | m2[rand_gen(sizeof(m2)/sizeof(mode_t))] | m3[rand_gen(sizeof(m3)/sizeof(mode_t))] ;
        return (rand_gen(10) == 0) ? mode : rand_gen(65535);
} 

/*Generates a file mode */
mode_t rand_filemode(void){
        //flags to open the file
        mode_t accessflags[] = {O_RDONLY, O_RDWR, O_WRONLY};
        mode_t createflags[] = {0, O_CREAT, O_TRUNC};
        mode_t statusflags[] = {0, O_APPEND, O_ASYNC, O_DIRECT, O_LARGEFILE,  O_NOFOLLOW, O_NONBLOCK, O_SYNC};
        mode_t flags = accessflags[rand_gen(sizeof(accessflags)/sizeof(mode_t))] | createflags[rand_gen(sizeof(createflags)/sizeof(mode_t))] 
                | statusflags[rand_gen(sizeof(statusflags)/sizeof(mode_t))];

        return (rand_gen(10) == 0) ? flags : rand_gen(65535);        
}        

/*generates flags for dir based syscalls like fchownat,
 * fchmodat,fstatat64*/
int rand_dirflags(void){

        int flags[] = {0 , AT_SYMLINK_NOFOLLOW};
        if(rand_gen(10)){
                return  flags[rand_gen(sizeof(flags)/sizeof(flags[0]))];
        } else{
                return rand_gen(RANDOM_MAX);
        }
}


unsigned long rand_bitflip(unsigned long data){
        if(DEBUG){       
                return data;
        }else{
                int i = 0;
                int nbits = rand_gen(2);
                for(i = 0; i < nbits; i++){
                        data ^= (1 << rand_gen(2));
                }
                //TODO - only fd should not be 0 or 1 or 2
                while((data == 0 )|| (data == 1) ||(data == 2)){
                        data = rand_gen(50);
                }
                return data;
        }
}

