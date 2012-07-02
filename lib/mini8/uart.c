////////////////////////////////////////////////////////////////////////////////
// file       : uart.c
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8 or compatible Atmel MCU
// description: ring-buffered interrupt-driven UART driver
////////////////////////////////////////////////////////////////////////////////

#include "uart.h"

/* Module values */

// Recieve ring buffer
volatile static char RxBuf[RXBUF_SIZE];
volatile static uint8_t RxHead;
volatile static uint8_t RxTail;

// Transcieve ring buffer
volatile static char TxBuf[TXBUF_SIZE];
volatile static uint8_t TxHead;
volatile static uint8_t TxTail;

/* Macros */

// Recieve ring buffer
#define IncRx(RxPointer) \
	((RxPointer == RXBUF_SIZE-1) ? 0 : RxPointer + 1)
#define RxBufEmpty() \
	(RxHead == RxTail)
#define RxBufFull() \
	(IncRx(RxHead) == RxTail)

// Transcieve ring buffer
#define IncTx(TxPointer) \
	((TxPointer == TXBUF_SIZE-1) ? 0 : TxPointer + 1)
#define TxBufEmpty() \
	(TxHead == TxTail)
#define TxBufFull() \
	(IncTx(TxHead) == TxTail)

/* Interrupts */

// Interrupt USART RXC
#define ActivateRXCIE() \
	UCSRB |= (1<<RXCIE)
#define DeactivateRXCIE() \
	UCSRB &= ~(1<<RXCIE)
ISR(USART_RXC_vect) {
	if (!RxBufFull()) {
		RxBuf[RxHead] = UDR;
		RxHead = IncRx(RxHead);
	} 
}

// Interrupt USART UDRE
#define ActivateUDRIE() \
	UCSRB |= (1<<UDRIE)
#define DeactivateUDRIE() \
	UCSRB &= ~(1<<UDRIE)
ISR(USART_UDRE_vect) {
	if (TxBufEmpty()) {
		DeactivateUDRIE();
	} else {
		UDR = TxBuf[TxTail];
		TxTail = IncTx(TxTail);
	}
}

/* Functions */

void uart_putc(char const data) {
	if (data == '\n')
		uart_putc('\r');
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();
	while (TxBufFull())
		sleep_cpu();
	sleep_disable();
	TxBuf[TxHead] = data;
	TxHead = IncTx(TxHead);
	ActivateUDRIE();
}

void uart_puts(char const *str) {
	char c;
	while ((c = *(str++)))
		uart_putc(c);
}

void uart_puts_p(prog_char const *str) {
	char c;
	while ((c = pgm_read_byte(str++)))
		uart_putc(c);
}

char uart_getc(void) {
	char val;
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();
	while (RxBufEmpty()) // wait for char in RX buffer
		sleep_cpu();
	sleep_disable();
	val = RxBuf[RxTail];
	RxTail = IncRx(RxTail);
	return val;
}

uint8_t uart_rx(void) {
	return !RxBufEmpty();
}

#if UART_STREAM
// Basic stream input/output, no processing

int	uart_getc_f(FILE *stream) {
	return uart_getc();
}

int	uart_putc_f(char c, FILE *stream) {
	uart_putc(c);
	return 0;
}

FILE uart_stream = FDEV_SETUP_STREAM(uart_putc_f, uart_getc_f, _FDEV_SETUP_RW);

#endif // UART_STREAM

// Initialize UART0
void uart_init(uint16_t const baud_rate_reg, uint8_t use_u2x) {
	UCSRA = use_u2x ? 1<<U2X : 0;
	UCSRB = (1<<RXCIE)| // enable RXC0 interrupt
			 (0<<TXCIE)| // disable TXC0 interrupt
			 (0<<UDRIE)| // disable UDRE0 interrupt
			 (1<<RXEN)| // enable UART0 reciever
			 (1<<TXEN)| // enable UART0 transmitter
			 (0<<UCSZ2); // ->	 
	UCSRC = (1<<UCSZ1)| // ->
	         (1<<UCSZ0)| // 8 bit frames
			 (0<<UMSEL)| // asynchrounus operation
		     (0<<UPM1)|(0<<UPM0)| // parity disabled
		     (0<<USBS); // one stop bit
	UBRRH = baud_rate_reg>>8;
	UBRRL = baud_rate_reg;
}

