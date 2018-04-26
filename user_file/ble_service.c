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

//////////////////////////////

 Receive_Cmd receivecmd;
extern char LOGO[3];
extern char ID[6];
extern uint8_t g_bat_present;
extern uint8_t g_lock_flag;
extern uint8_t g_moto_flag;
extern bool g_devicename_set;
extern uint32_t g_number ;
extern uint8_t g_receicve_cmd;
extern uint32_t						                  m_last_tickcount;
extern uint32_t						                  m_ltmin_tickcount;
extern uint32_t							              m_total_tickcount	;
extern ble_system_clock_t                    m_clock ;
extern ble_system_clock_t                    m_correct_clock;    
extern bool   m_timer_battery;
extern ble_system_clock_t  						open_clock;
extern ble_system_clock_t 						close_clock;
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

void send_data_to_phone(uint8_t cmd,uint8_t status,uint8_t reset,unsigned int cipher)
{

	uint8_t crc = 0x00,i = 0;
	uint8_t send_buf[20] = {0xAA};
	if(m_blue_connect)
	{
		

		// Ð­ÒéÍ·
		send_buf[0] = 0xAA;
		
		// ²úÆ·±àºÅ	
		send_buf[1] = 0x1F;
		send_buf[2] = 0x41;
		
		// Éú²ú±àºÅ
		send_buf[3] = 0x01;

		//cmd
		send_buf[4] = cmd;
		//send_buf[5] = 0x40;

		// RESET
		send_buf[5] = reset;


		send_buf[6] = 0x00;
		send_buf[7] = 0x00;

		
		send_buf[8]  = (cipher>>24)&0xff;
		send_buf[9] =(cipher>>16)&0xff;
		send_buf[10] = (cipher>>8)&0xff;
		send_buf[11] = cipher&0xff;
//		

		if(cmd!=0x07)
		{
			send_buf[12] = 0x00;
			send_buf[13] = 0x00;
			send_buf[14] = 0x00;
			send_buf[15] = 0x00;
			send_buf[16] = 0x00;
			send_buf[17] = 0x00;
		}
		else
		{
			send_buf[12] = m_clock.year&0xff;
			send_buf[13] = m_clock.month&0xff;
			send_buf[14] = m_clock.day&0xff;
			send_buf[15] = m_clock.hour&0xff;
			send_buf[16] = m_clock.minute&0xff;
			send_buf[17] = m_clock.second&0xff;		
		}
		for(i = 1;i<18;i++)
			crc ^=	send_buf[i];
		//Ð£Ñé
		send_buf[18] = crc;
		// Ð­ÒéÎ²
		send_buf[19] = 0x55;
		ble_data_update(&m_ble,send_buf, 20);
  }
#if dbg 
	printf("return data:\r\n");
	for(i=0;i<20;i++)
		printf("0x%02x ",send_buf[i]);
	printf("\r\n");

#endif	
	
	
}



//void send_data_to_phone(uint8_t cmd,uint8_t status,uint8_t reset,unsigned int cipher)
//{
//	uint8_t crc = 0x00,i = 0;
//	uint8_t send_buf[20] = {0xAA};
//	if(m_blue_connect)
//	{
//		

//		// Э��ͷ
//		send_buf[0] = 0xAA;
//		
//		// ��Ʒ���	
//		send_buf[1] = 0x1F;
//		send_buf[2] = 0x40;
//		
//		// �������
//		send_buf[3] = 0x50;

//		//cmd
//		send_buf[4] = cmd;
//		//send_buf[5] = 0x40;

//		// RESET
//		send_buf[5] = reset;


//		send_buf[6] = 0x00;
//		send_buf[7] = 0x00;

//		
//		send_buf[8]  = (cipher>>24)&0xff;
//		send_buf[9] =(cipher>>16)&0xff;
//		send_buf[10] = (cipher>>8)&0xff;
//		send_buf[11] = cipher&0xff;
////		
//		send_buf[12] = status;
//		send_buf[13] = 0x00;
//		send_buf[14] = 0x00;
//		send_buf[15] = 0x00;
//		send_buf[16] = 0x00;
//		send_buf[17] = 0x00;
//		for(i = 1;i<18;i++)
//			crc ^=	send_buf[i];
//		//У��
//		send_buf[18] = crc;
//		// Э��β
//		send_buf[19] = 0x55;
//		ble_data_update(&m_ble,send_buf, 20);
//  }
//#if dbg 
//	printf("return data:\r\n");
//	for(i=0;i<20;i++)
//		printf("0x%02x ",send_buf[i]);
//	printf("\r\n");

