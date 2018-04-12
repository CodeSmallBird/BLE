#ifndef _POWER_CTRL_H_
#define _POWER_CTRL_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "nrf6310.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include"app_button.h"
#include"app_timer.h"
#include"app_gpiote.h"
#include"ble_service.h"


#if 0
#define BAT_ADC_REF_VOLTAGE_IN_MILLIVOLTS        1200                                      /**< Reference voltage (in milli volts) used by ADC while doing conversion. */

#define BAT_ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
													((((ADC_VALUE) * BAT_ADC_REF_VOLTAGE_IN_MILLIVOLTS) / 256))
													
#else
#define BAT_ADC_REF_VOLTAGE_IN_MILLIVOLTS                    8                                      /**< Reference voltage (in milli volts) used by ADC while doing conversion. */

#define BAT_ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
													(((ADC_VALUE) * BAT_ADC_REF_VOLTAGE_IN_MILLIVOLTS)-422)

#endif


#define POW_LED1_INIT            nrf_gpio_cfg_output(POW_LED1)
#define POW_LED1_HIGH           nrf_gpio_pin_set(POW_LED1)
#define POW_LED1_LOW             nrf_gpio_pin_clear(POW_LED1)
#define POW_LED1_ON              POW_LED1_HIGH
#define POW_LED1_OFF             POW_LED1_LOW

#define POW_LED2_INIT            nrf_gpio_cfg_output(POW_LED2)
#define POW_LED2_HIGH           nrf_gpio_pin_set(POW_LED2)
#define POW_LED2_LOW             nrf_gpio_pin_clear(POW_LED2)
#define POW_LED2_ON              POW_LED2_HIGH
#define POW_LED2_OFF             POW_LED2_LOW

#define POW_LED3_INIT            nrf_gpio_cfg_output(POW_LED3)
#define POW_LED3_HIGIH           nrf_gpio_pin_set(POW_LED3)
#define POW_LED3_LOW             nrf_gpio_pin_clear(POW_LED3)
#define POW_LED3_ON              POW_LED3_HIGIH
#define POW_LED3_OFF             POW_LED3_LOW

#define POW_LED4_INIT            nrf_gpio_cfg_output(POW_LED4)
#define POW_LED4_HIGH           nrf_gpio_pin_set(POW_LED4)
#define POW_LED4_LOW             nrf_gpio_pin_clear(POW_LED4)
#define POW_LED4_ON              POW_LED4_HIGH
#define POW_LED4_OFF             POW_LED4_LOW

#define POW_MCU_ON_INIT            nrf_gpio_cfg_output(MCU_ON_CTL)
#define POW_MCU_ON_HIGH           nrf_gpio_pin_set(MCU_ON_CTL)
#define POW_MCU_ON_LOW             nrf_gpio_pin_clear(MCU_ON_CTL)
#define POW_MCU_ON_ON              POW_MCU_ON_HIGH
#define POW_MCU_ON_OFF             POW_MCU_ON_LOW

#define POW_MCU_OFF_INIT            nrf_gpio_cfg_output(MCU_OFF_CTL)
#define POW_MCU_OFF_HIGH           nrf_gpio_pin_set(MCU_OFF_CTL)
#define POW_MCU_OFF_LOW             nrf_gpio_pin_clear(MCU_OFF_CTL)
#define POW_MCU_OFF_ON              POW_MCU_OFF_HIGH
#define POW_MCU_OFF_OFF             POW_MCU_OFF_LOW

#define ADC_INPUT_INIT         nrf_gpio_cfg_input(ADC_INPUT, NRF_GPIO_PIN_NOPULL) 


#define CHARGE_DET_INIT	          nrf_gpio_cfg_input(CHARGE_DETEC,NRF_GPIO_PIN_NOPULL)
#define GET_CHARGE_DET_STATE	          nrf_gpio_pin_read(CHARGE_DETEC)

typedef struct
{
	uint8_t sys_state;
	uint8_t on_ctrl;
	uint8_t off_ctrl;
	uint16_t ctrl_delay;
	uint8_t detec_time;
	uint16_t voltage;
	uint16_t cmp_voltage;
}POWER_CTRL;

extern POWER_CTRL power_ctrl;

typedef struct
{
	unsigned char 	Time;
	unsigned char 	SampNow;
	unsigned char 	SampPrev;
	unsigned char 	Wobble;
	unsigned char 	Compara;
	unsigned char 	Result;
}DETECT_BUFF;
extern DETECT_BUFF detect_buff_charge;

enum
{
	BATTERY_DISCHARGING,		//·Åµç
	BATTERY_CHARGING,		    //³äµç

};

enum
{
	BATTERY_LEVER1,
	BATTERY_LEVER2,
	BATTERY_LEVER3,
	BATTERY_LEVER4,
};


extern void power_ctrl_init(void);
extern void power_ctrl_polling(void);
extern void SetPowerOff(void);
extern void SetPowerOn(void);




#endif

