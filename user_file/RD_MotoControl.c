/********************************************************************************
 *  Copyright (C), Shenzhen Bonten Health Techology Co.,Ltd.										*
 *																																							*
 *	Author: Kevin    Version : V1.2    Date: 2015/06/10													*
 *																																							*
 * 	FileName: 		 	global_variable.c																						*
 * 	Description:    //模块描述																									*
 * 	Function List:  //主要函数及其功能																					*
 *   	1. -------																																*
 * 	History:         																														*
 *   <author>   <time>   <version>   <desc>																			*
 *		kevin		 15/06/10	   V 1.2		1. 修复快速切屏出现花屏问�?							  *
 ********************************************************************************/

#include "RD_MotoControl.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "nrf_gpio.h"
#include "nrf6310.h"
#include "data_struct.h"
#include "global_var.h"
#include"pwm.h"

#define APP_TIMER_PRESCALER            0                   													 /**< Value of the RTC1 PRESCALER register. */
#define MOTO_HANDLE_MEAS_INTERVAL      APP_TIMER_TICKS(500, APP_TIMER_PRESCALER)     /**< Battery level measurement interval (ticks). */

#define 	MOTE_MAX_NUM		7

extern uint16_t  n_time_shake;

static app_timer_id_t									 m_moto_handle_id;													   /**< 马达控制定时器ID.*/

static Moto_Ctr_Type                   M_Moto_Def               = {0};               /**< 马达定义 */
#if NOR_USER_MOTOR
static uint8_t                         moto_pin_sel             = 1;                 //选定某个引脚的马�?
#endif
#if NOR_USER_MOTOR
static uint8_t                         Random[20]               = {18,3,6,12,20,13,7,8,10,11,2,4,9,17,1,15,14,16,5,19};//随机�?
#endif

#if !defined(ADD_NRF_PWM_CTRL)	
static uint8_t                         Moto_Number[MOTE_MAX_NUM]          = {0,MOTO_PIN_01,MOTO_PIN_02,MOTO_PIN_03,MOTO_PIN_04,MOTO_PIN_05,
                                                                    MOTO_PIN_06,};
#endif 
uint8_t			nShake_mode = 0 ;

#if defined(ADD_WOR_MODE)
extern uint8_t  n_work_mode;
#endif

//设置马达震动类型 shakeDirection 0正向 1反向
void Rd_MotoShakeTypeSet(bool isShake,uint8_t shakeType,uint8_t* shakeSel,bool shakeDirection)
{
	M_Moto_Def.moto_is_shake = isShake;
	M_Moto_Def.moto_shake_direction = shakeDirection;
	M_Moto_Def.moto_shake_type = shakeType;
	M_Moto_Def.moto_select[2] = shakeSel[0];
	M_Moto_Def.moto_select[1] = shakeSel[1];
	M_Moto_Def.moto_select[0] = shakeSel[2];
	if(M_Moto_Def.moto_shake_type == SHAKE_SEQUENCE)
	{
	#if NOR_USER_MOTOR
		if(M_Moto_Def.moto_shake_direction)
		  moto_pin_sel = 0;
		else
			moto_pin_sel = 19;
	#endif
	}
#if !defined(ADD_NRF_PWM_CTRL)	

	for(uint8_t i=1;i<MOTE_MAX_NUM;i++)
		  nrf_gpio_pin_clear(Moto_Number[i]);
#endif	
	if(M_Moto_Def.moto_is_shake)
	{
		moto_timer_start();
	}
	else
	{
		moto_timer_stop();
	#if !defined(ADD_NRF_PWM_CTRL)	
		for(uint8_t i=1;i<MOTE_MAX_NUM;i++)
		  nrf_gpio_pin_clear(Moto_Number[i]);
	#endif
	}
}

void Rd_MotoShake_time(uint16_t ntime )
{
	if (ntime <= 0)
 	{
 		Rd_MotoShakeMode(false,SHAKE_STOP,0,0);
 	}
}

#if NOR_USER_MOTOR

