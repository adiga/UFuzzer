typedef struct node{
  void *data;
  struct node *next;
}linkedlist;

void addnode(linkedlist **ptr, void *data, unsigned int size);
void delnode(linkedlist **ptr, void *data);
void *getnodedata(linkedlist **ptr, int nodenum);
void clearlist(linkedlist **ptr);

