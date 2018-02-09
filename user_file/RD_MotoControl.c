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
#include "data_struct.h"
#include "global_var.h"
#include "ble_service.h"

#define APP_TIMER_PRESCALER            0                   													 /**< Value of the RTC1 PRESCALER register. */
#define MOTO_HANDLE_MEAS_INTERVAL      APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)     /**< Battery level measurement interval (ticks). */

#define 	MOTE_MAX_NUM		7



static app_timer_id_t									 m_moto_handle_id;													   /**< 马达控制定时器ID.*/

bool n_trade_type		= false ;


/////////////////////////////////////////

extern Receive_Cmd receivecmd;
uint16_t moto_cnt = 0;
extern uint8_t g_lock_flag;
extern uint8_t g_moto_flag ;
//void BLE_HEARTBit(void);


#if 0
#define MOTO_TIME 			20									//200ms
#define MOTO_TIMEOUT	    300								//30S超时反转
#define LOCK_BUZZER_CNT     6									//关锁蜂鸣器快速响三下
#define UNLOCK_BUZZER_CNT 	8									//开锁响4�?
#else
#define MOTO_TIME 			1									//500ms
#define MOTO_TIMEOUT	    300*2								//30S超时反转
#define LOCK_BUZZER_CNT     6									//关锁蜂鸣器快速响三下
#define UNLOCK_BUZZER_CNT 	8  									//开锁响4�?
#endif





////////////////////////////////////////











static void moto_bra_timeout_handler(void * p_context)
{
	UNUSED_PARAMETER(p_context);
#if 0
		//printf("1S TIMER,lock = %d,g_moto_flag = %d\r\n",g_lock_flag,g_moto_flag);
#endif	
	moto_cnt++;
	if(g_moto_flag == 1)		//开锁正�?
	{
		if(moto_cnt<=UNLOCK_BUZZER_CNT) 	
		{
			nrf_gpio_pin_toggle(BUZZER_EN);
		}	
		if(moto_cnt>=MOTO_TIME&&moto_cnt<MOTO_TIMEOUT)			//1.2S ,30s
		{		
			Moto_DISABLE();
			if(g_lock_flag == 1)
			{
				moto_cnt = 0;
				g_moto_flag = 0;
				nrf_gpio_pin_clear(BUZZER_EN);
				moto_timer_stop();
				//send ble ,modify name 			
				return ;
			}
		}
		/*else if(moto_cnt>= MOTO_TIMEOUT)			//30s
		{
#if dbg
		//printf("moto_cnt = %d,g_lock_flag = %d,g_moto_flag = %d\r\n",moto_cnt,g_lock_flag,g_moto_flag);
#endif
			Moto_ENABLE();
			Moto_B2A();
			g_moto_flag = 2;		//反转
			moto_cnt = 0;
			return ;
		}		*/
	}
	else if(g_moto_flag == 2)			//关锁反转
	{
		if(moto_cnt<=LOCK_BUZZER_CNT)		
		{
			nrf_gpio_pin_toggle(BUZZER_EN);
		}	
		else
		{
			g_moto_flag = 0;
			moto_cnt = 0;
			nrf_gpio_pin_clear(BUZZER_EN);
			moto_timer_stop();	
			return ;
		}
		if(moto_cnt>=MOTO_TIME) 		//1.2S
		{
#if dbg
			//printf("moto_cnt = %d,g_lock_flag = %d,g_moto_flag = %d\r\n",moto_cnt,g_lock_flag,g_moto_flag);
#endif

			Moto_DISABLE();
//			if(moto_cnt>=LOCK_BUZZER_CNT)
//			{
//				nrf_gpio_pin_clear(BUZZER_EN);
//				g_moto_flag = 0;
//				moto_cnt = 0;
//				nrf_gpio_pin_clear(BUZZER_EN);
//				moto_timer_stop();
//			}
//			return ;
		}
		/*
		if(moto_cnt>=LOCK_BUZZER_CNT)
		{
				g_moto_flag = 0;
				moto_cnt = 0;
				nrf_gpio_pin_clear(BUZZER_EN);
				moto_timer_stop();			
		}*/
		return ;
	}

}


		
		

		

/* 创建lcd和马达震动超时定时器*/
static void moto_timer_create(void)
{
	uint32_t err_code = app_timer_create(&m_moto_handle_id,
															APP_TIMER_MODE_REPEATED,
															moto_bra_timeout_handler);//moto_timeout_handler);
	APP_ERROR_CHECK(err_code);

}



/**@brief Function for starting application timers.
 */
void moto_timer_start(void)
{
	uint32_t err_code = app_timer_start(m_moto_handle_id, MOTO_HANDLE_MEAS_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);

#if dbg
	//printf("moto_timer_start \r\n");
#endif	
}

/**@brief Function for starting application timers.
 */
void moto_timer_stop(void)
{
	uint32_t err_code = app_timer_stop(m_moto_handle_id);
	APP_ERROR_CHECK(err_code);
#if dbg
		//printf("moto_timer_stop \r\n");
#endif	
}

extern void gpio_timer_create(void);

void reset_bt(void)
{
	nrf_gpio_cfg_output(MOTO_EN);
	nrf_gpio_cfg_output(IA_EN);
	nrf_gpio_cfg_output(IB_EN);
	nrf_gpio_cfg_output(BUZZER_EN);
	nrf_gpio_pin_set(MOTO_EN);
	nrf_gpio_pin_clear(IA_EN);
	nrf_gpio_pin_set(IB_EN);
	nrf_gpio_pin_set(BUZZER_EN);
	nrf_delay_ms(50);
	NVIC_SystemReset();
}


//马达初始�?
void Rd_MotoInit(void)
{

#if 1
	nrf_gpio_cfg_output(MOTO_EN);
	nrf_gpio_cfg_output(IA_EN);
	nrf_gpio_cfg_output(IB_EN);
	nrf_gpio_cfg_output(BUZZER_EN);
	//nrf_gpio_cfg_output(MT2503_RESET_PIN);
	//nrf_gpio_cfg_output(MT2503_B1);
	//nrf_gpio_cfg_output(MT2503_B2);
	//nrf_gpio_cfg_output(MT2503_GOTO_LOCK_PIN);
	//nrf_gpio_cfg_output(MT2503_GOTO_UNLOCK_PIN);
	//nrf_gpio_cfg_output(BLE_HEART_PIN);
	nrf_gpio_pin_clear(MOTO_EN);
	nrf_gpio_pin_clear(IA_EN);
	nrf_gpio_pin_clear(IB_EN);
	nrf_gpio_pin_clear(BUZZER_EN);
	//nrf_gpio_pin_clear(MT2503_RESET_PIN);
	//nrf_gpio_pin_clear(MT2503_B1);
	//nrf_gpio_pin_clear(MT2503_B2);
	//nrf_gpio_pin_set(BUZZER_EN);
	//nrf_gpio_pin_clear(MT2503_GOTO_LOCK_PIN);
	//nrf_gpio_pin_clear(MT2503_GOTO_UNLOCK_PIN);
	//nrf_gpio_pin_clear(BLE_HEART_PIN);
	nrf_gpio_cfg_input(MOTO_STOP_PIN,(nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pullup);
	//nrf_gpio_cfg_input(MT2503_HEART_PIN,(nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pulldown);

	Moto_ENABLE();
	Moto_B2A();
	nrf_delay_ms(50);
	Moto_DISABLE();	
	g_lock_flag = false;
	moto_timer_create();
	gpio_timer_create();
	//BLE_HEARTBit();
#endif	
}

