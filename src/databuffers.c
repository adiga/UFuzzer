/* Data management file : store, retrieve, clean linked lists of data
 * */
#include "common-header.h"
#include "databuffers.h"
#include "lib/xmalloc.h"
#include "random.h"

//void deletefd(int fd);
void delete_files(void);

static linkedlist *fd_list = NULL, *dfd_list = NULL, *pfd_list = NULL, *zfd_list = NULL, *file_list = NULL,
                  *syscall_list = NULL,*best_syscall_list = NULL,  *address_list = NULL;
static int nfd = 0, ndfd = 0, npfd = 0, nzfd = 0, nfiles = 0, nsyscalls = 0, naddrs = 0;


/* Free all resources
 * */
void free_resources(void){

        clearfdlist();
        clearaddresslist();
        clearsyscall();
        clearlist(&best_syscall_list);
        delete_files();
        xprintmem();

}
void clearsyscall(void){
        clearlist(&syscall_list);
        nsyscalls = 0;

}

/* get number of open files*/
int getnfd(void){
        return (nfd + ndfd + npfd);
}


/* Add fd to the fd_list
 * Increment the number of open files*/
void addfd(int fd, char type){
        switch(type){
                case 'f':  addnode(&fd_list, &fd, sizeof(fd)); // add node
                           nfd++;
                           if(nfd > 100) { //remove a node 
                                int *fdptr = getnodedata(&fd_list,rand_gen(nfd) + 1);
                                assert(fdptr != NULL);
                                close(*fdptr);
                                delnode(&fd_list, fdptr);
                                nfd--;
                           }
                           break;

                case 'd':  addnode(&dfd_list, &fd, sizeof(fd));
                           ndfd++;
                           if(ndfd > 50){
                                int *dfd = getnodedata(&dfd_list, rand_gen(ndfd) + 1);
                                assert(dfd != NULL);
                                close(*dfd);
                                delnode(&dfd_list, dfd);
                                ndfd--;
                           }
                           break;

                case 'p': addnode(&pfd_list, &fd, sizeof(fd));
                          npfd++;
                          if(npfd > 6){
                                int *pfd = getnodedata(&pfd_list, npfd);
                                assert(pfd != NULL);
                                close(*pfd);
                                delnode(&pfd_list, pfd);
                                npfd--;
                          }
                          break;

                case 'z': addnode(&zfd_list, &fd, sizeof(fd));
                          nzfd++;
                          if(nzfd > 6){
                                int *zfd = getnodedata(&zfd_list, nzfd);
                                assert(zfd != NULL);
                                close(*zfd);
                                delnode(&zfd_list, zfd);
                                nzfd--;
                          }
                          break;

                default: printf("Error in addfd: Should not happen\n");
                         exit(1);
        }
}

/* Get an random fd  */
int getfd(void){
        int *fd = NULL;
        switch(rand_gen(3)){
                case 0: if(nfd == 0) return getfd();
                        fd = getnodedata(&fd_list, rand_gen(nfd) + 1);
                        assert(fd != NULL);
                        break;
                case 1: if(ndfd == 0) return getfd();
                        fd = getnodedata(&dfd_list, rand_gen(ndfd) + 1);
                        assert(fd != NULL);
                        break;
                case 2: if(npfd == 0) return getfd();
                        fd = getnodedata(&pfd_list, rand_gen(npfd) + 1);
                        assert(fd != NULL);
                        break;
                default: printf("Error in getfd: Should not happen\n");
                         exit(1);
        }
        return *fd;  
}


/* delete fd from the fd_list 
 * decrement the number of open files*/
/*
void deletefd(int fd){

        int i, *fdptr = NULL;
        //return if nopenfiles is zero
        if(nopenfiles == 0) return;
        //Not the best way
        for(i = 0; i < nopenfiles; i++){
                fdptr = getnodedata(&fd_list, i+1);
                assert(fdptr != NULL);
                if(*fdptr == fd) break;
        }
        //delete fd from the fd_list
        delnode(&fd_list, fdptr);
        //decrement number of openfiles;
        nopenfiles--;

}*/

