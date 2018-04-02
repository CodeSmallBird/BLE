#ifndef _NFC_MS522_H_
#define _NFC_MS522_H_

#include "nrf_delay.h"
#include "nrf6310.h"
#include "ble_service.h"
#include "spi_drv.h "



#define SPEED_106 0x00
#define SPEED_212 0x10
#define SPEED_424 0x20
#define SPEED_848 0x30
#define SPEED_1696 0x40
#define SPEED_3392 0x50

#define NRSTPD_PIN	18	

#if defined(NFC_POWER_PIN)
#define NRSTPD_INIT   		do{nrf_gpio_cfg_output(NFC_POWER_PIN); nrf_gpio_cfg_output(NRSTPD_PIN);}while(0)  	
#define NRSTPD_ON 		    do{nrf_gpio_pin_set(NFC_POWER_PIN); nrf_gpio_pin_set(NRSTPD_PIN);}while(0)  	//nrf_gpio_pin_set(NRSTPD_PIN);
#define NRSTPD_OFF 		    do{nrf_gpio_pin_clear(NRSTPD_PIN);nrf_gpio_pin_clear(NFC_POWER_PIN);}while(0)  			//nrf_gpio_pin_clear(NRSTPD_PIN);
#else
#define NRSTPD_INIT  	nrf_gpio_cfg_output(NRSTPD_PIN);
#define NRSTPD_ON 		nrf_gpio_pin_set(NRSTPD_PIN);
#define NRSTPD_OFF 		nrf_gpio_pin_clear(NRSTPD_PIN);
#endif

extern void nfc_card_init(void);
extern bool nfc_recv_start(void);
extern bool nfc_work_repeat(void);


#endif