//马达震动执行
void Rd_MotoShake(void)
{
	//static uint8_t PwmInter = 10;
	static bool    ShakeOrStop = false;
//	nrf_delay_ms(PwmInter);
	if(M_Moto_Def.moto_is_shake)
	{
		if(M_Moto_Def.moto_select[2]&0x08)
		{
		 if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[20]);
		 else
			nrf_gpio_pin_clear(Moto_Number[20]);
	  }
		
		if(M_Moto_Def.moto_select[2]&0x04)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[19]);
		else
			nrf_gpio_pin_clear(Moto_Number[19]);
	  }
		
		if(M_Moto_Def.moto_select[2]&0x02)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[18]);
		else
			nrf_gpio_pin_clear(Moto_Number[18]);
	  }
		
		if(M_Moto_Def.moto_select[2]&0x01)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[17]);
		else
			nrf_gpio_pin_clear(Moto_Number[17]);
	  }
		//=================================
		if(M_Moto_Def.moto_select[1]&0x80)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[16]);
		else
			nrf_gpio_pin_clear(Moto_Number[16]);
	  }
		
		if(M_Moto_Def.moto_select[1]&0x40)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[15]);
		else
			nrf_gpio_pin_clear(Moto_Number[15]);
	  }
		
		if(M_Moto_Def.moto_select[1]&0x20)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[14]);
		else
			nrf_gpio_pin_clear(Moto_Number[14]);
	  }
		
		if(M_Moto_Def.moto_select[1]&0x10)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[13]);
		else
			nrf_gpio_pin_clear(Moto_Number[13]);
	  }
		
		if(M_Moto_Def.moto_select[1]&0x08)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[12]);
		else
			nrf_gpio_pin_clear(Moto_Number[12]);
	  }
		
		if(M_Moto_Def.moto_select[1]&0x04)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[11]);
		else
			nrf_gpio_pin_clear(Moto_Number[11]);
	  }
		
		if(M_Moto_Def.moto_select[1]&0x02)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[10]);
		else
			nrf_gpio_pin_clear(Moto_Number[10]);
	  }
		
		if(M_Moto_Def.moto_select[1]&0x01)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[9]);
		else
			nrf_gpio_pin_clear(Moto_Number[9]);
  	}
		//===================================
		if(M_Moto_Def.moto_select[0]&0x80)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[8]);
		else
			nrf_gpio_pin_clear(Moto_Number[8]);
	  }
		
		if(M_Moto_Def.moto_select[0]&0x40)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[7]);
		else
			nrf_gpio_pin_clear(Moto_Number[7]);
	  }
		
		if(M_Moto_Def.moto_select[0]&0x20)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[6]);
		else
			nrf_gpio_pin_clear(Moto_Number[6]);
	  }
		
		if(M_Moto_Def.moto_select[0]&0x10)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[5]);
		else
			nrf_gpio_pin_clear(Moto_Number[5]);
	  }
		
		if(M_Moto_Def.moto_select[0]&0x08)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[4]);
		else
			nrf_gpio_pin_clear(Moto_Number[4]);
	  }
		
		if(M_Moto_Def.moto_select[0]&0x04)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[3]);
		else
			nrf_gpio_pin_clear(Moto_Number[3]);
	  }
		
		if(M_Moto_Def.moto_select[0]&0x02)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[2]);
		else
			nrf_gpio_pin_clear(Moto_Number[2]);
	  }
		
		if(M_Moto_Def.moto_select[0]&0x01)
		{
			if(ShakeOrStop)
			nrf_gpio_pin_set(Moto_Number[1]);
		else
			nrf_gpio_pin_clear(Moto_Number[1]);
	  }
		
		if(ShakeOrStop)
		 ShakeOrStop = false;
		else
		 ShakeOrStop = true;
	}
	else
	{
		ShakeOrStop = true;
	}
}

//马达震动奇数
void Rd_MotoShakeOdd(void)
{
	M_Moto_Def.moto_select[2] = 0x05;
	M_Moto_Def.moto_select[1] = 0x55;
	M_Moto_Def.moto_select[0] = 0x55;
}

//马达震动偶数
void Rd_MotoShakeEven(void)
{
	M_Moto_Def.moto_select[2] = 0x0A;
	M_Moto_Def.moto_select[1] = 0xAA;
	M_Moto_Def.moto_select[0] = 0xAA;
}

