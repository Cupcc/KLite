/******************************************************************************
* Copyright (c) 2015-2017 jiangxiaogang<kerndev@foxmail.com>
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
#include "internal.h"

ksem_t ksem_create(uint32_t value)
{
	return (ksem_t)kobject_create(value);
}

void ksem_destroy(ksem_t sem)
{
	kobject_delete((struct object*)sem);
}

void ksem_wait(ksem_t sem)
{
	struct object* obj;
	obj = (struct object*)sem;
	
	ksched_lock();
	if(obj->data != 0)
	{
		obj->data--;
		ksched_unlock();
		return;
	}
	kobject_wait(obj, sched_tcb_now);
	ksched_unlock();
	ksched_execute();
}

int ksem_timedwait(ksem_t sem, uint32_t timeout)
{
	struct object* obj;
	obj = (struct object*)sem;
	
	ksched_lock();
	if(obj->data != 0)
	{
		obj->data--;
		ksched_unlock();
		return 1;
	}
	if(timeout == 0)
	{
		ksched_unlock();
		return 0;
	}
	kobject_timedwait(obj, sched_tcb_now, timeout);
	ksched_unlock();
	ksched_execute();
	return (sched_tcb_now->timeout != 0);
}

void ksem_post(ksem_t sem)
{
	struct object* obj;
	obj = (struct object*)sem;
	
	ksched_lock();
	if(obj->head == NULL)
	{
		obj->data++;
		ksched_unlock();
		return;
	}
	kobject_post(obj, obj->head->tcb);
	ksched_unlock();
}

uint32_t ksem_getvalue(ksem_t sem)
{
	struct object* obj;
	obj = (struct object*)sem;
	return obj->data;
}
