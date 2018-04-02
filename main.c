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
//#include "api_md5.h"

#if defined(ADD_NFC_CARD_FUNCV)
#include "nfc_ms522.h"
#endif
#if defined(ADD_NFC_CARD_FUNCV2)
#include "api_nfc.h"
#endif

bool                                         m_blue_connect                     = false;         //蓝牙连接事件
ble_t                                        m_ble                              = {0};           //蓝牙

#define APP_GPIOTE_MAX_USERS                 3                                          /**< Maximum number of users of the GPIOTE handler. */

#define APP_TIMER_PRESCALER                  0
#define BUTTON_DETECTION_DELAY               APP_TIMER_TICKS(5, APP_TIMER_PRESCALER)   /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

///////////////////////////


//////////////////////////////////
extern Receive_Cmd receivecmd;
uint8_t g_moto_flag = 0;						//用于判断电机正反�? 正转   2 反转
uint8_t g_heart_flag = 0;
uint8_t g_mt2503_err = 0;
uint8_t g_bat_present = 0;
extern unsigned char device_name[20];
uint8_t g_receicve_cmd = 0;					//用于接收到蓝牙开�?
bool g_vibrate_status = false;					//震动状�?
unsigned int m_time_count = 0;					//用于定时修改蓝牙名称
extern ble_system_clock_t m_clock ;				//当前时间
extern ble_system_clock_t  	open_clock;			//开锁时�?
extern ble_system_clock_t 	close_clock;		//关锁时间
//char ID[6] = "55555";
char ID[6] = "ZH200";
///////////////////////////////////
char LOGO[3] = "AB";

//////////////////////////////////
extern void key_timer_start(void);
extern void key_timer_stop(void);




extern void ble_evt_dispatch(ble_evt_t * p_ble_evt);

uint32_t g_number = 0;

static void gpiote_init(void)
{
	APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}


////////////////////////

void update_bt_information(uint8_t lock_state,uint32_t ride_time)
{
	device_name_info.lock_state = lock_state;
	device_name_info.card_ride.ride_time = ride_time;
#if dbg
	printf("ride_time:%d--lock_state:%d\r\n",ride_time,lock_state);
#endif
	gen_DeviceName();
}

void sys_close_lock(void)
{
#if dbg
	printf("SYS_CLOSE_LOCK\r\n");			
#endif
	Moto_B2A();
	nrf_gpio_pin_clear(BUZZER_EN);
	moto_start();
	buzzer_start(BUZZER_NORMAL);
	memcpy(&last_card_ride_info,&device_name_info.card_ride,sizeof(CARD_RIDE_INFO));
#if defined(FLASH_READ_WRITE)
	memcpy(flash_save_data,&last_card_ride_info,4);
#endif
	update_bt_information(CLOSE_LOCK,get_m_clock_counter());
	send_data_to_phone(TRADE_RETURN,device_name_info.card_ride);
	return;
}

void sys_open_lock(void)
{
#if dbg
	printf("SYS_OPEN_LOCK\r\n");
#endif
	nrf_gpio_pin_clear(BUZZER_EN);
	moto_timer_stop();	
	update_bt_information(OPEN_LOCK,0);
	send_data_to_phone(OPEN_SUCCES,device_name_info.card_ride);
	return ;
}



static void mt2503_heart_event_handler(uint8_t pin_no, uint8_t button_action)
{
    uint32_t pins_state = NRF_GPIO->IN;
	switch (pin_no)
	{
		case MT2503_HEART_PIN:
		{	
			if(!((pins_state>>MT2503_HEART_PIN)&0x01))
			{
				g_heart_flag = 1;
				send_data_to_phone_time();
			//	buzzer_start(BUZZER_NORMAL);
			}
		#if dbg
			printf("mt2503 heart bit\r\n");
		#endif
		}
			break;
		case MOTO_STOP_PIN:
		{
			if((pins_state>>MOTO_STOP_PIN)&0x01)
			{
				sys_open_lock();	
			}
			else if(!((pins_state>>MOTO_STOP_PIN)&0x01))
			{
				sys_close_lock();
			}
		}
		break;
		
		default:break;
	}


}


extern Receive_Cmd receivecmd;

#if defined(ADD_NFC_CARD_FUNCV)
#define IRQ_PIN_NUM	3
#else
#define IRQ_PIN_NUM	2
#endif
static void buttons_init(void)
{
	static app_button_cfg_t buttons[] =
	{
		{MT2503_HEART_PIN, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, mt2503_heart_event_handler},
		{MOTO_STOP_PIN, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, mt2503_heart_event_handler}
	#if defined(ADD_NFC_CARD_FUNCV)
		,{NFC_IRQ_PIN, APP_BUTTON_ACTIVE_HIGH, NRF_GPIO_PIN_PULLDOWN, mt2503_heart_event_handler}
	#endif
	};
	APP_BUTTON_INIT(buttons,IRQ_PIN_NUM, BUTTON_DETECTION_DELAY, false);
}

void EXIT_KEY_Init(nrf_gpio_pin_sense_t s);
void mt2503_heart_exit_init(void);

//按键控制及显示初始化函数
void key_display_init(void)
{
	gpiote_init();
	buttons_init();
	app_button_enable();
	buzzer_timer_create();		//防止蜂鸣器一直响
#if defined(ADD_NFC_CARD_FUNCV)||defined(ADD_NFC_CARD_FUNCV2)
	nfc_card_init();
#endif



}


