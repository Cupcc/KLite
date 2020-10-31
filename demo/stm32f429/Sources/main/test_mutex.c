/*
* KLite测试工程
* 蒋晓岗<kerndev@foxmail.com>
*/
#include "kernel.h"
#include "log.h"

static mutex_t    m_mutex;

//演示线程
//测试线程自动退出
static void demo1_child_thread(void *arg)
{
    uint32_t time;
    time = kernel_time();
    LOG("demo2_child_thread: 0x%08X\r\n", thread_self());
	mutex_lock(m_mutex);
	LOG("demo2_child_thread: 0x%08X take mutex... expire=%dms.\r\n", thread_self(), kernel_time() - time);
	sleep(500);
	mutex_unlock(m_mutex);
    LOG("demo2_child_thread: 0x%08X exited, expire=%dms.\r\n", thread_self(), kernel_time() - time);
}

//演示线程
//测试线程创建与同步
static void demo1_thread(void *arg)
{
    uint32_t cnt;
    thread_t thrd;
    cnt = 0;
	LOG("demo2_thread: 0x%08X\r\n", thread_self());
    while(1)
    {
		//循环创建子线程，直到内存用完，创建失败
        thrd = thread_create(demo1_child_thread, 0, 0);
        if(thrd == NULL)
        {
            LOG("create child threads: %d\r\n", cnt);
            sleep(10000);
			cnt = 0;
            continue;
        }
        cnt++;
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
	m_mutex = mutex_create();
	thread_create(usage_thread, 0, 0);
	thread_create(demo1_thread, 0, 0);
}
