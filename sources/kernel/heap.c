/******************************************************************************
* Copyright (c) 2015-2023 jiangxiaogang<kerndev@foxmail.com>
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
#include "internal.h"
#include "kernel.h"

#define MEM_ALIGN_BYTE      (4)
#define MEM_ALIGN_MASK      (MEM_ALIGN_BYTE - 1)
#define MEM_ALIGN_PAD(m)    (((m) + MEM_ALIGN_MASK) & (~MEM_ALIGN_MASK))
#define MEM_ALIGN_CUT(m)    ((m) & (~MEM_ALIGN_MASK))

struct heap_node
{
	struct heap_node *prev;
	struct heap_node *next;
	uint32_t          used;
};

struct heap
{
	struct tcb_list   list;
	uint32_t          lock;
	uint32_t          size;
	struct heap_node *head;
	struct heap_node *free;
};

static struct heap *m_default_heap;

static struct heap_node *find_free_node(struct heap_node *node)
{
	uint32_t free;
	for(; node->next != NULL; node = node->next)
	{
		free = ((uint32_t)node->next) - ((uint32_t)node) - node->used;
		if(free > sizeof(struct heap_node))
		{
			break;
		}
	}
	return node;
}

static void heap_node_init(struct heap *heap, uint32_t start, uint32_t end)
{
	struct heap_node *node;
	node = (struct heap_node *)start;
	heap->head = node;
	heap->free = node;
	node->used = sizeof(struct heap_node);
	node->prev = NULL;
	node->next = (struct heap_node *)(end - sizeof(struct heap_node));
	node = node->next;
	node->used = sizeof(struct heap_node);
	node->prev = heap->head;
	node->next = NULL;
}

static void heap_mutex_lock(struct heap *heap)
{
	cpu_enter_critical();
	if(heap->lock == 0)
	{
		heap->lock = 1;
	}
	else
	{
		sched_tcb_wait(sched_tcb_now, (struct tcb_list *)heap);
		sched_switch();
	}
	cpu_leave_critical();
}

static void heap_mutex_unlock(struct heap *heap)
{
	cpu_enter_critical();
	if(sched_tcb_wake_from((struct tcb_list *)heap))
	{
		sched_preempt(false);
	}
	else
	{
		heap->lock = 0;
	}
	cpu_leave_critical();
}

__weak void heap_fault(heap_t heap)
{

}

heap_t heap_create(void *addr, uint32_t size)
{
	uint32_t start;
	uint32_t end;
	heap_t   heap = (heap_t)addr;
	start = MEM_ALIGN_PAD((uint32_t)(heap + 1));
	end   = MEM_ALIGN_CUT((uint32_t)addr + size);
	memset(heap, 0, sizeof(struct heap));
	heap->size = size;
	heap_node_init(heap, start, end);
	if(m_default_heap == NULL)
	{
		m_default_heap = heap;
	}
	return heap;
}

void *heap_alloc(heap_t heap, uint32_t size)
{
	uint32_t free;
	uint32_t need;
	struct heap_node *temp;
	struct heap_node *node;
	heap = (heap == NULL) ? m_default_heap : heap;
	need = MEM_ALIGN_PAD(size + sizeof(struct heap_node));
	heap_mutex_lock(heap);
	for(node = heap->free; node->next != NULL; node = node->next)
	{
		free = ((uint32_t)node->next) - ((uint32_t)node) - node->used;
		if(free >= need)
		{
			temp = (struct heap_node *)((uint32_t)node + node->used);
			temp->prev = node;
			temp->next = node->next;
			temp->used = need;
			node->next->prev = temp;
			node->next = temp;
			if(node == heap->free)
			{
				heap->free = find_free_node(node);
			}
			heap_mutex_unlock(heap);
			return (void *)(temp + 1);
		}
	}
	heap_mutex_unlock(heap);
	heap_fault(heap);
	return NULL;
}

void heap_free(heap_t heap, void *mem)
{
	struct heap_node *node;
	node = (struct heap_node *)mem - 1;
	heap = (heap == NULL) ? m_default_heap : heap;
	heap_mutex_lock(heap);
	if(node->prev->next == node)
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;
		if(node->prev < heap->free)
		{
			heap->free = node->prev;
		}
	}
	heap_mutex_unlock(heap);
}

void heap_usage(heap_t heap, uint32_t *used, uint32_t *free)
{
	uint32_t sum = 0;
	struct heap_node *node;
	heap = (heap == NULL) ? m_default_heap : heap;
	heap_mutex_lock(heap);
	for(node = heap->head; node->next != NULL; node = node->next)
	{
		sum += ((uint32_t)node->next) - ((uint32_t)node) - node->used;
	}
	heap_mutex_unlock(heap);
	if(used != NULL)
	{
		*used = heap->size - sum;
	}
	if(free != NULL)
	{
		*free = sum;
	}
}
