#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define RC5_IN  (PIND & (1<<3))

volatile unsigned char timerL; 
volatile unsigned char timerH; 

void init_rc5(void)
{
	TCCR0 = (1<<CS00);  // wlacza Timer0  
	TIMSK = (1<<TOIE0); // w≥πcza przerwanie "Timer0 Overflow"

	sei();
}

ISR(TIMER0_OVF_vect)
{
   volatile static unsigned char inttemp;

   // zmienna timerL zwiÍksza siÍ co 32us
   timerL++;

   // zmienna timerH  zwiÍksza siÍ co 8.192ms (32us*256) 
   inttemp++;
   if(!inttemp ) timerH++;
}

 unsigned int detect(void) //http://hobby.abxyz.bplaced.net/index.php?pid=3&aid=16
 {
    unsigned char temp;
    unsigned char ref1;
    unsigned char ref2;
    unsigned char bitcnt;
    unsigned int command;

    timerH  = 0;
    timerL  = 0;

    // Czeka na okres ciszy na linii wejúcia uC trwajπcy  3.5ms
    // Jeúli nie wykryje takiego okresu ciszy w ciπgu 131ms,
    // to koÒczy dzia≥anie funkcji z b≥Ídem
    while( timerL<110)
    {
       if(timerH>=16)  return  command = -1;

       if(!RC5_IN) timerL = 0;
    }

    // Czeka na  pierwszy bit startowy. 
    // Jeúli nie wykryje bitu startowego w ciπgu 131ms,
    // to koÒczy dzia≥anie funkcji z b≥Ídem
    while(RC5_IN)  
         if(timerH>=16)  return command = -1 ;


    // Pomiar czasu trwani niskiego poziom sygan≥u 
    // w pierwszym bicie startowym.
    // Jeúli nie wykryje rosnπcego zbocza sygna≥u w ciπgu  
    // 1ms, to koÒczy dzia≥anie funkcji z b≥Ídem 
    timerL = 0;
    while(!RC5_IN)
         if(timerL>34) return command = -1;

    //
    temp = timerL;
    timerL = 0;

    // ref1 - oblicza  3/4 czasu trwania bitu
    ref1 =temp+(temp>>1);

    // ref2 - oblicza 5/4 czasu trwania bitu
    ref2 =(temp<<1)+(temp>>1);

 
    // Oczekuje na zbocze opadajπce sygna≥u w úrodku drugiego
    // bitu startowego.
    // Jeúli nie wykryje zbocza w ciπgu 3/4 czasu trwania 
    // bitu, to koÒczy dzia≥anie funkcji z b≥Ídem 
    while(RC5_IN)
         if(timerL > ref1) return command = -1;

    // W momencie wykrycia zbocza sygna≥u, synchronizuje
    // zmienπ timerL dla prÛbkowania  bitu toggle
    timerL = 0;

    // Odczytuje dekoduje pozosta≥e 12 bitÛw polecenia rc5
    for(bitcnt=0, command = 0; bitcnt <12; bitcnt++)
    {
       // Czeka 3/4 czasu trwania bitu od momentu wykrycia
       // zbocza sygna≥u w po≥owie poprzedniego bitu 
       while(timerL < ref1) {};
 
       // PrÛbkuje - odczytuje port we  uC
       if(!RC5_IN)
       {
          // Jeúli odczytano 0, zapamiÍtuje w zmiennej 
          // "command" bit o wartoúci 0         
          command <<= 1 ;

          // Oczekuje na zbocze rosnπce sygna≥u w úrodku bitu.
          // Jeúli nie wykryje zbocza w ciπgu 5/4 czasu trwania 
          // bitu, to koÒczy dzia≥anie funkcji z b≥Ídem    
          while(!RC5_IN)
             if(timerL > ref2) return command = -1;
       }
       else
       {
          // Jeúli odczytano 1, zapamiÍtuje w zmiennej 
          // "command" bit o wartoúci 1  
          command = (command <<1 ) | 0x01;

          // Oczekuje na zbocze opadajπce sygna≥u w úrodku bitu.
          // Jeúli nie wykryje zbocza w ciπgu 5/4 czasu trwania 
          // bitu, to koÒczy dzia≥anie funkcji z b≥Ídem 
          while(RC5_IN)
             if(timerL > ref2) return command = -1;
       }

       // W momencie wykrycia zbocza sygna≥u, synchronizuje
       // zmienπ timerL dla prÛbkowania kolejnego bitu
       timerL = 0;
   }

   // Zwraca kod polecenia rc5
   // bity 0..5 numer przycisku
   // bity 6..10  kod systemu(urzπdzenia)
   // bit 11 toggle bit
   
   //obcinamy toggle bit, nie jest nam potrzebny:)   
   command &= ~(1 << 11);
   
   return command;
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


int main(void)
{
  unsigned int cmd;
unsigned char i=0;

  DDRD |= (1 << PD2) | (1 << PD1); //RX+DATA pin init
  DDRD &= ~(1<<0); //TX
  DDRC = 0xff;
  init_rc5(); 
  USARTInit(51); 

  while (1)
  {
     // Wykrywa i dekoduje polecenie pilota RC5 
     cmd = detect();
	 
     // Jeúli odebrano komendÍ 
     if(cmd != -1)
     {  
		USARTWriteChar('G');
		USARTWriteChar(':');		
		USARTWriteChar((cmd+48)); // narazie dla jednoznakowych
		USARTWriteChar('\r'); //CR=LF
		USARTWriteChar('\n');
		_delay_ms(1); //haxxx
		PORTC = 0xff;		
	}
	if (i == 10){   // zeby sprawdzic czy uklad chodzi jak cza.
		USARTWriteChar('-');
		USARTWriteChar('-');
		USARTWriteChar('\n');
		USARTWriteChar('\r');
		i=0;	
    }
    i++;
  }

  return 0;
}
