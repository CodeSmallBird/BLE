#include"api_card_type.h"

#if defined(SUPPORT_CPU_CARD_OPEN_LOCK)
	#include ".\cpu_card\api_cpu_card.c"
#else
	#include ".\ordinary_card\api_ordinary_card.c"
#endif