//#endif	
//	
//	
//}

#endif



#if 1








unsigned char device_name[20] = {0};
//unsigned int device_num = 10000531;

//unsigned int device_num = 23178950;

unsigned int device_num = 12345678;
void SetBle_Name(void)
{
	memset(device_name,0,sizeof(device_name));
	sprintf((char *)device_name,"UGO%d",device_num);
}

unsigned int encrypt(int plainText, int key)  
{  
	return ((plainText+12131679)^1024);  
}  

unsigned int decrypt(int cipherText, int key)  
{  
	return ((cipherText^key)-12131679);  
//	return ((cipherText^key)-20170101);  
}  

int Shangxun_Moto_return(uint8_t cmd,uint8_t status,uint8_t reset,unsigned int cipher)
{

	send_data_to_phone(cmd,status,reset,cipher);	
	return 0;
}


int LOCK_Status(Receive_Cmd *rec)
{
	return 0;
}

#if defined(MT2503_GOTO_LOCK_PIN)
void mt2503_lock_noticfy(void)
{
	nrf_gpio_pin_set(MT2503_GOTO_LOCK_PIN);
	nrf_delay_ms(200);
	nrf_gpio_pin_clear(MT2503_GOTO_LOCK_PIN);
}
#endif
#if defined(MT2503_GOTO_UNLOCK_PIN)
void mt2503_unlock_noticfy(void)
{
	nrf_gpio_pin_set(MT2503_GOTO_UNLOCK_PIN);
//	nrf_delay_ms(200);
//	nrf_gpio_pin_clear(MT2503_GOTO_UNLOCK_PIN);
	gpio_timer_start();
}
#endif

void Moto_ENABLE(void)
{
	nrf_gpio_pin_set(MOTO_EN);
}
void Moto_DISABLE(void)
{
	nrf_gpio_pin_clear(MOTO_EN);
	nrf_gpio_pin_clear(IA_EN);
	nrf_gpio_pin_clear(IB_EN);
}
void Moto_A2B(void)
{
	nrf_gpio_pin_set(IA_EN);
	nrf_gpio_pin_clear(IB_EN);
}
void Moto_B2A(void)
{
	nrf_gpio_pin_set(IB_EN);
	nrf_gpio_pin_clear(IA_EN);
}


static int Shangxun_Moto_Handle(uint8_t cmd,uint8_t reset,unsigned int cipher)
{
	if(cmd == 0x01&&!g_moto_flag&&g_lock_flag==false)
	{
		g_receicve_cmd = 1;
		Moto_ENABLE();
		Moto_A2B();
		g_moto_flag = 1;
		nrf_gpio_pin_clear(BUZZER_EN);
		moto_timer_start();
//		buzzer_timer_stop();
//		buzzer_timer_start();	

	}
	return 0;
}
static void Shangxun_Cmd_return(uint8_t cmd,uint8_t reset,unsigned int cipherText)
{
		
	switch(cmd)
	{
		case  0x00:
			break;
		case 0x01:
			Shangxun_Moto_Handle(cmd,reset,cipherText);
			break;
		case 0x02:
//			Shangxun_Moto_Handle(cmd,reset,cipherText);
			break;
		default:break;
	}
	
	
	
	
}



static void Shangxun_Cmd_handle(uint8_t cmd,uint8_t reset ,unsigned int cipherText)
{
	switch(cmd)
	{

		case 0x01:
			Shangxun_Cmd_return(cmd,reset,cipherText);
			break;
		default:
			Shangxun_Cmd_return(0x00,reset,cipherText);
			break;
	}
}












///////////////////////////////

void MT2503_Reset(void)
{
//	nrf_gpio_pin_clear(MT2503_RESET_PIN);
//	nrf_delay_ms(1000);
//	nrf_gpio_pin_set(MT2503_RESET_PIN);	
}




