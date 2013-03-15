//---------------------------------------------------------------
//   Plik "main.c"
//
//   KURS AVR-GCC (abxyz.bplaced.net)
// 
//   Dekoder  RC5
// 
//   (schemat i opis dzia³ania w artykule)
//   testowanie na atmega8 (8MHz)
//---------------------------------------------------------------

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Odbiornik podczerwieni SFH5110  przy³¹czona do portu  PB0 
#define RC5_IN   (PIND & (1<<3))

//
typedef unsigned char u8;
typedef unsigned int  uint;

// Zmienne globalne pe³ni¹ rolê  programowych zegarów
// napêdzanych przerwaniem TIMER0_OVF
volatile u8 timerL; 
volatile u8 timerH; 

//---------------------------------------------------------------
// Funkcja konfiguruje i uruchamia Timer0 
// oraz w³¹cza przerwanie od przepe³nienia timera,
// przerwanie powinno wystêpowaæ co 32us.  
//---------------------------------------------------------------
void init_rc5(void)
{
  //atmega8
  TCCR0 = (1<<CS00);  // w³¹cza Timer0  
  TIMSK = (1<<TOIE0); // w³¹cza przerwanie "Timer0 Overflow"

/*
  //atmega88
  TCCR0B = (1<<CS00);
  TIMSK0 = (1<<TOIE0);
*/
  // Zezwala na przerwania 
  sei();
}

//---------------------------------------------------------------
// Procedura obs³ugi przerwania  Timer0 Overflow"
//---------------------------------------------------------------
ISR(TIMER0_OVF_vect)
{
   volatile  static u8 inttemp;

   // zmienna timerL zwiêksza siê co 32us
   timerL++;

   // zmienna timerH  zwiêksza siê co 8.192ms (32us*256) 
   inttemp++;
   if(!inttemp ) timerH++;
}

//---------------------------------------------------------------
// Funkcja wykrywa i dekoduje  komendê pilota RC5                                             
//---------------------------------------------------------------
 uint detect(void)
 {
    u8 temp;
    u8 ref1;
    u8 ref2;
    u8 bitcnt;
    uint command;

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
   return command;
 }

void init_uart(void){
	unsigned int ubrr = 8000000 / 16 / 9600 - 1;
	/* Set baud rate*/
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	/* Enable receiver and transmitter*/
	UCSRB = (1<<RXEN)|(1<<TXEN);
	/* Set frame format: 8data, 2stop bit*/
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
}

/*void send_uart(cmd){
	PORTD |= (1<<2); //ustawiamy pin DATA

	while( !( UCSRA & (1<<UDRE)) )
	;
	
	UDR = cmd;
	PORTD = (PORTD & 0b11111011); //zerujemy pin DATA
}*/
 
 unsigned char receive_uart(void){
	/* Wait for data to be received*/
	while( !(UCSRA & (1<<RXC)) )
	;
	/* Get and return received data from buffer*/
	return UDR;
 }
 
//---------------------------------------------------------------
// GLÓWNA FUNKCJA PROGRAMU                                                   
//---------------------------------------------------------------
int main(void)
{
  //
  uint cmd;


  DDRD |= (1<<2); //DATA pin init
  DDRC = 0xff;
  // uruchamia Timer0 i przerwanie
  init_rc5();
  
  //uruchomienie UARTa
  init_uart();

  while (1)
  {
     // Wykrywa i dekoduje polecenie pilota RC5 
     cmd = detect();

     // Jeœli odebrano komendê 
     if(cmd == 10)
     {  
		PORTC = 0xff;
     }
  }

  return 0;
}
