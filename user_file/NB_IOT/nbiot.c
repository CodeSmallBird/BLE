#include"nbiot.h"



/*
APN
--中国电信 CTWAP,CTNET
--中国移动 CMNET
--中国联通 3GWAP

*/
uint8_t *nb_apn = "\"CMNET\"";//"\"CTWAP\"";	
uint8_t *apn_addr ="\"47.95.199.128\",55552,0";
uint8_t *socket_id_port ="1,5000";
NB_INFOR nb_infor;
SEND_NET_INFOR send_net_infor;


//ms 定时器
void time_tick_run(void)
{
	if(	GET_DEALY_TIME)
		GET_DEALY_TIME--;

}


void nbiot_pin_init(void)
{
	NB_RESET_PIN_INIT;
	NB_RESET_PIN_LOW;
	
	NB_POWER_PIN_INIT;
	NB_POWER_PIN_HIGH;

	
	NB_SLEEP_PIN_INIT;
	NB_SLEEP_PIN_HIGH;
	
	NB_SLEEP_STATE_PIN_INIT;
	
}

void nbiot_param_init(void)
{
	nbiot_pin_init();
	memset(&nb_infor,0,sizeof(nb_infor));
}

bool nbiot_power_on(void)
{
	bool status = false;
	nb_debug("nbiot_power_on---%d--%d\r\n",nb_infor.power_state,GET_NB_SLEEP_STATE_PIN);
	switch(nb_infor.power_state)
	{
		case NB_POWER_ON_HIGH:
			
			NB_POWER_PIN_LOW;
			SET_DEALY_TIME(1000);
			nb_infor.power_state++;
			break;
		case NB_POWER_ON_LOW:
			
			NB_POWER_PIN_HIGH;
			NB_SLEEP_PIN_LOW;
			nb_infor.power_state++;
			SET_DEALY_TIME(5000);
			break;
		case NB_POWER_ON_WAITE:
			//if(GET_NB_SLEEP_STATE_PIN == NB_STATE_LOW)
			{	
				status = true;
			}
			nb_infor.power_state = NB_POWER_ON_HIGH;
			SET_DEALY_TIME(1000);
			break;			
	}
	return status;
}


bool card_register_send(void)
{
	uint8_t len = 0;
	uint8_t nb_send_buff[100] = {0};
	len = sprintf((char*)nb_send_buff,"AT");
	
	if(GET_REGISTER_FLAG)
	{
		SET_REGISTER_FLAG(0);
		SET_DEALY_TIME(300);
		return false;
	}
	switch(GET_REGISTER_STATE)
	{
		case ATV_X_SET:
		    len += sprintf((char*)((char*)(nb_send_buff+len)),"V0");
			SET_DEALY_TIME(200);
			break;
		case ATE_X_SET:
			len += sprintf((char*)((char*)(nb_send_buff+len)),"E0");
			SET_DEALY_TIME(200);
			break;
		case CPIN_CHECK:
			len += sprintf((char*)((char*)(nb_send_buff+len)),"+CPIN?");
			SET_DEALY_TIME(200);
			break;
		
		case SET_CEREG:
			len += sprintf((char*)((char*)(nb_send_buff+len)),"+CEREG=2");
			SET_DEALY_TIME(200);
			break;
		
		case SET_MODODR:
			len += sprintf((char*)((char*)(nb_send_buff+len)),"+MODODR=5");
			SET_DEALY_TIME(200);
			break;

		case CHECK_MODODR:
			len += sprintf((char*)((char*)(nb_send_buff+len)),"+MODODR?");
			SET_DEALY_TIME(200);
			break;

		case CHECK_CEREG:
			len += sprintf((char*)((char*)(nb_send_buff+len)),"+CEREG?");
			SET_DEALY_TIME(200);
			break;
		case START_GPS:	//启动GPS
			len += sprintf((char*)((char*)(nb_send_buff+len)),"+GPSSTART");
			SET_DEALY_TIME(200);
			break;
		case AT_GET_GPS: //查询获得AT
			len += sprintf((char*)((char*)(nb_send_buff+len)),"+NMEAS=0");
			SET_DEALY_TIME(200);
			break;
		case REGISTER_OK:	// 注册成功
			SET_REGISTER_STATE(ATV_X_SET);
			nb_debug("REGISTER_OK");
			return true;
	}
	//nb_uart.send_buff[len] = 0x0d;	//
	nb_send_buff[len] = 0x0d;
	len += 1;
	//调用串口发送
	//nb_debug("card_register_send:%d\r\n",len);
	port_data_send(nb_send_buff,len);
	SET_REGISTER_FLAG(1);
	return false;
}

