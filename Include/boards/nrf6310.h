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
//#define _SUPPORT_BM250_

#define CHANG_PROTOCAL					// �ı�Э��

//#define DEBUG_UART

#define CUT_CODE


#define ADD_HELP_FUNCV_ORDER						//��������ָ��Э��

//#define TEST_USE_BLE_SEND_REPLACE_2503		//����������2503

#define CONTROL_UART							// ���ƴ��ڿ��أ����ٵ���
#define SECOND_30_OUT_NUOPEN_TO_LOCK			//30����û�п����Զ�����
#define SEND_80KEY_SECRET					//�Է��͵�80��key���ַ����м���
#define ADD_CHECK_IN_KEY						//	���check_in_key ָ��


#define FLASH_READ_WRITE						// flash �� ����д����

//#define CHANGE_UART_DEAL_PLACE			       //�ı䴮�ڴ�����
#define KEY_IO_SET_PULL_DOWN	// ��λ���״̬����
#if defined(KEY_IO_SET_PULL_DOWN)
#define USE_KEY_HIGH_OK
#endif


#if defined(ADD_CHECK_IN_KEY)

#define CHECK_IN_KEY_INDEX                   8

#endif
#if 0
#define MT2503_HEART_PIN  	 5//1//5			//B2	//dingtian
#define MOTO_STOP_PIN		     2			//KEY	dingtian
#define BLE_HEART_PIN			 6			//B3

#define MT2503_B1				     4
#define MT2503_B2 				 5
#define MT2503_B3 			   	 6
#define MT2503_B4 				 7

#define MT2503_GOTO_UNLOCK_PIN	 4			//B1
#define MT2503_GOTO_LOCK_PIN	 7			//B4

#define MT2503_RESET_PIN	     12		//dingtian

#define MT2503_TX_PIN			 4	//dingtian
#define MT2503_RX_PIN 			 7
#define BUZZER_EN 				 10
#define MOTO_EN   				 0
#define IA_EN 					 30
#define IB_EN 					 29

#else
//#define MT2503_HEART_PIN  	 5//1//5			//B2	//dingtian
#define MOTO_STOP_PIN		     1//2			//KEY	dingtian
//#define BLE_HEART_PIN			 4//6			//B3

//#define MT2503_B1				     4
//#define MT2503_B2 				 5
//#define MT2503_B3 			   	 6
//#define MT2503_B4 				 7

#define MT2503_GOTO_UNLOCK_PIN	 4			//B1
#define MT2503_GOTO_LOCK_PIN	 7			//B4

#define MT2503_RESET_PIN	     11//12		//dingtian

#define MT2503_TX_PIN			7//4	//dingtian
#define MT2503_RX_PIN 			6//7
#define BUZZER_EN 				 10
#define MOTO_EN   				 0
#if defined(KEY_IO_SET_PULL_DOWN)
#define IA_EN 					30 // 30
#define IB_EN 					29 // 29
#else
#define IA_EN 					30 // 30
#define IB_EN 					29 // 29
#endif
#if defined(_SUPPORT_BM250_)
#define TWI_MASTER_CONFIG_CLOCK_PIN_NUMBER 					14
#define TWI_MASTER_CONFIG_DATA_PIN_NUMBER 					13
#endif

#endif

//************IA_EN = 30 IB_EN = 29***********/




//=======�������Ŷ���======
#define DET_5V            7
#define CHARGE_OK         6

//#define LED_BLUE          18
#define LED_RED           25//17


#endif  // NRF6310_H__
