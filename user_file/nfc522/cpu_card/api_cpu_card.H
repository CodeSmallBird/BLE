#ifndef 	_API_CPU_CARD__H_
#define 	_API_CPU_CARD__H_

#include"rc522.h"
#include "nrf_delay.h"
#include "nrf6310.h"
#include "ble_service.h"
#include "RD_MotoControl.h"
#include ".\cpu_card\des.h"

#define s8  char
#define u8 unsigned char
#define u16 unsigned int

enum
{
	CPU_CARD_READ_MODE,
	CPU_CARD_WRITE_MODE,
	CPU_CARD_CREAT_MODE,
	CPU_CARD_PAY_MODE,
	CPU_CARD_EXIT,
};


enum
{
	CHOOSE_MF_FILE_MODE,
	GET_RANDOM_DATA_MODE,
	EXTERNAL_CONFIRM_MODE,
	CHOOSE_BINARY_FILE_MODE,
	READ_BINARY_FILE_MODE,
	WRITE_BINARY_FILE_MODE,
	CREATE_BINARY_FILE,
	WORK_PROCESS_EXIT,
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
	REG_MONEY_HIGH_HIGH,
	REG_MONEY_HIGH_LOW,
	REG_MONEY_LOW_HIGH,
	REG_MONEY_LOW_LOW,

	REG_RTIME_HIGH_HIGH,
	REG_RTIME_HIGH_LOW,
	REG_RTIME_LOW_HIGH,
	REG_RTIME_LOW_LOW,
	
};
	
#define MAXRLEN 64
#define COST_MIN		1

typedef struct
{
	u8 Pcb; 				//CPU卡APDU指令分组号
	u8 ats[3];
	u8 cpu_type[2];		//卡片类型
	u8 card_snr[4];
	char work_mode;// = CPU_CARD_WRITE_MODE;
	char random_data[8];
	u8 reccv_buf[MAXRLEN];
	u16 recv_len;		//接收数据长度
	u8 send_buf[16];
	u8 send_length;
}CPU_CARD_INFOR;


extern void nfc_card_init(void);
//extern unsigned char 	set_nfc_wallet_block(void);
//extern unsigned char write_money_to_card(unsigned char dd_mode);
//extern unsigned    get_card_infor(void);
//extern unsigned char Pcd_SetTimer(unsigned long delaytime);//设定超时时间（ms）




#endif

