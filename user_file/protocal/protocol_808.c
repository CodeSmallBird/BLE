#include"protocol_808.h"

//����У��

#define MSG_ID_HGIH  	0
#define MSG_ID_LOW  	1


#define MSG_NATURE_HIGH	2
#define MSG_NATURE_LOW	3

#define MSG_NUM_HIGH_BYTE	10	
#define MSG_NUM_LOW_BYTE	11	

#define HEAD_LEN			12	//���ְ���Ϣͷ�ĳ���
#define MSG_DATA_START			12// ���ְ�ʱ�����ݿ�ʼλ

#define LOCK_CTRL_BIT		1<<0	//���ſ���λ
#define BUZZER_CTRL_BIT		1<<1	//����������λ




SEND_DATA_INFOR	send_data;
RECV_DATA_INFOR recv_data;
DECRYP_DATA_INFOR decryp_data;
GENERAL_RESP_INFOR general_resp;
DEVICE_VER_INFOR device_ver;
LOCK_REGISTER_INFOR lock_register;
LOCK_POSITION_INFOR lock_position;
HEAD_MSG head_msg;
SEND_MSG_NUM send_msg_num;


char *apn_name;
char *ip_name;
#define TCP_PORT		80


uint8_t aunthen_code[AUTHEN_LENGTH] = {0};
uint8_t sim_iccid[10] = {0};
extern void lock_positon_resp(uint8_t *paramert);
extern void send_data_pack(uint8_t *send_data,uint8_t len);




void app_general_resp(uint8_t *paramert)
{
	
}

void lock_register_resp(uint8_t *paramert)	// ���ռ�Ȩ��Ϣ
{
	LOCK_REGISTER_REP *lock_register_temp;
	lock_register_temp =(LOCK_REGISTER_REP *)&(paramert[MSG_DATA_START]);
	if(!lock_register_temp->result)
	{
		memcpy(aunthen_code,lock_register_temp->authen_code,sizeof(lock_register_temp->authen_code));
	}
}

void set_lock_paramter(uint8_t *paramert)
{
	uint8_t num_param = paramert[0];		// ��������
	uint16_t param_id = 0;
	param_id = paramert[1]<<8|paramert[2];
	for(;num_param>0;num_param--)
	{
		switch(param_id)
		{
			case 0x0010:	// apn

				break;
			case 0x0013:	//����������ַ��IP������
			
				break;
			case 0x0018:	//������TCP�˿�
			
				break;
			case 0x0029:	//ȱʡʱ��㱨���
						
				break;

			case 0x0027:	//����ʱ�㱨ʱ����
						
				break;
		}
	}


}


void check_lock_paramter(uint8_t *paramert)
{
//	uint16_t msg_num = paramert[0]<<8 | paramert[1];
//	uint8_t num_param =paramert[2];



}



void app_ctrl_lock1(uint8_t *paramert)
{
//	uint16_t msg_num = (paramert[MSG_NUM_HIGH_BYTE]<<8 | paramert[MSG_NUM_LOW_BYTE]);
	if((paramert[MSG_DATA_START]&LOCK_CTRL_BIT))	//���ſ���
	{
		if(paramert[MSG_DATA_START+1])	//����
		{

		}
		else	//����
		{

		}
	}
	else if((paramert[MSG_DATA_START]&BUZZER_CTRL_BIT))	//����������
	{
		if(paramert[MSG_DATA_START+1])	//�򿪷�����
		{

		}
		else	//�رշ�����
		{

		}
	}
	// ��Ϣ�ظ�
	memset(&send_data,0,sizeof(send_data));
	
	send_data.data[MSG_ID_HGIH] = paramert[MSG_ID_HGIH];
	send_data.data[MSG_ID_LOW]  = paramert[MSG_ID_LOW];
	send_data.data[MSG_NUM_HIGH_BYTE] =	paramert[MSG_NUM_HIGH_BYTE];
	send_data.data[MSG_NUM_LOW_BYTE]  =	paramert[MSG_NUM_LOW_BYTE];
	send_data.len +=2;
	lock_positon_resp(paramert);						//��Ӹ���λ����Ϣ
	send_data.data[MSG_NUM_HIGH_BYTE] |= (((send_data.len-HEAD_LEN)>>8)&0x01);
	send_data.data[MSG_NUM_LOW_BYTE] = (send_data.len-HEAD_LEN);
	send_data_pack(send_data.data,send_data.len);		//��������
}

void app_ctrl_lock2(uint8_t *paramert)
{

}

void app_request_position(uint8_t *paramert)
{

}


