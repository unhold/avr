////////////////////////////////////////////////////////////////////////////////
// file       : fonts.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : 5*7 dotmatrix
// description: character fonts for a 5*7 dotmatrix display, stored in Flash
////////////////////////////////////////////////////////////////////////////////

#ifndef FONTS_H
#define FONTS_H

#include <avr/pgmspace.h>

typedef prog_uchar const font_pgm_t[7];
typedef uint8_t font_t[7];

extern font_pgm_t font_heart[1]; // Fonts for custom callback function:
	// dotmatrix_babsi

extern font_pgm_t font_walker[3]; // Fonts for custom callback function:
	// dotmatrix_walker

extern font_pgm_t font_unknown[1]; // Unknown character

extern font_pgm_t font_alpha[26]; // The alphabetic characters, upper case

extern font_pgm_t font_num[10]; // Digits 0-9

#endif // FONTS_H
