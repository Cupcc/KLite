#include "kernel.h"
#include "nrf52.h"
#include "nrf_log.h"

static void daemon_thread_entry(void *arg)
{
	NRF_P0->DIRSET = 1<<22;
	while(1)
	{
		NRF_P0->OUTSET = 1<<22;
		sleep(250);
		NRF_P0->OUTCLR = 1<<22;
		sleep(250);
		for(int i=0; i<1000000; i++); /* eat about 18% CPU */
	}
}

static void usage_thread_entry(void *arg)
{
	uint32_t tick;
	uint32_t idle;
	uint32_t used;
	uint32_t free;
	while(1)
	{
		tick = kernel_tick_count();
		idle = kernel_tick_idle();
		sleep(5000);
		tick = kernel_tick_count() - tick;
		idle = kernel_tick_idle() - idle;
		heap_usage(&used, &free);
		NRF_LOG_INFO("CPU: %d%%, HEAP: used=%d, free=%d", 100 * (tick - idle) / tick, used, free);
	}
}

void daemon_init(void)
{
	thread_t thrd;
	thrd = thread_create(daemon_thread_entry, NULL, 1024);
	thread_set_priority(thrd, THREAD_PRIORITY_LOWEST);
	thrd = thread_create(usage_thread_entry, NULL, 1024);
	thread_set_priority(thrd, THREAD_PRIORITY_HIGH);
}
