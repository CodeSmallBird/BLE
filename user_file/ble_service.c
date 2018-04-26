// Board/nrf6310/ble/ble_app_hrs/main.c
/** @example Board/nrf6310/ble/ble_app_hrs/main.c
 *
 * @brief Heart Rate Service Sample Application main file.
 *
 * This file contains the source code for a sample application using the Heart Rate service
 * (and also Battery and Device Information services). This application uses the
 * @ref srvlib_conn_params module.
 */
#include "ble_service.h"
#include "data_struct.h"
#include "global_var.h"
#include "ble_gattc.h"
//#include "mtk_uart.h"
#include "RD_MotoControl.h"
#include "RD_BatteryCharge.h"
#include "nrf_delay.h"

#define USE_ANOTHER_NAME

#define   TX_POWER_LEVEL                        (0)                                        /**< TX Power Level value. This will be set both in the TX Power service, in the advertising data, and also used to set the radio transmit power. */

static    app_timer_id_t                        m_time_event_id;
static    uint16_t                              m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */
static    ble_gap_sec_params_t                  m_sec_params;                              /**< Security requirements for this application. */
static    ble_gap_adv_params_t                  m_adv_params;                              /**< Parameters to be passed to the stack when starting advertising. */
static    uint16_t                              m_adv_interval = 800;

uint8_t  n_time = 0 ;
uint8_t  n_shake = 0 ;
uint8_t  n_call_in = 0 ;
uint8_t  n_shake_status = 0 ;
uint8_t  n_cal_in_status = 0 ;
uint16_t  n_time_shake = 0 ;

#if defined(ADD_WOR_MODE)
uint8_t  n_work_mode = 0 ;
#endif
#if defined(USE_ANOTHER_NAME)
unsigned char device_name[20] = {0};
#endif

