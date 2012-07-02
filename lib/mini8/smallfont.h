////////////////////////////////////////////////////////////////////////////////
// file       : smallfont.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : 5*5 dotmatrix (or bigger)
// description: character fonts for a 5*5 dotmatrix display, stored in Flash
//            : fonts have variable width, making them suitabe for scrolltext
////////////////////////////////////////////////////////////////////////////////

#ifndef _SMALLFONT_INCLUDED_
#define _SMALLFONT_INCLUDED_

#include <inttypes.h>
#include <avr/pgmspace.h>

#ifdef LIBCONF
#	include "libconf.h"
#else // LIBCONF
/* not working at the moment
#	define SMALLFONT_ALPHA 1 // Enable alphabetic characters
#	define SMALLFONT_DIGIT 1 // Enable digits
#	define SMALLFONT_PUNCT1 1 // Enable common punctuation characters
*/
#	define SMALLFONT_PUNCT2 0 // Enable seldom punctuation characters
#endif // LIBCONF

// Type for a font with 5 pixels height and variable width.
// Pixels are stored in the lower 5 bits.
// Font is terminated by zero.
typedef prog_uchar smallfont_at[];

// Type for a pointer to a font.
typedef prog_uchar* smallfont_pt;

// Return a pointer to the font for the character.
smallfont_pt smallfont_lookup(char);

#endif // _SMALLFONT_INCLUDED_
