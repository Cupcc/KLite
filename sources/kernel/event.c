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

struct event
{
	struct tcb_node *head;
	struct tcb_node *tail;
	uint32_t value;
	bool valid;
};

event_t event_create(void)
{
	struct event *p_event;
	p_event = heap_alloc(sizeof(struct event));
	if(p_event != NULL)
	{
		p_event->head = NULL;
		p_event->tail = NULL;
		p_event->value = 0;
		p_event->valid = false;
	}
	return (event_t)p_event;
}

void event_delete(event_t event)
{
	heap_free(event);
}

void event_reset(event_t event)
{
	struct event *p_event;
	p_event = (struct event *)event;
	sched_lock();
	p_event->value = 0;
	p_event->valid = false;
	sched_unlock();
}

void event_wait(event_t event, uint32_t *value)
{
	struct event *p_event;
	p_event = (struct event *)event;
	sched_lock();
	if(p_event->valid)
	{
		*value = p_event->value;
		p_event->valid = false;
		sched_unlock();
		return;
	}
	sched_tcb_wait(sched_tcb_now, (struct tcb_list *)p_event);
	sched_switch();
	sched_unlock();
	*value = p_event->value;
}

bool event_timed_wait(event_t event, uint32_t *value, uint32_t timeout)
{
	struct event *p_event;
	p_event = (struct event *)event;
	sched_lock();
	if(p_event->valid)
	{
		*value = p_event->value;
		p_event->valid = false;
		sched_unlock();
		return true;
	}
	if(timeout == 0)
	{
		sched_unlock();
		return false;
	}
	sched_tcb_timed_wait(sched_tcb_now, (struct tcb_list *)p_event, timeout);
	sched_switch();
	sched_unlock();
	
	if(sched_tcb_now->timeout == 0)
	{
		return false;
	}
	*value = p_event->value;
	return true;
}

void event_post(event_t event, uint32_t value)
{
	struct event *p_event;
	p_event = (struct event *)event;
	sched_lock();
	if(sched_tcb_wake_one((struct tcb_list *)p_event))
	{
		p_event->value = value;
		sched_preempt();
	}
	else
	{
		p_event->valid = true;
		p_event->value = value;
	}
	sched_unlock();
}