/**@brief Function for error handling, which is called when an error has occurred.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    // This call can be used for debug purposes during application development.
    // @note CAUTION: Activating this code will write the stack to flash on an error.
    //                This function should NOT be used in a final product.
    //                It is intended STRICTLY for development/debugging purposes.
    //                The flash write will happen EVEN if the radio is active, thus interrupting
    //                any communication.
    //                Use with care. Uncomment the line below to use.
    // ble_debug_assert_handler(error_code, line_num, p_file_name);

    // On assert, the system can only recover with a reset.
//    NVIC_SystemReset();
}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

#if NOR_USER_MOTOR	
//鍗忚鏍￠獙
static bool checkout_receive_data(uint8_t* receive_buf, uint8_t size)
{
	if(receive_buf[0] != 0xF5 || receive_buf[size-1] != 0x55) return false;
	uint8_t pure_data_size = receive_buf[2];
	if((5+pure_data_size) != size) return false;
	uint8_t check_res = 0;
	for(uint8_t i=1;i<size - 2;i++) check_res += receive_buf[i];
	if(receive_buf[size-2] != check_res) return false;
	return true;
}
#endif

static bool checkout_receive_start_end_data(uint8_t* receive_buf,uint8_t size)
{
	if(receive_buf[0] == 0xAA && receive_buf[9] == 0x55) 
		return true;
	return false;
}


//鎵撳寘鍙戦�?
void send_data_to_phone(uint8_t cmd, uint8_t* data_buff, uint16_t size)
{
	if(m_blue_connect)
	{
		uint8_t send_buf[20] = {0xAA};

		// 协议头
		send_buf[0] = 0xAA;
		
		// 产品编号	
		send_buf[1] = 0x1F;
		send_buf[2] = 0x40;
		
		// 生产编号
		send_buf[3] = 0x50;

		// 电量
		send_buf[4] = *data_buff;
		//send_buf[5] = 0x40;

		// 振动方式
		send_buf[5] = 0x01;

		// 温度
		send_buf[6] = 0x00;
		send_buf[7] = 0x00;

		// 心率
		send_buf[8] = 0x00;

		// 状态
		send_buf[9] = 0x00;

		// 预留
	#if defined(ADD_WOR_MODE)
		send_buf[10] = n_work_mode;
	#else
		send_buf[10] = 0x00;
	#endif
		send_buf[11] = 0x00;
		send_buf[12] = 0x00;
		send_buf[13] = 0x00;
		send_buf[14] = 0x00;
		send_buf[15] = 0x00;
		send_buf[16] = 0x00;
		send_buf[17] = 0x00;

		//校验
		send_buf[18] = 0x00;

		// 协议尾
		send_buf[19] = 0x55;

		
		ble_data_update(&m_ble,send_buf, 20);
  }
}


extern bool                                         allShake;

#if 1

uint16_t get_shake_time(void)
{
	uint16_t ret_value = 0 ;

	ret_value = n_time * 60 * 2 ;
	
	return ret_value ;
}

uint8_t get_shake_key_time(void)
{
	uint8_t ret_value = 0 ;

	ret_value =  10 ;
	
	return ret_value ;
}

uint8_t get_shake_Strength(void)
{
	uint8_t ret_value = 0 ;

	ret_value = n_shake ;
	
	return ret_value ;
}


static void receive_data_handle(ble_t * p_trans, ble_s_evt_t * p_evt,uint8_t* p_buff,uint8_t len)
{	
  if(checkout_receive_start_end_data(p_buff, len))
	{
		// 防丢开关
		// AA000000010000000055
		{
			//n_time = 			p_buff[1] ;
			//n_shake = 			p_buff[2] ;	
			//n_call_in = 		p_buff[3] ;	
			//n_shake_status = 	p_buff[4] ;

			if ((p_buff[1] == 0) && (p_buff[2] == 0) && (p_buff[4] ==1 || p_buff[4] == 0))
			{
				n_shake_status = 	p_buff[4] ;
				return ;
			}
  		}
		StopKeyPressShake();
		//n_start = 			p_buff[0] ;
		
		n_call_in = 		p_buff[3] ;	
		//n_shake_status = 	p_buff[4] ;	
		n_cal_in_status = p_buff[5] ;	
	#if defined(ADD_WOR_MODE)
		n_work_mode = p_buff[7] ;
	#endif

	
		// 来电提醒
		if ((1 == n_call_in) && (n_cal_in_status == 1))
		{
			//uint8_t n_key_time = 0 ;
			
			n_time_shake = get_shake_key_time();
			Rd_MotoShakeMode(true,SHAKE_OPPOINT,n_shake,0);
			n_call_in = 0 ;
			n_cal_in_status = 0 ;
			return ;
		}
		n_time = 			p_buff[1] ;
		n_shake = 			p_buff[2] ;	
		n_shake_status = 	p_buff[4] ;

		if (n_time !=0 && n_shake != 0)
		{
		#if 0
			if (n_time >=10)
			{
				n_time = 10 ;
			}
		#else
			if (n_time >=15)
			{
				n_time = 15 ;
			}
		#endif
			if (n_shake >= 6)
			{
				n_shake = 6 ;
			}
			
		#if defined(ADD_NRF_PWM_CTRL)	
			pwm_duty = n_shake;
		#endif
			n_time_shake = get_shake_time();
			//get_shake_Strength();
			//Rd_MotoShakeFace();
			//Rd_MotoShake();
			Rd_MotoShakeMode(true,SHAKE_OPPOINT,n_shake,0);
			{
				uint8_t percent = battery_level_transform();
				
				send_data_to_phone(0,&percent,0);
			}
		}
	}
}

void SetKeyPressTime(void)
{
	//n_time = 10 ;
	n_time = 15 ;
	n_time_shake = get_shake_time();
}

void StopKeyPressShake(void)
{
	n_time = 0 ;
	n_time_shake = get_shake_time();
	Rd_MotoShakeMode(false,SHAKE_STOP,0,0);
}

#else
//鎺ユ敹浜嬩欢骞剁紦瀛樹簨浠舵暟鎹?
static void receive_data_handle(ble_t * p_trans, ble_s_evt_t * p_evt,uint8_t* p_buff,uint8_t len)
{
	uint8_t ret[5] = {0x01};
  if(checkout_receive_data(p_buff, len))
	{
			switch(p_buff[1])
			{
				case 0x00:
					Rd_MotoShakeTypeSet(false,SHAKE_STOP,0,0);
				  send_data_to_phone(0x00,ret,1);
				  allShake = false;
					break;
				case 0x01:
					Rd_MotoShakeTypeSet(true,SHAKE_ODD,0,0);
				  send_data_to_phone(0x01,ret,1);
				  allShake = false;
					break;
				case 0x02:
					Rd_MotoShakeTypeSet(true,SHAKE_EVEN,0,0);
				  send_data_to_phone(0x02,ret,1);
				  allShake = false;
					break;
				case 0x03:
					Rd_MotoShakeTypeSet(true,SHAKE_RANDOM,0,0);
				  send_data_to_phone(0x03,ret,1);
				allShake = false;
					break;
				case 0x04:
					Rd_MotoShakeTypeSet(true,SHAKE_FACE,0,0);
				  send_data_to_phone(0x04,ret,1);
				allShake = false;
					break;
				case 0x05:
					Rd_MotoShakeTypeSet(true,SHAKE_BACK,0,0);
				  send_data_to_phone(0x05,ret,1);
				allShake = false;
					break;
				case 0x06:
					Rd_MotoShakeTypeSet(true,SHAKE_SEQUENCE,0,p_buff[3]);
				  ret[0] = p_buff[3];
				  send_data_to_phone(0x06,ret,2);
				allShake = false;
					break;
				case 0x07:{
					uint8_t shakeSel[3] = {p_buff[3],p_buff[4],p_buff[5]};
					Rd_MotoShakeTypeSet(true,SHAKE_OPPOINT,shakeSel,0);
					send_data_to_phone(0x07,ret,1);
					allShake = false;
					break;
				}
				case 0x50:
					Bat_Return();
					break;
			}
	}
}
#endif

/**
 *鏈湴鏃堕挓tick鍑芥暟
 */
