#include "llist.h"
#include <stdio.h>

void llist_init(llist* list) {
	if (list == NULL)
		return;

	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}

void llist_append(llist* list, node* new) {
	if (list == NULL || new == NULL)
		return;

	if (list->size == 0) {
		list->head = new;
		list->tail = new;
		list->size = 1;

		new->next = NULL;
		new->prev = NULL;
	}
	
	else
		llist_insert_after(list, list->tail, new);
}

void llist_insert_first(llist* list, node* new) {
	if (list == NULL || new == NULL)
		return;

	if (list->size == 0)
		llist_append(list, new);

	else
		llist_insert_before(list, list->head, new);
}

void llist_insert_after(llist* list, node* prev, node* new) {
	if (list == NULL || prev == NULL || new == NULL)
		return;

	if (prev == list->tail) {
		new->prev = list->tail;
		list->tail->next = new;
		list->tail = new;

		new->next = NULL;
	}
	
	else {
		new->prev = prev;
		new->next = prev->next;

		prev->next->prev = new;
		prev->next = new;
	}

	list->size++;

}

void llist_insert_before(llist* list, node* next, node* new) {
	if (list == NULL || next == NULL || new == NULL)
		return;
	
	if (next == list->head) {
		new->next = list->head;
		list->head->prev = new;
		list->head = new;

		new->prev = NULL;
	}

	else {
		new->next = next;
		new->prev = next->prev;

		next->prev->next = new;
		next->prev = new;
	}

	list->size++;
}

void llist_remove(llist* list, node* to_remove) {
	if (list == NULL || to_remove == NULL)
		return;

	if (list->size == 1) {
		list->head = NULL;
		list->tail = NULL;
	}

	else if (list->head == to_remove) {
		to_remove->next->prev = NULL;
		list->head = to_remove->next;
	}

	else if (list->tail == to_remove) {
		to_remove->prev->next = NULL;
		list->tail = to_remove->prev;
	}

	else {
		to_remove->prev->next = to_remove->next;
		to_remove->next->prev = to_remove->prev;
	}

	to_remove->prev = NULL;
	to_remove->next = NULL;
	list->size--;
}
