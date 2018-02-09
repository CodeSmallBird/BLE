#include"api_md5.h"


void RorDatas(unsigned char *input,unsigned char *Secret,unsigned char *output,unsigned char len)
{
	uint8_t i,j=0;
	uint8_t CharScret[4] = {'Z','H','Y','S'};
	for(i=0;i<len/4;i++)
	{
		for(j=0;j<4;j++)
		{
			output[i*4+j] = input[i*4+j]^Secret[j]^CharScret[j];
		}
	}
}


void encrypt_data_funcv(unsigned char *input,unsigned char *Secret,unsigned char *output,unsigned char len)
{
	//int i;	
	unsigned char md5_decrypt[16];		
	unsigned char RorData[16];
	
	MD5_CTX md5;  
	RorDatas(input,Secret,RorData,len);
	memcpy(input,RorData,len);
	MD5Init(&md5);
	MD5Update(&md5,RorData,len);  
	MD5Final(&md5,md5_decrypt); 
	memcpy(output,md5_decrypt,16);	
}

unsigned char   decrypt_data_funcv(unsigned char *input,unsigned char *Secret,unsigned char *output,unsigned char len)
{
	unsigned char md5_decrypt[16];	
	unsigned char RorData[16];
	MD5_CTX md5;  
	MD5Init(&md5);
	MD5Update(&md5,input,len);  
	MD5Final(&md5,md5_decrypt);
	if(memcmp(md5_decrypt,input+len,4) == 0)
	{
		RorDatas(input,Secret,RorData,len);
		memcpy(input,RorData,len);
		return 1;
	}
	return 0;
	
}