void check_lock_attribute(uint8_t *paramert)
{

}

void app_update_lock(uint8_t *paramert)
{

}



/******************send_buff_parape*************/
void check_lock_param_resp(uint8_t *paramert)
{
	send_data.data[0] = lock_register.lock_info.mode;
	memcpy(send_data.data+1, lock_register.lock_info.manufacturer,5);
	memcpy(send_data.data+6, lock_register.lock_info.uuid,20);
	memcpy(send_data.data+26, lock_register.lock_info.mac,7);
	memcpy(send_data.data+33,sim_iccid,10);
	memcpy(send_data.data+43,sim_iccid,10);
	send_data.data[44] = HARD_WARE_LEN;
	memcpy(send_data.data+45,device_ver.hardware,HARD_WARE_LEN);
	send_data.data[45+HARD_WARE_LEN] = SOFT_WARE_LEN;
	memcpy(send_data.data+45+HARD_WARE_LEN+SOFT_WARE_LEN,device_ver.software,SOFT_WARE_LEN);
}

void lock_positon_resp(uint8_t *paramert)
{
	uint8_t len = 0;
	len = sizeof(lock_position);
	memcpy(send_data.data+send_data.len,&lock_position,len);
	send_data.len += len;
//������Ϣ��ӣ�--�����͸�����Ϣ
}

void lock_ctrl_resp(uint8_t *paramert)
{
	uint8_t len =0 ;
	len = sizeof(lock_position);
//	memcpy(send_data.data,Ӧ����ˮ��,2);
	memcpy(send_data.data+2,&lock_position,len);
	len += 2;
}

void check_lock_attribute_resp(uint8_t *paramert)
{
	send_data.data[0] = lock_register.lock_info.mode;
	memcpy(send_data.data+1, lock_register.lock_info.manufacturer,5);
	memcpy(send_data.data+6, lock_register.lock_info.uuid,20);
	memcpy(send_data.data+26, lock_register.lock_info.mac,7);
	memcpy(send_data.data+33,sim_iccid,10);
	memcpy(send_data.data+43,sim_iccid,10);
	send_data.data[44] = HARD_WARE_LEN;
	memcpy(send_data.data+45,device_ver.hardware,HARD_WARE_LEN);
	send_data.data[45+HARD_WARE_LEN] = SOFT_WARE_LEN;
	memcpy(send_data.data+45+HARD_WARE_LEN+SOFT_WARE_LEN,device_ver.hardware,SOFT_WARE_LEN);
}

void request_position_resp(uint8_t *paramert)
{
	uint8_t len =0;
//	memcpy(send_data.data,Ӧ����ˮ��,2);
	memcpy(send_data.data+2,&lock_position,len);
	len +=2;
}

//�������ݴ���
//*paramert--������Ϣ������ȥ����ͷβ��ʶ��
void recv_data_deal(uint8_t *paramert)	//��Ϣͷ+��ϢID
{
	uint16_t msg_id = ((paramert[0]<<8)|(paramert[1]));
	
	switch(msg_id)
	{
		case APP_GENERAL_RESP:
			app_general_resp(paramert);
			
			break;
		case LOCK_REGISTER_RESP:
			lock_register_resp(paramert);
			break;		
		case SET_LOCK_PARAMETER:
			set_lock_paramter(paramert);
			
			break;
		case CHECK_LOCK_PARAMETER:
			check_lock_paramter(paramert);

			break;
		case APP_CTRL_LOCK1:
			app_ctrl_lock1(paramert);
			
			break;
		
		case APP_CTRL_LOCK2:
			app_ctrl_lock2(paramert);

			break;
		
		case CHECK_LOCK_ATTRIBUTE:
			check_lock_attribute(paramert);
			break;
		
		case APP_REQUEST_POSITION:
			app_request_position(paramert);
			
			break;	
		
		case APP_UPDATE_LOCK:
			app_update_lock(paramert);
			
			break;		
		default:
			break;
	}

}

void api_data_send(uint8_t *send_data,uint8_t len)
{
	// ���÷��ͽӿ�

}

