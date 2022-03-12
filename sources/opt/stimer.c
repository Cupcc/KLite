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
#include "stimer.h"

struct stimer
{
	struct stimer *prev;
	struct stimer *next;
	void (*handler)(void *);
	void  *arg;
	uint32_t reload;
	uint32_t timeout;
};

struct stimer_list
{
	struct stimer *head;
	struct stimer *tail;
	mutex_t mutex;
	event_t event;
};

static struct stimer_list *m_timer_list = NULL;

static bool stimer_init(void)
{
	if(m_timer_list != NULL)
	{
		return true;
	}
	m_timer_list = heap_alloc(sizeof(struct stimer_list));
	if(m_timer_list != NULL)
	{
		memset(m_timer_list, 0, sizeof(struct stimer_list));
		m_timer_list->mutex = mutex_create();
		m_timer_list->event = event_create(true);
		return m_timer_list->event != NULL;
	}
	return false;
}

static uint32_t stimer_process(uint32_t time)
{
	struct stimer *node;
	uint32_t timeout;
	timeout = UINT32_MAX;
	mutex_lock(m_timer_list->mutex);
	for(node = m_timer_list->head; node != NULL; node = node->next)
	{
		if(node->reload == 0)
		{
			continue;
		}
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

bool stimer_service(void)
{
	uint32_t last;
	uint32_t time;
	uint32_t timeout;
	if(!stimer_init())
	{
		return false;
	}
	last = kernel_tick_count();
	while(1)
	{
		time = kernel_tick_count() - last;
		last = kernel_tick_count();
		timeout = stimer_process(time);
		time = kernel_tick_count() - last;
		if(timeout > time)
		{
			event_timed_wait(m_timer_list->event, timeout - time);
		}
	}
}

stimer_t stimer_create(void (*handler)(void *), void *arg)
{
	struct stimer *timer;
	if(!stimer_init())
	{
		return NULL;
	}
	timer = heap_alloc(sizeof(struct stimer));
	if(timer != NULL)
	{
		memset(timer, 0, sizeof(struct stimer));
		timer->handler = handler;
		timer->arg = arg;
		mutex_lock(m_timer_list->mutex);
		list_append(m_timer_list, timer);
		mutex_unlock(m_timer_list->mutex);
	}
	return timer;
}

void stimer_delete(stimer_t timer)
{
	mutex_lock(m_timer_list->mutex);
	list_remove(m_timer_list, timer);
	mutex_unlock(m_timer_list->mutex);
	heap_free(timer);
}

void stimer_start(stimer_t timer, uint32_t timeout)
{
	mutex_lock(m_timer_list->mutex);
	timer->reload  = (timeout > 0) ? timeout : 1; /* timeout can't be 0 */
	timer->timeout = timer->reload;
	mutex_unlock(m_timer_list->mutex);
	event_set(m_timer_list->event);
}

void stimer_stop(stimer_t timer)
{
	mutex_lock(m_timer_list->mutex);
	timer->reload = 0;
	mutex_unlock(m_timer_list->mutex);
	event_set(m_timer_list->event);
}
