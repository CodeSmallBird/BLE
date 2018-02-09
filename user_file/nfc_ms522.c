#include"nfc_ms522.h"

#define DATA_LEN		100
uint8_t read_nfc_data[DATA_LEN];
uint8_t send_nfc_data_save[DATA_LEN];



static app_timer_id_t   				m_nfc_timeout_id;
#define NFC_ON_MEAS_INTERVAL            APP_TIMER_TICKS(50, 0)

uint8_t get_nfc_ver(void)
{
	uint8_t read_data = 0;
	read_data = ReadRawRC(0x37);
	printf("nfc_data:%d\r\n",read_data);
	return read_data;
}


uint8_t get_irq_status(void)
{
	uint8_t read_data = 0;
	read_data = ReadRawRC(0x07);
	//ie_printf("irq_status:%d\r\n",read_data);
	read_data = ReadRawRC(0x08);
	//ie_printf("sys_status:%d\r\n",read_data);
	return read_data;
}

uint8_t get_error_status(void)
{
	uint8_t read_data = 0;
	read_data = ReadRawRC(0x06);
	//ie_printf("error_status:%d\r\n",read_data);

	read_data = ReadRawRC(0x0d);
	//ie_printf("bit_fram:%d\r\n",read_data);

	read_data = ReadRawRC(0x14);
	//ie_printf("Tx_Control:%d\r\n",read_data);
	return read_data;
}
extern Receive_Cmd receivecmd;


void nfc_card_work_set(uint8_t nfc_set)
{
#if 1
	if(nfc_set)
	{
		if(g_lock_flag == 1)
			return;
		//printf("nfc_start\r\d");
		NRSTPD_ON;
		nrf_delay_ms(1);
		if(nfc_work_repeat())
		{
			open_key_ctrl();
		}
		NRSTPD_OFF;
		//printf("nfc_end\r\d");

	}
	else
	{
		NRSTPD_OFF;
	}
#else
	NRSTPD_OFF;
#endif
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

void nrc_timeout_handler(void * p_context)
{
	static uint8_t run_flag = 0;
	static uint8_t nfc_run_cnt = 0;

	nfc_run_cnt++;
	if(nfc_run_cnt % 20 == 0)
	{
		run_flag = 1;
	}
	else
	{
		run_flag  = 0;
	}
	nfc_card_work_set(run_flag);
}
void nfc_timer_create(void)
{
	uint32_t err_code;
	err_code = app_timer_create(&m_nfc_timeout_id,APP_TIMER_MODE_REPEATED,nrc_timeout_handler);
	APP_ERROR_CHECK(err_code);
	nfc_timer_start();
}


void nfc_card_io_init(void)
{
	SpiInit();
	NRSTPD_INIT;
	nfc_card_work_set(1);
	SpiReset();
}


void nfc_card_send(uint8_t *send_data,uint8_t len)
{
	uint8_t i;
	WriteRawRC(0x0a,0x80);
	WriteRawRC(0x12,0x00);             // no crc
	WriteRawRC(0x13,0x00);             // no crc
	for(i=0;i<len;i++)
	{
		WriteRawRC(0x09,send_data[i]);
	}
	WriteRawRC(0x01,0x0C);          //trans and receive
	WriteRawRC(0x0D,0x87);          //start send
}

bool read_card_id(void)
{
	uint8_t len = 0;

	WriteRawRC(0x0a,0x80);	// clear_buff

	WriteRawRC(0x09,0x93);          //wirte command
	WriteRawRC(0x09,0x20);

	WriteRawRC(0x01,0x0C);          //trans and receive
	WriteRawRC(0x0D,0x80);          //start send
	nrf_delay_us(1000);
	WriteRawRC(0x0e,0x20);		// ???
	len = ReadRawRC(0x0a); 	//num
	if(len == 0)
		return false;
	for(int i=0;i<len;i++)
	{
		read_nfc_data[i] = ReadRawRC(0x09); // read data
		//ie_printf(" read_nfc_data[%d]:%x\r\n",i,read_nfc_data[i]);
	}
	memcpy(device_name_info.card_ride.card_id,read_nfc_data,4);
	device_name_info.lock_state = REQUEST_OPEN_LOCK;
	return true;
}

bool card_requester(void)
{
	/*****************REQA************************/
	uint8_t len;
	WriteRawRC(0x12,0x00);          //TX CRC
	WriteRawRC(0x13,0x00);          //RX CRC

	WriteRawRC(0x0a,0x80);
	WriteRawRC(0x0D,0x07);          //

	WriteRawRC(0x09,0x26);          //wirte data

	//  SpiRxData = ReadRawRC(0x0a);	// num
	//  SpiRxData = ReadRawRC(0x01);
	len = ReadRawRC(0x01);

	WriteRawRC(0x01,0x0C);          //trans and receive
	WriteRawRC(0x0D,0x87);          //start send
	nrf_delay_us(1000);
	len = ReadRawRC(0x0a); 	//num
	if(len>0)
	{
		printf("len--%d\r\n",len);
		for(int i=0;i<len;i++)
		read_nfc_data[i] = ReadRawRC(0x09);	// read data
		return true;
	}
	else
		return false;
}



bool nfc_recv_start(void)
{
	//get_nfc_ver();

	if(card_requester())
	{
		if(read_card_id())
			return true;
	}
	return false;
}

void nfc_card_init(void)
{
	nfc_card_io_init();
//	WriteRawRC(0x01,0x0f);		// ¸´Î»
	nrf_delay_ms(500);

	WriteRawRC(0x18,0x85);
	WriteRawRC(0x19,0x4D);
	WriteRawRC(0x26,0x79);

	WriteRawRC(0x28,0x3F);
	WriteRawRC(0x29,0x03);

	WriteRawRC(0x0C,0x10);          //MS512

	WriteRawRC(0x12,SPEED_106);
	WriteRawRC(0x13,SPEED_106);
	WriteRawRC(0x14,0x83);
	WriteRawRC(0x15,0x40);          //Type A
	nfc_timer_create();

}


bool nfc_work_repeat(void)
{
	WriteRawRC(0x18,0x85);
	WriteRawRC(0x19,0x4D);
	WriteRawRC(0x26,0x79);

	WriteRawRC(0x28,0x3F);
	WriteRawRC(0x29,0x03);

	WriteRawRC(0x0C,0x10);		  //MS512

	WriteRawRC(0x12,SPEED_106);
	WriteRawRC(0x13,SPEED_106);
	WriteRawRC(0x14,0x83);
	WriteRawRC(0x15,0x40);		  //Type A

	if(nfc_recv_start())
		return true;
	else
		return false;
}



