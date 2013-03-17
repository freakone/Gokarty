#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
 
#include "rfm70.h"  
 
int main()
{
    uint8_t bufor[32];  
    initRFM(); //inicjalizacja RFM70
     
    setModeRX(); //tryb odbioru
    setChannel(8); // kanał 8
     
    while (1)
    {   
        if (int len = receivePayload(bufor)) // odbieramy dane i przesyłamy je dalej za pomocą uarta
        {
			for(int i = 0; i < len; i++)
				uart_put((char)bufor[i]);
            uart_puts("\n");
        }   
    }
}