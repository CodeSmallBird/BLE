#ifndef _BLE_PSTORAGE_FLASH_H_
#define _BLE_PSTORAGE_FLASH_H_

#include "common.h"
#include "data_struct.h"
#include "pstorage_platform.h"

ble_responce_t RegisterFlash(pstorage_handle_t* swap_flash_handle,const uint16_t blk_size,const uint8_t blk_count);
ble_responce_t LoadFlash(pstorage_handle_t  swap_flash_handle,uint8_t  dest_data[],const uint16_t size,const uint8_t block_num,const uint16_t offset);
ble_responce_t StoreFlash(pstorage_handle_t  swap_flash_handle,uint32_t source_data[],const uint16_t size,const uint8_t block_num,const uint16_t offset);
ble_responce_t UpdateFlash(pstorage_handle_t  swap_flash_handle,uint32_t source_dat[],const uint16_t size,const uint8_t block_num,const uint16_t offset);
ble_responce_t ClearFlash(pstorage_handle_t  swap_flash_handle,const uint16_t blk_count_x_size);
ble_responce_t FlashEventClear(void);

#endif //_BLE_PSTORAGE_FLASH_H_




