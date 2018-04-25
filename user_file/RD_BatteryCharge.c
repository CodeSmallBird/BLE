#include "RD_BatteryCharge.h"
#include "nrf_delay.h"
#include "nrf6310.h"
#include "ble_service.h"


extern bool                                         m_blue_connect;

#if defined(CHANG_RED_LED_WORK)	
#define RED_ON		nrf_gpio_pin_set(LED_RED)
#define RED_OFF		nrf_gpio_pin_clear(LED_RED)
#endif

#if defined(CHAGRE_REST_ONCE)
uint8_t charge_reset_falg = false;
#endif


//设置AD输出端口
static uint16_t bat_start(void)
{
    NRF_ADC->CONFIG = (ADC_CONFIG_RES_8bit                             << ADC_CONFIG_RES_Pos)     |
										  (ADC_CONFIG_INPSEL_AnalogInputNoPrescaling       << ADC_CONFIG_INPSEL_Pos)  |
										  (ADC_CONFIG_REFSEL_VBG                           << ADC_CONFIG_REFSEL_Pos)  |
										  (ADC_CONFIG_PSEL_AnalogInput2                    << ADC_CONFIG_PSEL_Pos)    |
										  (ADC_CONFIG_EXTREFSEL_None                       << ADC_CONFIG_EXTREFSEL_Pos);
    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
		nrf_delay_ms(5);
	
		NRF_ADC->EVENTS_END  = 0;    // Stop any running conversions.
		NRF_ADC->TASKS_START = 1;	
		while(NRF_ADC->EVENTS_END == 0);
		if (NRF_ADC->EVENTS_END != 0)
    {
        uint16_t     adc_result = 0;
				
        NRF_ADC->EVENTS_END     = 0;
        adc_result              = NRF_ADC->RESULT;
        NRF_ADC->TASKS_STOP     = 1;
				NRF_ADC->ENABLE         = ADC_ENABLE_ENABLE_Disabled;
        __NOP();__NOP();__NOP();				

        return  adc_result;				
    } 
		return 0;
}

/**
 *电量处理函数
 */
static uint16_t battery_leave_handler(void)
{
		uint16_t ad_data = bat_start();
				
		return BAT_ADC_RESULT_IN_MILLI_VOLTS(ad_data);
}

