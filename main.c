#include "ble_service.h"
#include "data_struct.h"
#include "ble_trans.h"
#include "nrf_delay.h"
#include "nrf6310.h"
#include "RD_MotoControl.h"
#include "RD_BatteryCharge.h"
#include "app_timer.h"
#include "app_button.h"
#include "app_gpiote.h"
#include "simple_uart.h"
#include "ble_flash.h"
#include "nrf_nvmc.h"
#include "pstorage.h"
#include "pstorage_platform.h"
#include "pstorage_lhat.h"
#include "ble_exdevice.h"
bool                                         m_blue_connect                     = false;         //蓝牙连接事件
ble_t                                        m_ble                              = {0};           //蓝牙





#define APP_GPIOTE_MAX_USERS                 3                                          /**< Maximum number of users of the GPIOTE handler. */

#define APP_TIMER_PRESCALER                  0
#define BUTTON_DETECTION_DELAY               APP_TIMER_TICKS(5, APP_TIMER_PRESCALER)   /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#if defined(_SUPPORT_BM250_)
#define KEY_CHECK_MEAS_INTERVAL              APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)
#else
#define KEY_CHECK_MEAS_INTERVAL              APP_TIMER_TICKS(2000, APP_TIMER_PRESCALER)
#endif
#define BUZZER_MEAS_INTERVAL              APP_TIMER_TICKS(400, APP_TIMER_PRESCALER)
#define GPIO_MEAS_INTERVAL              APP_TIMER_TICKS(1200, APP_TIMER_PRESCALER)




static app_timer_id_t                        m_key_timeout_id;
static app_timer_id_t                        m_buzzer_timeout_id;
static app_timer_id_t                        m_gpio_timeout_id;


//////////////////////////////////
extern uint16_t moto_cnt;
extern Receive_Cmd receivecmd;
uint8_t g_lock_flag = 2;						//锁状态 1 锁住，0 开锁 ，2 初始值，
uint8_t g_moto_flag = 0;						//用于判断电机正反转1 正转   2 反转
uint8_t g_heart_flag = 0;
uint8_t g_mt2503_err = 0;
uint8_t g_devicename_set = 0;
uint8_t g_bat_present = 0;
extern unsigned char device_name[20];
uint8_t g_receicve_cmd = 0;					//用于接收到蓝牙开锁
bool g_vibrate_status = false;					//震动状态
unsigned int m_time_count = 0;					//用于定时修改蓝牙名称
extern bool m_timer_battery ;					//定时刷新电池百分比
extern ble_system_clock_t m_clock ;				//当前时间
extern bool n_trade_type;
extern bool n_trade_type_46;
extern bool	b_beef_flag;
//extern ble_system_clock_t  	open_clock;			//开锁时间
//extern ble_system_clock_t 	close_clock;		//关锁时间
//extern char pReceive_buff[];
//char ID[6] = "55555";
char ID[6] = "ZH200";
///////////////////////////////////
char LOGO[3] = "AB";

//////////////////////////////////
extern void key_timer_start(void);
extern void key_timer_stop(void);

uint32_t g_number = 0;



extern void ble_evt_dispatch(ble_evt_t * p_ble_evt);


/*
uint32_t g_number = 0;
void gen_DeviceName(void)
{
	uint8_t len = 0;
	uint8_t persent = 0;
	uint8_t status = 0;;
	if(!g_devicename_set)	return ;

	len += sprintf((char *)device_name,"%s",LOGO);
	len += sprintf((char *)device_name+len,"%s",ID);
	persent = battery_level_transform();
	if(persent>=100)
		persent = 99;
	len += sprintf((char *)device_name+len,"%02d",persent);
	//len += sprintf((char *)device_name+len,"%02d%02d",open_clock.hour,open_clock.minute);
	//len += sprintf((char *)device_name+len,"%02d%02d",close_clock.hour,close_clock.minute);
	status |= g_lock_flag<<0;			//锁状态
	status |= 1<<1;						//电机状态
//	status |= 1<<2;						//按键状态
	status &= ~(1<<2);					//1关锁震动，0关锁正常
	status &= 0x07;
	len += sprintf((char *)device_name+len,"%01d",status);
#if dbg
	//printf("device_name = %s\r\n",device_name);
#endif
//	advertising_stop();
	gap_params_init();
	advertising_init();
//	advertising_start();

}
*/


