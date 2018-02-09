// Board/nrf6310/ble/ble_app_hrs/main.c
/** @example Board/nrf6310/ble/ble_app_hrs/main.c
 *
 * @brief Heart Rate Service Sample Application main file.
 *
 * This file contains the source code for a sample application using the Heart Rate service
 * (and also Battery and Device Information services). This application uses the
 * @ref srvlib_conn_params module.
 */
#include "ble_service.h"
#include "data_struct.h"
#include "global_var.h"
#include "ble_gattc.h"
//#include "mtk_uart.h"
#include "RD_MotoControl.h"
#include "RD_BatteryCharge.h"
#include "ble_exdevice.h"
#include "nrf_delay.h"
#include "ble_date_time.h"
#include <ctype.h>

#define   TX_POWER_LEVEL                        (0)                                        /**< TX Power Level value. This will be set both in the TX Power service, in the advertising data, and also used to set the radio transmit power. */

static    app_timer_id_t                        m_time_event_id;
static    uint16_t                              m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */
static    ble_gap_sec_params_t                  m_sec_params;                              /**< Security requirements for this application. */
static    ble_gap_adv_params_t                  m_adv_params;                              /**< Parameters to be passed to the stack when starting advertising. */
static    uint16_t                              m_adv_interval = 800;

Receive_Lock_Cmd lock_receive;
Receive_data_save_leaseid leaseid_data_save;
Receive_data_save_open open_data_save;


//uint8_t pReceive_buff[] = {0} ;
//uint8_t pReceive_buff11[] = {0} ;
unsigned char device_name[20] = {0};
unsigned char mac_address[16] = {0};	//20
unsigned char eas_code[18] = {0};
ble_gap_addr_t	 mac_addr;
bool	b_beef_flag = false ;
bool	b_timestamp_flag = false ;
bool 	n_trade_type_46		= false ;
static uint32_t m_clock_counter = 0;
static uint32_t m_clock_tick = 0;
extern void reset_bt(void);

pstorage_handle_t dest_block_id[3];

#if defined(CHANG_PROTOCAL)
uint8_t key_code[16] ={0};//{"1234567890123456"};
uint8_t scret_key[96] __attribute__ ((aligned (4)));	//80
unsigned char de_scret_key[16]={0};
uint8_t resp_result = RESP_NUORMAL;
#endif

#if defined(SECOND_30_OUT_NUOPEN_TO_LOCK)
OPEN_DELAY_CHECK open_delay_check;
#endif

//unsigned char buff_size = 0 ;
//unsigned char buff_size_second = 0 ;


//////////////////////////////

 Receive_Cmd receivecmd;
extern char LOGO[3];
extern char ID[6];
extern uint8_t g_bat_present;
extern uint8_t g_lock_flag;
extern uint8_t g_moto_flag;
extern uint8_t g_devicename_set;
extern uint32_t g_number ;
extern uint8_t g_receicve_cmd;
extern uint32_t						                  m_last_tickcount;
extern uint32_t						                  m_ltmin_tickcount;
extern uint32_t							              m_total_tickcount	;
extern ble_system_clock_t                    m_clock ;
//extern ble_system_clock_t                    m_correct_clock;
extern bool   m_timer_battery;
extern bool n_trade_type;
//extern bool n_trade_type_46;
//extern ble_system_clock_t  						open_clock;
//extern ble_system_clock_t 						close_clock;
///////////////////////////
#if defined(CHANG_PROTOCAL)
extern bool check_ase_data(uint8_t *compare_key_code);
extern bool secret_ase_data(void);
#endif

