/*
	Biblioteka RFM70 dla AVR'ów oparta na projekcie Daniela z http:://projects.web4clans.com
	
	Autor: freakone
	WWW: freakone.pl
	@: kamil@freakone.pl
*/

#include "rfm70.h"

///////////////////////////////////////////////////////
// Makra komend i wartoœci inicjalizuj¹ce rejestry	//
//////////////////////////////////////////////////////

//komendy
const uint8_t PROGMEM RFM70_cmd_adrRX0[] = { (0x20|0x0A), 0x34,0x43,0x10,0x10,0x01};
const uint8_t PROGMEM RFM70_cmd_adrTX[]  = { (0x20|0x10), 0x34,0x43,0x10,0x10,0x01};
const uint8_t PROGMEM RFM70_cmd_adrRX1[] = { (0x20|0x0B), 0x35,0x43,0x10,0x10,0x02};
const uint8_t PROGMEM RFM70_cmd_switch_cfg[] = { 0x50, 0x53 }; // zmiana rejestru
const uint8_t PROGMEM RFM70_cmd_flush_rx[] = { 0xe2, 0x00 }; // flush RX FIFO
const uint8_t PROGMEM RFM70_cmd_flush_tx[] = { 0xe1, 0x00 }; // flush TX FIFO
const uint8_t PROGMEM RFM70_cmd_activate[] = { 0x50, 0x73 }; // aktywacja
const uint8_t PROGMEM RFM70_cmd_tog1[]={ (0x20|0x04), 0xd9 | 0x06, 0x9e, 0x86, 0x0b };
const uint8_t PROGMEM RFM70_cmd_tog2[]={ (0x20|0x04), 0xd9 & ~0x06, 0x9e, 0x86, 0x0b}; //set1[4]

//Bank 0
const uint8_t PROGMEM RFM70_bank0Init[][2] = {
	{ (0x20|0x00), 0x0F }, //Disable CRC ,CRC=1byte, POWER UP, TX
	{ (0x20|0x01), 0x3F }, //Enable auto acknowledgement data pipe0-5
	{ (0x20|0x02), 0x3F }, //Enable RX Addresses pipe0-5
	{ (0x20|0x03), 0x03 }, //RX/TX address field width 5byte
	{ (0x20|0x04), 0x08 }, //x = 250 ms = 4000ms, y = 15 tries
	{ (0x20|0x05), 0x17 }, //channel = 0x17
	{ (0x20|0x06), 0x3F }, //air data rate-2M,out power 5dbm,setup LNA gain high (0dBM)
	{ (0x20|0x07), 0x07 }, //
	{ (0x20|0x08), 0x00 }, //
	{ (0x20|0x09), 0x00 }, //
	{ (0x20|0x0C), 0xc3 }, //LSB Addr pipe 2
	{ (0x20|0x0D), 0xc4 }, //LSB Addr pipe 3
	{ (0x20|0x0E), 0xc5 }, //LSB Addr pipe 4
	{ (0x20|0x0F), 0xc6 }, //LSB Addr pipe 5
	{ (0x20|0x11), 0x20 }, //Payload len pipe0
	{ (0x20|0x12), 0x20 }, //Payload len pipe0
	{ (0x20|0x13), 0x20 }, //Payload len pipe0
	{ (0x20|0x14), 0x20 }, //Payload len pipe0
	{ (0x20|0x15), 0x20 }, //Payload len pipe0
	{ (0x20|0x16), 0x20 }, //Payload len pipe0
	{ (0x20|0x17), 0x20 }, //Payload len pipe0
	{ (0x20|0x1C), 0x3F }, //Enable dynamic payload legth data pipe0-5
	{ (0x20|0x1D), 0x07 } //Enables Dynamic Payload Length,Enables Payload with ACK
};