void card_register_recv_data_deal(uint8_t *deal_data)
{
	uint8_t* p_data;
	p_data = deal_data;
	switch(GET_REGISTER_STATE)
	{
		case ATV_X_SET:
			if(
			#if 0
				(strstr((char*)p_data,"V0") != NULL)
				&&
			#endif
				(strstr((char*)p_data,"0\r") != NULL))
			{
				SET_REGISTER_STATE(CPIN_CHECK);
			}
			break;
		case ATE_X_SET:
			if(
				#if 0
					(strstr((char*)p_data,"E0") != NULL)
					&&
				#endif
				(strstr((char*)p_data,"0\r") != NULL))
			{
				SET_REGISTER_STATE(CPIN_CHECK);
			}
			break;
		case CPIN_CHECK:
			if((strstr((char*)p_data,"+CPIN:") != NULL)
				&&(strstr((char*)p_data,"READY") != NULL))
			{
				SET_REGISTER_STATE(SET_CEREG);
			}
			break;
		
		case SET_CEREG:
			if(strstr((char*)p_data,"0\r") != NULL)
			{
				SET_REGISTER_STATE(SET_MODODR);
			}
			break;
		
		case SET_MODODR:
			if(strstr((char*)p_data,"0\r") != NULL)
			{
				SET_REGISTER_STATE(CHECK_MODODR);
			}
			break;

		case CHECK_MODODR:
			if((strstr((char*)p_data,"+MODODR:") != NULL)
				&&(strstr((char*)p_data,"5\r") != NULL))
			{
				SET_REGISTER_STATE(CHECK_CEREG);
			}
			else
			{
				SET_REGISTER_STATE(SET_MODODR);
			}
			break;

		case CHECK_CEREG:
			if((strstr((char*)p_data,"+CEREG:") != NULL)
				&&(strstr((char*)p_data,",1") != NULL))
			{
				SET_REGISTER_STATE(START_GPS);	// 
			}
			break;
		case START_GPS:
			if(strstr((char*)p_data,"2") != NULL)
			{
				SET_REGISTER_STATE(AT_GET_GPS);	// 
			}
			break;
		case AT_GET_GPS:	//关闭主动上报GPS
			if(strstr((char*)p_data,"0\r") != NULL)
			{
				SET_REGISTER_STATE(REGISTER_OK);	// 注册OK
			}
			break;
		default:
			break;
			
	}
	SET_REGISTER_FLAG(0);
}


bool nb_register_apn(void)
{
	return card_register_send();
}


bool nb_setup_net(void)
{
	uint8_t len = 0;	
	uint8_t nb_send_buff[100] = {0};
	len = sprintf((char*)nb_send_buff,"AT");
	
	if(GET_NET_REGISTER_FLAG)
		return false;
	switch(GET_NET_REGISTER_STATE)
	{
		case PRE_CHECK_LSIPCALL:
		    len += sprintf((char*)(nb_send_buff+len),"+LSIPCALL?");
			SET_DEALY_TIME(200);
			break;
		case CMMNET_SET:
		    len += sprintf((char*)(nb_send_buff+len),"+LSIPPROFILE=1,%s",nb_apn);
			SET_DEALY_TIME(500);
			break;
			
		case CHECK_LSIPCALL:
		    len += sprintf((char*)(nb_send_buff+len),"+LSIPCALL?");
			SET_DEALY_TIME(200);
			break;
			
		case LSIPCALL_SET:
			len += sprintf((char*)(nb_send_buff+len),"+LSIPCALL=1");
			SET_DEALY_TIME(1500);
			break;
		
		case SET_NET_OK:
			SET_NET_REGISTER_STATE(CMMNET_SET);
			return true;

	}
	nb_send_buff[len] = 0x0d;	//
	len += 1;
	//调用串口发送
	port_data_send(nb_send_buff,len);
	SET_NET_REGISTER_FLAG(1);
	return false ;
}