/////////////////////////////////////////////////

extern unsigned char device_name[20];

void mt2503_heart_exit_init(void)
{
	nrf_gpio_pin_pull_t config =  NRF_GPIO_PIN_PULLDOWN; 
	nrf_gpio_pin_sense_t sense = NRF_GPIO_PIN_SENSE_HIGH;
	//配置四个按键最为输入和上拉，并
	nrf_gpio_cfg_sense_input(MT2503_HEART_PIN,config, sense);

}
void EXIT_KEY_Init(nrf_gpio_pin_sense_t s)
{
	//配置上拉�?sense
	nrf_gpio_pin_pull_t config =  NRF_GPIO_PIN_PULLDOWN; 
	nrf_gpio_pin_sense_t sense = s;
	nrf_gpio_cfg_sense_input(MOTO_STOP_PIN,config, sense);
	NRF_GPIOTE->INTENSET =(uint32_t)1 << 31;
	NVIC_SetPriority(GPIOTE_IRQn, APP_IRQ_PRIORITY_LOW);
	NVIC_ClearPendingIRQ(GPIOTE_IRQn);
	NVIC_EnableIRQ(GPIOTE_IRQn);
}


///////////////

pstorage_handle_t		handle_mac;
pstorage_module_param_t param_mac;
pstorage_handle_t block_handle_mac;
uint8_t           dest_data[32] = {0};


static void example_cb_handler(pstorage_handle_t  * handle,
                               uint8_t              op_code,
                               uint32_t             result,
                               uint8_t            * p_data,
                               uint32_t             data_len)
{
	switch(op_code)
	{
		case PSTORAGE_LOAD_OP_CODE:
		   if (result == NRF_SUCCESS)
		   {
		#if dbg
			printf("PSTORAGE_LOAD_OP_CODE SUCCESS \r\n");
		#endif
		   }
		   else
		   {
			   // Store operation failed.
		   }
		   // Source memory can now be reused or freed.
		   break;		
		case PSTORAGE_STORE_OP_CODE:
		   if (result == NRF_SUCCESS)
		   {
		#if dbg
				printf("PSTORAGE_STORE_OP_CODE SUCCESS \r\n");
		#endif

		   }
		   else
		   {
			   // Update operation failed.
		   }
		   break;
	   case PSTORAGE_CLEAR_OP_CODE:
		   if (result == NRF_SUCCESS)
		   {

		#if dbg
			printf("PSTORAGE_CLEAR_OP_CODE SUCCESS \r\n");
		#endif

		   }
		   else
		   {
			   // Clear operation failed.
		   }
		   break;
	}
}


void xq_pstorageinit(void)
{
	uint32_t retval;
	retval = pstorage_init();
	if(retval == NRF_SUCCESS)
	{

	}
	else
	{

	}	
}


void xq_pstorageregister(void)
{
	uint32_t retval = 0;
		  
	param_mac.block_size  = 16;
	param_mac.block_count = 20;
	param_mac.cb		  = example_cb_handler;	
	retval = pstorage_register(&param_mac, &handle_mac);
	if (retval == NRF_SUCCESS)
	{

	}
	else
	{

	}
	retval = pstorage_block_identifier_get(&handle_mac, 2, &block_handle_mac);
	if (retval == NRF_SUCCESS)
	{
	}
	else
	{
	}
}
void xq_pstoragestore(uint8_t cmd,uint8_t *data)
{
	uint32_t retval;
	if(!data)	return;
	if(cmd == 1)
	{
		retval = pstorage_store(&block_handle_mac, data, 8, 0);
		nrf_delay_ms(100);
		if (retval == NRF_SUCCESS)
		{

		}
		else
		{

		}
	}
	else
	{}

}

void xq_pstorage_clear(uint8_t cmd)
{
	uint32_t retval;
	if(cmd == 1)
	{
		retval = pstorage_clear(&block_handle_mac, 16);
		nrf_delay_ms(500);
		if (retval == NRF_SUCCESS)
		{
		}
		else
		{
		}
	}
	
}


void xq_pstorageload(void)
{
	uint32_t retval;
	retval = pstorage_load(dest_data, &block_handle_mac,8, 0);
	nrf_delay_ms(100);
	if (retval == NRF_SUCCESS)
	{

		if(dest_data[0] == 0x55)
		{

		}
		else
		{		

		}

	}
	else
	{
		// Failed to load, take corrective action.
	}
}


void xq_pstorage_init(void)
{
	xq_pstorageinit();
	xq_pstorageregister();
	nrf_delay_ms(500);
	xq_pstorageload();
}


/***************************************/
int main(void)
{
	for(uint8_t num = 0; num< 32 ; num++) 
	DISABLE_PIN_FN(num);
	simple_uart_config(MT2503_TX_PIN,MT2503_RX_PIN,false);
#if B4_UART
	printf("uart init\r\n");
#endif	
	BleInit();         
	Rd_MotoInit();      	   
	key_display_init();
#if !defined(DEBUG_UART)
	Uart_Deinit(MT2503_RX_PIN,MT2503_TX_PIN);
#endif
	for(;;)
	{
		power_manage();
//		gen_DeviceName();
	#if defined(FLASH_READ_WRITE)
		update_flash_data();
	#endif
	
	
	}
}



