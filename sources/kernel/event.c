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

struct event
{
	struct tcb_list list;
	bool auto_reset;
	bool state;
};

event_t event_create(bool auto_reset)
{
	struct event *event;
	event = heap_alloc(sizeof(struct event));
	if(event != NULL)
	{
		memset(event, 0, sizeof(struct event));
		event->auto_reset = auto_reset;
	}
	return (event_t)event;
}

void event_delete(event_t event)
{
	heap_free(event);
}

void event_set(event_t event)
{
	sched_lock();
	event->state = true;
	if(event->auto_reset)
	{
		if(sched_tcb_wake_from(&event->list))
		{
			event->state = false;
		}
	}
	else
	{
		while(sched_tcb_wake_from(&event->list));
	}
	sched_preempt(false);
	sched_unlock();
}

void event_reset(event_t event)
{
	sched_lock();
	event->state = false;
	sched_unlock();
}

void event_wait(event_t event)
{
	sched_lock();
	if(event->state)
	{
		if(event->auto_reset)
		{
			event->state = false;
		}
		sched_unlock();
		return;
	}
	sched_tcb_wait(sched_tcb_now, &event->list);
	sched_switch();
	sched_unlock();
}

uint32_t event_timed_wait(event_t event, uint32_t timeout)
{
	sched_lock();
	if(event->state)
	{
		if(event->auto_reset)
		{
			event->state = false;
		}
		sched_unlock();
		return true;
	}
	if(timeout == 0)
	{
		sched_unlock();
		return false;
	}
	sched_tcb_timed_wait(sched_tcb_now, &event->list, timeout);
	sched_switch();
	sched_unlock();
	return sched_tcb_now->timeout;
}
