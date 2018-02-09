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
�������ƣ�Disp_ZF
��    �ܣ�����Һ����ʾ�ַ�
��    ����addr--��ʾλ�õ��׵�ַ
          pt--ָ����ʾ���ݵ�ָ��
          num--��ʾ�ַ�����
����ֵ  ����
********************************************/
void LCD1602_Disp_ZF(unsigned char addr,const unsigned char * pt,unsigned char num)
{
	unsigned char i;
	LCD1602_write_com(addr); 
	for(i = 0;i < (num*1);i++) 
		LCD1602_write_data(*(pt++)); 
} 