static void detection_buf_timeout_hander(void * p_context)
{
#if defined(CTRL_POWER_OUT)
	static int count = 0;
	count++;
	if(count <50)
		return;
	count =0;
#endif

	UNUSED_PARAMETER(p_context);
  charge_check_handle();
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
void timers_init(void)
{
	// Initialize timer module.
	APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);
	
	uint32_t err_code = app_timer_create(&m_time_event_id, APP_TIMER_MODE_REPEATED, detection_buf_timeout_hander);
	APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
void gap_params_init(void)
{
	uint32_t                err_code;
	ble_gap_conn_params_t   gap_conn_params;
	ble_gap_conn_sec_mode_t sec_mode;
	
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
#if !defined(USE_ANOTHER_NAME)	
	err_code = sd_ble_gap_device_name_set(&sec_mode,(const uint8_t *)DEVICE_NAME,
										strlen(DEVICE_NAME));
#else
	err_code = sd_ble_gap_device_name_set(&sec_mode,(const uint8_t *)device_name,
										strlen((char*)device_name));
#endif

																				
	APP_ERROR_CHECK(err_code);

	err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HEART_RATE_SENSOR_HEART_RATE_BELT);
	APP_ERROR_CHECK(err_code);

	memset(&gap_conn_params, 0, sizeof(gap_conn_params));

	gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
	gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
	gap_conn_params.slave_latency     = SLAVE_LATENCY;
	gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

	err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
	APP_ERROR_CHECK(err_code);
																				
	err_code = sd_ble_gap_tx_power_set(TX_POWER_LEVEL);
  APP_ERROR_CHECK(err_code);		
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
	  ble_gap_addr_t 	mac_addr;
	  ble_advdata_manuf_data_t  manuf_data;

//    ble_uuid_t adv_uuids[] =
//    {
//				{BLE_UUID_TROLLEY_CASE_SERVICE,BLE_UUID_TYPE_BLE}
//    };
		
		err_code = sd_ble_gap_address_get(&mac_addr);
	  APP_ERROR_CHECK(err_code);
	  manuf_data.data.size = 6;
	  manuf_data.data.p_data = mac_addr.addr;

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags.size              = sizeof(flags);
    advdata.flags.p_data            = &flags;
		advdata.p_manuf_specific_data   = &manuf_data;
//    advdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
//    advdata.uuids_complete.p_uuids  = adv_uuids;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    m_adv_params.p_peer_addr = NULL;                           // Undirected advertisement.
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = m_adv_interval;
    m_adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;
}