//static void key_timeout_handler(void * p_context)
//{
//	uint32_t err_code = 0;
//	static uint32_t timer_cnt = 0;
//	UNUSED_PARAMETER(p_context);
//	timer_cnt++;
//	if(timer_cnt<=150)	return ;
//	else
//	{
//		timer_cnt = 0;
//		key_timer_stop();
//		if(!g_heart_flag)
//		{
//			g_mt2503_err++;
//#if dbg
//			//printf("mt2503 no heart bit\r\n");
//#endif
//		}
//		else
//		{
//			g_mt2503_err = 0;
//			g_heart_flag = 0;
//		}
//		if(g_mt2503_err>=3)
//		{
//#if dbg
//			//printf("mt2503 reset\r\n");
//#endif
//			g_mt2503_err = 0;
//			g_heart_flag = 0;
//			MT2503_Reset();
//		}
//		key_timer_start();
//	}
//}


static void key_timeout_handler(void * p_context)
{

	static uint32_t timer_cnt = 0;
#if defined(_SUPPORT_BM250_)
	static uint8_t buzzer_cnt = 0,alarm_cnt = 0,vib_stauts = 0;
	static uint8_t time_cnt1 = 0;
#endif
	UNUSED_PARAMETER(p_context);
#if defined(_SUPPORT_BM250_)
	time_cnt1++;
	if(vib_stauts)
	{
		//报警
		if(buzzer_cnt==0&&alarm_cnt<5)
		{
			buzzer_cnt = 1;
			alarm_cnt++;
			nrf_gpio_pin_set(BUZZER_EN);
		}
		else
		{
			buzzer_cnt++;
			nrf_gpio_pin_clear(BUZZER_EN);
			if(buzzer_cnt>=3)
				buzzer_cnt = 0;
			if(alarm_cnt >= 5)
			{
				alarm_cnt = 0;
				buzzer_cnt = 0;
				vib_stauts = 0;
			}

		}

	}

	if(time_cnt1<20)
	{
		return;
	}
#endif
	timer_cnt++;
#if defined(_SUPPORT_BM250_)
	time_cnt1 = 0;
	if(timer_cnt%3 == 0)
	{
		g_vibrate_status = bma250_get_vibrate_status();
		if(g_vibrate_status&&(g_lock_flag==0))
		{
			vib_stauts	= g_vibrate_status;
		}
	}
#endif

	if(b_beef_flag==true)
	{
		if(timer_cnt>=50)
		{
			b_beef_flag=false ;
			nrf_gpio_pin_clear(BUZZER_EN);
		}
	}
	if(timer_cnt%150==0)			//5min
	{

		key_timer_stop();
		if(!g_heart_flag)
		{
			g_mt2503_err++;
#if dbg
			//printf("mt2503 no heart bit\r\n");
#endif
		}
		else
		{
#if dbg
			//printf("receive handle bit\r\n");
#endif
			g_mt2503_err = 0;
			g_heart_flag = 0;
		}
		if(g_mt2503_err>=5)
		{
#if dbg
			//printf("mt2503 reset\r\n");
#endif
			g_mt2503_err = 0;
			g_heart_flag = 0;
			MT2503_Reset();
		}
		key_timer_start();
	}
	else if(timer_cnt%330==0)		//11min
	{
		timer_cnt = 0;
		g_bat_present = battery_level_transform();
		if(g_bat_present>=100)
			g_bat_present = 99;
		else if(g_bat_present<=0)
			g_bat_present = 0;

	}
	else
	{
		return ;
	}
}



