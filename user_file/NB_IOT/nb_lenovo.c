#include"lenovo.h"
NB_UART nb_uart;

uint8_t NbUartPushReceiveByte(uint8_t Data)
{
	if(nb_uart.Rece.Length < sizeof(nb_uart.Rece.Data))
	{
		nb_uart.Rece.Data[nb_uart.Rece.Write++] = Data;
		if(nb_uart.Rece.Write >= sizeof(nb_uart.Rece.Data))
			nb_uart.Rece.Write = 0;
		nb_uart.Rece.Length++;
		return true;
	}
	return false;
}

uint8_t NbUartPopReceiveByte(uint8_t *Data)
{
	uint8_t Result;

	Result = false;
	if(nb_uart.Rece.Length)
	{	
		*Data = nb_uart.Rece.Data[nb_uart.Rece.Read++];
		if(nb_uart.Rece.Read >= sizeof(nb_uart.Rece.Data))
			nb_uart.Rece.Read = 0;
		nb_uart.Rece.Length--;
		Result = true;
	}
	return Result;
}




























