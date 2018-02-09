#ifndef _LENOVO_H_
#define _LENOVO_H_

#include "simple_uart.h"

#define NFC_SEND_LENGTH              	100
#define NFC_REVC_LENGTH              	100
#define NFC_REVC_FIFO_LENGTH             100

typedef struct
{
	uint8_t send_buff[NFC_SEND_LENGTH];
	struct{
		uint8_t Data[NFC_REVC_LENGTH];
		uint8_t	Read;
		uint8_t	Write;
		uint8_t Length;
	}Rece;	
	struct{
		uint8_t Data[NFC_REVC_FIFO_LENGTH];
		uint8_t Length;
	}ReceFifo;
	uint8_t DealState;
	
}NFC_UART;


enum
{
	HEAD_DATA_STEP_1,
	HEAD_DATA_STEP_2,
	LEN_DATA_STEP,
	RECV_DATA_STEP,
};



extern NFC_UART nfc_uart;
extern uint8_t NFCUartPopReceiveByte(uint8_t *Data);
extern uint8_t NFCUartPushReceiveByte(uint8_t Data);






#endif
