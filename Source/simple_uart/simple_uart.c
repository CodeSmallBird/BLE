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

//void simple_uart_config(  uint8_t rts_pin_number,
//                          uint8_t txd_pin_number,
//                          uint8_t cts_pin_number,
//                          uint8_t rxd_pin_number,
//                          bool    hwfc)
//{
///** @snippet [Configure UART RX and TX pin] */
//  nrf_gpio_cfg_output(txd_pin_number);
//  nrf_gpio_cfg_input(rxd_pin_number, NRF_GPIO_PIN_NOPULL);  

//  NRF_UART0->PSELTXD = txd_pin_number;
//  NRF_UART0->PSELRXD = rxd_pin_number;
///** @snippet [Configure UART RX and TX pin] */
//  if (hwfc)
//  {
//    nrf_gpio_cfg_output(rts_pin_number);
//    nrf_gpio_cfg_input(cts_pin_number, NRF_GPIO_PIN_NOPULL);
//    NRF_UART0->PSELCTS = cts_pin_number;
//    NRF_UART0->PSELRTS = rts_pin_number;
//    NRF_UART0->CONFIG  = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
//  }

//  NRF_UART0->BAUDRATE         = (UART_BAUDRATE_BAUDRATE_Baud38400 << UART_BAUDRATE_BAUDRATE_Pos);
//  NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
//  NRF_UART0->TASKS_STARTTX    = 1;
//  NRF_UART0->TASKS_STARTRX    = 1;
//  NRF_UART0->EVENTS_RXDRDY    = 0;
//	
//	
//	/////////////////
//	NRF_UART0->INTENSET         = UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos;
//	NVIC_ClearPendingIRQ(UART0_IRQn);//
//	NVIC_SetPriority(UART0_IRQn, APP_IRQ_PRIORITY_LOW);
//	NVIC_EnableIRQ(UART0_IRQn);
//}

void Uart_Deinit(uint8_t txd_pin_number,uint8_t rxd_pin_number)
{

	////printf("\r\n\r\n");
	NVIC_DisableIRQ(UART0_IRQn);
    nrf_gpio_cfg_input(txd_pin_number, NRF_GPIO_PIN_NOPULL);
#if 0
	nrf_gpio_cfg_output(rxd_pin_number);
	nrf_gpio_pin_clear(rxd_pin_number);
#else
	nrf_gpio_cfg_input(rxd_pin_number, NRF_GPIO_PIN_NOPULL);
#endif
	NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);

}


void simple_uart_config(uint8_t txd_pin_number,uint8_t rxd_pin_number,bool hwfc)
{
/** @snippet [Configure UART RX and TX pin] */
  nrf_gpio_cfg_output(txd_pin_number);
  nrf_gpio_cfg_input(rxd_pin_number, NRF_GPIO_PIN_NOPULL);  

  NRF_UART0->PSELTXD = txd_pin_number;
  NRF_UART0->PSELRXD = rxd_pin_number;
/** @snippet [Configure UART RX and TX pin] */
//  if (hwfc)
//  {
//    nrf_gpio_cfg_output(rts_pin_number);
//    nrf_gpio_cfg_input(cts_pin_number, NRF_GPIO_PIN_NOPULL);
//    NRF_UART0->PSELCTS = cts_pin_number;
//    NRF_UART0->PSELRTS = rts_pin_number;
//    NRF_UART0->CONFIG  = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
//  }

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


#if defined(CONTROL_UART)
bool close_flag = true;
uint16_t close_delay_time = 0;
void open_uart(void)
{
	simple_uart_config(MT2503_TX_PIN,MT2503_RX_PIN,false);
	close_delay_time = 100;
	close_flag = false;
}
void close_uart(void)
{
	Uart_Deinit(MT2503_TX_PIN,MT2503_RX_PIN);
	close_flag = true;
}

#endif








#define  USART_REC_LEN 200
unsigned char RXdata[USART_REC_LEN] = {0};
unsigned char UartData_Ok = false;

unsigned short USART_RX_STA = 0x00;

#if !defined(ADD_HELP_FUNCV_ORDER)

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
	
	//////////////////////////////////////
	
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
	
	}
#else
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
#if defined(CONTROL_UART)
	close_delay_time =50;
#endif
	
	if(((Res == '$')||(Res == '&'))&&((USART_RX_STA&0x4000) == 0))
	{
		USART_RX_STA = 0;
		RXdata[USART_RX_STA&0X3FFF]=Res;
		USART_RX_STA |= 0x4000;
		
		USART_RX_STA++;
		return;
	}
	
	if(((USART_RX_STA&0x8000) == 0)&&(USART_RX_STA&0x4000))
	{
	#if 0	
		if((USART_RX_STA&0x3fff) == 1)
		{
			if(Res == 0x76)
			{
				RXdata[USART_RX_STA&0X3FFF]=Res;
				USART_RX_STA++;
			}
			else
			{
				USART_RX_STA=0x00;
				memset(RXdata,0,USART_REC_LEN);
			}
		}
		else
		{
			RXdata[USART_RX_STA&0X3FFF] = Res;
			USART_RX_STA++;
			if((USART_RX_STA&0X3FFF) == (RXdata[2]+5))
			{
				USART_RX_STA |=0x8000;
			}
		}	
	#else
		if((USART_RX_STA&0x3fff) == 1)
		{
				RXdata[USART_RX_STA&0X3FFF]=Res;
				USART_RX_STA++;
			
		}
		else
		{
			RXdata[USART_RX_STA&0X3FFF] = Res;
			USART_RX_STA++;
			if((USART_RX_STA&0X3FFF) == (RXdata[1]+2))
			{
				USART_RX_STA |=0x8000;
			}
			else if ((USART_RX_STA&0X3FFF) > (RXdata[1]+2)) 
			{
				USART_RX_STA=0x00;
				memset(RXdata,0,USART_REC_LEN);			
			}
		}
	#endif
	}
}   