/* 创建按键超时定时器*/
static void key_timer_create(void)
{
	uint32_t err_code;

	//按键检测定时器
	err_code = app_timer_create(&m_key_timeout_id,
															APP_TIMER_MODE_REPEATED,
															key_timeout_handler);
	APP_ERROR_CHECK(err_code);

}

/**@brief Function for starting application timers.
 */
void key_timer_start(void)
{
	uint32_t err_code;

	err_code = app_timer_start(m_key_timeout_id, KEY_CHECK_MEAS_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);
#if dbg
	//printf("key_timer_start \r\n");

#endif
}

/**@brief Function for starting application timers.
 */
void key_timer_stop(void)
{
	uint32_t err_code;

	// Stop key timers
	err_code = app_timer_stop(m_key_timeout_id);
	APP_ERROR_CHECK(err_code);

#if dbg
	//printf("key_timer_stop\r\n");
#endif
}


void buzzer_timer_stop(void)
{
	uint32_t err_code;
	err_code = app_timer_stop(m_buzzer_timeout_id);
	APP_ERROR_CHECK(err_code);

}

void buzzer_timeout_handler(void * p_context)
{
	nrf_gpio_pin_clear(BUZZER_EN);
	Moto_DISABLE();
	buzzer_timer_stop();

}
void buzzer_timer_create(void)
{
	uint32_t err_code;
	err_code = app_timer_create(&m_buzzer_timeout_id,APP_TIMER_MODE_REPEATED,buzzer_timeout_handler);
	APP_ERROR_CHECK(err_code);
}
void buzzer_timer_start(void)
{
	uint32_t err_code;
	err_code = app_timer_start(m_buzzer_timeout_id, BUZZER_MEAS_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);

}

void gpio_timer_start(void)
{
	uint32_t err_code;
	err_code = app_timer_start(m_gpio_timeout_id, GPIO_MEAS_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);
}

void gpio_timer_stop(void)
{
	uint32_t err_code;
	err_code = app_timer_stop(m_gpio_timeout_id);
	APP_ERROR_CHECK(err_code);

}

void gpio_timeout_handler(void * p_context)
{
	//nrf_gpio_pin_clear(MT2503_GOTO_UNLOCK_PIN);
	gpio_timer_stop();
}
void gpio_timer_create(void)
{
	uint32_t err_code;
	err_code = app_timer_create(&m_gpio_timeout_id,APP_TIMER_MODE_REPEATED,gpio_timeout_handler);
	APP_ERROR_CHECK(err_code);
}




static void gpiote_init(void)
{
	APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}


////////////////////////


#if defined(SECOND_30_OUT_NUOPEN_TO_LOCK)
void open_lock(void)
{

	g_lock_flag = true;
	//				nrf_delay_ms(5);
#if dbg
	//printf("OPEN LOCK\r\n");
#endif
	//memcpy(&open_clock,&m_clock,sizeof(ble_system_clock_t));
	if(g_receicve_cmd)
	{
		nrf_gpio_pin_clear(BUZZER_EN);
		g_receicve_cmd = 0;
		moto_timer_stop();
	}
	//tudo t0 2503
	//send_data_to_phone(0x01,g_bat_present,0x00,receivecmd.cipher);
	//mt2503_unlock_noticfy();
	n_trade_type = true ;
	send_data_to_phone(0x50,NULL);
#if defined(SECOND_30_OUT_NUOPEN_TO_LOCK)
	open_delay_check.start = false;
#endif
	//gen_DeviceName();
	/////////////////////////////////////////
	{
		//unsigned char nn_cmd = 0x42;
		//send_ble_data_to_app_return(nn_cmd,pReceive_buff);
	}
	return ;
}

