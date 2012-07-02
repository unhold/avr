/**
 *  _   _ ____  ____    ___        ____                      _       
 * | | | / ___|| __ )  |_ _|_ __  |  _ \ ___ _ __ ___   ___ | |_ ___ 
 * | | | \___ \|  _ \   | || '__| | |_) / _ \ '_ ` _ \ / _ \| __/ _ \
 * | |_| |___) | |_) |  | || |    |  _ <  __/ | | | | | (_) | ||  __/
 *  \___/|____/|____/  |___|_|    |_| \_\___|_| |_| |_|\___/ \__\___|
 *   
 * 
 * Copyright 2007 J�rgen Birkler
 * jorgen.birkler@gmail.com
 * USB HID device for IR receiver
 * License: GNU GPL v2 (see License.txt) or proprietary (contact author)
 * 
 * Bit length IR receiver (Suitable for Nec remotes, Onkyo, Pioneer, Goldstar, and others)
 * 
 * SIG_OCMP1 is triggered on timeout
 * 
 * SIG_INPUT_CAPTURE1 is triggered on each fall (because the signal is inverted)
 * of the output from the IR receiver.
 * 
 * Basic description of function:
 * -# Wait for startbit
 * -# Measure length of start bit
 * -# Receive bit; measure length from rise-to-rise;
 *    ONE is short pulse
 *    ZERO is a long pulse
 * -# For NEC there are 32 bits: 8 bit address, 8 bit address inversed, 8 bit command, 8 bit command inversed
 *    the transmission length is the same due to the inverted bits
 * . 
 * 
 * Interrupts :
 *    ________________          ___       ___     ___       ____
 * __/                \________/   \_____/   \___/   \_____/    \_____....
 * 
 *   ^                ^        ^   ^     ^   ^   ^   ^     ^    ^
 *   |                |        |   |     |   |   |   |     |    |
 * 
 * 
 *    
 * 
 * Timing diagram:  
 * - T =420 �s to approx 424 �s in the USA and Canada 
 * - T=454 �s to approx 460 �s in Europe and others 
 * - The header is 8T high and 8T low 
 * - ONE is coded 2T high and 6T low
 * - ZERO is coded 2T high and 2T low
 * - Start bit is 8T high and 8T low 
 * break = 6T / 16T * startbit len = 3/8 * startbit len
 * 
 * For Panasonic a ONE is roughly 3.6ms and a ZERO 1.8ms
 *   
 * 
 * Main resource:
 * - http://www.sbprojects.com/knowledge/ir/nec.htm
 * .
 * See for more resources about protocols:
 * - http://www.epanorama.net/links/irremote.html
 * - http://www.lirc.org/
 * - http://www.hifi-remote.com/wasser/
 * - http://en.wikipedia.org/wiki/Consumer_IR
 * - http://www.ee.washington.edu/circuit_archive/text/ir_decode.txt
 * - http://users.pandora.be/davshomepage/panacode.htm
 * - http://www.hifi-remote.com/infrared/IR-PWM.shtml
 * - http://picprojects.org/projects/sirc/sonysirc.pdf
 * .
 *  
 */

#include "irrx.h"
#include "irrx_config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

// *******************************************************************************
// *
// * Protocol definition
// *
// *******************************************************************************

#if defined(IRRX_PROTOCOL_NEC)
#define IRRX_BITLENGTH_STARTBIT_uS (2400L) 
#define IRRX_BITLENGTH_ZERO_uS (3600L) 
#define IRRX_BITLENGTH_ONE_uS (1800L)
#define IRRX_STARTBITS 2
#define IRRX_NORMAL_BITS 32
#define IRRX_RECEIVE_TIMEOUT_uS 200000L 
#define IRRX_RECEIVE_OK(_irrxbuffer) ((_irrxbuffer.bitbuffer.nec.addr ^ _irrxbuffer.bitbuffer.nec.addr) == 0xFF &&   \
                					 (_irrxbuffer.bitbuffer.nec.command ^ _irrxbuffer.bitbuffer.nec.command_inv) == 0xFF)

