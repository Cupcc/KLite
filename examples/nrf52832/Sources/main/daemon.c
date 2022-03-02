#include "kernel.h"
#include "nrf52.h"
#include "nrf_log.h"

static void daemon_thread_entry(void *arg)
{
	NRF_P0->DIRSET = 1<<22;
	while(1)
	{
		NRF_P0->OUTSET = 1<<22;
		sleep(500);
		NRF_P0->OUTCLR = 1<<22;
		sleep(500);
		NRF_LOG_INFO("alive %d ms", kernel_time());
	}
}

void daemon_init(void)
{
	thread_t thrd;
	thrd = thread_create(daemon_thread_entry, NULL, 1024);
	thread_set_priority(thrd, THREAD_PRIORITY_LOWEST);
}