//马达震动随机
void Rd_MotoShakeRandom(void)
{
	static uint8_t i = 0;
	M_Moto_Def.moto_select[2] = 0x00;
	M_Moto_Def.moto_select[1] = 0x00;
	M_Moto_Def.moto_select[0] = 0x00;
	uint32_t pinSel = 1 << Random[i];
	M_Moto_Def.moto_select[2] = (uint8_t)(pinSel>>16);
	M_Moto_Def.moto_select[1] = (uint8_t)(pinSel>>8);;
	M_Moto_Def.moto_select[0] = (uint8_t)pinSel;
	i++;
	if(i == 20) i = 0;
}

//马达震动 正面
void Rd_MotoShakeFace(void)
{
	M_Moto_Def.moto_select[2] = 0x00;
	M_Moto_Def.moto_select[1] = 0x03;
	M_Moto_Def.moto_select[0] = 0xFF;
}

//马达震动 背面
void Rd_MotoShakeBack(void)
{
	M_Moto_Def.moto_select[2] = 0x0F;
	M_Moto_Def.moto_select[1] = 0xFC;
	M_Moto_Def.moto_select[0] = 0x00;
}

//马达震动 顺序
void Rd_MotoShakeSequence(void)
{
	static uint32_t count = 0;
	count++;
	if(M_Moto_Def.moto_shake_direction) //正向
	{
		if(count%1 == 0)
		{
			M_Moto_Def.moto_select[2] = 0x00;
	    M_Moto_Def.moto_select[1] = 0x00;
	    M_Moto_Def.moto_select[0] = 0x00;
			uint32_t pinSel = 1 << moto_pin_sel;
			M_Moto_Def.moto_select[2] = (uint8_t)(pinSel>>16);
	    M_Moto_Def.moto_select[1] = (uint8_t)(pinSel>>8);;
	    M_Moto_Def.moto_select[0] = (uint8_t)pinSel;
			moto_pin_sel++;
			if(moto_pin_sel == 20) moto_pin_sel = 0;
		}
	}
	else                                //反向
	{
		if(count%1 == 0)
		{
			M_Moto_Def.moto_select[2] = 0x00;
	    M_Moto_Def.moto_select[1] = 0x00;
	    M_Moto_Def.moto_select[0] = 0x00;
			uint32_t pinSel = 1 << moto_pin_sel;
			M_Moto_Def.moto_select[2] = (uint8_t)(pinSel>>16);
	    M_Moto_Def.moto_select[1] = (uint8_t)(pinSel>>8);;
	    M_Moto_Def.moto_select[0] = (uint8_t)pinSel;
			moto_pin_sel--;
			if(moto_pin_sel == 0xFF) moto_pin_sel = 19;
	  }
	}
	
	if(count >= 0xFFFFFFF8)count = 0;
}


//马达震动 指定某几个马�?
void Rd_MotoShakeOppoint(void)
{
}
#endif

void Rd_MotoGrade(uint8_t nShake)
{
	switch(nShake)
	{
		case 0:
			{
				nShake_mode = 5 ;	
			}
			break ;
		case 1:
			{
				nShake_mode = 5 ;
			}
			break ;
		case 2:
			{
				nShake_mode = 6 ;	
			}
			break ;
		case 3:
			{
				nShake_mode = 4 ;	
			}
			break ;
		case 4:
			{
				nShake_mode = 3;	
			}
			break ;
		case 5:
			{
				nShake_mode = 2 ;
			}
			break ;
		case 6:
			{
				nShake_mode = 1 ;
			}
			break ;
		default:
			{
				nShake_mode = 3 ;
			}
			break ;
	}
}

extern void send_data_to_phone(uint8_t cmd, uint8_t* data_buff, uint16_t size);

//设置马达震动类型 shakeDirection 0正向 1反向
void Rd_MotoShakeMode(bool isShake,uint8_t shakeType,uint8_t shakeSel,bool shakeDirection)
{
	M_Moto_Def.moto_is_shake = isShake;
	//M_Moto_Def.moto_shake_direction = shakeDirection;
	M_Moto_Def.moto_shake_type = shakeType;
	Rd_MotoGrade(shakeSel);
	
	//for(uint8_t i=1;i<MOTE_MAX_NUM;i++)
	//	  nrf_gpio_pin_clear(Moto_Number[i]);
	
	if(M_Moto_Def.moto_is_shake)
	{
		moto_timer_start();
	}
	else
	{
		uint8_t percent = 100;
		send_data_to_phone(0,&percent,0);
		
		moto_timer_stop();
	#if defined(CHANG_RED_LED_WORK)
		if(charge_flag)
			charge_flag =false;
	#endif
	#if !defined(ADD_NRF_PWM_CTRL)	
		for(uint8_t i=1;i<MOTE_MAX_NUM;i++)
		  nrf_gpio_pin_clear(Moto_Number[i]);
	#endif
	
	#if defined(ADD_NRF_PWM_CTRL)
		pwm_ctrl(PWM_FALSE);
	#endif
	}
}

