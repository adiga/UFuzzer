/* Vikram Adiga
 * Generic linked list add node, delete node, get data from node
 * and clear entire list
 */
#include "linkedlist.h"
#include "xmalloc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void addnode(linkedlist **ptr, void *data, unsigned int size){
        linkedlist *newnode = NULL;
        int i = 0;

        //creating memory for node and data
        newnode = (linkedlist *)xmalloc(sizeof(linkedlist));
        assert(newnode);
        memset(newnode, 0, sizeof(linkedlist));
        assert(size != 0);
        newnode->data = xmalloc(size);
        assert(newnode->data);
        memset(newnode->data, 0, size);

        // printf("A: ptr: %d %d\n",newnode,*ptr);
        //copy data byte by byte
        for(i = 0; i < size; i++){
                *((char *)newnode->data + i) = *((char *)data + i);
        }

        //add at the head
        newnode->next = *ptr;
        //add it back to pointer
        *ptr = newnode;
}

void delnode(linkedlist **ptr, void *data){
        linkedlist *old, *curr;

        old = curr = *ptr;
        assert(curr != NULL);
        while(curr != NULL &&  curr->data != data){
                old = curr;
                curr = curr->next;

        }
        assert(curr != NULL);
        //printf("D: ptr: %d\n",curr);
        if(curr == old){
                *ptr = curr->next;
        }else{
                old->next = curr->next;
        }
        xfree(curr->data);
        xfree(curr);
}

void *getnodedata(linkedlist **ptr, int nodenum){
        int i;
        linkedlist *temp = *ptr;
        //Should be > 1
        assert(nodenum != 0);
        for(i = 1; i < nodenum && temp != NULL; i++){
                // printf("%d\n",(int)temp);
                temp = temp->next;
        }
        if(temp == NULL){
                printf("%d\n",(int)temp);
                printf("Here: isNULL = %d nodenum = %d i = %d\n", ((*ptr==NULL)?1:0), nodenum, i);
                return NULL;
        }        
        else 
                return temp->data;

}

void clearlist(linkedlist **ptr){
        linkedlist *temp;
        while(*ptr != NULL){
                temp = *ptr;
                *ptr = (*ptr)->next;
                xfree(temp->data);
                xfree(temp);
        }
}
