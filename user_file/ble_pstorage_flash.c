#include "nrf_delay.h"
#include "pstorage.h"
#include "pstorage_platform.h"
#include "nrf_error.h"
#include "data_struct.h"

flash_op_flag f_op_flag;

#define FLASH_OP_TIMEOUT  1000

#define ASSERT_FLASH(flash_flag,FLASH_OP_TIMEOUT)  uint16_t timeout=0;                                \
                                                   while(flash_flag==false){                          \
																									  nrf_delay_ms(1);                                  \
																									  timeout++;                                        \
																									  if(timeout>=FLASH_OP_TIMEOUT)return FAIL;         \
																									 }                                                  \
																									 flash_flag = false;                                \
																									 return SUCCESS;                                    

//flash操作事件完成后回调事件标志位
static void f_cb_handler(pstorage_handle_t *  p_handle,
									uint8_t              op_code,
									uint32_t             result,
									uint8_t *            p_data,
									uint32_t             data_len)
{
	 switch(op_code)
   {
		 case PSTORAGE_ERROR_OP_CODE:
			   if (result == NRF_SUCCESS)
				 {
						 f_op_flag.f_operror_flag = true;
				 }
				 else
				 {
						 f_op_flag.f_operror_flag = false;
				 }
				 break;
		 case PSTORAGE_STORE_OP_CODE:
			   if (result == NRF_SUCCESS)
				 {
						 f_op_flag.f_store_flag = true;
				 }
				 else
				 {
						 f_op_flag.f_store_flag = false;
				 }
				 break;
		 case PSTORAGE_LOAD_OP_CODE:
				 if (result == NRF_SUCCESS)
				 {
						 f_op_flag.f_load_flag = true;
				 }
				 else
				 {
						 f_op_flag.f_load_flag = false;
				 }
				 break;
			case PSTORAGE_CLEAR_OP_CODE:
				 if (result == NRF_SUCCESS)
				 {
						 f_op_flag.f_clear_flag = true;
				 }
				 else
				 {
						 f_op_flag.f_clear_flag = false;
				 }
				 break;
			case PSTORAGE_UPDATE_OP_CODE:
				 if (result == NRF_SUCCESS)
				 {
						 f_op_flag.f_updata_flag = true;
				 }
				 else
				 {
						 f_op_flag.f_updata_flag = false;
				 }
				 break;
			default:
				 break;
   }
}
      

//注册flash块，相当于分配内存
ble_responce_t RegisterFlash(pstorage_handle_t*  swap_flash_handle,const uint16_t blk_size,const uint8_t blk_count)
{
	pstorage_module_param_t     param;
	uint32_t                    retval;
	param.block_size  = blk_size;
	param.block_count = blk_count;
	param.cb          = f_cb_handler;
			
	retval = pstorage_register(&param, swap_flash_handle);
  if(retval == NRF_SUCCESS)
	{
		nrf_delay_ms(50);
		return SUCCESS;
	}else{
		return FAIL;
	}
}


//从flash中获取数据
ble_responce_t LoadFlash(pstorage_handle_t  swap_flash_handle,uint8_t  dest_data[],const uint16_t size,const uint8_t block_num,const uint16_t offset)
{
	uint32_t          retval;   
	pstorage_handle_t m_flash_handle;
	retval = pstorage_block_identifier_get(&swap_flash_handle, block_num,&m_flash_handle);
	if(retval == NRF_SUCCESS)
	{
		retval = pstorage_load(dest_data, &m_flash_handle, size, offset);
		if(retval == NRF_SUCCESS)
		{
			nrf_delay_ms(50);
			return SUCCESS;
		}else{
			return FAIL;
		}
	}else{
		return FAIL;
	}
}



//存储数据到flash
ble_responce_t StoreFlash(pstorage_handle_t  swap_flash_handle,uint32_t source_data[],const uint16_t size,const uint8_t block_num,const uint16_t offset)
{
	uint32_t          retval;    
	pstorage_handle_t m_flash_handle;
	retval= pstorage_block_identifier_get(&swap_flash_handle, block_num, &m_flash_handle);
  if(retval == NRF_SUCCESS)
	{
		retval = pstorage_store(&m_flash_handle, (uint8_t*)source_data, size, offset);
		if(retval == NRF_SUCCESS)
		{
			ASSERT_FLASH(f_op_flag.f_store_flag,FLASH_OP_TIMEOUT)
		}else{
			return FAIL;
		}
  }else{
		return FAIL;
	}
}


//清理flash中的数据
ble_responce_t ClearFlash(pstorage_handle_t  swap_flash_handle,const uint16_t blk_count_x_size)
{
	uint32_t  retval;    
	retval = pstorage_clear(&swap_flash_handle, blk_count_x_size);
	if(retval == NRF_SUCCESS)
	{
		ASSERT_FLASH(f_op_flag.f_clear_flag,FLASH_OP_TIMEOUT)
	}else{
		return FAIL;
	}
}


//更新flash数据
ble_responce_t UpdateFlash(pstorage_handle_t  swap_flash_handle,uint32_t source_dat[],const uint16_t size,const uint8_t block_num,const uint16_t offset)
{
	 uint32_t retval;
	 pstorage_handle_t m_flash_handle;
	 retval= pstorage_block_identifier_get(&swap_flash_handle,block_num,&m_flash_handle);
	 if(retval == NRF_SUCCESS)
	 {
	   retval =pstorage_update(&m_flash_handle,(uint8_t*)source_dat,size,offset);
	   if(retval == NRF_SUCCESS)
	   {				 
		   ASSERT_FLASH(f_op_flag.f_updata_flag,FLASH_OP_TIMEOUT)
	   }else{
		  return FAIL;
	   }
	 }else{
		return FAIL;
	}
}

//flash事件队列清理
ble_responce_t FlashEventClear(void)
{
	uint32_t count;
	uint16_t delay;
	do{
		count = 0;
		pstorage_access_status_get(&count);
		if(count == 0)
		{
			return SUCCESS;
		}
		delay++;
		nrf_delay_ms(1);
		if(delay >= 1000) return FAIL;
	}while(count != 0);
	return SUCCESS;
}




