/************************************************************
  Copyright (C), Shenzhen Bonten Health Techology Co.,Ltd.
  FileName: 		 ble_config.c
  Author: Kevin  Version :  V0.0    Date:2015/03/18
  Description:     //ģࠩĨ˶
  Version:         //ѦѾۅ
  Function List:   //׷Ҫگ˽ܰ٦Ŝ
    1. -------
  History:         //zʷўل݇¼
     <author>   <time>   <version >       <desc>
***********************************************************/

#ifndef BLE_ADC_BAT_H_
#define BLE_ADC_BAT_H_

#include "nrf_gpio.h"
#include <stdbool.h>
#include <stdint.h>
#include "nrf_soc.h"
#include "core_cminstr.h"
#include "app_error.h"

#define BAT_MAX_VOLTAGE                          3700		//充满
#define BAT_70PER_VOLTAGE                        3400		//>2800
#define BAT_30PER_VOLTAGE                        3150		//2600~2800
#define BAT_MIN_VOLTAGE                          2900		//低电关机

#define BAT_ADC_REF_VOLTAGE_IN_MILLIVOLTS        1200                                      /**< Reference voltage (in milli volts) used by ADC while doing conversion. */
#define BAT_ADC_PRE_SCALING_COMPENSATION         6                                         /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/

//#define BAT_ADC_PRE_VOLTAGE_MAX			6216		// 4.1V   // 6354  4.2V
//#define BAT_ADC_RRE_VOLTAGE_MIN			5148		// 3.4V

#define BAT_ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
													((((ADC_VALUE) * BAT_ADC_REF_VOLTAGE_IN_MILLIVOLTS + 128) / 256) * BAT_ADC_PRE_SCALING_COMPENSATION)
													
#define BAT_PERCENT(voltage)    voltage>=BAT_MAX_VOLTAGE?100:(voltage-BAT_MIN_VOLTAGE)/((BAT_MAX_VOLTAGE-BAT_MIN_VOLTAGE)/100)


//电量等级转换函数
uint8_t battery_level_transform(void);


//充电检测处理函数 
void charge_check_handle(void);

//开机检测电量是否开机成功
bool power_on_for_battery_check(void);

//????
uint8_t Bat_Return(void);
uint16_t bat_get_vol_value(void);


#endif

/**
  @}
  @}
*/


