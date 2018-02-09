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
#include "app_util.h"
#include "nrf.h"
#include "simple_uart.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "stdio.h"
#include	"string.h"
#include "nrf_delay.h"
#include "simple_uart.h"
#include "pstorage.h"
#include "ble_service.h"
//////////////////////////

extern uint32_t g_number ;
extern char LOGO[3] ;

int fputc(int ch, FILE *f)
{
        NRF_UART0->TXD = (uint8_t) ch;


  while (NRF_UART0->EVENTS_TXDRDY!=1)
  {
    // Wait for TXD data to be sent
                
  }

  NRF_UART0->EVENTS_TXDRDY=0;
        return ch;
}






/////////////////////////
uint8_t simple_uart_get(void)
{
  while (NRF_UART0->EVENTS_RXDRDY != 1)
  {
    // Wait for RXD data to be received
  }
  
  NRF_UART0->EVENTS_RXDRDY = 0;
  return (uint8_t)NRF_UART0->RXD;
}

bool simple_uart_get_with_timeout(int32_t timeout_ms, uint8_t *rx_data)
{
  bool ret = true;
  
  while (NRF_UART0->EVENTS_RXDRDY != 1)
  {
    if (timeout_ms-- >= 0)
    {
      // wait in 1ms chunk before checking for status
      nrf_delay_us(1000);
    }
    else
    {
      ret = false;
      break;
    }
  }  // Wait for RXD data to be received

  if (timeout_ms >= 0)
  {
    // clear the event and set rx_data with received byte
      NRF_UART0->EVENTS_RXDRDY = 0;
      *rx_data = (uint8_t)NRF_UART0->RXD;
  }

  return ret;
}

void simple_uart_put(uint8_t cr)
{
  NRF_UART0->TXD = (uint8_t)cr;

  while (NRF_UART0->EVENTS_TXDRDY!=1)
  {
    // Wait for TXD data to be sent
  }

  NRF_UART0->EVENTS_TXDRDY=0;
}

void simple_uart_putstring(const uint8_t *str)
{
  uint_fast8_t i = 0;
  uint8_t ch = str[i++];
  while (ch != '\0')
  {
    simple_uart_put(ch);
    ch = str[i++];
  }
}

void Uart_Deinit(uint8_t txd_pin_number,uint8_t rxd_pin_number)
{

	//printf("\r\n\r\n");
	NVIC_DisableIRQ(UART0_IRQn);
	nrf_gpio_cfg_output(txd_pin_number);
	nrf_gpio_cfg_output(rxd_pin_number);
	nrf_gpio_pin_clear(txd_pin_number);
	nrf_gpio_pin_clear(rxd_pin_number);
	NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);

}


void simple_uart_config(uint8_t txd_pin_number,uint8_t rxd_pin_number,bool hwfc)
{
	/** @snippet [Configure UART RX and TX pin] */
	nrf_gpio_cfg_output(txd_pin_number);
	nrf_gpio_cfg_input(rxd_pin_number, NRF_GPIO_PIN_NOPULL);  

	NRF_UART0->PSELTXD = txd_pin_number;
	NRF_UART0->PSELRXD = rxd_pin_number;

	NRF_UART0->BAUDRATE         = (UART_BAUDRATE_BAUDRATE_Baud115200 << UART_BAUDRATE_BAUDRATE_Pos);
	NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->TASKS_STARTTX    = 1;
	NRF_UART0->TASKS_STARTRX    = 1;
	NRF_UART0->EVENTS_RXDRDY    = 0;


	/////////////////
	NRF_UART0->INTENSET         = UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos;
	NVIC_ClearPendingIRQ(UART0_IRQn);//
	NVIC_SetPriority(UART0_IRQn, APP_IRQ_PRIORITY_LOW);
	NVIC_EnableIRQ(UART0_IRQn);
}

#define  USART_REC_LEN 200
unsigned char RXdata[USART_REC_LEN] = {0};
unsigned char UartData_Ok = false;

unsigned short USART_RX_STA = 0x00;
void UART0_IRQHandler(void)
{
	unsigned char Res = 0;
	while(NRF_UART0->EVENTS_RXDRDY != 1)
	{
		// Wait for RXD data to be received
	}
	Res = NRF_UART0->RXD;
	
	NRF_UART0->EVENTS_RXDRDY = 0;
	NVIC_ClearPendingIRQ(UART0_IRQn);
	
	NFCUartPushReceiveByte(Res);	
	
	//////////////////////////////////////
#if 0	
	if((USART_RX_STA&0x8000)==0)//?車那??∩赤那3谷
	{
		if(USART_RX_STA&0x4000)//?車那?米?芍?0x0d
		{
			if(Res!=0x0a)USART_RX_STA=0;//?車那?∩赤?車,??D??a那?
			else 
			{
				RXdata[USART_RX_STA&0X3FFF]=Res;
				USART_RX_STA|=0x8000;	//?車那?赤那3谷
			}
		}
		else //?1??那?米?0X0D
		{	
			if(Res==0x0d)
			{
				RXdata[USART_RX_STA&0X3FFF]=Res ;
				USART_RX_STA++;	
				USART_RX_STA|=0x4000;
			}
			else
			{
				RXdata[USART_RX_STA&0X3FFF]=Res ;
				USART_RX_STA++;
				if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//?車那?那y?Y∩赤?車,??D??a那??車那?	  
			}		 
		}
	}
#endif
}


	
