#include "nrf_sdh_rtos.h"
#include "nrf_sdh.h"
#include "kernel.h"

#define NRF_LOG_MODULE_NAME nrf_sdh_rtos
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

static event_t                  m_event;

void SWI2_IRQHandler(void)
{
	event_set(m_event);
}

/* This function gets events from the SoftDevice and processes them. */
static void softdevice_task(void * arg)
{
    NRF_LOG_INFO("Enter softdevice_task.");
    while (true)
    {
        nrf_sdh_evts_poll();
		event_wait(m_event);
    }
}

void nrf_sdh_rtos_init(void)
{
	thread_t thrd;
	m_event = event_create(true);
	thrd = thread_create(softdevice_task, NULL, 4096);
	thread_set_priority(thrd, THREAD_PRIORITY_HIGHEST);
	NRF_LOG_INFO("SoftDevice task created: %p", thrd);
}