/**@brief Function for error handling, which is called when an error has occurred.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    // This call can be used for debug purposes during application development.
    // @note CAUTION: Activating this code will write the stack to flash on an error.
    //                This function should NOT be used in a final product.
    //                It is intended STRICTLY for development/debugging purposes.
    //                The flash write will happen EVEN if the radio is active, thus interrupting
    //                any communication.
    //                Use with care. Uncomment the line below to use.
    // ble_debug_assert_handler(error_code, line_num, p_file_name);

    // On assert, the system can only recover with a reset.
    NVIC_SystemReset();
}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


#if defined(CHANG_PROTOCAL)
void rand_data(uint8_t *save_data,uint8_t length)
{
	uint8_t i;
	for(i=0;i<length;i++)
	{
		save_data[i] =(rand()%('~'-'!'+1)+'!');
		if(save_data[i] < '0')
			save_data[i] = (rand()%('9'-'0'+1)+'0');
		else if((save_data[i] > '9')&&(save_data[i] < 'A'))
			save_data[i] = (rand()%('Z'-'A'+1)+'A');
		else if((save_data[i] > 'Z')&&(save_data[i] < 'a'))
			save_data[i] = (rand()%('z'-'a'+1)+'a');
		else if(save_data[i] > 'z')
			save_data[i] = (rand()%('9'-'0'+1)+'0');
		////printf("save_data--:%c\r\n",save_data[i]);
	}
}


#endif

char *strupr(char *str)
{
    char *orign=str;
    for(; (*str)!=('\0'); str++)
        *str = toupper(*str);
    return orign;
}


#if defined(SEND_80KEY_SECRET)
//#define ROR_LEN 		 sizeof(ror_char)
uint8_t ror_char[] = "mintbike";
//len Ϊ8�ı���
void key_code_encrypt(uint8_t *input,uint8_t *output,uint8_t len)
{
	uint8_t i,j;
	for(i=0;i<len;i++)
	{
		for(j=0;j<8;j++)
			output[i*10 + j] = (input[i*10 + j]^ror_char[j]);
	}
}
#endif


int checkout_receive_start_end_data(uint8_t* receive_buf,uint8_t size)
{
	//uint8_t receive_crc = 0x00,cal_crc = 0x00;
//	uint8_t i = 0,j=0;
	//if(size<20)		return false;
	if(receive_buf[0] == 0x67 && receive_buf[1] == 0x74)
	{
		lock_receive.len = receive_buf[2];
		if(lock_receive.len<=15)
		{
			return 1 ;
		}
		else
		{
			return 2;
		}
	}
	else if(receive_buf[0] != 0x67 && receive_buf[1] != 0x74)
	{
		if((receive_buf[0] == 0x00)
			&&(receive_buf[1] == 0x00)
			&&(receive_buf[2] == 0x00)
			&&(receive_buf[3] == 0x00)
			&&(receive_buf[4] == 0x00)
			&&(receive_buf[5] == 0x00)
			&&(receive_buf[6] == 0x00)
			&&(receive_buf[17] == 0x00)
			)
		{
			return 4;
		}
		else
		{
			return 3;
		}

	}
	else
	{
		return 5;
	}
}


static bool ble_checkout_receive_data(uint8_t* receive_buf,uint8_t size)
{
	uint8_t receive_crc = 0x00,cal_crc = 0x00;
	uint8_t i = 0;//j=0;
	//if(size<20)		return false;
#if MOTING_DEBUG

	//printf("receive_buf[0] = 0x%02x,receive_buf[1] = 0x%02x,size=%d\r\n",receive_buf[0],receive_buf[1],size);
#endif
	if(receive_buf[0] == 0x67 && receive_buf[1] == 0x74)
	{
		receive_crc = receive_buf[size-1];

		for(i = 3;i<(size-1);i++)
		{
			cal_crc ^= receive_buf[i];
		}
		if(cal_crc!=receive_crc)
		{
#if MOTING_DEBUG

			//printf("cal_crc = 0x%02x,receive_crc = 0x%02x\r\n",cal_crc,receive_crc);
#endif
			return false;
		}
		else
		{
#if MOTING_DEBUG

			//printf("cal_crc = 0x%02x,receive_crc = 0x%02x\r\n",cal_crc,receive_crc);
#endif
			return true;
		}
	}
	else
	{
		return false;
	}
}


#if 1

uint8_t ASCII_2_HEX_byte(uint8_t o_data)
{
	uint8_t tempData=0;
    if((o_data>=10)&&(o_data<=16))         //0~~~~~9
    {
      tempData = o_data+0x61-10;
    }
    else if(o_data<=9)   //a~~~~f
    {
      tempData = o_data+0x30;
    }
    else
    {
      return 0;         //illegal data
    }
		return tempData;
}

void send_data_to_phone(uint8_t cmd,uint8_t* p_buff)
{

	uint8_t crc = 0x00;//,i = 0;
	uint8_t send_buf[120] = {0};	//
	uint8_t nn_len = 0;
#if dbg
	//printf("send_data_to_phone m_blue_connect =%d,cmd=0x%02x \r\n",m_blue_connect,cmd);
#endif
	if(m_blue_connect)
	{
		send_buf[0] = 0x67;
		send_buf[1] = 0x74;

		switch(cmd)
		{
			case 0x41:
			{
				unsigned char resultcode = 0 ;
				unsigned char i = 0 ;
			//	uint32_t      err_code;
			//	uint32_t      err_code;
			//	ble_gap_addr_t 	mac_addr;

			#if defined(CHANG_PROTOCAL)
				rand_data(key_code,sizeof(key_code));
			#endif
				leaseid_data_save.vbatvalue = (unsigned int)bat_get_vol_value();
				leaseid_data_save.versioninfo=0x1267;	//0X1166
				leaseid_data_save.lockstatus=g_lock_flag;
			#if !defined(CHANG_PROTOCAL)
				send_buf[2] = 11;
			#endif
				send_buf[3] = cmd;

				send_buf[4] = resultcode;
				send_buf[5] = (leaseid_data_save.versioninfo>>8)&0xff;
				send_buf[6] = (leaseid_data_save.versioninfo)&0xff;
				send_buf[7] = leaseid_data_save.lockstatus;
			#if defined(CHANG_PROTOCAL)
				for(i=0;i<16;i++)
					send_buf[8+i] =key_code[i];

				send_buf[24] = (leaseid_data_save.vbatvalue)&0xff;
				send_buf[25] = (leaseid_data_save.vbatvalue>>8)&0xff;
				send_buf[26] = leaseid_data_save.smscount;
				crc = 0 ;
				for(i=3;i<27;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[27] = crc;
				nn_len = 28;
				send_buf[2] = nn_len-5;
			#else
				send_buf[8]  = 0x00;
				send_buf[9]  = 0x00;
				send_buf[10] = 0x00;
				send_buf[11] = 0x00;
				send_buf[12] = (leaseid_data_save.vbatvalue)&0xff;
				send_buf[13] = (leaseid_data_save.vbatvalue>>8)&0xff;
				send_buf[14] = leaseid_data_save.smscount;

				crc = 0 ;
				for(i=3;i<15;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[15] = crc;
				nn_len = 16;
			#endif
			}
			break ;
			case 0x42:
			{
				unsigned char resultcode = 0 ;
				unsigned char i = 0 ;
			#if defined(CHANG_PROTOCAL)
				resultcode = resp_result;
			#endif

				send_buf[2] = 1;
				send_buf[3] = cmd;

				send_buf[4] = resultcode;

				crc = 0 ;
				for(i=3;i<5;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[5] = crc;
				nn_len = 6;
			}
			break ;
			case 0x46:
			{
				unsigned char i = 0 ;
				bool nntradetype = 0;//n_trade_type_46 ;
				unsigned int nnvbatpers = 80 ;
				uint32_t      err_code;
				ble_gap_addr_t 	mac_addr;

				unsigned long int nntimestamp = open_data_save.timestamp + (m_clock_counter/10);

				//ble_date_time_encode(p_data_time,p_encoded_buffer);

				if(n_trade_type_46==true)
				{
					nntradetype=true;
				}
				else
				{
					nntradetype=false;
				}
#if MOTING_DEBUG
				{

					//printf("nntradetype = %d,open_data_save.timestamp=%d,m_clock_counter=%d\r\n",nntradetype,nntimestamp,m_clock_counter);
				}
#endif
				err_code = sd_ble_gap_address_get(&mac_addr);
				APP_ERROR_CHECK(err_code);



				send_buf[2] = 70;
				send_buf[3] = cmd;

				// user id
				send_buf[4] = open_data_save.userid[0];
				send_buf[5] = open_data_save.userid[1];
				send_buf[6] = open_data_save.userid[2];
				send_buf[7] = open_data_save.userid[3];
				send_buf[8] = open_data_save.userid[4];
				send_buf[9] = open_data_save.userid[5];
				//
				send_buf[10] = (nntimestamp)&0xff;
				send_buf[11] = (nntimestamp>>8)&0xff;
				send_buf[12] = (nntimestamp>>16)&0xff;
				send_buf[13] = (nntimestamp>>24)&0xff;
				send_buf[14] = ASCII_2_HEX_byte((mac_addr.addr[0]&0xf0)>>4);
				send_buf[15] = ASCII_2_HEX_byte((mac_addr.addr[0]&0x0f));
				send_buf[16] = ASCII_2_HEX_byte((mac_addr.addr[1]&0xf0)>>4);
				send_buf[17] = ASCII_2_HEX_byte(mac_addr.addr[1]&0x0f);
				send_buf[18] = ASCII_2_HEX_byte((mac_addr.addr[2]&0xf0)>>4);
				send_buf[19] = ASCII_2_HEX_byte(mac_addr.addr[2]&0x0f);
				send_buf[20] = ASCII_2_HEX_byte((mac_addr.addr[3]&0xf0)>>4);
				send_buf[21] = ASCII_2_HEX_byte(mac_addr.addr[3]&0x0f);
				send_buf[22] = ASCII_2_HEX_byte((mac_addr.addr[4]&0xf0)>>4);

				//
				send_buf[23] = (nntimestamp)&0xff;
				send_buf[24] = (nntimestamp>>8)&0xff;
				send_buf[25] = (nntimestamp>>16)&0xff;
				send_buf[26] = (nntimestamp>>24)&0xff;
				//
				send_buf[27] = nntradetype;
				//
				send_buf[28]=leaseid_data_save.longitude[0];
				send_buf[29]=leaseid_data_save.longitude[1];
				send_buf[30]=leaseid_data_save.longitude[2];
				send_buf[31]=leaseid_data_save.longitude[3];
				send_buf[32]=leaseid_data_save.longitude[4];
				send_buf[33]=leaseid_data_save.longitude[5];
				send_buf[34]=leaseid_data_save.longitude[6];
				send_buf[35]=leaseid_data_save.longitude[7];
				send_buf[36]=leaseid_data_save.longitude[8];
				send_buf[37]=leaseid_data_save.longitude[9];
				send_buf[38]=leaseid_data_save.longitude[10];
				send_buf[39]=leaseid_data_save.longitude[11];
				send_buf[40]=leaseid_data_save.latitude[0];
				send_buf[41]=leaseid_data_save.latitude[1];
				send_buf[42]=leaseid_data_save.latitude[2];
				send_buf[43]=leaseid_data_save.latitude[3];
				send_buf[44]=leaseid_data_save.latitude[4];
				send_buf[45]=leaseid_data_save.latitude[5];
				send_buf[46]=leaseid_data_save.latitude[6];
				send_buf[47]=leaseid_data_save.latitude[7];
				send_buf[48]=leaseid_data_save.latitude[8];
				send_buf[49]=leaseid_data_save.latitude[9];
				send_buf[50]=leaseid_data_save.latitude[10];
				send_buf[51]=leaseid_data_save.latitude[11];
				//
			#if defined(CHANG_PROTOCAL)
				secret_ase_data();
			#endif
				send_buf[52]=eas_code[0];
				send_buf[53]=eas_code[1];
				send_buf[54]=eas_code[2];
				send_buf[55]=eas_code[3];
				send_buf[56]=eas_code[4];
				send_buf[57]=eas_code[5];
				send_buf[58]=eas_code[6];
				send_buf[59]=eas_code[7];
				send_buf[60]=eas_code[8];
				send_buf[61]=eas_code[9];
				send_buf[62]=eas_code[10];
				send_buf[63]=eas_code[11];
				send_buf[64]=eas_code[12];
				send_buf[65]=eas_code[13];
				send_buf[66]=eas_code[14];
				send_buf[67]=eas_code[15];

				//
				send_buf[68]=open_data_save.keyindex;
				//
			#if 0
				send_buf[69]=(nnvbatpers>16)&0xff;
				send_buf[70]=(nnvbatpers>>8)&0xff;
				send_buf[71]=(nnvbatpers)&0xff;

				//
				send_buf[72]=(nnvbat>>8)&0xff;
				send_buf[73]=(nnvbat)&0xff;
			#else
				nnvbatpers = Bat_Return();
				send_buf[69]= 0x00;//(nnvbatpers>16)&0xff;
				send_buf[70]= 0x00; //(nnvbatpers>>8)&0xff;
				send_buf[71]=(nnvbatpers)&0xff;

				leaseid_data_save.vbatvalue = (unsigned int)bat_get_vol_value();
				send_buf[72]=(leaseid_data_save.vbatvalue)&0xff;
				send_buf[73]=(leaseid_data_save.vbatvalue>>8)&0xff;
			#endif

				crc = 0 ;
				for(i=3;i<74;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[74] = crc;
				nn_len = 75;

			}
			break ;
			case 0x16:
			{
//				unsigned char resultcode = 0 ;
				unsigned char i = 0 ;

				send_buf[2] = 0;
				send_buf[3] = 0x46;

				//send_buf[4] = 0;
				//send_buf[5] = 0;

				crc = 0 ;
				for(i=3;i<4;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[4] = crc;
				nn_len = 5;
			}
			break;
			case 0x4d:
			{
				unsigned char resultcode = 0 ;
				unsigned char i = 0 ;

				send_buf[2] = 1;
				send_buf[3] = cmd;

				send_buf[4] = resultcode;

				crc = 0 ;
				for(i=3;i<5;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[5] = crc;
				nn_len = 6;

			}
			break ;
			case 0x50:
			{
				bool resultcode = n_trade_type ;
				unsigned char i = 0 ;
				uint32_t tick = (m_clock_counter/10);

				send_buf[2] = 5;
				send_buf[3] = cmd;

				send_buf[4] = resultcode;
				if(n_trade_type)
				{
					send_buf[5] = (tick)&0xff;
					send_buf[6] = (tick>>8)&0xff;
					send_buf[7] = (tick>>16)&0xff;
					send_buf[8] = (tick>>24)&0xff;
				}
				else
				{
					send_buf[5] = 0x00;
					send_buf[6] = 0x00;
					send_buf[7] = 0x00;
					send_buf[8] = 0x00;
				}


				crc = 0 ;
				for(i=3;i<9;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[9] = crc;
				nn_len = 10;
			}
			break ;
		#if defined(ADD_CHECK_IN_KEY)
			case 0x60:
			{
			
			#if defined(FLASH_READ_WRITE)
				if(resp_result == RESP_NO_POWER)
				{
					unsigned char resultcode = 0 ;
					unsigned char i = 0 ;
				#if defined(CHANG_PROTOCAL)
					resultcode = resp_result;
					resp_result = 0;
				#endif

					send_buf[2] = 1;
					send_buf[3] = cmd;

					send_buf[4] = resultcode;

					crc = 0 ;
					for(i=3;i<5;i++)
					{
						crc ^= send_buf[i];
					}
					send_buf[5] = crc;
					nn_len = 6;
				}
				else
			#endif
				{
					unsigned char i = 0 ;
					send_buf[3] = cmd;
					nn_len =4;
					{
						memcpy(send_buf+nn_len,scret_key,80);
					}
					nn_len += 80;
					memcpy(send_buf+nn_len,mac_addr.addr,6);
					nn_len += 6;
					
					memcpy(send_buf+nn_len,scret_key+CHECK_IN_KEY_INDEX,8);
					nn_len += 8;
					send_buf[2] = nn_len-4;
					crc = 0 ;
					for(i=3;i<nn_len;i++)
					{
						crc ^= send_buf[i];
					}
					send_buf[nn_len] = crc;
					nn_len += 1;
				}
			}
			break;

			case 0x64:
			{

			}
			break;
			case 0x65:
			{
				unsigned char resultcode = 0 ;
				unsigned char i = 0 ;
				resultcode = resp_result;
				send_buf[2] = 1;
				send_buf[3] = cmd;

				send_buf[4] = resultcode;

				crc = 0 ;
				for(i=3;i<5;i++)
				{
				crc ^= send_buf[i];
				}
				send_buf[5] = crc;
				nn_len = 6;
			}
			break;
		#endif

			case 0x53:
			{
				//unsigned char resultcode = 0 ;
				unsigned char i = 0 ;
				unsigned char beefstatus = 0 ;
				unsigned char motorstatus = 0 ;
				unsigned int sunbord = 652 ;
				unsigned char transaction = 0 ;
				unsigned long int nntimestamp = m_clock_tick/10;

				send_buf[2] = 9;
				send_buf[3] = cmd;

				send_buf[4] = (nntimestamp)&0xff;
				send_buf[5] = (nntimestamp>>8)&0xff;
				send_buf[6] = (nntimestamp>>16)&0xff;
				send_buf[7] = (nntimestamp>>24)&0xff;

				send_buf[8] = (sunbord>>8)&0xff;
				send_buf[9] = (sunbord)&0xff;
				send_buf[10] = beefstatus;
				send_buf[11] = motorstatus;
				send_buf[12] = transaction;

				crc = 0 ;
				for(i=3;i<13;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[13] = crc;
				nn_len = 14;
			}
			break ;

		#if defined(TEST_USE_BLE_SEND_REPLACE_2503)
			
			case 0x5D:
			{
				unsigned char i = 0 ;
				send_buf[2] = 28;
				send_buf[3] = 0x5D;
				send_buf[4] = 0x3A;
				send_buf[5] = 0x21;
				send_buf[6] = 0x11;
				send_buf[7] = 0x14;
				send_buf[8] = 0xF2	;			
				send_buf[9] = 0x00;
				send_buf[10] = 0x00;
				send_buf[11] = 0x00;
				send_buf[12] = 0x00;
				send_buf[13] = 0x00;
				send_buf[14] = 0x24;
				send_buf[15] = 0x5;
				send_buf[16] = 0x00;
				send_buf[17] = 0x00;
				send_buf[18] = 0x50;
				send_buf[19] = 0x00;
				send_buf[20] = 0x00;
				send_buf[21] = 0x00;
				send_buf[22] = 0x00;
				send_buf[23] = 0x00;
				send_buf[24] = 0x00;
				send_buf[25] = 0x00;
				send_buf[26] = 0x00;
				send_buf[27] = 0x00;
				send_buf[28] = 0xB1;
				send_buf[29] = 0x01;
				send_buf[30] = 0x0D;
				send_buf[31] = 0x0A;
				crc = 0 ;
				for(i=3;i<32;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[32] = crc;
				nn_len = 33;
			}
			  	break;
			case 0x5E:
				{
				unsigned char i = 0 ;
				send_buf[2]= 28;	
				send_buf[3]= 0x5E;	
				send_buf[4]= 0x3A;	
				send_buf[5]= 0x21;	
				send_buf[6]= 0x12;	
				send_buf[7]= 0x14;	
				send_buf[8]=  0xA5; 
				send_buf[9]=  0x0B; 
				send_buf[10]= 0x3C; 
				send_buf[11]= 0x09; 
				send_buf[12]= 0x05; 
				send_buf[13]= 0x00; 
				send_buf[14]= 0x00; 
				send_buf[15]= 0x00; 
				send_buf[16]= 0x00; 
				send_buf[17]= 0x00; 
				send_buf[18]= 0x31; 
				send_buf[19]= 0x0C; 
				send_buf[20]= 0x4C; 
				send_buf[21]= 0x4C; 
				send_buf[22]= 0x8C; 
				send_buf[23]= 0x5C; 
				send_buf[24]= 0xDF; 
				send_buf[25]= 0xFF; 
				send_buf[26]= 0x00; 
				send_buf[27]= 0x00; 
				send_buf[28]= 0xDC; 
				send_buf[29]= 0x04; 
				send_buf[30]= 0x0D; 
				send_buf[31]= 0x0A; 
				for(i=3;i<32;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[32] = crc;
				nn_len = 33;
			}
				break;
			
			#endif

		#if !defined(CUT_CODE)
			case 0x5D:
			{
				unsigned char resultcode = 0 ;
				unsigned char i = 0 ;

				send_buf[2] = 1;
				send_buf[3] = cmd;

				send_buf[4] = resultcode;

				crc = 0 ;
				for(i=3;i<5;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[5] = crc;
				nn_len = 6;
			}
			break;

			
			case 0x54:
			{
				unsigned char resultcode = 0 ;
				unsigned char i = 0 ;

				send_buf[2] = 1;
				send_buf[3] = cmd;

				send_buf[4] = resultcode;

				crc = 0 ;
				for(i=3;i<5;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[5] = crc;
				nn_len = 6;
			}
			break ;
			case 0x55:
			{
				unsigned char resultcode = 0 ;
				unsigned char i = 0 ;




				send_buf[2] = 1;
				send_buf[3] = cmd;

				send_buf[4] = resultcode;

				crc = 0 ;
				for(i=3;i<5;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[5] = crc;
				nn_len = 6;

			}
			break ;
			case 0x56:
			{
				//unsigned char resultcode = 0 ;
				unsigned char i = 0 ;
				unsigned char ncurrent = 0 ;
				unsigned char nrmp = 0 ;
				unsigned char nerror = 0 ;
				unsigned char nnstatus = 0 ;
				unsigned int vbatvalue = 0 ;
				unsigned int vspeed = 0 ;
				unsigned int ndistance = 0 ;




				send_buf[2] = 7;
				send_buf[3] = cmd;

				send_buf[4] = (vbatvalue)&0xff;
				send_buf[5] = (vbatvalue>>8)&0xff;

				send_buf[6] = ncurrent;

				send_buf[7] = (vspeed)&0xff;
				send_buf[8] = (vspeed>>8)&0xff;

				send_buf[9] = nrmp;

				send_buf[10] = nerror;

				send_buf[11] = nnstatus;

				send_buf[12] = (ndistance)&0xff;
				send_buf[13] = (ndistance>>8)&0xff;

				crc = 0 ;
				for(i=3;i<14;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[14] = crc;
				nn_len = 15;
			}
			break ;
			case 0x57:
			{
				unsigned char i = 0 ;
				unsigned int vbat_temp=0;
				unsigned int vbat_value=0;
				unsigned int char_count=0;
				unsigned int char_long_time=0;
				unsigned int char_cur_time=0;
				unsigned int char_full_value=0;
				unsigned int he_dian_tai=0;
				unsigned int bms_get_vbat_value=0;
				unsigned int bms_get_vbat_cur=0;




				send_buf[2] = 18;
				send_buf[3] = cmd;

				send_buf[4] = (vbat_temp)&0xff;
				send_buf[5] = (vbat_temp>>8)&0xff;

				send_buf[6] = (vbat_value)&0xff;
				send_buf[7] = (vbat_value>>8)&0xff;

				send_buf[8] = (char_count)&0xff;
				send_buf[9] = (char_count>>8)&0xff;

				send_buf[10] = (char_long_time)&0xff;
				send_buf[11] = (char_long_time>>8)&0xff;

				send_buf[12] = (char_cur_time)&0xff;
				send_buf[13] = (char_cur_time>>8)&0xff;

				send_buf[14] = (char_full_value)&0xff;
				send_buf[15] = (char_full_value>>8)&0xff;

				send_buf[16] = (he_dian_tai)&0xff;
				send_buf[17] = (he_dian_tai>>8)&0xff;

				send_buf[18] = (bms_get_vbat_value)&0xff;
				send_buf[19] = (bms_get_vbat_value>>8)&0xff;

				send_buf[20] = (bms_get_vbat_cur)&0xff;
				send_buf[21] = (bms_get_vbat_cur>>8)&0xff;

				crc = 0 ;
				for(i=3;i<22;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[22] = crc;
				nn_len = 23;

			}
			break ;
			case 0x58:
			{
				unsigned char i = 0,j=0 ;
				unsigned int control_array[32]={0};
				unsigned int bms_array[32]={0};





				send_buf[2] = 64;
				send_buf[3] = cmd;

				for(i=0;i<32;i++,j++)
				{
					send_buf[4+i]=control_array[i];
				}
				for(i=0;i<32;i++)
				{
					send_buf[4+i+j]=bms_array[i];
				}

				crc = 0 ;
				for(i=3;i<64;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[68] = crc;
				nn_len = 69;

			}
			break ;
			case 0x59:
			{
				unsigned char resultcode = 0 ;
				unsigned char i = 0 ;




				send_buf[2] = 1;
				send_buf[3] = cmd;

				send_buf[4] = resultcode;

				crc = 0 ;
				for(i=3;i<5;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[5] = crc;
				nn_len = 6;
			}
			break ;
			case 0x5a:
			{
				unsigned char resultcode = 0 ;
				unsigned char i = 0 ;




				send_buf[2] = 1;
				send_buf[3] = cmd;

				send_buf[4] = resultcode;

				crc = 0 ;
				for(i=3;i<5;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[5] = crc;
				nn_len = 6;
			}
			break ;
			case 0x5b:
			{
				unsigned char resultcode = 0 ;
				unsigned char i = 0 ;




				send_buf[2] = 1;
				send_buf[3] = cmd;

				send_buf[4] = resultcode;

				crc = 0 ;
				for(i=3;i<5;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[5] = crc;
				nn_len = 6;
			}
			break ;

			case 0xfe:
			case 0xff:
			{
//				unsigned char resultcode = 0 ;
				unsigned char i = 0 ;




				send_buf[2] = 1;
				send_buf[3] = cmd;

				send_buf[4] = RESULT_CODE_SAM_LOW;

				crc = 0 ;
				for(i=3;i<5;i++)
				{
					crc ^= send_buf[i];
				}
				send_buf[5] = crc;
				nn_len = 6;
			}
 			break ;
		#endif
			default:
				break;
		}

	#if MOTING_DEBUG
		{
			unsigned char i = 0;
			for(i=0;i<nn_len;i++)
			{
				//printf("[%d]=%x\r\n",i,send_buf[i]);
			}
			//printf("\r\n");
		}
	#endif


		//ble_data_update(&m_ble,send_buf, sizeof(send_buf));
		{

			if(nn_len<20)
			{
				ble_data_update(&m_ble,send_buf, nn_len);
			}
			else
			{
				unsigned char send_array_n = nn_len/20 ;
				unsigned char send_array_m = nn_len%20 ;
				unsigned char i=0,j=0 ;
				uint8_t send_buf_back[20] = {0};

				for(i=0;i<send_array_n;i++)
				{
					for(j=0;j<20;j++)
					{
						send_buf_back[j]=send_buf[i*20+j];
					}
					ble_data_update(&m_ble,send_buf_back, 20);

#if MOTING_DEBUG
					{
						unsigned char k = 0;
						for(k=0;k<20;k++)
						{
							unsigned char uuu = i*20+k;
							//printf("send_buf[%d]=0x%02x\r\n",uuu,send_buf_back[k]);
						}
						//printf("\r\n");
					}
#endif
					memset(send_buf_back,0,sizeof(send_buf_back));
				}
				for(j=0;j<send_array_m;j++)
				{
					send_buf_back[j]=send_buf[send_array_n*20+j];
				}
				ble_data_update(&m_ble,send_buf_back, send_array_m);
#if MOTING_DEBUG
				{
					unsigned char k = 0;
					for(k=0;k<send_array_m;k++)
					{
						unsigned char uuu = send_array_n*20+k;
						//printf("send_buf[%d]=0x%02x\r\n",uuu,send_buf_back[k]);
					}
					//printf("\r\n");
					//printf("send_array_n=%d,send_array_m=%d\r\n",send_array_n,send_array_m);
				}
#endif
				memset(send_buf_back,0,sizeof(send_buf_back));
			}
		}
  }
#if MOTING_DEBUG
	{
		unsigned char i = 0;
		//printf("return data:\r\n");
		for(i=0;i<nn_len;i++)
			//printf("0x%02x ",send_buf[i]);
		//printf("\r\n");
	}
#endif


}

#endif



#if 1










//unsigned int device_num = 10000531;

//unsigned int device_num = 23178950;
/*

unsigned int device_num = 12345678;
void SetBle_Name(void)
{
	memset(device_name,0,sizeof(device_name));
	sprintf((char *)device_name,"UGO%d",device_num);
}

unsigned int encrypt(int plainText, int key)
{
	return ((plainText+12131679)^1024);
}

unsigned int decrypt(int cipherText, int key)
{
	return ((cipherText^key)-12131679);
//	return ((cipherText^key)-20170101);
}
*/
/*int Shangxun_Moto_return(uint8_t cmd,uint8_t status,uint8_t reset,unsigned int cipher)
{

	//send_data_to_phone(cmd,status,reset,cipher);
	return 0;
}
*/

