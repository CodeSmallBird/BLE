#include "msp430x44x.h"
#include "TypeA.c"

void main( void )
{
  WDTCTL = WDTPW + WDTHOLD;

  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  MS430Init();
  
  while(1)
  {
     CardTypeA(SPEED_106);
  }
 
}
