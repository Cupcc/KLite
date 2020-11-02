/******************************************************************************
* Copyright (c) 2015-2020 jiangxiaogang<kerndev@foxmail.com>
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
#include "sched.h"

struct semaphore
{
	struct tcb_node *head;
	struct tcb_node *tail;
	uint32_t cur_value;
	uint32_t max_value;
};

sem_t sem_create(uint32_t init_value, uint32_t max_value)
{
	struct semaphore *p_sem;
	p_sem = heap_alloc(sizeof(struct semaphore));
	if(p_sem != NULL)
	{
		p_sem->head = NULL;
		p_sem->tail = NULL;
		p_sem->cur_value = init_value;
		p_sem->max_value = max_value;
	}
	return (sem_t)p_sem;
}

void sem_delete(sem_t sem)
{
	heap_free(sem);
}

void sem_reset(sem_t sem)
{
	struct semaphore *p_sem;
	p_sem = (struct semaphore *)sem;
	sched_lock();
	p_sem->cur_value = 0;
	sched_unlock();
}

bool sem_post(sem_t sem)
{
	struct semaphore *p_sem;
	p_sem = (struct semaphore *)sem;
	sched_lock();
	if(sched_tcb_wake_one((struct tcb_list *)p_sem))
	{
		sched_preempt();
		sched_unlock();
		return true;
	}
	if(p_sem->cur_value < p_sem->max_value)
	{
		p_sem->cur_value++;
		sched_unlock();
		return true;
	}
	sched_unlock();
	return false;
}

void sem_wait(sem_t sem)
{
	struct semaphore *p_sem;
	p_sem = (struct semaphore *)sem;
	sched_lock();
	if(p_sem->cur_value > 0)
	{
		p_sem->cur_value--;
		sched_unlock();
		return;
	}
	sched_tcb_wait(sched_tcb_now, (struct tcb_list *)p_sem);
	sched_switch();
	sched_unlock();
}

bool sem_timed_wait(sem_t sem, uint32_t timeout)
{
	struct semaphore *p_sem;
	p_sem = (struct semaphore *)sem;
	sched_lock();
	if(p_sem->cur_value > 0)
	{
		p_sem->cur_value--;
		sched_unlock();
		return true;
	}
	if(timeout == 0)
	{
		sched_unlock();
		return false;
	}
	sched_tcb_timed_wait(sched_tcb_now, (struct tcb_list *)p_sem, timeout);
	sched_switch();
	sched_unlock();
	return (sched_tcb_now->timeout != 0);
}

void sem_get_value(sem_t sem, uint32_t *value)
{
	struct semaphore *p_sem;
	p_sem = (struct semaphore *)sem;
	sched_lock();
	*value = p_sem->cur_value;
	sched_unlock();
}
