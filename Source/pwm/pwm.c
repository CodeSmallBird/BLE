#include "pwm.h"
volatile uint32_t pwm_duty = 6;
static uint8_t pwm_state = PWM_NONE;
#if defined(CHANGE_MODE_CTRL)
PWM_WORK pwm_work;
#endif

//使用了三个通道，都通向了GPIOTE的输出功能
void ppi_init(void)
{
/*
选择PPI通道链接的输入和输出设备
输入：timer2的定时比较器0,1,2
输出:引脚事件1
*/
    NRF_PPI->CH[PPI_CH_CC0].EEP = (uint32_t)&NRF_TIMER2->EVENTS_COMPARE[PPI_CH_IN_CMP_0];
    NRF_PPI->CH[PPI_CH_CC0].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[PPI_CH_OUT_GPIOTE_CH];

    NRF_PPI->CH[PPI_CH_CC1].EEP = (uint32_t)&NRF_TIMER2->EVENTS_COMPARE[PPI_CH_IN_CMP_1];
    NRF_PPI->CH[PPI_CH_CC1].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[PPI_CH_OUT_GPIOTE_CH];
    
    NRF_PPI->CH[PPI_CH_CC2].EEP = (uint32_t)&NRF_TIMER2->EVENTS_COMPARE[PPI_CH_IN_CMP_2];
    NRF_PPI->CH[PPI_CH_CC2].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[PPI_CH_OUT_GPIOTE_CH];

#if 1
    //开启通道
    NRF_PPI->CHEN = PPI_CH_CC0_ENABLE
                    | PPI_CH_CC1_ENABLE
                    | PPI_CH_CC2_ENABLE;
#endif
}

//这是pwm_pin配置函数
//使用了GPIOTE模块


void open_pwm_pin(void)
{
    nrf_gpio_cfg_output(PWM_PIN_02);
	nrf_gpio_pin_set(PWM_PIN_02);

    nrf_gpio_cfg_output(PWM_PIN_01);
	nrf_gpio_pin_set(PWM_PIN_01);
}


void close_pwm_pin(void)
{
    nrf_gpio_cfg_output(PWM_PIN_02);
	nrf_gpio_pin_clear(PWM_PIN_02);

    nrf_gpio_cfg_output(PWM_PIN_01);
	nrf_gpio_pin_clear(PWM_PIN_01);

}


void pwm_gpiote_init(void)  //io输出
{
    // Connect GPIO input buffers and configure PWM_OUTPUT_PIN_NUMBER as an output.
	close_pwm_pin();
}

//这是定时器初始化
//初始化了定时器和三个比较器并打开比较器1的中断功能
void timer2_init(void) 
{
    // 设置16m时钟
	#if 1
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    // 等待时钟开启
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) 
    {
        //Do nothing.
    }
	#else
		NRF_CLOCK->EVENTS_HFCLKSTARTED=0;  
    NRF_CLOCK->TASKS_HFCLKSTART=1;  
   // while(NRF_CLOCK->EVENTS_HFCLKSTARTED==0) 
		{
        //Do nothing.
    }	
	#endif
		
    NRF_TIMER2->MODE      = TIMER_MODE_MODE_Timer;
    NRF_TIMER2->BITMODE   = TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos;
    NRF_TIMER2->PRESCALER = TIMER_PRESCALERS;

    // 清除timer
    NRF_TIMER2->TASKS_CLEAR = 1;

    //设置timer  cc
    NRF_TIMER2->CC[PPI_CH_IN_CMP_1] = MAX_SAMPLE_LEVELS;


    // Interrupt setup.
    NRF_TIMER2->INTENSET = CAMPARE_INTERRENT;
	NRF_TIMER2->TASKS_START = 0;

	NVIC_SetPriority(TIMER2_IRQn, 3);

	NVIC_ClearPendingIRQ(TIMER2_IRQn);

	NVIC_EnableIRQ(TIMER2_IRQn);

}

/*
这是定时器中断函数：
通道1比较中断
*/
//1个step_tick 1.92ms
//

static const uint32_t temp_pwm_period[31] = {
	 25,30,29,29,28,28,28,27,27,26
	,26,25,25,24,24,20,20,19,19,18
	,18,17,17,16,15,14,13,12,10,9,9
};