/*clear the all fd */
void clearfdlist(void){
        
        int i  = 0, *fd = NULL;
        //fd
        for(i = 0; i < nfd; i++){
                fd = getnodedata(&fd_list, i+1);
                close(*fd);
        }
        clearlist(&fd_list);
        nfd = 0;

        //dfd
        for(i = 0; i < ndfd; i++){
                fd = getnodedata(&dfd_list, i+1);
                close(*fd);
        }
        clearlist(&dfd_list);
        ndfd = 0;
       
        //pfd 
        for(i = 0; i < npfd; i++){
                fd = getnodedata(&pfd_list, i+1);
                close(*fd);
        }
        clearlist(&pfd_list);
        npfd = 0;
        
        //zfd
        for(i = 0; i < nzfd; i++){
                fd = getnodedata(&zfd_list, i+1);
                close(*fd);
        }
        clearlist(&zfd_list);
        nzfd = 0;

}

/* Return number of files created */
int getnfiles(void){
        return nfiles;
}

/* Add filename to file list */
void addfilename(char *filename, int size){
        assert(filename != NULL);
        addnode(&file_list, filename, size);
        nfiles++;
}

/* Get a filename from the file list */
char *getfilename(void){
        char *filename = (char *)getnodedata(&file_list, rand_gen(nfiles) + 1);
        assert(filename != NULL);
        return filename;
}	

/*delete all the files */
void delete_files(void){
        int i;
        char *fileptr = NULL;
        for(i = 0; i < nfiles; i++){
                fileptr = (char *)getnodedata(&file_list, i + 1);
                assert(fileptr != NULL);
                remove(fileptr);
        }
        clearlist(&file_list);
        nfiles = 0;
        remove(FILE_DIR);
}


/*add address to the address list*/
void addaddress(void *address, unsigned int size){
        assert(address != NULL);
        addnode(&address_list,address, size);
        naddrs++;
}

/*get first address */
void *getaddress(void){
        void *address = getnodedata(&address_list, 1);
        assert(address != NULL);
        return address;
}

/*free each address*/
void clearaddresslist(void){
        clearlist(&address_list);
        naddrs = 0;
}

/*forcefully free each address*/
void force_clearaddresslist(void){
        address_list = NULL;
        naddrs = 0;
        clearlist(&syscall_list);
        nsyscalls = 0;
}

/*Return syscall list and clear syscall list*/
linkedlist *getsyscalllist(){
        linkedlist *temp = syscall_list;
        syscall_list = NULL;
        nsyscalls = 0;
        return temp;
}


/*Add syscall from syscall list */
void addsyscall(syscalldata *data, int size){
        assert(data != NULL);
        addnode(&syscall_list, data, size);
        nsyscalls++;
        if(nsyscalls > NUM_RANDCALLS) {
                printf("Error: num of syscall exceeding NUM_RANDCALLS\n");
                exit(1);
        }
}

/*Get sycall from syscall list */
syscalldata *getsyscall(int nodenum){

        syscalldata *data = (syscalldata *)getnodedata(&syscall_list, nodenum + 1);
        //assert(data !=NULL);
        if(data == NULL){
        printf("Error : exiting ..... number of syscalls: %d nodenum: %d\n", nsyscalls, nodenum);
        exit(1);
        }
        return data;		

}

/*Copy syscall list to best syscall list*/
void copy_syscall_to_best(void){
        if(best_syscall_list != NULL)  clearlist(&best_syscall_list);
        int i = 0;
        for(i = NUM_RANDCALLS; i > 0; i--){
                syscalldata *data = (syscalldata *)getnodedata(&syscall_list, i);
                assert(data != NULL);
                addnode(&best_syscall_list, data,sizeof(syscalldata));
        }
}

/*Copy best syscall to syscall list*/
void copy_best_to_syscall(void){
        if(syscall_list != NULL) clearlist(&syscall_list);
        int i = 0;
        for(i = NUM_RANDCALLS; i > 0; i--){
                syscalldata *data = (syscalldata *)getnodedata(&best_syscall_list, i);
                assert(data != NULL);
                addnode(&syscall_list, data,sizeof(syscalldata));
        }
}
