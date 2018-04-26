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
#ifndef NRF6310_H__
#define NRF6310_H__



#define ADD_WOR_MODE	//添加工作模式指令

#define CHANG_RED_LED_WORK		//改变红灯工作模式

#define CHAGRE_REST_ONCE			//���ʱ����һ���踴λ


#define FLASH_READ_WRITE				//���flash��д


#define ADD_NRF_PWM_CTRL			//	���PWMģʽ����
#if defined(ADD_NRF_PWM_CTRL)
#define CHANGE_MODE_CTRL	//	����ģʽ����

//#define CTRL_POWER_OUT	//���������ѹ
#endif

#if defined(CHANG_RED_LED_WORK)
enum
{
	BLE_BREAK,
	BLE_CONNECT,
	BLE_LOW_POWER,
	BLE_CHARGE,
	BLE_CHARGE_OK,
};

enum
{
	LIGHT_OFF,
	LIGHT_ON,
	LIGHT_FLASH1,
	LIGHT_FLASH2,
	LIGHT_FLASH3,
};
	
extern void set_led_mode(uint8_t set_mode);
extern void charge_ble_work(void);
extern bool charge_flag;

extern void Rd_MotoShakeMode(bool isShake,uint8_t shakeType,uint8_t shakeSel,bool shakeDirection);


#endif

//#define TEST_LED

/*
//==========正面===========
#define MOTO_PIN_01       5
#define MOTO_PIN_02       8
#define MOTO_PIN_03       9
#define MOTO_PIN_04       10
#define MOTO_PIN_05       11
#define MOTO_PIN_06       12
#define MOTO_PIN_07       13
#define MOTO_PIN_08       14
#define MOTO_PIN_09       15
#define MOTO_PIN_10       16

//==========背面===========
#define MOTO_PIN_11       2
#define MOTO_PIN_12       3
#define MOTO_PIN_13       4
#define MOTO_PIN_14       23
#define MOTO_PIN_15       24
#define MOTO_PIN_16       25
#define MOTO_PIN_17       26
#define MOTO_PIN_18       27
#define MOTO_PIN_19       28
#define MOTO_PIN_20       29
*/
	//==========正面===========
#if defined(ADD_NRF_PWM_CTRL)	
#define MOTO_PIN_01       5
#define MOTO_PIN_02       8
#else
#define MOTO_PIN_01       5
#define MOTO_PIN_02       8
#define MOTO_PIN_03       9
#define MOTO_PIN_04       10
#define MOTO_PIN_05       11
#define MOTO_PIN_06       12
#endif
//=======其他引脚定义======
#define DET_5V            7
#define CHARGE_OK         6

//#define LED_BLUE          18
#if defined(TEST_LED)
#define LED_RED           17//17
#define TEST_RED           25//17
#else
#define LED_RED     		 25	//17
#endif

#define NOR_USER_MOTOR	                 0

#endif  // NRF6310_H__
