#include "ble_service.h"
#include "data_struct.h"
#include "ble_trans.h"
#include "nrf_delay.h"
#include "nrf6310.h"
//#include "RD_MotoControl.h"
//#include "RD_BatteryCharge.h"
#include "app_timer.h"
#include "app_button.h"
#include "app_gpiote.h"
#include "simple_uart.h"
#include "ble_flash.h"
#include "nrf_nvmc.h"
#include "pstorage.h"
#include "pstorage_platform.h"
#include "pstorage_lhat.h"
#include "ble_exdevice.h"

bool                                         m_blue_connect                     = false;         //蓝牙连接事件
ble_t                                        m_ble                              = {0};           //蓝牙


char ID[6] = "ZH200";
///////////////////////////////////
char LOGO[3] = "AB";

/***************************************/
int main(void)
{
	for(uint8_t num = 0; num< 32 ; num++) 
	DISABLE_PIN_FN(num);
#if 1//defined(DEBUG_RUN)
	simple_uart_config(BLE_TX,BLE_RX,false);
#endif
	BleInit(); 
	power_ctrl_init();
	DataTrainsInit();
#if 1//defined(DEBUG_RUN)
	printf("Sys_init_Ok\r\n");
#endif
	for(;;)
	{
		power_manage();
		power_ctrl_polling();
		DataTrainsPolling();
	}
}