#define IRRX_GETCODE(_irrxbuffer) ((_irrxbuffer.bitbuffer.nec.addr << 8) | _irrxbuffer.bitbuffer.nec.command) 
#define IRRX_MSB_FIRST 1
#define IRRX_LSB_FIRST 0

#elif defined(IRRX_PROTOCOL_SONY)

#define IRRX_BITLENGTH_STARTBIT_uS (2400L) 
#define IRRX_BITLENGTH_ZERO_uS (600L)
#define IRRX_BITLENGTH_ONE_uS (1200L)
#define IRRX_STARTBITS 1
#define IRRX_NORMAL_BITS 12
#define IRRX_NORMAL1_BITS 15
#define IRRX_NORMAL2_BITS 20
#define IRRX_RECEIVE_TIMEOUT_uS 36000L
#define IRRX_BUTTON_UP_TIMEOUT_uS 50000L
#define IRRX_MSB_FIRST 1
#define IRRX_LSB_FIRST 0

#define IRRX_RECEIVE_OK(_irrxbuffer) (1) 
#define IRRX_GETCODE(_irrxbuffer) ((_irrxbuffer.bitbuffer.all))

#else
#error IRRX Protocol not defined
#endif 

// *******************************************************************************
// *
// * Internal macros
// *
// *******************************************************************************

///IR receiver is connected to ICP (input capture pin)
#if defined(__AVR_AT90S4433__)
//PB0 is irrx input w/ pull-up. irrx power is directly from vcc
#define IRRX_INIT_PORTS()  {DDRB&=~_BV(PB0);PORTB &= ~_BV(PB0);}
#define IRRX_IS_LOW() (!(PINB & _BV(PB0)))
#define IRRX_POWER_ON() (void)(0)
#define IRRX_POWER_OFF() (void)(0)

#elif defined(__AVR_ATmega8__)
//PB0 is irrx input wo/ pull-up.
#define IRRX_INIT_PORTS()  {DDRB&=~_BV(PB0);PORTB &= ~_BV(PB0);}
#define IRRX_IS_LOW() (!(PINB & _BV(PB0)))
#define IRRX_POWER_ON() 	PORTB|=_BV(PB7)
#define IRRX_POWER_OFF() PORTB&=~_BV(PB7)

#else
#error "Hardware not configured for IR. Add more CPU definitions."
#endif 

#if (1 || defined(IRRX_TRACE_))
#include <stdio.h>
#define IRRX_TRACE1_(str) printf_P(PSTR(str))
#define IRRX_TRACE2_(str,p1) printf_P(PSTR(str),p1)
#else 
#define IRRX_TRACE1_(str) (void)0
#define IRRX_TRACE2_(str,p1) (void)0 
#endif  

///Causes a compilation error if expression is not true
#define IRRX_STATIC_ASSERT(expr) extern char static_assert[ (!!(expr))*2 - 1]

///Set in #bitcount to indicate that reception is complete
#define IRRX_BIT_COUNT_COMPLETE_MASK 0x80

///Enter critical section
#define TIMER1_CRITICAL_SECTION_ENTER() { uint8_t saved_sreg = TIMSK; TIMSK &=~ (_BV(TICIE1) | _BV(TOIE1)); {

///Exit critical section
#define TIMER1_CRITICAL_SECTION_EXIT() } TIMSK = saved_sreg; }

