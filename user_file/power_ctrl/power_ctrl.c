#include "power_ctrl.h"

POWER_CTRL power_ctrl;
DETECT_BUFF detect_buff;

#define APP_GPIOTE_MAX_USERS            1     /**< Maximum number of users of the GPIOTE handler. */
#define BUTTON_DETECTION_DELAY             APP_TIMER_TICKS(5, APP_TIMER_PRESCALER)   /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

static void gpiote_init(void)
{
	APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}

static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
	static uint8_t state = 1;
    uint32_t pins_state = NRF_GPIO->IN;
	
#if defined(DEBUG_RUN)
	printf("pins_state:%d\r\n",pins_state>>BUTTON_PIN);
#endif
	switch (pin_no)
	{
		case BUTTON_PIN:
		{
			if(((pins_state>>BUTTON_PIN)&0x01)== false)
			{
			
			#if defined(DEBUG_RUN)
				printf("按下\r\n");
			#endif
				if(!power_ctrl.ctrl_delay)
				{
					if(state)
					{
						SetPowerOff();
						state = 0;
					}
					else
					{
						SetPowerOn();
						state = 1;
					}

				}
			}
			else if(((pins_state>>BUTTON_PIN)&0x01)== true)
			{
			
			#if defined(DEBUG_RUN)
				printf("抬起\r\n");
			#endif
			}
		}
		break;
		default:break;
	}
}

static void buttons_init(void)
{
	static app_button_cfg_t buttons[] =
	{
		{BUTTON_PIN, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_event_handler}
	};
	APP_BUTTON_INIT(buttons,1, BUTTON_DETECTION_DELAY, false);
}

void key_display_init(void)
{
	gpiote_init();
	buttons_init();
	app_button_enable();
}



static uint16_t bat_start(void)
{

    NRF_ADC->CONFIG = (ADC_CONFIG_RES_8bit                             << ADC_CONFIG_RES_Pos)     |
					  (ADC_CONFIG_INPSEL_AnalogInputNoPrescaling       << ADC_CONFIG_INPSEL_Pos)  |
					  (ADC_CONFIG_REFSEL_VBG                           << ADC_CONFIG_REFSEL_Pos)  |
					  (ADC_CONFIG_PSEL_AnalogInput4                    << ADC_CONFIG_PSEL_Pos)    |
					  (ADC_CONFIG_EXTREFSEL_None                       << ADC_CONFIG_EXTREFSEL_Pos);
    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
	
		NRF_ADC->EVENTS_END  = 0;    // Stop any running conversions.
		NRF_ADC->TASKS_START = 1;	
		nrf_delay_ms(5);
		while(NRF_ADC->EVENTS_END == 0);
		if (NRF_ADC->EVENTS_END != 0)
	    {
	        uint16_t     adc_result = 0;
					
	        NRF_ADC->EVENTS_END     = 0;
	        adc_result              = NRF_ADC->RESULT;
	        NRF_ADC->TASKS_STOP     = 1;
			NRF_ADC->ENABLE         = ADC_ENABLE_ENABLE_Disabled;
	        __NOP();__NOP();__NOP();				
		#if defined(DEBUG_RUN)
			//printf("adc_result:%d\r\n",adc_result);		
		#endif
	        return  adc_result;				
	    } 
		return 0;
}

/**
 *单位为mv
 */
static uint16_t battery_leave_handler(void)
{
		uint16_t ad_data = bat_start();
	#if defined(DEBUG_RUN)
		//printf("ad_data:%d\r\n",ad_data);
	#endif
		if(ad_data <53)
			return 0;
		return 31*BAT_ADC_RESULT_IN_MILLI_VOLTS(ad_data);
}




//鐢甸噺杞崲鍑芥暟
uint16_t battery_level_transform(void)
{
//	static uint16_t filter_vol[6] = {0,0,0,0,0,0};	
//	uint8_t i,j;
	uint16_t battery = 0;
//	uint16_t temp_battery[2] = {0,0};
	battery =  battery_leave_handler();
#if 0
	for(i=0;i<6;i++)
	{
		if(battery > filter_vol[i])
			break;
	}
	if(i<6)
	{
		temp_battery[0] = filter_vol[i];
		filter_vol[i] = battery;
		for(j=i+1;j<6;j++)
		{
			temp_battery[1] = filter_vol[j];
			filter_vol[j] = temp_battery[0];
			temp_battery[0] = temp_battery[1];
		}
	}
	battery =0;
	for(i=1;i<5;i++)
	{
		battery += filter_vol[i];
	}

	return battery/4;
#else
	return battery;
#endif
}
//mv
#define BATTERY_LEVER0          5000
#define BATTERY_LEVER1   		27000
#define BATTERY_LEVER2   		30000
#define BATTERY_LEVER3   		34000
#define BATTERY_LEVER4 38000

