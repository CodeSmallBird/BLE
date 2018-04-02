#ifndef _DES_H_
#define _DES_H_

#include "ble_service.h"

//extern void destest(void);

//extern bool   DES_Encrypt ( char * lpSour, char * lpDest, unsigned char  uLen, char * lpKey );  

extern void en_des_run(char Out[8], char In[8],char key[8] );



#endif

