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
#include "soft_timer.h"

struct soft_timer
{
	struct soft_timer *prev;
	struct soft_timer *next;
	void (*handler)(void *);
	void  *arg;
	uint32_t reload;
	uint32_t timeout;
};

struct soft_timer_list
{
	struct soft_timer *head;
	struct soft_timer *tail;
	mutex_t mutex;
	event_t event;
};

static struct soft_timer_list *m_timer_list;

static bool soft_timer_init(void)
{
	if(m_timer_list != NULL)
	{
		return true;
	}
	m_timer_list = heap_alloc(sizeof(struct soft_timer_list));
	if(m_timer_list != NULL)
	{
		memset(m_timer_list, 0, sizeof(struct soft_timer_list));
		m_timer_list->mutex = mutex_create();
		m_timer_list->event = event_create(true);
		return m_timer_list->event != NULL;
	}
	return false;
}

static uint32_t soft_timer_process(uint32_t time)
{
	struct soft_timer *node;
	struct soft_timer *next;
	uint32_t timeout;
	timeout = UINT32_MAX;
	mutex_lock(m_timer_list->mutex);
	for(node = m_timer_list->head; node != NULL; node = next)
	{
		next = node->next;
		if(node->timeout > time)
		{
			node->timeout -= time;
		}
		else
		{
			node->handler(node->arg);
			node->timeout = node->reload;
		}
		if(node->timeout < timeout)
		{
			timeout = node->timeout;
		}
	}
	mutex_unlock(m_timer_list->mutex);
	return timeout;
}

bool soft_timer_service(void)
{
	uint32_t last;
	uint32_t time;
	uint32_t timeout;
	if(!soft_timer_init())
	{
		return false;
	}
	last = kernel_time();
	while(1)
	{
		time = kernel_time() - last;
		last = kernel_time();
		timeout = soft_timer_process(time);
		time = kernel_time() - last;
		if(timeout > time)
		{
			event_timed_wait(m_timer_list->event, timeout - time);
		}
	}
}

soft_timer_t soft_timer_create(void (*handler)(void *), void *arg)
{
	struct soft_timer *timer;
	if(!soft_timer_init())
	{
		return NULL;
	}
	timer = heap_alloc(sizeof(struct soft_timer));
	if(timer != NULL)
	{
		memset(timer, 0, sizeof(struct soft_timer));
		timer->handler = handler;
		timer->arg = arg;
	}
	return timer;
}

void soft_timer_delete(soft_timer_t timer)
{
	heap_free(timer);
}

void soft_timer_start(soft_timer_t timer, uint32_t timeout)
{
	mutex_lock(m_timer_list->mutex);
	if(timer->reload == 0)
	{
		timer->reload  = (timeout != 0) ? timeout : 1; /* timeout can't be 0 */
		timer->timeout = timer->reload;
		list_append(m_timer_list, timer);
	}
	mutex_unlock(m_timer_list->mutex);
	event_set(m_timer_list->event);
}

void soft_timer_stop(soft_timer_t timer)
{
	mutex_lock(m_timer_list->mutex);
	if(timer->reload != 0) /* check if timer started */
	{
		list_remove(m_timer_list, timer);
		timer->reload = 0;
		event_set(m_timer_list->event);
	}
	mutex_unlock(m_timer_list->mutex);
}
