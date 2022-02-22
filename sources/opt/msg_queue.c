/******************************************************************************
* Copyright (c) 2015-2022 jiangxiaogang<kerndev@foxmail.com>
*
* This file is part of KLite distribution.
*
* KLite is free software, you can redistribute it and/or modify it under
* the MIT Licence.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
******************************************************************************/
#include "kernel.h"
#include "fifo.h"
#include "msg_queue.h"

struct msg_queue
{
	fifo_t fifo;
	mutex_t mutex;
	event_t empty;
	event_t full;
};

msg_queue_t msg_queue_create(uint32_t size)
{
	msg_queue_t queue;
	queue = heap_alloc(sizeof(struct msg_queue) + size);
	if(queue != NULL)
	{
		fifo_init(&queue->fifo, queue + 1, size);
		queue->mutex = mutex_create();
		queue->empty = event_create(true);
		queue->full = event_create(true);
		if(queue->full == NULL)
		{
			return NULL;
		}
	}
	return queue;
}

void msg_queue_delete(msg_queue_t queue)
{
	mutex_delete(queue->mutex);
	event_delete(queue->empty);
	event_delete(queue->full);
	heap_free(queue);
}

uint32_t msg_queue_send(msg_queue_t queue, void *msg, uint32_t msg_size, uint32_t timeout)
{
	uint32_t ret;
	uint32_t ttl;
	ttl = msg_size + sizeof(uint32_t);
	while(1)
	{
		mutex_lock(queue->mutex);
		ret = fifo_space(&queue->fifo);
		if(ret >= ttl)
		{
			fifo_write(&queue->fifo, &msg_size, sizeof(uint32_t));
			fifo_write(&queue->fifo, msg, msg_size);
			mutex_unlock(queue->mutex);
			event_set(queue->full);
			return msg_size;
		}
		if(timeout > 0)
		{
			mutex_unlock(queue->mutex);
			timeout = event_timed_wait(queue->empty, timeout);
			continue;
		}
		return 0;
	}
}

uint32_t msg_queue_recv(msg_queue_t queue, void *msg, uint32_t msg_size, uint32_t timeout)
{
	uint32_t ret;
	uint32_t ttl;
	uint32_t over;
	uint8_t  dummy;
	while(1)
	{
		mutex_lock(queue->mutex);
		ret = fifo_read(&queue->fifo, &ttl, sizeof(uint32_t));
		if(ret != 0)
		{
			fifo_read(&queue->fifo, msg, (msg_size < ttl) ? msg_size : ttl);
			if(msg_size < ttl)
			{
				over = ttl - msg_size;
				while(over--)
				{
					fifo_read(&queue->fifo, &dummy, 1);
				}
			}
			mutex_unlock(queue->mutex);
			event_set(queue->empty);
			return ttl;
		}
		if(timeout > 0)
		{
			mutex_unlock(queue->mutex);
			timeout = event_timed_wait(queue->full, timeout);
			continue;
		}
		return 0;
	}
}