int send_ble_data_to_app_return(uint8_t cmd,uint8_t* p_buff)
{
	send_data_to_phone(cmd,p_buff);
	return 0;
}


int LOCK_Status(Receive_Cmd *rec)
{
	return 0;
}


void mt2503_lock_noticfy(void)
{
	//nrf_gpio_pin_set(MT2503_GOTO_LOCK_PIN);
	//nrf_delay_ms(200);
	//nrf_gpio_pin_clear(MT2503_GOTO_LOCK_PIN);
}
void mt2503_unlock_noticfy(void)
{
	//nrf_gpio_pin_set(MT2503_GOTO_UNLOCK_PIN);
//	nrf_delay_ms(200);
//	nrf_gpio_pin_clear(MT2503_GOTO_UNLOCK_PIN);
//	gpio_timer_start();


}
void Moto_ENABLE(void)
{
	nrf_gpio_pin_set(MOTO_EN);
}
void Moto_DISABLE(void)
{
	nrf_gpio_pin_clear(MOTO_EN);
	nrf_gpio_pin_clear(IA_EN);
	nrf_gpio_pin_clear(IB_EN);
}
void Moto_A2B(void)
{
	nrf_gpio_pin_set(IA_EN);
	nrf_gpio_pin_clear(IB_EN);
}
void Moto_B2A(void)
{
	nrf_gpio_pin_set(IB_EN);
	nrf_gpio_pin_clear(IA_EN);
}

