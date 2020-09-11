/******************************************************************************
* Copyright (c) 2015-2019 jiangxiaogang<kerndev@foxmail.com>
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
#include "list.h"

static struct tcb_list m_list_exit;

thread_t thread_self(void)
{
    return (thread_t)sched_tcb_now;
}

thread_t thread_create(void (*entry)(void*), void *arg, uint32_t stack_size)
{
    struct tcb *tcb;
    stack_size = stack_size ? stack_size : 1024;
    tcb = heap_alloc(sizeof(struct tcb) + stack_size);
    if(tcb != NULL)
    {
        tcb->stack = (uintptr_t)(tcb + 1);
        tcb->stack_size = stack_size;
        tcb->entry = entry;
        tcb->arg = arg;
        tcb->prio = THREAD_PRIORITY_NORMAL;
        tcb->node_wait.tcb = tcb;
        tcb->node_sched.tcb = tcb;
        sched_tcb_init(tcb);
        sched_lock();
        sched_tcb_ready(tcb);
        sched_unlock();
    }
    return (thread_t)tcb;
}

void thread_delete(thread_t thread)
{
    struct tcb *tcb;
    tcb = (struct tcb *)thread;
    sched_lock();
    sched_tcb_suspend(tcb);
    sched_unlock();
    heap_free(tcb);
}

void thread_suspend(void)
{
    sched_lock();
    sched_tcb_suspend(sched_tcb_now);
    sched_switch();
    sched_unlock();
}

void thread_resume(thread_t thread)
{
    sched_lock();
    sched_tcb_resume(thread);
    sched_preempt();
    sched_unlock();
}

void thread_sleep(uint32_t time)
{
    sched_lock();
    sched_tcb_sleep(sched_tcb_now, time);
    sched_switch();
    sched_unlock();
}

uint32_t thread_time(thread_t thread)
{
    struct tcb *tcb;
    tcb = (struct tcb *)thread;
    return tcb->time;
}

void thread_set_priority(thread_t thread, int prio)
{
    struct tcb *tcb;
    tcb = (struct tcb *)thread;
    sched_lock();
    tcb->prio = prio;
    sched_tcb_sort(tcb);
    sched_unlock();
}

int thread_get_priority(thread_t thread)
{
    struct tcb *tcb;
    tcb = (struct tcb *)thread;
    return tcb->prio;
}

void thread_exit(void)
{
    sched_lock();
    sched_tcb_suspend(sched_tcb_now);
	list_append(&m_list_exit, &sched_tcb_now->node_sched);
    sched_switch();
    sched_unlock();
}

void thread_free(void)
{
	struct tcb_node *node;
    while(m_list_exit.head)
    {
		node = m_list_exit.head;
        sched_lock();
		list_remove(&m_list_exit, node);
        sched_unlock();
        heap_free(node->tcb);
    }
}
