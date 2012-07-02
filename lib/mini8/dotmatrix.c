////////////////////////////////////////////////////////////////////////////////
// file       : dotmatrix.c
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, 5*7 dotmatrix with row decoder (mini8)
// description: put text on the dotmatrix display, showing one letter by one
//            : interrupt driven timing using Timer2
////////////////////////////////////////////////////////////////////////////////

#include "dotmatrix.h"

#define TIMER2_OCR_VALUE \
	((F_CPU)/(256L*7L*(DOTMATRIX_REFRESH_RATE)))
		// Calculate the Timer2 OCR value
#define CALLBACK_AFTER \
	(7L*(DOTMATRIX_CALLBACK_TIME)*(DOTMATRIX_REFRESH_RATE)/1000L)
		// Calculate the callback counter value

static volatile font_t font;
static volatile uint8_t line;
static volatile dotmatrix_callback_t callback;

#define dotmatrix_adress(_adress_) \
	PORTB = (PORTB & ~(1<<3|1<<4|1<<5)) | (((_adress_) << 3) & (1<<3|1<<4|1<<5))

#define dotmatrix_data(_data_) \
	PORTD = (PORTD & ~(1<<4|1<<5|1<<6|1<<7)) | \
		(((_data_) << 4 ) & (1<<4|1<<5|1<<6|1<<7)); \
	PORTC = (PORTC & ~(1<<5)) | ((_data_<<1) & (1<<5))

#define next_line() \
	line = (line == 6 ? 0 : line+1)

#ifdef DOTMATRIX_REVERSE
inline static char bitreverse(char c) {
	return (c & 1<<0) << 4 |
		   (c & 1<<1) << 2 |
		   (c & 1<<2) |
		   (c & 1<<3) >> 2 |
		   (c & 1<<4) >> 4;
}
#endif // DOTMATRIX_REVERSE

void dotmatrix_init(void) {
	// Set the adress pins to output, 0
	PORTB &= ~(1<<3|1<<4|1<<5);
	DDRB |= 1<<3|1<<4|1<<5;
	// Set the data pins to output, 0
	PORTD &= ~(1<<4|1<<5|1<<6|1<<7);
	DDRD |= 1<<4|1<<5|1<<6|1<<7;
	PORTC &= ~(1<<5);
	DDRC |= 1<<5;
	// Initialize the dotmatrix state machine
	dotmatrix_putfont_p(font_unknown[0]);
	line = 0;
	callback = 0;
	// Initialize the strobe timer
	TCCR2 = 1<<WGM21 | 0<<WGM20 | 1<<CS22 | 1<<CS21 | 0<<CS20;
		// ctc mode, oc2 disconnected, clk/256
	OCR2 = TIMER2_OCR_VALUE; // 111 ... 60 Hz @ 12 MHz clk
	TIMSK &= ~(1 << TOIE2); // overflow interrupt disabled
	TIMSK |= (1 << OCIE2); // compare match interrupt enabled
}

void dotmatrix_set_callback(dotmatrix_callback_t c) {
	callback = c;
}

void dotmatrix_putc(uint8_t c) {
	if (c >= 'a' && c <= 'z') {
		// upper- and lower-case letters are shown as uppercase
		dotmatrix_putfont_p(font_alpha[c-'a']);
	} else if (c >= 'A' && c <= 'Z') {
		dotmatrix_putfont_p(font_alpha[c-'A']);
	} else if ( c >= '0' && c <= '9') {
		// its a number
		dotmatrix_putfont_p(font_num[c-'0']);
	} else if (c == ' ') {
		// its a space
		// no special font necessary, just clear every pixel
		uint8_t i;
		for (i = 0; i != 7; ++i) {
			font[i] = 0;
		}
	} else if (c == '#') { // the '#' character is decoded as a heart
		dotmatrix_putfont_p(font_heart[0]);
	// special characters can be added here
	} else {
		dotmatrix_putfont_p(font_unknown[0]);
	}
}

void dotmatrix_putfont(font_t f) {
	uint8_t i;
	for (i = 0; i != 7; ++i) {
		#ifdef DOTMATRIX_REVERSE
			font[i] = f[i];
		#else // DOTMATRIX_REVERSE
			font[i] = bitreverse(f[i]);
		#endif // DOTMATRIX_REVERSE
	}
}

void dotmatrix_putfont_p(font_pgm_t f) {
	uint8_t i;
	for (i = 0; i != 7; ++i) {
		#ifdef DOTMATRIX_REVERSE
			font[i] = pgm_read_byte(f+i);
		#else // DOTMATRIX_REVERSE
			font[i] = bitreverse(pgm_read_byte(f+i));
		#endif // DOTMATRIX_REVERSE
	}
}

void dotmatrix_walker(void) {
	uint8_t const index[] = 
		"\00\00\01\01\00\00\01\01\02\02\01\01\02\02\01\01 stayin alive ";
	static uint8_t frame;
	if (index[frame] < 3) {
		dotmatrix_putfont_p(font_walker[index[frame]]);
	} else {
		dotmatrix_putc(index[frame]);
	}
	frame = (frame == sizeof(index)-2 ? 0 : frame + 1);
}

void dotmatrix_babsi(void) {
	uint8_t const index[] = "christian # # barbara   ";
	static uint8_t frame;
	dotmatrix_putc(index[frame]);
	++frame;
	if (!index[frame]) frame = 0;
}

ISR(TIMER2_COMP_vect) {
	static uint16_t count = CALLBACK_AFTER;
	TIMSK &= ~(1 << OCIE2);
	sei();
	#ifdef DOTMATRIX_REVERSE
		dotmatrix_adress(6-line);
	#else // DOTMATRIX_REVERSE
		dotmatrix_adress(line);
	#endif // DOTMATRIX_REVERSE
	dotmatrix_data(font[line]);
	next_line();
	if (callback != 0 && --count == 0) {
		count = CALLBACK_AFTER;
		callback();
	}
	TIMSK |= (1 << OCIE2);
}
