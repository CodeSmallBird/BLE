/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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
#ifndef NRF6310_H__
#define NRF6310_H__

#define USE_NEW_PIN

#define ADD_NB_MODULE			//Ìí¼ÓNBÄ£¿é

#define NB_DEBUG
//#define RESET_TEST

#define MOTO_STOP_PIN			28
#define IB_EN  		29
#define IA_EN  		30
#define BUZZER_EN	27
#define MOTO_EN		0
#define ADC_IN		3

#define MT2503_TX_PIN	8//6
#define MT2503_RX_PIN         7 

#if defined(ADD_NB_MODULE)

//#define INT0_PIN       0
#define SDA0_PIN       1
#define SCL0_PIN       2 
//#define ADC_IN		3
#define SCL1_PIN       4
#define SDA1_PIN       5

//#define NB_TX_PIN 6
//#define NB_RX_PIN 7

#define UART_CTS_PIN 6//       8
#define UART_RTS_PIN       9

#define AP_POWER_ON_PIN 10
#define AP_RESET_PIN             11
#define MODULE_WAKEUP_PIN             12

#define LED1_PIN				13
#define INT1_PIN				24//21	
#define MCU_WAKEUP_PIN				22	

#if defined(NB_DEBUG)
#define DEBUG_TX           21//      24
#define DEBUG_RX          25
#endif
#endif














#endif  // NRF6310_H__



