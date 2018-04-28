#include "pwm.h"
volatile uint32_t pwm_duty = 6;
static uint8_t pwm_state = PWM_NONE;
#if defined(CHANGE_MODE_CTRL)
PWM_WORK pwm_work;
#endif

//ʹ��������ͨ������ͨ����GPIOTE���������
void ppi_init(void)
{
/*
ѡ��PPIͨ�����ӵ����������豸
���룺timer2�Ķ�ʱ�Ƚ���0,1,2
���:�����¼�1
*/
    NRF_PPI->CH[PPI_CH_CC0].EEP = (uint32_t)&NRF_TIMER2->EVENTS_COMPARE[PPI_CH_IN_CMP_0];
    NRF_PPI->CH[PPI_CH_CC0].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[PPI_CH_OUT_GPIOTE_CH];

    NRF_PPI->CH[PPI_CH_CC1].EEP = (uint32_t)&NRF_TIMER2->EVENTS_COMPARE[PPI_CH_IN_CMP_1];
    NRF_PPI->CH[PPI_CH_CC1].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[PPI_CH_OUT_GPIOTE_CH];
    
    NRF_PPI->CH[PPI_CH_CC2].EEP = (uint32_t)&NRF_TIMER2->EVENTS_COMPARE[PPI_CH_IN_CMP_2];
    NRF_PPI->CH[PPI_CH_CC2].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[PPI_CH_OUT_GPIOTE_CH];

#if 1
    //����ͨ��
    NRF_PPI->CHEN = PPI_CH_CC0_ENABLE
                    | PPI_CH_CC1_ENABLE
                    | PPI_CH_CC2_ENABLE;
#endif
}

//����pwm_pin���ú���
//ʹ����GPIOTEģ��


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


void pwm_gpiote_init(void)  //io���
{
    // Connect GPIO input buffers and configure PWM_OUTPUT_PIN_NUMBER as an output.
	close_pwm_pin();
}

//���Ƕ�ʱ����ʼ��
//��ʼ���˶�ʱ���������Ƚ������򿪱Ƚ���1���жϹ���
void timer2_init(void) 
{
    // ����16mʱ��
	#if 1
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    // �ȴ�ʱ�ӿ���
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

    // ���timer
    NRF_TIMER2->TASKS_CLEAR = 1;

    //����timer  cc
    NRF_TIMER2->CC[PPI_CH_IN_CMP_1] = MAX_SAMPLE_LEVELS;


    // Interrupt setup.
    NRF_TIMER2->INTENSET = CAMPARE_INTERRENT;
	NRF_TIMER2->TASKS_START = 0;

	NVIC_SetPriority(TIMER2_IRQn, 3);

	NVIC_ClearPendingIRQ(TIMER2_IRQn);

	NVIC_EnableIRQ(TIMER2_IRQn);

}

/*
���Ƕ�ʱ���жϺ�����
ͨ��1�Ƚ��ж�
*/
//1��step_tick 1.92ms
//
#if 0
static const uint32_t temp_pwm_period[31] = {
	 25,30,29,29,28,28,28,27,27,26
	,26,25,25,24,24,20,20,19,19,18
	,18,17,17,16,15,14,13,12,10,9,9
};
#else
static const uint32_t temp_pwm_period[31] = {
	 25,25,25,2,2,2,2,2,2,2
	,2,2,2,2,2,2,2,2,2,2
	,18,17,17,16,15,14,13,12,10,9,9
};


#endif
void TIMER2_IRQHandler(void) //timer2�жϺ���
{
	int period = MAX_SAMPLE_LEVELS+pwm_duty*2;
	uint32_t next_sample = (30-pwm_work.para_infor.temp_pwm_duty)*period/30;
    if ((NRF_TIMER2->EVENTS_COMPARE[PPI_CH_IN_CMP_1] != 0) && 
       ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE1_Msk) != 0))
    {
		NRF_TIMER2->TASKS_CLEAR = 1;
        NRF_TIMER2->EVENTS_COMPARE[PPI_CH_IN_CMP_1] = 0;
        NRF_TIMER2->CC[PPI_CH_IN_CMP_1]     = period;
		
		pwm_work.para_infor.cnt++;
		 if(pwm_work.para_infor.cnt< next_sample)
		 {
			 close_pwm_pin();
		 }
		 else if(pwm_work.para_infor.cnt<= period)
		{
			open_pwm_pin();
		}
		else
		{
			close_pwm_pin();
			pwm_work.para_infor.cnt = 0;
			pwm_work.para_infor.temp_pwm++;
			if(pwm_work.para_infor.temp_pwm >
				(temp_pwm_period[pwm_work.para_infor.temp_pwm_duty]-pwm_work.para_infor.mode))
			{
				pwm_work.para_infor.temp_pwm = 0;
				pwm_work.para_infor.temp_pwm_duty += pwm_work.para_infor.direction;
				//if(pwm_work.para_infor.temp_pwm_duty>(28-pwm_duty*2))	//
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
	//uint8_t period =30; 	//3sһ������
	uint8_t stop_tick = 0;	//0.5s �ݼ�

	if(pwm_work.work_state != work_mode)
		pwm_work.work_state = work_mode;
	else
		return;
	pwm_work.para_infor.mode = work_mode;

	switch(work_mode)
	{
		case PWM_MODE_NONE:
			pwm_ctrl(PWM_FALSE);
			return;
			
		case PWM_MODE_1:
			stop_tick = 0;		

			break;
		case PWM_MODE_2:
			stop_tick = 5;//0.5sֹͣ

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

