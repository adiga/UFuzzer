#include "lib/linkedlist.h"
void free_resources(void);

int getnfd(void);
void addfd(int fd, char type);
int getfd(void);
void clearfdlist(void);

int getnfiles(void);
void addfilename(char *filename, int size);
char *getfilename(void);

void addaddress(void *address, unsigned int size);
void *getaddress(void);
void clearaddresslist(void);
void force_clearaddresslist(void);

linkedlist *getsyscalllist();
void addsyscall(syscalldata *data, int size);
syscalldata *getsyscall(int nodenum);
