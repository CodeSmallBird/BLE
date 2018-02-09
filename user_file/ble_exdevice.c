/***************************************************
******内外部RTC、gsensor外部设备 *******
****************************************************/
#include "ble_exdevice.h"
#include "twi_master.h"
#include  "nrf_delay.h"
#include "ble_service.h"
#include "math.h"
/////////////////////////////////////////////

uint32_t						                  m_last_tickcount					  = 0;	
uint32_t						                  m_ltmin_tickcount				  = 0;		//记录上一次的CONTER数值，计算RTC时间
uint32_t							              m_total_tickcount					  = 0; 	
ble_system_clock_t                    m_clock                           		  =  {17,7,17,18,38,00};        
//ble_system_clock_t                    m_correct_clock                    =  {17,7,17,18,38,00};  
//ble_system_clock_t  						open_clock;							//  记录开锁时间
//ble_system_clock_t 						close_clock;								//  记录关锁时间
bool   m_timer_battery                    = false;      	  							//	gesensor
///////////////////////////////////////////////
/*
void syn_local_timer(ble_system_clock_t local_clock)
{
	m_clock.year   = local_clock.year;
	m_clock.month  = local_clock.month;
	m_clock.day    = local_clock.day;
	m_clock.hour   = local_clock.hour;
	m_clock.minute = local_clock.minute;
	m_clock.second = local_clock.second;

	m_correct_clock = m_clock;
	m_correct_clock.tnum = m_correct_clock.second;
	m_ltmin_tickcount = NRF_RTC0->COUNTER;
	m_last_tickcount = m_ltmin_tickcount;
	m_total_tickcount = 0;
}

*/


/**********************BMA250*******************************/
#if defined(_SUPPORT_BM250_)

#define BMA250_ADDR_WR 															0x30
#define BMA250_ADDR_RD																(BMA250_ADDR_WR+1)


#define MOTION_SENSOR_CHIP_ID         		 								0x00 

#define MOTION_SENSOR_ACC_X_LSB         								0x02  
#define MOTION_SENSOR_ACC_X_MSB        								0x03 
#define MOTION_SENSOR_ACC_Y_LSB         								0x04  
#define MOTION_SENSOR_ACC_Y_MSB        								0x05 
#define MOTION_SENSOR_ACC_Z_LSB         								0x06  
#define MOTION_SENSOR_ACC_Z_MSB        								0x07 

#define MOTION_SENSOR_INT_STATUS0      								0x09  
#define MOTION_SENSOR_INT_STATUS1     								0x0A 
#define MOTION_SENSOR_INT_STATUS2     								0x0B 
#define MOTION_SENSOR_INT_STATUS3      								0x0C 
#define MOTION_SENSOR_FIFO_STATUS     								0x0E 
#define MOTION_SENSOR_PMU_RANGE										0x0F 

//0x03 :+-2G    0x05: -+4G   0x08: +-8G  0x0c :  +-16G


#define MOTION_SENSOR_PMU_BW           									0x10 
//  000 00XXX    000 01000    000 01001   000 01010    000 01011  000 01100
//    7.81               7.81           15.63          31.25           62.5         125

//  000 01101    000 01110    000 01111   000 1XXXX    
//   250              500           1000          1000          

#define MOTION_SENSOR_PMU_LPW            								0x11
//      0              0                    0                    0     0    0     0                0
// suspend     low_power_en    deep_suspend     sleep_dur<3:0>            reserved
//Bit7-Bit5    000:NORMAL    001:DEEP_SUSPEND   010 : LOW_POWER    100:SUSPEND
//
//Bit4-Bit1    0000--0101  0.5ms    

#define MOTION_SENSOR_LOW_NOISE          								0x12
// BIT6:lowpower_mode                  BIT5:sleeptimer_mode 
//

#define MOTION_SENSOR_ACCD_HBW           								0x13
//BIT7:data_high_bw    BIT6 : shadow_dis

#define MOTION_SENSOR_BGW_SOFTRESET      						0x14  //0xB6 triggers a reset

#define MOTION_SENSOR_INT_EN_0           									0x16
#define MOTION_SENSOR_INT_EN_1           									0x17  
#define MOTION_SENSOR_INT_EN_2           									0x18

#define MOTION_SENSOR_INT_MAP_0         			 					0x19
#define MOTION_SENSOR_INT_MAP_1         								0x1A
#define MOTION_SENSOR_INT_MAP_2          								0x1B

#define MOTION_SENSOR_INT_SRC           		 							0x1E
#define MOTION_SENSOR_INT_OUT_CTRL       							0x1F
#define MOTION_SENSOR_INT_RST_LATCH      							0x21

#define MOTION_SENSOR_INT_0              										0x22
#define MOTION_SENSOR_INT_1              										0x23
#define MOTION_SENSOR_INT_2              										0x24
#define MOTION_SENSOR_INT_3             	 									0x25 
#define MOTION_SENSOR_INT_4              										0x26
#define MOTION_SENSOR_INT_5              										0x27
#define MOTION_SENSOR_INT_6              										0x28 
#define MOTION_SENSOR_INT_7													0x29  
#define MOTION_SENSOR_INT_8              										0x2A
#define MOTION_SENSOR_INT_9              										0x2B
#define MOTION_SENSOR_INT_A              										0x2C
#define MOTION_SENSOR_INT_B              										0x2D
#define MOTION_SENSOR_INT_C              										0x2E 
#define MOTION_SENSOR_INT_D              										0x2F 
                                        

