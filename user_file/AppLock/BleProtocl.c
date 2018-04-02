#include"BleProtocl.h"
#if defined(ADD_BT_OPEN_LOCK)

extern uint32_t g_number;
extern bool    m_blue_connect       ;      
extern ble_t    m_ble;
extern uint32_t encrypt(uint32_t plainText, uint32_t key);
extern uint32_t  decrypt(uint32_t cipherText, uint32_t key);
extern uint8_t battery_level_transform(void);

extern char ID[6];
extern char LOGO[3];
BLE_RECV ble_recv;



void ble_open_lock(void)
{
	device_name_info.ctrl_mode = APP_OPEN;
	open_key_ctrl();
}

void app_open_send_data_to_phone(uint8_t cmd)
{

	uint8_t crc = 0x00,i = 0;
	uint8_t send_buf[20] = {0xAA};
 	if(m_blue_connect)
	{
		send_buf[0] = 0xAA;
		
		send_buf[1] = 0x1F;
		send_buf[2] = 0x41;
		
		send_buf[3] = 0x01;

		//cmd
		send_buf[4] = cmd;

		// RESET
		send_buf[5] = 0x00;
		

		send_buf[6] = 0x00;
		send_buf[7] = 0x00;

 		send_buf[8]  = (ble_recv.cipher>>24)&0xff;
		send_buf[9] =(ble_recv.cipher>>16)&0xff;
		send_buf[10] = (ble_recv.cipher>>8)&0xff;
		send_buf[11] = ble_recv.cipher&0xff;

	#if 1
		send_buf[12] = 0x00;
		send_buf[13] = 0x00;
		send_buf[14] = 0x00;
		send_buf[15] = 0x00;
		send_buf[16] = 0x00;
		send_buf[17] = 0x00;
	#else
 		if(cmd!=0x07)
		{
			send_buf[12] = 0x00;
			send_buf[13] = 0x00;
			send_buf[14] = 0x00;
			send_buf[15] = 0x00;
			send_buf[16] = 0x00;
			send_buf[17] = 0x00;
		}
		else
		{
			send_buf[12] = m_clock.year&0xff;
			send_buf[13] = m_clock.month&0xff;
			send_buf[14] = m_clock.day&0xff;
			send_buf[15] = m_clock.hour&0xff;
			send_buf[16] = m_clock.minute&0xff;
			send_buf[17] = m_clock.second&0xff; 	
		}
	#endif
		
		for(i = 1;i<18;i++)
			crc ^=	send_buf[i];
		
		send_buf[18] = crc;
		
		send_buf[19] = 0x55;
		ble_data_update(&m_ble,send_buf, 20);
  }
#if 0//dbg 
	printf("return data:\r\n");
	for(i=0;i<20;i++)
		printf("0x%02x ",send_buf[i]);
	printf("\r\n");

#endif	
		
}

static bool checkout_receive_start_end_data(uint8_t* receive_buf,uint8_t size)
{
	uint8_t receive_crc = 0x00,cal_crc = 0x00;
	uint8_t i = 0;
	if(size<20)		return false;
	if(receive_buf[0] == 0xAA && receive_buf[19] == 0x55) 
	{
		receive_crc = receive_buf[18];
		
		for(i = 1;i<18;i++)
		{
			cal_crc ^= receive_buf[i];
		}
		if(cal_crc!=receive_crc)
		{
		#if dbg
			printf("cal_crc = 0x%02x,receive_crc = 0x%02x\r\n",cal_crc,receive_crc);
		#endif
			return false;
		}
			
		else
		return true;
	}
	else
		return false;
}



void BleProtoclDeal(uint8_t* receive_buf,uint8_t size)
{
	unsigned int cipher = 0x00;
	unsigned char *p_buff = receive_buf;
  if(checkout_receive_start_end_data(p_buff, size))
	{
	#if dbg
		printf("receive data ok\r\n");
	#endif
		memset(&ble_recv,0,sizeof(ble_recv));
		ble_recv.cmd = p_buff[1];
		ble_recv.reset = p_buff[2];
		ble_recv.cipher |=  (p_buff[5]<<24);
		ble_recv.cipher |=  (p_buff[6]<<16);
		ble_recv.cipher |=  (p_buff[7]<<8);
		ble_recv.cipher |=  (p_buff[8]<<0);
		cipher = decrypt(ble_recv.cipher,1024);
		if(cipher == g_number)
		{
			if(ble_recv.reset == 0x01)
			{
			#if 0
				//Shangxun_Moto_return(cmd,0x00,receivecmd.reset,receivecmd.cipher);
			#endif
			}
			else 
			if(ble_recv.cmd == 0x07)
			{
			#if 0
				s_clock.year = p_buff[9] ;
				s_clock.month = p_buff[10] ;
				s_clock.day = p_buff[11] ;
				s_clock.hour = p_buff[12] ;
				s_clock.minute = p_buff[13] ;
				s_clock.second = p_buff[14] ;
				SetRTC_Timer(&s_clock);
			#endif
			}
			else
			{
				ble_open_lock();
			}
			return ;
		}
		else
		{
			app_open_send_data_to_phone(0x05);	//Ð£ÑéÂëÒì³£
			return ;
		}
	}
	else
	{
		//send_data_to_phone(CHECK_FAIL,last_card_ride_info);
	}

}


extern unsigned char device_name[20];

void BleOpenUpdateDeviceName(void)
{
	uint8_t len = 0;
	uint8_t status = 0;
	len += sprintf((char *)device_name,"%s",LOGO);
	len += sprintf((char *)device_name+len,"%s",ID);
	device_name_info.vol = battery_level_transform();
	if(device_name_info.vol>=100)
		device_name_info.vol = 99;
	len += sprintf((char *)device_name+len,"%02d",device_name_info.vol);
	len += sprintf((char *)device_name+len,"%02d%02d",0,0);
	len += sprintf((char *)device_name+len,"%02d%02d",0,0);
	status |= device_name_info.lock_state<<0;
	status |= 1<<1;						 
	status &= ~(1<<2);					 
	status &= 0x07;
	len += sprintf((char *)device_name+len,"%01d",status);
#if dbg
	printf("device_name = %s\r\n",device_name);	
#endif
	gap_params_init();
	advertising_init();
	
}

#endif