//Bank 1
const uint8_t PROGMEM RFM70_bank1Init[][5] = {
	// address data
	{ (0x20|0x00), 0x40, 0x4B, 0x01, 0xE2 },
	{ (0x20|0x01), 0xC0, 0x4B, 0x00, 0x00 },
	{ (0x20|0x02), 0xD0, 0xFC, 0x8C, 0x02 },
	{ (0x20|0x03), 0x99, 0x00, 0x39, 0x41 },
	{ (0x20|0x04), 0xb9, 0x9E, 0x86, 0x0B }, // b9? f9?
	{ (0x20|0x05), 0x24, 0x06, 0x7F, 0xA6 },
	{ (0x20|0x06), 0x00, 0x00, 0x00, 0x00 },
	{ (0x20|0x07), 0x00, 0x00, 0x00, 0x00 },
	{ (0x20|0x08), 0x00, 0x00, 0x00, 0x00 },
	{ (0x20|0x09), 0x00, 0x00, 0x00, 0x00 },
	{ (0x20|0x0a), 0x00, 0x00, 0x00, 0x00 },
	{ (0x20|0x0b), 0x00, 0x00, 0x00, 0x00 },
	{ (0x20|0x0C), 0x00, 0x12, 0x73, 0x00 },
	{ (0x20|0x0D), 0x36, 0xb4, 0x80, 0x00 }
};

//Bank1 rejestr 14
const uint8_t PROGMEM RFM70_bank1R0EInit[] = {
	// address Data...
	(0x20|0x0E), 0x41,0x20,0x08,0x04,0x81,0x20,0xCF,0xF7,0xFE,0xFF,0xFF
};


//////////////
// DEBUG	//
//////////////

void blinker(uint8_t scheme)
{
	uint8_t tmp = scheme;
	
	while(tmp)
	{
		if(tmp & scheme)
			PORT_BLINK |= (1<<BLINK);
		else
			PORT_BLINK &= ~(1<<BLINK);
			
		_delay_ms(250);
		tmp >>= 1;		
	}

}

void debug(uint8_t token)
{
	#if defined(USART_LIB) && defined(DEBUG)
	
	switch(token)
	{
	case 0x01:
		uart_puts("rfm error: blad inicjalizacji!");
		break;

	case 0x02:
		uart_puts("rfm error: bufor przepelniony!");
		break;
		
	case 0x03:
		uart_puts("rfm ok: modul zainicjalizowany!");
	}
	
	#endif
	
	if(token == 0x01)
	{
		while(1)
			blinker(0b10100011);
	}
}

///////////////////////////////////////////////////////
// funkcje inicjalizuj¹ce i ustawiaj¹ce rejestry	//
//////////////////////////////////////////////////////

void initRFM() //inicjalizacja sprzêtu
{
	#ifdef USART_LIB
	USART_Init();
	#endif
	
	// pin Chip Enable - wyjœcie i stan niski
	DDR_SPI |= (1<<CE);
	PORT_SPI &=~(1<<CE);
	
	//ustawiamy diodê informacyjn¹ 
	DDR_BLINK |= (1<<BLINK);

	//inicjalizacja spi i pinu Chip Select
	DDR_SPI |= (1<<SCK)|(1<<MOSI)|(1<<CSN);
	DDR_SPI &= ~(1 << MISO);
	PORT_SPI |= (1<<CSN)|(1 << MISO);
	PORT_SPI &=~((1<<MOSI)|(1<<SCK));	

	SPCR = (1<<SPE)|(1<<MSTR);
	SPCR = (SPCR & ~SPI_CLOCK_MASK) | (RFM77_DEFAULT_SPI_CLOCK_DIV & SPI_CLOCK_MASK);
	SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((RFM77_DEFAULT_SPI_CLOCK_DIV >> 2) & SPI_2XCLOCK_MASK);	
	
	_delay_ms(RFM70_BEGIN_INIT_WAIT_MS);//koniecznie opóŸnienie - datasheet
	initRegisters(); // inicjalizujemy rejestry
}

