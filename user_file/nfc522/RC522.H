#ifndef _RC_522_H_
#define _RC_522_H_

#include "nrf_delay.h"
#include "nrf6310.h"
#include "ble_service.h"
#include "spi_drv.h "


#if 1
#define MAXRLEN 64  

#define FIND_CARD_TIME				1400//50
#define READ_CARD_TIME				1400//300
#define WRITE_CARD_TIME				2000//1400  //1200//600
#else
#define MAXRLEN 64  

#define FIND_CARD_TIME				50
#define READ_CARD_TIME				300
#define WRITE_CARD_TIME			   1200//	1200  //1200//600
#endif

/////////////////////////////////////////////////////////////////////
//函数原型
/////////////////////////////////////////////////////////////////////
char PcdReset(void);
void PcdAntennaOn(void);
void PcdAntennaOff(void);
char PcdRequest(unsigned char req_code,unsigned char *pTagType);   
char PcdAnticoll(unsigned char *pSnr);
char PcdSelect(unsigned char *pSnr);         
char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr);     
char PcdRead(unsigned char addr,unsigned char *pData);     
char PcdWrite(unsigned char addr,unsigned char *pData);    
char PcdValue(unsigned char dd_mode,unsigned char addr,unsigned char *pValue);   
char PcdBakValue(unsigned char sourceaddr, unsigned char goaladdr);                                 
char PcdHalt(void);
char PcdComMF522(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int  *pOutLenBit,
                 unsigned int WaitTime);
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData);
void WriteRawRC(unsigned char Address,unsigned char value);
unsigned char ReadRawRC(unsigned char Address); 
void SetBitMask(unsigned char reg,unsigned char mask); 
void ClearBitMask(unsigned char reg,unsigned char mask); 
char M500PcdConfigISOType(unsigned char type);
char PcdBakValue(unsigned char sourceaddr, unsigned char goaladdr);
char PcdValue(unsigned char dd_mode,unsigned char addr,unsigned char *pValue);
/////////////////////////////////////////////////////////////////////
//MF522命令字
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE              0x00               //取消当前命令
#define PCD_AUTHENT           0x0E               //验证密钥
#define PCD_RECEIVE           0x08               //接收数据
#define PCD_TRANSMIT          0x04               //发送数据
#define PCD_TRANSCEIVE        0x0C               //发送并接收数据
#define PCD_RESETPHASE        0x0F               //复位
#define PCD_CALCCRC           0x03               //CRC计算

/////////////////////////////////////////////////////////////////////
//Mifare_One卡片命令字
/////////////////////////////////////////////////////////////////////
#define PICC_REQIDL           0x26               //寻天线区内未进入休眠状态
#define PICC_REQALL           0x52               //寻天线区内全部卡
#define PICC_ANTICOLL1        0x93               //防冲撞
#define PICC_ANTICOLL2        0x95               //防冲撞
#define PICC_AUTHENT1A        0x60               //验证A密钥
#define PICC_AUTHENT1B        0x61               //验证B密钥
#define PICC_READ             0x30               //读块
#define PICC_WRITE            0xA0               //写块
#define PICC_DECREMENT        0xC0               //扣款
#define PICC_INCREMENT        0xC1               //充值
#define PICC_RESTORE          0xC2               //调块数据到缓冲区
#define PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define PICC_HALT             0x50               //休眠

/////////////////////////////////////////////////////////////////////
//MF522 FIFO长度定义
/////////////////////////////////////////////////////////////////////
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte

/////////////////////////////////////////////////////////////////////
//MF522寄存器定义
/////////////////////////////////////////////////////////////////////
// PAGE 0
#define     RFU00                 0x00    
#define     CommandReg            0x01    
#define     ComIEnReg             0x02    
#define     DivlEnReg             0x03    
#define     ComIrqReg             0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     RFU0F                 0x0F
// PAGE 1     
#define     RFU10                 0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     RFU1A                 0x1A
#define     RFU1B                 0x1B
#define     MifareReg             0x1C
#define     RFU1D                 0x1D
#define     RFU1E                 0x1E
#define     SerialSpeedReg        0x1F
// PAGE 2    
#define     RFU20                 0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     RFU23                 0x23
#define     ModWidthReg           0x24
#define     RFU25                 0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsCfgReg            0x28
#define     ModGsCfgReg           0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
// PAGE 3      
#define     RFU30                 0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     RFU3C                 0x3C   
#define     RFU3D                 0x3D   
#define     RFU3E                 0x3E   
#define     RFU3F		  0x3F


#define Idle	0x00
#define Mem		0x01
#define RandomID	0x02
#define CalcCRC	0x03
#define Transmit	0x04
#define NoCmdChange	0x07
#define Receive	0x08
#define Transceive	0x0C
#define MFAuthent	0x0E
#define SoftReset	0x0F
/////////////////////////////////////////////////////////////////////
//和MF522通讯时返回的错误代码
/////////////////////////////////////////////////////////////////////
#define MI_OK                          0
#define MI_NOTAGERR                    ((char)-1)
#define MI_ERR                         ((char)-2)




#define NRSTPD_PIN	18	

#if defined(NFC_POWER_PIN)
#define NRSTPD_INIT   		do{nrf_gpio_cfg_output(NFC_POWER_PIN); nrf_gpio_cfg_output(NRSTPD_PIN);}while(0)  	
#define NRSTPD_ON 		    do{nrf_gpio_pin_set(NFC_POWER_PIN); nrf_gpio_pin_set(NRSTPD_PIN);}while(0)  	//nrf_gpio_pin_set(NRSTPD_PIN);
#define NRSTPD_OFF 		    do{nrf_gpio_pin_clear(NRSTPD_PIN);nrf_gpio_pin_clear(NFC_POWER_PIN);}while(0)  			//nrf_gpio_pin_clear(NRSTPD_PIN);
#else
#define NRSTPD_INIT  	nrf_gpio_cfg_output(NRSTPD_PIN);
#define NRSTPD_ON 		nrf_gpio_pin_set(NRSTPD_PIN);
#define NRSTPD_OFF 		nrf_gpio_pin_clear(NRSTPD_PIN);
#endif

extern unsigned char Pcd_SetTimer(unsigned long delaytime);//设定超时时间（ms）


#define s8  char
#define u8 unsigned char
#define u16 unsigned int


#endif

