#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "softdevice_handler.h"
#include "sdk.h"

static sem_t m_ble_sem;

//处理BLE事件
static uint32_t sdk_event_handler(void)
{
	//NRF_LOG_RAW_INFO("sd sem post...\r\n");
	sem_post(m_ble_sem);
    return NRF_SUCCESS;
}

//协议栈线程
void sdk_loop(void)
{
	thread_set_priority(thread_self(), THREAD_PRIORITY_HIGHEST);
    while(1)
    {
		NRF_LOG_RAW_INFO("sd sem wait...\r\n");
		sem_wait(m_ble_sem);
		NRF_LOG_RAW_INFO("sd evt exec...\r\n");
        intern_softdevice_events_execute();
    }
}

//初始化协议栈
void sdk_init(void)
{
	nrf_clock_lf_cfg_t m_clock_lf_cfg = 
	{
		.source        = NRF_CLOCK_LF_SRC_XTAL,
		.rc_ctiv       = 0,
		.rc_temp_ctiv  = 0,
		.xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM
	};
	m_ble_sem = sem_create(0, 1);
	NRF_LOG_INIT(NULL);
    SOFTDEVICE_HANDLER_INIT(&m_clock_lf_cfg, sdk_event_handler);
	NRF_LOG_RAW_INFO("sd init ok\r\n");
}