void port_data_send(uint8_t *send_buff,uint8_t len)
{
	uint8_t i =0 ;
	for(;i<len;i++)
	{
		simple_uart_put(send_buff[i]);
	}
}

	

extern ble_t  m_ble;      


void ble_send_datas(uint8_t* send_data, uint16_t size)
{
	if(size<20)
	{
		ble_data_update(&m_ble,send_data, size);
	}
	else
	{
		unsigned char send_array_n = size/20 ;
		unsigned char send_array_m = size%20 ;
		unsigned char i=0,j=0 ;
		uint8_t send_buf_back[20] = {0};

		for(i=0;i<send_array_n;i++)
		{
			for(j=0;j<20;j++)
			{
				send_buf_back[j]=send_data[i*20+j];
			}
			ble_data_update(&m_ble,send_buf_back, 20);

			memset(send_buf_back,0,sizeof(send_buf_back));
		}
		for(j=0;j<send_array_m;j++)
		{
			send_buf_back[j]=send_data[send_array_n*20+j];
		}
		ble_data_update(&m_ble,send_buf_back, send_array_m);
		memset(send_buf_back,0,sizeof(send_buf_back));
	}
}




void receive_uart_parse(void)
{

	if(USART_RX_STA&0x8000)
	{
	#if 0
		switch(RXdata[3])
		{
				case 0x54:
				case 0x5C:
				case 0x5D:
				case 0x5E:
				case 0x5F:
					ble_data_update(&m_ble,RXdata, USART_RX_STA&0X3FFF);
					memset(RXdata,0,USART_RX_STA&0X3FFF);
					USART_RX_STA = 0;
					break;
				
				case 0x60:
				case 0x61:
				case 0x62:
				case 0x63:

					break;
			default:
				break;
		}
	#else
		switch(RXdata[0])
		{
			case '$':
				ble_send_datas(RXdata+2, RXdata[1]);
				memset(RXdata,0,USART_RX_STA&0X3FFF);
				USART_RX_STA = 0;
				break;
			
			case '&':
				memset(RXdata,0,USART_RX_STA&0X3FFF);
				USART_RX_STA = 0;
				break;
			default:
				break;
		}
	#endif
	}

}


#endif

/*	
	
//extern unsigned int device_num;
//extern unsigned char device_name[20];
static int ParseDeviceName_Set(char *buff)
{
	char *p;
	uint8_t len = 0;
	int ret = 0x00;
	char name[15] = {0x00};
//	char device_name[16];
	unsigned char i = 0;
	char tmp[10] = {0};
	if(strlen(buff)<7)		return -1;

//	if(buff[0]<'A'||buff[1]>'Z')	return -1;
//	if(buff[0]<'A'||buff[1]>'Z')	return -1;
//	memcpy(LOGO,buff,2);
//	LOGO[3] = '\0';
	
	p = &buff[2];
#if dbg
	//printf("ParseDeviceName_Set:%s,%d\r\n",buff,strlen(buff));
#endif
	for(i=0;i<5;i++)
	{
		name[i] = p[i];
	}
	if(name[0]<'A'||name[0]>'Z')	return -1;
	if(name[1]<'A'||name[1]>'Z')	return -1;
	if(name[2]<'0'||name[2]>'9')	return -1;
	if(name[3]<'0'||name[3]>'9')	return -1;
	if(name[4]<'0'||name[4]>'9')	return -1;
	len += sprintf(tmp+len,"%02d%02d%s",name[0],name[1],&name[2]);
	tmp[len] = '\0';
#if dbg
		//printf("Device name = %s \r\n",tmp);
#endif
	ret = atoi(tmp);
	if(ret<0)	
	{
#if dbg
		//printf("format error\r\n");
#endif
		return  -1;
	}
	else
		g_number = ret;
	return ret;
}
	extern 	void Storage_Module_init(void);

void BlueDeviceName_Set(pstorage_handle_t * handle)
{

	pstorage_handle_t dest_block_id;
	int value;
		if(USART_RX_STA&0x8000)
		{
//#if dbg
//	//printf("value = %s\r\n",RXdata)		;
//			
//#endif			
			value = ParseDeviceName_Set((char *)RXdata);
			if(value>0)
			{
				Storage_Module_init();
				pstorage_block_identifier_get(handle, 1, &dest_block_id);
				pstorage_update(&dest_block_id, (uint8_t *)&value, 4, 0);
				nrf_delay_ms(2000);
				USART_RX_STA = 0;
//				NVIC_SystemReset();
			}
			else
			{
#if dbg
				//printf("value = %s\r\n",RXdata);
#endif
				USART_RX_STA = 0;
			}

		}
		
}
//	
	*/
	
