/********************************************************************************
 *  Copyright (C), Shenzhen Bonten Health Techology Co.,Ltd.										*
 *																																							*
 *	Author: Kevin    Version : V1.2    Date: 2015/06/10													*
 *																																							*
 * 	FileName: 		 	global_variable.c																						*
 * 	Description:    //模块描述																									*
 * 	Function List:  //主要函数及其功能																					*
 *   	1. -------																																*
 * 	History:         																														*
 *   <author>   <time>   <version>   <desc>																			*
 *		kevin		 15/06/10	   V 1.2		1. 修复快速切屏出现花屏问�?							  *
 ********************************************************************************/

#include "RD_MotoControl.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "nrf_gpio.h"
#include "nrf6310.h"
#include "ble_service.h"


//#define DEBUG_MOTO_CTR
#define APP_TIMER_PRESCALER            0                   													 /**< Value of the RTC1 PRESCALER register. */
#define MOTO_NORAML_HANDLE                       APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)     /**normal ring */


#define 	MOTE_MAX_NUM		     7


#define BUZZER_NORMAL_HANDLE                  APP_TIMER_TICKS(25, APP_TIMER_PRESCALER)
#define BUZZER_ERROR_HANDLE                   APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)     /**normal ring */



static app_timer_id_t									 m_moto_handle_id;													   /**< 马达控制定时器ID.*/



/////////////////////////////////////////
uint16_t moto_cnt = 0;
static uint8_t   buzzer_time_cnt = 0;
static app_timer_id_t                        m_buzzer_timeout_id;








////////////////////////////////////////////
extern Receive_Cmd receivecmd;
extern uint8_t g_moto_flag ;




static void moto_timeout_handler(void * p_context)
{
	moto_cnt++;
	if(moto_cnt == 1)
	{
		//ֹͣ���Ķ���
		Moto_DISABLE();
		if(device_name_info.lock_state == CLOSE_LOCK)
			moto_timer_stop();
		
	}
	else if(moto_cnt == MOTO_TIMEOUT)		//15s ��ʱ
	{
		// ����
		Moto_B2A();
	}
	else if(moto_cnt >= (MOTO_TIMEOUT+1))		//������ʱ
	{
		//ֹͣ���Ķ���
		Moto_DISABLE();
		//����Ϊ����״̬
		device_name_info.lock_state = CLOSE_LOCK;
		moto_timer_stop();
	}
#if defined(DEBUG_MOTO_CTR)
	if(moto_cnt%30 == 0)
	{
		printf("moto_cnt:%d\r\n",moto_cnt);
	}
#endif


}


/* 创建lcd和马达震动超时定时器*/
static void moto_timer_create(void)
{
	uint32_t err_code = app_timer_create(&m_moto_handle_id,
															APP_TIMER_MODE_REPEATED,
															moto_timeout_handler);//moto_timeout_handler);
	APP_ERROR_CHECK(err_code);

}



/**@brief Function for starting application timers.
 */
 //
void moto_timer_start(void)
{
	moto_cnt = 0;
	uint32_t err_code = app_timer_start(m_moto_handle_id, MOTO_NORAML_HANDLE, NULL);
	APP_ERROR_CHECK(err_code);

#if dbg
	printf("moto_timer_start \r\n");
#endif	
}

/**@brief Function for starting application timers.
 */
void moto_timer_stop(void)
{
	uint32_t err_code = app_timer_stop(m_moto_handle_id);
	APP_ERROR_CHECK(err_code);
	Moto_DISABLE();
#if dbg
		printf("moto_timer_stop \r\n");
#endif	
}

void moto_start(unsigned char direction)
{
	moto_timer_stop();
	moto_timer_start();
	if(direction == TURN_POSITIVE)
		Moto_A2B();
	else
		Moto_B2A();
}

void buzzer_timer_stop(void)
{
	uint32_t err_code;
	err_code = app_timer_stop(m_buzzer_timeout_id);
	APP_ERROR_CHECK(err_code);
	
}

void buzzer_timeout_handler(void * p_context)
{
	buzzer_time_cnt++;	
	if(buzzer_time_cnt % 2 == 0)
	{
		nrf_gpio_pin_toggle(BUZZER_EN);
	}
	else if(buzzer_time_cnt > 5)
	{
		nrf_gpio_pin_clear(BUZZER_EN);
		buzzer_timer_stop();
	}
#if dbg
	printf("buzzer_time_cnt:%d \r\n",buzzer_time_cnt);
#endif
}
void buzzer_timer_create(void)
{
	uint32_t err_code;
	err_code = app_timer_create(&m_buzzer_timeout_id,APP_TIMER_MODE_REPEATED,buzzer_timeout_handler);
	APP_ERROR_CHECK(err_code);
}
void buzzer_timer_start(uint8_t mode)
{
	uint32_t err_code;
	buzzer_time_cnt = 0;
	if(mode == BUZZER_NORMAL)
		err_code = app_timer_start(m_buzzer_timeout_id, BUZZER_NORMAL_HANDLE, NULL);
	else
		err_code = app_timer_start(m_buzzer_timeout_id, BUZZER_ERROR_HANDLE, NULL);
	APP_ERROR_CHECK(err_code);

}

void buzzer_start(uint8_t mode)
{
	buzzer_timer_stop();
	buzzer_timer_start(mode);
}


//马达初始�?
void Rd_MotoInit(void)
{
	nrf_gpio_cfg_output(MOTO_EN);
	nrf_gpio_cfg_output(IA_EN);
	nrf_gpio_cfg_output(IB_EN);
	nrf_gpio_pin_clear(MOTO_EN);
	nrf_gpio_pin_clear(IA_EN);
	nrf_gpio_pin_clear(IB_EN);
	
	nrf_gpio_cfg_output(BUZZER_EN);
	nrf_gpio_pin_clear(BUZZER_EN);

	nrf_gpio_cfg_input(MOTO_STOP_PIN,(nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pullup);
	nrf_gpio_cfg_input(MT2503_HEART_PIN,(nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pulldown);

	Moto_B2A();
	nrf_delay_ms(50);
	Moto_DISABLE();	
	moto_timer_create();

}

