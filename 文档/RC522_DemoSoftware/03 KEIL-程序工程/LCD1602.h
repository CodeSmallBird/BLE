#ifndef __LCD1602_H__
#define __LCD1602_H__

#include<reg52.h>

sbit LCD1602_RS = P1^0;
sbit LCD1602_RW = P1^1;
sbit LCD1602_E  = P1^2;
#define  LCD1602_Data  P0

void delay_50us_1602(unsigned int t);
void LCD1602_write_com(unsigned char com);
void LCD1602_write_data(unsigned int dat);
void LCD1602_init(void);
void LCD1602_Clear(void);
/*******************************************
�������ƣ�Disp_ZF
��    �ܣ�����Һ����ʾ�ַ�
��    ����addr--��ʾλ�õ��׵�ַ
          pt--ָ����ʾ���ݵ�ָ��
          num--��ʾ�ַ�����
����ֵ  ����
********************************************/
void LCD1602_Disp_ZF(unsigned char addr,const unsigned char * pt,unsigned char num); 

#endif
