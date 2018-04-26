#ifndef 	_NB_IOT_H_
#define 	_NB_IOT_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lenovo.h"
#include "nrf6310.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"


#define NB_POWER_PIN_INIT				nrf_gpio_cfg_output(AP_POWER_ON_PIN)
#define NB_POWER_PIN_LOW				nrf_gpio_pin_set(AP_POWER_ON_PIN) //nrf_gpio_pin_clear(AP_POWER_ON_PIN)
#define NB_POWER_PIN_HIGH				nrf_gpio_pin_clear(AP_POWER_ON_PIN) //nrf_gpio_pin_set(AP_POWER_ON_PIN)

#define NB_POWER_PIN_ON			        NB_POWER_PIN_HIGH
#define NB_POWER_PIN_OFF                NB_POWER_PIN_LOW


#define NB_RESET_PIN_INIT				nrf_gpio_cfg_output(AP_RESET_PIN)
#define NB_RESET_PIN_LOW				nrf_gpio_pin_clear(AP_RESET_PIN)	
#define NB_RESET_PIN_HIGH				nrf_gpio_pin_set(AP_RESET_PIN)


#define NB_SLEEP_PIN_INIT				nrf_gpio_cfg_output(MCU_WAKEUP_PIN)
#define NB_SLEEP_PIN_LOW				nrf_gpio_pin_clear(MCU_WAKEUP_PIN)	
#define NB_SLEEP_PIN_HIGH				nrf_gpio_pin_set(MCU_WAKEUP_PIN)

#define NB_SLEEP_STATE_PIN_INIT			    nrf_gpio_cfg_input(MODULE_WAKEUP_PIN,(nrf_gpio_pin_pull_t)NRF_GPIO_PIN_PULLUP)
#define GET_NB_SLEEP_STATE_PIN			    nrf_gpio_pin_read(MODULE_WAKEUP_PIN)

enum
{
	NB_STATE_LOW,
	NB_STATE_HGIH

};


/*		数据接收状态	*/
enum{
	NB_REVC_STATE_START_A,
	NB_REVC_STATE_START_T,
	NB_REVC_STATE_DATA,
};

typedef struct
{
	uint16_t ms_tick;
	uint8_t power_state;
	struct{
		uint8_t state;
		uint8_t send_flag;
	}sim_register;
	struct{
		uint8_t state;
		uint8_t send_flag;
	}net_register;
	struct{
		uint8_t state;
		uint8_t send_flag;
	}gps_register;
	struct
	{
		uint8_t step;
		uint8_t send_flag;
	}nb_send_step;

	struct
	{
		uint8_t step;
		uint8_t send_flag;
	}psm_step;

	struct
	{
		uint8_t step;
		struct
		{
			uint8_t send_step;
			uint8_t send_flag;
		}order;
	}psm_exit_step;
	
	uint8_t recv_deal_state;
	uint8_t work_mode;

}NB_INFOR;

extern NB_INFOR nb_infor;

#define SEND_NET_LENGTH		  200
typedef struct
{
	uint8_t data[SEND_NET_LENGTH];
	uint8_t length;

}SEND_NET_INFOR;
extern SEND_NET_INFOR send_net_infor;


enum
{
	NB_POWER_ON_HIGH,
	NB_POWER_ON_LOW,
	NB_POWER_ON_WAITE,
};

enum
{
	ATV_X_SET,
	ATE_X_SET,
	CPIN_CHECK,	
	SET_CEREG,
	SET_MODODR,
	CHECK_MODODR,
	CHECK_CEREG,
	START_GPS,
	AT_GET_GPS,
	REGISTER_OK,
};

enum
{
	SEND_GET_GPS_DATA,
	GET_GPS_OK,
};


enum
{
	NB_MODE_WAIT,
	NB_MODE_POWE_ON_MODE,
	NB_REGISTER_APN_MODE,
	NB_REGISTER_NET_MODE,
	NB_GET_GPS_INFOR_MODE,
	NB_DATA_SEN_RECV_MODE,
	NB_PSM_MODE,
	NB_PSM_EXIT_MODE,
};


enum
{
	PRE_CHECK_LSIPCALL,
	CMMNET_SET,
	CHECK_LSIPCALL,	
	LSIPCALL_SET,	
	SET_NET_OK,
};

enum
{
	LSIPOPEN_SET,
	LSIPOPEN_CHECK,
	SEND_READY,
	START_SEND,
	SEND_OK,
	
};