void TIMER2_IRQHandler(void) //timer2中断函数
{

	uint32_t next_sample = (30-pwm_work.para_infor.temp_pwm_duty)*MAX_SAMPLE_LEVELS/30;
    if ((NRF_TIMER2->EVENTS_COMPARE[PPI_CH_IN_CMP_1] != 0) && 
       ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE1_Msk) != 0))
    {
		NRF_TIMER2->TASKS_CLEAR = 1;
        NRF_TIMER2->EVENTS_COMPARE[PPI_CH_IN_CMP_1] = 0;
        NRF_TIMER2->CC[PPI_CH_IN_CMP_1]             = MAX_SAMPLE_LEVELS;
		
		pwm_work.para_infor.cnt++;
		 if(pwm_work.para_infor.cnt< next_sample)
		 {
			 close_pwm_pin();
		 }
		 else if(pwm_work.para_infor.cnt<= MAX_SAMPLE_LEVELS)
		{
			open_pwm_pin();
		}
		else
		{
			close_pwm_pin();
			pwm_work.para_infor.cnt = 0;
			pwm_work.para_infor.temp_pwm++;
			if(pwm_work.para_infor.temp_pwm >temp_pwm_period[pwm_work.para_infor.temp_pwm_duty])
			{
				pwm_work.para_infor.temp_pwm = 0;
				pwm_work.para_infor.temp_pwm_duty += pwm_work.para_infor.direction;
				if(pwm_work.para_infor.temp_pwm_duty>28)
				{
					pwm_work.para_infor.direction = -1;
					
				}
				else if(pwm_work.para_infor.temp_pwm_duty < 1)
				{
					pwm_work.para_infor.direction = 1;
				}
					
			}
		}
    }
	
}


void pwm_base_init(void)
{
	memset(&pwm_work,0,sizeof(pwm_work));
	pwm_work.para_infor.direction = 1;
	pwm_gpiote_init();	
	timer2_init(); 
}

void pwm_infor_rest(void){

	memset(&pwm_work.para_infor,0,sizeof(pwm_work.para_infor));
	pwm_work.para_infor.direction = 1;
}

void pwm_ctrl(uint8_t ctrl)
{
	pwm_state = ctrl;
}


void pwm_work_set(void)
{
	static uint8_t last_state = PWM_NONE;
	if(last_state == pwm_state)
	{
		return;
	}
	last_state = pwm_state;
	if(pwm_state == PWM_NONE)
	{

	}
	else if(pwm_state == PWM_TRUE)
	{
		close_pwm_pin();
		NRF_TIMER2->TASKS_CLEAR = 1;
		NRF_TIMER2->CC[PPI_CH_IN_CMP_1] = MAX_SAMPLE_LEVELS;
		NRF_TIMER2->TASKS_START = 1;
	}
	else if(pwm_state == PWM_FALSE)
	{	
		NRF_TIMER2->TASKS_STOP = 1;
		NRF_TIMER2->TASKS_CLEAR = 1;
		NRF_TIMER2->EVENTS_COMPARE[PPI_CH_IN_CMP_1] = 0;
		close_pwm_pin();
		pwm_work.work_state = PWM_MODE_NONE;
	}
}

//521--1s
void set_pwm_start_stop_time(uint8_t work_mode)
{
#if 1
	//uint8_t period =30; 	//3s一个周期
	uint8_t stop_tick = 0;	//0.5s 递加

	if(pwm_work.work_state != work_mode)
		pwm_work.work_state = work_mode;
	else
		return;

	switch(work_mode)
	{
		case PWM_MODE_NONE:
			pwm_ctrl(PWM_FALSE);
			return;
			
		case PWM_MODE_1:
			stop_tick = 0;		

			break;
		case PWM_MODE_2:
			stop_tick = 5;//0.5s停止

			break;
		case PWM_MODE_3:
			stop_tick = 10;

			break;
		case PWM_MODE_4:
			stop_tick = 15;
			break;
		case PWM_MODE_5:
			stop_tick = 20;
			break;
		case PWM_MODE_6:
			stop_tick = 25;
			break;
		default:
			break;
			
	}
	pwm_ctrl(PWM_TRUE);
	pwm_work.work_time = (stop_tick)*521/10;
	pwm_infor_rest();
#endif
}