void nb_recv_net_order_deal(uint8_t *deal_data)
{
	uint8_t* p_data;
	p_data = deal_data;
	switch(GET_NET_REGISTER_STATE)
	{
		case PRE_CHECK_LSIPCALL:
			if((strstr((char*)p_data,"+LSIPCALL:1,") != NULL)
				&&(strstr((char*)p_data,"0\r") != NULL))
			{
				SET_NET_REGISTER_STATE(SET_NET_OK);
			}
			else
				SET_NET_REGISTER_STATE(CMMNET_SET);
			break;
		case CMMNET_SET:
			if(strstr((char*)p_data,"0\r") != NULL)
			{
				SET_NET_REGISTER_STATE(CHECK_LSIPCALL);
			}
			break;
		case CHECK_LSIPCALL:
			if((strstr((char*)p_data,"+LSIPCALL:1,") != NULL)
				&&(strstr((char*)p_data,"0\r") != NULL))
			{
				SET_NET_REGISTER_STATE(SET_NET_OK);
			}
			else
				SET_NET_REGISTER_STATE(LSIPCALL_SET);
			break;

		case LSIPCALL_SET:
			if((strstr((char*)p_data,"+LSIPCALL:1,") != NULL)
				&&(strstr((char*)p_data,"0\r") != NULL))
			{
				SET_NET_REGISTER_STATE(SET_NET_OK);
			}
			else
				SET_NET_REGISTER_STATE(CHECK_LSIPCALL);
			break;
		
		default:
			break;
			
	}
	SET_NET_REGISTER_FLAG(0);
}


bool nb_register_net_mode(void)
{
	return nb_setup_net();
}


bool nb_data_send_fun(void)
{
	uint8_t len = 0;
	uint8_t nb_send_buff[200] = {0};
	
	if(GET_NB_SEND_FLAG)
		return false;
	len = sprintf((char*)nb_send_buff,"AT");
	switch(GET_NB_SEND_STEP)
	{
		case LSIPOPEN_SET:
			len += sprintf((char*)(nb_send_buff+len),"+LSIPOPEN=%s%s",socket_id_port,apn_addr);
			SET_DEALY_TIME(2000);
			break;
	
		case LSIPOPEN_CHECK:
			len += sprintf((char*)(nb_send_buff+len),"+LSIPOPEN?");
			SET_DEALY_TIME(200);
			break;
			
		case  SEND_READY:
			#if 0
			{
				char i = 0;
				send_net_infor.length = 10;
				for(;i<send_net_infor.length;i++)
					send_net_infor.data[i] = '0'+i;
			}
			#endif
			len += sprintf((char*)(nb_send_buff+len),"+LSIPSEND=1,\"%.*s\""
				,send_net_infor.length,send_net_infor.data);
			SET_DEALY_TIME(100);
			break;
		case START_SEND:
			len += sprintf((char*)(nb_send_buff+len),"+LSIPPUSH=1");
			SET_DEALY_TIME(100);
			break;

		case SEND_OK:
			/*根据需要跳到不同的step*/
		
			SET_NB_SEND_STEP(LSIPOPEN_SET);
			return true;
	}
	//调用串口发送数据
	SET_NB_SEND_FLAG(1);
	nb_send_buff[len] = 0x0d;	//
	len += 1;
	//调用串口发送
	port_data_send(nb_send_buff,len);
	return false;
}

void nb_data_recv_deal(uint8_t *deal_data)
{
	uint8_t* p_data;
	p_data = deal_data;
	if(strstr((char*)p_data,"+LSIPCLOSE") != NULL)	//tcp链接断开
	{
		SET_NB_SEND_STEP(LSIPOPEN_SET);
	}
	else
	{
		switch(GET_NB_SEND_STEP)
		{

			case LSIPOPEN_SET:
				if(strstr((char*)p_data,"+LSIPOPEN=1,") != NULL)
				{
					//if(strstr((char*)p_data,"OK") != NULL)
					{
						SET_NB_SEND_STEP(SEND_READY);
					}
				}
				else
				{
					SET_NB_SEND_STEP(LSIPOPEN_CHECK);
				}
					
				break;

			case LSIPOPEN_CHECK:
				if(strstr((char*)p_data,"+LSIPOPEN:") != NULL)
				{
					SET_NB_SEND_STEP(SEND_READY);
				}
				else
				{
					SET_NB_SEND_STEP(LSIPOPEN_SET);
				}
				break;
			case  SEND_READY:
				if(strstr((char*)p_data,"0\r") != NULL)
				{
					SET_NB_SEND_STEP(START_SEND);
				}
				break;
			case START_SEND:
				if(strstr((char*)p_data,"0\r") != NULL)
				{
					SET_NB_SEND_STEP(SEND_OK);//发送OK
				}
				break;
			default:
				break;
		}
	}
	
	//调用串口发送数据
	
	SET_NB_SEND_FLAG(0);
}

