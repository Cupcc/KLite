/*
* 描述: 数据队列
* 作者: 蒋晓岗<kerndev@foxmail.com>
*/
#include "kernel.h"
#include "fifo.h"
#include "queue.h"

bool queue_create(queue_t *queue, uint32_t size)
{
	queue->fifo = fifo_create(size);
	queue->sem = sem_create(0, 1);
	queue->mutex = mutex_create();
	if(queue->mutex != NULL)
	{
		return true;
	}
	return false;
}

void queue_delete(queue_t *queue)
{
	mutex_delete(queue->mutex);
	sem_delete(queue->sem);
	fifo_delete(queue->fifo);
}

bool queue_send(queue_t *queue, void *data, int size)
{
	uint32_t ret;
	ret = fifo_write(queue->fifo, data, size);
	sem_post(queue->sem);
	return ret == size;
}

int queue_recv(queue_t *queue, void *data, int size, int timeout)
{
	int ret;
	ret = 0;
	while(ret < size)
	{
		ret += fifo_read(queue->fifo, data, size);
		if(!sem_timed_wait(queue->sem, timeout))
		{
			break;
		}
	}
	return ret;
}