#if !defined(CUT_CODE)
static int Shangxun_Moto_Handle(uint8_t cmd,uint8_t reset,unsigned int cipher)
{
	if(cmd == 0x01&&!g_moto_flag&&g_lock_flag==false)
	{
		g_receicve_cmd = 1;
		Moto_ENABLE();
		Moto_A2B();
		g_moto_flag = 1;
		nrf_gpio_pin_clear(BUZZER_EN);
		moto_timer_start();
//		buzzer_timer_stop();
//		buzzer_timer_start();

	}
	return 0;
}
#endif


static int moting_Moto_Handle(uint8_t cmd,uint8_t reset,unsigned int cipher)
{
	if(cmd == 0x01)
	{
		g_receicve_cmd = 1;
		Moto_ENABLE();
		Moto_A2B();
		g_moto_flag = 1;
		nrf_gpio_pin_clear(BUZZER_EN);
		moto_timer_start();
//		buzzer_timer_stop();
//		buzzer_timer_start();
	#if defined(SECOND_30_OUT_NUOPEN_TO_LOCK)
		open_delay_check.start = true;
		open_delay_check.cnt = 0;
	#endif

	}
	return 0;
}


#if 0
static void Shangxun_Cmd_return(uint8_t cmd,uint8_t reset,unsigned int cipherText)
{

	switch(cmd)
	{
		case  0x00:
			break;
		case 0x01:
			Shangxun_Moto_Handle(cmd,reset,cipherText);
			break;
		case 0x02:
//			Shangxun_Moto_Handle(cmd,reset,cipherText);
			break;
		default:break;
	}




}

static void Shangxun_Cmd_handle(uint8_t cmd,uint8_t reset ,unsigned int cipherText)
{
	switch(cmd)
	{

		case 0x01:
			Shangxun_Cmd_return(cmd,reset,cipherText);
			break;
		default:
			Shangxun_Cmd_return(0x00,reset,cipherText);
			break;
	}
}

#endif

///////////////////////////////

void MT2503_Reset(void)
{
	//nrf_gpio_pin_set(MT2503_RESET_PIN);
	//nrf_delay_ms(1000);
	//nrf_gpio_pin_clear(MT2503_RESET_PIN);
}



/*
static void SetRTC_Timer(ble_system_clock_t *clock)
{
	if(!clock)	return ;
	if( clock->year != m_clock.year||
		clock->month!=m_clock.month||
		clock->day!=m_clock.day||
		clock->hour!=m_clock.hour||
		clock->minute!=m_clock.minute)
	{
//			m_clock.year = clock->year;
//			m_clock.month = clock->month;
//			m_clock.day = clock->day;
//			m_clock.hour = clock->hour;
//			m_clock.minute = clock->minute;
//			m_clock.second = clock->second;
//			m_clock.tnum = m_clock.second;
			memcpy(&m_clock,clock,sizeof(ble_system_clock_t));
//			´Óµ±Ç°¼ÆÊýÖµ¿ªÊ¼¼ÆÊý
			memcpy(&m_correct_clock,&m_clock,sizeof(ble_system_clock_t));
			m_correct_clock.tnum = m_correct_clock.second;
			m_ltmin_tickcount = NRF_RTC0->COUNTER;
			m_last_tickcount = m_ltmin_tickcount;
			m_total_tickcount = 0;
	}
	else
	{
		if(clock->second>=m_clock.second)
		{
			if(clock->second-m_clock.second>=45)
			{
//				m_clock.year = clock->year;
//				m_clock.month = clock->month;
//				m_clock.day = clock->day;
//				m_clock.hour = clock->hour;
//				m_clock.minute = clock->minute;
//				m_clock.second = clock->second;
				memcpy(&m_clock,clock,sizeof(ble_system_clock_t));

				memcpy(&m_correct_clock,&m_clock,sizeof(ble_system_clock_t));
				m_correct_clock.tnum = m_correct_clock.second;
				m_ltmin_tickcount = NRF_RTC0->COUNTER;
				m_last_tickcount = m_ltmin_tickcount;
				m_total_tickcount = 0;

			}
		}
	}

//	send_data_to_phone(0x01,g_bat_present,0x00,receivecmd.cipher);
	//send_data_to_phone(0x07,0x00,0x00,receivecmd.cipher);
}
*/