bool nb_data_sen_recv_mode(void)
{
	 return nb_data_send_fun();
}

bool nb_psm_order_send_fun(void)
{
	uint8_t len = 0;
	uint8_t nb_send_buff[100] = {0};
	len = sprintf((char*)nb_send_buff,"AT");
	switch(GET_NB_PSM_SEND_STEP)
	{
		case PSM_SET:
			len += sprintf((char*)(nb_send_buff+len),"+CPSMS=1");
			SET_DEALY_TIME(50);
			break;
	
		case  PSM_SET_OK:
			SET_NB_SEND_STEP(PSM_SET);
			return true;
	}
	//调用串口发送数据
	port_data_send(nb_send_buff,len);
	SET_NB_PSM_SEND_FLAG(1);
	return false ;
}

//获取GPS信息

bool nb_get_gps_infor_funcv(void)
{
	uint8_t len = 0;
	uint8_t nb_send_buff[100] = {0};
	len = sprintf((char*)nb_send_buff,"AT");
	if(GET_GPS_REGISTER_FLAG)
		return false;

	switch(GET_GPS_REGISTER_STATE)
	{
		case SEND_GET_GPS_DATA:
			len += sprintf((char*)(nb_send_buff+len),"+GPSNMEA");
			SET_DEALY_TIME(300);
			break;
		case GET_GPS_OK:
			SET_NB_SEND_STEP(SEND_GET_GPS_DATA);
			SET_DEALY_TIME(300);
			return true;
		default:
			break;
	}
	//调用串口发送数据
	port_data_send(nb_send_buff,len);
	SET_GPS_REGISTER_FLAG(1);
	return false ;
}

void nb_get_gps_infor_dela(uint8_t *deal_data)
{
	uint8_t* p_data;
	p_data = deal_data;
	switch(GET_GPS_REGISTER_STATE)
	{
		case SEND_GET_GPS_DATA:
			if(strstr((char*)p_data,"+GPSNMEA:") != NULL)
			{
				send_net_infor.length = nb_uart.ReceFifo.Length;
				memcpy(send_net_infor.data,p_data,send_net_infor.length);
				SET_GPS_REGISTER_STATE(GET_GPS_OK);
			}
			else
				SET_GPS_REGISTER_STATE(SEND_GET_GPS_DATA);
			break;
		default:
			break;
	}
	SET_GPS_REGISTER_FLAG(0);
}


bool nb_get_gps_infor_mode(void)
{
	return nb_get_gps_infor_funcv();
}


void nb_psm_recv_deal(uint8_t *deal_data)
{
	uint8_t* p_data;
	p_data = deal_data;
	switch(GET_NB_PSM_SEND_STEP)
	{
		case PSM_SET:
			if(strstr((char*)p_data,"0\r") != NULL)
			{
				SET_NB_SEND_STEP(PSM_SET_OK);
			}
			break;
		default:
			break;
	}
	SET_NB_PSM_SEND_FLAG(0);
}


bool nb_psm_mode(void)
{
	 return nb_psm_order_send_fun();
}


