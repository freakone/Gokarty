#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
 
#include "rfm70.h"  
 
int main()
{
    uint8_t test[]={'g', 'o', 'k', 'a','r', 't'};    
    initRFM(); //inicjalizacja RFM70
     
    setModeTX(); //tryb nadawania
    setChannel(8); // kanał 8
    setPower(3); // maksymalna moc (0: -10dBm | 1: -5dBm | 2: 0dBm | 3: 5dBm)
     
    while (1)
    {   
        sendPayload(test, 6, 0); //tablica, dlugosc, 0 - bez potwierdzenia | 1 - z potwierdzeniem
        _delay_ms(20);  
    }
}