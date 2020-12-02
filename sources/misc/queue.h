#ifndef __QUEUE_H
#define __QUEUE_H

typedef struct
{
	fifo_t fifo;
	sem_t sem;
	mutex_t mutex;
}queue_t;

bool queue_create(queue_t *queue, uint32_t size);
void queue_delete(queue_t *queue);
bool queue_send(queue_t *queue, void *data, int size);
int  queue_recv(queue_t *queue, void *data, int size, int timeout);

#endif
