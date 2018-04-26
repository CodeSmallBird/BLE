#ifndef _PSTORAGE_LHAT_H_
#define _PSTORAGE_LHAT_H_
#include "common.h"
#include "data_struct.h"
#include "pstorage_platform.h"
//#include <stdint.h>
//#include <string.h>
//#include "nordic_common.h"
//#include "nrf.h"
//#include "app_error.h"
//#include "nrf_gpio.h"
//#include "nrf51_bitfields.h"
//#include "ble.h"
//#include "ble_hci.h"
//#include "ble_srv_common.h"
//#include "ble_advdata.h"
//#include "ble_dis.h"
//#include "ble_conn_params.h"
//#include "boards.h"
//#include "ble_sensorsim.h"
//#include "softdevice_handler.h"
//#include "app_timer.h"
//#include "ble_error_log.h"
//#include "ble_bondmngr.h"
//#include "ble_debug_assert_handler.h"
//#include "pstorage.h"
//#include "ble_trans.h"


typedef struct bt_flash_op_flag
{
	bool f_store_flag;
	bool f_load_flag;
	bool f_clear_flag;
	bool f_updata_flag;
}bt_flash_op_flag_t;
	




ble_responce_t f_register_flash(pstorage_handle_t*  swap_flash_handle,const uint16_t blk_size,const uint8_t blk_count);
ble_responce_t f_load_flash(pstorage_handle_t  swap_flash_handle,uint8_t*  dest_data,const uint16_t dat_size,const uint8_t block_num,const uint16_t offset);

ble_responce_t f_store_flash(pstorage_handle_t  swap_flash_handle,uint32_t* source_data,const uint16_t dat_size,const uint8_t block_num,const uint16_t offset);
ble_responce_t f_clear_flash(pstorage_handle_t  swap_flash_handle,const uint16_t blk_count_x_size);
ble_responce_t f_update_flash(pstorage_handle_t  swap_flash_handle,uint32_t* source_dat,const uint16_t dat_size,const uint8_t block_num,const uint16_t offset);
ble_responce_t f_flash_event_clear(void);



#endif 


