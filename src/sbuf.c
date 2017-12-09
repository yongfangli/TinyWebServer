#include "csapp.h"
#include "sbuf.h"

//Create an empty, bounded, shared FIFO buffer with n slots
void sbuf_init(sbuf_t *sp, int n)
{
	sp->buf = Calloc(n, sizeof(int));
	sp->n = n;
	sp->front = sp->rear = 0;
	sp->mutex = sem_open("mutex", O_CREAT, 0644, 1);
	sp->slots = sem_open("slots", O_CREAT, 0644, n);
	sp->items = sem_open("items", O_CREAT, 0644, 0);
}

//Clean up buffer sp
void sbuf_deinit(sbuf_t *sp)
{
	Free(sp->buf);
}

//Insert item onto the rear of shared buffer sp
void sbuf_insert(sbuf_t *sp, int item)
{
	P(sp->slots);
	P(sp->mutex);
	sp->buf[(++sp->rear)%(sp->n)] = item;
	V(sp->mutex);
	V(sp->items);
}

//Remove and return the first item from buffer sp
int sbuf_remove(sbuf_t *sp)
{
	int item;
	P(sp->items);
	P(sp->mutex);
	item = sp->buf[(++sp->front)%(sp->n)];
	V(sp->mutex);
	V(sp->slots);
	return item;
}

