#ifndef _DATA_TRANS_H_
#define _DATA_TRANS_H_
#include <stdbool.h>
#include <stdint.h>
#include"string.h"
#include"ble_service.h"
/*---------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------*/
typedef unsigned char U8;  
typedef unsigned char u8;  
#define TRUE 1
#define FALSE 0


#define UART_UART_SEND_LENGTH			30
#define UART_UART_REVC_LENGTH			30
#define UART_REVC_DATA_SIZE				20
#define UART_UART_REVC_FIFO_LENGTH		20


/*		数据接收状态	*/
enum{
	UART_REVC_STATE_START_CR,
	UART_REVC_STATE_START_LF,
	UART_REVC_STATE_DATA,
	UART_REVC_STATE_TAIL1,
	UART_REVC_STATE_TAIL2
	
};

#define TRANS_DATA_LENGTH 30

typedef struct{
	struct{
		u8 data[TRANS_DATA_LENGTH];
		u8 length;
	}help_to_app;
	struct{
		u8 data[TRANS_DATA_LENGTH];
		u8 length;
	}app_to_help;
}TRANS_DATA;
extern TRANS_DATA trans_data;


typedef struct{
	U8 Mode;
	U8 Time;
	U8 State;
	U8 SendState;
	U8 RevcState;
	struct{
		U8 	Data[UART_UART_SEND_LENGTH];
		U8	Read;
		U8	Write;
		U8 	Length;
	}Send;
	struct{
		U8 	Data[UART_UART_REVC_LENGTH];
		U8	Read;
		U8	Write;
		U8 	Length;
	}Rece;	
	struct{
		U8 	Data[UART_UART_REVC_FIFO_LENGTH];
		U8 	Length;
	}ReceFifo;
}UART_UART_FORMAT;

enum
{
	HEAD_BYTE_1,
	HEAD_BYTE_2,
	DATA_LEN,
	DATA_DEAL,
	TREAIN_CHECK,
	TRAIN_END
};


#define UART_POLLING_TIME							8

#define GET_UART_MODE							(Uart.Mode)
#define SET_UART_MODE(x)						(Uart.Mode = x)

#define GET_UART_TIME							(Uart.Time)
#define SET_UART_TIME(x)						(Uart.Time = x/UART_POLLING_TIME)

#define GET_UART_STATE							(Uart.State)
#define SET_UART_STATE(x)						(Uart.State = x)

#define GET_UART_SEND_STATE						(Uart.SendState)
#define SET_UART_SEND_STATE(x)					(Uart.SendState = x)

#define GET_UART_REVC_STATE						(Uart.RevcState)
#define SET_UART_REVC_STATE(x)					(Uart.RevcState = x)


extern UART_UART_FORMAT Uart;
extern void AppToHelpDataTran(uint8_t* p_buff,uint8_t len);
extern U8   UartPushReceiveByte(U8 Data);
extern U8   UartPopSendByte(U8 *Data);
extern void ProtocolSend(void);
extern void DataTrainsInit(void);
extern void DataTrainsPolling(void);
/*---------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------*/


#endif

