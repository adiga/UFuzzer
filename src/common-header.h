/* Contains the shared data among all files
 * Modify the options as required
 * */

#define _GNU_SOURCE
#define _ATFILE_SOURCE
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/******* Changes the below options as required *****/
#define DEBUG 0 // 1 - TURN ON DEBUG MODE, 0 - TURN ON ALL FEATURES
#define CODE_COVERAGE 1 //CODE COVERAGE: 1 - ON , 0 - 0FF
#define NUM_RANDCALLS 10000 //Number of random calls 
#define NUM_RUNS 1000 //Number of runs of random calls
//#define OLDER_LINUX  //uncomment this if compilation problem encountered

/****** Changes the options below this if you understand the impact of changes *****/
#ifndef OLDER_LINUX
#define NUM_SYSCALLS 51 //Should be updated as syscalls are added
#else
#define NUM_SYSCALLS 45
#endif

/*file and dir value should not be more than 9 digits*/
#define MAX_FILES 100
#define IOVEC_BUF_MAX 20  // IOVEV_BUF_MAX should be less than max buffers
#define PAGE_SIZE (getpagesize()/4)
#define RANDOM_MAX RAND_MAX 
#define FD_MAX RAND_MAX
#define FILE_DIR "/tmp/data/"
#define MAX_SYSCALL_ARGS 7 

typedef struct{
        int number;
        unsigned long args[MAX_SYSCALL_ARGS];
}syscalldata;

