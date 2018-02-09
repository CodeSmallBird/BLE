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
#include "ble_exdevice.h"
#include "nrf_delay.h"
#define   TX_POWER_LEVEL                        (0)                                        /**< TX Power Level value. This will be set both in the TX Power service, in the advertising data, and also used to set the radio transmit power. */

static    app_timer_id_t                        m_time_event_id;
static    uint16_t                              m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */
static    ble_gap_sec_params_t                  m_sec_params;                              /**< Security requirements for this application. */
static    ble_gap_adv_params_t                  m_adv_params;                              /**< Parameters to be passed to the stack when starting advertising. */
static    uint16_t                              m_adv_interval = 800;

//static uint8_t ad_flag =0;		//广播标志
//////////////////////////////


////////////////////////

 Receive_Cmd receivecmd;
extern char LOGO[3];
extern char ID[6];
extern uint8_t g_bat_present;
extern uint8_t g_moto_flag;
extern uint8_t g_devicename_set;
extern uint32_t g_number ;
extern uint8_t g_receicve_cmd;
extern uint32_t						                  m_last_tickcount;
extern uint32_t						                  m_ltmin_tickcount;
extern uint32_t							              m_total_tickcount	;
extern ble_system_clock_t                    m_clock ;
extern ble_system_clock_t                    m_correct_clock;    
extern ble_system_clock_t  						open_clock;
extern ble_system_clock_t 						close_clock;

DEVIC_NAME_INFO device_name_info;
CARD_RIDE_INFO last_card_ride_info;
static ble_gap_addr_t 	mac_addr;
static uint32_t m_clock_counter = 0;

///////////////////////////

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
	//	printf("%d",error_code);
	//	printf("%d",line_num);
    NVIC_SystemReset();
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

uint32_t encrypt(uint32_t plainText, uint32_t key)  
{  
	return ((plainText+12131679)^key);  
}  

uint32_t  decrypt(uint32_t cipherText, uint32_t key)  
{  
	return ((cipherText^key)-12131679);  
}  


static uint32_t key_data=0xAA55;

// 字节长度应是2的倍数
void data_encrypt(uint8_t *encrypt_data,uint8_t length)
{
	uint32_t temp_data;
	
	for(uint8_t i=0;i<length/2;i++)
	{
		temp_data = (((uint32_t)encrypt_data[i*2])<<8 | encrypt_data[i*2+1]);
		temp_data = encrypt(temp_data,key_data);
		encrypt_data[i*2] 	 =(uint8_t)(temp_data>>8);
		encrypt_data[i*2+1] =(uint8_t)temp_data;
	}
}

void data_decrypt(uint8_t *decrypt_data,uint8_t length)
{
	uint32_t temp_data;
	
	for(uint8_t i=0;i<length/2;i++)
	{
		temp_data = (((uint32_t)decrypt_data[i*2])<<8 | decrypt_data[i*2+1]);
		temp_data = decrypt(temp_data,key_data);
		decrypt_data[i*2] 	 =(uint8_t)(temp_data>>8);
		decrypt_data[i*2+1] =(uint8_t)temp_data;
	}
}



static bool checkout_receive_start_end_data(uint8_t* receive_buf,uint8_t size)
{
	uint8_t receive_crc = 0x00,cal_crc = 0x00;
	uint8_t i = 0;
	if(size<20)		return false;
	if(receive_buf[0] == 0xAA && receive_buf[19] == 0x55) 
	{
		receive_crc = receive_buf[18];
		
		for(i = 1;i<18;i++)
		{
				cal_crc ^= receive_buf[i];
		}
		if(cal_crc!=receive_crc)
		{
		#if dbg
			printf("cal_crc = 0x%02x,receive_crc = 0x%02x\r\n",cal_crc,receive_crc);
		#endif
			return false;
		}
			
		else
		return true;
	}
	else
		return false;
}



#if 1


