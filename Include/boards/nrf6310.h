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
#include "power_ctrl.h"

//#define DEBUG_RUN


#define CHARGE_DETEC 24

#define POW_LED1	    26
#define POW_LED2	    29
#define POW_LED3	    28
#define POW_LED4        27

#define MCU_OFF_CTL         1
#define MCU_ON_CTL         2

#define BLE_TX        6
#define BLE_RX        5

#define ADC_INPUT 3
#define BUTTON_PIN    25
#endif  // NRF6310_H__
