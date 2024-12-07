#include "int_queue.h"
#include <stdio.h>

void init_queue(queue* q) {
	if (q == NULL)
		return;
	q->start = 0;
	q->end = 0;
	q->size = 0;
}

int queue_add(queue* q, int element) {
	if (q->size == CAPACITY)
		return -1;
	
	q->array[q->end] = element;
	q->end = (q->end + 1) % CAPACITY;
	q->size++;
	return 0;
}

int queue_remove(queue* q) {
	if (q->size == 0)
		return -1;

	int ret = q->array[q->start];
	q->start = (q->start + 1) % CAPACITY;
	q->size--;
	return ret;
}
