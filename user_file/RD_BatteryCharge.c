#include "RD_BatteryCharge.h"
#include "nrf_delay.h"
#include "nrf6310.h"
#include "ble_service.h"

extern bool                                         m_blue_connect;

////////////////////////////

extern uint8_t g_bat_present;

///////////////////////////////


//设置AD输出端口
static uint16_t bat_start(void)
{
//	  NRF_ADC->CONFIG = (ADC_CONFIG_RES_8bit							 << ADC_CONFIG_RES_Pos) 	|
//										  (ADC_CONFIG_INPSEL_AnalogInputNoPrescaling	   << ADC_CONFIG_INPSEL_Pos)  |
//										  (ADC_CONFIG_REFSEL_VBG						   << ADC_CONFIG_REFSEL_Pos)  |
//										  (ADC_CONFIG_PSEL_AnalogInput2 				   << ADC_CONFIG_PSEL_Pos)	  |
//										  (ADC_CONFIG_EXTREFSEL_None					   << ADC_CONFIG_EXTREFSEL_Pos);
	
	 NRF_ADC->CONFIG = (ADC_CONFIG_RES_10bit							 << ADC_CONFIG_RES_Pos) 	|
										  (ADC_CONFIG_INPSEL_AnalogInputNoPrescaling	   << ADC_CONFIG_INPSEL_Pos)  |
										  (ADC_CONFIG_REFSEL_VBG						   << ADC_CONFIG_REFSEL_Pos)  |
										  (ADC_CONFIG_PSEL_AnalogInput4 				   << ADC_CONFIG_PSEL_Pos)	  |
										  (ADC_CONFIG_EXTREFSEL_None					   << ADC_CONFIG_EXTREFSEL_Pos);
	
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
		nrf_delay_ms(5);
	
		NRF_ADC->EVENTS_END  = 0;	 // Stop any running conversions.
		NRF_ADC->TASKS_START = 1;	
		while(NRF_ADC->EVENTS_END == 0);
		if (NRF_ADC->EVENTS_END != 0)
	{
		uint16_t	 adc_result = 0;
				
		NRF_ADC->EVENTS_END 	= 0;
				while(NRF_ADC->BUSY&1);
		adc_result				= NRF_ADC->RESULT;
		NRF_ADC->TASKS_STOP 	= 1;
				NRF_ADC->ENABLE 		= ADC_ENABLE_ENABLE_Disabled;
		__NOP();__NOP();__NOP();				

		return	adc_result; 			
	} 
	return 0;
}


/**
 *电量处理函数
 */
#define N 11
static float battery_leave_handler(void)
{	
	unsigned char i = 0,j=0;
	float vbat = 0.0;
	uint16_t tmp;
	float value = 0.0;
	uint16_t value_buf[N];
	for(i=0;i<N;i++)
	{
		value_buf[i] = bat_start();
	}
	for (j=0;j<N-1;j++)
	{
		for (i=0;i<N-j;i++)
		{
			if ( value_buf[i]>value_buf[i+1] )
			{
				tmp = value_buf[i];
				value_buf[i] = value_buf[i+1];
				value_buf[i+1] = tmp;
			}
		}
	}
#if 0	 
	value = value_buf[(N-1)/2]*1.0;
	vbat = (value*1.2/1024)*3.6;
	return vbat;
#elif 1
	value = value_buf[(N-1)/2]*1.0;
	vbat = (value*4/1024)*3.3;
	return vbat;
#else

	for(i=1;i<N-1;i++)
	sum += value_buf[i];
	ad_data = sum/(N-2); 
	value = ad_data*1.0;
	vbat = (value*1.2/1024)*4.0;
	return vbat;
#endif	 
}


float BAT_ADC_PRE_VOLTAGE_MAX=3.6;// 4.1V   // 6354  4.2V
float BAT_ADC_RRE_VOLTAGE_MIN=2.5;	// 3.4V


uint8_t battery_level_present(float nbattery)
{
	float temp_data = 0 ;
	uint8_t ret_data = 0 ;
	
	if (nbattery <= BAT_ADC_RRE_VOLTAGE_MIN)
	{
		nbattery = BAT_ADC_RRE_VOLTAGE_MIN ;
	}
	temp_data = nbattery - BAT_ADC_RRE_VOLTAGE_MIN ;
		
	ret_data = ((temp_data * 100) / (BAT_ADC_PRE_VOLTAGE_MAX - BAT_ADC_RRE_VOLTAGE_MIN)) ;

	return ret_data ;
}




//电量转换函数
uint8_t battery_level_transform(void)
{
	float battery =  battery_leave_handler();
	//uint8_t battery_level = BAT_PERCENT(battery);
	uint8_t battery_level = battery_level_present(battery);

	return battery_level;
}


//开机检测电量是否开机成�?
bool power_on_for_battery_check(void)
{
	uint8_t m_battery_leave =  battery_level_transform();//获取电量等级
	if(m_battery_leave == 0)//开机不成功
	{
		return false;
	}
	else                    //开机成�?
	{
		return true;
	}
}




void charge_check_handle(void)
{
	static uint8_t battery_cnt = 0;
	battery_cnt++;
	if(battery_cnt>=120)		//(10*60)/5 	10min
	{
		battery_cnt = 0;
		g_bat_present = battery_level_transform();
		if(g_bat_present>=100)
			g_bat_present = 100;
		else if(g_bat_present<=0)
			g_bat_present = 0;
			
	}
	else
	{
		
	}


}

void factory_test(void)
{
	static uint8_t tmp = 0;
	tmp++;
	{
	    nrf_gpio_pin_toggle(BUZZER_EN);
		if(tmp<=3)
		{
//			tmp = 1;
			Moto_A2B();
		}
		else if(tmp<=6)
		{
//			tmp = 0;
			Moto_B2A();
		}
		else if(tmp>=6)
		{
			tmp = 0;
		}
//		Moto_DISABLE();
	}
}

//电量返回
void Bat_Return(void)
{
	uint8_t percent = battery_level_transform();

}





