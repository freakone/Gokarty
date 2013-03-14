#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


typedef unsigned char  u8;
typedef  unsigned int  uint;

void led_on()
{ 
	TCCR1A = (1 << COM1A0); //oc1a toggle on compare match
	TCRR1B = (1 << WGM12) | (1 << CS10); //ctc, no prescaler
	TCCR1C = (1 << FOC1A); // force output compare
}

void led_off()
{
	TCCR1A = (1 << COM1A1); //oc1a clear on compate match
	TCRR1B = (1 << WGM12) | (1 << CS10); //ctc, no prescaler
	TCCR1C = (1 << FOC1A); // force output compare
}

void timer_clear()
{
	TCCR1A = 0;
	TCCR1B = 0;
}


 void send_cmd(uint cmd)
 {
    uint m;

    // doł±cza 2 bity startowe 
    cmd |= (3<<12); 

    for(m=(1<<13); m>0; m>>=1)
    {
       if(cmd & m)// je¶li bit o warto¶ci 1
       {   
           led_off();
           _delay_ms(0.89); // czas trwania połowy bitu 
           led_on();
           _delay_ms(0.89);
       }
       else // je¶li bit o warto¶ci 0 
       {   
           led_on(); 
           _delay_ms(0.89);
           led_off(); 
           _delay_ms(0.89);
       } 
    }
    led_off();  
	timer_clear();
 }

int main(void)
{
   u8 cmd;
   u8 toggle = 0 ;  

   // PB1(OC1A) to nasz ledek
   DDRB = (1<<PB1);
   
   //nasz tsopik pracuje na 30kHz
   OCR1AH = 0;
   OCR1AL = 132; 
 
   while(1)
     {
        // wysyłamy numerek 13
        cmd =  13|((toggle & 0x01)<<11);
        toggle++;
  
        // Wysyła  komendę 
        send_cmd(cmd); 
        _delay_ms(89); // 50 bitów "ciszy"
     }
}



