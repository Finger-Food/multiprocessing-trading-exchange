#ifndef INT_QUEUE_H
#define INT_QUEUE_H

#define CAPACITY (40)

#define LIGHT_WEIGHT

typedef struct {
	int array[CAPACITY];
	int start;
	int end;
	int size;
} queue;

extern void init_queue(queue* q);

extern int queue_add(queue* q, int element);

extern int queue_remove(queue* q);

#endif