void send_data_to_phone_time(void)
{

	uint8_t send_buf[20] = {0xAA};
	if(m_blue_connect)
	{
		send_buf[0]  = 0xaa;
		send_buf[1]  |=(((m_clock.year/10)&0x0f)<<4);
		send_buf[1]  |= (((m_clock.year%10)&0x0f));

		send_buf[2]  |=(((m_clock.month/10)&0x0f)<<4);
		send_buf[2]  |= (((m_clock.month%10)&0x0f));

		send_buf[3]  |=(((m_clock.day/10)&0x0f)<<4);
		send_buf[3]  |= (((m_clock.day%10)&0x0f));

		send_buf[4]  |=(((m_clock.hour/10)&0x0f)<<4);
		send_buf[4]  |= (((m_clock.hour%10)&0x0f));

		send_buf[5]  |=(((m_clock.minute/10)&0x0f)<<4);
		send_buf[5]  |= (((m_clock.minute%10)&0x0f));

		send_buf[6]  |=(((m_clock.second/10)&0x0f)<<4);
		send_buf[6]  |= (((m_clock.second%10)&0x0f));
		send_buf[7]  = 0x55;
 		ble_data_update(&m_ble,send_buf, 20);
	}

	
}

void send_data_to_phone(uint8_t cmd,CARD_RIDE_INFO param)
{

	uint8_t crc = 0x00,i = 0;
	uint8_t send_buf[20] = {0xAA};
	if(m_blue_connect)
	{
		send_buf[0] = 0xAA;
		
		send_buf[1] = 0x1F;
		send_buf[2] = 0x41;
		
		send_buf[3] = 0x01;

		//cmd
		send_buf[4] = cmd;

		// RESET
		send_buf[5] = 0x00;

		// 卡ID 及 交易时间
		{
			memcpy(send_buf+6,&param,sizeof(CARD_RIDE_INFO)); 
		}
		data_encrypt(send_buf+6,CARD_ID_LENGTH);
		
		for(i = 1;i<18;i++)
			crc ^=	send_buf[i];
		
		send_buf[18] = crc;
		
		send_buf[19] = 0x55;
		ble_data_update(&m_ble,send_buf, 20);
  }
#if 0//dbg 
	printf("return data:\r\n");
	for(i=0;i<20;i++)
		printf("0x%02x ",send_buf[i]);
	printf("\r\n");

#endif	
	
	
}

#endif

unsigned char device_name[20] = {0};
static	DEVIC_NAME_INFO device_name_info_temp;

void gen_DeviceName(void)
{
	char string_time[8] ={0};
	uint8_t len = 0;
	uint8_t i = 0;
	if(memcmp(&device_name_info_temp,&device_name_info,sizeof(device_name_info)) == 0)
		return;
	device_name_info.vol = battery_level_transform();
	if(device_name_info.vol>=100)
		device_name_info.vol = 99;
	memcpy(&device_name_info_temp,&device_name_info,sizeof(device_name_info));
	
	len += sprintf((char *)device_name,"%.2s",device_name_info.logo);

	len += sprintf((char *)device_name+len,"%02d",device_name_info.vol);	
	len += sprintf((char *)device_name+len,"%1d",device_name_info.lock_state);
	
	for(i=0;i<4;i++)
	{
		len += sprintf((char *)device_name+len,"%02x",device_name_info.card_ride.card_id[i]);
	}
	//device_name_info.card_ride.ride_time = 1036800;
	sprintf(string_time,"%05x",device_name_info.card_ride.ride_time);
	len += sprintf((char *)device_name+len,"%05s",string_time);
#if dbg
	printf("device_name = %s\r\n",device_name);	
#endif
	gap_params_init();
	advertising_init();
}

void SetBle_Name(void)
{
	memset(&device_name_info,0,sizeof(device_name_info));
	memset(&device_name_info_temp,0,sizeof(device_name_info_temp));
	sprintf((char *)device_name_info.logo,"%.2s","ZH");
	sprintf((char *)device_name,"%.2s",device_name_info.logo);
}

void Moto_DISABLE(void)
{
	nrf_gpio_pin_clear(MOTO_EN);
	nrf_gpio_pin_clear(IA_EN);
	nrf_gpio_pin_clear(IB_EN);
}
void Moto_A2B(void)
{
	nrf_gpio_pin_set(MOTO_EN);
	nrf_gpio_pin_set(IA_EN);
	nrf_gpio_pin_clear(IB_EN);
}
void Moto_B2A(void)
{
	nrf_gpio_pin_set(MOTO_EN);
	nrf_gpio_pin_set(IB_EN);
	nrf_gpio_pin_clear(IA_EN);
}


void open_key_ctrl(void)
{
	Moto_A2B();
	nrf_gpio_pin_clear(BUZZER_EN);
	moto_start();
	buzzer_start(BUZZER_NORMAL);
	device_name_info.lock_state = REQUEST_OPEN_LOCK;
}

