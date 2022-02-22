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
#include "event_group.h"

struct event_group
{
	mutex_t mutex;
	cond_t cond;
	uint32_t bits;
};

event_group_t event_group_create(void)
{
	struct event_group *event;
	event = heap_alloc(sizeof(struct event_group));
	if(event != NULL)
	{
		memset(event, 0, sizeof(struct event_group));
		event->mutex = mutex_create();
		event->cond = cond_create();
		if(event->cond == NULL)
		{
			return NULL;
		}
	}
	return event;
}

void event_group_delete(event_group_t event)
{
	heap_free(event);
}

void event_group_set(event_group_t event, uint32_t bits)
{
	mutex_lock(event->mutex);
	event->bits |= bits;
	mutex_unlock(event->mutex);
	cond_broadcast(event->cond);
}

void event_group_clear(event_group_t event, uint32_t bits)
{
	mutex_lock(event->mutex);
	event->bits &= ~bits;
	mutex_unlock(event->mutex);
}

static uint32_t wait_bits(event_group_t event, uint32_t bits, bool wait_all, bool auto_clear)
{
	uint32_t cmp;
	cmp = event->bits & bits;
	if((wait_all && (cmp == bits)) || ((!wait_all) && (cmp != 0)))
	{
		if(auto_clear)
		{
			event->bits &= ~bits;
		}
		return cmp;
	}
	return 0;
}

uint32_t event_group_wait(event_group_t event, uint32_t bits, bool wait_all, bool auto_clear)
{
	uint32_t ret;
	mutex_lock(event->mutex);
	ret = wait_bits(event, bits, wait_all, auto_clear);
	while(ret == 0)
	{
		cond_wait(event->cond, event->mutex);
		ret = wait_bits(event, bits, wait_all, auto_clear);
	}
	mutex_unlock(event->mutex);
	return ret;
}

uint32_t event_group_timed_wait(event_group_t event, uint32_t bits, bool wait_all, bool auto_clear, uint32_t timeout)
{
	uint32_t ret;
	mutex_lock(event->mutex);
	ret = wait_bits(event, bits, wait_all, auto_clear);
	while((ret == 0) && (timeout > 0))
	{
		timeout = cond_timed_wait(event->cond, event->mutex, timeout);
		ret = wait_bits(event, bits, wait_all, auto_clear);
	}
	mutex_unlock(event->mutex);
	return ret;
}
