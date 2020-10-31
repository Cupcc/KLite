/*
* KLite测试工程
* 蒋晓岗<kerndev@foxmail.com>
*/
#include "kernel.h"
#include "log.h"
#include "gpio.h"

//演示线程
//测试线程休眠时间
static void blink_thread(void *arg)
{
    LOG("blink_thread: 0x%08X\r\n", thread_self());
	gpio_open(PC, 10, GPIO_MODE_OUT, GPIO_OUT_PP);
	while(1)
	{
		gpio_write(PC, 10, 1);
        sleep(10);
		gpio_write(PC, 10, 0);
        sleep(10);
	}
}


//计算资源占用率
//利用“空闲时间/总时间”计算CPU使用率
static void usage_thread(void *arg)
{
	uint32_t ver;
	uint32_t tick;
	uint32_t idle;
	uint32_t used;
	uint32_t total;

    thread_set_priority(thread_self(), THREAD_PRIORITY_HIGH);
	ver = kernel_version();
	heap_usage(&total, &used);
	LOG("\r\n");
	LOG("KLite V%d.%d.%d\r\n", (ver>>24)&0xFF, (ver>>16)&0xFF, ver&0xFFFF);	
	while(1)
	{
		tick = kernel_time();
		idle = kernel_idle_time();
		sleep(1000);
		tick = kernel_time() - tick;
		idle = kernel_idle_time() - idle;
		heap_usage(&total, &used);
		LOG("CPU:%2d%%, RAM:%d/%dB\r\n", 100*(tick-idle)/tick, used, total);
	}
}

void app_init(void)
{
	thread_create(usage_thread, 0, 0);
	thread_create(blink_thread, 0, 0);
}