static void dis_init(void)
{
	uint32_t       err_code;
	ble_dis_init_t dis_init;	
	
	// Initialize Device Information Service.
	memset(&dis_init, 0, sizeof(dis_init));
	
	ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)MANUFACTURER_NAME);

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

	err_code = ble_dis_init(&dis_init);
	APP_ERROR_CHECK(err_code);
}

//
static void trans_init(void)
{
	uint32_t       err_code;
	ble_init_t trans_init;
	memset(&trans_init, 0, sizeof(trans_init));
	
	// Here the sec level for the Battery Service can be changed/increased.
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&trans_init.data_char_attr_md.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&trans_init.data_char_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&trans_init.data_char_attr_md.write_perm);

//	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&trans_init.trans_report_attr_md.read_perm);
//	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&trans_init.trans_report_attr_md.write_perm);

	trans_init.evt_handler          = receive_data_handle;
	trans_init.support_notification = true;
	
	err_code = ble_init(&m_ble, &trans_init);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
void services_init(void)
{
	 dis_init();
	 trans_init();
}

/**@brief Function for initializing security parameters.
 */
void sec_params_init(void)
{
    m_sec_params.timeout      = SEC_PARAM_TIMEOUT;
    m_sec_params.bond         = SEC_PARAM_BOND;
    m_sec_params.mitm         = SEC_PARAM_MITM;
    m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    m_sec_params.oob          = SEC_PARAM_OOB;
    m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}

/**@brief Function for starting application timers.
 */