#if defined(CHANG_PROTOCAL)
bool check_ase_data(uint8_t *compare_key_code)
{
	uint8_t i =0 ;
	AES_ECB_decrypt((const uint8_t*)open_data_save.Aescode , (const uint8_t*)(&(scret_key[open_data_save.keyindex])), de_scret_key, 16);
	for(i=0;i<16;i++)
	{
	//	//printf("%d--Aescode:%x\r\n",i,open_data_save.Aescode[i]);
	//	//printf("%d--scret_key:%c\r\n",i,scret_key[open_data_save.keyindex+i]);
	//	//printf("%d--de_scret_key:%x--key_code:%x\r\n",i,de_scret_key[i],key_code[i]);
		if(compare_key_code[i] != de_scret_key[i])
		{
			return false;
		}
	}
	return true;
}

bool secret_ase_data(void)
{
	AES_ECB_encrypt((const uint8_t*)key_code, (const uint8_t*)(scret_key+open_data_save.keyindex), eas_code, 16);
	return true;
}
#endif

#if defined(CHANGE_UART_DEAL_PLACE)

void RecvDataDeal(void)
{
	unsigned char nn_cmd;

	if(lock_receive.recv_len == 0)
		return;
	if(true==ble_checkout_receive_data(lock_receive.recbuff,lock_receive.recv_len))
	{
		switch(lock_receive.cmd)
		{
			case 0x81:
			{
				if(lock_receive.recbuff[2]>0)
				{
					leaseid_data_save.longitude[0] = lock_receive.recbuff[4];
					leaseid_data_save.longitude[1] = lock_receive.recbuff[5];
					leaseid_data_save.longitude[2] = lock_receive.recbuff[6];
					leaseid_data_save.longitude[3] = lock_receive.recbuff[7];
					leaseid_data_save.longitude[4] = lock_receive.recbuff[8];
					leaseid_data_save.longitude[5] = lock_receive.recbuff[9];
					leaseid_data_save.longitude[6] = lock_receive.recbuff[10];
					leaseid_data_save.longitude[7] = lock_receive.recbuff[11];
					leaseid_data_save.longitude[8] = lock_receive.recbuff[12];
					leaseid_data_save.longitude[9] = lock_receive.recbuff[13];
					leaseid_data_save.latitude[0] = lock_receive.recbuff[14];
					leaseid_data_save.latitude[1] = lock_receive.recbuff[15];
					leaseid_data_save.latitude[2] = lock_receive.recbuff[16];
					leaseid_data_save.latitude[3] = lock_receive.recbuff[17];
					leaseid_data_save.latitude[4] = lock_receive.recbuff[18];
					leaseid_data_save.latitude[5] = lock_receive.recbuff[19];
					leaseid_data_save.latitude[6] = lock_receive.recbuff[20];
					leaseid_data_save.latitude[7] = lock_receive.recbuff[21];
					leaseid_data_save.latitude[8] = lock_receive.recbuff[22];
				}
				{
					nn_cmd = 0x41;
				}
			}
			break;
			case 0x82:
			{
				open_data_save.keyindex = lock_receive.recbuff[4]-128;
				open_data_save.userid[0] = lock_receive.recbuff[5];
				open_data_save.userid[1] = lock_receive.recbuff[6];
				open_data_save.userid[2] = lock_receive.recbuff[7];
				open_data_save.userid[3] = lock_receive.recbuff[8];
				open_data_save.userid[4] = lock_receive.recbuff[9];
				open_data_save.userid[5] = lock_receive.recbuff[10];
				open_data_save.userid[6] = lock_receive.recbuff[11];

				open_data_save.timestamp = ((uint32_t)lock_receive.recbuff[12])&0xff;
				open_data_save.timestamp |= (((uint32_t)lock_receive.recbuff[13])<<8)&0xff00;
				open_data_save.timestamp |= (((uint32_t)lock_receive.recbuff[14])<<16)&0xff0000;
				open_data_save.timestamp |= (((uint32_t)lock_receive.recbuff[15])<<24)&0xff000000;

				{
					uint8_t i;
					for(i=16;i<32;i++)
					{
						open_data_save.Aescode[i-16] = lock_receive.recbuff[i];
					}
				}


		#if defined(CHANG_PROTOCAL)
				if(check_ase_data(key_code))
		#endif
				{
					nrf_gpio_pin_toggle(BUZZER_EN);
					moting_Moto_Handle(0x01,0x00,0x00);
					b_timestamp_flag=true;
					n_trade_type_46=false;
			#if defined(CHANG_PROTOCAL)
					resp_result = RESP_NUORMAL;
			#endif
				}
		#if defined(CHANG_PROTOCAL)
				else
				{
					resp_result = RESP_ASE_ERROR;
				}

		#endif
				{
					nn_cmd = 0x42;
				}
			}

			break;
			case 0x86:
			{
				if(lock_receive.recbuff[2]==0x00)
				{
					nn_cmd = 0x46;
				}
				else
				{
					nn_cmd = 0x16;
				}
			}
			break;
			case 0x89:
			{
				if(lock_receive.recbuff[4]==0xf0)
				{
					reset_bt();
				}
			}
			break;
			case 0x8D:
			{
				if(lock_receive.recbuff[8]==0xf2)
				{
					nn_cmd = 0x5d;
					b_beef_flag=true ;
					nrf_gpio_pin_toggle(BUZZER_EN);
				}
			}
			break;
			case 0x90:
			{
				nn_cmd = 0x50;
			}
			break;
			case 0x93:
			{
				nn_cmd = 0x53;
			}
			break;

			
	#if defined(ADD_HELP_FUNCV_ORDER)
			case 0x94:
			case 0x9C:
			case 0x9D:
			case 0x9E:
			case 0x9F:
			{
			#if !defined(TEST_USE_BLE_SEND_REPLACE_2503)
				ble_send_data_to_mt2503_uart(0x01,lock_receive.recbuff);
			#else
				nn_cmd = lock_receive.cmd - 0x40;
			#endif
			}
			break;
	#endif
			case 0xA0:
			{
		#if defined(FLASH_READ_WRITE)
				if(sys_flag_data[0]== 0xAA)
					resp_result = RESP_NO_POWER;
		#endif
				nn_cmd = 0x60;
			}
			break;
	#if defined(ADD_CHECK_IN_KEY)
			case 0xA1:
			case 0xA2:
			case 0xA3:

			break;
			case 0xA4:
			nn_cmd = 0x64;
			break;
			case 0xA5:
			{
				unsigned char mac_address_temp[16] = {0};
				open_data_save.keyindex = lock_receive.recbuff[4]-128;
				{
					uint8_t i;
					for(i=5;i<21;i++)
					{
						open_data_save.Aescode[i-5] = lock_receive.recbuff[i];
					}
				}
				memcpy(mac_address_temp,mac_address,16);
				strupr((char *)mac_address_temp);
				if(check_ase_data(mac_address_temp))
				{
					resp_result = 1;		//�ɹ�
			#if defined(FLASH_READ_WRITE)
					sys_flag_data[0] = 0xAA;
			#endif
				}
				else
				{
					resp_result = 0;		//ʧ��
				}
				
				nn_cmd = 0x65;
		 }
			break;
	#endif
		
			default:
			{
				nn_cmd = 0xfe;
			}
			break;
		}

	}
		else
		{
			nn_cmd = 0xff;
		}
		send_ble_data_to_app_return(nn_cmd,lock_receive.recbuff);
		memset(&lock_receive,0,sizeof(lock_receive));

}
#endif