#define MOTION_SENSOR_FIFO_CONFIG_0      							0x30
#define MOTION_SENSOR_PMU_SELF_TEST      						0x32 
#define MOTION_SENSOR_TRIM_NVM_CTRL      						0x33 
#define MOTION_SENSOR_BGW_SPI_WDT        							0x34 

#define MOTION_SENSOR_OFC_CTRL           								0x36 
#define MOTION_SENSOR_OFC_SETTING        							0x37
#define MOTION_SENSOR_OFC_OFFSET_X       							0x38
#define MOTION_SENSOR_OFC_OFFSET_Y       							0x39
#define MOTION_SENSOR_OFC_OFFSET_Z       							0x3A 

#define MOTION_SENSOR_TRIM_GP0           								0x3B
#define MOTION_SENSOR_TRIM_GP1           								0x3C

#define MOTION_SENSOR_FIFO_CONFIG_1      							0x3E
#define MOTION_SENSOR_FIFO_DATA          								0x3F






void TWI_Init(void)
{
	twi_master_init();
}

bool bma250_IICWrite(uint8_t RegAdd, uint8_t Data) 
{  
		uint8_t w2_data[2];
		w2_data[0] = RegAdd;
		w2_data[1] = Data;
		return twi_master_transfer(BMA250_ADDR_WR, w2_data, 2, TWI_ISSUE_STOP);
}


void BMA250_Init(void)
{	
	bma250_IICWrite(MOTION_SENSOR_PMU_LPW,0x00 );
	bma250_IICWrite(MOTION_SENSOR_PMU_RANGE,0x08 );//0x03:+-2G  0x05:+-4G ,0x08 :+-8G  0x0C:+-16G
	bma250_IICWrite(MOTION_SENSOR_PMU_BW,0x0D );
	bma250_IICWrite(MOTION_SENSOR_ACCD_HBW,0xFF);
	nrf_delay_ms(1);
	bma250_IICWrite(MOTION_SENSOR_PMU_LPW,0x00 );
	bma250_IICWrite(MOTION_SENSOR_PMU_RANGE,0x08 );//0x03:+-2G  0x05:+-4G ,0x08 :+-8G  0x0C:+-16G
	bma250_IICWrite(MOTION_SENSOR_PMU_BW,0x0C );
	bma250_IICWrite(MOTION_SENSOR_ACCD_HBW,0xFF);
}


uint8_t BMA250_IICRead(uint8_t RegAdd)
{            
	uint8_t value;
	bool transfer_succeeded;
	transfer_succeeded = twi_master_transfer(BMA250_ADDR_WR, &RegAdd, 1, TWI_DONT_ISSUE_STOP);	//write
	transfer_succeeded &=twi_master_transfer(BMA250_ADDR_RD, &value, 1, TWI_ISSUE_STOP);	//read
	return value;
}


uint8_t  BMA250_ReadID(void)
{
	uint8_t reg ;
	reg = BMA250_IICRead(MOTION_SENSOR_CHIP_ID);
	return reg;
}

uint8_t ChunHui_X = 0;
uint8_t ChunHui_Y= 0;
uint8_t ChunHui_Z = 0;
uint8_t Pre_bma_X = 0;
uint8_t Pre_bma_Y= 0;
uint8_t Pre_bma_Z = 0;

void BMA250_Read_XYZ(void)
{
	ChunHui_X = (BMA250_IICRead(MOTION_SENSOR_ACC_X_MSB)&0xFF);
	ChunHui_Y = (BMA250_IICRead(MOTION_SENSOR_ACC_Y_MSB)&0xFF);
	ChunHui_Z = (BMA250_IICRead(MOTION_SENSOR_ACC_Z_MSB)&0xFF);
//#if dbg
//	//printf("bma250 :%d,%d,%d\r\n",ChunHui_X,ChunHui_Y,ChunHui_Z);
//#endif
}
static uint8_t jianfa(uint8_t x,uint8_t y)
{
	if(x>y)
		return (x-y);
	else
		return (y-x);
}
bool bma250_get_vibrate_status(void)
{
	unsigned int ret_data;
	int x,y,z;
	unsigned int tmp = 0;
	int i = 0;
	Pre_bma_X = ChunHui_X;
	Pre_bma_Y = ChunHui_Y;
	Pre_bma_Z = ChunHui_Z;
	for(i=0;i<15;i++)
	{
		BMA250_Read_XYZ();
	}
#if 1
	x = jianfa(Pre_bma_X,ChunHui_X);
	y = jianfa(Pre_bma_Y,ChunHui_Y);
	z = jianfa(Pre_bma_Z,ChunHui_Z);
	tmp = x*x+y*y+z*z;
	ret_data = (unsigned int)sqrt((double)(x*x+y*y+z*z));
#else
	ret_data = abs(Pre_bma_X+Pre_bma_Y+Pre_bma_Z-ChunHui_X-ChunHui_Y-ChunHui_Z);
#endif
#if dbg
	//printf("x:%d,y:%d,z:%d,%d\r\n",x,y,z,ret_data);
#endif

	if(ret_data>8)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool bma250_get_vibrate_status2(void)
{
	int ret_data;
	Pre_bma_X = ChunHui_X;
	Pre_bma_Y = ChunHui_Y;
	Pre_bma_Z = ChunHui_Z;
	BMA250_Read_XYZ();
	ret_data = abs(Pre_bma_X+Pre_bma_Y+Pre_bma_Z-ChunHui_X-ChunHui_Y-ChunHui_Z);
	if(ret_data>=10)	//8
	{
		return true;
	}
	else
	{
		
		return false;
	}
	
	
}


#endif
