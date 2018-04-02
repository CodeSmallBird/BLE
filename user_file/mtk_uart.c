/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include <stdint.h>

#include "nrf.h"
#include "mtk_uart.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"
#include "app_util.h"
#include "data_struct.h"
#include "global_var.h"
#include "app_timer.h"
#include "nordic_common.h"

#define APP_TIMER_PRESCALER_K                0  
#define APP_GPIOTE_MAX_USERS                 3                                          /**< Maximum number of users of the GPIOTE handler. */
#define KEY_CHECK_MEAS_INTERVAL              APP_TIMER_TICKS(100, APP_TIMER_PRESCALER_K)

static  app_timer_id_t                       m_uart_timeout_id;
static  uint8_t	                             Uart_Data_Buffer[20]  = {0};
static  uint8_t                              Uart_Data_Length      = 0x00;

static  bool                                 uart_ready = false;


static void uart_timeout_handler(void * p_context)
{
	UNUSED_PARAMETER(p_context);
  if(Uart_Data_Length > 5 && Uart_Data_Length < 21 && Uart_Data_Buffer[Uart_Data_Length-1] != 0x00)
	{
		uart_ready = true;
	}
	
	if(uart_ready){
		uart_ready    = false;
    ble_data_update(&m_ble,Uart_Data_Buffer, Uart_Data_Length);
		memset(Uart_Data_Buffer,0,sizeof(Uart_Data_Buffer));
	}
}

/*按键事件触发*/
static void uart_timer_create(void)
{
	uint32_t err_code;
	err_code = app_timer_create(&m_uart_timeout_id,
															APP_TIMER_MODE_REPEATED,
															uart_timeout_handler);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting application timers.
 */
static void uart_timer_start(void)
{
	uint32_t err_code;

	// Start application timers
	err_code = app_timer_start(m_uart_timeout_id, KEY_CHECK_MEAS_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting application timers.
 */
//static void key_timer_stop(void)
//{
//	uint32_t err_code;

//	// Stop key timers
//	err_code = app_timer_stop(m_uart_timeout_id);
//	APP_ERROR_CHECK(err_code);
//}


/*******************************************
 *函数说明:串口初始化
 *函数参数:无
 *函数返回:无
 *******************************************/
void uart_init(void)
{
	/** @snippet [Configure UART RX and TX pin] */ 

  NRF_UART0->PSELTXD = UART_TX_PIN_NUMBER;
  NRF_UART0->PSELRXD = UART_RX_PIN_NUMBER;

  NRF_UART0->BAUDRATE         = (UART_BAUDRATE_BAUDRATE_Baud115200 << UART_BAUDRATE_BAUDRATE_Pos);
  NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->TASKS_STARTTX    = 1;
  NRF_UART0->TASKS_STARTRX    = 1;  
	NRF_UART0->INTENSET         = UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos;
  NRF_UART0->EVENTS_RXDRDY    = 0;
	
	NVIC_ClearPendingIRQ(UART0_IRQn);//清理挂起的串口中断
	NVIC_SetPriority(UART0_IRQn, APP_IRQ_PRIORITY_LOW);
	NVIC_EnableIRQ(UART0_IRQn);
	
	uart_timer_create();
	uart_timer_start();
}

/*******************************************
 *函数说明：串口关闭
 *函数参数：无
 *函数返回:无
 *******************************************/
void uart_disable(void)
{
	DISABLE_PIN_FN(UART_TX_PIN_NUMBER)
	DISABLE_PIN_FN(UART_RX_PIN_NUMBER)
	NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->TASKS_STARTTX = 0;
  NRF_UART0->TASKS_STARTRX = 0;  
}


/*******************************************
 *函数说明:串口发送字符串
 *函数参数:str 字符串 ；len 数据长度
 *函数返回:无
 *******************************************/
void uart_putstring(const uint8_t* str, uint16_t len)
{
  uint_fast8_t i = 0;
	
  while(i < len)
  {
		NRF_UART0->TXD = (uint8_t)str[i++];

		while(NRF_UART0->EVENTS_TXDRDY != 1)
		{
			// Wait for TXD data to be sent
		}

		NRF_UART0->EVENTS_TXDRDY = 0;
  }
}

/*******************************************
 *函数说明：发送一个字节到
 *函数参数：cr  发送的数据
 *函数返回:无
 *******************************************/
static void myuart_put(uint8_t cr)
{
	  NRF_UART0->EVENTS_TXDRDY = 0;
		NRF_UART0->TXD = cr;

		while(NRF_UART0->EVENTS_TXDRDY != 1)
		{
			//Wait for TXD data to be sent
		}

		NRF_UART0->EVENTS_TXDRDY=0;
}

/*******************************************
 *函数说明：发送多个字节的数据
 *函数参数：datas 发送的数据 len 数据长度
 *函数返回:无
 *******************************************/
void uart_send_multiple_bytes(uint8_t* datas,uint8_t len)
{
	for(uint8_t i = 0;i < len;i++)
	  myuart_put(datas[i]);
}

 /******************************************
 *函数说明:串口中断接收数据
 *函数参数:无
 *函数返回:无
 *******************************************/
void UART0_IRQHandler(void)
{
	while(NRF_UART0->EVENTS_RXDRDY != 1)
	{
		// Wait for RXD data to be received
	}
	
	NRF_UART0->EVENTS_RXDRDY = 0;
	NVIC_ClearPendingIRQ(UART0_IRQn);
	
	Uart_Data_Buffer[Uart_Data_Length++]  = (uint8_t)NRF_UART0->RXD;

}

