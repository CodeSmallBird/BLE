#ifndef _DATA_EXCHANGE_H
#define _DATA_EXCHANGE_H

#include <stdbool.h>
#include <string.h>
#include "nrf51.h"


#define DISABLE_PIN_FN(PIN_NUM)  do{                            																														    \
																		NRF_GPIO->PIN_CNF[(PIN_NUM)] = (GPIO_PIN_CNF_SENSE_Disabled 	<< GPIO_PIN_CNF_SENSE_Pos) |	\
																																	 (GPIO_PIN_CNF_DRIVE_S0S1 			<< GPIO_PIN_CNF_DRIVE_Pos) |	\
																																	 (GPIO_PIN_CNF_PULL_Disabled 		<< GPIO_PIN_CNF_PULL_Pos)  |	\
																																	 (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |	\
																																	 (GPIO_PIN_CNF_DIR_Input 				<< GPIO_PIN_CNF_DIR_Pos);			\
																   }while(0);																							/**< Disable引脚功能.*/


typedef struct
{
	bool                            moto_is_shake                        ;     //马达是否在震动
	bool                            moto_shake_direction                 ;     //震动方向
	uint8_t                         moto_shake_type                      ;     //马达震动类型
	uint8_t                         moto_select[3]                       ;     //马达的选择
}Moto_Ctr_Type;


typedef enum
{
	  FAIL=0,
		SUCCESS,	
		NOFOUND,	  		
}ble_responce_t;


#endif //_DATA_EXCHANGE_H



