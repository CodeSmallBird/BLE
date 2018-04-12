#include "data_trans.h"

#if defined(__DEBUG__)
	#define __DEBUG_Protocol
#endif

TRANS_DATA trans_data;

UART_UART_FORMAT Uart;

#define COM_UART_HEADER1					0xAA
#define COM_UART_HEADER2					0xAA


#define COM_UART_TAIL1						0x55
#define COM_UART_TAIL2						0x55

extern void simple_uart_put(uint8_t cr);


/*-----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/

void UartInit(void)
{
	memset(&Uart, 0, sizeof(Uart));
	memset(&trans_data, 0, sizeof(trans_data));
}
/*-----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/
U8 UartPushSendByte(U8 Data)
{
//	U8 Inter;
	U8 Result;

	Result = FALSE;
	//DISABLE_INTERRUPT;
	if(Uart.Send.Length < sizeof(Uart.Send.Data))
	{
		Uart.Send.Data[Uart.Send.Write++] = Data;
		if(Uart.Send.Write >= sizeof(Uart.Send.Data))
			Uart.Send.Write = 0;
		Uart.Send.Length++;
		Result = TRUE;
	}
	//ENABLE_INTERRUPT;
	return Result;
}
U8 UartPopSendByte(U8 *Data)
{
	if(Uart.Send.Length)
	{	
		*Data = Uart.Send.Data[Uart.Send.Read++];
		if(Uart.Send.Read >= sizeof(Uart.Send.Data))
			Uart.Send.Read = 0;
		Uart.Send.Length--;
		return TRUE;
	}
	return FALSE;
}

U8 UartPushReceiveByte(U8 Data)
{
	if(Uart.Rece.Length < sizeof(Uart.Rece.Data))
	{
		Uart.Rece.Data[Uart.Rece.Write++] = Data;
		if(Uart.Rece.Write >= sizeof(Uart.Rece.Data))
			Uart.Rece.Write = 0;
		Uart.Rece.Length++;
		return TRUE;
	}
	return FALSE;
}
U8 UartPopReceiveByte(U8 *Data)
{
//	U8 Inter;
	U8 Result;

	Result = FALSE;
	//DISABLE_INTERRUPT;
	if(Uart.Rece.Length)
	{	
		*Data = Uart.Rece.Data[Uart.Rece.Read++];
		if(Uart.Rece.Read >= sizeof(Uart.Rece.Data))
			Uart.Rece.Read = 0;
		Uart.Rece.Length--;
		Result = TRUE;
	}
	//ENABLE_INTERRUPT;
	return Result;
}





/*-----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/
void UartPolling(void)
{
	U8 Data;
	static U8 len = 0;
	
#if 0//defined(__DEBUG_Protocol)
	{
		U8 i;
		if(Uart.Rece.Length>0)
		{
			ie_printf("Recv_data:\r\n");
			for(i=0;i<Uart.Rece.Length;i++)
			{
				ie_printf("0x%x ",(U16)Uart.Rece.Data[Uart.Rece.Read+i]);
			}
		}
	}
#endif
	while(UartPopReceiveByte(&Data))
	{
		switch(GET_UART_STATE)
		{
			case HEAD_BYTE_1:
				memset(&trans_data.help_to_app,0,sizeof(trans_data.help_to_app));
				if(Data == 0x55)
				{
					trans_data.help_to_app.data[trans_data.help_to_app.length++] = Data;
					SET_UART_STATE(HEAD_BYTE_2);
				}
				break;
			case HEAD_BYTE_2:
				if(Data == 0xAA)
				{
					trans_data.help_to_app.data[trans_data.help_to_app.length++] = Data;
					SET_UART_STATE(DATA_LEN);
				}
				else
					SET_UART_STATE(HEAD_BYTE_1);
				break;
			case DATA_LEN:
				trans_data.help_to_app.data[trans_data.help_to_app.length++] = Data;
				len = Data;
				SET_UART_STATE(DATA_DEAL);
				break;
			case DATA_DEAL:
				if(len)
				{
					len--;
					trans_data.help_to_app.data[trans_data.help_to_app.length++] = Data;
					if(len == 0)
					{
						SET_UART_STATE(TREAIN_CHECK);
					}
				}
				else
				{
						SET_UART_STATE(HEAD_BYTE_1);
				}
				break;
			case TREAIN_CHECK:
				trans_data.help_to_app.data[trans_data.help_to_app.length++] = Data;
				SET_UART_STATE(TRAIN_END);
				break;
			case TRAIN_END:
				trans_data.help_to_app.data[trans_data.help_to_app.length++] = Data;
				//发送数据到APP
				send_data_to_phone();
				SET_UART_STATE(HEAD_BYTE_1);
				break;
			default:
				break;

		}
	}
}


void ProtocolSend(void)
{
	U8 send_data = 0;
	for(int i=0;i<trans_data.app_to_help.length;i++)
		UartPushSendByte(trans_data.app_to_help.data[i]);

	while(UartPopSendByte(&send_data))
	{
		simple_uart_put(send_data);
	}
	
}

/*-----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/

void AppToHelpDataTran(uint8_t* p_buff,uint8_t len)
{
	if((p_buff[0] == 0x55)&&(p_buff[1] == 0xAA))
	{
		memcpy(trans_data.app_to_help.data,p_buff,len);
		trans_data.app_to_help.length = len;
		ProtocolSend();
	}
}
/*-----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/

void DataTrainsInit(void)
{
	UartInit();
}

void DataTrainsPolling(void)
{
	UartPolling();
}










