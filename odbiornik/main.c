#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define RC5_IN  (PIND & (1<<3))

volatile unsigned char timerL; 
volatile unsigned char timerH; 

void init_rc5(void)
{
	TCCR0 = (1<<CS00);  // wlacza Timer0  
	TIMSK = (1<<TOIE0); // w³¹cza przerwanie "Timer0 Overflow"

	sei();
}

ISR(TIMER0_OVF_vect)
{
   volatile static unsigned char inttemp;

   // zmienna timerL zwiêksza siê co 32us
   timerL++;

   // zmienna timerH  zwiêksza siê co 8.192ms (32us*256) 
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

    // Czeka na okres ciszy na linii wejœcia uC trwaj¹cy  3.5ms
    // Jeœli nie wykryje takiego okresu ciszy w ci¹gu 131ms,
    // to koñczy dzia³anie funkcji z b³êdem
    while( timerL<110)
    {
       if(timerH>=16)  return  command = -1;

       if(!RC5_IN) timerL = 0;
    }

    // Czeka na  pierwszy bit startowy. 
    // Jeœli nie wykryje bitu startowego w ci¹gu 131ms,
    // to koñczy dzia³anie funkcji z b³êdem
    while(RC5_IN)  
         if(timerH>=16)  return command = -1 ;


    // Pomiar czasu trwani niskiego poziom sygan³u 
    // w pierwszym bicie startowym.
    // Jeœli nie wykryje rosn¹cego zbocza sygna³u w ci¹gu  
    // 1ms, to koñczy dzia³anie funkcji z b³êdem 
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

 
    // Oczekuje na zbocze opadaj¹ce sygna³u w œrodku drugiego
    // bitu startowego.
    // Jeœli nie wykryje zbocza w ci¹gu 3/4 czasu trwania 
    // bitu, to koñczy dzia³anie funkcji z b³êdem 
    while(RC5_IN)
         if(timerL > ref1) return command = -1;

    // W momencie wykrycia zbocza sygna³u, synchronizuje
    // zmien¹ timerL dla próbkowania  bitu toggle
    timerL = 0;

    // Odczytuje dekoduje pozosta³e 12 bitów polecenia rc5
    for(bitcnt=0, command = 0; bitcnt <12; bitcnt++)
    {
       // Czeka 3/4 czasu trwania bitu od momentu wykrycia
       // zbocza sygna³u w po³owie poprzedniego bitu 
       while(timerL < ref1) {};
 
       // Próbkuje - odczytuje port we  uC
       if(!RC5_IN)
       {
          // Jeœli odczytano 0, zapamiêtuje w zmiennej 
          // "command" bit o wartoœci 0         
          command <<= 1 ;

          // Oczekuje na zbocze rosn¹ce sygna³u w œrodku bitu.
          // Jeœli nie wykryje zbocza w ci¹gu 5/4 czasu trwania 
          // bitu, to koñczy dzia³anie funkcji z b³êdem    
          while(!RC5_IN)
             if(timerL > ref2) return command = -1;
       }
       else
       {
          // Jeœli odczytano 1, zapamiêtuje w zmiennej 
          // "command" bit o wartoœci 1  
          command = (command <<1 ) | 0x01;

          // Oczekuje na zbocze opadaj¹ce sygna³u w œrodku bitu.
          // Jeœli nie wykryje zbocza w ci¹gu 5/4 czasu trwania 
          // bitu, to koñczy dzia³anie funkcji z b³êdem 
          while(RC5_IN)
             if(timerL > ref2) return command = -1;
       }

       // W momencie wykrycia zbocza sygna³u, synchronizuje
       // zmien¹ timerL dla próbkowania kolejnego bitu
       timerL = 0;
   }

   // Zwraca kod polecenia rc5
   // bity 0..5 numer przycisku
   // bity 6..10  kod systemu(urz¹dzenia)
   // bit 11 toggle bit
   
   //obcinamy toggle bit, nie jest nam potrzebny:)   
   command &= ~(1 << 11);
   
   return command;
 }


int main(void)
{
  unsigned int cmd;

  DDRD |= (1 << PD2) | (1 << PD1); //DATA pin init
  DDRC = 0xff;
  init_rc5();  

  while (1)
  {
     // Wykrywa i dekoduje polecenie pilota RC5 
     cmd = detect();
	 
     // Jeœli odebrano komendê 
     if(cmd == 10)
     {  
		PORTC |= (1 << PC0);
     }
	 
	 if(cmd == 1)
     {  
		PORTC |= (1 << PC1);
     }
  }

  return 0;
}