///////////////////////////////

void MT2503_Reset(void)
{
//	nrf_gpio_pin_clear(MT2503_RESET_PIN);
//	nrf_delay_ms(1000);
//	nrf_gpio_pin_set(MT2503_RESET_PIN);	
}

void string_to_hex(unsigned char *input_string, uint8_t *out_hex)
{
	uint8_t i;
	*out_hex = 0;
	for(i=0;i<2;i++)
	{
		if((input_string[i] >= '0')&&(input_string[0] <= '9'))
		{
			*out_hex |= (input_string[i] -'0')<<(4*(1-i));
		}
		else if((input_string[0] >= 'a')&&(input_string[0] <= 'f'))
		{
			*out_hex |= (input_string[0] -'a')<<(4*(1-i));
		}
		else if((input_string[0] >= 'A')&&(input_string[0] <= 'F'))
		{
			*out_hex |= (input_string[0] -'A')<<(4*(1-i));
		}
		else
		{
			*out_hex |= (0x0a)<<(4*(1-i));
		}
	}
}



static void receive_data_handle(ble_t * p_trans, ble_s_evt_t * p_evt,uint8_t* p_buff,uint8_t len)
{	
#if dbg
	uint8_t i = 0;
	printf("receive data:\r\n");
	for(i=0;i<len;i++)
	{
		printf("0x%02x ",p_buff[i]);
	}	
	printf("\r\n");
#endif
  	if(checkout_receive_start_end_data(p_buff, len))
	{
		uint8_t mac_check[6];
		memset(&receivecmd,0,sizeof(receivecmd));
		receivecmd.cmd = p_buff[1];
		receivecmd.reset = p_buff[2];

		memcpy(mac_check,p_buff+3,6);	// mac 地址

		data_decrypt(mac_check,6);
		if(memcmp(mac_check,mac_addr.addr,6) == 0)
		{
			if(receivecmd.reset == 0x01)
			{
				//使用复位功能的时候打开
				
			}
			else if(receivecmd.cmd == 0x01) //请求开锁
			{
				{
					uint8_t i; 
					//更新卡的ID号
					for(i=0;i<4;i++)
					{
						string_to_hex(p_buff+9+i*2,device_name_info.card_ride.card_id+i);
					}
				}
				open_key_ctrl();
			}
			else if(receivecmd.cmd == 0x02) //查询交易记录
			{
				send_data_to_phone(TRADE_RETURN,last_card_ride_info);
			}
			return ;
		}
		else
		{
			send_data_to_phone(KEY_SECRET_FAIL,last_card_ride_info);
		}
	}
	else
	{
		send_data_to_phone(CHECK_FAIL,last_card_ride_info);
	}
}

static void detection_buf_timeout_hander(void * p_context)
{	
	UNUSED_PARAMETER(p_context);
	static bool ride_flag = false;
//	static uint16_t request_time_delay = 0;


	if(device_name_info.card_ride.dect_delay_time)
		device_name_info.card_ride.dect_delay_time--;

	if(device_name_info.card_ride.delay_flag)
	{
		device_name_info.card_ride.delay_time++;
		//同一张卡15秒内不能刷第二次
		if(device_name_info.card_ride.delay_time>(8))		//30
		{
			device_name_info.card_ride.delay_time = 0;
			device_name_info.card_ride.delay_flag = false;
		}
	}
	if((device_name_info.lock_state == OPEN_LOCK)&&(ride_flag == false))
	{
		ride_flag = true;
		m_clock_counter = 0;
	}
	else if((device_name_info.lock_state == CLOSE_LOCK)&&(ride_flag == true))
	{
		ride_flag = false;
	}
	/*else if(device_name_info.lock_state == REQUEST_OPEN_LOCK)
	{
		request_time_delay++;
		//刷卡请求30s，没有开锁，清除请求状态
		if(request_time_delay>300)		
		{
			device_name_info.lock_state = CLOSE_LOCK;
			request_time_delay = 0;
		}
	}*/
	else
	{
		//request_time_delay = 0;
	}
	if(ride_flag)
		m_clock_counter++;
#if 0
	{
		static uint16_t ad_time =0;
			ad_time++;
		if(ad_time == 30)
		{
			if(ad_flag == 0)
				advertising_start();
		}
		else if(ad_time == 60)
		{
			if(ad_flag == 1)
				advertising_stop();
		}	
		else if(ad_time == 120)
		{
			ad_time = 0;
		}

	}
#endif	

#if defined(DEBUG_UART)
{
	static uint8_t tick_dida = 0;
	tick_dida++;
	if(tick_dida > 10)
	{
		printf("m_clock_counter:%ld\r\n",m_clock_counter);
		tick_dida = 0;
	}
}
#endif

}