void BatteryLedDispPoll(void)
{

	if(power_ctrl.detec_time>10)	//4
	{
		power_ctrl.voltage = battery_level_transform();
		power_ctrl.detec_time = 0;
	}
	//else
	//	return;

	POW_LED1_OFF;
	POW_LED2_OFF;
	POW_LED3_OFF;
	POW_LED4_OFF;
	if(power_ctrl.sys_state == 0)
		return;
	if(power_ctrl.voltage > BATTERY_LEVER4)
	{
		POW_LED1_ON;
		POW_LED2_ON;
		POW_LED3_ON;
		POW_LED4_ON;
	}
	else if(power_ctrl.voltage>BATTERY_LEVER3)
	{
		POW_LED1_ON;
		POW_LED2_ON;
		POW_LED3_ON;
	}
	else if(power_ctrl.voltage>BATTERY_LEVER2)
	{
		POW_LED1_ON;
		POW_LED2_ON;
	}
	else if(power_ctrl.voltage>BATTERY_LEVER1)
	{
		POW_LED1_ON;
	}
}


void SetPowerOff(void)
{
	power_ctrl.off_ctrl = 1;
	power_ctrl.ctrl_delay = 15;
#if defined(DEBUG_RUN)
	printf("SetPowerOff\r\n");
#endif
	power_ctrl.sys_state = 1;
}

void SetPowerOn(void)
{
	power_ctrl.on_ctrl = 1;
	power_ctrl.ctrl_delay = 15;
#if defined(DEBUG_RUN)
	printf("SetPowerOn\r\n");
#endif
	power_ctrl.sys_state = 0;
}


void PowerOffCtrlPoll(void)
{
	if(!power_ctrl.off_ctrl)
		return;

	if(power_ctrl.ctrl_delay)
	{
		POW_MCU_OFF_HIGH;
	}
	else
	{
		POW_MCU_OFF_LOW;
		power_ctrl.off_ctrl = 0;
	}
}

void PowerOnCtrlPoll(void)
{
	if(!power_ctrl.on_ctrl)
		return;
	if(power_ctrl.ctrl_delay)
	{
		POW_MCU_ON_HIGH;
	}
	else
	{
		POW_MCU_ON_LOW;
		power_ctrl.on_ctrl = 0;
	}
}



void power_ctrl_pin_init(void)
{
	POW_LED1_INIT;
	POW_LED1_OFF;

	POW_LED2_INIT;
	POW_LED2_OFF;
	
	POW_LED3_INIT;
	POW_LED3_OFF;
	
	POW_LED4_INIT;
	POW_LED4_OFF;

	
	POW_MCU_ON_INIT; 
	POW_MCU_ON_LOW;

	POW_MCU_OFF_INIT; 
	POW_MCU_OFF_LOW;

	ADC_INPUT_INIT;


}


void power_param_init(void)
{
	memset(&power_ctrl,0,sizeof(power_ctrl));
}

void power_ctrl_init(void)
{
	power_ctrl_pin_init();
	key_display_init();
	power_param_init();
}



void power_ctrl_polling(void)
{
	PowerOnCtrlPoll();
	PowerOffCtrlPoll();
	BatteryLedDispPoll();
}



void InitSamp(DETECT_BUFF    *Samp, U8 State)
{
	Samp->Time = 100;
	Samp->SampPrev = Samp->SampNow = State;
	Samp->Wobble = State;
	Samp->Compara = !State;
	Samp->Result = Samp->Compara;
}


//detec_buff

U8 DetectSamp(DETECT_BUFF      *Samp, U8 State)
{
	if(Samp->Time)
	{
		Samp->Time--;
		return 0;
	}
	Samp->SampPrev 		= 	Samp->SampNow;
	Samp->SampNow		= 	State;
	if(Samp->SampPrev 	!= 	Samp->SampNow)return 0;
	Samp->Compara		= 	Samp->Wobble;
	Samp->Wobble		= 	Samp->SampPrev;
	if(Samp->Compara 	== 	Samp->Wobble)return 0;

	Samp->Result = Samp->Compara;
	return 1;	
}


void DetecInit(void)
{
	memset(&detect_buff,0,sizeof(detect_buff));
	InitSamp(&detect_buff,GET_CHARGE_DET_STATE);
}


void DetectPoll(void)
{
	DetectSamp(&detect_buff,GET_CHARGE_DET_STATE);
}








