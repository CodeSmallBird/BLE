#ifndef _PROTOCOL_808_H
#define _PROTOCOL_808_H

#include "nrf_delay.h"
#include "nrf6310.h"
#include "ble_service.h"

#include "nrf_gpio.h"
#include <stdbool.h>
#include <stdint.h>
#include "nrf_soc.h"
#include "core_cminstr.h"
#include "app_error.h"


#define LOCK_GENERAL_RESP				0x0001        
#define APP_GENERAL_RESP				0x8001
#define LOCK_HEARTBEAT					0x0002
#define LOCK_REGISTER					0x0100
#define LOCK_REGISTER_RESP				0x8100

#define LOCK_AUTHENTICATION				0x0102
#define SET_LOCK_PARAMETER				0x8103
#define CHECK_LOCK_PARAMETER			0x8104
#define CHECK_LOCK_PARAMETER_RESP					0x0104
#define LOCK_POSTION_RESP				0x0200
#define APP_CTRL_LOCK1					0x8500
#define LOCK_CTRL_RESP					0x0500
#define CHECK_LOCK_ATTRIBUTE			0x8107
#define LOCK_ATTRIBUTE_RESP				0x0107
#define APP_UPDATE_LOCK					0x8108
#define APP_CTRL_LOCK2					0x8105
#define APP_REQUEST_POSITION			0x8201
#define REQUEST_POSITION_RESP			0x0201


#define AUTHEN_LENGTH 	20
#define HARD_WARE_LEN	5
#define SOFT_WARE_LEN	5
#define RECV_LENTH		80
#define DECRYP_LENTH	80
#define SEND_MSG_LENTH	80

#define APN_NAME_LEN	20
#define IP_NAME_LEN	20


extern char *apn_name;
extern char *ip_name;

typedef union
{
	struct 
	{
		uint16_t length:10; 
		uint16_t rsa:3; 
		uint16_t frames:1; 
		uint16_t rev:2;
	}Member;
	uint16_t all;
}NATURE;

typedef struct
{
	uint16_t total;
	uint16_t num;
}FRAME;


typedef struct
{
	uint16_t 	id;
	NATURE 		nature;
	uint8_t 	bcd[6];
	uint16_t	number;
	FRAME		frame;
}HEAD_MSG;



typedef struct
{
	uint8_t manufacturer[5];
	uint8_t uuid[20];
	uint8_t mac[7];
	uint8_t mode;
	uint8_t iccid[10];
}LOCK_INFOR;


typedef struct
{
	uint16_t province;
	uint16_t town;
	LOCK_INFOR lock_info;
}LOCK_REGISTER_INFOR;


typedef struct
{
	uint16_t resp_num;
	uint16_t app_id;
	uint8_t result;
}GENERAL_RESP_INFOR;

typedef struct
{
	uint8_t data[RECV_LENTH];
	uint8_t len;
}RECV_DATA_INFOR;

typedef struct
{
	uint8_t data[SEND_MSG_LENTH];
	uint8_t len;
}SEND_DATA_INFOR;

typedef struct		//解压接收并处理后的数据
{
	uint8_t data[RECV_LENTH];
	uint8_t len;
}DECRYP_DATA_INFOR;

typedef struct
{
	uint8_t data[DECRYP_LENTH];
	uint8_t len;
}SEND_MSG_INFOR;



typedef struct
{
	uint8_t hardware[HARD_WARE_LEN];
	uint8_t software[SOFT_WARE_LEN];
}DEVICE_VER_INFOR;


typedef struct
{
	uint16_t msg_num;
	uint8_t result;
	char*	authen_code;

}LOCK_REGISTER_REP;

typedef struct
{
	uint8_t acc_state:1			;
	uint8_t location_state:1	;	
	uint8_t latitude_state:1	;
	uint8_t longitude_state:1	;
}LOCK_STATE_INFOR;

typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
}TIME_INFOR;

typedef struct
{
	uint8_t warn_flag;	
	LOCK_STATE_INFOR lock_state;
	uint8_t latitude[4];
	uint8_t longitude[4];
	uint16_t elevation;
	uint16_t speed;
	uint16_t direction;
	TIME_INFOR time;
}LOCK_POSITION_INFOR;


//附加信息
typedef struct
{
	uint8_t id;
	uint8_t len;
	uint8_t *msg;
}ADJUNCT_MSG_INFOR;


//发送消息流水号存贮
typedef struct
{
	uint8_t lock_register;

}SEND_MSG_NUM;


extern SEND_MSG_NUM send_msg_num;


extern HEAD_MSG head_msg;
extern LOCK_REGISTER_INFOR lock_register;
extern uint8_t aunthen_code[AUTHEN_LENGTH];




extern void ProtocolInit(void);
extern void cmd_send_pack(uint16_t cmd, uint8_t *paramert);

#endif

