#include "kernel.h"
#include "bsp.h"

static void demo_thread1(void *arg)
{
	while(1)
	{
		bsp_led_on(0);
		thread_sleep(500);
		bsp_led_off(0);
		thread_sleep(500);
	}
}

static void demo_thread2(void *arg)
{
	while(1)
	{
		bsp_led_on(1);
		thread_sleep(200);
		bsp_led_off(1);
		thread_sleep(200);
	}
}

void app_init(void)
{
	thread_create(demo_thread1, NULL, 512);
	thread_create(demo_thread2, NULL, 512);
}
