#ifndef _BLE_PROTOCOL_H_
#define _BLE_PROTOCL_H_

#include "nrf6310.h"
#if defined(ADD_BT_OPEN_LOCK)
#include "nrf_delay.h"
#include "ble_service.h"
#include "RD_MotoControl.h"

typedef struct 
{
	unsigned char cmd;
	unsigned char reset;
	unsigned int cipher;
}BLE_RECV;

extern BLE_RECV ble_recv;

extern void BleProtoclDeal(uint8_t* receive_buf,uint8_t size);
extern void app_open_send_data_to_phone(uint8_t cmd);
extern void BleOpenUpdateDeviceName(void);
#endif

#endif