void close_lock(void)
{
#if dbg
	//printf("CLOSE LOCK\r\n");
#endif
	//				nrf_delay_ms(5);
	//memcpy(&close_clock,&m_clock,sizeof(ble_system_clock_t));
	//mt2503_lock_noticfy();
	Moto_ENABLE();
	Moto_B2A();
	g_moto_flag = 2;
	moto_cnt = 0;
	nrf_gpio_pin_clear(BUZZER_EN);
	moto_timer_stop();
	moto_timer_start();
	buzzer_timer_stop();
	buzzer_timer_start();
	g_lock_flag = false;
	n_trade_type = false ;
	send_data_to_phone(0x50,NULL);
	//gen_DeviceName();
	n_trade_type_46=true;
	return;
}
#endif

#if defined(SECOND_30_OUT_NUOPEN_TO_LOCK)
static void mt2503_heart_event_handler(uint8_t pin_no, uint8_t button_action)
{
    uint32_t pins_state = NRF_GPIO->IN;
	switch (pin_no)
	{
		case MOTO_STOP_PIN:
		{
		#if defined(KEY_IO_SET_PULL_DOWN)
			if(!((pins_state>>MOTO_STOP_PIN)&0x01)&&g_lock_flag == false)
		#else
			if(((pins_state>>MOTO_STOP_PIN)&0x01)&&g_lock_flag == false)
		#endif
			{
				open_lock();
			}
		#if defined(KEY_IO_SET_PULL_DOWN)
			else if(((pins_state>>MOTO_STOP_PIN)&0x01)&&g_lock_flag == true)
		#else
			else if(!((pins_state>>MOTO_STOP_PIN)&0x01)&&g_lock_flag == true)
		#endif
			{
				close_lock();
			}
		}
		default:break;
	}
}

#else
static void mt2503_heart_event_handler(uint8_t pin_no, uint8_t button_action)
{
    uint32_t pins_state = NRF_GPIO->IN;
	switch (pin_no)
	{
		case MOTO_STOP_PIN:
		{
		#if defined(KEY_IO_SET_PULL_DOWN)
			if(!((pins_state>>MOTO_STOP_PIN)&0x01)&&g_lock_flag == false)
		#else
			if(((pins_state>>MOTO_STOP_PIN)&0x01)&&g_lock_flag == false)
		#endif
			{

				g_lock_flag = true;
//				nrf_delay_ms(5);
#if dbg
				//printf("OPEN LOCK\r\n");
#endif
				//memcpy(&open_clock,&m_clock,sizeof(ble_system_clock_t));
				if(g_receicve_cmd)
				{
					nrf_gpio_pin_clear(BUZZER_EN);
					g_receicve_cmd = 0;
					moto_timer_stop();
				}
				//tudo t0 2503
				//send_data_to_phone(0x01,g_bat_present,0x00,receivecmd.cipher);
				//mt2503_unlock_noticfy();
				n_trade_type = true ;
				//gen_DeviceName();
				/////////////////////////////////////////
				{
					//unsigned char nn_cmd = 0x42;
					//send_ble_data_to_app_return(nn_cmd,pReceive_buff);
				}
				return ;
			}
		#if defined(KEY_IO_SET_PULL_DOWN)
			else if(((pins_state>>MOTO_STOP_PIN)&0x01)&&g_lock_flag == true)
		#else
			else if(!((pins_state>>MOTO_STOP_PIN)&0x01)&&g_lock_flag == true)
		#endif
			{
#if dbg
				//printf("CLOSE LOCK\r\n");
#endif
//				nrf_delay_ms(5);
				//memcpy(&close_clock,&m_clock,sizeof(ble_system_clock_t));
				//mt2503_lock_noticfy();
				Moto_ENABLE();
				Moto_B2A();
				g_moto_flag = 2;
				moto_cnt = 0;
				nrf_gpio_pin_clear(BUZZER_EN);
				moto_timer_stop();
				moto_timer_start();
				buzzer_timer_stop();
				buzzer_timer_start();
				g_lock_flag = false;
				n_trade_type = false ;
				send_data_to_phone(0x50,NULL);
				//gen_DeviceName();
				n_trade_type_46=true;
				return;
			}
		}
		default:break;
	}
}
#endif

