#include"uart_nfc.h"
NFC_UART nfc_uart;

uint8_t NFCUartPushReceiveByte(uint8_t Data)
{
	if(nfc_uart.Rece.Length < sizeof(nfc_uart.Rece.Data))
	{
		nfc_uart.Rece.Data[nfc_uart.Rece.Write++] = Data;
		if(nfc_uart.Rece.Write >= sizeof(nfc_uart.Rece.Data))
			nfc_uart.Rece.Write = 0;
		nfc_uart.Rece.Length++;
		return true;
	}
	return false;
}

uint8_t NFCUartPopReceiveByte(uint8_t *Data)
{
	uint8_t Result;

	Result = false;
	if(nfc_uart.Rece.Length)
	{	
		*Data = nfc_uart.Rece.Data[nfc_uart.Rece.Read++];
		if(nfc_uart.Rece.Read >= sizeof(nfc_uart.Rece.Data))
			nfc_uart.Rece.Read = 0;
		nfc_uart.Rece.Length--;
		Result = true;
	}
	return Result;
}


/*
串口协议：
0xaa55---2byte--头
len----1byte--数据长度
datas--len byte--数据内容

*/

void nfc_recv_data_deal(void)
{
#if 0
	uint8_t Data;
	while(NFCUartPopReceiveByte(&Data))
	{
		switch(nfc_uart.DealState)
		{
			case HEAD_DATA_STEP_1:
				if(Data == 0xaa)
					nfc_uart.DealState = HEAD_DATA_STEP_1;
				break;

			case HEAD_DATA_STEP_2:
				if(Data == 0x55)
					nfc_uart.DealState = HEAD_DATA_STEP_2;
				break;	
			case LEN_DATA_STEP:
				if(Data == 0x55)
					nfc_uart.DealState = LEN_DATA_STEP_1;
				break;
				
			case RECV_DATA_STEP:
				nb_uart.ReceFifo.Data[nb_uart.ReceFifo.Length++] =Data;
				break;

			case TAIL_DATA_STEP:
				if(Data == 0x55)
					nb_uart.ReceFifo.Data[nb_uart.ReceFifo.Length++] =Data;
				break;

				
		}
	}
#endif
	
}























