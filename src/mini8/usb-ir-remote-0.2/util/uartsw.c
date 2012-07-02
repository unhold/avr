/*
 * Copyright 2010 Jorgen Birkler
 * jorgen.birkler)a(gmail.com
 *
 *
 * software uart using timer1
 */

///see http://www.rn-wissen.de/index.php/Software-UART_mit_avr-gcc

#include <stdint.h>
#include <avr/interrupt.h>
#include <../hw.h>
#include "stream.h"


#define STATIC_ASSERT(expr) extern char static_assert[ (!!(expr))*2 - 1]


#define INSTR_BEFORE_RELOADED 10
#define TIMER_TICKS_INTR_LATENCY (INSTR_BEFORE_RELOADED * F_TIMER1 / F_CPU)
#define UARTSW_1_TX_TICK_DELAY (3)
#define UARTSW_2_TX_TICK_DELAY (3)
#define UARTSW_1_COMPARE_RELOAD (F_TIMER1 / UARTSW_1_BAUDRATE)
#define UARTSW_2_COMPARE_RELOAD (F_TIMER1 / UARTSW_1_BAUDRATE)


STATIC_ASSERT(UARTSW_1_COMPARE_RELOAD > 5); //We need some accuracy in the sample point time
STATIC_ASSERT(UARTSW_1_COMPARE_RELOAD < 255); //timer is 8bit so we can't have the next reload be more than 255

STATIC_ASSERT(UARTSW_2_COMPARE_RELOAD > 5); //We need some accuracy in the sample point time
STATIC_ASSERT(UARTSW_2_COMPARE_RELOAD < 255); //timer is 8bit so we can't have the next reload be more than 255


/* ------------------------------------------------------------------------- */
/* ------------------------ General Purpose Macros ------------------------- */
/* ------------------------------------------------------------------------- */

#define UARTSW_CONCAT(a, b)            a ## b
#define UARTSW_CONCAT_EXPANDED(a, b)   UARTSW_CONCAT(a, b)

#define UARTSW_OUTPORT(name)           UARTSW_CONCAT(PORT, name)
#define UARTSW_INPORT(name)            UARTSW_CONCAT(PIN, name)
#define UARTSW_DDRPORT(name)           UARTSW_CONCAT(DDR, name)


typedef struct {
	stream_buffer_t internal;
	uint8_t data[16];
}uart_stream_buffer_in_t;


#define PCMSK_A PCMSK0
//#define PCMSK_B PCMSK1 //don't support this one at the moment

#if UARTSW_1_RX_ENABLE!=0
#define UARTSW_1_RXD_PINCHANGE_IE_BIT (_BV(PCIE0))
#define UARTSW_1_RXD_PIN UARTSW_INPORT(UARTSW_1_RXD_PINNAME)
#define UARTSW_1_RXD_PIN UARTSW_INPORT(UARTSW_1_RXD_PINNAME)
#define UARTSW_1_RXD_PORT UARTSW_OUTPORT(UARTSW_1_RXD_PINNAME)
#define UARTSW_1_RXD_DDR UARTSW_DDRPORT(UARTSW_1_RXD_PINNAME)
#define UARTSW_1_RXD_PCMSK UARTSW_CONCAT_EXPANDED(PCMSK_, UARTSW_1_RXD_PINNAME)
#define UARTSW_1_RXD_BIT UARTSW_1_RXD_PINPORT
#if UARTSW_1_INVERT!=0
#define UARTSW_1_RXD_IS_LOW() ((UARTSW_1_RXD_PIN & _BV(UARTSW_1_RXD_BIT)))
#else //#ifdef UARTSW_1_INVERT
#define UARTSW_1_RXD_IS_LOW() (!(UARTSW_1_RXD_PIN & _BV(UARTSW_1_RXD_BIT)))
#endif //#ifdef UARTSW_1_INVERT
static volatile uint16_t inframe1;
static volatile uint8_t bitcnt1;
static uart_stream_buffer_in_t indata1;
static fifo_t infifo;

#endif

#if UARTSW_1_TX_ENABLE!=0
#define UARTSW_1_TXD_DDR UARTSW_DDRPORT(UARTSW_1_TXD_PINNAME)
#define UARTSW_1_TXD_PORT UARTSW_OUTPORT(UARTSW_1_TXD_PINNAME)
#define UARTSW_1_TXD_BIT UARTSW_1_TXD_PINPORT
#if UARTSW_1_INVERT!=0
#define UARTSW_1_TXD_0() UARTSW_1_TXD_PORT |= _BV(UARTSW_1_TXD_BIT)
#define UARTSW_1_TXD_1() UARTSW_1_TXD_PORT &= ~_BV(UARTSW_1_TXD_BIT)
#else //#ifdef UARTSW_1_INVERT
#define UARTSW_1_TXD_1() UARTSW_1_TXD_PORT |= _BV(UARTSW_1_TXD_BIT)
#define UARTSW_1_TXD_0() UARTSW_1_TXD_PORT &= ~_BV(UARTSW_1_TXD_BIT)
#endif //#ifdef UARTSW_1_INVERT
static volatile uint16_t outframe1;
#endif


