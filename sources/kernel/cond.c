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

struct cond
{
	struct tcb_node *head;
	struct tcb_node *tail;
};

cond_t cond_create(void)
{
	struct cond *p_cond;
	p_cond = heap_alloc(sizeof(struct cond));
	if(p_cond != NULL)
	{
		p_cond->head = NULL;
		p_cond->tail = NULL;
	}
	return (cond_t)p_cond;
}

void cond_delete(cond_t cond)
{
	heap_free(cond);
}

void cond_wait(cond_t cond)
{
	struct cond *p_cond;
	p_cond = (struct cond *)cond;
	sched_lock();
	sched_tcb_wait(sched_tcb_now, (struct tcb_list *)p_cond);
	sched_switch();
	sched_unlock();
}

bool cond_timed_wait(cond_t cond, uint32_t timeout)
{
	struct cond *p_cond;
	p_cond = (struct cond *)cond;
	sched_lock();
	if(timeout == 0)
	{
		sched_unlock();
		return false;
	}
	sched_tcb_timed_wait(sched_tcb_now, (struct tcb_list *)p_cond, timeout);
	sched_switch();
	sched_unlock();
	return (sched_tcb_now->timeout != 0);
}

bool cond_signal(cond_t cond)
{
	struct cond *p_cond;
	p_cond = (struct cond *)cond;
	sched_lock();
	if(sched_tcb_wake_one((struct tcb_list *)p_cond))
	{
		sched_preempt();
		sched_unlock();
		return true;
	}
	sched_unlock();
	return false;
}

bool cond_broadcast(cond_t cond)
{
	struct cond *p_cond;
	p_cond = (struct cond *)cond;
	sched_lock();
	if(sched_tcb_wake_one((struct tcb_list *)p_cond))
	{
		while(sched_tcb_wake_one((struct tcb_list *)p_cond));
		sched_preempt();
		sched_unlock();
		return true;
	}
	sched_unlock();
	return false;
}