uint8_t battery_level_present(uint16_t nbattery)
{
	uint16_t temp_data = 0 ;
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
	uint16_t battery =  battery_leave_handler();
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

//充电检测处理函�?
#if defined(CHANG_RED_LED_WORK)
bool  sys_open_connect = false;         //第一次开机连接标�?
static uint8_t led_count = 0 ;
uint8_t sys_mode = BLE_BREAK;
uint8_t led_run_mode =0;
static uint8_t flash_cnt = 0;;
static uint8_t flash2_run_mode = 0;
static bool led_flag = false;
extern void reback_sys_mode(uint8_t sys_run_mode);


void led_flash(void)
{
	if(led_count % 2 == 0)
	{
		if(!led_flag)
		{
			RED_ON;
			led_flag = true;
		}
		else
		{
			RED_OFF;
			led_flag = false;
			flash_cnt++;
		}
		if(flash_cnt > 10)
		{
			reback_sys_mode(sys_mode);
		}
			
	}
}

void led_flash2(void)		// 低电�?-
{
	static uint8_t cnt = 0;
	switch(flash2_run_mode)
	{
		case 0:
		case 1:
			if(!led_flag)
			{
				RED_ON;
				led_flag = true;
			}
			else
			{
				RED_OFF;
				led_flag = false;
				flash2_run_mode++;
				cnt = 0;
			}
			break;
		case 2:
			cnt++;
			if(cnt > 8)
			{
				flash2_run_mode = 0;
			}
	}
}
void led_flash3(void)	// 模式切换�?
{
	if(led_count % 2 == 0)
	{
		if(!led_flag)
		{
			RED_ON;
			led_flag = true;
		}
		else
		{
			RED_OFF;
			led_flag = false;
			flash_cnt++;
		}
		if(flash_cnt > 1)
		{
			reback_sys_mode(sys_mode);
		}
	}
}

void led_work_mode(uint8_t mode)
{
	led_count++;

	switch(mode)
	{	
		case LIGHT_OFF: //常灭
			RED_OFF;
			break;
		
		case LIGHT_ON:	//常亮
			RED_ON;
			break;
		
		case LIGHT_FLASH1:	//闪烁
			led_flash();
			break;
		
		case LIGHT_FLASH2:	//闪烁2
			led_flash2();
			break;
		case LIGHT_FLASH3:	//闪烁3
			led_flash3();
			break;
		default:
			break;
	}
}

void set_led_mode(uint8_t set_mode)
{
	switch(set_mode)
	{	
		case LIGHT_OFF: //常灭
		case LIGHT_ON:	//常亮	
			break;
	
		case LIGHT_FLASH1:	//闪烁--开�?
			flash_cnt = 0;
			break;
		
		case LIGHT_FLASH2:	//闪烁2--低电�?
			flash2_run_mode = 0;
			break;
		case LIGHT_FLASH3:	//闪烁3--切换模式
			flash_cnt = 0;
			break;
		default:
			break;
	}
	led_flag = false;
	led_run_mode = set_mode;
	led_count = 0;
}


void reback_sys_mode(uint8_t sys_run_mode)
{
	switch(sys_run_mode)
	{
		case BLE_BREAK:
			set_led_mode(LIGHT_OFF);
			break;
		
		case BLE_CONNECT:
			set_led_mode(LIGHT_ON);
			break;
		
		case BLE_LOW_POWER:
			set_led_mode(LIGHT_FLASH2);
			break;
		
		case BLE_CHARGE:
			set_led_mode(LIGHT_ON);
			break;
		
		default:
			set_led_mode(LIGHT_ON);
			break;
	}
}
#if defined(FLASH_READ_WRITE)
extern uint8_t sys_flag_data[SYS_FLAG_DATA_LEN];
#endif
void charge_check_handle(void)
{
	static uint8_t save_mode = BLE_BREAK;
 	led_work_mode(led_run_mode);
 	if(led_count % 4 != 0)	
		return;
#if !defined(CHAGRE_REST_ONCE)
	if((sys_open_connect == false) ||(led_run_mode == LIGHT_FLASH1)	)
	{
	
	}
	else
#endif
	{
		if(m_blue_connect == true)
		{
			sys_mode = BLE_CONNECT;
		}
		else
		{
			sys_mode = BLE_BREAK;
		}
		
		if((nrf_gpio_pin_read(CHARGE_OK) == 0)&&(nrf_gpio_pin_read(DET_5V) == 0))
		{
			sys_mode = BLE_CHARGE_OK;
		}
		else 
		if(nrf_gpio_pin_read(DET_5V) == 0)	
		{
			sys_mode = BLE_CHARGE;
		}
		else if(battery_level_transform() < 5)
		{
			sys_mode = BLE_LOW_POWER;
		}
	}

	if( save_mode != sys_mode )
	{
		save_mode = sys_mode;
		
	#if defined(CHAGRE_REST_ONCE)
		if(sys_mode == BLE_CHARGE)
		{
			if(charge_reset_falg)
			{
				charge_reset_falg = false;
				//д��flash
			#if defined(FLASH_READ_WRITE)
				sys_flag_data[0] = charge_reset_falg;
			#endif		
			}
			else
			{
				charge_reset_falg = true;
			#if defined(FLASH_READ_WRITE)
				sys_flag_data[0] = charge_reset_falg;
			#endif
			}

		}
	#endif
		switch(save_mode)
		{
			case BLE_BREAK:
				set_led_mode(LIGHT_OFF);
				break;
			
			case BLE_CONNECT:
				set_led_mode(LIGHT_ON);
				break;
			
			case BLE_LOW_POWER:
				set_led_mode(LIGHT_FLASH2);
				break;
			
			case BLE_CHARGE:
				set_led_mode(LIGHT_ON);
				charge_ble_work();
				break;
			case BLE_CHARGE_OK:
				set_led_mode(LIGHT_OFF);
				break;
			default:
				set_led_mode(LIGHT_ON);
				break;
		}
		if((save_mode != BLE_CHARGE)&&(charge_flag))
		{
			charge_flag =false;
			Rd_MotoShakeMode(false,0,0,0);
		}
	}
	if(sys_open_connect == false)
	{
		sys_open_connect = true;
		set_led_mode(LIGHT_FLASH1);
	}
	
 }

#else
void charge_check_handle(void)
{
	static uint8_t led_count = 0 ;
	if ((m_blue_connect == 1) && (nrf_gpio_pin_read(DET_5V) == 1))
	{
		nrf_gpio_pin_set(LED_RED);
	}
//	if(m_blue_connect == 0)
//	{
//		nrf_gpio_pin_clear(LED_RED);
//	}
	
	if(nrf_gpio_pin_read(DET_5V) == 0)      //检测到充电 电池电量跳动
	{
		if(nrf_gpio_pin_read(CHARGE_OK) == 0) //充电 红灯�?
		{
			static uint8_t led_full_count = 0 ;

			led_full_count++ ;
			if (led_full_count % 2 == 0)
			{
				nrf_gpio_pin_set(LED_RED);
			}
			else
			{
				nrf_gpio_pin_clear(LED_RED);
			}
		}
		else                                  //满电 蓝灯�?
		{
			//nrf_gpio_pin_set(LED_BLUE);
			nrf_gpio_pin_set(LED_RED);
		}
	}
	else                                    //检测到充电拔出 则灯全灭
	{
		if (m_blue_connect == 0)
		{
    		nrf_gpio_pin_clear(LED_RED);
			//nrf_gpio_pin_clear(LED_BLUE);
		}
	}
}
#endif
//电量返回
void Bat_Return(void)
{
	uint8_t percent = battery_level_transform();
	
  	send_data_to_phone(0x50,&percent,1);
}