//���ݷ�װ����
void send_data_pack(uint8_t *send_data,uint8_t len)
{
	uint8_t *pack_data,*temp_data;
	uint8_t i = 0,check_data = 0,real_send_len = 0;
	pack_data = malloc(2*len);
	temp_data = pack_data;
	//pack_data[len] = 0;
	memset(pack_data,0,len+10);
	for(i=0;i<len;i++)
	{
		check_data ^= send_data[i];
	}
	
	temp_data[0] = 0x7e;
	real_send_len++;
	i=0;
	while(i < len)
	{
		if(send_data[i] == 0x7e)
		{
			*(++temp_data) = 0x7e;	
			*(++temp_data) = 0x02;
			real_send_len+=2;
		}
		else if(send_data[i] == 0x7d)
		{
			*(++temp_data) = 0x7e;	
			*(++temp_data) = 0x01;
			real_send_len+=2;
		}
		else
		{
			*(++temp_data) = send_data[i];	
			real_send_len++;
		}
		i++;
	}
	*(++temp_data) = check_data;
	*(++temp_data) = 0x7e;
	real_send_len+=2;
	api_data_send(pack_data,real_send_len);
	free(pack_data);
}


void cmd_send_pack(uint16_t cmd, uint8_t *paramert)
{
	uint8_t len = 0;
	head_msg.id = cmd;
	//memcpy(head_msg.bcd,bcdֵ,6);	//���BCD��
	head_msg.number++;
	len = sizeof(head_msg);
	switch(cmd)
	{
		case LOCK_GENERAL_RESP:

			break;

		case LOCK_HEARTBEAT	:
			
			break;
		case LOCK_REGISTER	:
			len -= 4;//û����Ϣ����װ��
			memcpy(send_data.data+len,paramert,sizeof(LOCK_REGISTER_INFOR));
			len += sizeof(LOCK_REGISTER_INFOR);
			head_msg.nature.all = 0;
			head_msg.nature.Member.rsa = 0;		// ������
			head_msg.nature.Member.length = sizeof(LOCK_REGISTER_INFOR);	// ��Ϣ�峤��
			memcpy(send_data.data,&head_msg,sizeof(head_msg)-4);			//copy ��ϢID
			send_msg_num.lock_register = head_msg.number;
			break;
	
		case LOCK_AUTHENTICATION:
			memcpy(send_data.data+len,paramert,AUTHEN_LENGTH);
			len += AUTHEN_LENGTH;
			break;


		case CHECK_LOCK_PARAMETER_RESP:
			
			break;
		case LOCK_POSTION_RESP:
			lock_positon_resp(paramert);
			break;
	
		case LOCK_CTRL_RESP	:
			lock_ctrl_resp(paramert);
		
			break;
	
		case LOCK_ATTRIBUTE_RESP:
			
			check_lock_attribute_resp(paramert);
			break;
		case APP_REQUEST_POSITION:
			
			break;
			
		case REQUEST_POSITION_RESP:
			request_position_resp(paramert);
			break;
	}
	send_data_pack(send_data.data,len);
}


/********************************���ݽ�ѹ��У��**************/

bool recv_data_check(DECRYP_DATA_INFOR decryp_data_infor)
{
	uint8_t i =0 ,check_data =0;
	for(;i<(decryp_data_infor.len-1);i++)
	{
		check_data += decryp_data_infor.data[i];
	}

	if(decryp_data_infor.data[decryp_data_infor.len-1] == check_data)
	{
		if(decryp_data_infor.data[MSG_NATURE_HIGH]&0x04)	//rsa����
		{
			//rsa ����
		}
		return true;
	}
	else
		return false;
}

//���ս�ѹ
bool recv_data_decryption(uint8_t *recv_data,uint8_t len)
{
	uint8_t i =1;
	if((recv_data[0] != 0x7e) ||(recv_data[len-1] != 0x7e))
		return false;
	while(i < (len-1))
	{
		if(recv_data[i] == 0x7e)
		{
			if(recv_data[i+1] == 0x01 )
			{
				*(decryp_data.data + decryp_data.len) = 0x7d;
				 decryp_data.len++;
			}
			else if(recv_data[i+1] == 0x02 )
			{
				*(decryp_data.data + decryp_data.len) = 0x7e;
				 decryp_data.len++;
			}
			i++;
		}
		else
		{
			*(decryp_data.data + decryp_data.len) = recv_data[i];
			decryp_data.len++;
		}
		i++;
	}
	if(recv_data_check(decryp_data))
		return true;
	else
		return false;
}

void deal_recv_data(void)
{
	recv_data_decryption((uint8_t *)&recv_data,recv_data.len);
	recv_data_deal(decryp_data.data);
}

void ProtocolInit(void)
{
	apn_name = malloc(APN_NAME_LEN);
	ip_name = malloc(IP_NAME_LEN);
	memset(&decryp_data,0,sizeof(decryp_data));
	memset(&recv_data,0,sizeof(recv_data));
	memset(&head_msg,0,sizeof(head_msg));
}


