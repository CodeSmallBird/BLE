#ifndef _BLE_SERVICE_H_
#define _BLE_SERVICE_H_
#include "common.h"

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf51_bitfields.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "ble_sensorsim.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "ble_error_log.h"
#include "ble_bondmngr.h"
#include "ble_debug_assert_handler.h"
#include "pstorage.h"
#include "ble_trans.h"
#include"nbiot.h"
//#define DEVICE_NAME                          "UGO10000531"                              /**< Name of device. Will be included in the advertising data. */

#define DEVICE_NAME                          "UGO23178950"                              /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                    "Shenzhen AnTai ChunHui Co.,Ltd."/**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_TIMEOUT_IN_SECONDS           0                                          /**< The advertising timeout in units of seconds. */

#define APP_TIMER_PRESCALER                  0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS                 16//7                                          /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE              10//4                                          /**< Size of timer operation queues. */
//#define DETECTION_BUF_INTERVAL               APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)   /**< Sensor Contact Detected toggle interval (ticks). */
#define DETECTION_BUF_INTERVAL               APP_TIMER_TICKS(8, APP_TIMER_PRESCALER)   /**< Sensor Contact Detected toggle interval (ticks). */

#define MIN_CONN_INTERVAL                    MSEC_TO_UNITS(100, UNIT_1_25_MS)          	/**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL                    MSEC_TO_UNITS(600, UNIT_1_25_MS)          	/**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                        0                                          /**< Slave latency. */
#define CONN_SUP_TIMEOUT                     MSEC_TO_UNITS(4000, UNIT_10_MS)            /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY        APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT         3                                          /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_TIMEOUT                    30                                         /**< Timeout for Pairing Request or Security Request (in seconds). */
#define SEC_PARAM_BOND                       1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                       0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES            BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                        0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE               7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE               16                                         /**< Maximum encryption key size. */

#define DEAD_BEEF                            0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name);
void send_data_to_ble(uint8_t* send_buf, uint8_t size);
void timers_init(void);
void gap_params_init(void);
void advertising_init(void);
void services_init(void);
void sec_params_init(void);

void application_timers_start(void);
void application_timers_stop(void);
void advertising_start(void);
void advertising_stop(void);

void conn_params_init(void);
void ble_stack_init(void);
void bond_manager_init(void);
void power_manage(void);

//蓝牙初始�?
void BleInit(pstorage_handle_t *handle);

//打包发�?

void send_data_to_phone(uint8_t cmd,uint8_t status,uint8_t reset,unsigned int cipher);


void MT2503_Reset(void);


typedef struct eventDelay
{
	unsigned char event;
	unsigned int delay[3];
}EventDelay;



typedef struct receive_cmd
{
	unsigned char cmd;
	unsigned char reset;
	unsigned int cipher;
}Receive_Cmd;

//#define dbg  1
//#define ble_dbg 1
#define B4_UART 1

void ble_evt_dispatch(ble_evt_t * p_ble_evt);
unsigned int encrypt(int plainText, int key)  ;
int LOCK_Status(Receive_Cmd *rec);
void send_data_to_phone_time(void);

/////////////////////////////////////
void mt2503_lock_noticfy(void);
void mt2503_unlock_noticfy(void);
void Moto_ENABLE(void);
void Moto_DISABLE(void);
void Moto_A2B(void);
void Moto_B2A(void);
/////////////////////////////////////
int Shangxun_Moto_return(uint8_t cmd,uint8_t status,uint8_t reset,unsigned int cipher);
#endif // _BLE_SERVICE_H_


