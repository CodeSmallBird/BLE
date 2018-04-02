#ifndef _API_CARD_TYPE_H_
#define _API_CARD_TYPE_H_

//#define SUPPORT_CPU_CARD_OPEN_LOCK	//Ö§³ÖCPU¿¨¿ªËø



#if defined(SUPPORT_CPU_CARD_OPEN_LOCK)
	#include ".\cpu_card\api_cpu_card.h"
#else
	#include ".\ordinary_card\api_ordinary_card.h"
#endif



#endif

