#ifndef _API_MD5_H_
#define _API_MD5_H_
#include"md5.h"
#include "ble_service.h"


extern void encrypt_data_funcv(unsigned char *input,unsigned char *Secret,unsigned char *output,unsigned char len);
extern unsigned char decrypt_data_funcv(unsigned char *input,unsigned char *Secret,unsigned char *output,unsigned char len);


#endif