enum
{
	PSM_SET,
	PSM_SET_OK,
};

enum
{
	PSM_EXIT_SEND_CLOSE_PSM,
	PSM_EXIT_SEND_ATV_X_SET,
	PSM_EXIT_SEND_ATE_X_SET,
	PSM_EXIT_SEND_CEREG_SET,
	PSM_EXIT_SEND_CLOSE_OK,
};


enum
{
	NB_POWER_ON_SET,
	SET_PSM_EXIT_SET,
};



#define GET_DEALY_TIME              		 		nb_infor.ms_tick
#define SET_DEALY_TIME(x)          			        do{nb_infor.ms_tick = x/8;}while(0)   

#define GET_NB_RECV_STATE         					nb_infor.recv_deal_state
#define SET_NB_RECV_STATE(x)                   		do{nb_infor.recv_deal_state =x;}while(0)   


#define GET_REGISTER_STATE   		      			nb_infor.sim_register.state
#define SET_REGISTER_STATE(x)                   	do{nb_infor.sim_register.state =x;}while(0)   
#define GET_REGISTER_FLAG                       	nb_infor.sim_register.send_flag
#define SET_REGISTER_FLAG(x)          		        do{nb_infor.sim_register.send_flag =x;}while(0) 

#define GET_NET_REGISTER_STATE   		      		nb_infor.net_register.state
#define SET_NET_REGISTER_STATE(x)                   do{nb_infor.net_register.state =x;}while(0)   
#define GET_NET_REGISTER_FLAG                       nb_infor.net_register.send_flag
#define SET_NET_REGISTER_FLAG(x)                    do{nb_infor.net_register.send_flag =x;}while(0) 

#define GET_GPS_REGISTER_STATE   		      		nb_infor.gps_register.state
#define SET_GPS_REGISTER_STATE(x)                   do{nb_infor.gps_register.state =x;}while(0)   
#define GET_GPS_REGISTER_FLAG                       nb_infor.gps_register.send_flag
#define SET_GPS_REGISTER_FLAG(x)                    do{nb_infor.gps_register.send_flag =x;}while(0) 

#define GET_NB_WORK_MODE         	      		  	nb_infor.work_mode
#define SET_NB_WORK_MODE(x)       	           		do{nb_infor.work_mode =x;}while(0)   

#define GET_NB_SEND_STEP             	  		  	nb_infor.nb_send_step.step
#define SET_NB_SEND_STEP(x)          	         	do{nb_infor.nb_send_step.step =x;}while(0)   
#define GET_NB_SEND_FLAG            	   		  	nb_infor.nb_send_step.send_flag
#define SET_NB_SEND_FLAG(x)         	   	       	do{nb_infor.nb_send_step.send_flag =x;}while(0)   


#define GET_NB_PSM_SEND_STEP               		  	nb_infor.psm_step.step
#define SET_NB_PSM_SEND_STEP(x)        	           	do{nb_infor.psm_step.step =x;}while(0)   
#define GET_NB_PSM_SEND_FLAG          	     		nb_infor.psm_step.send_flag
#define SET_NB_PSM_SEND_FLAG(x)       	            do{nb_infor.psm_step.send_flag =x;}while(0)   


#define GET_NB_PSM_EXIT_STEP               		  	nb_infor.psm_exit_step.step
#define SET_NB_PSM_EXIT_STEP(x)        	           	do{nb_infor.psm_exit_step.step =x;}while(0) 

#define GET_NB_PSM_EXIT_SEND_STEP      	     		nb_infor.psm_exit_step.order.send_step
#define SET_NB_PSM_EXIT_SEND_STEP(x)   	            do{nb_infor.psm_exit_step.order.send_step =x;}while(0)  
#define GET_NB_PSM_EXIT_SEND_FLAG      	     		nb_infor.psm_exit_step.order.send_flag
#define SET_NB_PSM_EXIT_SEND_FLAG(x)   	            do{nb_infor.psm_exit_step.order.send_flag =x;}while(0)  

 
extern void nbiot_param_init(void);
extern	void nb_work_cycle(void);
extern void nb_recv_data_deal(void);


#if defined(NB_DEBUG)
extern void DebugSet(void);
extern void DebugClose(void);

#define nb_debug(args...) 	\
	do{DebugSet();printf(##args);\
		nrf_delay_ms(5);DebugClose();}while(0)
#else
#define nb_debug(fmt, args...) 	do {} while (0)
#endif



#endif

