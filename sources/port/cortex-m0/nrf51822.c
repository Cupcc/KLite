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
#include "kernel.h"
#include "nrf51.h"

static uint32_t m_sys_nesting;

void cpu_sys_enter_critical(void)
{
	__disable_irq();
	m_sys_nesting++;
}

void cpu_sys_leave_critical(void)
{
	m_sys_nesting--;
	if(m_sys_nesting == 0)
	{
		__enable_irq();
	}
}

void cpu_sys_init(void)
{
	cpu_sys_enter_critical();
	NRF_CLOCK->TASKS_LFCLKSTART = 1;
	NVIC_SetPriority(PendSV_IRQn, 255);
	NVIC_SetPriority(RTC1_IRQn, 255);
}

void cpu_sys_start(void)
{
	NRF_RTC1->POWER = 1;
	NRF_RTC1->PRESCALER = 31; // 32768 / (31 + 1) = 1024Hz
	NRF_RTC1->INTENSET = 0x01;
	NRF_RTC1->EVTEN = 0x01;
	NRF_RTC1->TASKS_START = 1;
	NVIC_EnableIRQ(RTC1_IRQn);
	cpu_sys_leave_critical();
}

void cpu_sys_sleep(uint32_t time)
{
	// Call wfi() can enter low power mode
	// But SysTick may be stopped after call wfi() on some device.
	//__wfi();
}

/* Please select a plan blow */

// Plan A: 1tick = 0.9765625ms
void RTC1_IRQHandler(void)
{
	NRF_RTC1->EVENTS_TICK = 0;
	kernel_tick(1); 
}

// Plan B: 43tick = 41.9921875ms
void RTC1_IRQHandler(void)
{
	static int fix43;
	NRF_RTC1->EVENTS_TICK = 0;
	if(++fix43 == 43)  
	{
		fix43 = 0;
		return;
	}
	kernel_tick(1);
}

