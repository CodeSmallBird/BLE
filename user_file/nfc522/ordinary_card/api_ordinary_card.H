#ifndef 	_API_ORDINARY_CARD_H_
#define 	_API_ORDINARY_CARD_H_

#include"rc522.h"
#include "nrf_delay.h"
#include "nrf6310.h"
#include "ble_service.h"
#include "RD_MotoControl.h"
#include"..\ordinary_card\api_md5.h"

enum
{
	GET_CARD_MODE,
	READ_CARD_MODE,
	WRITE_MONEY_TO_CARD_MODE,
	WRITE_DATA_TO_CARD_MODE,
	READ_DATA_FROM_CARD_MODE,
	CHANGE_CARD_SCREAT_MODE,
};

enum
{
	SET_CARD_ERROR,
	SET_CARD_OK,
	COPY_CARD_ERROR,
};


enum
{
	CHECK_CARD_LOCK_STATE,				//检测卡锁的状态
	SET_LOCK_CARD_OPEN_STATE,	//开始骑行
	SET_LOCK_CARD_CLOSE_STATE,	//结束骑行，更新卡状态
	SET_CARD_ERROR_STATE,					// 设置卡异常状态--锁定卡
};

enum
{
	CARD_UNACTIVATION_STATE,
	CARD_ACTIVATION_STATE,
	CARD_LOCK_STATE,
	
};


enum
{
	NFC_CARD_LOCK_CLOSE,
	NFC_CARD_LOCK_OPEN,
};

enum
{
	REG_CARD_STATE,
	REG_LOCK_STATE,
	REG_MONEY_LOW_LOW,
	REG_MONEY_LOW_HIGH,
	REG_MONEY_HIGH_LOW,
	REG_MONEY_HIGH_HIGH,

	REG_RTIME_LOW_LOW,
	REG_RTIME_LOW_HIGH,
	REG_RTIME_HIGH_LOW,
	REG_RTIME_HIGH_HIGH,

};


extern void nfc_card_init(void);
extern  char set_nfc_wallet_block(void);
extern  char write_money_to_card(unsigned char dd_mode);
extern  char get_card_infor(void);





#endif