//马达震动执行
void Rd_MotoShakeStart(void)
{
	uint8_t ntimecount = 0 ;
	if(M_Moto_Def.moto_is_shake)
	{	

	#if defined(ADD_WOR_MODE)
		if(!n_work_mode)
			ntimecount = Rd_MotoGetTimeCount();
		else
			ntimecount = n_work_mode -1;
	#else
		ntimecount = Rd_MotoGetTimeCount();
	#endif
		Rd_MotoSetShakeSwitchMode(ntimecount);
	
	#if defined(TEST_LED)
		nrf_gpio_pin_clear(TEST_RED);
	#endif
	}	
#if defined(TEST_LED)
	else
	{
		nrf_gpio_pin_set(TEST_RED);
	}
#endif
	
}

#if !defined(ADD_NRF_PWM_CTRL)	

void Rd_MotoShakeModeOne(void)
{
	nrf_gpio_pin_set(Moto_Number[nShake_mode]);
}


//马达震动执行
void Rd_MotoShakeModeTwo(void)
{
	static uint8_t    ShakeOrStop = 0;

	ShakeOrStop ++ ;

	if ((ShakeOrStop / 6 ) % 2 == 0)
	{
		nrf_gpio_pin_set(Moto_Number[nShake_mode]);
	}
	else
	{
		nrf_gpio_pin_clear(Moto_Number[nShake_mode]);
	}
}

void Rd_MotoShakeModeThree(void)
{
	static uint8_t    ShakeOrStop = 0;

	ShakeOrStop ++ ;

	if ((ShakeOrStop / 10 ) % 2 == 0)
	{
		nrf_gpio_pin_set(Moto_Number[nShake_mode]);
	}
	else
	{
		nrf_gpio_pin_clear(Moto_Number[nShake_mode]);
	}
}

void Rd_MotoShakeModeFour(void)
{
	static uint8_t    ShakeOrStop = 0;

	ShakeOrStop ++ ;

	if ((ShakeOrStop / 3 ) % 2 == 0)
	{
		nrf_gpio_pin_set(Moto_Number[nShake_mode]);
	}
	else
	{
		nrf_gpio_pin_clear(Moto_Number[nShake_mode]);
	}
}

void Rd_MotoShakeModeFive(void)
{
	static uint8_t    ShakeOrStop = 0;
	static uint8_t    ShakeOrStop_ext = 0;

	ShakeOrStop ++ ;

	if ((ShakeOrStop / 9 ) % 2 == 0)
	{
		ShakeOrStop_ext++ ;
		if ((ShakeOrStop_ext % 2 == 0))
		{
			nrf_gpio_pin_set(Moto_Number[nShake_mode]);
		}
		else
		{
			nrf_gpio_pin_clear(Moto_Number[nShake_mode]);
		}
	}
	else
	{
		nrf_gpio_pin_clear(Moto_Number[nShake_mode]);
	}

}

void Rd_MotoShakeModeSix(void)
{
	static uint8_t    ShakeOrStop = 0;
	static uint8_t    ShakeOrStop_ext = 0;

	ShakeOrStop ++ ;

	if ((ShakeOrStop / 10 ) % 2 == 0)
	{
		ShakeOrStop_ext++ ;
		if ((ShakeOrStop_ext % 3 == 0))
		{
			nrf_gpio_pin_set(Moto_Number[nShake_mode]);
		}
		else
		{
			nrf_gpio_pin_clear(Moto_Number[nShake_mode]);
		}
	}
	else
	{
		nrf_gpio_pin_clear(Moto_Number[nShake_mode]);
	}
}
#else
void Rd_MotoShakeModeOne(void)
{
#if defined(CHANGE_MODE_CTRL)
	set_pwm_start_stop_time(PWM_MODE_1);
#endif

}