///Init with 1/64 prescaler and input capture enabled
#define IRRX_TIMER1_uS_TO_TICKS(_uS)  (((_uS) * (F_CPU / 1000000L)) / (64L))
#define IRRX_TIMER1_ENABLE_INTERRUPTS() TIMSK |= (_BV(TICIE1) | _BV(OCIE1A))
#define IRRX_TIMER1_ENABLE_ICP_RISING_EDGE() {TCCR1B |=_BV(ICES1); TIMSK |= _BV(TICIE1);}
#define IRRX_TIMER1_ENABLE_ICP_FALLING_EDGE() {TCCR1B &=~_BV(ICES1); TIMSK |= _BV(TICIE1);}
#define IRRX_TIMER1_DISABLE_ICP() {TIMSK &=~_BV(TICIE1);}
#define IRRX_TIMER1_ENABLE_OCA() TIMSK |= (_BV(OCIE1A))
#define IRRX_TIMER1_DISABLE_OCA() TIMSK &= ~(_BV(OCIE1A))

#define IRRX_TIMER1_START() TCCR1A = 0;TCCR1B |= _BV(ICNC1) | _BV(CS11) | _BV(CS10)
#define IRRX_TIMER1_STOP() TCCR1B &= ~ (_BV(CS12)|_BV(CS11)|_BV(CS10)
#define IRRX_TIMER1_CLEAR_ICP_FLAG() TIFR |=_BV(ICF1)
#define IRRX_TIMER1_RESET() TCNT1 = 0

#define IRRX_RECEIVE_TIMEOUT_TICKS IRRX_TIMER1_uS_TO_TICKS(IRRX_RECEIVE_TIMEOUT_uS)
#define IRRX_BUTTON_UP_TIMEOUT_TICKS IRRX_TIMER1_uS_TO_TICKS(IRRX_BUTTON_UP_TIMEOUT_uS)


#define IRRX_BITLENGTH_TICKS_ZERO  IRRX_TIMER1_uS_TO_TICKS(IRRX_BITLENGTH_ZERO_uS)
#define IRRX_BITLENGTH_TICKS_ONE   IRRX_TIMER1_uS_TO_TICKS(IRRX_BITLENGTH_ONE_uS)
#define IRRX_BITLENGTH_TICKS_STARTBIT   IRRX_TIMER1_uS_TO_TICKS(IRRX_BITLENGTH_STARTBIT_uS)

#define IRRX_BITLENGTH_TICKS_STARTBIT_MIN   (IRRX_BITLENGTH_TICKS_STARTBIT * 3 / 4)
#define IRRX_BITLENGTH_TICKS_STARTBIT_MAX   (IRRX_BITLENGTH_TICKS_STARTBIT * 5 / 4)

#define IRRX_BITLENGTH_TICKS_BIT_MIN (IRRX_BITLENGTH_TICKS_ZERO * 3 / 4)
#define IRRX_BITLENGTH_TICKS_BIT_MAX (IRRX_BITLENGTH_TICKS_ONE * 5 / 4)

#define IRRX_BITLENGTH_TICKS_BIT_AVG ((IRRX_BITLENGTH_TICKS_ONE + IRRX_BITLENGTH_TICKS_ZERO) / 2)

IRRX_STATIC_ASSERT(IRRX_BITLENGTH_TICKS_ZERO < IRRX_BITLENGTH_TICKS_ONE); //Code doesn't handle this case yet.


IRRX_STATIC_ASSERT(IRRX_LSB_FIRST + IRRX_MSB_FIRST == 1); //Need one and only one

IRRX_STATIC_ASSERT(IRRX_BITLENGTH_TICKS_ONE >= 10);
//If this fail the prescaler needs to be changed
IRRX_STATIC_ASSERT(IRRX_BITLENGTH_TICKS_ZERO < 6000);
//If this fail the prescaler needs to be changed
IRRX_STATIC_ASSERT(IRRX_BITLENGTH_TICKS_STARTBIT_MIN >= 10);
//If this fail the prescaler needs to be changed
IRRX_STATIC_ASSERT(IRRX_BITLENGTH_TICKS_STARTBIT_MAX < 6000);
//If this fail the prescaler needs to be changed
IRRX_STATIC_ASSERT(IRRX_RECEIVE_TIMEOUT_TICKS> 100);
//If this fail the prescaler needs to be changed
IRRX_STATIC_ASSERT(IRRX_RECEIVE_TIMEOUT_TICKS < 65000);
//If this fail the prescaler needs to be changed

