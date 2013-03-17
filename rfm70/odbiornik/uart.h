#define F_OSC 8000000
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_OSC / (USART_BAUDRATE * 16UL))) - 1) 
#define USART_LIB 1

static inline void USART_Init()
{	
	UBRRL = BAUD_PRESCALE; 	// Load lower 8-bits of the baud rate value into the low byte of the UBRR register
	UBRRH = (BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
	
	UCSRB = (1<<RXEN)|(1<<TXEN);	
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
}

static inline unsigned char uart_receive( void )
{
	while ( !(UCSRA & (1<<RXC)) ) 	
		;			                

	return UDR;
}

static inline void uart_put( unsigned char data )
{
	while ( !(UCSRA & (1<<UDRE)) )
		; 			                

	UDR = data; 			        
}

static inline void uart_puts(const char *s )
{
    while (*s)
      uart_put(*s++);
}