uint32_t get_m_clock_counter(void)
{
	return m_clock_counter/10;
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
#if 1 
void gap_params_init(void)
{

}
#else
void gap_params_init(void)
{
	uint32_t                err_code;
	ble_gap_conn_params_t   gap_conn_params;
	ble_gap_conn_sec_mode_t sec_mode;
	
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
	
//	err_code = sd_ble_gap_device_name_set(&sec_mode,
//																				(const uint8_t *)DEVICE_NAME,
//																				strlen(DEVICE_NAME));
	
	
	err_code = sd_ble_gap_device_name_set(&sec_mode,
																				(const uint8_t *)device_name,
																				strlen((char *)device_name));
	
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
#endif

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
 #if 1
void advertising_init(void)
{

}
#else
void advertising_init(void)
{
	uint32_t      err_code;
	ble_advdata_t advdata;
	uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
	ble_advdata_manuf_data_t  manuf_data;

	//    ble_uuid_t adv_uuids[] =
	//    {
	//				{BLE_UUID_TROLLEY_CASE_SERVICE,BLE_UUID_TYPE_BLE}
	//    };

	err_code = sd_ble_gap_address_get(&mac_addr);
	APP_ERROR_CHECK(err_code);
	manuf_data.data.size = 0;
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
#endif

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
#if 0
    uint32_t err_code;
    
    err_code = sd_ble_gap_adv_start(&m_adv_params);
    APP_ERROR_CHECK(err_code);
	//ad_flag = 1;
	advertising_stop();
#endif
}

/**@brief Function for stopping advertising.
 */
void advertising_stop(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_stop();
    APP_ERROR_CHECK(err_code);
	//ad_flag = 0;
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
void ble_evt_dispatch(ble_evt_t * p_ble_evt)//钃濈墮浜嬩欢瀹夋帓娲惧嚭
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
	//SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, false);
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
	
//		err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
//	APP_ERROR_CHECK(err_code);
	
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

#if defined(FLASH_READ_WRITE)
static pstorage_module_param_t module_param;
static pstorage_handle_t handle;
pstorage_handle_t dest_block_id;
uint8_t flash_save_data[SYS_FLAG_DATA_LEN] __attribute__ ((aligned (4)));
uint8_t flash_save_data_temp[SYS_FLAG_DATA_LEN] __attribute__ ((aligned (4)));

static void my_cb(pstorage_handle_t * handle,uint8_t op_code,uint32_t result,
uint8_t * p_data, uint32_t data_len)
{
	switch(op_code)
	{
		case PSTORAGE_UPDATE_OP_CODE:
			if (result == NRF_SUCCESS)
			{
				//my_flag = 1; //当 flash update 完成后置位标志。 Main 函数中便可以读 flash 数据了
			#if defined(DEBUG_UART)
				printf("store success!\r\n");
			#endif
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
}


void falsh_read(void)
{
	Storage_Module_init();
	pstorage_block_identifier_get(&handle,1,&dest_block_id);
	pstorage_load((uint8_t *)flash_save_data, &dest_block_id, SYS_FLAG_DATA_LEN, 0);
	memcpy(flash_save_data_temp,flash_save_data,sizeof(flash_save_data));
	memcpy(&last_card_ride_info,flash_save_data_temp,4);
}

void falsh_write(void)
{
	pstorage_block_identifier_get(&handle,1,&dest_block_id);
	pstorage_update(&dest_block_id,(uint8_t *)flash_save_data, SYS_FLAG_DATA_LEN, 0);
	nrf_delay_ms(120);	
}

void update_flash_data(void)
{
	if(memcmp(flash_save_data_temp,flash_save_data,sizeof(flash_save_data)))
	{
		memcpy(flash_save_data_temp,flash_save_data,SYS_FLAG_DATA_LEN);
		Storage_Module_init();
		falsh_write();
	}
}
#endif



/**@brief Function for the Power manager.
 */
 void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}
 
void BleInit(void)
{
	ble_stack_init();
	SetBle_Name();
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