static void SetRTC_Timer(ble_system_clock_t *clock)
{
	if(!clock)	return ;
	if( clock->year != m_clock.year||
		clock->month!=m_clock.month||
		clock->day!=m_clock.day||
		clock->hour!=m_clock.hour||
		clock->minute!=m_clock.minute)
	{
//			m_clock.year = clock->year;
//			m_clock.month = clock->month;
//			m_clock.day = clock->day;
//			m_clock.hour = clock->hour;
//			m_clock.minute = clock->minute;
//			m_clock.second = clock->second;
//			m_clock.tnum = m_clock.second;
			memcpy(&m_clock,clock,sizeof(ble_system_clock_t));
//			´Óµ±Ç°¼ÆÊýÖµ¿ªÊ¼¼ÆÊý
			memcpy(&m_correct_clock,&m_clock,sizeof(ble_system_clock_t));
			m_correct_clock.tnum = m_correct_clock.second;
			m_ltmin_tickcount = NRF_RTC0->COUNTER;
			m_last_tickcount = m_ltmin_tickcount;
			m_total_tickcount = 0;		
	}
	else
	{
		if(clock->second>=m_clock.second)
		{
			if(clock->second-m_clock.second>=45)
			{
//				m_clock.year = clock->year;
//				m_clock.month = clock->month;
//				m_clock.day = clock->day;
//				m_clock.hour = clock->hour;
//				m_clock.minute = clock->minute;
//				m_clock.second = clock->second;
				memcpy(&m_clock,clock,sizeof(ble_system_clock_t));
				
				memcpy(&m_correct_clock,&m_clock,sizeof(ble_system_clock_t));
				m_correct_clock.tnum = m_correct_clock.second;
				m_ltmin_tickcount = NRF_RTC0->COUNTER;
				m_last_tickcount = m_ltmin_tickcount;
				m_total_tickcount = 0;
				
			}
		}
	}
	
//	send_data_to_phone(0x01,g_bat_present,0x00,receivecmd.cipher);
	send_data_to_phone(0x07,0x00,0x00,receivecmd.cipher);
}





static void receive_data_handle(ble_t * p_trans, ble_s_evt_t * p_evt,uint8_t* p_buff,uint8_t len)
{	
	unsigned int cipher = 0x00;
	unsigned char cmd  = 0x00;
	ble_system_clock_t s_clock = {0};
//	unsigned int cipherText = 0x0000000;
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
	
	#if dbg
		printf("receive data ok\r\n");
	#endif
		memset(&receivecmd,0,sizeof(receivecmd));
		receivecmd.cmd = p_buff[1];
		receivecmd.reset = p_buff[2];
		receivecmd.cipher |=  (p_buff[5]<<24);
		receivecmd.cipher |=  (p_buff[6]<<16);
		receivecmd.cipher |=  (p_buff[7]<<8);
		receivecmd.cipher |=  (p_buff[8]<<0);
		cipher = decrypt(receivecmd.cipher,1024);
		if(cipher == g_number)
		{
			if(receivecmd.reset == 0x01)
			{
				//ʹ�ø�λ���ܵ�ʱ���
//				MT2503_Reset();
//				cmd = 0x03;
//				nrf_delay_ms(3000);
			////////////////////////////////////////	
				Shangxun_Moto_return(cmd,0x00,receivecmd.reset,receivecmd.cipher);
			}
			else if(receivecmd.cmd == 0x07)
			{
				s_clock.year = p_buff[9] ;
				s_clock.month = p_buff[10] ;
				s_clock.day = p_buff[11] ;
				s_clock.hour = p_buff[12] ;
				s_clock.minute = p_buff[13] ;
				s_clock.second = p_buff[14] ;
				SetRTC_Timer(&s_clock);
			}
			else
			{
				Shangxun_Cmd_handle(receivecmd.cmd,receivecmd.reset,receivecmd.cipher);
			}
			return ;
		}
		else
		{
			Shangxun_Moto_return(cmd,0x00,receivecmd.reset,0x00);	
			return ;
		}
	}
}







#endif