void initRegisters(void) //inicjalizacja rejestrów
{
	// wybieramy bank 0
	selectBank(0);

	//przepisujemy wartoœci inicjalizacyjne
	for (int i = 0; i < 20; i++)
		writeRegVal(pgm_read_byte(&RFM70_bank0Init[i][0]), pgm_read_byte(&RFM70_bank0Init[i][1]));

	//w³¹czamy rejestry
	writeRegPgmBuf((uint8_t *)RFM70_cmd_adrRX0, sizeof(RFM70_cmd_adrRX0));
	writeRegPgmBuf((uint8_t *)RFM70_cmd_adrRX1, sizeof(RFM70_cmd_adrRX1));
	writeRegPgmBuf((uint8_t *)RFM70_cmd_adrTX, sizeof(RFM70_cmd_adrTX));

	if(!readRegVal(RFM70_REG_FEATURE))
		writeRegPgmBuf((uint8_t *)RFM70_cmd_activate, sizeof(RFM70_cmd_activate));

	writeRegVal(pgm_read_byte(&RFM70_bank0Init[22][0]), pgm_read_byte(&RFM70_bank0Init[22][1]));
	writeRegVal(pgm_read_byte(&RFM70_bank0Init[21][0]), pgm_read_byte(&RFM70_bank0Init[21][1]));

	//prze³¹czamy na bank 1
	selectBank(1);
	
	//analogicznie
	for (int i=0; i < 14; i++)
		writeRegPgmBuf((uint8_t *)RFM70_bank1Init[i], sizeof(RFM70_bank1Init[i]));

	writeRegPgmBuf((uint8_t *)RFM70_bank1R0EInit, sizeof(RFM70_bank1R0EInit));
	writeRegPgmBuf((uint8_t *)RFM70_cmd_tog1, sizeof(RFM70_cmd_tog1));
	writeRegPgmBuf((uint8_t *)RFM70_cmd_tog2, sizeof(RFM70_cmd_tog2));

	_delay_ms(RFM70_END_INIT_WAIT_MS); //czekamy
	
	if (readRegVal(0x08) != 0x63) // sprawdzamy poprawnoœæ inicjalizacji przez odczyt ID
		debug(0x01);		
	else
		debug(0x03);

	//wybieramy bank 0 i domyœlnie ustawiamy odbiornik w tryb RX
	selectBank(0);
	setModeRX();
}

uint8_t transmitSPI(uint8_t val) //funkcja przesy³ania danych przez sprzêtowy interfejs SPI
{
	SPDR = val;
	while (!(SPSR & _BV(SPIF)))
	;
	return SPDR;
}

void selectBank(uint8_t bank) //wybór banku ustawieñ
{
	uint8_t tmp = readRegVal(0x07) & 0x80;
	if(bank) 
	{
		if(!tmp)
			writeRegPgmBuf((uint8_t *)RFM70_cmd_switch_cfg, sizeof(RFM70_cmd_switch_cfg));
	} 
	else 
	{
		if(tmp)
			writeRegPgmBuf((uint8_t *)RFM70_cmd_switch_cfg, sizeof(RFM70_cmd_switch_cfg));
	}
}

void setModeRX(void) //w³¹czenie trybu odbiornika
{
	uint8_t val;
	writeRegPgmBuf((uint8_t *)RFM70_cmd_flush_rx, sizeof(RFM70_cmd_flush_rx)); // opró¿niamy bufor
	val = readRegVal(RFM70_REG_STATUS); // odczytujemy status
	writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_STATUS, val); //reset bitów przerwañ
	PORT_SPI &=~ (1<<CE); //blokujemy RFM
	val=readRegVal(RFM70_REG_CONFIG);
	val |= RFM70_PIN_PRIM_RX;
	writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_CONFIG, val); //ustawiamy tryb odbioru
	PORT_SPI |= (1<<CE); // odblokowujemy chip
}


void setModeTX(void) // w³¹czenie trybu nadawczego, analogicznie do RX
{
	uint8_t val;
	writeRegPgmBuf((uint8_t *)RFM70_cmd_flush_tx, sizeof(RFM70_cmd_flush_tx));
	PORT_SPI &=~ (1<<CE);
	val=readRegVal(RFM70_REG_CONFIG);
	val &= ~RFM70_PIN_PRIM_RX;
	writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_CONFIG, val);
	PORT_SPI |= (1<<CE); 
}

uint8_t getMode(void) // odczytanie trybu
{
	return readRegVal(RFM70_REG_CONFIG) & RFM70_PIN_PRIM_RX;
}

void setChannel(uint8_t cnum) // ustaw kana³
{
	writeRegVal( RFM70_CMD_WRITE_REG | RFM70_REG_RF_CH, cnum);
}

uint8_t getChannel(void) // odczytaj kana³
{
	return readRegVal(RFM70_REG_RF_CH);
}

void setPower(uint8_t pwr) // ustawianie mocy - 0: -10dBm | 1: -5dBm | 2: 0dBm | 3: 5dBm
{
	if (pwr > 3) return;
	uint8_t tmp = readRegVal(RFM70_REG_RF_SETUP);
	tmp &= 0xF9;
	tmp |= pwr << 1;
	writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_RF_SETUP, tmp);
}

