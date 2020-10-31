/*
* KLite���Թ���
* ������<kerndev@foxmail.com>
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

//��ʼ���߳�
void init(void *arg)
{
  	bsp_init();
	app_init();
}

//�����߳�
void idle(void *arg)
{
    kernel_idle();
}

//�������
int main(void)
{
	static uint8_t heap[16*1024];
    kernel_init((uint32_t)heap, sizeof(heap));
    thread_create(init, 0, 0);
    thread_create(idle, 0, 0);
    kernel_start();
}
