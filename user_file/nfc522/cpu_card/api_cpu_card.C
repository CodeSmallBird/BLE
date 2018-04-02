#include "api_cpu_card.H"
#include ".\cpu_card\des.c"

#if defined(DEBUG_UART)
#define DEBUG_APP_NFC
#endif

static app_timer_id_t   				m_nfc_timeout_id;
#if defined(ADD_HOER_DETEC)
#define NFC_ON_MEAS_INTERVAL               APP_TIMER_TICKS(50, 0)	//50
#else
#define NFC_ON_MEAS_INTERVAL               APP_TIMER_TICKS(350, 0)	//50
#endif

CPU_CARD_INFOR cpu_card_infor;



bool nfc_work_repeat_init(void)
{
	SpiInit();
	NRSTPD_ON;
	nrf_delay_ms(1);
	WriteRawRC(ModeReg,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
	WriteRawRC(TReloadRegL,30);           
	WriteRawRC(TReloadRegH,0);
	WriteRawRC(TModeReg,0x8D);
	WriteRawRC(TPrescalerReg,0x3E);
	WriteRawRC(ModGsCfgReg,0x3F); 
	WriteRawRC(TxAutoReg,0x40); 
	cpu_card_infor.Pcb = 0;
	M500PcdConfigISOType( 'A' );
	return true;
}

char find_card_mode(void)
{
	s8 status ;
	u8 Reqcode = 0x52;
	u8 *pTagType = cpu_card_infor.cpu_type;
	
	memset(cpu_card_infor.reccv_buf, 0x00, MAXRLEN); //清0
	//CPU_BaudRate_Set(0);
	status = PcdRequest(Reqcode, pTagType);  				//寻卡
	if(status == MI_ERR)
	{
		status = PcdRequest(Reqcode, pTagType);
		if(status == MI_ERR)
		{
			return MI_ERR;
		}
	}

	status = PcdAnticoll(cpu_card_infor.reccv_buf);								//防冲撞, 获取ID
	if(status == MI_ERR)
	{
		return MI_ERR;
	}

#if defined(DEBUG_UART)
	printf("find_card_mode-cpu_card_infor.recv_len:%d",(u8)cpu_card_infor.recv_len);
		for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
		{
			printf("0x%x ",cpu_card_infor.reccv_buf[i]);
		}
		printf("\r\n");
		
		printf("status:%d\r\n",status);
#endif
	memcpy(cpu_card_infor.card_snr,cpu_card_infor.reccv_buf,4);
	status = PcdSelect(cpu_card_infor.card_snr);								//选择卡片
	if(status == MI_ERR)
	{
		return MI_ERR;
	}
	return MI_OK;

}

//*************************************************************************
// 函数名	：PcdRats
// 描述		：转入APDU命令格式
// 入口		： 无
// 出口		： DataOut 输出的数据，	Len 输出数据的长度
// 返回		：成功返回MI_OK
//*************************************************************************

s8 PcdRats(u8 * DataOut,u8 * Len)
{
	s8 status =MI_ERR;  
	u16 unLen;
	u8 ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);	// 清空校验成功标志,清除MFCrypto1On位

	memset(ucComMF522Buf, 0x00, MAXRLEN);

	ucComMF522Buf[0] = 0xE0;		
	ucComMF522Buf[1] = 0x51;				

    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);	// 生成发送内容的CRC校验,保存到最后两个字节

	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,DataOut,&unLen,WRITE_CARD_TIME); 		// 将收到的卡片类型号保存

	if (status == MI_OK)
	{
		Len[0]= unLen/8-2;
		status = MI_OK;
	}	 
	else
		status = MI_ERR;	 
	return status;								//返回结果
}


//*************************************************************************
// 函数名	：CpuReset()
// 描述		：CPU卡专用复位
// 入口		：无
// 出口		：Data_Out 输出的复位数据信息及长度
// 返回		：成功返回9000
//*************************************************************************

u16 CardReset(u8 * Data_Out,u8 *  Len)
{
	s8 status = MI_ERR;	
	status = PcdRats(Data_Out,Len);										//卡片复位

#if defined(DEBUG_APP_NFC)
	u8 i;
	printf("CardReset\r\n");
	for(i=0;i<*Len;i++)
		printf("%x ",Data_Out[i]);
	printf("\r\n");
#endif
	return status;
}