extern Receive_Cmd receivecmd;


static void buttons_init(void)
{
#if 0
	static app_button_cfg_t buttons[] =
	{
		{MT2503_HEART_PIN, APP_BUTTON_ACTIVE_HIGH, GPIO_PIN_CNF_PULL_Pulldown, mt2503_heart_event_handler},
		{MOTO_STOP_PIN, APP_BUTTON_ACTIVE_LOW, GPIO_PIN_CNF_PULL_Pullup, mt2503_heart_event_handler}
	};
	APP_BUTTON_INIT(buttons,2, BUTTON_DETECTION_DELAY, false);
#else
	static app_button_cfg_t buttons[] =
	{
	#if defined(USE_KEY_HIGH_OK)
		{MOTO_STOP_PIN, APP_BUTTON_ACTIVE_HIGH, NRF_GPIO_PIN_PULLDOWN, mt2503_heart_event_handler}
	#else
		{MOTO_STOP_PIN, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, mt2503_heart_event_handler}
	#endif

	};
	APP_BUTTON_INIT(buttons,1, BUTTON_DETECTION_DELAY, false);
#endif
}

void EXIT_KEY_Init(nrf_gpio_pin_sense_t s);
void mt2503_heart_exit_init(void);

//按键控制及显示初始化函数
void key_display_init(void)
{
	gpiote_init();
	buttons_init();
//	mt2503_heart_exit_init();
//	EXIT_KEY_Init((nrf_gpio_pin_pull_t)GPIO_PIN_CNF_SENSE_Low);
	app_button_enable();
	key_timer_create();			//检测2503心跳
	key_timer_start();
	buzzer_timer_create();		//防止蜂鸣器一直响

}


/////////////////////////////////////////////////