uint8_t readRegVal(uint8_t cmd)  // odczytaj wartoœæ z rejestru
{
	uint8_t res;
	PORT_SPI &=~ (1<<CSN);
	_delay_ms(RFM70_CS_DELAY);

	transmitSPI(cmd);

	res=transmitSPI(0);
	PORT_SPI |= (1<<CSN);
	_delay_ms(RFM70_CS_DELAY);
	return res;
}

uint8_t writeRegVal(uint8_t cmd, uint8_t val) // zapisz wartoœæ do rejestru
{
	PORT_SPI &=~ (1<<CSN);
	_delay_ms(RFM70_CS_DELAY);
	transmitSPI(cmd);
	transmitSPI(val);
	PORT_SPI |= (1<<CSN);
	_delay_ms(RFM70_CS_DELAY);
	return 1;
}

void readRegBuf(uint8_t reg, uint8_t * buf, uint8_t len) //odczytaj tablicê z rejestru
{
	uint8_t status, byte_ctr;
	PORT_SPI &=~ (1<<CSN);
	_delay_ms(RFM70_CS_DELAY);
	status = transmitSPI(reg); // Select register to write, and read status UINT8
	for(byte_ctr = 0; byte_ctr < len; byte_ctr++)
		buf[byte_ctr] = transmitSPI(0); // Perform SPI_RW to read UINT8 from RFM70
	PORT_SPI |= (1<<CSN);
	_delay_ms(RFM70_CS_DELAY);
}



uint8_t writeRegPgmBuf(uint8_t * cmdbuf, uint8_t len) // zapisz tablicê do rejestru
{
	PORT_SPI &=~ (1<<CSN);
	_delay_ms(RFM70_CS_DELAY);
	while(len--) transmitSPI(pgm_read_byte(cmdbuf++));
	PORT_SPI |= (1<<CSN);
	_delay_ms(RFM70_CS_DELAY);
	return 1;
}

uint8_t writeRegCmdBuf(uint8_t cmd, uint8_t * buf, uint8_t len) //zapisz komendê do rejestru
{
	PORT_SPI &=~ (1<<CSN);
	_delay_ms(RFM70_CS_DELAY);
	transmitSPI(cmd);
	while(len--) transmitSPI(*(buf++));	
	PORT_SPI |= (1<<CSN);
	_delay_ms(RFM70_CS_DELAY);
	return 1;
}

//////////////////////////////////////////////////////////////////////////////////
// funkcje odpowiedzialne za wysy³anie i odbieranie danych oraz obs³ugê buforów //
//////////////////////////////////////////////////////////////////////////////////

uint8_t configRxPipe(uint8_t pipe_nr, uint8_t * adr, uint8_t plLen, uint8_t en_aa)
{

	uint8_t tmp;
	uint8_t nr = pipe_nr -1;

	if(plLen > 32 || nr > 5 || en_aa > 1)
		return 0;

	// write address
	if(nr<2)      // full length for rx pipe 0 an 1
		writeRegCmdBuf(RFM70_CMD_WRITE_REG | (RFM70_REG_RX_ADDR_P0 + nr), adr, sizeof(adr));
	else // only LSB for pipes 2..5
		writeRegVal(RFM70_CMD_WRITE_REG | (RFM70_REG_RX_ADDR_P0 + nr), adr[0]); //ODO:check this

	// static
	if (plLen) {
		// set payload len
		writeRegVal(RFM70_CMD_WRITE_REG | (RFM70_REG_RX_PW_P0 + nr), plLen);
		// set EN_AA bit
		tmp = readRegVal(RFM70_REG_EN_AA);
		if (en_aa)
		tmp |= 1 << nr;
		else
		tmp &= ~(1 << nr);
		writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_EN_AA, tmp);
		// clear DPL bit
		tmp = readRegVal(RFM70_REG_DYNPD);
		tmp &= ~(1 << nr);
		writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_DYNPD, tmp);        
		// set Enable pipe bit
		enableRxPipe(nr);
	}
	// dynamic
	else 
	{
		// set payload len to default
		writeRegVal(RFM70_CMD_WRITE_REG | (RFM70_REG_RX_PW_P0 + nr), 0x20);
		// set EN_AA bit
		tmp = readRegVal(RFM70_REG_EN_AA);
		tmp |= 1 << nr;
		writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_EN_AA, tmp);
		// set DPL bit
		tmp = readRegVal(RFM70_REG_DYNPD);
		tmp |= 1 << nr;
		writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_DYNPD, tmp);
		// set Enable pipe bit
		enableRxPipe(nr);
	}
	return 1;
}

