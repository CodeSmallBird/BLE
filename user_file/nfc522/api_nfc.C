#include "api_nfc.H"

#if defined(DEBUG_UART)
#define DEBUG_APP_NFC
#endif

#define WALLET_ADDR                 0x01
#define WALLET_ADDR_2    	        0x02

#define WALLET_CTR_ADDR		       	0x03

#define PRODUCT_DATA_BLOCK	0x00			

static app_timer_id_t   				m_nfc_timeout_id;
#if defined(ADD_HOER_DETEC)
#define NFC_ON_MEAS_INTERVAL               APP_TIMER_TICKS(50, 0)	//50
#else
#define NFC_ON_MEAS_INTERVAL               APP_TIMER_TICKS(350, 0)	//50
#endif
unsigned char CardType[4];
unsigned char    CardSnr[4];
unsigned char nfc_read_data[16];
unsigned char nfc_wirte_data[16] ={0x01,0x00,0x77,0x77,0x77,0x77,0,0,0,0,0,0};
unsigned char nfc_money_data[4];

//unsigned char    KeyACode1[6] = {0x00,0x00,0x00,0x00,0x00,0x00};//{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
//unsigned char    KeyBCode1[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

unsigned char    KeyACode1[6] = {0x1A,0x3A,0x5A,0x7A,0x9A,0xBA};
unsigned char    KeyBCode1[6] = {0x1B,0x3B,0x5B,0x7B,0x9B,0xAB};
unsigned char    ctrlCode1[4] = {0xff,0x0f,0x00,0x00};
unsigned char	 ScretCode1[16] = {0x1A,0x3A,0x5A,0x7A,0x9A,0xBA,0xFF,0x07,0x80,0x69,0x1B,0x3B,0x5B,0x7B,0x9B,0xAB};
//unsigned char	 ScretCode1[16] = {0x1A,0x3A,0x5A,0x7A,0x9A,0xBA,0xFF,0x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