//extern unsigned int device_num;
extern unsigned char device_name[20];
unsigned char  my_flag = 0;
pstorage_module_param_t module_param;
pstorage_handle_t handle;
static void my_cb(pstorage_handle_t * handle,uint8_t op_code,uint32_t result,
uint8_t * p_data, uint32_t data_len)
{
//	char tmp[20] = {0};
	switch(op_code)
	{
	case PSTORAGE_UPDATE_OP_CODE:
		if (result == NRF_SUCCESS)
		{
			my_flag = 1; //当 flash update 完成后置位标志。 Main 函数中便可以读 flash 数据了
#if defined(DEBUG_UART)
			printf("store success!\r\n");
#endif
		//	NVIC_SystemReset();
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
#if defined(CHANG_PROTOCAL)
	module_param.block_count = 4;
	module_param.block_size = 64;
#else
	module_param.block_count = 2;
	module_param.block_size = 16;
#endif
	module_param.cb = my_cb;
	pstorage_init();
	pstorage_register(&module_param, &handle);

	nrf_delay_ms(100);
//	pstorage_block_identifier_get(&handle, 1, &dest_block_id);
//	pstorage_update(&dest_block_id, (uint8_t *)&my_buff, 4, 0);
#if dbg
//	//printf("Storage_Module_init\r\n");
#endif

}
char name[100] = {0};
int BlueDeviceName_Get(void)
{
	unsigned char i = 0;
	pstorage_handle_t dest_block_id;
	nrf_delay_ms(50);
	pstorage_block_identifier_get(&handle,1,&dest_block_id);
	pstorage_load((uint8_t *)name,&dest_block_id,8,0);
	for(i = 0;i<strlen(name);i++)
	{
			if(name[i]>='0'&&name[i]<='9')
				continue;
			else
			{
#if dbg
				//printf("Device name :%s\r\n",name);
				//printf("format error\r\n");
#endif
				memset(name,0,sizeof(name));
				return -1;
			}

	}
#if dbg
	//printf("Device name :%s\r\n",name);
#endif
	return 0;

}

void mt2503_heart_exit_init(void)
{
//	nrf_gpio_pin_pull_t config =  GPIO_PIN_CNF_PULL_Pulldown;
//	nrf_gpio_pin_sense_t sense = GPIO_PIN_CNF_SENSE_High;
	//配置四个按键最为输入和上拉，并
	//nrf_gpio_cfg_sense_input(MT2503_HEART_PIN,config, sense);

}



#if 0
void test_encrpty_data(void)
{
	uint8_t i;
	unsigned char key[16] = "2L468R7C5Q9PEA6V";
	unsigned char source[16] = "2L468R7C5Q9PEA6V";
	unsigned char scret_key[16] = {0};
	unsigned char decryp_data[16] = {0};
	AES_ECB_encrypt(source , key, scret_key, 16);
	printf("scret_key\r\n");
	for(i=0;i<16;i++)
	{
		printf("%x ",scret_key[i]);

	}
	printf("\r\n");

	AES_ECB_decrypt(scret_key , key, decryp_data, 16);

	printf("decryp_data\r\n");
	for(i=0;i<16;i++)
	{
		printf(" %c",decryp_data[i]);

	}
	printf("\r\n");

}
#endif


/****************************************/
uint8_t g_last_status = 0;
extern unsigned char RXdata[128];
extern unsigned char RxDataLen;
extern unsigned short USART_RX_STA ;
pstorage_handle_t   block_id;
extern unsigned char mac_address[16];

//uint8_t                   my_flag = 0;
//uint8_t                   my_buff[8]={'A', 'B', 'C', 'D', 'E', 'F', 'G','H'};

/***************************************/
int main(void)
{
#if defined(_SUPPORT_BM250_)
	uint8_t bma250_id = 0;
#endif

	for(uint8_t num = 0; num< 32 ; num++)
	DISABLE_PIN_FN(num);

#if dbg
	simple_uart_config(MT2503_TX_PIN,MT2503_RX_PIN,false);
#else
	simple_uart_config(MT2503_TX_PIN,MT2503_RX_PIN,false);
#endif
#if 1//defined(DEBUG_UART)
	printf("uart init\r\n");
	printf("%s--%s\n", __DATE__, __TIME__);
#endif

	BleInit(&handle);          //蓝牙初始化
	Rd_MotoInit();      	   //马达初始化
	key_display_init();
	//EXIT_KEY_Init((nrf_gpio_pin_pull_t)GPIO_PIN_CNF_SENSE_Low);

	if(g_devicename_set)
	{

	#if dbg
		//printf("!!!!!!BLE SET SUCCESS!!!!!!! \r\n");
	#else
		////printf("!!!!!!BLE SET SUCCESS!!!!!!! \r\n");
		//Uart_Deinit(MT2503_RX_PIN,MT2503_TX_PIN);
	#endif
	}
#if defined(_SUPPORT_BM250_)
	TWI_Init();
	bma250_id = BMA250_ReadID();
#endif
	nrf_delay_ms(10000);
	ble_send_data_to_mt2503_uart(2,NULL);
	Uart_Deinit(MT2503_TX_PIN,MT2503_RX_PIN);

///////////////////////////////////////////////
#if defined(DEBUG_UART)
	//printf("\r\n main start up\r\n");
#endif

	for(;;)
	{
		power_manage();
	#if defined(ADD_HELP_FUNCV_ORDER)
		receive_uart_parse();
	#endif	

	#if defined(CHANGE_UART_DEAL_PLACE)
		RecvDataDeal();
	#endif
	#if defined(FLASH_READ_WRITE)
		update_flash_data();
	#endif

//	test_encrpty_data();

	
	}
}




