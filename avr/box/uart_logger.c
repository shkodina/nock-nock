#include "uart_logger.h"
#include "common_makros.h"
#include <avr/interrupt.h>
#include "leds.h"
//=============================================================================

#include <avr/io.h>

//=============================================================================

#define RXB8 1
#define TXB8 0
#define UPE 2
#define OVR 3
#define FE 4
#define UDRE 5
#define RXC 7

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)

//============================================================================

// USART Receiver buffer
#define RX_BUFFER_SIZE 128
volatile char rx_buffer[RX_BUFFER_SIZE];
volatile unsigned char rx_counter = 0; 

//=============================================================================
void GPIO_Configuration(void);
void USART_Configuration(void);
unsigned char getData(char * buffer, unsigned char buf_len);
//=============================================================================
inline void usart_init(void)
{
	    GPIO_Configuration();

	    USART_Configuration();

}
//=============================================================================
void GPIO_Configuration(void)
{
	UPBIT(DDRD, 1); // Tx

}
//=============================================================================
void USART_Configuration(void)
{

// whith interrupt

	#define USART_BAUDRATE 9600
	#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

	UCSRB = ( 1 << TXEN ) | ( 1 << RXEN ) | (1 << RXCIE ); // rx enable, tx enable, rx_interrupt enable
	UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // Use 8-bit character sizes

	UBRRH = 0;//(BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
	UBRRL = 49;//BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register




sei();

}
//=============================================================================
//=============================================================================
//      PUBLICK      FUNCTIONS      PUBLICK      FUNCTIONS      PUBLICK     
//=============================================================================
//=============================================================================
void loggerInit(){
	usart_init();
}
//=============================================================================
void loggerWriteByteInBit(const unsigned char byte){
	char bit[8];
	for (char i = 0, j = 7; i < 8; i++, j--){
		bit[i] = ((byte & (1<<j)) > 0) ? '1' : '0';
	}
	loggerWrite(bit,8);
}
//=============================================================================
void loggerWrite(const unsigned char *message, char count){
    while(count--)
    {
        while ( !( UCSRA & (1<<5)) ) {} 
        UDR=*message++;
    }
}
//=============================================================================
void loggerWriteToMarker(const unsigned char *message, char end_markser){
    while(*message != end_markser)
    {
        while ( !( UCSRA & (1<<5)) ) {} 
        UDR=*message++;
    }
}
//=============================================================================
ISR (USART_RXC_vect)
{
	//ledTaggle(LEDRED1);
	char status,data;
	status = UCSRA;
	data = UDR;
//	if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0){
		if (rx_counter == RX_BUFFER_SIZE)
			return;

	   	rx_buffer[rx_counter++] = data;
//	}
}
//=============================================================================
unsigned char getData(char * buffer, unsigned char buf_len){

	if (rx_counter == 0)
		return 0;
	cli();
	unsigned char c,
		end = (buf_len < RX_BUFFER_SIZE) ? buf_len : RX_BUFFER_SIZE;
		end = (end < rx_counter) ? end : rx_counter;
		c = end;

	while(end--){
		buffer[end] = rx_buffer[end];
	}

	rx_counter = 0;
	sei();

	return c;
}