#if UARTSW_2_RX_ENABLE!=0
#define UARTSW_2_RXD_PINCHANGE_IE_BIT (_BV(PCIE0))
#define UARTSW_2_RXD_PIN UARTSW_INPORT(UARTSW_2_RXD_PINNAME)
#define UARTSW_2_RXD_PORT UARTSW_OUTPORT(UARTSW_2_RXD_PINNAME)
#define UARTSW_2_RXD_DDR UARTSW_DDRPORT(UARTSW_2_RXD_PINNAME)
#define UARTSW_2_RXD_PCMSK UARTSW_CONCAT_EXPANDED(PCMSK_, UARTSW_2_RXD_PINNAME)
#define UARTSW_2_RXD_BIT UARTSW_2_RXD_PINPORT
#if UARTSW_2_INVERT!=0
#define UARTSW_2_RXD_IS_LOW() ((UARTSW_2_RXD_PIN & _BV(UARTSW_2_RXD_BIT)))
#else //#ifdef UARTSW_2_INVERT
#define UARTSW_2_RXD_IS_LOW() (!(UARTSW_2_RXD_PIN & _BV(UARTSW_2_RXD_BIT)))
#endif //#ifdef UARTSW_2_INVERT
static volatile uint16_t inframe2;
static volatile uint8_t bitcnt2;
static uart_stream_buffer_in_t indata2;
#endif

#if UARTSW_2_TX_ENABLE!=0
#define UARTSW_2_TXD_DDR UARTSW_DDRPORT(UARTSW_2_TXD_PINNAME)
#define UARTSW_2_TXD_PORT UARTSW_OUTPORT(UARTSW_2_TXD_PINNAME)
#define UARTSW_2_TXD_BIT UARTSW_2_TXD_PINPORT
#if UARTSW_2_INVERT!=0
#define UARTSW_2_TXD_0() UARTSW_2_TXD_PORT |= _BV(UARTSW_2_TXD_BIT)
#define UARTSW_2_TXD_1() UARTSW_2_TXD_PORT &= ~_BV(UARTSW_2_TXD_BIT)
#else //#ifdef UARTSW_2_INVERT
#define UARTSW_2_TXD_1() UARTSW_2_TXD_PORT |= _BV(UARTSW_2_TXD_BIT)
#define UARTSW_2_TXD_0() UARTSW_2_TXD_PORT &= ~_BV(UARTSW_2_TXD_BIT)
#endif //#ifdef UARTSW_2_INVERT
static volatile uint16_t outframe2;
#endif //#if UARTSW_2_TX_ENABLE!=0



ISR(PCINT_vect,ISR_NOBLOCK)
{
	uint8_t tcnt1 = TCNT1;
	//Start receving sampler if not already started and and pin is low
#if UARTSW_1_RX_ENABLE!=0
	if (!(TIMSK & _BV(OCIE1A)) && !(UARTSW_1_RXD_PIN & _BV(UARTSW_1_RXD_BIT))) {
		OCR1A = tcnt1 + UARTSW_1_COMPARE_RELOAD / 2 - TIMER_TICKS_INTR_LATENCY;
		TIMSK |= _BV(OCIE1A); //Start sampling
		inframe1 = 0;
		bitcnt1 = 0;
	}
#endif //#if UARTSW_1_RX_ENABLE!=0
#if UARTSW_2_RX_ENABLE!=0
	if (!(TIMSK & _BV(OCIE1B)) && UARTSW_2_RXD_IS_LOW()) {
		OCR1B = tcnt1 + UARTSW_2_COMPARE_RELOAD / 2 - TIMER_TICKS_INTR_LATENCY;
		TIMSK |= _BV(OCIE1B); //Start sampling
		inframe2 = 0;
		bitcnt2 = 0;
	}
#endif //#if UARTSW_2_RX_ENABLE!=0
}



#if UARTSW_1_RX_ENABLE!=0 || UARTSW_1_TX_ENABLE!=0
ISR(TIMER1_COMPA_vect,ISR_NOBLOCK)
{
	OCR1A += UARTSW_1_COMPARE_RELOAD;
	uint16_t data;
#if UARTSW_1_TX_ENABLE!=0
	data = outframe1;
	if (data) {
		if (data & 1)  {
			UARTSW_1_TXD_1();
		}
		else {
			UARTSW_1_TXD_0();
		}
		if (data == 1)
		{
			TIMSK &= ~_BV(OCIE1A);
		}
		outframe1 = data >> 1;
	}
	else
#endif //#if UARTSW_2_TX_ENABLE!=0
	{
#if UARTSW_1_RX_ENABLE!=0
		data = inframe1 >> 1;

		if (!UARTSW_1_RXD_IS_LOW()) {
			data |= (1 << 9);
		}

		uint8_t bits = bitcnt1+1;

		if (10 == bits)
		{
			//Have stop bit and start bit?
			if ((data & 1) == 0 && data >= (1 << 9))
			{
				stream_putchar(&indata1, data >> 1);
			}
			TIMSK &= ~_BV(OCIE1A); //stop sampling
		}
		else
		{
			bitcnt1 = bits;
			inframe1 = data;
		}
#endif //#if UARTSW_1_RX_ENABLE!=0
	}
}
#endif