void receive_data_handle(ble_t * p_trans, ble_s_evt_t * p_evt,uint8_t* p_buff,uint8_t len)
{
	static unsigned char buff_size_back = 0 ;
	static uint8_t psend_buff[150] = {0} ;
	uint8_t psend_buff_second[20] = {0} ;
#if !defined(CHANGE_UART_DEAL_PLACE)
	unsigned char nn_cmd;
#endif

	if(1==checkout_receive_start_end_data(p_buff, len))
	{
		memcpy(psend_buff,p_buff,len);
		//buff_size = len;
		buff_size_back = len ;
  	}
	else if(2==checkout_receive_start_end_data(p_buff, len))
	{
		memcpy(psend_buff,p_buff,len);
		//buff_size = len;
		buff_size_back = len ;
		return ;
  	}
	else if(3==checkout_receive_start_end_data(p_buff, len))
	{
		memcpy(psend_buff_second,p_buff,len);
		//buff_size = len;
		//buff_size_back = len ;
		memcpy(psend_buff+buff_size_back,p_buff,len);
		buff_size_back =buff_size_back+len;
  	}
	else if(4==checkout_receive_start_end_data(p_buff, len))
	{
		return ;
	}
	else
	{
		memcpy(psend_buff,p_buff,len);
		buff_size_back = len ;
		return ;
	}

#if defined(CHANGE_UART_DEAL_PLACE)
	memset(&lock_receive,0,sizeof(lock_receive));
	memcpy(lock_receive.recbuff,psend_buff,buff_size_back);
	lock_receive.len = psend_buff[2];
	lock_receive.cmd = psend_buff[3];
	lock_receive.recv_len = buff_size_back;
	buff_size_back = 0 ;
	memset(psend_buff,0,sizeof(psend_buff));
#else
	if(true==ble_checkout_receive_data(psend_buff,buff_size_back))
	{
	
		memset(&lock_receive,0,sizeof(lock_receive));
		lock_receive.len = psend_buff[2];
		lock_receive.cmd = psend_buff[3];
#if MOTING_DEBUG
	uint8_t i;
		//printf("receive cmd =0x%02x ,lock_receive.len=%02d\r\n",lock_receive.cmd,lock_receive.len);
		{
			for(i=0;i<lock_receive.len;i++)
			{
				//printf("%i--%x ",i,psend_buff[4+i]);
			}
		}
		//printf("\r\n");
#endif
		switch(lock_receive.cmd)
		{
			case 0x81:
			{
				if(psend_buff[2]>0)
				{
					leaseid_data_save.longitude[0] = psend_buff[4];
					leaseid_data_save.longitude[1] = psend_buff[5];
					leaseid_data_save.longitude[2] = psend_buff[6];
					leaseid_data_save.longitude[3] = psend_buff[7];
					leaseid_data_save.longitude[4] = psend_buff[8];
					leaseid_data_save.longitude[5] = psend_buff[9];
					leaseid_data_save.longitude[6] = psend_buff[10];
					leaseid_data_save.longitude[7] = psend_buff[11];
					leaseid_data_save.longitude[8] = psend_buff[12];
					leaseid_data_save.longitude[9] = psend_buff[13];
					leaseid_data_save.latitude[0] = psend_buff[14];
					leaseid_data_save.latitude[1] = psend_buff[15];
					leaseid_data_save.latitude[2] = psend_buff[16];
					leaseid_data_save.latitude[3] = psend_buff[17];
					leaseid_data_save.latitude[4] = psend_buff[18];
					leaseid_data_save.latitude[5] = psend_buff[19];
					leaseid_data_save.latitude[6] = psend_buff[20];
					leaseid_data_save.latitude[7] = psend_buff[21];
					leaseid_data_save.latitude[8] = psend_buff[22];
				}
				{
					nn_cmd = 0x41;
				}
			}
			break;
			case 0x82:
			{
				open_data_save.keyindex = psend_buff[4]-128;
				open_data_save.userid[0] = psend_buff[5];
				open_data_save.userid[1] = psend_buff[6];
				open_data_save.userid[2] = psend_buff[7];
				open_data_save.userid[3] = psend_buff[8];
				open_data_save.userid[4] = psend_buff[9];
				open_data_save.userid[5] = psend_buff[10];
				open_data_save.userid[6] = psend_buff[11];

				open_data_save.timestamp = ((uint32_t)psend_buff[12])&0xff;
				open_data_save.timestamp |= (((uint32_t)psend_buff[13])<<8)&0xff00;
				open_data_save.timestamp |= (((uint32_t)psend_buff[14])<<16)&0xff0000;
				open_data_save.timestamp |= (((uint32_t)psend_buff[15])<<24)&0xff000000;

				{
					uint8_t i;
					for(i=16;i<32;i++)
					{
						open_data_save.Aescode[i-16] = psend_buff[i];
					}
				}


			#if defined(CHANG_PROTOCAL)
				if(check_ase_data(key_code))
			#endif
				{
					nrf_gpio_pin_toggle(BUZZER_EN);
					moting_Moto_Handle(0x01,0x00,0x00);
					b_timestamp_flag=true;
					n_trade_type_46=false;
				#if defined(CHANG_PROTOCAL)
					resp_result = RESP_NUORMAL;
				#endif
				}
			#if defined(CHANG_PROTOCAL)
				else
				{
					resp_result = RESP_ASE_ERROR;
				}

			#endif
				{
					nn_cmd = 0x42;
				}
			}

			break;
			case 0x86:
			{
				if(psend_buff[2]==0x00)
				{
					nn_cmd = 0x46;
				}
				else
				{
					nn_cmd = 0x16;
				}
			}
			break;
			case 0x89:
			{
				if(psend_buff[4]==0xf0)
				{
					reset_bt();
				}
			}
			break;
			case 0x8D:
			{
				if(psend_buff[8]==0xf2)
				{
					nn_cmd = 0x5d;
					b_beef_flag=true ;
					nrf_gpio_pin_toggle(BUZZER_EN);
				}
			}
			break;
			case 0x90:
			{
				nn_cmd = 0x50;
			}
			break;
			case 0x93:
			{
				nn_cmd = 0x53;
			}
			break;

			
		#if defined(ADD_HELP_FUNCV_ORDER)
			case 0x94:
			case 0x9C:
			case 0x9D:
			case 0x9E:
			case 0x9F:
			{
				ble_send_data_to_mt2503_uart(0x01,psend_buff);
			}
			break;
		#endif
			case 0xA0:
			{
			#if defined(FLASH_READ_WRITE)
				if(sys_flag_data[0]== 0xAA)
					resp_result = RESP_NO_POWER;
			#endif
				nn_cmd = 0x60;
			}
			break;
		#if defined(ADD_CHECK_IN_KEY)
			case 0xA1:
			case 0xA2:
			case 0xA3:

			break;
			case 0xA4:
			nn_cmd = 0x64;
			break;
			case 0xA5:
			{
				unsigned char mac_address_temp[16] = {0};
				open_data_save.keyindex = psend_buff[4]-128;
				{
					uint8_t i;
					for(i=5;i<21;i++)
					{
						open_data_save.Aescode[i-5] = psend_buff[i];
					}
				}
				memcpy(mac_address_temp,mac_address,16);
				strupr((char *)mac_address_temp);
				if(check_ase_data(mac_address_temp))
				{
					resp_result = 1;		//�ɹ�
				#if defined(FLASH_READ_WRITE)
					sys_flag_data[0] = 0xAA;
				#endif
				}
				else
				{
					resp_result = 0;		//ʧ��
				}
				
				nn_cmd = 0x65;
		 }
			break;
		#endif
		
			default:
			{
				nn_cmd = 0xfe;
			}
			break;
		}

	}
	else
	{
		nn_cmd = 0xff;
	}
	send_ble_data_to_app_return(nn_cmd,psend_buff);
	buff_size_back = 0 ;
	memset(psend_buff,0,sizeof(psend_buff));
#endif
}

#endif
/*
static void local_clock_maintain(void)
{
	uint32_t nCurTick = NRF_RTC0->COUNTER;
	m_total_tickcount += (nCurTick - m_last_tickcount);
	if(m_last_tickcount >= nCurTick)
	{
		m_total_tickcount += ((uint32_t)0xFFFFFF + nCurTick - m_last_tickcount);
	}
	else
	{
		m_total_tickcount += (nCurTick - m_last_tickcount);
	}
	m_last_tickcount = nCurTick;
//	m_clock.second = (nCurTick - m_ltmin_tickcount + 10) / 0x8000 + m_correct_clock.tnum;
//add by lxj 0613
	if(m_ltmin_tickcount>=nCurTick)
		m_clock.second = (0xFFFFFF+nCurTick - m_ltmin_tickcount + 10) / 0x8000 + m_correct_clock.tnum;
	else
		m_clock.second = (nCurTick - m_ltmin_tickcount + 10) / 0x8000 + m_correct_clock.tnum;
//add end
	if(m_clock.second == 60)
	{
		m_correct_clock.tnum = 0;
		m_ltmin_tickcount = nCurTick;

		m_clock.second = m_clock.second % 60;
		if(++m_clock.minute == 60)
		{
			m_clock.minute = 0;
			if(++m_clock.hour == 24)
			{
				m_total_tickcount = 0;
				m_correct_clock.hour = 0;
				m_correct_clock.minute = 0;
				m_correct_clock.second = 0;

				m_clock.hour = 0;
				switch(m_clock.month)
				{
					case 1 :
					case 3 :
					case 5 :
					case 7 :
					case 8 :
					case 10 :
					case 12 :
					{
						if(++m_clock.day == 31)
						{
							m_clock.day = 0;
							if(++m_clock.month == 12)
							{
								m_clock.month = 0;
								if(++m_clock.year == 100)
									m_clock.year = 0;
							}
						}
					}
						break;
					case 2 :
					{
						if(m_clock.year / 4 == 0)
						{
							if(++m_clock.day == 29)
							{
								m_clock.day = 0;
								if(++m_clock.month == 12)
								{
									m_clock.month = 0;
									if(++m_clock.year == 100)
										m_clock.year = 0;
								}
							}
						}
						else
						{
							if(++m_clock.day == 28)
							{
								m_clock.day = 0;
								if(++m_clock.month == 12)
								{
									m_clock.month = 0;
									if(++m_clock.year == 100)
										m_clock.year = 0;
								}
							}
						}
					}
						break;
					case 4 :
					case 6 :
					case 9 :
					case 11 :
					{
						if(++m_clock.day == 30)
						{
							m_clock.day = 0;
							if(++m_clock.month == 12)
							{
								m_clock.month = 0;
								if(++m_clock.year == 100)
									m_clock.year = 0;
							}
						}
					}
						break;
				}
			}
		}
	}
}
*/


/**
 *本地时钟tick函数
 */
