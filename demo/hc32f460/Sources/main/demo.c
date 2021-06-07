/******************************************************************************
* KLite²âÊÔÎÄ¼ş
* ½¯Ïş¸Ú<kerndev@foxmail.com>
******************************************************************************/
#include "kernel.h"

static mutex_t  m_lock;
static uint32_t m_counter;

static void demo_thread(void *arg)
{
	while(1)
	{
		mutex_lock(m_lock);
		m_counter++;
		mutex_unlock(m_lock);
		sleep(100);
	}
}

void demo_init(void)
{
	m_lock = mutex_create();
	thread_create(demo_thread, 0, 0);
	thread_create(demo_thread, 0, 0);
}
