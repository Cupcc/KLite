/*
* KLite测试工程
* 蒋晓岗<kerndev@foxmail.com>
*/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "kernel.h"
#include "gpio.h"
#include "log.h"
#include "app.h"

void bsp_init(void)
{
	gpio_init(PA);
	gpio_init(PB);
	gpio_init(PC);
	log_init();
}

//初始化线程
void init(void *arg)
{
  	bsp_init();
	app_init();
}

//空闲线程
void idle(void *arg)
{
    kernel_idle();
}

//程序入口
int main(void)
{
	static uint8_t heap[16*1024];
    kernel_init((uint32_t)heap, sizeof(heap));
    thread_create(init, 0, 0);
    thread_create(idle, 0, 0);
    kernel_start();
}