//*************************************************************************
// 函数名	：PcdSwitchPCB(void)
// 描述		：切换分组号
// 入口		：
// 出口		：
// 返回		：成功返回MI_OK
//*************************************************************************

void PcdSwitchPCB(void)
{
	switch(cpu_card_infor.Pcb)
	{
		case 0x00:
			cpu_card_infor.Pcb=0x0A;
			break;
		case 0x0A:
			cpu_card_infor.Pcb=0x0B;
			break;
		case 0x0B:
			cpu_card_infor.Pcb=0x0A;
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//*************************************************************************
// 函数名	：Pcd_Cmd
// 描述		：执行上位机发来的指令
// 入口		：pDataIn : 要执行的指令 In_Len  指令数据长度					
// 出口		：pDataOut：输出执行后的返回数据   Out_Len输出的数据长度
// 返回		：MI_OK
//*************************************************************************
 
u8 Pcd_Cmd(u8* pDataIn, u8  In_Len, u8* pDataOut,u8 * Out_Len)
{
	s8 status =MI_ERR;  
	u16 unLen;
	u8 ucComMF522Buf[MAXRLEN]; 
	u8 i;

	ClearBitMask(Status2Reg,0x08);					// 清空校验成功标志,清除MFCrypto1On位
	memset(ucComMF522Buf, 0x00, MAXRLEN);

 	PcdSwitchPCB();
	
	ucComMF522Buf[0] = cpu_card_infor.Pcb;
	ucComMF522Buf[1] = 0x01;

	ucComMF522Buf[2] = pDataIn[0];				// CLA
	ucComMF522Buf[3] = pDataIn[1];				// INS 			
	ucComMF522Buf[4] = pDataIn[2];				// P1						 
	ucComMF522Buf[5] = pDataIn[3];				// P2					
	ucComMF522Buf[6] = pDataIn[4];				// LEN	
	
	for(i=0;i<ucComMF522Buf[6];i++)				//DATA
	{
		ucComMF522Buf[7+i] = pDataIn[5+i];	  
	}
		
    CalulateCRC(ucComMF522Buf,In_Len+2,&ucComMF522Buf[In_Len+2]);	// 生成发送内容的CRC校验,保存到最后两个字节
	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,In_Len+4,pDataOut,&unLen,WRITE_CARD_TIME); 		
								
	if (status == MI_OK)
	{
	#if defined(DEBUG_UART)
		printf("unLen:%d\r\n",unLen);
	#endif
		//Out_Len[0] = unLen/8-4;	 //接收到数据的长度,不要前面和后面的各两个字节，才是返回的有用数据
		Out_Len[0] = unLen/8;	 //接收到数据的长度,不要前面和后面的各两个字节，才是返回的有用数据
		return MI_OK;

	}
	else
	{
	    Out_Len[0] = unLen/8-4;
		if((pDataOut[2]==0x90)&&(pDataOut[3]==0x00))
		return MI_OK;
		else
		return MI_ERR;
	}
}


//卡的开关锁流程
//card_infor--读入的卡的扇区信息
// 返回： 0 异常   1 正常
/***************************************************************************************/
/*************CPU卡的读写流程************************************************************/
/***************************************************************************************/
//选择MF文件
char choose_mf_file(void)
{

	char status = MI_ERR;
	unsigned char Send_Buf[] = {0x00,0xa4,0x00,0x00,0x02,0x3F,0x00};
	
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("choose_mf_file-cpu_card_infor.recv_len:%d",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif
	return status;
		
}



//MF 秘钥文件的建立
char creat_screat_file(void)
{
	char status = MI_ERR;
	unsigned char Send_Buf[] = {0x80,0xE0,0x00,0x00,0x07,
									  0x3F,0x00,0x50,0x01,0xf0,0xff,0xff};
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("creat_screat_file-cpu_card_infor.recv_len:%d",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif
	if(cpu_card_infor.recv_len>5)
	{
		if((cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-4] == 0x90)&&(cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-3] == 0x00))
			status = MI_OK;
	}
		
	return status;
		
}


//装载8字节线路保护秘钥
char load_line_code_file(void)
{
	char status = MI_ERR;
	unsigned char Send_Buf[] = {0x80,0xD4,0x01,0x00,0x0D,
									 0x36,0xF0,0xF0,0xFF,0x33,
									 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("load_line_code_file-cpu_card_infor.recv_len:%d",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif
	if(cpu_card_infor.recv_len>5)
	{
		if((cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-4] == 0x90)&&(cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-3] == 0x00))
			status = MI_OK;
	}

	return status;	
}


//装载16字节外部认证秘钥
char load_external_code_file(void)
{
	char status = MI_ERR;
	unsigned char Send_Buf[] = {0x80,0xD4,0x01,0x00,0x15,
									 0x39,0xF0,0xF0,0xAA,0x88,
									 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
									 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("load_external_code_file-cpu_card_infor.recv_len:%d",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif
	if(cpu_card_infor.recv_len>5)
	{
		if((cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-4] == 0x90)&&(cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-3] == 0x00))
			status = MI_OK;
	}

	return status;	
}

//建立定长记录文件
char creat_const_record_len_file(void)
{
	char status = MI_ERR;
	unsigned char Send_Buf[] = {0x80,0xE0,0x00,0x08,0x07,
									 0x2A,0x02,0x13,0xF0,0x00,0xFF,0xFF};
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("creat_const_record_len_file-cpu_card_infor.recv_len:%d",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif
	if(cpu_card_infor.recv_len>5)
	{
		if((cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-4] == 0x90)&&(cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-3] == 0x00))
			status = MI_OK;
	}

	return status;	
}
//写入记录内容
char write_const_record_len_file(void)
{
	char status = MI_ERR;
	unsigned char Send_Buf[] = {0x00,0xE2,0x00,0x08,0x13,
									 0x61,0x11,0x4F,0x09,0xA0,0x00,0x00,
									 0x00,0x03,0x86,0x98,0x07,0x01,0x50,
									 0x04,0x50,0x42,0x4F,0x43};
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("write_const_record_len_file-cpu_card_infor.recv_len:%d",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif
	if(cpu_card_infor.recv_len>5)
	{
		if((cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-4] == 0x90)&&(cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-3] == 0x00))
			status = MI_OK;
	}

	return status;	
}

