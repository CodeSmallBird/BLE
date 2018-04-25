#ifndef _PWM_H_
#define _PWM_H_
#include <stdbool.h>
#include <stdint.h>
#include<string.h>
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include"nrf_gpiote.h"
#include"nrf6310.h"
#include "nrf51.h"



enum
{
	PWM_NONE,
	PWM_FALSE,
	PWM_TRUE,
};


//PPI 的参数设置
/*
选择通道1,2,3
*/
#define PPI_CH_CC0 		    1		// 在I2C中用了0，所以这里选择1
#define PPI_CH_CC1 		    2
#define PPI_CH_CC2    		3
#define PPI_CH_CC0_ENABLE       		(PPI_CHEN_CH1_Enabled << PPI_CHEN_CH1_Pos)
#define PPI_CH_CC1_ENABLE       		(PPI_CHEN_CH2_Enabled << PPI_CHEN_CH2_Pos)
#define PPI_CH_CC2_ENABLE       		(PPI_CHEN_CH3_Enabled << PPI_CHEN_CH3_Pos)


#define PPI_CH_CC0_DISABLE       		(PPI_CHEN_CH1_Disabled << PPI_CHEN_CH1_Pos)
#define PPI_CH_CC1_DISABLE       		(PPI_CHEN_CH2_Disabled << PPI_CHEN_CH2_Pos)
#define PPI_CH_CC2_DISABLE       		(PPI_CHEN_CH3_Disabled << PPI_CHEN_CH3_Pos)


/*
time2 比较通道 0,1,2的配置
并设置通道1中断
*/
#define PPI_CH_IN_CMP_0		    0
#define PPI_CH_IN_CMP_1		    1
#define PPI_CH_IN_CMP_2		    2
#define CAMPARE_INTERRENT              TIMER_INTENSET_COMPARE1_Enabled << TIMER_INTENSET_COMPARE1_Pos

#define PWM_PIN_01			MOTO_PIN_02
#define PWM_PIN_02			MOTO_PIN_01
#define PPI_CH_OUT_GPIOTE_CH		    1		// 在I2C中用了0，所以这里选择gtioe选择通道1
#define PPI_CH_OUT_GPIOTE_CH_2             2		// 在I2C中用了0，所以这里选择gtioe选择通道1


#define MAX_SAMPLE_LEVELS        10          //1400			// PWM周期
#define DUTY_SAMPLE_LEVELS              10            			// pwm 占空比 
#define TIMER_PRESCALERS                8


typedef enum
{
    NRF_PPI_CHANNEL_GROUP0 = 0, /**< Channel group 0. */
    NRF_PPI_CHANNEL_GROUP1 = 1, /**< Channel group 1. */
    NRF_PPI_CHANNEL_GROUP2 = 2, /**< Channel group 2. */
    NRF_PPI_CHANNEL_GROUP3 = 3, /**< Channel group 3. */
#ifdef NRF52
    NRF_PPI_CHANNEL_GROUP4 = 4, /**< Channel group 4. */
    NRF_PPI_CHANNEL_GROUP5 = 5  /**< Channel group 5. */
#endif
} nrf_ppi_channel_group_t;

//static uint32_t            m_channels_allocated;   /**< Bitmap representing channels availability. 1 when a channel is allocated, 0 otherwise. */

#ifdef NRF52

    #define NRF_PPI_ALL_APP_CHANNELS_MASK   ((uint32_t)0xFFFFFFFFuL & ~(NRF_PPI_CHANNELS_USED))  /**< All PPI channels available to the application. */
    #define NRF_PPI_PROG_APP_CHANNELS_MASK  ((uint32_t)0x000FFFFFuL & ~(NRF_PPI_CHANNELS_USED))  /**< Programmable PPI channels available to the application. */
    #define NRF_PPI_ALL_APP_GROUPS_MASK     ((uint32_t)0x0000003FuL & ~(NRF_PPI_GROUPS_USED))    /**< All PPI groups available to the application. */

#else

    #define NRF_PPI_ALL_APP_CHANNELS_MASK   ((uint32_t)0xFFF0FFFFuL & ~(NRF_PPI_CHANNELS_USED))  /**< All PPI channels available to the application. */
    #define NRF_PPI_PROG_APP_CHANNELS_MASK  ((uint32_t)0x0000FFFFuL & ~(NRF_PPI_CHANNELS_USED))  /**< Programmable PPI channels available to the application. */
    #define NRF_PPI_ALL_APP_GROUPS_MASK     ((uint32_t)0x0000000FuL & ~(NRF_PPI_GROUPS_USED))    /**< All PPI groups available to the application. */

#endif

#if defined(CHANGE_MODE_CTRL)
enum
{
	PWM_MODE_NONE,
	PWM_MODE_1,
	PWM_MODE_2,
	PWM_MODE_3,
	PWM_MODE_4,
	PWM_MODE_5,
	PWM_MODE_6,
};

typedef struct
{
	uint32_t work_time;
	uint32_t stop_time;
	uint32_t work_state;
}PWM_WORK;

extern PWM_WORK pwm_work;
#endif

extern volatile uint32_t  pwm_duty;
extern void TIMER2_IRQHandler(void);
extern void pwm_base_init(void);
extern void pwm_ctrl(uint8_t pwm_switch);
extern void pwm_work_set(void);
extern void set_pwm_start_stop_time(uint8_t work_mode);

#endif

