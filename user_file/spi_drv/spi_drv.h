#ifndef _SPI_DRV_H_
#define _SPI_DRV_H_

#include "nrf_delay.h"
#include "nrf6310.h"
#include "ble_service.h"

extern void SpiInit(void);
extern unsigned char ReadRawRC(unsigned char Address);
extern void WriteRawRC(unsigned char Address, unsigned char Data);
extern void SpiReset(void);
extern void SetBitMask(unsigned char reg,unsigned char mask);  
extern void ClearBitMask(unsigned char reg,unsigned char mask);

#endif

