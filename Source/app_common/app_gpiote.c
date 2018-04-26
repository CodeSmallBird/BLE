/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "app_gpiote.h"
#include <stdlib.h>
#include <string.h>
#include "app_util.h"
#include "nrf_error.h"
#include "nrf_gpio.h"
#include "ble_service.h"
#include "RD_MotoControl.h"
#include "ble_exdevice.h"
#include "nrf_delay.h"
/////////////////////////////
extern Receive_Cmd receivecmd;
extern uint16_t moto_cnt;
extern uint8_t g_lock_flag ;
extern uint8_t g_moto_flag ;
extern uint8_t g_bat_present;
extern uint8_t g_receicve_cmd;

extern void moto_timer_stop(void);
extern void moto_timer_start(void);
extern ble_system_clock_t  						open_clock;
extern ble_system_clock_t 						close_clock;
extern ble_system_clock_t                  		m_clock;
extern void gen_DeviceName(void);
extern uint16_t moto_cnt;
extern uint8_t g_heart_flag;


extern void EXIT_KEY_Init(nrf_gpio_pin_sense_t s);
extern void gen_DeviceName(void);
extern void buzzer_timer_stop(void);
extern void buzzer_timer_start(void);
/////////////////////////////



/**@brief GPIOTE user type. */
typedef struct
{
    uint32_t                   pins_mask;             /**< Mask defining which pins user wants to monitor. */
    uint32_t                   pins_low_to_high_mask; /**< Mask defining which pins will generate events to this user when toggling low->high. */
    uint32_t                   pins_high_to_low_mask; /**< Mask defining which pins will generate events to this user when toggling high->low. */
    uint32_t                   sense_high_pins;       /**< Mask defining which pins are configured to generate GPIOTE interrupt on transition to high level. */
    app_gpiote_event_handler_t event_handler;         /**< Pointer to function to be executed when an event occurs. */
} gpiote_user_t;

STATIC_ASSERT(sizeof(gpiote_user_t) <= GPIOTE_USER_NODE_SIZE);
STATIC_ASSERT(sizeof(gpiote_user_t) % 4 == 0);

static uint32_t        m_enabled_users_mask;          /**< Mask for tracking which users are enabled. */
static uint8_t         m_user_array_size;             /**< Size of user array. */
static uint8_t         m_user_count;                  /**< Number of registered users. */
static gpiote_user_t * mp_users = NULL;               /**< Array of GPIOTE users. */


/**@brief Function for toggling sense level for specified pins.
 *
 * @param[in]   p_user   Pointer to user structure.
 * @param[in]   pins     Bitmask specifying for which pins the sense level is to be toggled.
 */
static void sense_level_toggle(gpiote_user_t * p_user, uint32_t pins)
{
    uint32_t pin_no;
    
    for (pin_no = 0; pin_no < NO_OF_PINS; pin_no++)
    {
        uint32_t pin_mask = (1 << pin_no);
        
        if ((pins & pin_mask) != 0)
        {
            uint32_t sense;

            // Invert sensing.
            if ((p_user->sense_high_pins & pin_mask) == 0)
            {
                sense = GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos;
                p_user->sense_high_pins |= pin_mask;
            }
            else
            {
                sense = GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos;
                p_user->sense_high_pins &= ~pin_mask;
            }

            NRF_GPIO->PIN_CNF[pin_no] &= ~GPIO_PIN_CNF_SENSE_Msk;
            NRF_GPIO->PIN_CNF[pin_no] |= sense;
        }
    }
}

#if 1
void GPIOTE_IRQHandler_EX(uint32_t pins_state);

void GPIOTE_IRQHandler(void)
{
    uint8_t  i;
    uint32_t pins_changed;
    uint32_t pins_state = NRF_GPIO->IN;
    
    // Clear event.
    NRF_GPIOTE->EVENTS_PORT = 0;
//	GPIOTE_IRQHandler_EX(pins_state);
#if 1
    // Check all users.
    for (i = 0; i < m_user_count; i++)
    {
        gpiote_user_t * p_user = &mp_users[i];

        // Check if user is enabled.
        if (((1 << i) & m_enabled_users_mask) != 0)
        {
            uint32_t transition_pins;
            uint32_t event_low_to_high;
            uint32_t event_high_to_low;

            // Find set of pins on which there has been a transition.
            transition_pins = (pins_state ^ ~p_user->sense_high_pins) & p_user->pins_mask;

            // Toggle SENSE level for all pins that have changed state.
            sense_level_toggle(p_user, transition_pins);
            
            // Second read after setting sense.
            // Check if any pins have changed while serving this interrupt.
            pins_changed = NRF_GPIO->IN ^ pins_state;
            if (pins_changed)
            {
                // Transition pins detected in late stage.
                uint32_t late_transition_pins;

                pins_state          |= pins_changed;

                // Find set of pins on which there has been a transition.
                late_transition_pins = (pins_state ^ ~p_user->sense_high_pins) & p_user->pins_mask;

                // Toggle SENSE level for all pins that have changed state in last phase.
                sense_level_toggle(p_user, late_transition_pins);

                // Update pins that has changed state since the interrupt occurred.
                transition_pins |= late_transition_pins;
            }

            // Call user event handler if an event has occurred.
            event_high_to_low = (~pins_state & p_user->pins_high_to_low_mask) & transition_pins;
            event_low_to_high = (pins_state & p_user->pins_low_to_high_mask) & transition_pins;

            if ((event_low_to_high | event_high_to_low) != 0)
            {
                p_user->event_handler(event_low_to_high, event_high_to_low);
            }
        }
    }
#endif
}



