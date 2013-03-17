#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>



void USARTWriteChar(char data)
{
   PORTD |= (1<<2);
   while(!(UCSRA & (1<<UDRE)));
   UDR=data;
}

unsigned char USARTReadChar( void ) {
	PORTD &= ~(1<<2);
	while ( !(UCSRA & (1<<RXC)) );
	return UDR;
}

void code_recognized(unsigned int cmd)
{
	if(cmd == 11)
    {  
		PORTC |= (1 << PC1);		
	}
	
	if(cmd == 20)
    {  
		PORTC |= (1 << PC0);		
	}
	
	
	USARTWriteChar((char)cmd);
}

ISR(INT1_vect)
{
   static unsigned char n, pb;
   static unsigned int code;
  
   switch(n)
   {
      case 0:

         if(MCUCR&(1<<ISC10))
         {
            MCUCR &= ~(1<<ISC10);
	    TCNT0 = 0x00; TIFR |= (1<<TOV0);
         }
         else 
         {
	    if((TCNT0>112)||(TIFR&(1<<TOV0))) // 112 = 3,6ms
	    {	    
               pb=0;	    
	       code = 0x03;
               TCNT0  = 0x00; TIFR |= (1<<TOV0);
               n = 1;
	    }
            MCUCR |= (1<<ISC10);
	 }
      break;

      case 1:
      case 2:	
         if((TCNT0>14)&&(TCNT0<42))  // 14 = 0.4ms, 42 = 1.35ms
	 {
            MCUCR ^= (1<<ISC10);
            TCNT0  = 0x00; TIFR |= (1<<TOV0);
	    n++;
	 }
	 else n = 0;

      break;

      default:  
    
         if((TCNT0>14)&&(TCNT0<42))
	 {
             if(!pb)
             {		     
	        pb = 1;
                code <<= 1;
		if(code&0x2) code |= 1;
             }
	     else
             {
                pb = 0;
		n++;
	     }

	     TCNT0 = 0x00; TIFR |= (1<<TOV0);
	 }
	 else if((TCNT0>42)&&(TCNT0<70)) // 70=2.25ms
	 {
             code <<= 1;
	     if(!(code&0x2)) code |= 1;

	     TCNT0 = 0x00; TIFR |= (1<<TOV0);
             n++;
	 }
         else  n = 0;		 

         MCUCR ^= (1<<ISC10);

	 if(n==9)
	 {
	     n = 0;
		 code = code & ~(3 << 6);
		 code_recognized(code);
	 }

      break;
   }
   
   
}


void USARTInit(uint16_t ubrr_value)
{
   

   UBRRL = ubrr_value;
   UBRRH = (ubrr_value>>8);

   /*Set Frame Format
   >> Asynchronous mode
   >> No Parity
   >> 1 StopBit
   >> char size 8
   */
   UCSRC=(1<<URSEL)|(3<<UCSZ0);

   UCSRB=(1<<RXEN)|(1<<TXEN);
}

void init_rc5(void)
{
	 DDRD  &= ~(1<<PD3); 
    PORTD &= ~(1<<PD3);
    //
    MCUCR |= (1<<ISC11); // opadajace zbocze 
    GICR  |= (1<<INT1);	
    //
    TCCR0 = (1<<CS02); // prescaler 256
    TIFR |= (1<<TOV0);    

    sei();
}

int main(void)
{
  volatile unsigned int cmd;
  volatile unsigned int i1 = 0, i2 = 0;

  DDRD = (1 << PD2) | (1 << PD1); //RX+DATA pin init
  DDRD &= ~(1<<0); //TX
  DDRC = 0xff;
  init_rc5(); 
  USARTInit(51);

  while (1)
  {			
	if(PORTC & (1 << PC1))
		i2++;
		
	if(PORTC & (1 << PC0))
		i1++;
		
	if(i2 > 100)
	{
		PORTC &= ~(1 << PC1);	
		i2 = 0;
	}
	
	if(i1 > 100)
	{
		PORTC &= ~(1 << PC0);	
		i1 = 0;
	}
	
	_delay_ms(50);
  }

  return 0;
}
