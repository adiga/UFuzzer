/* Vikram Adiga
 * Provides malloc and free sys calls and keeps track of memory usage
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "xmalloc.h"

#define hashkey 255
//store memory allocated and freed
static unsigned long long  memalloc = 0, memfree = 0;
//Table to store the addresses
typedef struct node{
        void *p; 
        int size;
        struct node *next;
}hashnode;
static hashnode *hasharray[hashkey+1] = {NULL};

/*add an entry to hash table*/
void addhashentry(void *ptr,int size);
/*del an entry and return the size*/
int delhashentry(void *ptr);

void addhashentry(void *ptr,int size){
        int offset = (int)ptr & hashkey;
        hashnode *newnode = (hashnode *)malloc(sizeof(hashnode));
        assert(newnode != NULL);
        memset(newnode, 0, sizeof(hashnode));
        newnode->p = ptr;
        newnode->size = size;

        if(hasharray[offset] != NULL){
                newnode->next = hasharray[offset];
        }
        hasharray[offset] = newnode;

        // printf("A : offset: %d, ptr = %d\n", offset, (int)ptr);  

}
int delhashentry(void *ptr){
        int offset = (int)ptr & hashkey;
        int size;
        hashnode *curr, *old;

        //get the hashlist from the hasharry
        old = curr = (hashnode *)hasharray[offset];

        //printf("D : offset: %d, ptr = %d\n", offset, (int)ptr);   

        assert(curr != NULL);

        //search for the ptr in the hashlist
        while((curr != NULL) && curr->p != ptr){
                old = curr;
                curr = curr->next;
        }
        assert(curr != NULL);
        size = curr->size;

        if(curr == old){
                hasharray[offset] = curr->next;
        }else{
                old->next = curr->next;
        }
        free(curr);
        return size;

}

void *xmalloc(size_t size){

        void *ptr = malloc(size);
        assert(ptr != NULL);
        memset(ptr, 0 ,size);
        addhashentry(ptr,size);
        memalloc += size;

        return ptr;

}

void xfree(void *ptr){
        memfree += delhashentry(ptr);
        free(ptr);

}

void xprintmem(){
        printf("Memory usage(bytes): allocated: %llu, freed: %llu, unfreed: %llu\n", memalloc,memfree, (memalloc - memfree));  
}
