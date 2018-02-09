#include "LCD1602.h"

void delay_50us_1602(unsigned int t)
{
	unsigned char j;
	for(;t>0;t--)
		for(j=19;j>0;j--);
}

void LCD1602_write_com(unsigned char com) 
{
	LCD1602_E = 0;
	LCD1602_RS = 0;
	LCD1602_RW = 0;
	LCD1602_Data= com;
	delay_50us_1602(10);
	LCD1602_E = 1;
	delay_50us_1602(20);
	LCD1602_E = 0;
}

void LCD1602_write_data(unsigned int dat)
{
	LCD1602_E = 0;
	LCD1602_RS = 1;
	LCD1602_RW = 0;
	LCD1602_Data = dat;
	delay_50us_1602(10);
	LCD1602_E = 1;
	delay_50us_1602(20);
	LCD1602_E = 0;
}

void LCD1602_init(void)
{
   delay_50us_1602(300);
   LCD1602_write_com(0X38);
   delay_50us_1602(100);
   LCD1602_write_com(0X38);
   delay_50us_1602(100);
   LCD1602_write_com(0X38);
   LCD1602_write_com(0X38);
   LCD1602_write_com(0X08);
   LCD1602_write_com(0X01);
   LCD1602_write_com(0X06);
   LCD1602_write_com(0X0C);
   
}
void LCD1602_Clear(void)
{
	LCD1602_write_com(0X01);
}
/*******************************************
函数名称：Disp_ZF
功    能：控制液晶显示字符
参    数：addr--显示位置的首地址
          pt--指向显示数据的指针
          num--显示字符个数
返回值  ：无
********************************************/
void LCD1602_Disp_ZF(unsigned char addr,const unsigned char * pt,unsigned char num)
{
	unsigned char i;
	LCD1602_write_com(addr); 
	for(i = 0;i < (num*1);i++) 
		LCD1602_write_data(*(pt++)); 
} 
