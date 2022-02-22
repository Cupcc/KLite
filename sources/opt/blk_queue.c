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
#include <string.h>
#include "kernel.h"
#include "list.h"
#include "mem_pool.h"
#include "blk_queue.h"

struct queue_node
{
	struct queue_node *prev;
	struct queue_node *next;
	uint8_t *data;
};

struct blk_queue
{
	struct queue_node *head;
	struct queue_node *tail;
	sem_t sem;
	mutex_t mutex;
	mem_pool_t pool;
	uint32_t item_size;
};

blk_queue_t blk_queue_create(uint32_t item_size, uint32_t queue_depth)
{
	blk_queue_t queue;
	queue = heap_alloc(sizeof(struct blk_queue));
	if(queue != NULL)
	{
		memset(queue, 0, sizeof(struct blk_queue));
		queue->item_size = item_size;
		queue->sem = sem_create(0);
		queue->mutex = mutex_create();
		queue->pool = mem_pool_create(sizeof(struct queue_node) + item_size, queue_depth);
	}
	return queue;
}

void blk_queue_delete(blk_queue_t queue)
{
	mem_pool_delete(queue->pool);
	sem_delete(queue->sem);
	mutex_delete(queue->mutex);
	heap_free(queue);
}

bool blk_queue_send(blk_queue_t queue, void *item, uint32_t timeout)
{
	struct queue_node *node;
	node = mem_pool_alloc(queue->pool);
	if(node != NULL)
	{
		memcpy(node->data, item, queue->item_size);
		mutex_lock(queue->mutex);
		list_append(queue, node);
		mutex_unlock(queue->mutex);
		sem_post(queue->sem);
		return true;
	}
	return false;
}

bool blk_queue_recv(blk_queue_t queue, void *item, uint32_t timeout)
{
	struct queue_node *node;
	if(sem_timed_wait(queue->sem, timeout))
	{
		mutex_lock(queue->mutex);
		node = queue->head;
		memcpy(item, node->data, queue->item_size);
		list_remove(queue, node);
		mutex_unlock(queue->mutex);
		mem_pool_free(queue->pool, node);
		return true;
	}
	return false;
}
