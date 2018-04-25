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
#if defined(ADD_NRF_PWM_CTRL)
#include"pwm.h"
#endif


bool                                         m_blue_connect                     = false;         //蓝牙连接事件
ble_t                                        m_ble                              = {0};           //蓝牙

#define APP_GPIOTE_MAX_USERS                 3                                          /**< Maximum number of users of the GPIOTE handler. */

#define APP_TIMER_PRESCALER                  0
#define BUTTON_DETECTION_DELAY               APP_TIMER_TICKS(2000, APP_TIMER_PRESCALER)   /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define KEY_CHECK_MEAS_INTERVAL              APP_TIMER_TICKS(10, APP_TIMER_PRESCALER)

#define PUSH_KEL                             30

static app_timer_id_t                        m_key_timeout_id;


static uint16_t                              m_key_counter                      = 0;    /**< 按键按下时长.*/
static uint16_t                              m_button_free                      = 0;    /**< 按键释放时长.*/

bool                                         allShake = false;
bool                                         b_keypress_start_flag = false;

void key_timer_stop(void);
extern void StopKeyPressShake(void);
extern void Rd_MoteKeyPress(void);

//按键时长超时计数函数
static void key_timeout_handler(void * p_context)
{
	UNUSED_PARAMETER(p_context);
	
  if(nrf_gpio_pin_read(PUSH_KEL) == 0)  								//Enter键被按下
	{
		if(m_key_counter++ >= 200 && m_button_free < 5) //长按3s
		{
			m_key_counter = 0;
			m_button_free = 0;
			key_timer_stop();
#if NOR_USER_MOTOR			
			if(allShake == false)
			{
				uint8_t shakeSel[3] = {0x0F,0xFF,0xFF};
				Rd_MotoShakeTypeSet(true,SHAKE_OPPOINT,shakeSel,0);
				allShake = true;
			}
			else
			{
				uint8_t shakeSel[3] = {0x00,0x00,0x00};
				Rd_MotoShakeTypeSet(true,SHAKE_OPPOINT,shakeSel,0);
				allShake = false;
			}
#endif			
		}
	}
	else   												//按键释放
	{
		if(m_button_free++ >= 5)   //按键防抖
		{
			if(m_key_counter >= 10)
			{
				m_key_counter = 0;
			  m_button_free = 0;
			  key_timer_stop();
				
				if(allShake == false)
				{
				  uint8_t shakeSel[3] = {0x0F,0xFF,0xFF};
			    Rd_MotoShakeTypeSet(true,SHAKE_OPPOINT,shakeSel,0);
					allShake = true;
				}
				else
				{
				  uint8_t shakeSel[3] = {0x00,0x00,0x00};
			    Rd_MotoShakeTypeSet(true,SHAKE_OPPOINT,shakeSel,0);
					allShake = false;
				}
			}
		}
	}
}

/* 创建按键超时定时�?*/
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

	// Start application timers
	err_code = app_timer_start(m_key_timeout_id, KEY_CHECK_MEAS_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting application timers.
 */
void key_timer_stop(void)
{
	uint32_t err_code;

	// Stop key timers
	err_code = app_timer_stop(m_key_timeout_id);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling button events.
 *
 * @param[in]   pin_no   The pin number of the button pressed.
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
	if (button_action == APP_BUTTON_PUSH)
	{
		switch (pin_no)
		{
			case PUSH_KEL:
				if (b_keypress_start_flag == false)
				{
					//m_key_counter = 0;
					//m_button_free = 0;					
					//key_timer_start();
					SetKeyPressTime();
					//get_shake_time();
					Rd_MoteKeyPress();
					b_keypress_start_flag = true ;
				}
				else
				{
					StopKeyPressShake() ;
					b_keypress_start_flag = false ;
				}
				break;
			default:
				APP_ERROR_HANDLER(pin_no);
				break;
		}
	}
}

/**@brief Function for initializing the GPIOTE handler module.
 */
static void gpiote_init(void)
{
	APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}

/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
	static app_button_cfg_t buttons[] =
	{
		{PUSH_KEL, false, NRF_GPIO_PIN_PULLUP, button_event_handler}
	};

	APP_BUTTON_INIT(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY, false);
}

//按键控制及显示初始化函数
void key_display_init(void)
{
	gpiote_init();
	buttons_init();
	app_button_enable();
	key_timer_create();
}


int main(void)
{
	BleInit();            //蓝牙初始�?
	Rd_MotoInit();        //马达初始�?
	key_display_init();
	for(;;)
	{
		power_manage();
	//	Rd_MotoShake();
		Rd_MotoShakeStart();
	#if defined(FLASH_READ_WRITE)
		update_flash_data();
	#endif
		pwm_work_set();
	}
}