bool nb_psm_exit_send_mode(void)
{
	uint8_t len = 0 ;
	uint8_t nb_send_buff[80] = {0};
	len = sprintf((char*)nb_send_buff,"AT");
	switch(GET_NB_PSM_EXIT_SEND_STEP)
	{
	
		case PSM_EXIT_SEND_CLOSE_PSM:
			len += sprintf((char*)(nb_send_buff+len),"+CPSMS=0");
			break;
		
		case PSM_EXIT_SEND_ATV_X_SET:
		    len += sprintf((char*)(nb_send_buff+len),"V0");
			SET_DEALY_TIME(200);
			break;

		case PSM_EXIT_SEND_ATE_X_SET:
		    len += sprintf((char*)(nb_send_buff+len),"E0");
			SET_DEALY_TIME(200);
			break;
			
		case PSM_EXIT_SEND_CEREG_SET:
		    len += sprintf((char*)(nb_send_buff+len),"+CEREG?");
			SET_DEALY_TIME(200);
			break;
		
		case PSM_EXIT_SEND_CLOSE_OK:

			SET_NB_PSM_EXIT_SEND_STEP(PSM_EXIT_SEND_CLOSE_PSM);
			return true;
			
		default:
			break;
	}
	//调用串口发送数据
	port_data_send(nb_uart.send_buff,len);
	SET_NB_PSM_EXIT_SEND_FLAG(1);
	return false ;

}


void nb_psm_exit_recv_order_deal(uint8_t *deal_data)
{
	uint8_t* p_data;
	p_data = deal_data;
	switch(GET_NB_PSM_EXIT_SEND_STEP)
	{
	
		case PSM_EXIT_SEND_CLOSE_PSM:
			if(strstr((char*)p_data,"OK\r") != NULL)
			{
				SET_NB_SEND_STEP(PSM_EXIT_SEND_ATV_X_SET);
			}
			break;
		case PSM_EXIT_SEND_ATV_X_SET:
			if(strstr((char*)p_data,"0\r") != NULL)
			{
				SET_NB_SEND_STEP(PSM_EXIT_SEND_ATE_X_SET);
			}
			break;

		case PSM_EXIT_SEND_ATE_X_SET:
			if(strstr((char*)p_data,"0\r") != NULL)
			{
				SET_NB_SEND_STEP(PSM_EXIT_SEND_CLOSE_OK);
			}
			break;

		case PSM_EXIT_SEND_CEREG_SET:
			if(strstr((char*)p_data,",1\r") != NULL)
			{
				SET_NB_SEND_STEP(PSM_EXIT_SEND_CLOSE_OK);
			}
			break;

			
		default:
			break;
	}
	SET_NB_PSM_EXIT_SEND_FLAG(0);
	
}
	
bool nb_psm_exit_mode(void)
{
	switch(GET_NB_PSM_EXIT_STEP)
	{
		case NB_POWER_ON_SET:
			if(nbiot_power_on())
			{
				SET_NB_PSM_EXIT_STEP(SET_PSM_EXIT_SET);
			}
			break;
		case SET_PSM_EXIT_SET:
			if(nb_psm_exit_send_mode())
				return true;
			break;

		default:
			break;
	}
	return false ;
}

