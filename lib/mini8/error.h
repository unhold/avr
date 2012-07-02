////////////////////////////////////////////////////////////////////////////////
// file       : error.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, 5*7 dotmatrix with row decoder (mini8)
// description: Signal error states, perform hardware reset
//            : Uses the Watchdog Timer
////////////////////////////////////////////////////////////////////////////////

#ifndef ERROR_H
#define ERROR_H

#include <inttypes.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

enum {
	ERROR_BADISR = 1
};

#ifdef LIBCONF
#	include "libconf.h"
#else // LIBCONF
//#	define ERROR_TIMEOUT WDTO_2S
		// Set the time the error state is displayed before MCU reset
		// For other possibilitys see <avr/wdt.h> or avr-libc documentation
		// Undefine to permanently display the error
#endif

ISR(BADISR_vect, ISR_NAKED);
	// ISR catching unhandeled interrupts
	// Executes the error function with number ERROR_BADISR

__attribute__((noreturn)) void error(uint8_t number);
	// The lower 5 bits of number are shown on the dotmatrix display
	// Then the MCU is hardware-reset by the WDT after ERROR_TIMEOUT

__attribute__((noreturn)) static inline void reset(void) {
	// Reset the CPU using the Watchdog timer with minimal timout
	cli();
	wdt_enable(WDTO_15MS);
	while (1);
}

#endif // ERROR_H
