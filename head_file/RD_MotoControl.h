/************************************************************
  Copyright (C), Shenzhen Bonten Health Techology Co.,Ltd.
  FileName: 		 ble_config.h
  Author: Kevin  Version :  V0.0    Date:2015/03/18
  Description:     //模块描述
  Version:         //模块版本�?
  Function List:   //主要函数及其功能
    1. -------
  History:         //历史修给记录
     <author>   <time>   <version >       <desc>
***********************************************************/

#ifndef RD_MOTOCONTROL_H 
#define RD_MOTOCONTROL_H
#include "common.h"

#define MOTO_TIME 			1									//500ms
#define MOTO_TIMEOUT	     300
#define LOCK_BUZZER_CNT     6									
#define UNLOCK_BUZZER_CNT 	8  									


enum
{
	BUZZER_NORMAL,
	BUZZER_ERROR,
};

enum
{
	TURN_POSITIVE,
	TURN_REVERSE,
};

extern void Rd_MotoInit(void);
extern void factory_test(void);

extern void moto_timer_start(void);
extern void moto_timer_stop(void);
void moto_start(unsigned char direction);

extern void buzzer_start(uint8_t mode);
extern void buzzer_timer_create(void);



#endif //RD_MOTOCONTROL_H



