/*
	Biblioteka RFM70 dla AVR'ów oparta na projekcie Daniela z http:://projects.web4clans.com
	
	Autor: freakone
	WWW: freakone.pl
	@: kamil@freakone.pl
*/

#include "uart.h" // jeœli chcemy korzystaæ z uarta
#define DEBUG 1 // jeœli chcemy debugowaæ - uart równie¿ musi byæ zainkludowany

//RFM70
#define DDR_SPI DDRB
#define PORT_SPI PORTB
#define SCK  PB7
#define MISO PB6
#define MOSI PB5
#define CE   PB4
#define CSN  PB3

//DIODA
#define DDR_BLINK DDRD
#define PORT_BLINK PORTD
#define BLINK PD7