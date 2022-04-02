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
#ifndef __INTERNAL_H
#define __INTERNAL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

struct tcb_list
{
	struct tcb_node *head;
	struct tcb_node *tail;
};

struct tcb_node
{
	struct tcb_node *prev;
	struct tcb_node *next;
	struct tcb      *tcb;
};

struct tcb
{
	void            *stack;
	void           (*entry)(void*);
	uint32_t         prio;
	uint32_t         time;
	uint32_t         timeout;
	struct tcb_list *list_sched;
	struct tcb_list *list_wait;
	struct tcb_node  node_sched;
	struct tcb_node  node_wait;
};

extern struct tcb *sched_tcb_now;
extern struct tcb *sched_tcb_next;

#define sched_lock()   cpu_sys_enter_critical()
#define sched_unlock() cpu_sys_leave_critical()

void sched_init(void);
void sched_idle(void);
void sched_timing(uint32_t time);
void sched_switch(void);
void sched_preempt(bool round_robin);
void sched_tcb_append(struct tcb *tcb);
void sched_tcb_remove(struct tcb *tcb);
void sched_tcb_sort(struct tcb *tcb);
void sched_tcb_ready(struct tcb *tcb);
void sched_tcb_sleep(struct tcb *tcb, uint32_t timeout);
void sched_tcb_wait(struct tcb *tcb, struct tcb_list *list);
void sched_tcb_timed_wait(struct tcb *tcb, struct tcb_list *list, uint32_t timeout);
struct tcb *sched_tcb_wake_from(struct tcb_list *list);

void  cpu_sys_init(void);
void  cpu_sys_start(void);
void  cpu_sys_sleep(uint32_t time);
void  cpu_sys_enter_critical(void);
void  cpu_sys_leave_critical(void);
void  cpu_contex_switch(void);
void *cpu_contex_init(void *stack, void *entry, void *arg, void *exit);

void heap_init(void *addr, uint32_t size);
void heap_overflow(void);

#endif