void nb_recv_data_deal(void)
{
	uint8_t Data;
	//if(GET_DEALY_TIME)
	//	return;
	while(NbUartPopReceiveByte(&Data))
	{
	#if 1
		switch(GET_NB_RECV_STATE)
		{
			case NB_REVC_STATE_START_A:
				if(Data == 'A')
					SET_NB_RECV_STATE(NB_REVC_STATE_START_T);
				break;
				
			case NB_REVC_STATE_START_T:
				if(Data == 'T')
				{
					SET_NB_RECV_STATE(NB_REVC_STATE_DATA);
					nb_uart.ReceFifo.Length = 0;
				}
				else
					SET_NB_RECV_STATE(NB_REVC_STATE_START_A);
				break;
				
			case NB_REVC_STATE_DATA:
				nb_uart.ReceFifo.Data[nb_uart.ReceFifo.Length++] =Data;
				if(GET_DEALY_TIME<20)
					SET_DEALY_TIME(20);
				break;
		}
	#else
		nb_uart.ReceFifo.Data[nb_uart.ReceFifo.Length++] =Data;
		if(GET_DEALY_TIME<10)
			SET_DEALY_TIME(10);
	#endif
	}
	if(GET_DEALY_TIME)
		return;
	if(nb_uart.ReceFifo.Length >= 2)	//假设接收数据完成
	{
		{
			char i=0;
			nb_debug("\r\n");
			for(;i<nb_uart.ReceFifo.Length;i++)
				nb_debug("%c",nb_uart.ReceFifo.Data[i]);
			nb_debug("\r\n");
		}
		switch(GET_NB_WORK_MODE)
		{
			case NB_REGISTER_APN_MODE:
			//	if(GET_REGISTER_FLAG)
				{
					card_register_recv_data_deal(nb_uart.ReceFifo.Data);
				}
				break;
			case NB_REGISTER_NET_MODE:
				if(GET_NET_REGISTER_FLAG)
				{
					nb_recv_net_order_deal(nb_uart.ReceFifo.Data);
				}
				break;
			case NB_GET_GPS_INFOR_MODE:
				if(GET_GPS_REGISTER_FLAG)
				{
					nb_get_gps_infor_dela(nb_uart.ReceFifo.Data);
				}
				break;
			case NB_DATA_SEN_RECV_MODE:
				if(GET_NB_SEND_FLAG)
				{
					nb_data_recv_deal(nb_uart.ReceFifo.Data);
				}
				break;
			case NB_PSM_MODE:
				if(GET_NB_PSM_SEND_FLAG)
				{
					nb_psm_recv_deal(nb_uart.ReceFifo.Data);
				}
				break;
			case NB_PSM_EXIT_MODE:
				if(GET_NB_PSM_EXIT_SEND_FLAG)
				{
					nb_psm_exit_recv_order_deal(nb_uart.ReceFifo.Data);
				}
				break;
			default:
				break;
		}
		memset(&nb_uart.ReceFifo,0,sizeof(nb_uart.ReceFifo));
		SET_NB_RECV_STATE(0);
	}
}

void nb_work_cycle(void)
{
	if(GET_DEALY_TIME)
	{
		GET_DEALY_TIME--;
		return;
	}
	//nb_debug("nb_work_cycle:%d\r\n",GET_NB_WORK_MODE);
	switch(GET_NB_WORK_MODE)
	{
		case NB_MODE_WAIT:
			
			SET_DEALY_TIME(1000);
			SET_NB_WORK_MODE(NB_MODE_POWE_ON_MODE);
			break;
		case NB_MODE_POWE_ON_MODE:
		if(nbiot_power_on())
			SET_NB_WORK_MODE(NB_REGISTER_APN_MODE);
			break;
		
		case NB_REGISTER_APN_MODE:
		if(nb_register_apn())
			SET_NB_WORK_MODE(NB_REGISTER_NET_MODE);
			break;
		
		case NB_REGISTER_NET_MODE:
		if(nb_register_net_mode())
			SET_NB_WORK_MODE(NB_GET_GPS_INFOR_MODE);
			break;
		
		case NB_GET_GPS_INFOR_MODE:
			if(nb_get_gps_infor_mode())
				SET_NB_WORK_MODE(NB_DATA_SEN_RECV_MODE);
			break;
		case NB_DATA_SEN_RECV_MODE:
			if(nb_data_sen_recv_mode())
				SET_NB_WORK_MODE(NB_PSM_MODE);
			break;
		
		case NB_PSM_MODE:
			nb_psm_mode();
			break;
		
		case NB_PSM_EXIT_MODE:
			if(nb_psm_exit_mode())
				SET_NB_WORK_MODE(NB_REGISTER_NET_MODE);
			break;
		
		default:
			break;
	}

}


#if defined(NB_DEBUG)

void DebugSet(void)
{
//	simple_uart_config(MT2503_TX_PIN,MT2503_RX_PIN,false);
//	Uart_Deinit(MT2503_TX_PIN,MT2503_RX_PIN);
	nrf_gpio_cfg_output(DEBUG_TX);
	nrf_gpio_cfg_input(DEBUG_RX, NRF_GPIO_PIN_NOPULL);  

	NRF_UART0->PSELTXD = DEBUG_TX;
	NRF_UART0->PSELRXD = DEBUG_RX;


}

void DebugClose(void)
{
	nrf_gpio_cfg_output(MT2503_TX_PIN);
	nrf_gpio_cfg_input(MT2503_RX_PIN, NRF_GPIO_PIN_NOPULL);  

	NRF_UART0->PSELTXD = MT2503_TX_PIN;
	NRF_UART0->PSELRXD = MT2503_RX_PIN;
}
#endif