#if UARTSW_2_RX_ENABLE!=0 || UARTSW_2_TX_ENABLE!=0
ISR(TIMER1_COMPB_vect,ISR_NOBLOCK)
{
	OCR1B += UARTSW_1_COMPARE_RELOAD;
	uint16_t data;
#if UARTSW_2_TX_ENABLE!=0
	data = outframe2;
	if (data) {
		if (data & 1)  {
			UARTSW_2_TXD_1();
		}
		else {
			UARTSW_2_TXD_0();
		}
		if (data == 1)
		{
			TIMSK &= ~_BV(OCIE1B);
		}
		outframe2 = data >> 1;
	}
	else
#endif
	{
#if UARTSW_2_RX_ENABLE!=0
		data = inframe2 >> 1;

		if (!UARTSW_2_RXD_IS_LOW()) {
			data |= (1 << 9);
		}

		uint8_t bits = bitcnt2+1;

		if (10 == bits)
		{
			//Have stop bit and start bit?
			if ((data & 1) == 0 && data >= (1 << 9))
			{
				stream_putchar(&indata2.internal, data >> 1);
			}
			TIMSK &= ~_BV(OCIE1B); //stop sampling
		}
		else
		{
			bitcnt2 = bits;
			inframe2 = data;
		}
#endif
	}
}
#endif

// Inline assembly: The nop = do nothing for one clock cycle.
#define nop()  __asm__ __volatile__("nop")

#if UARTSW_1_TX_ENABLE!=0
void uartsw1_putc (const char c)
{
    do
    {
        sei(); nop(); cli(); // yield();
    } while (outframe1);

    // frame = *.P.7.6.5.4.3.2.1.0.S   S=Start(0), P=Stop(1), *=Endemarke(1)
    outframe1 = (3 << 9) | (((uint8_t) c) << 1);
    TIMSK |= _BV(OCIE1A);
    TIFR   = _BV(OCF1A);
    OCR1A = TCNT1 + UARTSW_1_TX_TICK_DELAY;

    sei();
}
#endif

#if UARTSW_1_RX_ENABLE!=0

int uartsw1_getc_nowait(void)
{
	return stream_getchar_nowait(&indata1.internal);
}

int uartsw1_getc_wait(void)
{
	return stream_getchar_wait(&indata1.internal);
}

#endif

#if UARTSW_2_TX_ENABLE!=0
void uartsw2_putc (const char c)
{
    do
    {
        sei(); nop(); cli(); // yield();
    } while (outframe2);

    // frame = *.P.7.6.5.4.3.2.1.0.S   S=Start(0), P=Stop(1), *=Endemarke(1)
    outframe2 = (3 << 9) | (((uint8_t) c) << 1);

    TIMSK |= _BV(OCIE1B);
    TIFR   = _BV(OCF1B);
    OCR1B = TCNT1 + UARTSW_2_TX_TICK_DELAY;

    sei();
}
#endif

#if UARTSW_2_RX_ENABLE!=0


int uartsw2_getc_nowait(void)
{
	return stream_getchar_nowait(&indata2.internal);
}

int uartsw2_getc_wait(void)
{
	return stream_getchar_wait(&indata2.internal);
}

#endif

void uartsw_init(void)
{
    uint8_t sreg = SREG;
    cli();

#if UARTSW_1_RX_ENABLE!=0
    indata1.internal.bufmask = 0xf;
    UARTSW_1_RXD_PCMSK |= _BV(UARTSW_1_RXD_PINPORT); //pin change mask on
    UARTSW_1_RXD_DDR  &= ~_BV(UARTSW_1_RXD_BIT);
    UARTSW_1_RXD_PORT |=  _BV(UARTSW_1_RXD_BIT);
    GIMSK |= UARTSW_1_RXD_PINCHANGE_IE_BIT;
#endif
#if UARTSW_2_RX_ENABLE!=0
    indata2.internal.bufmask = 0xf;
    UARTSW_2_RXD_PCMSK |= _BV(UARTSW_2_RXD_PINPORT); //pin change mask on
    UARTSW_2_RXD_DDR  &= ~_BV(UARTSW_2_RXD_BIT);
    UARTSW_2_RXD_PORT |=  _BV(UARTSW_2_RXD_BIT); //pullup
    //GIMSK |= UARTSW_2_RXD_PINCHANGE_IE_BIT;
#endif
#if UARTSW_1_TX_ENABLE!=0
    UARTSW_1_TXD_PORT |= _BV(UARTSW_1_TXD_BIT);
    UARTSW_1_TXD_DDR  |= _BV(UARTSW_1_TXD_BIT);
    outframe1 = 0;
#endif
#if UARTSW_2_TX_ENABLE!=0
    UARTSW_2_TXD_PORT |= _BV(UARTSW_2_TXD_BIT);
    UARTSW_2_TXD_DDR  |= _BV(UARTSW_2_TXD_BIT);
    outframe2 = 0;
#endif

    SREG = sreg;
}

