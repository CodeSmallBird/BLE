/************************************************************
  Copyright (C), Shenzhen Bonten Health Techology Co.,Ltd.
  FileName: 		 ble_config.h
  Author: Kevin  Version :  V0.0    Date:2015/03/18
  Description:     //模块描述
  Version:         //模块版本号
  Function List:   //主要函数及其功能
    1. -------
  History:         //历史修给记录
     <author>   <time>   <version >       <desc>
***********************************************************/

#ifndef GLOBAL_VARIABLE_H 
#define GLOBAL_VARIABLE_H
#include "common.h"


//马达初始化
void Rd_MotoInit(void);

//moto启动
void moto_timer_start(void);
//moto停止
void moto_timer_stop(void);
void factory_test(void);



#endif //GLOBAL_VARIABLE_H



