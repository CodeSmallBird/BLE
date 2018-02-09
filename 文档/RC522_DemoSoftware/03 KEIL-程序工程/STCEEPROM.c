#include "STCEEPROM.h"

#define ISP_BYTE_READ    1		//ISP字节读
#define ISP_BYTE_PROGRAM 2		//ISP直接编程
#define ISP_SECTOR_ERASE 3		//ISP扇区擦除

/********************************************************************
* 名称 : ISP_ERASE()
* 功能 : EEPROM的扇区擦除
* 输入 : Address（擦除扇区的地址)
* 输出 : 无
***********************************************************************/
void ISP_ERASE(Address)
{
	ISP_ADDRL = Address;
	ISP_ADDRH = Address >> 8;
	ISP_CONTR = 0x81;				
	ISP_CMD = ISP_SECTOR_ERASE;	
	ISP_TRIG = 0x46;	//PDF手册中说先像 ISP_TRIG 中写入 0x46, 然后写入 0xb9
	ISP_TRIG = 0xb9;
	_nop_();
	_nop_();	
}

/********************************************************************
* 名称 : ISP_READ()
* 功能 : 从EEPROM相应的地址中读出值
* 输入 : Address(想要读取EEPROM的地址）
* 输出 : Read_Data（从EEPROM读出的值）
*********************************************************************/
unsigned int ISP_READ(Address)
{
	unsigned char Read_Data;
	ISP_ADDRL = Address;
	ISP_ADDRH = Address >> 8;
	EA = 0;	
	ISP_CONTR = 0x81;
	ISP_CMD = ISP_BYTE_READ;
	ISP_TRIG = 0x46;
	ISP_TRIG = 0xb9;
	_nop_();
	_nop_();
	Read_Data = ISP_DATA;
	return Read_Data;	
}

/********************************************************************
* 名称 : ISP_PROGRAM()
* 功能 : 实现EEPROM的字节编程
* 输入 : Address为编程的地址，Data_temp为想要编程的值
* 输出 : 无
***********************************************************************/
void ISP_PROGRAM(Address, Data_temp)
{
	ISP_DATA = Data_temp;
	ISP_ADDRL = Address;
	ISP_ADDRH = Address >> 8;
	ISP_CONTR = 0x81;
	ISP_CMD = ISP_BYTE_PROGRAM;
	ISP_TRIG = 0x46;
	ISP_TRIG = 0xb9;
	_nop_();
	_nop_(); 	
}