//马达震动执行
void Rd_MotoShakeModeTwo(void)
{
#if defined(CHANGE_MODE_CTRL)
	set_pwm_start_stop_time(PWM_MODE_2);
#endif
}

void Rd_MotoShakeModeThree(void)
{
#if defined(CHANGE_MODE_CTRL)
	set_pwm_start_stop_time(PWM_MODE_3);
#endif
}

void Rd_MotoShakeModeFour(void)
{
#if defined(CHANGE_MODE_CTRL)
	set_pwm_start_stop_time(PWM_MODE_4);
#endif
}

void Rd_MotoShakeModeFive(void)
{
#if defined(CHANGE_MODE_CTRL)
	set_pwm_start_stop_time(PWM_MODE_5);
#endif
}


void Rd_MotoShakeModeSix(void)
{
#if defined(CHANGE_MODE_CTRL)
	set_pwm_start_stop_time(PWM_MODE_6);
#endif
}



#endif


static void moto_bra_timeout_handler(void * p_context)
{
	UNUSED_PARAMETER(p_context);
	switch(M_Moto_Def.moto_shake_type)
	{
		case SHAKE_STOP     : //停止震动
			M_Moto_Def.moto_is_shake = false;
		#if !defined(ADD_NRF_PWM_CTRL)	
			for(uint8_t i=1;i<MOTE_MAX_NUM;i++)
		  		nrf_gpio_pin_clear(Moto_Number[i]);
		#endif
			break;
	  case SHAKE_ODD      : //奇数震动
			break;
	  case SHAKE_EVEN     : //偶数震动
			break;
	  case SHAKE_RANDOM   : //随机震动
			break;
	  case SHAKE_FACE     : //正面震动
			break;
	  case SHAKE_BACK     : //背面震动
			break;
	  case SHAKE_SEQUENCE : //顺序震动
			break;
	  case SHAKE_OPPOINT  : //指定震动
	  	{
			n_time_shake-- ;	
			//Rd_MotoShakeOppoint();
			Rd_MotoShake_time(n_time_shake);
	  	}
		break;
	}
}

uint8_t Rd_MotoGetTimeCount(void)
{
	uint8_t ret_value = 0 ;
	ret_value = (n_time_shake / 60) % 6 ;

	return ret_value ;
}

#if defined(ADD_NRF_PWM_CTRL)	
void Rd_MotoSetShakeSwitchMode(uint8_t nmode)
{
#if defined(CHANG_RED_LED_WORK)
	static uint8_t rd_mode = 15;
	if(rd_mode != nmode)
	{
		rd_mode = nmode;
		set_led_mode(LIGHT_FLASH3);
	}
#endif
	switch (nmode)
	{
		case 0 :
			{
				Rd_MotoShakeModeOne();
			}
			break ;
		case 1 :
			{
				Rd_MotoShakeModeTwo();
			}
			break ;	
		case 2 :
			{
				Rd_MotoShakeModeThree();	
			}
			break ;
		case 3:
			{
				Rd_MotoShakeModeFour();
			}
			break ;
		case 4:
			{
				Rd_MotoShakeModeFive();
			}
			break ;
		case 5:
			{
				Rd_MotoShakeModeSix();
			}
			break ;	
		default :
			{
				//for(uint8_t i=1;i<MOTE_MAX_NUM;i++)
			  	//	nrf_gpio_pin_clear(Moto_Number[i]);
			  	Rd_MotoShakeModeFour();
			}
			break ;
	}


}
#else
void Rd_MotoSetShakeSwitchMode(uint8_t nmode)
{
#if defined(CHANG_RED_LED_WORK)
	static uint8_t rd_mode = 15;
	if(rd_mode != nmode)
	{
		rd_mode = nmode;
		set_led_mode(LIGHT_FLASH3);
	}
#endif
	switch (nmode)
	{
		case 0 :
			{
				Rd_MotoShakeModeOne();
			}
			break ;
		case 1 :
			{
				Rd_MotoShakeModeTwo();
			}
			break ;	
		case 2 :
			{
				Rd_MotoShakeModeThree();	
			}
			break ;
		case 3:
			{
				Rd_MotoShakeModeFour();
			}
			break ;
		case 4:
			{
				Rd_MotoShakeModeFive();
			}
			break ;
		case 5:
			{
				Rd_MotoShakeModeSix();
			}
			break ;	
		default :
			{
				//for(uint8_t i=1;i<MOTE_MAX_NUM;i++)
			  	//	nrf_gpio_pin_clear(Moto_Number[i]);
			  	Rd_MotoShakeModeFour();
			}
			break ;
	}
}
#endif
void Rd_MoteKeyPress(void)
{
	static uint8_t nshakecount = 1 ;

	//if (nshakecount++ >6)
	{
		nshakecount = 6 ;
	}
	Rd_MotoShakeMode(true,SHAKE_OPPOINT,nshakecount,0);
}


