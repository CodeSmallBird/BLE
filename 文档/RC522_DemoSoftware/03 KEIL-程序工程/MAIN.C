#include "reg52.h"
#include "main.h"
#include "mfrc522.h"	
#include <string.h>	
#include "LCD1602.h"
#include "STCEEPROM.h"
#include"KEY4X4.h"
#define uchar unsigned char
#define uint  unsigned int

sbit Speak = P1^4;
sbit RED = P1^5;
sbit GREEN = P1^6;
sbit Relay = P1^7;

sbit KEY5 = P1^3;

unsigned char idata RevBuffer[30];
unsigned char Card_Num;  		   
void iccardcode();
//系统初始化
void InitializeSystem()
{

     PcdReset();
     PcdAntennaOff(); 
     PcdAntennaOn();  
	 M500PcdConfigISOType( 'A' );

 
}
unsigned char idata UID[4],Temp[4];
unsigned char Table[3]; 
unsigned char Num;
unsigned char Count,Countf,Count1,Count2,Count3,Count4,Count5,C_flag;
void Auto_Reader(void)
{
    if(PcdRequest(0x52,Temp)==0)
    {
      if(PcdAnticoll(UID)==0)
      { 
	  	  	Speak=0;delay_10ms(20);Speak=1;


			Count =  UID[0];
			if((Count==Count1)||(Count==Count2)||(Count==Count3)||(Count==Count4)||(Count==Count5))
			{
				LCD1602_Disp_ZF(0x8a+0x40,"OK   ",5);
				GREEN = 0;Relay=0;delay_10ms(200);Relay=1;GREEN = 1;
				LCD1602_Disp_ZF(0x8a+0x40,"     ",5);
				LCD1602_Disp_ZF(0x84+0x40,"   ",3);

				C_flag=1;
				Table[0]=UID[0]/100+0x30;
				Table[1]=UID[0]/10%10+0x30;
				Table[2]=UID[0]%10+0x30;
				LCD1602_Disp_ZF(0x84+0x40,Table,3);
			}
			else
			{
				LCD1602_Disp_ZF(0x8a+0x40,"Error",5);
				RED = 0;delay_10ms(200);RED = 1;
				Speak=0;delay_10ms(5000);Speak=1;
				LCD1602_Disp_ZF(0x8a+0x40,"     ",5);
				LCD1602_Disp_ZF(0x84+0x40,"   ",3);
				
				C_flag=2;
			}

		}
	}
}
unsigned char Key_Value=0;
unsigned char KEY_t[6];
unsigned char idata MI_1,MI_2,MI_3,MI_4,MI_5,MI_6;
unsigned char idata SMI_1=0,SMI_2=0,SMI_3=0,SMI_4=0,SMI_5=0,SMI_6=0;
unsigned char MI_flag=0,MI_Num=0;
unsigned char MI_Mode=0;
void KEY_MAIN(void)
{
		P2 = 0xf0;
		if(P2 != 0xf0)
		{
			KEY4X4_Delay_1ms(15);	//按键消抖
			if(P2 != 0xf0)
			{	
				Key_Value = KEY4X4_Keyscan();
				switch(Key_Value)
				{
					case 15: Key_Value = 1;break;
					case 14: Key_Value = 2;break;
					case 13: Key_Value = 3;break;
					case 12: Key_Value = 11;break;

					case 11: Key_Value = 4;break;
					case 10: Key_Value = 5;break;
					case 9: Key_Value = 6;break;
					case 8: Key_Value = 12;break;

				   	case 7: Key_Value = 7;break;
					case 6: Key_Value = 8;break;
					case 5: Key_Value = 9;break;
					case 4: Key_Value = 13;break;

					case 3: Key_Value = 15;break;
					case 2: Key_Value = 0;break;
					case 1: Key_Value = 16;break;
					case 0: Key_Value = 14;break;
				}
				Speak=0;delay_10ms(10);Speak=1;
				////////////////////////////////
				if(MI_Mode==0)//输入密码模式
			   {
			   		if((MI_Num<6)&&(Key_Value<11))
					{
				   		MI_Num++;//输入第几个数字
					    switch(MI_Num)
						{
							case 1: MI_1= Key_Value;break;
							case 2: MI_2= Key_Value;break;
							case 3: MI_3= Key_Value;break;
							case 4: MI_4= Key_Value;break;
							case 5: MI_5= Key_Value;break;
							case 6: MI_6= Key_Value;break;
						}
						KEY_t[MI_Num-1]=Key_Value+0x30;
						LCD1602_Disp_ZF(0x87,KEY_t,MI_Num);
					}
					if(Key_Value==14)//删除
					{
						if(MI_Num==0)MI_Num=1;
						MI_Num--;
						LCD1602_Disp_ZF(0x87,"      ",6);
						LCD1602_Disp_ZF(0x87,KEY_t,MI_Num);	
					}
					if(Key_Value==12)//修改密码
					{
						if(MI_flag==1)//开锁成功才可以修改密码
						{
							MI_Mode=1;
							MI_Num=0;
							LCD1602_Disp_ZF(0x80,"Set-1:          ",16);
							LCD1602_Disp_ZF(0x87,"      ",6);
						}
						
					}
					if(Key_Value==15)//清除密码，000000
					{

							SMI_1=0;SMI_2=0;SMI_3=0;SMI_4=0;SMI_5=0;SMI_6=0;
							LCD1602_Disp_ZF(0x80,"     Set OK     ",16);
							GREEN = 0;delay_10ms(200);GREEN = 1;
							LCD1602_Disp_ZF(0x80,"Input:          ",16);

							MI_Num=0;
							MI_Mode=0;	
							ISP_ERASE(0x2c00);		//注意：字节编程时必须要先要擦除整个扇区
							ISP_PROGRAM(0x2c00, Countf);
							ISP_PROGRAM(0x2c01, Count1);
							ISP_PROGRAM(0x2c02, Count2);
							ISP_PROGRAM(0x2c03, Count3);
							ISP_PROGRAM(0x2c04, Count4);
							ISP_PROGRAM(0x2c05, Count5);
							ISP_PROGRAM(0x2c10, SMI_1);
							ISP_PROGRAM(0x2c11, SMI_2);
							ISP_PROGRAM(0x2c12, SMI_3);
							ISP_PROGRAM(0x2c13, SMI_4);
							ISP_PROGRAM(0x2c14, SMI_5);
							ISP_PROGRAM(0x2c15, SMI_6);
						
					}
					if(Key_Value==11)//确定
					{
						if((MI_1==SMI_1)&&(MI_2==SMI_2)&&(MI_3==SMI_3)&&(MI_4==SMI_4)&&(MI_5==SMI_5)&&(MI_6==SMI_6))
						{//密码正确，开门
							LCD1602_Disp_ZF(0x8e,"OK",2);
							GREEN = 0;Relay=0;delay_10ms(200);Relay=1;GREEN = 1;
							LCD1602_Disp_ZF(0x87,"      ",6);
							LCD1602_Disp_ZF(0x8e,"  ",2);
							Speak=0;delay_10ms(20);Speak=1;
							MI_Num=0;
							MI_flag=1;		
						}
						else //密码不对
						{
							LCD1602_Disp_ZF(0x8e,"Er",2);
							RED = 0;delay_10ms(200);RED = 1;
							Speak=0;delay_10ms(200);Speak=1;
							LCD1602_Disp_ZF(0x87,"      ",6);
							LCD1602_Disp_ZF(0x8e,"  ",2);
							MI_flag=0;
							MI_Num=0;	
						}
					}
			   }
				   	//LCD1602_Disp_ZF(0x80,"Set-1:          ",16);
	//LCD1602_Disp_ZF(0x80,"     Set OK     ",16);
			   if(MI_Mode==1)//修改密码模式
			   {
			   		if((MI_Num<6)&&(Key_Value<11))
					{
				   		MI_Num++;//输入第几个数字
					    switch(MI_Num)
						{
							case 1: SMI_1= Key_Value;break;
							case 2: SMI_2= Key_Value;break;
							case 3: SMI_3= Key_Value;break;
							case 4: SMI_4= Key_Value;break;
							case 5: SMI_5= Key_Value;break;
							case 6: SMI_6= Key_Value;break;
						}
						KEY_t[MI_Num-1]=Key_Value+0x30;
						LCD1602_Disp_ZF(0x87,KEY_t,MI_Num);
					}
					if(Key_Value==14)//删除
					{
						if(MI_Num==0)MI_Num=1;
						MI_Num--;
						LCD1602_Disp_ZF(0x87,"      ",6);
						LCD1602_Disp_ZF(0x87,KEY_t,MI_Num);	
					}
					if(Key_Value==11)//确定
					{
							LCD1602_Disp_ZF(0x80,"     Set OK     ",16);
							GREEN = 0;delay_10ms(200);GREEN = 1;
							LCD1602_Disp_ZF(0x80,"Input:          ",16);

							MI_Num=0;
							MI_Mode=0;
							ISP_ERASE(0x2c00);		//注意：字节编程时必须要先要擦除整个扇区
							ISP_PROGRAM(0x2c00, Countf);
							ISP_PROGRAM(0x2c01, Count1);
							ISP_PROGRAM(0x2c02, Count2);
							ISP_PROGRAM(0x2c03, Count3);
							ISP_PROGRAM(0x2c04, Count4);
							ISP_PROGRAM(0x2c05, Count5);
							ISP_PROGRAM(0x2c10, SMI_1);
							ISP_PROGRAM(0x2c11, SMI_2);
							ISP_PROGRAM(0x2c12, SMI_3);
							ISP_PROGRAM(0x2c13, SMI_4);
							ISP_PROGRAM(0x2c14, SMI_5);
							ISP_PROGRAM(0x2c15, SMI_6);		
					
					}
			   }
				 ///////////////////////////////////////////////////
				delay_10ms(20);
				//////////////////////////////////////
		     }
	   }

	   ////////////////////////////////////
}
void main(void)
{     
	InitializeSystem();
	LCD1602_init();
	LCD1602_Disp_ZF(0x80+0x40,"Num:",4);
	LCD1602_Disp_ZF(0x80,"Input:          ",16);
	//LCD1602_Disp_ZF(0x80,"Set-1:          ",16);
	//LCD1602_Disp_ZF(0x80,"Set-2:          ",16);
	//LCD1602_Disp_ZF(0x80,"     Set OK     ",16);

	Countf=ISP_READ(0x2c00);		
	Count1=ISP_READ(0x2c01);
	Count2=ISP_READ(0x2c02);
	Count3=ISP_READ(0x2c03);
	Count4=ISP_READ(0x2c04);
	Count5=ISP_READ(0x2c05);

	SMI_1=ISP_READ(0x2c10);		
	SMI_2=ISP_READ(0x2c11);
	SMI_3=ISP_READ(0x2c12);
	SMI_4=ISP_READ(0x2c13);
	SMI_5=ISP_READ(0x2c14);
	SMI_6=ISP_READ(0x2c15);
	while (1)
	{	
		////////////////////////////////
		iccardcode();
		Auto_Reader();
		////////////////////////////////

		KEY_MAIN();
		if(!KEY5)
		{
			delay_10ms(50);
			if(!KEY5)
			{
				if(C_flag==2)
				{
					LCD1602_Disp_ZF(0x8a+0x40,"OK   ",5);
					delay_10ms(100);
					Countf++;
					if(Countf==6)Countf=1;
					if(Countf==1) Count1 = Count;
					if(Countf==2) Count2 = Count;
					if(Countf==3) Count3 = Count;
					if(Countf==4) Count4 = Count;
					if(Countf==5) Count5 = Count;
					LCD1602_Disp_ZF(0x8a+0x40,"     ",5);

				}
				if(C_flag==1)
				{
					LCD1602_Disp_ZF(0x8a+0x40,"Error",5);
					delay_10ms(100);
					if(Count==Count1)Count1=0;
					if(Count==Count2)Count2=0;
					if(Count==Count3)Count3=0;
					if(Count==Count4)Count4=0;
					if(Count==Count5)Count5=0;
					LCD1602_Disp_ZF(0x8a+0x40,"     ",5);

				}
				Speak=0;delay_10ms(20);Speak=1;
				ISP_ERASE(0x2c00);		//注意：字节编程时必须要先要擦除整个扇区
				ISP_PROGRAM(0x2c00, Countf);
				ISP_PROGRAM(0x2c01, Count1);
				ISP_PROGRAM(0x2c02, Count2);
				ISP_PROGRAM(0x2c03, Count3);
				ISP_PROGRAM(0x2c04, Count4);
				ISP_PROGRAM(0x2c05, Count5);
				ISP_PROGRAM(0x2c10, SMI_1);
				ISP_PROGRAM(0x2c11, SMI_2);
				ISP_PROGRAM(0x2c12, SMI_3);
				ISP_PROGRAM(0x2c13, SMI_4);
				ISP_PROGRAM(0x2c14, SMI_5);
				ISP_PROGRAM(0x2c15, SMI_6);
				C_flag =0;
				while(!KEY5);
			}	
		}
		//////////////////////
	}
}
void iccardcode()
{	     
  	unsigned char cmd;
	unsigned char status;

	
	cmd = RevBuffer[0];
	switch(cmd)
 	{
		case 1:     // Halt the card     //终止卡的操作
			status= PcdHalt();;			
			RevBuffer[0]=1;
			RevBuffer[1]=status;
			break;			
		case 2:     // Request,Anticoll,Select,return CardType(2 bytes)+CardSerialNo(4 bytes)
			        // 寻卡，防冲突，选择卡    返回卡类型（2 bytes）+ 卡系列号(4 bytes)
			status= PcdRequest(RevBuffer[1],&RevBuffer[2]);
			if(status!=0)
			{
				status= PcdRequest(RevBuffer[1],&RevBuffer[2]);
				if(status!=0)				
				{
					RevBuffer[0]=1;	
					RevBuffer[1]=status;
					break;
				}
			}  
			RevBuffer[0]=3;	
			RevBuffer[1]=status;
			break;
			
		case 3:                         // 防冲突 读卡的系列号 MLastSelectedSnr
			status = PcdAnticoll(&RevBuffer[2]);
			if(status!=0)
			{
				RevBuffer[0]=1;	
				RevBuffer[1]=status;
				break;
			}
			//memcpy(MLastSelectedSnr,&RevBuffer[2],4);
			RevBuffer[0]=5;
			RevBuffer[1]=status;
			break;	
		case 4:		                    // 选择卡 Select Card
			//status=PcdSelect(MLastSelectedSnr);
			if(status!=MI_OK)
			{
				RevBuffer[0]=1;	
				RevBuffer[1]=status;
				break;
			}
			RevBuffer[0]=3;
			RevBuffer[1]=status;			
			break;
		case 5:	    // Key loading into the MF RC500's EEPROM
            //status = PcdAuthState(RevBuffer[1], RevBuffer[3], DefaultKey, MLastSelectedSnr);// 校验卡密码
			RevBuffer[0]=1;
			RevBuffer[1]=status;			
			break;							
		case 6: 
			RevBuffer[0]=1;
			RevBuffer[1]=status;			
			break;				
		case 7:     
    		RevBuffer[0]=1;
			RevBuffer[1]=status;			
			break;
		case 8:     // Read the mifare card
		            // 读卡
			status=PcdRead(RevBuffer[1],&RevBuffer[2]);
			if(status==0)
			{RevBuffer[0]=17;}
			else
			{RevBuffer[0]=1;}
			RevBuffer[1]=status;			
			break;
		case 9:     // Write the mifare card
		            // 写卡  下载密码
			status=PcdWrite(RevBuffer[1],&RevBuffer[2]);
			RevBuffer[0]=1;
			RevBuffer[1]=status;			
			break;
		case 10:
            PcdValue(RevBuffer[1],RevBuffer[2],&RevBuffer[3]);
			RevBuffer[0]=1;	
			RevBuffer[1]=status;
			break;
		case 12:    // 参数设置
		    PcdBakValue(RevBuffer[1], RevBuffer[2]);
			RevBuffer[0]=1;	//contact
			RevBuffer[1]=0;
			break;		
	}
}