bool nfc_work_repeat_init(void)
{
	SpiInit();
	NRSTPD_ON;
	nrf_delay_ms(1);
#if 1
	WriteRawRC(ModeReg,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
	WriteRawRC(TReloadRegL,30);           
	WriteRawRC(TReloadRegH,0);
	WriteRawRC(TModeReg,0x8D);
	WriteRawRC(TPrescalerReg,0x3E);
	WriteRawRC(TxAutoReg,0x40); 
	M500PcdConfigISOType( 'A' );
#else
/*
	{  
		uint8_t SpiRxData = 0;
	 // delay_ms(1);
	  WriteRawRC(0x15,0x40);          //Type A  
	  WriteRawRC(0x14,0x83);          //打开天线
	 // delay_ms(1);
	}
	*/
#endif
	return true;
}

//判断数据是否需要加密
bool data_write_to_card(unsigned char addr,unsigned char *pData)
{
	bool status = 0 ;
	uint8_t encrpty_data[16];
	uint8_t Write_data[16];
	memcpy(Write_data,pData,16);
	if(addr == WALLET_ADDR)
	{
	#if defined(DEBUG_APP_NFC)
		printf("Secret_data\r\n");
	#endif
		encrypt_data_funcv(Write_data,CardSnr,encrpty_data,12);
		memcpy(Write_data+12,encrpty_data,4);
	}
	else
	{

	}
	status = PcdWrite(addr,Write_data);
	if(status == MI_OK)
		status = 1;
	else
		status = 0;
#if defined(DEBUG_APP_NFC)
	if(status == 1)
	{
		printf("写入数据--addr:%d\r\n",addr);
		{
			uint8_t i;
			for(i=0;i<16;i++)
				printf("%02x ",Write_data[i]);
			printf("\r\n");
		}
	}
	printf("write_status:%d\r\n",status);
#endif
	return status;

}

//判断数据是否需要解密
bool data_read_from_card(unsigned char addr,unsigned char *pData)
{
	char status = 0 ;
	uint8_t decrpty_data[16];
	uint8_t read_data[16];	
	memset(pData,0,16);
	status = PcdRead(addr,pData);
	if((addr == WALLET_ADDR)&&(status == MI_OK))
	{
		memcpy(read_data,pData,16);
		if(decrypt_data_funcv(read_data,CardSnr,decrpty_data,12) == 0)
			status = MI_ERR;
		else
		{
			memcpy(pData,read_data,16);
		}
		
	}
	else
	{

	}
	if(status == MI_OK)
		status = 1;
	else
		status = 0;
	
#if defined(DEBUG_APP_NFC)
	if(status == 1)
	{
		printf("读出数据\r\n");
		{
			uint8_t i;
			for(i=0;i<16;i++)
				printf("%d ",pData[i]);
			printf("\r\n");
		}
	}
	printf("read_from_status:%d\r\n",status);
#endif
	return status;

}



//检查卡片是否为复制卡
bool check_card_infor(void)
{
	bool status = 0 ;
	//对扇区0的数据块0写入数据--验证是否为复制卡
	if(data_write_to_card(PRODUCT_DATA_BLOCK,"Test") == MI_ERR)
	{
	#if defined(DEBUG_APP_NFC)
		printf("是复制卡\r\n");
	#endif								
		status =  1;
	}
	else
	{
	#if defined(DEBUG_APP_NFC)
		printf("不是复制卡\r\n");
	#endif
	}
	return status;
}

bool find_card_mode(void)
{
	bool status = 0;
	if(PcdRequest(0x52,CardType) == 0)		//寻卡成功
	{
		if(PcdAnticoll(CardSnr) == 0)		// 防冲撞ok
		{
			 if(PcdSelect(CardSnr) == 0)	//选定卡片	
			 {
			 
			#if 0//defined(DEBUG_APP_NFC)
				printf("选定卡片ok\r\n");
				{
					uint8_t i;
					printf("CARD_ID\r\n");
					for(i=0;i<4;i++)
						printf("%d ",CardSnr[i]);
					printf("\r\n");
				}
			#endif
				// 验证KEYB,对扇区0的控制块的秘钥验证
				if(PcdAuthState(PICC_AUTHENT1A,WALLET_CTR_ADDR,KeyACode1,CardSnr) == 0)
				{
					status = 1;
				}
			#if defined(DEBUG_APP_NFC)
				printf("find_card_mode --status:%d\r\n",status);
			#endif
			 }
		}
	}
	return status;
}

// 写入卡片数据
unsigned char write_data_to_card(void)
{
	unsigned char status = SET_CARD_ERROR;
#if defined(DEBUG_APP_NFC)
	printf("write_data_to_card\r\n");
#endif
	if(find_card_mode())
	{
		if(data_write_to_card(WALLET_ADDR,nfc_wirte_data) == 0)
		{
		#if defined(DEBUG_APP_NFC)
			printf("WALLET_ADDR--写入OK\r\n");
		#endif
			if(check_card_infor())
			{
				status = SET_CARD_OK;
				buzzer_start(BUZZER_NORMAL);//蜂鸣器响
			}
			else
			{
				status = COPY_CARD_ERROR;
				buzzer_start(BUZZER_ERROR);//蜂鸣器响
			}
		}
	}
	return status;
}


// 改变秘钥
unsigned char change_scret_to_card(void)
{
	unsigned char status = SET_CARD_ERROR;
#if defined(DEBUG_APP_NFC)
	printf("write_data_to_card\r\n");
#endif
	if(find_card_mode())
	{
		if(data_write_to_card(WALLET_CTR_ADDR,ScretCode1) == 0)
		{
		#if defined(DEBUG_APP_NFC)
			printf("WALLET_CTR_ADDR--写入OK\r\n");
		#endif
			if(check_card_infor())
			{
				status = SET_CARD_OK;
				buzzer_start(BUZZER_NORMAL);//蜂鸣器响
			}
			else
			{
				status = COPY_CARD_ERROR;
				buzzer_start(BUZZER_ERROR);//蜂鸣器响
			}
		}
	}
	return status;
}






//dd_mode[IN]：命令字
//               0xC0 = 扣款
//               0xC1 = 充值
//pValue[IN]：4字节增(减)值，低位在前
unsigned char write_money_to_card(unsigned char dd_mode)
{
	unsigned char status = SET_CARD_ERROR;
	if(find_card_mode())
	{
		//对块1进行充值或扣款
		if(PcdValue(dd_mode,WALLET_ADDR,nfc_money_data) == 0)
		{
			if(check_card_infor())
			{
				status =  SET_CARD_OK;
			}
			else
			{
				status =  COPY_CARD_ERROR;
			}
		}
	}
	return status;
}

//@钱包扇区初始化
//扇区0，数据块1作为钱包地址
//扇区0，数据控制块设置参数：允许对数据块1各种操作
unsigned char set_nfc_wallet_block(void)
{
	unsigned char status = 0;
	unsigned char ctrl_data[16];
	if(PcdRequest(0x52,CardType) == 0)		//寻卡成功
	{
		if(PcdAnticoll(CardSnr) == 0)		// 防冲撞ok
		{
			 if(PcdSelect(CardSnr) == 0)	//选定卡片	
			 {
				// 验证KEYA,对控制块3的区域验证块1，
				if(PcdAuthState(PICC_AUTHENT1A,WALLET_CTR_ADDR,KeyACode1,CardSnr) == 0)
				{
					memcpy(ctrl_data,KeyACode1,6);	
					memcpy(ctrl_data+4,ctrlCode1,4);
					memcpy(ctrl_data+10,KeyBCode1,6);
					if(data_write_to_card(WALLET_CTR_ADDR,ctrl_data) == 0)
						status = 1;
				}

			 }
		}
	}
	return status;
}

//卡的开关锁流程
//card_infor--读入的卡的扇区信息
// 返回： 0 异常   1 正常

#define COST_MIN		1

unsigned char update_card_info(uint8_t *card_data_infor_data)
{
	unsigned char status = 0;
	unsigned char error = 0;
	
	memcpy(nfc_wirte_data,card_data_infor_data,sizeof(nfc_wirte_data));
	for(;error<3;error++)
	{
		if(data_write_to_card(WALLET_ADDR,nfc_wirte_data) == 1)
		{
			status = 1;
			device_name_info.card_ride.delay_flag = true;
			device_name_info.card_ride.dect_delay_time = 5;
			break;
		}
	}
	return status ;
}

//返回值： 1 正常  --- 0 异常
unsigned char nfc_lock_process(uint8_t *card_data_infor)
{
	unsigned char status = 0;
	uint8_t deal_data[16];
	int32_t money = 0;
	memcpy(deal_data,card_data_infor,16);
	money |= ((int32_t)deal_data[REG_MONEY_HIGH_HIGH])<<24;
	money |= ((int32_t)deal_data[REG_MONEY_HIGH_LOW])<<16;
	money |= ((int32_t)deal_data[REG_MONEY_LOW_HIGH])<<8;
	money |= deal_data[REG_MONEY_LOW_LOW];
	
	if(deal_data[REG_CARD_STATE] == CARD_ACTIVATION_STATE)		//激活状态
	{
	#if defined(DEBUG_APP_NFC)
		printf("nfc_card_infor.id:---111\r\n");
		for(uint8_t i = 0;i<4;i++)
			printf("%d--%d ",device_name_info.card_ride.card_id[i],CardSnr[i]);
		printf("\r\n");
	#endif
		if(memcmp(device_name_info.card_ride.card_id,CardSnr,sizeof(CardSnr))  == 0)//卡号一致
		{
			if(device_name_info.card_ride.delay_flag == true)
			{
				device_name_info.card_ride.delay_time = 0;
				status = 1;
			}
			else
			{
				if(deal_data[REG_LOCK_STATE] == NFC_CARD_LOCK_CLOSE)	// 卡状态为关锁
				{
					//进入开锁流程
					goto NFC_OPEN_LOCK;
				}
				else if(deal_data[REG_LOCK_STATE] == NFC_CARD_LOCK_OPEN)// 卡状态为开锁
				{
				//进入结束计费流程
				#if defined(DEBUG_APP_NFC)
					printf("结束计费\r\n");
				#endif
				#if defined(DEBUG_APP_NFC)
					printf("money:%ld--run_time:%ld\r\n",money,device_name_info.card_ride.ride_time);
				#endif
					money -= (device_name_info.card_ride.ride_time/3600) *COST_MIN;		//1h 1元
					deal_data[REG_MONEY_LOW_LOW]	= money;
					deal_data[REG_MONEY_LOW_HIGH] 	= money>>8;
					deal_data[REG_MONEY_HIGH_LOW] 	= money>>16;
					deal_data[REG_MONEY_HIGH_HIGH]	= money>>24;
					deal_data[REG_LOCK_STATE] 		= NFC_CARD_LOCK_CLOSE;

					deal_data[REG_RTIME_LOW_LOW]	= device_name_info.card_ride.ride_time;
					deal_data[REG_RTIME_LOW_HIGH]	= device_name_info.card_ride.ride_time>>8;
					deal_data[REG_RTIME_HIGH_LOW]	= device_name_info.card_ride.ride_time>>16;
					deal_data[REG_RTIME_HIGH_HIGH]	= device_name_info.card_ride.ride_time>>24;
					
					if(update_card_info(deal_data))
					{
						status = 1;
						memcpy(card_data_infor,deal_data,16);
						buzzer_start(BUZZER_NORMAL);//蜂鸣器响
					}
				}

			}
		}
		else		//卡号不一致
		{
			if(deal_data[REG_LOCK_STATE] == NFC_CARD_LOCK_CLOSE)	// 卡状态为关锁
			{
				//进入开锁流程
			NFC_OPEN_LOCK:	
			#if defined(DEBUG_APP_NFC)
				printf("NFC_OPEN_LOCK:%d\r\n",money);
			#endif
				if(money > 0)//进入开锁流程
				{
				#if 0
					if(update_card_info(deal_data))
					{
						open_key_ctrl();
						status = 1;

					}
				#else
					deal_data[REG_LOCK_STATE] = NFC_CARD_LOCK_OPEN;
					if(update_card_info(deal_data))
					{
						status = 1;
						memcpy(card_data_infor,deal_data,16);
						memcpy(device_name_info.card_ride.card_id,CardSnr,4);
						open_key_ctrl();
					}

				#endif
				}
			}
			else if(deal_data[REG_LOCK_STATE] == NFC_CARD_LOCK_OPEN)// 卡状态为开锁
			{
				//设置卡为锁定状态
				deal_data[REG_CARD_STATE] = CARD_LOCK_STATE;
				if(update_card_info(deal_data))
				{
				
				}
				
			}
		}
	}
	return status;
}


unsigned char get_card_infor(void)
{
	unsigned char status = SET_CARD_ERROR;
	if(find_card_mode())
	{
		//对扇区0的数据块1读取
		if(data_read_from_card(WALLET_ADDR,nfc_read_data) == 1)
		{
		#if 0//defined(DEBUG_APP_NFC)
			printf("扇区0--数据块1\r\n");
			{
				uint8_t i;
				printf("nfc_read_data\r\n");
				for(i=0;i<16;i++)
					printf("%d ",nfc_read_data[i]);
				printf("\r\n");
			}
		#endif
			if(nfc_lock_process(nfc_read_data))
			{
			/*	if(check_card_infor())
				{
					status =  SET_CARD_OK;
				}
				else
				{
					status =  COPY_CARD_ERROR;
				}
			*/
			}
			else
			{
				buzzer_start(BUZZER_ERROR);//蜂鸣器响
			}
		}
	}
	return status;
}



unsigned char read_card_infor(void)
{
	unsigned char status = SET_CARD_ERROR;
	if(find_card_mode())
	{
		//对扇区0的数据块1读取
		if(data_read_from_card(WALLET_CTR_ADDR,nfc_read_data) == 0)
		{
		#if defined(DEBUG_APP_NFC)
			printf("扇区0--数据块1\r\n");
			{
				uint8_t i;
				printf("nfc_read_data\r\n");
				for(i=0;i<16;i++)
					printf("%d ",nfc_read_data[i]);
				printf("\r\n");
			}
		#endif
			if(check_card_infor())
			{
				status =  SET_CARD_OK;
			}
			else
			{
				status =  COPY_CARD_ERROR;
			}
		}
	}
	return status;
}

uint8_t nfc_work_mode(uint8_t nfc_mode,uint8_t *param)
{
	uint8_t status = 0;
	switch(nfc_mode)
	{
		case GET_CARD_MODE:
			status = get_card_infor();
			break;
		case WRITE_MONEY_TO_CARD_MODE:
			status = write_money_to_card(*param);
			break;
		case WRITE_DATA_TO_CARD_MODE:
			status = write_data_to_card();
			break;
		case READ_DATA_FROM_CARD_MODE:
			status = read_card_infor();
			break;
		case CHANGE_CARD_SCREAT_MODE:
			status = change_scret_to_card();
			break;
		default:
			break;
	}
	return status;
}


void nfc_timer_start(void)
{
	uint32_t err_code;
	err_code = app_timer_start(m_nfc_timeout_id, NFC_ON_MEAS_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);
}

void nfc_timer_stop(void)
{
	uint32_t err_code;
	err_code = app_timer_stop(m_nfc_timeout_id);
	APP_ERROR_CHECK(err_code);
}

void nfc_card_work_set(uint8_t nfc_set)
{
	if((device_name_info.lock_state != CLOSE_LOCK)
		||(device_name_info.card_ride.dect_delay_time))
		 nfc_set = 0;
	if(nfc_set)
	{
		nfc_timer_stop();
		nfc_work_repeat_init();
		if(nfc_work_mode(GET_CARD_MODE,0))
		{
		
		}
		SpiReset();
		NRSTPD_OFF;
		nfc_timer_start();
	}
	else
	{
		SpiReset();
		NRSTPD_OFF;
	}
}
#if defined(ADD_HOER_DETEC)
void hou_er_detect_init(void)
{
	nrf_gpio_cfg_input(KEY_TEST,(nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pullup);
}

#endif
void nrc_timeout_handler(void * p_context)
{
	static uint8_t run_flag = 0;
#if defined(ADD_HOER_DETEC)
	if(nrf_gpio_pin_read(KEY_TEST) == 0)
	{
		run_flag = 1;
	}
	else
		run_flag = 0;
#else
	static uint8_t nfc_run_cnt = 0;
	nfc_run_cnt++;
	if(nfc_run_cnt % 2 == 0)
	{
		run_flag = 1;
	}
	else
	{
		run_flag  = 0;
	}
#endif

	nfc_card_work_set(run_flag);
}

void nfc_timer_create(void)
{
	uint32_t err_code;
	err_code = app_timer_create(&m_nfc_timeout_id,APP_TIMER_MODE_REPEATED,nrc_timeout_handler);
	APP_ERROR_CHECK(err_code);
	nfc_timer_start();
}


void nfc_card_init(void)
{
#if defined(ADD_HOER_DETEC)
	hou_er_detect_init();
#endif
#if 1
	PcdReset();
	PcdAntennaOff(); 
	PcdAntennaOn();  
	M500PcdConfigISOType( 'A' );
	nfc_timer_create();
#else
	NRSTPD_INIT;
	NRSTPD_OFF;
	SpiInit();
	SpiReset();
#endif
}



