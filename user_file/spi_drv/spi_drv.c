#include"spi_drv.h"


#define MOSI	12
#define MISO	13
#define SCK		14
#define NSS     15

#define UCLK_OUT	nrf_gpio_cfg_output(SCK)
#define UCLK_0   	nrf_gpio_pin_clear(SCK)
#define UCLK_1   	nrf_gpio_pin_set(SCK)


#define SIMO_OUT	nrf_gpio_cfg_output(MOSI)
#define SIMO_0   	nrf_gpio_pin_clear(MOSI)
#define SIMO_1   	nrf_gpio_pin_set(MOSI)

#define CS_OUT		nrf_gpio_cfg_output(NSS)
#define CS_0    	nrf_gpio_pin_clear(NSS)
#define CS_1    	nrf_gpio_pin_set(NSS)

#define SOMI_IN     nrf_gpio_cfg_input(MISO,NRF_GPIO_PIN_PULLUP)
#define SOMI    	nrf_gpio_pin_read(MISO)
#define SOMI_OUT	nrf_gpio_cfg_output(MISO)
#define SOMI_0   	nrf_gpio_pin_clear(MISO)
#define SOMI_1   	nrf_gpio_pin_set(MISO)

#define CNT_TIME	2		//2
void delay(int x)
{
  for(int i=0;i<=x;i++);
}
void SpiInit(void)
{
    //SPI 1
	SIMO_OUT;
	SOMI_IN;
	UCLK_OUT;
	CS_OUT;
    
    CS_1;
    UCLK_0;
    SIMO_0;       
}

void SpiReset(void)
{
    CS_0;
    UCLK_0;
    SOMI_OUT;   
	SOMI_0;  
}

//SPI 1
unsigned char ReadRawRC(unsigned char Address)
{
    unsigned char RegData;
    CS_0;
    UCLK_0;
    delay(CNT_TIME);
    
    Address = (Address << 1)|0x80;
    for(unsigned char i=0;i<8;i++)                     //write address
    {
      if((Address&0x80)==0x80)
        SIMO_1;
      else
        SIMO_0;
      UCLK_1;
      Address <<= 1;
      UCLK_0;
    }
    
    for(unsigned char i=0;i<8;i++)                    //read data
    {
      UCLK_1;
      RegData <<=1;
      RegData |= SOMI;
      UCLK_0;
    }
    delay(CNT_TIME);
    CS_1;
    
    return RegData;
}

void WriteRawRC(unsigned char Address, unsigned char Data)
{
    
    CS_0;
    UCLK_0;
    delay(CNT_TIME);
    
    Address  =  Address << 1 ;
    Address  = (Address&(~0x80));

    for(unsigned char i=0;i<8;i++)                     //write address
    {
      if((Address&0x80)==0x80)
        SIMO_1;
      else
        SIMO_0;
      UCLK_1;
      Address <<= 1;
      UCLK_0;
    }
    
    for(unsigned char i=0;i<8;i++)                     //write data
    {
      if((Data&0x80)==0x80)
        SIMO_1;
      else
        SIMO_0;
      UCLK_1;
      Data <<= 1;
      UCLK_0;
    }
    
    delay(CNT_TIME);
    CS_1;
    
}

/////////////////////////////////////////////////////////////////////
//功    能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
/////////////////////////////////////////////////////////////////////
void SetBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 

