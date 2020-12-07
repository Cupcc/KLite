#include <stdint.h>
#include <string.h>
#include "nrf.h"
#include "nrf_log.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_nus.h"
#include "ble_dis.h"
#include "app_scheduler.h"
#include "app_timer.h"
#include "app_timer_appsh.h"
#include "softdevice_handler.h"

#include "kernel.h"
#include "queue.h"

#define SCHED_MAX_EVENT_DATA_SIZE       MAX(APP_TIMER_SCHED_EVT_SIZE, 64)           /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE                32                                          /**< Maximum number of events in the scheduler queue. */

#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         32                                          /**< Size of timer operation queues. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

ble_nus_t        		m_nus;                                      /**< Structure to identify the Nordic UART Service. */
uint16_t         		m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
queue_t                 m_queue;

//NUS服务数据
static void ble_nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
{
	NRF_LOG_RAW_INFO("[%d]recv:\r\n", kernel_time());
	NRF_LOG_RAW_HEXDUMP_INFO(p_data, length);
	queue_send(&m_queue, p_data, length);
}

//BLE事件处理
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_RAW_INFO("Device Connected.\r\n");
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_RAW_INFO("Device Disconnected.\r\n");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;
            req = p_ble_evt->evt.gatts_evt.params.authorize_request;
            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;
		}
#if (NRF_SD_BLE_API_VERSION == 3)
		case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
			err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle, GATT_MTU_SIZE_DEFAULT);
			APP_ERROR_CHECK(err_code);
			break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST
#endif       		
        default:
            // No implementation needed.
            break;
    }
}

//初始化BLE服务
static void services_init(void)
{
    uint32_t       err_code;
	ble_nus_init_t nus_init;
	ble_dis_init_t dis_init;
	
	//初始化设备信息服务
    memset(&dis_init, 0, sizeof(dis_init));
    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, "jiangxiaogang");
    ble_srv_ascii_to_utf8(&dis_init.model_num_str,  "KLite-DEMO");
    ble_srv_ascii_to_utf8(&dis_init.serial_num_str, "0");
    ble_srv_ascii_to_utf8(&dis_init.hw_rev_str, "nRF52832");
    ble_srv_ascii_to_utf8(&dis_init.fw_rev_str, "1.0.0");
    ble_srv_ascii_to_utf8(&dis_init.sw_rev_str, "4.2.0");
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);
    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
	
	//初始化串口服务
    memset(&nus_init, 0, sizeof(nus_init));
	nus_init.data_handler = ble_nus_data_handler;
    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}

//初始化连接参数
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));
    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = true;
    cp_init.evt_handler                    = NULL;
    cp_init.error_handler                  = NULL;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

//Function for the GAP initialization.
//This function will set up all the necessary GAP (Generic Access Profile) 
//parameters of the device. It also sets the permissions and appearance.
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;
	uint8_t                	ble_name_buf[32];
	int 					ble_name_len;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
	
	//蓝牙名称的初始化
	ble_name_len = sprintf((char*)ble_name_buf, "KLite");
	err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)ble_name_buf, ble_name_len);
	APP_ERROR_CHECK(err_code);
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

//初始化广播数据
static void advertising_init(void)
{
	uint32_t                 err_code;
	ble_advdata_t            advdata;
	ble_advdata_t            scandata;
	ble_adv_modes_config_t   options;
	ble_advdata_manuf_data_t manuf_data;
	uint8_t                  manuf_data_payload[8];
	
	//厂家私有数据,长度限制8字节
	memset(&manuf_data_payload, 0, sizeof(manuf_data_payload));
	memset(&manuf_data, 0, sizeof(manuf_data));
	manuf_data_payload[0]         = 0x00;
	manuf_data_payload[1]         = 0x01;
	manuf_data_payload[2]         = 0x02;
	manuf_data_payload[3]         = 0x03;
	manuf_data.company_identifier = 0x584C;
	manuf_data.data.p_data        = manuf_data_payload;
	manuf_data.data.size          = 4;

	memset(&advdata, 0, sizeof(advdata));
	advdata.flags                 = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;//广播不超时
	advdata.name_type             = BLE_ADVDATA_FULL_NAME;

	static ble_uuid_t adv_uuids[] ={{BLE_UUID_NUS_SERVICE,BLE_UUID_TYPE_VENDOR_BEGIN}};
	memset(&scandata, 0, sizeof(scandata));
	scandata.p_manuf_specific_data  = &manuf_data;
	scandata.uuids_complete.uuid_cnt= sizeof(adv_uuids) / sizeof(adv_uuids[0]);
	scandata.uuids_complete.p_uuids = adv_uuids;

	memset(&options, 0, sizeof(options));
	options.ble_adv_fast_enabled  = true;
	options.ble_adv_fast_interval = 400;    //广播间隔*0.625ms=250ms
	options.ble_adv_fast_timeout  = 0;      //广播超时(秒),广播不超时
	err_code = ble_advertising_init(&advdata, &scandata, &options, NULL, NULL);
	APP_ERROR_CHECK(err_code);
}

//处理来自协议栈的事件,分发到所有BLE模块
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    ble_nus_on_ble_evt(&m_nus, p_ble_evt);
    on_ble_evt(p_ble_evt);
}

//处理系统事件
static void sys_evt_dispatch(uint32_t sys_evt)
{
    ble_advertising_on_sys_evt(sys_evt);
}

static void ble_stack_init(void)
{
    uint32_t err_code;
    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(0, 1, &ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    // Enable BLE stack.
    ble_enable_params.common_enable_params.vs_uuid_count = 1; //默认是1，否则dfu和nus不能共存
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    // Subscribe for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for System events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

static void ble_data_xfer(void *p)
{
	int ret;
	uint8_t data[20];
	while(1)
	{
		ret = queue_recv(&m_queue, &data, 20, 1000);
		NRF_LOG_RAW_INFO("[%d]queue_recv ret=%d\r\n", kernel_time(), ret);
		if(ret > 0)
		{
			ble_nus_string_send(&m_nus, data, ret);
		}
	}
}

//BLE初始化
void app_init(void)
{
	queue_create(&m_queue, 256);
	APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);
	APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
	ble_stack_init();
    gap_params_init();
    services_init();
    advertising_init();
    conn_params_init();
    ble_advertising_start(BLE_ADV_MODE_FAST);
	thread_create(ble_data_xfer, 0, 1024);
}
