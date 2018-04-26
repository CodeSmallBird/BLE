#ifndef _LENOVO_H_
#define _LENOVO_H_

#include "simple_uart.h"

#define NB_SEND_LENGTH                  200
#define NB_REVC_LENGTH                  200
#define NB_REVC_FIFO_LENGTH             200

typedef struct
{
	uint8_t send_buff[NB_SEND_LENGTH];
	struct{
		uint8_t Data[NB_REVC_LENGTH];
		uint8_t	Read;
		uint8_t	Write;
		uint8_t Length;
	}Rece;	
	struct{
		uint8_t Data[NB_REVC_FIFO_LENGTH];
		uint8_t Length;
	}ReceFifo;
	
}NB_UART;

extern NB_UART nb_uart;

extern uint8_t NbUartPopReceiveByte(uint8_t *Data);
extern uint8_t NbUartPushReceiveByte(uint8_t Data);






#endif
