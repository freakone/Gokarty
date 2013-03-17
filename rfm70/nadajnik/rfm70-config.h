/*
	Biblioteka RFM70 dla AVR'�w oparta na projekcie Daniela z http:://projects.web4clans.com
	
	Autor: freakone
	WWW: freakone.pl
	@: kamil@freakone.pl
*/

#include "uart.h" // je�li chcemy korzysta� z uarta
#define DEBUG 1 // je�li chcemy debugowa� - uart r�wnie� musi by� zainkludowany

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