#ifndef LLIST_H
#define LLIST_H

typedef struct node node;

struct node {
	void* value;
	node* next;
	node* prev;
};

typedef struct {
	node* head;
	node* tail;
	int size;
} llist;

extern void llist_init(llist* list); 

extern void llist_append(llist* list, node* node);

extern void llist_insert_first(llist* list, node* node);

extern void llist_insert_after(llist* list, node* prev, node* node);

extern void llist_insert_before(llist* list, node* next, node* node);

extern void llist_remove(llist* list, node* to_remove);

#endif
