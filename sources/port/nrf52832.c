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
#include "nrf_rtc.h"
#include "nrf_nvic.h"
#include "nrf_log.h"
#include "softdevice_handler.h"

#define SYS_RTC        NRF_RTC2
#define SYS_RTC_IRQ    RTC2_IRQn
#define SYS_RTC_ISR    RTC2_IRQHandler

void cpu_sys_init(void)
{
	nrf_rtc_prescaler_set(SYS_RTC, 31); //32768Hz/(31+1)=1024Hz
	nrf_rtc_task_trigger (SYS_RTC, NRF_RTC_TASK_CLEAR);
    nrf_rtc_task_trigger (SYS_RTC, NRF_RTC_TASK_START);
	nrf_rtc_int_enable   (SYS_RTC, RTC_INTENSET_TICK_Msk);
	
	NVIC_SetPriority(PendSV_IRQn, 255);
    NVIC_SetPriority(SYS_RTC_IRQ, 254);
	NVIC_ClearPendingIRQ(SYS_RTC_IRQ);
    NVIC_EnableIRQ(SYS_RTC_IRQ);
}

void cpu_sys_idle(uint32_t time)
{
	uint32_t tmp;
	uint32_t cnt1;
	uint32_t cnt2;
	uint8_t  pcr;
		
	if(time < 10)
	{
		sd_app_evt_wait();
	}
	else
	{
		sd_nvic_critical_region_enter(&pcr);
		nrf_rtc_int_disable(SYS_RTC, RTC_INTENSET_TICK_Msk);
		
		cnt1 = SYS_RTC->COUNTER;
        //NRF_LOG_RAW_INFO("idle=%d(%d)\r\n", cnt1, time);
		
		tmp = (cnt1 + time) & 0x00FFFFFF;
		nrf_rtc_cc_set(SYS_RTC, 0, tmp);
		nrf_rtc_int_enable(SYS_RTC, NRF_RTC_INT_COMPARE0_MASK);
		
		sd_app_evt_wait();
		
		cnt2 = SYS_RTC->COUNTER;
		//NRF_LOG_RAW_INFO("wake=%d(%d)\r\n", time, cnt2 - cnt1);
		nrf_rtc_int_disable(SYS_RTC, NRF_RTC_INT_COMPARE0_MASK);
		nrf_rtc_int_enable(SYS_RTC, RTC_INTENSET_TICK_Msk);
		if(cnt2 != cnt1)
		{
			NVIC_SetPendingIRQ(SYS_RTC_IRQ);
		}
		sd_nvic_critical_region_exit(pcr);
	}
}

void SYS_RTC_ISR(void)
{
	static uint32_t cnt0;
	uint32_t cnt1;
	nrf_rtc_event_clear(SYS_RTC, NRF_RTC_EVENT_TICK);
	nrf_rtc_event_clear(SYS_RTC, NRF_RTC_EVENT_COMPARE_0);
	cnt1 = SYS_RTC->COUNTER;
    kernel_tick(cnt1 - cnt0);
	cnt0 = cnt1;
}
