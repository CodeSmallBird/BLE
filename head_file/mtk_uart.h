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

#ifndef BT_UART_H
#define BT_UART_H

/*lint ++flb "Enter library region" */

#include <stdbool.h>
#include <stdint.h>

/** @file
* @brief Simple UART driver
*
*
* @defgroup nrf_drivers_simple_uart Simple UART driver
* @{
* @ingroup nrf_drivers
* @brief Simple UART driver
*/

#define UART_TX_PIN_NUMBER (9U)  	   //tx	
#define UART_RX_PIN_NUMBER (11U)     //rx

/** @brief Function for reading a character from UART with timeout on how long to wait for the byte to be received.
Execution is blocked until UART peripheral detects character has been received or until the timeout expires, which even occurs first
\return bool True, if byte is received before timeout, else returns False.
@param timeout_ms maximum time to wait for the data.
@param rx_data pointer to the memory where the received data is stored.
*/
bool uart_get_with_timeout(uint32_t timeout_ms, uint8_t* rx_data);

/** @brief Function for sending a string to UART.
Execution is blocked until UART peripheral reports all characters to have been send.
Maximum string length is 254 characters including null character in the end.
@param str Null terminated string to send.
*/
void uart_putstring(const uint8_t* str,uint16_t len);

/** @brief Function for configuring UART to use 38400 baud rate.
@param rts_pin_number Chip pin number to be used for UART RTS
@param txd_pin_number Chip pin number to be used for UART TXD
@param cts_pin_number Chip pin number to be used for UART CTS
@param rxd_pin_number Chip pin number to be used for UART RXD
@param hwfc Enable hardware flow control
*/

/*******************************************
 *函数说明:串口初始化
 *函数参数:无
 *函数返回:无
 *******************************************/
void uart_init(void);
/*******************************************
 *函数说明：串口关闭
 *函数参数：无
 *函数返回:无
 *******************************************/
void uart_disable(void);
/*******************************************
 *函数说明：发送多个字节的数据
 *函数参数：datas 发送的数据 len 数据长度
 *函数返回:无
 *******************************************/
void uart_send_multiple_bytes(uint8_t* datas,uint8_t len);


/**
 *@}
 **/

/*lint --flb "Leave library region" */
#endif
