/**
 * \file
 *  _     _      _    _
 * | |__ (_)_ __| | _| | ___ _ __ ___  ___
 * | '_ \| | '__| |/ / |/ _ \ '__/ __|/ _ \
 * | |_) | | |  |   <| |  __/ | _\__ \  __/
 * |_.__/|_|_|  |_|\_\_|\___|_|(_)___/\___|
 *
 *
 * Initializes the UART a FILE stream and attaches a circular ram buffer
 * so that output is fast.
 *
 * A software inverter is also assumed to be connected from the UART TX to the INT1 line
 * so that the output can be made compatible with a PC without any external inverter or conversion logic.
 *
 * Copyright 2007 J�rgen Birkler
 * jorgen.birkler@gmail.com
 *
 */



#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart_io.h"
#include "uart_io_config.h"



#define UART_IO_CONCAT(a, b)            a ## b
#define UART_IO_CONCAT_EXPANDED(a, b)   UART_IO_CONCAT(a, b)

#define UART_IO_UTIL_OUTPORT(name)           UART_IO_CONCAT(PORT, name)
#define UART_IO_UTIL_INPORT(name)            UART_IO_CONCAT(PIN, name)
#define UART_IO_UTIL_DDRPORT(name)           UART_IO_CONCAT(DDR, name)



#if defined(UART_IO_TX_OUTPORT) && defined(UART_IO_TX_OUTPIN)
#if defined(__AVR_ATmega8__)
#define INVERTER1_IN_PORT D
#define INVERTER1_IN_PIN 3
#else
#error "Hardware not configured for device"
#endif


#define INVERTER1_IN_OUTPORT UART_IO_UTIL_OUTPORT(INVERTER1_IN_PORT)
#define INVERTER1_IN_DDRPORT UART_IO_UTIL_DDRPORT(INVERTER1_IN_PORT)
#define INVERTER1_IN_INPORT UART_IO_UTIL_INPORT(INVERTER1_IN_PORT)


#define INVERTER1_OUT_OUTPORT UART_IO_UTIL_OUTPORT(UART_IO_TX_OUTPORT)
#define INVERTER1_OUT_DDRPORT UART_IO_UTIL_DDRPORT(UART_IO_TX_OUTPORT)
#define INVERTER1_OUT_INPORT UART_IO_UTIL_INPORT(UART_IO_TX_OUTPORT)
#define INVERTER1_OUT_PIN UART_IO_TX_OUTPIN

//Software inverters P = INV(PD3)
///Init Int1 as input and one pin as output, enable interrupt for all changes
static void INVERTER1_ENABLE(void)
{
  INVERTER1_OUT_DDRPORT |= _BV(INVERTER1_OUT_PIN);
  INVERTER1_IN_DDRPORT &= ~_BV(INVERTER1_IN_PIN);
  INVERTER1_IN_OUTPORT &= ~_BV(INVERTER1_IN_PIN);
  MCUCR &= ~_BV(ISC11);
  MCUCR |= _BV(ISC10);
  GICR |= _BV(INT1);
}

#define INVERTER1_IN_IS_HIGH() bit_is_set(INVERTER1_IN_INPORT,INVERTER1_IN_PIN)
#define INVERTER1_OUT_HIGH() INVERTER1_OUT_OUTPORT |= _BV(INVERTER1_OUT_PIN)
#define INVERTER1_OUT_LOW() INVERTER1_OUT_OUTPORT &= ~_BV(INVERTER1_OUT_PIN)

//Declare naked so that compiler is not pushing registers; assembly without using any registers...
SIGNAL(SIG_INTERRUPT1) __attribute__ ((naked));
SIGNAL(SIG_INTERRUPT1)
{
  if (INVERTER1_IN_IS_HIGH())
  {
    INVERTER1_OUT_LOW();
  }
  else
  {
    INVERTER1_OUT_HIGH();
  }
  __asm__("reti"::);
}


#else //if defined(UART_IO_TX_OUTPORT) && defined(UART_IO_TX_OUTPIN)

#define INVERTER1_ENABLE() (void)0

#endif //if defined(UART_IO_TX_OUTPORT) && defined(UART_IO_TX_OUTPIN)


static int uart_file_ioputchar(char c, FILE *stream);

#define UART_IO_BUFFER_SIZE (1<<(UART_IO_BUFFER_SIZE_FACTOR))

typedef struct {
  uint8_t buffer[UART_IO_BUFFER_SIZE];
  uint8_t put;
  uint8_t get;
} circular_buffer_t;

typedef struct {
  circular_buffer_t tx;
  circular_buffer_t rx;
} uart_t;


static uart_t uart;

static FILE uart_file_io = FDEV_SETUP_STREAM(uart_file_ioputchar, NULL, _FDEV_SETUP_WRITE);

static int uart_file_ioputchar(char c, FILE *stream)
{
  if (c == '\n')
  {
    c = '\r';
  }
  if ((uart.tx.put + 1) % UART_IO_BUFFER_SIZE == uart.tx.get)
  {
    return 1; //Full
  }
  uart.tx.buffer[uart.tx.put] = c;
  ++uart.tx.put;
  uart.tx.put %= UART_IO_BUFFER_SIZE;

  //Enable interrupt
  UCSRB |= _BV(UCSRB);
  return 0;
}

SIGNAL(SIG_UART_DATA)
{
  if (uart.tx.put == uart.tx.get)
  {
    UCSRB &= ~_BV(UCSRB); //Disable interrupt
  }
  else {
    UDR = uart.tx.buffer[uart.tx.get];
    ++uart.tx.get;
    uart.tx.get %= UART_IO_BUFFER_SIZE;
  }
}


#define UART_IO_GET_UBRR_X1(baud) ((F_CPU/((baud)*16L))-1)
#define UART_IO_GET_UBRR_X2(baud) ((F_CPU/((baud)*8L))-1)

static void uart_io_enable_tx(void)
{
	UCSRA = 0;
	#define BAUD UART_IO_BAUDRATE
	#include <util/setbaud.h>
	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
	#if USE_2X
	UCSRA |= (1 << U2X);
	#else
	UCSRA &= ~(1 << U2X);
	#endif
  UCSRB = /*_BV(RXEN) | */_BV(TXEN);
  UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
  (void)UDR;
}


FILE* uart_io_init(void)
{
  uart_io_enable_tx();
  INVERTER1_ENABLE();
  return &uart_file_io;
}

char uart_io_ok_to_powersave(void)
{
  return 0;
}




