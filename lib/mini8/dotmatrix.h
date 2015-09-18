////////////////////////////////////////////////////////////////////////////////
// file       : dotmatrix.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, 5*7 dotmatrix with row decoder (mini8)
// description: put text on the dotmatrix display, showing one letter bye one
//            : interrupt driven timing using Timer2
////////////////////////////////////////////////////////////////////////////////

#ifndef DOTMATRIX_H
#define DOTMATRIX_H

#include <avr/io.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "fonts.h"

#define DOTMATRIX_REVERSE 0 // Activate to rotate the letters on the display
#define DOTMATRIX_REFRESH_RATE 60 // Refresh rate in [Hz]
#define DOTMATRIX_CALLBACK_TIME 200 // Callback every t in [ms]

typedef void(*dotmatrix_callback_t)(void); // Type of a callback function

void dotmatrix_init(void); // This HAS TO BE CALLED before use
	// Note that the status of the I-flag is not changed;
	// for correct function of the module it has to be enabled outside

void dotmatrix_on(void);

void dotmatrix_off(void);

void dotmatrix_set_callback(dotmatrix_callback_t);
	// Set a callback func to call every DOTMATRIX_CALLBACK_TIME milliseconds

void dotmatrix_putc(uint8_t); // Put a character to the display immediateley

void dotmatrix_putfont(font_t);	// Put a font in RAM to the display immediateley

void dotmatrix_putfont_p(font_pgm_t);
	// Put a font in Flash to the display immediateley

void dotmatrix_walker(void); // Custom callback funtion example,
	// creating a stickman on the display dancing "stayin' alive" :)

void dotmatrix_babsi(void); // Custom callback function example,
	// creating a message dedicated to Babsi on the display :)

#endif // DOTMATRIX_H
