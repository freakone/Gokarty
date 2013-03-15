#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define RC5_IN  (PIND & (1<<3))

volatile unsigned char timerL; 
volatile unsigned char timerH; 

void init_rc5(void)
{
	TCCR0 = (1<<CS00);  // wlacza Timer0  
	TIMSK = (1<<TOIE0); // w��cza przerwanie "Timer0 Overflow"

	sei();
}

ISR(TIMER0_OVF_vect)
{
   volatile static unsigned char inttemp;

   // zmienna timerL zwi�ksza si� co 32us
   timerL++;

   // zmienna timerH  zwi�ksza si� co 8.192ms (32us*256) 
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

    // Czeka na okres ciszy na linii wej�cia uC trwaj�cy  3.5ms
    // Je�li nie wykryje takiego okresu ciszy w ci�gu 131ms,
    // to ko�czy dzia�anie funkcji z b��dem
    while( timerL<110)
    {
       if(timerH>=16)  return  command = -1;

       if(!RC5_IN) timerL = 0;
    }

    // Czeka na  pierwszy bit startowy. 
    // Je�li nie wykryje bitu startowego w ci�gu 131ms,
    // to ko�czy dzia�anie funkcji z b��dem
    while(RC5_IN)  
         if(timerH>=16)  return command = -1 ;


    // Pomiar czasu trwani niskiego poziom sygan�u 
    // w pierwszym bicie startowym.
    // Je�li nie wykryje rosn�cego zbocza sygna�u w ci�gu  
    // 1ms, to ko�czy dzia�anie funkcji z b��dem 
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

 
    // Oczekuje na zbocze opadaj�ce sygna�u w �rodku drugiego
    // bitu startowego.
    // Je�li nie wykryje zbocza w ci�gu 3/4 czasu trwania 
    // bitu, to ko�czy dzia�anie funkcji z b��dem 
    while(RC5_IN)
         if(timerL > ref1) return command = -1;

    // W momencie wykrycia zbocza sygna�u, synchronizuje
    // zmien� timerL dla pr�bkowania  bitu toggle
    timerL = 0;

    // Odczytuje dekoduje pozosta�e 12 bit�w polecenia rc5
    for(bitcnt=0, command = 0; bitcnt <12; bitcnt++)
    {
       // Czeka 3/4 czasu trwania bitu od momentu wykrycia
       // zbocza sygna�u w po�owie poprzedniego bitu 
       while(timerL < ref1) {};
 
       // Pr�bkuje - odczytuje port we  uC
       if(!RC5_IN)
       {
          // Je�li odczytano 0, zapami�tuje w zmiennej 
          // "command" bit o warto�ci 0         
          command <<= 1 ;

          // Oczekuje na zbocze rosn�ce sygna�u w �rodku bitu.
          // Je�li nie wykryje zbocza w ci�gu 5/4 czasu trwania 
          // bitu, to ko�czy dzia�anie funkcji z b��dem    
          while(!RC5_IN)
             if(timerL > ref2) return command = -1;
       }
       else
       {
          // Je�li odczytano 1, zapami�tuje w zmiennej 
          // "command" bit o warto�ci 1  
          command = (command <<1 ) | 0x01;

          // Oczekuje na zbocze opadaj�ce sygna�u w �rodku bitu.
          // Je�li nie wykryje zbocza w ci�gu 5/4 czasu trwania 
          // bitu, to ko�czy dzia�anie funkcji z b��dem 
          while(RC5_IN)
             if(timerL > ref2) return command = -1;
       }

       // W momencie wykrycia zbocza sygna�u, synchronizuje
       // zmien� timerL dla pr�bkowania kolejnego bitu
       timerL = 0;
   }

   // Zwraca kod polecenia rc5
   // bity 0..5 numer przycisku
   // bity 6..10  kod systemu(urz�dzenia)
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
	 
     // Je�li odebrano komend� 
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
