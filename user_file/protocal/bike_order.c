#include "bike_order.h"

//�ն�ע��
void lock_register_app_order(void)
{
	cmd_send_pack(LOCK_REGISTER,(uint8_t *)&lock_register);
}

//�ն˼�Ȩ
void lock_authen_order(void)
{
	cmd_send_pack(LOCK_AUTHENTICATION,aunthen_code);
}

//λ����Ϣ�����㱨
void active_lock_position(void)
{
	cmd_send_pack(LOCK_POSTION_RESP,NULL);
}


V_FUNC_V ORDER_BIKE_TO_APP[]=
{
	lock_register_app_order,
	lock_authen_order,
	lock_authen_order,
	active_lock_position,
};


