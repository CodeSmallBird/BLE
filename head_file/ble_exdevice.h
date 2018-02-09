#ifndef _BLE_EXDEVICE_H_
#define _BLE_EXDEVICE_H_
#include "common.h"
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf51_bitfields.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "ble_sensorsim.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "ble_error_log.h"
#include "ble_bondmngr.h"
#include "ble_debug_assert_handler.h"
#include "pstorage.h"
#include "ble_trans.h"



typedef struct
{
		uint8_t     	year  ;
		uint8_t       month ;
		uint8_t 			day   ;
		uint8_t       hour  ;
		uint8_t       minute;
		uint8_t       second;
	  uint8_t	      tnum;					//定时器次数
}ble_system_clock_t;







void BMA250_Read_XYZ(void);
void TWI_Init(void);
bool bma250_get_vibrate_status(void);
bool bma250_get_vibrate_status2(void);
uint8_t  BMA250_ReadID(void);



#endif


