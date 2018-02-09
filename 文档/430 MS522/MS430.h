//MICRO DEFIN

#pragma once
#include "msp430x44x.h"

#define UCLK_0   P3OUT &= ~(1<<3)
#define UCLK_1   P3OUT |=  (1<<3)
#define SIMO_0   P3OUT &= ~(1<<1)
#define SIMO_1   P3OUT |=  (1<<1)
#define CS_0     P3OUT &= ~(1<<0)
#define CS_1     P3OUT |=  (1<<0)
#define SOMI     ((P3IN & BIT2)>>2)


#define UCLK2_0   P6OUT &= ~(1<<1)
#define UCLK2_1   P6OUT |=  (1<<1)
#define SIMO2_0   P6OUT &= ~(1<<2)
#define SIMO2_1   P6OUT |=  (1<<2)
#define CS2_0     P6OUT &= ~(1<<0)
#define CS2_1     P6OUT |=  (1<<0)
#define SOMI2     ((P6IN & BIT3)>>3)

void delay(int x)
{
  for(int i=0;i<=x;i++);
}
void MS430Init(void)
{
    //SPI 1
    P3DIR  |=  (1<<1);          //P3.1 SIMO    
    P3DIR  &= ~(1<<2);          //P3.2 SOMI
    P3DIR  |=  (1<<3);          //P3.3 UCLK
    P3DIR  |=  (1<<0);          //P3.0 CS
    //SPI 2
    P6DIR  |=  (1<<2);          //P3.2 SIMO    
    P6DIR  &= ~(1<<3);          //P6.3 SOMI
    P6DIR  |=  (1<<1);          //P6.1 UCLK
    P6DIR  |=  (1<<0);          //P6.0 CS
    //SIGIN & SIGOUT
    P6DIR  &= ~(1<<6);          //P6.6 SIGOUT
    P6DIR  |=  (1<<7);          //P6.7 SIGIN
    
    CS_1;
    UCLK_0;
    SIMO_0;
    
    CS2_1;
    UCLK2_0;
    SIMO2_0;
       
}
//SPI 1
char ReadReg(char Address)
{
    char RegData;
    CS_0;
    UCLK_0;
    delay(5);
    
    Address = (Address << 1)|0x80;
    for(int i=0;i<8;i++)                     //write address
    {
      if((Address&0x80)==0x80)
        SIMO_1;
      else
        SIMO_0;
      UCLK_1;
      Address <<= 1;
      UCLK_0;
    }
    
    for(int i=0;i<8;i++)                    //read data
    {
      UCLK_1;
      RegData <<=1;
      RegData |= SOMI;
      UCLK_0;
    }
    delay(5);
    CS_1;
    
    return RegData;
}

void WriteReg(char Address,char Data)
{
    
    CS_0;
    UCLK_0;
    delay(5);
    
    Address  =  Address << 1 ;
    Address  = (Address&(~0x80));

    for(int i=0;i<8;i++)                     //write address
    {
      if((Address&0x80)==0x80)
        SIMO_1;
      else
        SIMO_0;
      UCLK_1;
      Address <<= 1;
      UCLK_0;
    }
    
    for(int i=0;i<8;i++)                     //write data
    {
      if((Data&0x80)==0x80)
        SIMO_1;
      else
        SIMO_0;
      UCLK_1;
      Data <<= 1;
      UCLK_0;
    }
    
    delay(5);
    CS_1;
    
}
//SPI 2
char ReadReg2(char Address)
{
    char RegData;
    CS2_0;
    UCLK2_0;
    delay(5);
    
    Address = (Address << 1)|0x80;
    for(int i=0;i<8;i++)                     //write address
    {
      if((Address&0x80)==0x80)
        SIMO2_1;
      else
        SIMO2_0;
      UCLK2_1;
      Address <<= 1;
      UCLK2_0;
    }
    
    for(int i=0;i<8;i++)                    //read data
    {
      UCLK2_1;
      RegData <<=1;
      RegData |= SOMI2;
      UCLK2_0;
    }
    delay(5);
    CS2_1;
    
    return RegData;
}

void WriteReg2(char Address,char Data)
{
    
    CS2_0;
    UCLK2_0;
    delay(5);
    
    Address  =  Address << 1 ;
    Address  = (Address&(~0x80));

    for(int i=0;i<8;i++)                     //write address
    {
      if((Address&0x80)==0x80)
        SIMO2_1;
      else
        SIMO2_0;
      UCLK2_1;
      Address <<= 1;
      UCLK2_0;
    }
    
    for(int i=0;i<8;i++)                     //write data
    {
      if((Data&0x80)==0x80)
        SIMO2_1;
      else
        SIMO2_0;
      UCLK2_1;
      Data <<= 1;
      UCLK2_0;
    }
    
    delay(5);
    CS2_1;
    
}


/*
char ReadFIFO(char Address,char LenByte)
{
    char FIFOData;
    CS_0;
    UCLK_0;
    delay(5);
    
    Address = (Address << 1)|0x80;
    for(int i=0;i<8;i++)                     //write address
    {
      if((Address&0x80)==0x80)
        SIMO_1;
      else
        SIMO_0;
      UCLK_1;
      Address <<= 1;
      UCLK_0;
    }
    
    for(int i=0;i<8;i++)                    //read data
    {
      UCLK_1;
      FIFOData <<=1;
      FIFOData |= SOMI;
      UCLK_0;
    }
    delay(5);
    CS_1;
    
    return FIFOData;
}
*/