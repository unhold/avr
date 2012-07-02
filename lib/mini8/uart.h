////////////////////////////////////////////////////////////////////////////////
// file       : uart.c
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8 or compatible Atmel MCU
// description: ring-buffered interrupt-driven UART driver
////////////////////////////////////////////////////////////////////////////////

#ifndef UART_H
#define UART_H

#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>

#define RXBUF_SIZE 8 // Size of Receive-Buffer in Byte
#define TXBUF_SIZE 128 // Size of Transceive-Buffer in Byte

#define UARTCLK F_CPU // Clock frequency of UART in Hertz
	// F_CPU is defined by WinAVR

// Calculates 'baud_rate_reg' from a desired baud rate '_BR_' (for Normal Mode)
#define BAUDRATE(_BR_) \
	(UARTCLK/(16L*(_BR_))-1), 0

// Calculates 'baud_rate_reg' from a desired baud rate '_BR_'
// (for Double Speed Mode)
#define BAUDRATE_U2X(_BR_) \
	(UARTCLK/(8L*(_BR_))-1), 1

// Enable the function uart_stream()
#define UART_STREAM 1

// Initializes UART0
void uart_init(uint16_t const baud_rate_reg, uint8_t const use_u2x);

// Sends a character
void uart_putc(char const data); 

// Sends a string
void uart_puts(char const * str); 

// Sends a program string
void uart_puts_p(prog_char const * str);

// Takes the next value out of RX buffer and returns it
char uart_getc(void);

// Returns zero if RX buffer is empty, else a non-zero value
uint8_t uart_rx(void);

#if UART_STREAM
#	include <stdio.h>

	// A stream for reading and writing the UART
	extern FILE uart_stream;

#endif // UART_STREAM

#endif // UART_H