void GPIOTE_IRQHandler_EX(uint32_t pins_state)
{

#if !defined(PIN_VER1)
#if defined(MT2503_HEART_PIN)
	if((pins_state>>MT2503_HEART_PIN)&0x01)
	{
		g_heart_flag = 1;	
#if dbg
		printf("mt2503 heart flag\r\n");
#endif
//		send_data_to _phone_time();
	}
#endif
#endif
	if(1)		
	{  
		nrf_delay_ms(20);
	#if !defined(PIN_VER1)
		if(((pins_state>>MOTO_STOP_PIN)&0x01)&&g_lock_flag == false)//ËÉ¿ª
		{
	
			g_lock_flag = true;
			nrf_delay_ms(5);
#if dbg
			printf("OPEN LOCK\r\n");
#endif
			memcpy(&open_clock,&m_clock,sizeof(ble_system_clock_t));	
			if(g_receicve_cmd)
			{
				nrf_gpio_pin_clear(BUZZER_EN);
				g_receicve_cmd = 0;
				moto_timer_stop();	
			}
			//tudo t0 2503
			send_data_to_phone(0x01,g_bat_present,0x00,receivecmd.cipher);
		#if defined(MT2503_GOTO_UNLOCK_PIN)
			mt2503_unlock_noticfy();
		#endif
			gen_DeviceName();
			EXIT_KEY_Init((nrf_gpio_pin_sense_t)NRF_GPIO_PIN_SENSE_LOW);
			return ;
		}
		else
	#endif
		if(!((pins_state>>MOTO_STOP_PIN)&0x01)&&g_lock_flag == true)																					//Ñ¹×¡
		{
#if dbg
			printf("CLOSE LOCK\r\n");			
#endif
			nrf_delay_ms(5);
			memcpy(&close_clock,&m_clock,sizeof(ble_system_clock_t));
			//mt2503_lock_noticfy();
			Moto_ENABLE();
			Moto_B2A();
			g_moto_flag = 2;
			moto_cnt = 0;
			nrf_gpio_pin_clear(BUZZER_EN);
			moto_timer_stop();
			moto_timer_start();
			buzzer_timer_stop();
			buzzer_timer_start();
			g_lock_flag = false;	
			gen_DeviceName();
			EXIT_KEY_Init((nrf_gpio_pin_sense_t)NRF_GPIO_PIN_SENSE_HIGH);
			return;
		}		  
		else
			return ;
	}
}


#else

/**@brief Function for handling the GPIOTE interrupt.
 */
void GPIOTE_IRQHandler(void)
{

	if((NRF_GPIOTE->EVENTS_IN[0]==1))		
	{  
		NRF_GPIOTE->EVENTS_IN[0]=0;  
//		nrf_delay_ms(00);
		if((((NRF_GPIO->IN)>>MOTO_STOP_PIN)&0x01)&&g_lock_flag == false)			//ËÉ¿ª
		{

			g_lock_flag = true;
#if dbg
			printf("OPEN LOCK\r\n");
#endif
			memcpy(&open_clock,&m_clock,sizeof(ble_system_clock_t));
			nrf_gpio_pin_clear(BUZZER_EN);
			moto_timer_stop();
			//tudo t0 2503
			send_data_to_phone(0x01,g_bat_present,0x00,receivecmd.cipher);
			mt2503_lock_noticfy();
			gen_DeviceName();

		}
		else if(!(((NRF_GPIO->IN)>>MOTO_STOP_PIN)&0x01)&&g_lock_flag == true)																					//Ñ¹×¡
		{
#if dbg
			printf("CLOSE LOCK\r\n");			
#endif
			memcpy(&close_clock,&m_clock,sizeof(ble_system_clock_t));
		#if defined(MT2503_GOTO_UNLOCK_PIN)
			mt2503_unlock_noticfy();
		#endif
			Moto_ENABLE();
			Moto_B2A();
			g_moto_flag = 2;
			moto_cnt = 0;
			nrf_gpio_pin_clear(BUZZER_EN);
			moto_timer_stop();
			moto_timer_start();
			g_lock_flag = false;	

			gen_DeviceName();
		}		  
	}
}

