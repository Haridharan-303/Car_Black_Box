#include <xc.h>
#include "main.h"

void __interrupt() isr(void)
{
    extern char sec;
    extern unsigned char return_time;
    static unsigned int count = 0;
    
    if (TMR2IF == 1)
    {
        // 1:16 prescale
        if (++count == 1250)
        {
            count = 0;
            if(sec > 0)
                sec--;
            else if(return_time > 0)
                return_time--;
        }
        
        TMR2IF = 0;
    }
}