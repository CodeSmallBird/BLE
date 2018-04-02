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


//#define DEBUG_UART
//#define ADD_NFC_CARD_FUNCV	//鏄惁娣诲姞NFC 鍔熻兘
//#define PIN_V2
#define ADD_NFC_CARD_FUNCV2	// 读写卡功能
//#define TEST_CARD_DEBUG_FUNCV	//测试时可以打开该宏

#if defined(ADD_NFC_CARD_FUNCV2)
#define FLASH_READ_WRITE

//#define ADD_HOER_DETEC		//添加霍尔传感器

#define CTRL_NFC_POWER	//控制NFCpower电源

#define CARD_CONTEN_UPDATE		//修改写卡和加密内容

#define  ADD_BT_OPEN_LOCK		    // 添加蓝牙开锁功能

#endif

#if defined(ADD_NFC_CARD_FUNCV)
#define NFC_IRQ_PIN			 17			
#endif

#if defined(ADD_NFC_CARD_FUNCV)
#define MT2503_HEART_PIN  	 7//1//5			//B2
#define MOTO_STOP_PIN		     30			//KEY
//#define BLE_HEART_PIN			 6			//B3

//#define MT2503_B1				     4
//#define MT2503_B2 				 5
//#define MT2503_B3 			   	 6
//#define MT2503_B4 				 7

//#define MT2503_GOTO_UNLOCK_PIN	 4			//B1
//#define MT2503_GOTO_LOCK_PIN	 7			//B4

//#define MT2503_RESET_PIN	     12

#define MT2503_TX_PIN			 	5 //4
#define MT2503_RX_PIN 		 		4 //5


#define BUZZER_EN 				 10
#define MOTO_EN   				 11
#define IA_EN 					 9
#define IB_EN 					 8
//************IA_EN = 30 IB_EN = 29***********/




//=======鍏朵粬寮曡剼瀹氫箟======
//#define DET_5V            7
//#define CHARGE_OK         6

//#define LED_BLUE          18
//#define LED_RED           25//17

#elif defined(PIN_V2)
#define MT2503_HEART_PIN  	 25//1//5			//B2

#define MOTO_STOP_PIN				16//	1
#define IB_EN  		29
#define IA_EN  		30
#define BUZZER_EN	10
#define MOTO_EN		0
#define ADC_IN		3

#define MT2503_TX_PIN	6//4
#define MT2503_RX_PIN 	7		 //7
//#define INT1_PIN	6

#else
#define MT2503_HEART_PIN  	 7//1//5			//B2
#define MOTO_STOP_PIN		     30			//KEY
//#define BLE_HEART_PIN			 6			//B3

//#define MT2503_B1				     4
//#define MT2503_B2 				 5
//#define MT2503_B3 			   	 6
//#define MT2503_B4 				 7

//#define MT2503_GOTO_UNLOCK_PIN	 4			//B1
//#define MT2503_GOTO_LOCK_PIN	 7			//B4

//#define MT2503_RESET_PIN	     12

#define MT2503_TX_PIN			 	5//4
#define MT2503_RX_PIN 		 		4//5


#define BUZZER_EN 				 10
#define MOTO_EN   				 11
#define IA_EN 					 9
#define IB_EN 					 8

//************IA_EN = 30 IB_EN = 29***********/

#if defined(ADD_HOER_DETEC)
#define KEY_TEST 19
#endif
#if defined(CTRL_NFC_POWER)
#define NFC_POWER_PIN		6
#endif


//=======鍏朵粬寮曡剼瀹氫箟======
//#define DET_5V            7
//#define CHARGE_OK         6

//#define LED_BLUE          18
//#define LED_RED           25//17

#endif

#endif  // NRF6310_H__