#if defined(SECOND_30_OUT_NUOPEN_TO_LOCK)
extern void close_lock(void);
#endif
static void detection_buf_timeout_hander(void * p_context)
{
	UNUSED_PARAMETER(p_context);

	//charge_check_handle();
	//factory_test();

#if defined(CHANGE_UART_DEAL_PLACE)
	{
		static unsigned char ms_tick = 0;
		ms_tick++;
		if(ms_tick>9)
			ms_tick = 0;
		else
			return;
	}
#endif
	
	m_clock_tick++;
	if(n_trade_type)
		m_clock_counter++;

#if defined(CONTROL_UART)
	if(close_delay_time)
	{
		close_delay_time--;
		if(!close_delay_time)
		{
			close_uart();
		}
	}
#endif

#if defined(SECOND_30_OUT_NUOPEN_TO_LOCK)
	if(open_delay_check.start)
	{
		open_delay_check.cnt++;
		if(open_delay_check.cnt>300)
		{
			open_delay_check.start = false;
			open_delay_check.cnt = 0;
			close_lock();
		}
	}
#endif


#if defined(CHANG_PROTOCAL)
	srand(m_clock_counter);
#endif
	//local_clock_maintain();
//	if(m_clock_counter >= 288000) 			//8H
//	if(m_clock_counter >= 180000)			//5H

//	if(m_clock_counter%300 == 0) 			//30s for test
	{
		m_timer_battery = true;
	}
//	if(m_clock_counter >= 288000) 		m_clock_counter = 1;			//8h
//	if(m_clock_counter >= 180000)		m_clock_counter = 1;
	if(b_timestamp_flag==true)
	{
		b_timestamp_flag=false ;
		m_clock_counter=0;
	}
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
void timers_init(void)
{
	// Initialize timer module.
	APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);

	uint32_t err_code = app_timer_create(&m_time_event_id, APP_TIMER_MODE_REPEATED, detection_buf_timeout_hander);
	APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
void gap_params_init(void)
{
	uint32_t                err_code;
	ble_gap_conn_params_t   gap_conn_params;
	ble_gap_conn_sec_mode_t sec_mode;

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
#if 0
	err_code = sd_ble_gap_device_name_set(&sec_mode,
																				(const uint8_t *)DEVICE_NAME,
																				strlen(DEVICE_NAME));

#else
	err_code = sd_ble_gap_device_name_set(&sec_mode,
																				(const uint8_t *)device_name,
																				strlen((char *)device_name));
#endif

	APP_ERROR_CHECK(err_code);

	err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HEART_RATE_SENSOR_HEART_RATE_BELT);
	APP_ERROR_CHECK(err_code);

	memset(&gap_conn_params, 0, sizeof(gap_conn_params));

	gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
	gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
	gap_conn_params.slave_latency     = SLAVE_LATENCY;
	gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

	err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
	APP_ERROR_CHECK(err_code);

	err_code = sd_ble_gap_tx_power_set(TX_POWER_LEVEL);
  APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */

void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
	  ble_gap_addr_t 	mac_addr;
	  ble_advdata_manuf_data_t  manuf_data;

//    ble_uuid_t adv_uuids[] =
//    {
//				{BLE_UUID_TROLLEY_CASE_SERVICE,BLE_UUID_TYPE_BLE}
//    };

		err_code = sd_ble_gap_address_get(&mac_addr);
	  APP_ERROR_CHECK(err_code);
	  manuf_data.data.size = 0;
	  manuf_data.data.p_data = mac_addr.addr;

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags.size              = sizeof(flags);
    advdata.flags.p_data            = &flags;
		advdata.p_manuf_specific_data   = &manuf_data;
//    advdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
//    advdata.uuids_complete.p_uuids  = adv_uuids;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    m_adv_params.p_peer_addr = NULL;                           // Undirected advertisement.
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = m_adv_interval;
    m_adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;
}


static void dis_init(void)
{
	uint32_t       err_code;
	ble_dis_init_t dis_init;

	// Initialize Device Information Service.
	memset(&dis_init, 0, sizeof(dis_init));

	ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)MANUFACTURER_NAME);

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

	err_code = ble_dis_init(&dis_init);
	APP_ERROR_CHECK(err_code);
}

//
static void trans_init(void)
{
	uint32_t       err_code;
	ble_init_t trans_init;
	memset(&trans_init, 0, sizeof(trans_init));

	// Here the sec level for the Battery Service can be changed/increased.
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&trans_init.data_char_attr_md.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&trans_init.data_char_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&trans_init.data_char_attr_md.write_perm);

//	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&trans_init.trans_report_attr_md.read_perm);
//	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&trans_init.trans_report_attr_md.write_perm);

	trans_init.evt_handler          = receive_data_handle;
	trans_init.support_notification = true;

	err_code = ble_init(&m_ble, &trans_init);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
void services_init(void)
{
	 dis_init();
	 trans_init();
}

/**@brief Function for initializing security parameters.
 */
void sec_params_init(void)
{
    m_sec_params.timeout      = SEC_PARAM_TIMEOUT;
    m_sec_params.bond         = SEC_PARAM_BOND;
    m_sec_params.mitm         = SEC_PARAM_MITM;
    m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    m_sec_params.oob          = SEC_PARAM_OOB;
    m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}

/**@brief Function for starting application timers.
 */