#endif


/**@brief Function for sense disabling for all pins for specified user.
 *
 * @param[in]  user_id   User id.
 */
static void pins_sense_disable(app_gpiote_user_id_t user_id)
{
    uint32_t pin_no;

    for (pin_no = 0; pin_no < 32; pin_no++)
    {
        if ((mp_users[user_id].pins_mask & (1 << pin_no)) != 0)
        {
            NRF_GPIO->PIN_CNF[pin_no] &= ~GPIO_PIN_CNF_SENSE_Msk;
            NRF_GPIO->PIN_CNF[pin_no] |= GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos;
        }
    }
}


uint32_t app_gpiote_init(uint8_t max_users, void * p_buffer)
{
    if (p_buffer == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Check that buffer is correctly aligned.
    if (!is_word_aligned(p_buffer))
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    // Initialize file globals.
    mp_users             = (gpiote_user_t *)p_buffer;
    m_user_array_size    = max_users;
    m_user_count         = 0;
    m_enabled_users_mask = 0;
    
    memset(mp_users, 0, m_user_array_size * sizeof(gpiote_user_t));
    
    // Initialize GPIOTE interrupt (will not be enabled until app_gpiote_user_enable() is called).
    NRF_GPIOTE->INTENCLR = 0xFFFFFFFF;
    
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    NVIC_SetPriority(GPIOTE_IRQn, APP_IRQ_PRIORITY_HIGH);
    NVIC_EnableIRQ(GPIOTE_IRQn);
    
    return NRF_SUCCESS;
}


uint32_t app_gpiote_user_register(app_gpiote_user_id_t *     p_user_id,
                                  uint32_t                   pins_low_to_high_mask,
                                  uint32_t                   pins_high_to_low_mask,
                                  app_gpiote_event_handler_t event_handler)
{
    // Check state and parameters.
    if (mp_users == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if (event_handler == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (m_user_count >= m_user_array_size)
    {
        return NRF_ERROR_NO_MEM;
    }
    
    // Allocate new user.
    mp_users[m_user_count].pins_mask             = pins_low_to_high_mask | pins_high_to_low_mask;
    mp_users[m_user_count].pins_low_to_high_mask = pins_low_to_high_mask;
    mp_users[m_user_count].pins_high_to_low_mask = pins_high_to_low_mask;
    mp_users[m_user_count].event_handler         = event_handler;
    
    *p_user_id = m_user_count++;
    
    // Make sure SENSE is disabled for all pins.
    pins_sense_disable(*p_user_id);
    
    return NRF_SUCCESS;
}


uint32_t app_gpiote_user_enable(app_gpiote_user_id_t user_id)
{
    uint32_t pin_no;
    uint32_t pins_state;

    // Check state and parameters.
    if (mp_users == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if (user_id >= m_user_count)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Clear any pending event.
    NRF_GPIOTE->EVENTS_PORT = 0;
    pins_state              = NRF_GPIO->IN;

    // Enable user.
    if (m_enabled_users_mask == 0)
    {
        NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
    }
    m_enabled_users_mask |= (1 << user_id);
    
    // Enable sensing for all pins for specified user.
    mp_users[user_id].sense_high_pins = 0;
    for (pin_no = 0; pin_no < 32; pin_no++)
    {
        uint32_t pin_mask = (1 << pin_no);
        
        if ((mp_users[user_id].pins_mask & pin_mask) != 0)
        {
            uint32_t sense;

            if ((pins_state & pin_mask) != 0)
            {
                sense = GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos;
            }                
            else
            {
                sense = GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos;
                mp_users[user_id].sense_high_pins |= pin_mask;
            }
            
            NRF_GPIO->PIN_CNF[pin_no] &= ~GPIO_PIN_CNF_SENSE_Msk;
            NRF_GPIO->PIN_CNF[pin_no] |= sense;
        }
    }
    
    return NRF_SUCCESS;
}


uint32_t app_gpiote_user_disable(app_gpiote_user_id_t user_id)
{
    // Check state and parameters.
    if (mp_users == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if (user_id >= m_user_count)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    // Disable sensing for all pins for specified user.
    pins_sense_disable(user_id);

    // Disable user.
    m_enabled_users_mask &= ~(1UL << user_id);
    if (m_enabled_users_mask == 0)
    {
        NRF_GPIOTE->INTENCLR = GPIOTE_INTENSET_PORT_Msk;
    }
    
    return NRF_SUCCESS;
}


uint32_t app_gpiote_pins_state_get(app_gpiote_user_id_t user_id, uint32_t * p_pins)
{
    gpiote_user_t * p_user;

    // Check state and parameters.
    if (mp_users == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if (user_id >= m_user_count)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Get pins.
    p_user  = &mp_users[user_id];
    *p_pins = NRF_GPIO->IN & p_user->pins_mask;
    
    return NRF_SUCCESS;
}