static void local_clock_maintain(void)
{
	uint32_t nCurTick = NRF_RTC0->COUNTER;
	m_total_tickcount += (nCurTick - m_last_tickcount);
	if(m_last_tickcount >= nCurTick)
	{
		m_total_tickcount += ((uint32_t)0xFFFFFF + nCurTick - m_last_tickcount);
	}
	else
	{
		m_total_tickcount += (nCurTick - m_last_tickcount);
	}
	m_last_tickcount = nCurTick;
//	m_clock.second = (nCurTick - m_ltmin_tickcount + 10) / 0x8000 + m_correct_clock.tnum;
//add by lxj 0613
	if(m_ltmin_tickcount>=nCurTick)
		m_clock.second = (0xFFFFFF+nCurTick - m_ltmin_tickcount + 10) / 0x8000 + m_correct_clock.tnum;	
	else
		m_clock.second = (nCurTick - m_ltmin_tickcount + 10) / 0x8000 + m_correct_clock.tnum;	
//add end	
	if(m_clock.second == 60)
	{
		m_correct_clock.tnum = 0;
		m_ltmin_tickcount = nCurTick;
			
		m_clock.second = m_clock.second % 60;
		if(++m_clock.minute == 60)
		{
			m_clock.minute = 0;
			if(++m_clock.hour == 24)
			{
				m_total_tickcount = 0;
				m_correct_clock.hour = 0;
				m_correct_clock.minute = 0;
				m_correct_clock.second = 0;
				
				m_clock.hour = 0;
				switch(m_clock.month)
				{
					case 1 :
					case 3 :
					case 5 :
					case 7 :
					case 8 :
					case 10 :
					case 12 :
					{
						if(++m_clock.day == 31)
						{
							m_clock.day = 0;
							if(++m_clock.month == 12)
							{
								m_clock.month = 0;
								if(++m_clock.year == 100)
									m_clock.year = 0;
							}
						}
					}
						break;	
					case 2 :
					{
						if(m_clock.year / 4 == 0)
						{
							if(++m_clock.day == 29)
							{
								m_clock.day = 0;
								if(++m_clock.month == 12)
								{
									m_clock.month = 0;
									if(++m_clock.year == 100)
										m_clock.year = 0;
								}
							}
						}
						else
						{
							if(++m_clock.day == 28)
							{
								m_clock.day = 0;
								if(++m_clock.month == 12)
								{
									m_clock.month = 0;
									if(++m_clock.year == 100)
										m_clock.year = 0;
								}
							}
						}
					}
						break;
					case 4 :
					case 6 :
					case 9 :
					case 11 :
					{
						if(++m_clock.day == 30)
						{
							m_clock.day = 0;
							if(++m_clock.month == 12)
							{
								m_clock.month = 0;
								if(++m_clock.year == 100)
									m_clock.year = 0;
							}
						}
					}
						break;
				}
			}
		}
	}
}



/**
 *本地时钟tick函数
 */

static void detection_buf_timeout_hander(void * p_context)
{	
	UNUSED_PARAMETER(p_context);
	static uint32_t m_clock_counter = 0;
	static uint32_t m_clock_counter1 = 0;
	if(m_clock_counter1 < 13)
	{
		m_clock_counter1++;
		return;
	}
	m_clock_counter1 = 0;
	
	//charge_check_handle();
	//factory_test();
	m_clock_counter++;

	if(m_clock_counter % 10)
	{
		nrf_gpio_pin_clear(MT2503_TX_PIN);
	}
	else if(m_clock_counter % 10)
	{
		nrf_gpio_pin_set(MT2503_TX_PIN);
	}
	#if 0//defined(RESET_TEST)
	{
		if(m_clock_counter >350)
		{
			NB_RESET_PIN_LOW;
		}
		else if(m_clock_counter >300)
			NB_RESET_PIN_HIGH;
	}
	#endif
#if 0
	local_clock_maintain();
//	if(m_clock_counter >= 288000) 			//8H	
	if(m_clock_counter >= 180000)			//5H

//	if(m_clock_counter%300 == 0) 			//30s for test
	{
		m_timer_battery = true;
	}
//	if(m_clock_counter >= 288000) 		m_clock_counter = 1;			//8h
	if(m_clock_counter >= 180000)		m_clock_counter = 1;	
#endif
#if 0//B4_UART
	if(m_clock_counter % 20 == 0)
	{
		nb_debug("sys_run--%d\r\n",GET_NB_WORK_MODE);
		switch(GET_NB_WORK_MODE)
		{
			case NB_REGISTER_APN_MODE:
				nb_debug(" %d\r\n",GET_REGISTER_STATE);
				break;
			
			case NB_REGISTER_NET_MODE:
				nb_debug(" %d\r\n",GET_NET_REGISTER_STATE);
				break;
			
			case NB_DATA_SEN_RECV_MODE:
				nb_debug(" %d\r\n",GET_NB_SEND_STEP);
				break;
			
			case NB_PSM_MODE:
				nb_debug(" %d\r\n",GET_NB_PSM_SEND_STEP);
				break;
			
			case NB_PSM_EXIT_MODE:
				nb_debug(" %d\r\n",GET_NB_PSM_EXIT_STEP);
				break;
			
			default:
				break;
		}
	}
#endif
	
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
void ble_evt_dispatch(ble_evt_t * p_ble_evt)//蓝牙事件安排派出
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


/**@brief Function for the Power manager.
 */
 void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}
extern void Storage_Module_init(void);
//蓝牙初始�?

void BleInit(pstorage_handle_t *handle)
{
	int number = 0;
	//uint8_t err_code =0;
	uint8_t persent = 80;
	pstorage_handle_t dest_block_id;
	ble_stack_init();
	
	device_num = 12345678;
	sprintf((char *)device_name,"NB_IOT2");
	
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