typedef struct
{
	uint16_t ticks_one;
	uint16_t ticks_zero;
	uint16_t ticks_min;
	uint16_t ticks_max;
	uint16_t ticks_startbit_min;
	uint16_t ticks_startbit_max;
	uint16_t ticks_receive_timeout;
} irrx_settings_t;

#if defined(IRRX_DEBUG_PARAMETERS)
const irrx_settings_t irrx_settings PROGMEM  =
	{ IRRX_BITLENGTH_TICKS_ONE, IRRX_BITLENGTH_TICKS_ZERO,
			IRRX_BITLENGTH_TICKS_BIT_MIN, IRRX_BITLENGTH_TICKS_BIT_MAX,
			IRRX_BITLENGTH_TICKS_STARTBIT_MIN,
			IRRX_BITLENGTH_TICKS_STARTBIT_MAX, IRRX_RECEIVE_TIMEOUT_TICKS };
#endif //#if defined(IRRX_DEBUG_PARAMETERS)

// *******************************************************************************
// *
// * Functions and ISRs
// *
// *******************************************************************************


///Contains the currently received bits
static volatile irrx_code_t irrx_receivebuffer = 0;

typedef uint8_t irrx_bitcount_t;

///Internal stuff
typedef struct
{
	///counted bits
	irrx_bitcount_t bitcount;
	///Last received code
	irrx_code_t last_received;
	union
	{
		uint32_t all;
#if defined(IRRX_PROTOCOL_NEC)
		struct
		{
			uint8_t addr;
			uint8_t addr_inv;
			uint8_t command;
			uint8_t command_inv;
		}nec;
#endif //#if defined(IRRX_PROTOCOL_NEC)
#if defined(IRRX_PROTOCOL_SONY)
		struct
		{
			uint8_t device : 5;
			uint8_t command : 7;
			uint32_t padding : 20;
		} sony12;
		struct
		{
			uint8_t device : 8;
			uint8_t command : 7;
			uint32_t padding : 17;
		} sony15;
		struct
		{
			uint8_t extended : 8;
			uint8_t device : 5;
			uint8_t command : 7;
			uint16_t padding : 12;
		} sony20;
#endif //#if defined(IRRX_PROTOCOL_SONY)
	} bitbuffer;
} irrx_internal_t;

///Internal buffer and state
static irrx_internal_t irrx_internal;

IRRX_STATIC_ASSERT(sizeof(irrx_internal.bitbuffer) == sizeof(uint32_t));

ISR(TIMER1_COMPA_vect,ISR_NOBLOCK)
{
	///Timeout; no receive for some time, check what we have
	//Last bit?
	if (irrx_internal.bitcount == IRRX_NORMAL_BITS ||
		irrx_internal.bitcount == IRRX_NORMAL1_BITS ||
		irrx_internal.bitcount == IRRX_NORMAL2_BITS)
	{
		irrx_receivebuffer = IRRX_GETCODE(irrx_internal);
		OCR1A += IRRX_BUTTON_UP_TIMEOUT_TICKS;
		IRRX_TRACE2_("G(%x)",irrx_receivebuffer);
	}
	else {
		irrx_receivebuffer = IRRX_NO_CODE;
		IRRX_TIMER1_DISABLE_OCA();
		IRRX_TRACE1_(".");
		if (irrx_internal.bitcount != 0)
		{
			IRRX_TRACE1_("k");
		}
	}
	irrx_internal.bitcount = 0;
	irrx_internal.bitbuffer.all = 0;
	//Set interrupt to detect start of start bit (falling edge because signal is inverted)
	IRRX_TIMER1_ENABLE_ICP_FALLING_EDGE();
}


