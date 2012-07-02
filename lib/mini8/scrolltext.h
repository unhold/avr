////////////////////////////////////////////////////////////////////////////////
// file       : scrolltext.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, 5*7 dotmatrix with row decoder (mini8)
// description: put scrolltext on the dotmatrix display
//            : interrupt driven timing using Timer2
//            : sleeps MCU in blocking calls
////////////////////////////////////////////////////////////////////////////////

#ifndef _SCROLLTEXT_INCLUDED_
#define _SCROLLTEXT_INCLUDED_

#include "smallfont.h"
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef LIBCONF
#	include "libconf.h"
#else // LIBCONF
// Parameters, can be freely adjusted, but keep in mind that
// over/underflows could happen in the expressions where these are used
#	define SCROLL_REFRESH 120 // Refresh rate (in Hz)
#	define SCROLL_TIME 500 // Scroll time (in ms)
#	define SCROLL_BUF_SIZE 16 // Size of the character ring buffer (in byte)
#	define SCROLL_SLEEP 1 // Enable(1)/disable(0) sleep for power saving
#	define SCROLL_REVERSE 1 // Dotmatrix upside-down
#endif // LIBCONF

#if SCROLL_SLEEP == 1
#	include <avr/sleep.h>
#endif // SCROLL_SLEEP

void scrolltext_init(void);	// Intialize module, HAS TO BE CALLED
	// before any other call to the module
	// Note that the status of the I-flag is not changed;
	// for correct function of the module it has to be enabled outside
	
void scrolltext_putc(char const); // Write char from RAM into buffer,
	// _blocks_ when buffer full (with sleep for power saving)

void scrolltext_puts(char const*); // Write zero-terminated string from RAM
	// into buffer, _blocks_ when buffer full (with sleep for power saving)

void scrolltext_puts_p(prog_char const*); // Write zero-terminated string
	// from Flash into buffer, _blocks_ when buffer full
	// (with sleep for power saving)

uint8_t scrolltext_full(void); // Returns (logically) if buffer full

uint8_t scrolltext_empty(void); // Returns (logically) if buffer empty

void scrolltext_wait(void); // Wait until all characters displayed
	// (with sleep for power saving)

#endif // _SCROLLTEXT_INCLUDED_
