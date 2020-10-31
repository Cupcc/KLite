/*
* KLite���Թ���
* ������<kerndev@foxmail.com>
*/
#include "kernel.h"
#include "log.h"

static sem_t    m_sem;


//��ʾ�߳�
//�����߳��Զ��˳�
static void demo1_child_thread(void *arg)
{
    uint32_t time;
	uint32_t data;
    time = kernel_time();
    LOG("demo2_child_thread: 0x%08X\r\n", thread_self());
    sem_wait(m_sem);
    LOG("demo2_child_thread: 0x%08X exited, data=%d, expire=%dms.\r\n", thread_self(), data, kernel_time() - time);
}

//��ʾ�߳�
//�����̴߳�����ͬ��
static void demo1_thread(void *arg)
{
    uint32_t cnt;
    thread_t thrd;
    cnt = 0;
	LOG("demo2_thread: 0x%08X\r\n", thread_self());
    while(1)
    {
		//ѭ���������̣߳�ֱ���ڴ����꣬����ʧ��
        thrd = thread_create(demo1_child_thread, 0, 0);
        if(thrd == NULL)
        {
            LOG("create child threads: %d\r\n", cnt);
            while(cnt)
            {
                sem_post(m_sem);    //�����¼���֪ͨ�ȴ��е��߳�
                cnt--;
            }
            LOG("all threads exit.\r\n");
            sleep(10000);
            continue;
        }
        cnt++;
    }
}

//������Դռ����
//���á�����ʱ��/��ʱ�䡱����CPUʹ����
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
	m_sem = sem_create(0, -1UL);
	thread_create(usage_thread, 0, 0);
	thread_create(demo1_thread, 0, 0);
}
