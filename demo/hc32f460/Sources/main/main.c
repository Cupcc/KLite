/******************************************************************************
* KLite���Թ���
* ������<kerndev@foxmail.com>
******************************************************************************/
#include "kernel.h"
#include "demo.h"

//��ʼ���߳�
void init(void *arg)
{
	demo_init();
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
