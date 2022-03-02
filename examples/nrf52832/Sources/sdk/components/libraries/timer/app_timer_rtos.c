#include "sdk_common.h"
#include "kernel.h"
#include "soft_timer.h"
#if NRF_MODULE_ENABLED(APP_TIMER)

#include "app_timer.h"
#include <stdlib.h>
#include <string.h>
#include "nrf.h"
#include "nrf_log.h"
#include "app_error.h"

/**@brief This structure keeps information about osTimer.*/
typedef struct
{
	soft_timer_t                timer;
	void                      * argument;
	app_timer_timeout_handler_t func;
	bool                        active;
	bool                        single_shot;
}app_timer_info_t;

/* Check if app_timer_t variable type can held our app_timer_info_t structure */
STATIC_ASSERT(sizeof(app_timer_info_t) <= sizeof(app_timer_t));

static void app_timer_thread(void *arg)
{
	while(1)
	{
		soft_timer_service();
	}
}

static void app_timer_handler(void *arg)
{
	app_timer_info_t *info;
	info = (app_timer_info_t *)arg;
	if(info->active)
	{
		info->func(info->argument);
		if(info->single_shot)
		{
			info->active = false;
			soft_timer_stop(info->timer);
		}
	}
}

uint32_t app_timer_init(void)
{
	thread_create(app_timer_thread, NULL, 1024);
	return NRF_SUCCESS;
}

uint32_t app_timer_create(app_timer_id_t const * p_timer_id,app_timer_mode_t mode, app_timer_timeout_handler_t timeout_handler)
{
	app_timer_info_t *info;
	info = (app_timer_info_t *)*p_timer_id;
	info->single_shot = (mode == APP_TIMER_MODE_SINGLE_SHOT);
	info->active = false;
	info->func = timeout_handler;
	NRF_LOG_INFO("%s: id=%p", __func__, *p_timer_id);
	info->timer = soft_timer_create(app_timer_handler, info);
	return 0;
}

uint32_t app_timer_start(app_timer_id_t timer_id, uint32_t timeout_ticks, void * p_context)
{
	app_timer_info_t *info;
	info = (app_timer_info_t *)timer_id;
	info->argument = p_context;
	info->active = true;
	NRF_LOG_INFO("%s: id=%p", __func__, timer_id);
	soft_timer_start(info->timer, timeout_ticks);
	return NRF_SUCCESS;
}

uint32_t app_timer_stop(app_timer_id_t timer_id)
{
	app_timer_info_t *info;
	info = (app_timer_info_t *)timer_id;
	info->active = false;
	NRF_LOG_INFO("%s: id=%p", __func__, timer_id);
	soft_timer_stop(info->timer);
	return NRF_SUCCESS;
}

#endif