//建立二进制文件
//建立0xC0标识的文件
char creat_binary_file(void)
{
	char status = MI_ERR;
	unsigned char Send_Buf[] = {0x80,0xE0,0x00,0xC0,0x07,
									 0x28,0x02,0x16,0xF0,0xF0,0xFF,0xFF};
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("creat_binary_file-cpu_card_infor.recv_len:%d",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif
	status = MI_ERR;
	if(cpu_card_infor.recv_len>5)
	{
		if((cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-4] == 0x90)&&(cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-3] == 0x00))
			status = MI_OK;
	}

	return status;	
}

//选择二进制文件
//文件标识为0xc0
char choose_binary_file(void)
{
	char status = MI_ERR;
	unsigned char Send_Buf[] = {0x00,0xA4,0x00,0x00,0x02,0x00,0xC0};
																	 
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("choose_binary_file-cpu_card_infor.recv_len:%d",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif

	return status;	
}


//写二进制文件(当前文件)
//文件标识为0xc0
char write_binary_file(u8* write_buff,u8 length)
{
	char status = MI_ERR;
	unsigned char Send_Buf[48] = {0x00,0xD6,0x00,0x00,0x0};
	Send_Buf[4] = length;
	memcpy(Send_Buf+5,write_buff,length);
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf,5+length,cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("write_binary_file-cpu_card_infor.recv_len:%d",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif
	return status;	
}

//读二进制文件
//文件标识为0xc0
char read_binary_file(void)
{
	char status = MI_ERR;
	unsigned char Send_Buf[] = {0x00,0xB0,0x00,0x00,16};
																	 
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("read_binary_file-cpu_card_infor.recv_len:%d",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif
	return status;	
}

//DF下的文件建立及操作
//建立3f01文件
char creat_df_file(void)
{
	char status = MI_ERR;
	unsigned char Send_Buf[] = {0x80,0xE0,0x3F,0x01,0x11,0x38,0x03,
									 0x6F,0xF0,0xF0,0x95,0xFF,0xFF,
									 0xA0,0x00,0x00,0x00,0x03,0x86,
									 0x98,0x07,0x01};
																	 
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("cpu_card_infor.reccv_buf2--cpu_card_infor.recv_len:%d\r\n",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif
	if(cpu_card_infor.recv_len>5)
	{
		if((cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-4] == 0x90)&&(cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-3] == 0x00))
			status = MI_OK;
	}
	return status;	
}

//选择3f01文件
char choose_df_file(void)
{
	char status = MI_ERR;
	unsigned char Send_Buf[] = {0x00,0xA4,0x00,0x00,0x02,0x3F,0x01};
									
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("cpu_card_infor.reccv_buf2--cpu_card_infor.recv_len:%d\r\n",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif
	if(cpu_card_infor.recv_len>5)
	{
		if((cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-4] == 0x90)&&(cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-3] == 0x00))
			status = MI_OK;
	}
	return status;	
}

//选择MF文件
char get_randm_data(void)
{
	char status = MI_ERR;
	unsigned char Send_Buf[] = {0x00,0x84,0x00,0x00,0x4};
	
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("get_randm_data-cpu_card_infor.recv_len:%d",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	printf("status:%d\r\n",status);
#endif
	memset(cpu_card_infor.random_data,0,8);
	memcpy(cpu_card_infor.random_data,cpu_card_infor.reccv_buf+2,4);
	return status;
}

char sned_secret_data_to_card(void)
{
	char status = MI_ERR;
	
	char key[8]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	unsigned char Send_Buf[13] = {0x00,0x82,0x00,0x00,0x8};
	char en_out_data[8] = {0};
	
#if defined(DEBUG_UART)
	printf("cpu_card_infor.random_data");
	for(unsigned char i=0;i<8;i++)
	{
		printf("0x%x ",cpu_card_infor.random_data[i]);
	}
	printf("\r\n");
#endif	
    en_des_run(en_out_data,cpu_card_infor.random_data,key);
	
	memcpy(Send_Buf+5,en_out_data,8);
	
	Pcd_SetTimer(500);
	status=Pcd_Cmd(Send_Buf, sizeof(Send_Buf), cpu_card_infor.reccv_buf,(u8 *)(&cpu_card_infor.recv_len));
#if defined(DEBUG_UART)
	printf("sned_secret_data_to_card-cpu_card_infor.recv_len:%d",cpu_card_infor.recv_len);
	for(unsigned char i=0;i<cpu_card_infor.recv_len;i++)
	{
		printf("0x%x ",cpu_card_infor.reccv_buf[i]);
	}
	printf("\r\n");
	
	printf("status:%d\r\n",status);
#endif
	return status;
}

char cpu_card_work_process(u8 work_process)
{
	char status = MI_ERR;
	u8 err_cnt = 0;
TRY_AGAIN:
	memset(cpu_card_infor.reccv_buf, 0x00, MAXRLEN); //清0
	cpu_card_infor.recv_len = 0;
	switch(work_process)
	{
		case CHOOSE_MF_FILE_MODE:
			status = choose_mf_file();
			break;
		case GET_RANDOM_DATA_MODE:
			status = get_randm_data();
			break;
		case EXTERNAL_CONFIRM_MODE:
			status = sned_secret_data_to_card();
			break;
		case CHOOSE_BINARY_FILE_MODE:
			status = choose_binary_file();
			break;
		case READ_BINARY_FILE_MODE:
			status =read_binary_file();
			break;
		case WRITE_BINARY_FILE_MODE:
			status =write_binary_file(cpu_card_infor.send_buf,16);
			break;
		case CREATE_BINARY_FILE:
			status =creat_binary_file();
			break;
			
		default:
			break;
	}
	if(status == MI_OK)
	{
		status = MI_ERR;
		if(cpu_card_infor.recv_len>5)
		{
			if((cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-4] == 0x90)&&(cpu_card_infor.reccv_buf[cpu_card_infor.recv_len-3] == 0x00))
			{
				err_cnt = 0;
				status = MI_OK;

			}
		}
	}
	if(status == MI_ERR)
	{
		err_cnt++;
		if(err_cnt<2)
			goto TRY_AGAIN;
	}
	return status;
}

//读二进制文件
char  cpu_card_read_process(void)
{
	char status = MI_ERR;
	u8 write_work_process = CHOOSE_BINARY_FILE_MODE;
	do
	{
		status = cpu_card_work_process(write_work_process);
		switch(write_work_process)
		{
			case CHOOSE_BINARY_FILE_MODE:
				write_work_process = READ_BINARY_FILE_MODE;
				break;
			case READ_BINARY_FILE_MODE:
				write_work_process = WORK_PROCESS_EXIT;
				break;
			case WORK_PROCESS_EXIT:
			default:
				break;
		}
	}while((status == MI_OK)&&(write_work_process != WORK_PROCESS_EXIT));
	return status;
}

//写二进制文件
char  cpu_card_write_process(void)
{
	char status = MI_ERR;
	u8 write_data[16] = {0x01,0x00,0x77,0x77,0x77,0x77,0,0,0,0,0,0};
	u8 write_work_process = CHOOSE_BINARY_FILE_MODE;
	do
	{
		status = cpu_card_work_process(write_work_process);
		switch(write_work_process)
		{
			case CHOOSE_BINARY_FILE_MODE:
				write_work_process = WRITE_BINARY_FILE_MODE;
				memcpy(cpu_card_infor.send_buf,write_data,16); 
				break;
			case WRITE_BINARY_FILE_MODE:
				write_work_process = WORK_PROCESS_EXIT;
				break;
			case WORK_PROCESS_EXIT:
			default:
				break;
		}
	}while((status == MI_OK)&&(write_work_process != WORK_PROCESS_EXIT));
	return status;
}

//CPU card 创建二进制文件
char  cpu_card_create_binary_file(void)
{
	char status = MI_ERR;
	u8 write_work_process = CREATE_BINARY_FILE;
	do
	{
		status = cpu_card_work_process(write_work_process);
		switch(write_work_process)
		{
			case CREATE_BINARY_FILE:
				write_work_process = WORK_PROCESS_EXIT;
				break;
			case WORK_PROCESS_EXIT:
			default:
				break;
		}
	}while((status == MI_OK)&&(write_work_process != WORK_PROCESS_EXIT));
	return status;
}


char update_card_info(uint8_t *card_data_infor_data)
{
	char status = MI_ERR;
	memcpy(cpu_card_infor.send_buf,card_data_infor_data,16); 
	status = cpu_card_work_process(WRITE_BINARY_FILE_MODE);
	memset(cpu_card_infor.send_buf,0,16);
	if(status == MI_OK)
		status = 1;
	else
		status = 0;
	return status;
}


//返回值： 1 正常  --- 0 异常
char nfc_lock_process(uint8_t *card_data_infor)
{
	char status = 0;
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
			printf("%d--%d ",device_name_info.card_ride.card_id[i],cpu_card_infor.card_snr[i]);
		printf("\r\n");
	#endif
		if(memcmp(device_name_info.card_ride.card_id,cpu_card_infor.card_snr,sizeof(cpu_card_infor.card_snr))  == 0)//卡号一致
		{
		
		#if defined(DEBUG_APP_NFC)
			printf("卡一样\r\n");
				printf("%d--%d\r\n",device_name_info.card_ride.delay_flag,device_name_info.card_ride.delay_time);
		#endif
		
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
					printf("money:%ld--run_time:%ld\r\n",money,device_name_info.card_ride.ride_time);
				#endif
				#if defined(TEST_CARD_DEBUG_FUNCV)
					money -= (device_name_info.card_ride.ride_time/6) *COST_MIN; 	//6s 1元
				#else
					money -= (device_name_info.card_ride.ride_time/3600) *COST_MIN;		//1h 1元
				#endif
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
						device_name_info.card_ride.delay_flag = true;
						buzzer_start(BUZZER_NORMAL);//蜂鸣器响
						//printf("关锁\r\n");
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
					deal_data[REG_LOCK_STATE] = NFC_CARD_LOCK_OPEN;
					if(update_card_info(deal_data))
					{
						status = 1;
						memcpy(card_data_infor,deal_data,16);
						memcpy(device_name_info.card_ride.card_id,cpu_card_infor.card_snr,4);
						
					#if defined(ADD_BT_OPEN_LOCK)
						device_name_info.ctrl_mode = CARD_OPEN;
					#endif
						open_key_ctrl();
					}
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

char cpu_card_pay_mode(void)
{
	char status = MI_ERR;
	unsigned char nfc_read_data[16];
	u8 card_work_process = CHOOSE_BINARY_FILE_MODE;
	do
	{
		status = cpu_card_work_process(card_work_process);
		switch(card_work_process)
		{
			case CHOOSE_BINARY_FILE_MODE:
				card_work_process = READ_BINARY_FILE_MODE;
				break;
			case READ_BINARY_FILE_MODE:
				if(status == MI_OK)
				{
					memcpy(nfc_read_data,cpu_card_infor.reccv_buf+2,16);
					status = nfc_lock_process(nfc_read_data);
					if(status)
						status = MI_OK;
					else
					{
						device_name_info.ctrl_delay = 7;
						buzzer_start(BUZZER_ERROR);//蜂鸣器响
						status = MI_ERR;
					}
					card_work_process = WORK_PROCESS_EXIT;
				}
				break;
			case WORK_PROCESS_EXIT:
			default:
				break;
		}
	}while((status == MI_OK)&&(card_work_process != WORK_PROCESS_EXIT));
	return status;
}


/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/
char find_judge_card(void)
{
	char status = MI_ERR;
	u8 write_work_process = CHOOSE_MF_FILE_MODE;
	status =find_card_mode();
	if(status == MI_OK)
		status = CardReset(cpu_card_infor.reccv_buf,(u8*)&cpu_card_infor.recv_len); 
	if(status != MI_OK)
		return status; 
	do
	{
		status = cpu_card_work_process(write_work_process);
		switch(write_work_process)
		{
			case CHOOSE_MF_FILE_MODE:
				write_work_process = GET_RANDOM_DATA_MODE;
				break;
			case GET_RANDOM_DATA_MODE:
				write_work_process = EXTERNAL_CONFIRM_MODE;
				break;
			case EXTERNAL_CONFIRM_MODE:
				write_work_process = WORK_PROCESS_EXIT;
				break;
			case WORK_PROCESS_EXIT:
			default:
				break;
		}
	}while((status == MI_OK)&&(write_work_process != WORK_PROCESS_EXIT));
	return status;
}

char nfc_work_mode(uint8_t nfc_mode,uint8_t *param)
{
	char status = MI_ERR;
	if(find_judge_card	() == MI_ERR)
		return MI_ERR;
	switch(nfc_mode)
	{
		case CPU_CARD_READ_MODE:
			status = cpu_card_read_process();
			if(status == MI_OK)
				cpu_card_infor.work_mode = CPU_CARD_EXIT;
			break;
		case CPU_CARD_WRITE_MODE:
			status = cpu_card_write_process();
			if(status == MI_OK)
				cpu_card_infor.work_mode = CPU_CARD_EXIT;
			break;
		case CPU_CARD_PAY_MODE:
			status = cpu_card_pay_mode();
			if(status == MI_OK)
				cpu_card_infor.work_mode = CPU_CARD_EXIT;
			break;
		case CPU_CARD_CREAT_MODE:
			status = cpu_card_create_binary_file();
			if(status == MI_OK)
				cpu_card_infor.work_mode = CPU_CARD_EXIT;
		case CPU_CARD_EXIT:
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
		||(device_name_info.card_ride.dect_delay_time)
		||(device_name_info.ctrl_delay))
		 nfc_set = 0;
	
		// nfc_set = 1;

	if(nfc_set)
	{
		nfc_timer_stop();
		nfc_work_repeat_init();
		if(nfc_work_mode(CPU_CARD_PAY_MODE,0) == MI_OK)
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

void nfc_card_data_init(void)
{
	memset(&cpu_card_infor,0,sizeof(cpu_card_infor));
	cpu_card_infor.work_mode = READ_BINARY_FILE_MODE;
}

void nfc_card_init(void)
{
#if defined(ADD_HOER_DETEC)
	hou_er_detect_init();
#endif
	PcdReset();
	PcdAntennaOff(); 
	PcdAntennaOn();  
	M500PcdConfigISOType( 'A' );
	nfc_timer_create();
}