#if NOR_USER_MOTOR
static void moto_timeout_handler(void * p_context)
{
	UNUSED_PARAMETER(p_context);
	switch(M_Moto_Def.moto_shake_type)
	{
		case SHAKE_STOP     : //停止震动
			M_Moto_Def.moto_is_shake = false;
			break;
	  case SHAKE_ODD      : //奇数震动
			Rd_MotoShakeOdd();
			break;
	  case SHAKE_EVEN     : //偶数震动
			Rd_MotoShakeEven();
			break;
	  case SHAKE_RANDOM   : //随机震动
			Rd_MotoShakeRandom();
			break;
	  case SHAKE_FACE     : //正面震动
			Rd_MotoShakeFace();
			break;
	  case SHAKE_BACK     : //背面震动
			Rd_MotoShakeBack();
			break;
	  case SHAKE_SEQUENCE : //顺序震动
			Rd_MotoShakeSequence();
			break;
	  case SHAKE_OPPOINT  : //指定震动
			Rd_MotoShakeOppoint();
			break;
	}
}
#endif

/* 创建lcd和马达震动超时定时器*/
static void moto_timer_create(void)
{
	uint32_t err_code = app_timer_create(&m_moto_handle_id,
															APP_TIMER_MODE_REPEATED,
															moto_bra_timeout_handler);//moto_timeout_handler);
	APP_ERROR_CHECK(err_code);
}



/**@brief Function for starting application timers.
 */
void moto_timer_start(void)
{
	uint32_t err_code = app_timer_start(m_moto_handle_id, MOTO_HANDLE_MEAS_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);
	M_Moto_Def.moto_is_shake = true;
}

/**@brief Function for starting application timers.
 */
void moto_timer_stop(void)
{
	uint32_t err_code = app_timer_stop(m_moto_handle_id);
	APP_ERROR_CHECK(err_code);
	M_Moto_Def.moto_is_shake = false;
}


//马达初始�?
void Rd_MotoInit(void)
{
	for(uint8_t num = 0; num< 32 ; num++) 
		DISABLE_PIN_FN(num);
#if !defined(ADD_NRF_PWM_CTRL)	
	nrf_gpio_cfg_output(MOTO_PIN_01);
	nrf_gpio_cfg_output(MOTO_PIN_02);
	nrf_gpio_cfg_output(MOTO_PIN_03);
	nrf_gpio_cfg_output(MOTO_PIN_04);
	nrf_gpio_cfg_output(MOTO_PIN_05);
	nrf_gpio_cfg_output(MOTO_PIN_06);
#else
//	nrf_gpio_cfg_output(PWM_PIN_01);
//	nrf_gpio_cfg_output(PWM_PIN_02);
#endif


	moto_timer_create();
	
	nrf_gpio_cfg_input(DET_5V, NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(CHARGE_OK, NRF_GPIO_PIN_PULLUP);
	
	//nrf_gpio_cfg_output(LED_BLUE);
	nrf_gpio_cfg_output(LED_RED);

	
#if defined(TEST_LED)
	nrf_gpio_cfg_output(TEST_RED);
	nrf_gpio_pin_clear(TEST_RED);
#endif
}


#if defined(CHANG_RED_LED_WORK)
bool charge_flag = false;
void charge_ble_work(void)
{
	charge_flag =true;
	n_time_shake = 60;
	Rd_MotoShakeMode(true,SHAKE_OPPOINT,2,0);
}
#endif