void enableRxPipe(uint8_t pipe_nr) 
{
	uint8_t nr = pipe_nr - 1;
	if (nr > 5) return;
	uint8_t tmp;
	// set Enable pipe bit
	tmp = readRegVal(RFM70_REG_EN_RXADDR);
	tmp |= 1 << nr;
	writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_EN_RXADDR, tmp);
}

void disableRxPipe(uint8_t pipe_nr) 
{
	uint8_t nr = pipe_nr - 1;
	if (nr > 5) return;
	uint8_t tmp;
	// set Enable pipe bit
	tmp = readRegVal(RFM70_REG_EN_RXADDR);
	tmp &= ~(1 << nr);
	writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_EN_RXADDR, tmp);

}

void configTxPipe(uint8_t * adr, uint8_t pltype) 
{
	// write TX address
	writeRegCmdBuf(RFM70_CMD_WRITE_REG | RFM70_REG_TX_ADDR, adr, sizeof(adr));
	// write RX0 address
	writeRegCmdBuf(RFM70_CMD_WRITE_REG | RFM70_REG_RX_ADDR_P0, adr, sizeof(adr));
	// set static or dynamic payload
	uint8_t tmp;
	tmp = readRegVal(RFM70_REG_DYNPD);
	if(pltype == TX_DPL) // dynamic
	tmp |= 1;
	else  
	tmp &= ~(1 << 0);
	writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_DYNPD, tmp);
}

uint8_t sendPayload(uint8_t * payload, uint8_t len, uint8_t toAck)	// choose 0=nAck, 1=AckRequest
{
	
	// check TX_FIFO
	uint8_t status;
	status = readRegVal(RFM70_REG_FIFO_STATUS); 
	if (status & RFM70_FIFO_STATUS_TX_FULL) 
	{
		debug(0x02);
		return 0;
	}

	// send payload
	PORT_SPI &=~ (1<<CSN);
	_delay_ms(RFM70_CS_DELAY);
	if(toAck == -1)
		transmitSPI(RFM70_CMD_W_ACK_PAYLOAD);
	else if (toAck == 0)
		transmitSPI(RFM70_CMD_W_TX_PAYLOAD_NOACK);
	else
		transmitSPI(RFM70_CMD_WR_TX_PLOAD);
		
	while(len--) transmitSPI(*(payload++));
	PORT_SPI |= (1<<CSN);
	_delay_ms(RFM70_CS_DELAY);
	PORT_BLINK ^= (1<<BLINK);
	return 1;
}

uint8_t receivePayload(uint8_t *payload)
{
	uint8_t len;
	// check RX_FIFO
	uint8_t status;
	status = readRegVal(RFM70_REG_STATUS);
	if (status & RFM70_IRQ_STATUS_RX_DR) { // RX_DR
		PORT_BLINK ^= (1<<BLINK);
		uint8_t fifo_sta;
		len = readRegVal(RFM70_CMD_RX_PL_WID); // Payload width
		readRegBuf(RFM70_CMD_RD_RX_PLOAD, payload, len);
		fifo_sta = readRegVal(RFM70_REG_FIFO_STATUS);
		
		if (fifo_sta & RFM70_FIFO_STATUS_RX_EMPTY) {
			status|= 0x40 & 0xCF; // clear status bit rx_dr
			writeRegVal(RFM70_CMD_WRITE_REG | RFM70_REG_STATUS, status); 
		}
		return len;
	}
	else
	{		
		return 0;
	}
}

void flushTxFIFO() 
{
	writeRegPgmBuf((uint8_t *)RFM70_cmd_flush_tx, sizeof(RFM70_cmd_flush_tx));
}

void flushRxFIFO() 
{
	writeRegPgmBuf((uint8_t *)RFM70_cmd_flush_rx, sizeof(RFM70_cmd_flush_rx));
}