void application_timers_start(void)
{
	uint32_t err_code;
	// Start application timers
	err_code = app_timer_start(m_time_event_id, DETECTION_BUF_INTERVAL, NULL);
	
	APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting application timers.
 */
void application_timers_stop(void)
{
    uint32_t err_code;

    // Stop key timers
    err_code = app_timer_stop(m_time_event_id);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
void advertising_start(void)
{
    uint32_t err_code;
    
    err_code = sd_ble_gap_adv_start(&m_adv_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for stopping advertising.
 */
void advertising_stop(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_stop();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in]   p_evt   Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = m_ble.send_data_handles.cccd_handle;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;
	  //int32_t   rssi;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
				    m_blue_connect = true;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
						err_code = ble_bondmngr_bonded_centrals_store();
						APP_ERROR_CHECK(err_code);
            m_blue_connect = false;
			if (n_shake_status == 1)
			{
				//uint8_t n_key_time = 0 ;
				StopKeyPressShake();
				n_time_shake = get_shake_key_time();
				Rd_MotoShakeMode(true,SHAKE_OPPOINT,n_shake,0);
				//return ;
			}
            advertising_start();
            break;
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
						err_code = sd_ble_gap_sec_params_reply(m_conn_handle,
																									 BLE_GAP_SEC_STATUS_SUCCESS,
																									 &m_sec_params);
						APP_ERROR_CHECK(err_code);
            break;
        case BLE_GAP_EVT_TIMEOUT:
            if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISEMENT)
            {
              advertising_start();
            }
            break;
        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)//钃濈墮浜嬩欢瀹夋帓娲惧嚭
{
    ble_bondmngr_on_ble_evt(p_ble_evt);
	  ble_on_ble_evt(&m_ble,p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
void ble_stack_init(void)
{
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_2000MS_CALIBRATION, false);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling a Bond Manager error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void bond_manager_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for the Bond Manager initialization.
 */
void bond_manager_init(void)
{
    uint32_t            err_code;
    ble_bondmngr_init_t bond_init_data;
    
    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code); 

    // Initialize the Bond Manager.
    bond_init_data.evt_handler             = NULL;
    bond_init_data.error_handler           = bond_manager_error_handler;
    bond_init_data.bonds_delete            = false;

    err_code = ble_bondmngr_init(&bond_init_data);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Power manager.
 */
 void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
} 



#if defined(FLASH_READ_WRITE)

pstorage_module_param_t module_param;
static pstorage_handle_t handle;
pstorage_handle_t dest_block_id;
uint8_t sys_flag_data[SYS_FLAG_DATA_LEN] __attribute__ ((aligned (4)));
uint8_t sys_flag_data_temp[SYS_FLAG_DATA_LEN] __attribute__ ((aligned (4)));

#if defined(CHAGRE_REST_ONCE)
extern uint8_t charge_reset_falg;
#endif

static void my_cb(pstorage_handle_t * handle,uint8_t op_code,uint32_t result,
uint8_t * p_data, uint32_t data_len)
{
//	char tmp[20] = {0};
	switch(op_code)
	{
		case PSTORAGE_UPDATE_OP_CODE:
			if (result == NRF_SUCCESS)
			{
				//my_flag = 1; //当 flash update 完成后置位标志。 Main 函数中便可以读 flash 数据了
			#if defined(DEBUG_UART)
				printf("store success!\r\n");
			#endif
			if(charge_reset_falg)
				NVIC_SystemReset();
			}
			else
			{
			// Update operation failed.
			}
			break;
	}
}

void Storage_Module_init(void)
{
	module_param.block_count = 2;
	module_param.block_size = 16;
	module_param.cb = my_cb;
	pstorage_init();
	pstorage_register(&module_param, &handle);

	nrf_delay_ms(50);	//100
//	pstorage_block_identifier_get(&handle, 1, &dest_block_id);
//	pstorage_update(&dest_block_id, (uint8_t *)&my_buff, 4, 0);
#if dbg
//	//printf("Storage_Module_init\r\n");
#endif

}


void falsh_read(void)
{
	Storage_Module_init();
	pstorage_block_identifier_get(&handle,1,&dest_block_id);
	pstorage_load((uint8_t *)sys_flag_data, &dest_block_id, 4, 0);
	//nrf_delay_ms(500);	//500
	memcpy(sys_flag_data_temp,sys_flag_data,sizeof(sys_flag_data_temp));
	charge_reset_falg = sys_flag_data[0];
	
}

void falsh_write(void)
{
	pstorage_block_identifier_get(&handle,1,&dest_block_id);
	pstorage_update(&dest_block_id,(uint8_t *)sys_flag_data, 4, 0);
	nrf_delay_ms(120);	//500
}

void update_flash_data(void)
{
	if(memcmp(sys_flag_data_temp,sys_flag_data,sizeof(sys_flag_data)))
	{
		memcpy(sys_flag_data_temp,sys_flag_data,4);
		Storage_Module_init();
		falsh_write();
	}
}
#endif



#if defined(ADD_NRF_PWM_CTRL)
extern	void pwm_base_init(void);
#endif

//钃濈墮鍒濆鍖?
void BleInit(void)
{
	// Initialize.
#if defined(USE_ANOTHER_NAME)		
	ble_gap_addr_t   mac_addr;
	uint32_t err_code;
#endif	
#if defined(ADD_NRF_PWM_CTRL)
	pwm_base_init();
#endif	
	
	ble_stack_init();
#if defined(USE_ANOTHER_NAME)	
	err_code = sd_ble_gap_address_get(&mac_addr);
	APP_ERROR_CHECK(err_code);
	sprintf((char*)device_name,"XMLY%02x%02x%02x",mac_addr.addr[0],mac_addr.addr[1],mac_addr.addr[2]);
#endif

	#if defined(FLASH_READ_WRITE)
		falsh_read();
	#endif

	bond_manager_init();
	timers_init();
	gap_params_init();
	advertising_init();
	services_init();
	conn_params_init();
	sec_params_init();

	// Start execution.
	application_timers_start();
	advertising_start();
}