ISR(TIMER1_CAPT_vect,ISR_NOBLOCK)
{
	volatile static uint16_t last_icr = 0;
	uint16_t icr_delta = ICR1;
	IRRX_TIMER1_CLEAR_ICP_FLAG();
	icr_delta -= last_icr;
	last_icr = ICR1;

	if (TCCR1B & _BV(ICES1)) {
		IRRX_TIMER1_ENABLE_ICP_FALLING_EDGE();
		IRRX_TRACE2_("%d",icr_delta);
		if (IRRX_BITLENGTH_TICKS_STARTBIT_MIN < icr_delta && icr_delta < IRRX_BITLENGTH_TICKS_STARTBIT_MAX)
		{
			//Ok we're are ready to collect bits. Start the collection done timeout
			//Set timeout ticks for the end of reception (will trigger)
			OCR1A = last_icr + IRRX_RECEIVE_TIMEOUT_TICKS;
			IRRX_TIMER1_ENABLE_OCA();
			IRRX_TRACE1_("B");
			irrx_internal.bitcount = 0;
			if (IRRX_LSB_FIRST) {
				irrx_internal.bitbuffer.all = ((uint32_t)0x8000L);
			}
			else if (IRRX_MSB_FIRST) {
				irrx_internal.bitbuffer.all = 1;
			}
		}
		else if (IRRX_BITLENGTH_TICKS_BIT_MIN < icr_delta && icr_delta < IRRX_BITLENGTH_TICKS_BIT_MAX)
		{
			irrx_internal.bitcount++;

			if (IRRX_LSB_FIRST) {				
				irrx_internal.bitbuffer.all >>= 1; //Make room for new bit
			}
			else if (IRRX_MSB_FIRST) {
				irrx_internal.bitbuffer.all <<= 1; //Make room for new bit
			}

			//0 or 1?
			if (icr_delta < (uint16_t)IRRX_BITLENGTH_TICKS_BIT_AVG)
			{
				if (IRRX_LSB_FIRST) {				
					irrx_internal.bitbuffer.all |= ((uint32_t)0x8000L);
				}
				else if (IRRX_MSB_FIRST) {
					irrx_internal.bitbuffer.all |= 1;
				}
				IRRX_TRACE1_("x");
			}
			else
			{
				IRRX_TRACE1_("o");
			}
		}
		else {
			irrx_internal.bitcount = 0;
		}
	}
	else {
		IRRX_TIMER1_ENABLE_ICP_RISING_EDGE();
	}
}
// *******************************************************************************
// *
// * External API
// *
// *******************************************************************************


void irrx_init(void)
{
	IRRX_INIT_PORTS();
	IRRX_POWER_OFF();
}
 
void irrx_power_on(void)
{
	irrx_internal.bitcount = 0;
	irrx_receivebuffer = IRRX_NO_CODE;
	IRRX_POWER_ON();
	IRRX_TIMER1_START();
	IRRX_TIMER1_ENABLE_INTERRUPTS();
	IRRX_TIMER1_ENABLE_ICP_FALLING_EDGE();
	IRRX_TRACE2_("zero bit ticks:%d",IRRX_BITLENGTH_TICKS_ZERO);
	IRRX_TRACE2_("one bit ticks:%d",IRRX_BITLENGTH_TICKS_ONE);
	IRRX_TRACE2_("one/zero bit ticks:%d",IRRX_BITLENGTH_TICKS_BIT_AVG);
	IRRX_TRACE2_("start bit ticks:%d",IRRX_BITLENGTH_TICKS_STARTBIT);
} 
  
void irrx_power_off(void)
{
	irrx_receivebuffer = IRRX_NO_CODE;
	IRRX_POWER_OFF();
}

irrx_code_t irrx_getcode(void)
{
	irrx_code_t result;
	TIMER1_CRITICAL_SECTION_ENTER()
	result = irrx_receivebuffer;
	TIMER1_CRITICAL_SECTION_EXIT()
	return result;
}
