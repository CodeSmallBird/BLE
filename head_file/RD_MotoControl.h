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

#ifndef GLOBAL_VARIABLE_H 
#define GLOBAL_VARIABLE_H
#include "common.h"


typedef enum
{
	SHAKE_STOP = 0,//停止震动
	SHAKE_ODD,     //奇数震动
	SHAKE_EVEN,    //偶数震动
	SHAKE_RANDOM,  //随机震动
	SHAKE_FACE,    //正面震动
	SHAKE_BACK,    //反面震动
	SHAKE_SEQUENCE,//顺序震动
	SHAKE_OPPOINT  //指定震动
}Moto_Shake_Type;

//马达初始�?
void Rd_MotoInit(void);

//moto启动
void moto_timer_start(void);

//moto停止
void moto_timer_stop(void);

//设置马达震动类型
void Rd_MotoShakeTypeSet(bool isShake,uint8_t shakeType,uint8_t* shakeSel,bool shakeDirection);

//马达震动执行
void Rd_MotoShake(void);
void Rd_MotoShakeStart(void);
void Rd_MotoShakeMode(bool isShake,uint8_t shakeType,uint8_t shakeSel,bool shakeDirection);
uint8_t Rd_MotoGetTimeCount(void);
void Rd_MotoSetShakeSwitchMode(uint8_t nmode);



#endif //GLOBAL_VARIABLE_H



