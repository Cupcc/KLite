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
	uint32_t used;
};

struct heap_contex
{
	struct tcb_list   list;
	uint32_t          lock;
	uint32_t          size;
	struct heap_node *head;
	struct heap_node *free;
};

static struct heap_contex m_heap;

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

static void heap_node_init(struct heap_contex *heap, uint32_t start, uint32_t end)
{
	struct heap_node *node;
	node = (struct heap_node *)start;
	heap->size = end - start;
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

static void heap_mutex_lock(struct heap_contex *heap)
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

static void heap_mutex_unlock(struct heap_contex *heap)
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

__weak void heap_fault(void)
{

}

void heap_init(void *addr, uint32_t size)
{
	uint32_t start;
	uint32_t end;
	start = MEM_ALIGN_PAD((uint32_t)addr);
	end = MEM_ALIGN_CUT((uint32_t)addr + size);
	memset(&m_heap, 0, sizeof(struct heap_contex));
	heap_node_init(&m_heap, start, end);
}

void *heap_alloc(uint32_t size)
{
	uint32_t free;
	uint32_t need;
	struct heap_node *temp;
	struct heap_node *node;
	need = MEM_ALIGN_PAD(size + sizeof(struct heap_node));
	heap_mutex_lock(&m_heap);
	for(node = m_heap.free; node->next != NULL; node = node->next)
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
			if(node == m_heap.free)
			{
				m_heap.free = find_free_node(node);
			}
			heap_mutex_unlock(&m_heap);
			return (void *)(temp + 1);
		}
	}
	heap_mutex_unlock(&m_heap);
	heap_fault();
	return NULL;
}

void heap_free(void *mem)
{
	struct heap_node *node;
	node = (struct heap_node *)mem - 1;
	heap_mutex_lock(&m_heap);
	node->prev->next = node->next;
	node->next->prev = node->prev;
	if(node->prev < m_heap.free)
	{
		m_heap.free = node->prev;
	}
	heap_mutex_unlock(&m_heap);
}

void heap_usage(uint32_t *used, uint32_t *free)
{
	uint32_t sum;
	struct heap_node *node;
	sum = 0;
	heap_mutex_lock(&m_heap);
	for(node = m_heap.head; node != NULL; node = node->next)
	{
		sum += node->used;
	}
	heap_mutex_unlock(&m_heap);
	if(used != NULL)
	{
		*used = sum;
	}
	if(free != NULL)
	{
		*free = m_heap.size - sum;
	}
}