void application_timers_start(void)
{
	uint32_t err_code;
	// Start application timers
	err_code = app_timer_start(m_time_event_id, DETECTION_BUF_INTERVAL, NULL);

	APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting application timers.
 */
void application_timers_stop(void)
{
    uint32_t err_code;

    // Stop key timers
    err_code = app_timer_stop(m_time_event_id);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
void advertising_start(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_start(&m_adv_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for stopping advertising.
 */
void advertising_stop(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_stop();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in]   p_evt   Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = m_ble.send_data_handles.cccd_handle;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;
	  //int32_t   rssi;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
				    m_blue_connect = true;
#if MOTING_DEBUG
			//printf("on_ble_evt  connect\r\n");
#endif
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
						err_code = ble_bondmngr_bonded_centrals_store();
						APP_ERROR_CHECK(err_code);
            m_blue_connect = false;

            advertising_start();
            break;
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
						err_code = sd_ble_gap_sec_params_reply(m_conn_handle,
																									 BLE_GAP_SEC_STATUS_SUCCESS,
																									 &m_sec_params);
						APP_ERROR_CHECK(err_code);
            break;
        case BLE_GAP_EVT_TIMEOUT:
            if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISEMENT)
            {
              advertising_start();
            }
            break;
        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
void ble_evt_dispatch(ble_evt_t * p_ble_evt)//蓝牙事件安排派出
{
    ble_bondmngr_on_ble_evt(p_ble_evt);
	  ble_on_ble_evt(&m_ble,p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
void ble_stack_init(void)
{
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
	//SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_2000MS_CALIBRATION, false);
	  SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, false);
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);

//		err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
//	APP_ERROR_CHECK(err_code);

}

/**@brief Function for handling a Bond Manager error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void bond_manager_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for the Bond Manager initialization.
 */
void bond_manager_init(void)
{
    uint32_t            err_code;
    ble_bondmngr_init_t bond_init_data;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    // Initialize the Bond Manager.
    bond_init_data.evt_handler             = NULL;
    bond_init_data.error_handler           = bond_manager_error_handler;
    bond_init_data.bonds_delete            = false;

    err_code = ble_bondmngr_init(&bond_init_data);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Power manager.
 */
 void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}
extern void Storage_Module_init(void);
//蓝牙初始�?



#if defined(FLASH_READ_WRITE)
uint8_t sys_flag_data[SYS_FLAG_DATA_LEN]={0};
uint8_t sys_flag_data_temp[SYS_FLAG_DATA_LEN]={0};
extern pstorage_handle_t handle;


void falsh_read(void)
{
	pstorage_block_identifier_get(&handle,1,&(dest_block_id[0]));
	pstorage_load((uint8_t *)sys_flag_data_temp, &(dest_block_id[0]), 8, 0);
	memcpy(sys_flag_data,sys_flag_data_temp,sizeof(sys_flag_data_temp));
	nrf_delay_ms(500);
	
}

void falsh_write(void)
{
	pstorage_block_identifier_get(&handle,1,&(dest_block_id[0]));
	pstorage_update(&(dest_block_id[0]),(uint8_t *)sys_flag_data_temp, sizeof(sys_flag_data_temp), 0);
	nrf_delay_ms(500);
}

void update_flash_data(void)
{
	if(memcmp(sys_flag_data_temp,sys_flag_data,sizeof(sys_flag_data)))
	{
		memcpy(sys_flag_data_temp,sys_flag_data,sizeof(sys_flag_data));
		Storage_Module_init();
		falsh_write();
	#if defined(DEBUG_UART)
		{
			uint8_t i;
			printf("\r\n ");
			for(i=0;i<sizeof(sys_flag_data_temp);i++)
				printf("%d ",sys_flag_data_temp[i]);
		}
		printf("\r\n ");
	#endif
	}
}

#endif




#if defined(CHANG_PROTOCAL)

void BleInit(pstorage_handle_t *handle)
{
	uint8_t addr_end = 0 ;
	ble_stack_init();
	Storage_Module_init();

	pstorage_block_identifier_get(handle,1,&(dest_block_id[0]));
	pstorage_load((uint8_t *)sys_flag_data_temp, &(dest_block_id[0]), 8, 0);
	memcpy(sys_flag_data,sys_flag_data_temp,sizeof(sys_flag_data_temp));
	nrf_delay_ms(1000);

	pstorage_block_identifier_get(handle,2,&(dest_block_id[1]));
	pstorage_load((uint8_t *)scret_key, &(dest_block_id[1]), 64, 0);
	nrf_delay_ms(1000);
	
	pstorage_block_identifier_get(handle,3,&(dest_block_id[2]));
	pstorage_load((uint8_t *)(scret_key+64), &(dest_block_id[2]), 18, 0);	//16
	nrf_delay_ms(1000);
	
	sd_ble_gap_address_get(&mac_addr);
	addr_end = (mac_addr.addr[4]>>4)&0x0f;
	sprintf((char *)device_name,"bike:%02x%02x%02x%02x%xK358",mac_addr.addr[0],mac_addr.addr[1],mac_addr.addr[2],mac_addr.addr[3],addr_end);
#if defined(DEBUG_UART)
	printf("mac:%02x%02x%02x%02x%02x%02x--%c%c%c\r\n",mac_addr.addr[0],mac_addr.addr[1],mac_addr.addr[2],
	mac_addr.addr[3],mac_addr.addr[4],mac_addr.addr[5],device_name[4],device_name[5],device_name[6]);
#endif
	sprintf((char *)mac_address,"%02x%02x%02x%02x%02x%02x",mac_addr.addr[0],mac_addr.addr[1],\
		mac_addr.addr[2],mac_addr.addr[3],mac_addr.addr[4],mac_addr.addr[5]);
//	printf("mac_address:%s\r\n",mac_address);
	memcpy(mac_address+12,"0000",4);
#if 1

	if((scret_key[80] != 0x55)&&(scret_key[81] != 0xaa))
	{
	

	#if defined(CHANG_PROTOCAL)
		uint16_t seed = 0;
		seed =((mac_addr.addr[0]<<8|mac_addr.addr[1])+(mac_addr.addr[2]<<8|mac_addr.addr[3])
			+(mac_addr.addr[4]<<8|mac_addr.addr[5]));

		srand(seed);
		rand_data(scret_key,68);
		memcpy(scret_key+68,mac_address,12);
	#endif
		scret_key[80] = 0x55;
		scret_key[81] = 0xaa;
	#if defined(DEBUG_UART)
		uint8_t i=0;
		printf("scret_key:\r\n");
		for(i=0;i<80;i++)
		{
			printf("%c",scret_key[i]);
		}
		printf("\r\n");
	#endif


		pstorage_block_identifier_get(handle, 2, &(dest_block_id[1]));
		pstorage_update(&(dest_block_id[1]), (uint8_t *)(scret_key), 64, 0);
		nrf_delay_ms(500);

		pstorage_block_identifier_get(handle, 3, &(dest_block_id[2]));
		pstorage_update(&(dest_block_id[2]), (uint8_t *)(scret_key+64), 20, 0);	//16
		nrf_delay_ms(500);
	}
#endif

	//////
	bond_manager_init();
	timers_init();
	gap_params_init();
	advertising_init();
	services_init();
	conn_params_init();
	sec_params_init();

	// Start execution.
	application_timers_start();
	advertising_start();
}

#else

void BleInit(pstorage_handle_t *handle)
{
	int number;
//	uint8_t persent = 80;
	pstorage_handle_t dest_block_id;

	/*
	ble_stack_init();

#if 1
	Storage_Module_init();

	pstorage_block_identifier_get(handle,1,&dest_block_id);
	pstorage_load((uint8_t *)&number, &dest_block_id, 4, 0);
#endif
//	number = 7475789;		//KK789
	number = 9072200;		//HI123

	if(number>0)
	{
#if B4_UART
	//printf("number  = %d\r\n",number);
#endif
		g_devicename_set = 1;
		g_number = number;
		nrf_delay_ms(500);
	#if 0
		//printf("number = %d\r\n",number);
	#endif
#if defined( UG0)
		sprintf((char *)device_name,"UGO%d",number);
#else
		sprintf((char *)device_name,"%s%c%c%03d%02d000000000",
		LOGO,
		(g_number/100000),((g_number/1000)%100),g_number%1000,persent);
		memcpy(ID,&device_name[2],5);
		ID[5] = '\0';
#if B4_UART
		//printf("LOGO:%s,ID:%s\r\n",LOGO,ID);
		//printf("SSID:%s\r\n",device_name);
#endif


		//strcpy((char *)device_name,"ABHI12380224023317");
#endif
	}
	else
	{
#if dbg
		device_num = 23333333;
#else
		device_num = 12345678;
#endif
#if defined( UG0)
		sprintf((char *)device_name,"UGO%d",device_num);
#else
		sprintf((char *)device_name,"HYTD");
#endif
	}
*/
	ble_gap_addr_t   mac_addr;
	uint32_t err_code;
	uint8_t addr_end = 0 ;
	// Initialize.
	ble_stack_init();
	err_code = sd_ble_gap_address_get(&mac_addr);
	APP_ERROR_CHECK(err_code);
	addr_end = (mac_addr.addr[4]>>4)&0x0f;
	sprintf((char *)device_name,"bike:%02x%02x%02x%02x%xK358",mac_addr.addr[0],mac_addr.addr[1],mac_addr.addr[2],mac_addr.addr[3],addr_end);
	//sprintf(device_name,"bike:%02x%02x%02x%02xK322",mac_addr.addr[0],mac_addr.addr[1],mac_addr.addr[2],mac_addr.addr[3]);
	//sprintf(device_name,"bike:111213141K358");
	sprintf((char *)mac_address,"%02x%02x%02x%02x%02x%02x",mac_addr.addr[0],mac_addr.addr[1],mac_addr.addr[2],mac_addr.addr[3],mac_addr.addr[4],mac_addr.addr[5]);
#if defined(CHANG_PROTOCAL)
	uint8_t i=0;
	srand(m_clock_tick);
	rand_data(scret_key,68);
//	for(i=0;i<68;i++)
//	{
//		scret_key[i] = rand()%('~'-'!'+1)+'!';
//	}
	memcpy(scret_key+68,mac_address,12);
#endif


	Storage_Module_init();
	pstorage_block_identifier_get(handle,1,&dest_block_id);
	pstorage_load((uint8_t *)&number, &dest_block_id, 4, 0);
	//////
	bond_manager_init();
	timers_init();
	gap_params_init();
	advertising_init();
	services_init();
	conn_params_init();
	sec_params_init();

	// Start execution.
	application_timers_start();
	advertising_start();
#if !defined(CUT_CODE)
	ble_init_data_parameter();
#endif
}
#endif


void ble_get_mac_addr_paramer(void)
{
	ble_gap_addr_t   mac_addr;
	uint32_t err_code;
	// Initialize.
	err_code = sd_ble_gap_address_get(&mac_addr);
	APP_ERROR_CHECK(err_code);
	//sprintf(mac_address,"%02x%02x%02x%02x%02x%02x",mac_addr.addr[0],mac_addr.addr[1],mac_addr.addr[2],mac_addr.addr[3],mac_addr.addr[4],mac_addr.addr[5]);

}
void ble_get_mac_addr_paramer_string(void)
{
	ble_gap_addr_t   mac_addr;
	uint32_t err_code;
	// Initialize.
	err_code = sd_ble_gap_address_get(&mac_addr);
	APP_ERROR_CHECK(err_code);
	//sprintf(mac_address,"%02x%02x%02x%02x%02x%02x",mac_addr.addr[0],mac_addr.addr[1],mac_addr.addr[2],mac_addr.addr[3],mac_addr.addr[4],mac_addr.addr[5]);

}


#if !defined(CUT_CODE)
void ble_init_data_parameter(void)
{
	//memset(pReceive_buff,0,sizeof(pReceive_buff));
	//memset(pReceive_buff11,0,sizeof(pReceive_buff11));
	//memset(&leaseid_data_save,0,sizeof(leaseid_data_save));
	//memset(&lock_receive,0,sizeof(lock_receive));
	//memset(&open_data_save,0,sizeof(open_data_save));
	{
		//ble_get_mac_addr_paramer();
		memset(&eas_code,0,sizeof(eas_code));
		eas_code[0]=0x30;
		eas_code[1]=0x31;
		eas_code[2]=0x32;
		eas_code[3]=0x33;
		eas_code[4]=0x34;
		eas_code[5]=0x35;
		eas_code[6]=0x36;
		eas_code[7]=0x37;
		eas_code[8]=0x38;
		eas_code[9]=0x39;
		/*eas_code[10]=mac_address[0];
		eas_code[11]=mac_address[1];
		eas_code[12]=mac_address[2];
		eas_code[13]=mac_address[3];
		eas_code[14]=mac_address[4];
		eas_code[15]=mac_address[5];*/
	}
}
#endif
extern void simple_uart_putstring(const uint8_t *str);

void ble_send_data_to_mt2503_uart(unsigned char nn_cmd,uint8_t *send_data)
{
	unsigned char s_send_buff[150]={0};
	unsigned char cmd=nn_cmd;
	unsigned char len=0;
#if defined(CONTROL_UART)
	open_uart();
#endif

	switch(cmd)
	{
	#if defined(ADD_HELP_FUNCV_ORDER)
		case 1:
		{
			len = sprintf((char *)s_send_buff,"start,cmd:%d,control:",cmd);
			memcpy(s_send_buff+len,send_data,send_data[2]+5);
			len += send_data[2]+5;
			len += sprintf((char *)s_send_buff+len,",end");
			port_data_send(s_send_buff,len);
		}
		break ;
	#endif
	#if defined(CHANG_PROTOCAL)
		case 2:
		{
			uint8_t send_key_data[80];
			uint8_t check_in_key_data[8];
			memcpy(send_key_data,scret_key,80);
			memcpy(check_in_key_data,scret_key+CHECK_IN_KEY_INDEX,8);	
			len=sprintf((char *)s_send_buff+len,"start,cmd:%d,mac:%.*s,##:%.*s,**:%.*s,end",
				cmd,12,mac_address,sizeof(send_key_data),send_key_data,sizeof(check_in_key_data),check_in_key_data);		
			simple_uart_putstring((uint8_t *)s_send_buff);
		}
		break ;
	#else
		case 2:
		{
			ble_gap_addr_t   mac_addr;
			uint32_t err_code;
			// Initialize.
			err_code = sd_ble_gap_address_get(&mac_addr);
			APP_ERROR_CHECK(err_code);

			len=sprintf((char *)s_send_buff+len,"start,cmd:%d,mac:%s,end",cmd,mac_address);
			simple_uart_putstring((uint8_t *)s_send_buff);
		}
		break ;
	#endif
		default:
			break ;
	}

}

