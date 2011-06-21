/* All syscalls defined here
 **/
#include "common-header.h"
#include <stdarg.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <termios.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/ax25.h>
#include <linux/cdk.h>
#include <time.h>
#include "syscalls.h"
#include "random.h"
#include "randomdata.h"
#include "databuffers.h"
#include "lib/xmalloc.h"
#ifndef OLDER_LINUX
#include <linux/falloc.h>
#endif

/* Adds syscalls to a list */
void mysyscall(int number, ...){

        va_list arg;
        int i = 0;
        unsigned long argbuf[MAX_SYSCALL_ARGS] = {0};

        /*Read the args into a buffer*/
        va_start(arg, number);
        for(i = 0; i < MAX_SYSCALL_ARGS; i++){
                argbuf[i] = va_arg(arg, unsigned long);
        }
        va_end(arg);

        /*bit flip 5% of times*/
        /*SYS_pipe2 ignored to avoid blocking*/
#ifndef OLDER_LINUX 
             if((DEBUG == 0) &&  (rand_gen(20) < 1) && (number != SYS_pipe2)){
#else
        if((DEBUG == 0) && (rand_gen(20) < 1)){
#endif
                int num_args = rand_gen(MAX_SYSCALL_ARGS);
                int it = 0;
                for(it = 0; it <= num_args; it++){
                        int off = rand_gen(MAX_SYSCALL_ARGS);
                        if(!(( (number == SYS_readv) || (number == SYS_read)) && off == 0))
                                argbuf[off] = rand_bitflip(argbuf[off]);
                }
        }
        /* populate syscalls and add it to list*/
        syscalldata data = {0};
        data.number = number;
        data.args[0] = argbuf[0];
        data.args[1] = argbuf[1];
        data.args[2] = argbuf[2];
        data.args[3] = argbuf[3];
        data.args[4] = argbuf[4];
        data.args[5] = argbuf[5];
        data.args[6] = argbuf[6];
        addsyscall(&data, sizeof(syscalldata));
}



/* open syscall
 * int open(const char *pathname, int flags, mode_t mode);
 * */
void myopen(void){
        mysyscall(SYS_open, rand_pathname(),rand_filemode(),rand_statusmode());
}


/* close syscall 
 * int close(int fd);
 * */
void myclose(void){
        mysyscall(SYS_close, rand_fd());
} 

/* read syscall 
 * ssize_t read(int fd, void *buf, size_t count);
 * */
void myread(void){
        unsigned int size = rand_gen(PAGE_SIZE);
        while(size == 0) size = rand_gen(PAGE_SIZE);
        char *buf = rand_buf('r', size);
        mysyscall(SYS_read, rand_fd(), buf, size);
}

/* write syscall 
 * ssize_t write(int fd, const void *buf, size_t count);
 * */
void mywrite(void){
        unsigned int size = rand_gen(PAGE_SIZE);
        while(size == 0) size = rand_gen(PAGE_SIZE);
        char *buf = rand_buf('w', size);
        mysyscall(SYS_write, rand_fd(), buf, size);
}

/* fsync call
 * int fsync(int fd);
 * */
void myfsync(void){
        mysyscall(SYS_fsync, rand_fd());
}

/* fdatasync call 
 * int fdatasync(int fd);
 * */
void myfdatasync(void){
        mysyscall(SYS_fdatasync, rand_fd());
}

/* fstatsfs call 
 * int fstatfs(int fd, struct statfs *buf);
 * */
void myfstatfs(void){ 
        mysyscall(SYS_fstatfs, rand_fd(),(struct statfs *)rand_buf('r', sizeof(struct statfs)));
}

/* fstat call 
 * int fstat(int fd, struct stat *buf);
 * */
void myfstat(void){
        mysyscall(SYS_fstat, rand_fd(), (struct stat *)rand_buf('r', sizeof(struct stat)));
}

/* dup call 
 * int dup(int oldfd);
 * */
void mydup(void){
        mysyscall(SYS_dup, rand_fd());
}

/* dup2 call 
 * int dup2(int oldfd, int newfd);
 * */
void mydup2(void){
        mysyscall(SYS_dup2, rand_fd(), rand_fd());
}

/* fchdir call 
 * int fchdir(int fd);
 * */
void myfchdir(void){
        mysyscall(SYS_fchdir, rand_fd());
}

/* fchmod call
 * int fchmod(int fd, mode_t mode);
 * */
void myfchmod(void){
        mysyscall(SYS_fchmod, rand_fd(), rand_statusmode());
}

/* readahead syscall 
 * ssize_t readahead(int fd, off64_t offset, size_t count);
 * */
void myreadahead(void){
        mysyscall(SYS_readahead, rand_fd(), rand64(), rand32());
}

/* readv syscall
 * ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
 * */
void myreadv(void){
        int iovcnt = rand_gen(IOVEC_BUF_MAX);
        while(iovcnt == 0) iovcnt = rand_gen(IOVEC_BUF_MAX);
        mysyscall(SYS_readv, rand_fd(), (struct iovec *)rand_iovec(iovcnt), iovcnt);
}

/* writev syscall 
 * ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
 * */
void mywritev(void){
        int iovcnt = rand_gen(IOVEC_BUF_MAX);
        while(iovcnt == 0) iovcnt = rand_gen(IOVEC_BUF_MAX);
        mysyscall(SYS_writev, rand_fd(), (struct iovec *)rand_iovec(iovcnt), iovcnt);
}


/* ftruncate syscall
 * int ftruncate(int fd, off_t length);
 * */
void myftruncate(void){
        mysyscall(SYS_ftruncate, rand_fd(), rand32());
}

/* sendfile syscall 
 * ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
 * */
void mysendfile(void){
        mysyscall(SYS_sendfile, rand_fd(), rand_fd(), (off_t *)rand_buf('w', sizeof(off_t)), rand32());
}

/* lseek syscall 
 * off_t lseek(int fd, off_t offset, int whence);
 * */
void mylseek(void){
        unsigned int whence[] = {SEEK_SET, SEEK_CUR, SEEK_END, rand32()};
        mysyscall(SYS_lseek, rand_fd(), rand32(), whence[rand_gen(sizeof(whence)/sizeof(whence[0]))]);
}

/* pread64 syscall
 * ssize_t pread64(int file_descriptor, void *buf, size_t nbyte, off64_t offset);
 * */
void mypread64(void){
        size_t size = rand_gen(PAGE_SIZE);
        while(size == 0) size = rand_gen(PAGE_SIZE);
        void *buf = (void *) rand_buf('r', size);
        mysyscall(SYS_pread64, rand_fd(), buf, size, rand64());
}

/* pwrite64 syscall 
 * ssize_t pwrite64 (int file_descriptor, const void *buf, size_t nbyte, off64_t offset);
 * */
void mypwrite64(void){
        size_t size = rand_gen(PAGE_SIZE);
        while(size == 0) size = rand_gen(PAGE_SIZE);
        const void *buf = (const void *) rand_buf('w', size);
        mysyscall(SYS_pwrite64, rand_fd(), buf, size, rand64());
}

/* openat syscall 
 * int openat(int dirfd, const char *pathname, int flags, mode_t mode);
 * */
void myopenat(void){
        mysyscall(SYS_openat, rand_fd(), rand_pathname(), rand_filemode(),rand_statusmode());
}

/* fchown syscall
 * int fchown(int fd, uid_t owner, gid_t group);
 * */
void myfchown(void){
        uid_t uid = (rand_gen(20) > 0)? getuid() : rand32();
        gid_t gid =  (rand_gen(20) > 0)? getgid() : rand32();
        mysyscall(SYS_fchown, rand_fd(), uid, gid); 
}

/* mkdirat syscall
 * int mkdirat(int dirfd, const char *pathname, mode_t mode);
 * */
void mymkdirat(void){
        mysyscall(SYS_mkdirat, rand_fd(), rand_pathname(), rand_statusmode());
}

/* mkdir syscall
 * int mkdir(const char *pathname, mode_t mode);
 * */
void mymkdir(void){
        mysyscall(SYS_mkdir, rand_pathname(), rand_statusmode());
}

/* unlinkat syscall
 * int unlinkat(int dirfd, const char *pathname, int flags);
 * */
void myunlinkat(void){
        int flags[] = {0, AT_REMOVEDIR, rand32()};
        mysyscall(SYS_unlinkat, rand_fd(), rand_pathname(), flags[rand_gen(sizeof(flags)/sizeof(flags[0]))]);
}

/* unlink syscall
 * int unlink(const char *pathname);
 * */
void myunlink(void){
        mysyscall(SYS_unlink, rand_pathname());
}

/* renameat syscall
 * int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath);
 * */
void myrenameat(void){
        mysyscall(SYS_renameat, rand_fd(), rand_pathname(), rand_fd(), rand_pathname());
}

/* rename syscall
 * int rename(const char *oldpath, const char *newpath);
 * */
void myrename(void){
        mysyscall(SYS_rename, rand_pathname(), rand_pathname());
}

/* fchmodat call
 * int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
 * */
void myfchmodat(void){
        mysyscall(SYS_fchmodat, rand_fd(), rand_pathname(), rand_statusmode(), rand_dirflags()); 
}

/* fchownat syscall
 * int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
 * */
void myfchownat(void){
        uid_t uid = (rand_gen(20) > 0)? getuid() : rand32();
        gid_t gid = (rand_gen(20) > 0)? getgid() : rand32();
        mysyscall(SYS_fchownat, rand_fd(), rand_pathname(), uid, gid, rand_dirflags());
}

/* faccessat syscall
 * int faccessat(int dirfd, const char *pathname, int mode, int flags);
 * */
void myfaccessat(void){
        int rwx[] = {0, R_OK, W_OK, X_OK};
        int size = sizeof(rwx)/sizeof(rwx[0]);
        int mode = (rand_gen(10) > 0)? (rwx[rand_gen(size)]|rwx[rand_gen(size)]|rwx[rand_gen(size)]) : F_OK;
        int flags[] = {0, AT_EACCESS, AT_SYMLINK_NOFOLLOW, rand32()};
        mysyscall(SYS_faccessat, rand_fd(), rand_pathname(), mode, flags[rand_gen(sizeof(flags)/sizeof(flags[0]))]);
}

/* access syscall 
 * int access(const char *pathname, int mode);
 * */
void myaccess(void){
        int rwx[] = {0, R_OK, W_OK, X_OK};
        int size = sizeof(rwx)/sizeof(rwx[0]);
        int mode = rand_gen(2)? F_OK : (rwx[rand_gen(size)]|rwx[rand_gen(size)]|rwx[rand_gen(size)]);
        mysyscall(SYS_access, rand_pathname(), mode);
}

/* symlinkat syscall
 * int symlinkat(const char *oldpath, int newdirfd, const char *newpath);
 * */
void mysymlinkat(void){
        mysyscall(SYS_symlinkat, rand_pathname(), rand_fd(), rand_pathname());
}

/* symlink syscall
 * int symlink(const char *oldpath, const char *newpath);
 * */
void mysymlink(void){
        mysyscall(SYS_symlink, rand_pathname(), rand_pathname());
}

/* linkat syscall
 * int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);
 * */
void mylinkat(void){
        int flags[] = {0, AT_SYMLINK_FOLLOW, rand32()};
        mysyscall(SYS_linkat, rand_fd(), rand_pathname(), rand_fd(), rand_pathname(), flags[rand_gen(sizeof(flags)/sizeof(flags[0]))]);
}

/* link syscall
 * int link(const char *oldpath, const char *newpath);
 * */
void mylink(void){
        mysyscall(SYS_link, rand_pathname(), rand_pathname());
}

/* readlink syscall
 * ssize_t readlink(const char *path, char *buf, size_t bufsiz);
 * */
void myreadlink(void){
        size_t size = rand_gen(PAGE_SIZE);
        while(size == 0) size = rand_gen(PAGE_SIZE);
        char *buf = rand_buf('r', size);
        mysyscall(SYS_readlink, rand_pathname(), buf, size);
}

/* readlinkat syscall
 * int readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);
 * */
void myreadlinkat(void){
        size_t size = rand_gen(PAGE_SIZE);
        while(size == 0) size = rand_gen(PAGE_SIZE);
        char *buf = rand_buf('r', size);
        mysyscall(SYS_readlinkat, rand_fd(), rand_pathname(),buf, size);
}

/* ioctl syscall
 * long sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);
 * */
void myioctl(void){
        unsigned long *arg = NULL;
        if(DEBUG || rand_gen(2)){
                arg = (unsigned long *)rand_buf('w', getpagesize());
        }else{
                arg = (unsigned long *)rand32();
        }
        mysyscall(SYS_ioctl, rand_fd(), rand32(), arg);
}

/* fstatsfs64 call 
 * long sys_fstatfs64(unsigned int fd, size_t sz, struct statfs64 __user *buf);
 * */
void myfstatfs64(void){
        size_t sz = sizeof(struct statfs64) * rand_gen(1000);
        while(sz == 0) sz = sizeof(struct statfs64) * rand_gen(1000);
        mysyscall(SYS_fstatfs64, rand_fd(), sz, (struct statfs64 *)rand_buf('r', sz));
}

/* fstat64 syscall
 * int fstat64(int fildes, struct stat64 *buf);
 * */
void myfstat64(void){
        mysyscall(SYS_fstat64, rand_fd(), (struct stat64 *)rand_buf('r', sizeof(struct stat64)));
}

/* ftruncate64 syscall
 * int ftruncate64(int fildes, off64_t length);
 * */
void myftruncate64(void){
        mysyscall(SYS_ftruncate64, rand_fd(), rand64());
}

/* fadvise64_64 syscall - fadvise64 has been skipped since it is obselete
 * long sys_fadvise64 (int fs, loff_t offset, loff_t len, int advice, int fs, loff_t offset, loff_t len, int advice);
 * */
void myfadvise64_64(void){
        int advise[] = {POSIX_FADV_NORMAL, POSIX_FADV_SEQUENTIAL, 
                POSIX_FADV_RANDOM, POSIX_FADV_NOREUSE,
                POSIX_FADV_WILLNEED, POSIX_FADV_DONTNEED, rand32()};
        int index = rand_gen(sizeof(advise)/sizeof(int));
        mysyscall(SYS_fadvise64_64, rand_fd(), rand64(), rand64(), advise[index]);
}

/* llseek syscall 
 * int _llseek(unsigned int fd, unsigned long offset_high, unsigned long offset_low, loff_t *result, unsigned int whence);
 * */
void myllseek(void){
        unsigned int whence[] = {SEEK_SET, SEEK_CUR, SEEK_END, rand32()};
        mysyscall(SYS__llseek, rand_fd(), rand32(), rand32(), (loff_t *)rand_buf('w', sizeof(loff_t)), whence[rand_gen(sizeof(whence)/sizeof(whence[0]))]);
}

/* fstatat64 syscall
 * long sys_fstatat64(int dfd, char __user *filename, struct stat64 __user *statbuf, int flag);
 * */
void myfstatat64(void){
        mysyscall(SYS_fstatat64, rand_fd(), rand_pathname(), (struct stat64 *)rand_buf('r', sizeof(struct stat64)), rand_dirflags());
}

/* ***************************************************************************************************
 * ************* Below this line are calls which cannot be compiled on 64 bit machines ***************
 * **************************************************************************************************/

#ifndef OLDER_LINUX
/* fallocate syscall
 * long fallocate(int fd, int mode, loff_t offset, loff_t len);
 * */
void myfallocate(void){
        mysyscall(SYS_fallocate, rand_fd(), FALLOC_FL_KEEP_SIZE, rand64(), rand64());
}

/* pipe2 syscall 
 * int pipe2(int pipefd[2], int flags);
 * NOTE: pipe call has been skipped since it may result in blocking
 * */
void mypipe2(void){
        mysyscall(SYS_pipe2, (int *)rand_buf('r', sizeof(int) * 2), (O_NONBLOCK|O_CLOEXEC));
}

/* dup3 syscall
 * int dup3(int oldfd, int newfd, int flags);
 * */
void mydup3(void){
        mysyscall(SYS_dup3, rand_fd(), rand_fd(), O_CLOEXEC);
}

/* preadv syscall
 * long sys_preadv(unsigned long fd, const struct iovec __user *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h);
 * */
void mypreadv(void){
        unsigned long vlen = rand_gen(IOVEC_BUF_MAX);
        while(vlen == 0) vlen = rand_gen(IOVEC_BUF_MAX);
        mysyscall(SYS_preadv, rand_fd(), rand_iovec(vlen), vlen, rand32(), rand32());
}

/* pwritev syscall 
 * long sys_pwritev(unsigned long fd, const struct iovec __user *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h);
 * */
void mypwritev(void){
        unsigned long vlen = rand_gen(IOVEC_BUF_MAX);
        while(vlen == 0) vlen = rand_gen(IOVEC_BUF_MAX);
        mysyscall(SYS_pwritev, rand_fd(), rand_iovec(vlen), vlen, rand32(), rand32());
}


/* utimensat syscall
 * int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags);
 * */
void myutimensat(void){
        struct timespec *times = (struct timespec *)rand_buf('w', sizeof(struct timespec) * 2);
        /*	long nsecflags[] = {UTIME_NOW, UTIME_OMIT, rand32()};
                if(times != NULL && rand_gen(2)){
                times[0].tv_sec  = (rand_gen(5) < 1)? rand32() : time(NULL); 
                times[0].tv_nsec = nsecflags[rand_gen(sizeof(nsecflags)/sizeof(nsecflags[0]))];         
                times[1].tv_sec  = (rand_gen(5) < 1)? rand32() : time(NULL); 
                times[1].tv_nsec = nsecflags[rand_gen(sizeof(nsecflags)/sizeof(nsecflags[0]))];         
                }*/
        mysyscall(SYS_utimensat, rand_fd(), rand_pathname(), times, rand_dirflags());
}
#endif

/*fcntl syscall */
/*
   void myfcntl(void){

   int fd = rand_fd_gen();
   int cmd[] = {F_DUPFD, F_DUPFD_CLOEXEC, F_GETFD, F_SETFD, 
   F_GETFL, F_SETFL, F_GETLK, F_SETLK, F_SETLKW};
   long arg[] = {0, 0, 1, 0, 1, 0, 2, 2, 2 };
   struct flock lock = {0};
   int off = rand_gen(sizeof(cmd)/sizeof(cmd[0]));


   }*/




