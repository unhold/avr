////////////////////////////////////////////////////////////////////////////////
// file       : scrolltext.c
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, 5*7 dotmatrix with row decoder (mini8)
// description: put scrolltext on the dotmatrix display
//            : interrupt driven timing using Timer2
//            : sleeps MCU in blocking calls
////////////////////////////////////////////////////////////////////////////////

#include "scrolltext.h"

#define TIMER2_OCR ((uint32_t)(F_CPU)/(256L*7L*(uint32_t)(SCROLL_REFRESH)))
	// Calculate the Timer2 OCR value
#define SCROLL_COUNT (1L*(SCROLL_TIME)*(SCROLL_REFRESH)/1000L)
	// Calculate the refresh counter value

volatile static uint8_t frame[7]; // Holds the currently displayed frame
volatile static uint8_t line; // Index of the currently displayed line of frame
volatile static uint8_t offset; // Offset of the lines on the display
volatile static uint8_t idle_count;
	// Number of lines to display before deactivating the ISR

// Display queue ring buffer
volatile static char buffer[SCROLL_BUF_SIZE];
volatile static uint8_t head;
volatile static uint8_t tail;

// Advance a display queue ring buffer index
static inline uint8_t buf_inc(uint8_t buf_idx) {
	return (buf_idx == SCROLL_BUF_SIZE-1) ? 0 : buf_idx+1;
}

// Put an address on the address lines
// address range is from 0 to 6
static inline void put_address(uint8_t address) {
	#if SCROLL_REVERSE
		address = 6 - address;
	#endif // SCROLL_REVERSE
	PORTB = (PORTB & ~(1<<3|1<<4|1<<5)) | (((address) << 3) & (1<<3|1<<4|1<<5));
}

// Put a data on the data lines
// The lower 5 bits are the used data
static inline void put_data(uint8_t data) {
	// This code is optimized for the AVR instruction set and compiler
	# if SCROLL_REVERSE
		PORTC &= ~(1<<PC5);	if (data&1<<0) PORTC |= 1<<PC5;
		PORTD &= ~(1<<PD7);	if (data&1<<1) PORTD |= 1<<PD7;
		PORTD &= ~(1<<PD6);	if (data&1<<2) PORTD |= 1<<PD6;
		PORTD &= ~(1<<PD5);	if (data&1<<3) PORTD |= 1<<PD5;
		PORTD &= ~(1<<PD4);	if (data&1<<4) PORTD |= 1<<PD4;	
	#else // SCROLL_REVERSE
		/* Old version, needs more instructions
		PORTD = (PORTD & ~(1<<4|1<<5|1<<6|1<<7)) |
		((data << 4) & (1<<4|1<<5|1<<6|1<<7)),
		PORTC = (PORTC & ~(1<<5)) | ((data<<1) & (1<<5));
		*/
		PORTC &= ~(1<<PC5);	if (data&1<<0) PORTC |= 1<<PC5;
		PORTD &= ~(1<<PD4);	if (data&1<<1) PORTD |= 1<<PD4;
		PORTD &= ~(1<<PD5);	if (data&1<<2) PORTD |= 1<<PD5;
		PORTD &= ~(1<<PD6);	if (data&1<<3) PORTD |= 1<<PD6;
		PORTD &= ~(1<<PD7);	if (data&1<<4) PORTD |= 1<<PD7;
	#endif
}

// Put a data on the data lines, according to the current line and offset
static inline void put_data_offset(void) {
	put_data(frame[(line+offset > 6 ? line+offset-7 : line+offset)]);
}

// Advance the line
static inline void next_line(void) {
	line = (line == 6 ? 0 : line+1);
}

// Advance the offset
static inline void next_offset(void) {
	offset = (offset == 0 ? 6 : offset-1);
}

// Read the data line i of character c
static inline uint8_t read_line(char c, uint8_t i) {
	return pgm_read_byte(smallfont_lookup(c) + i);
}

// Non-blocking ISR: I-flag is immediateley acitvated as the first action
ISR(TIMER2_COMP_vect) {
	static uint8_t frame_count;
	if (frame_count == 0) { // time to shift the display content
		static char character; // the actual character 
		static uint8_t index; // the actual line in the character
		frame_count = SCROLL_COUNT;
		if (frame[offset]) { // if the last line written was not zero, 
			// continue with the actual character
			++index; // next line of the character
		} else {
			index = 0;
			if (head == tail) { // buffer empty, put empty line to display
				character = 0;
				if (idle_count) { // let the display content shift out
					// before deactivating the ISR
					--idle_count;
				} else {
					TIMSK &= ~(1 << OCIE2); // deactivate ISR
				}
			} else { // character in buffer, read from buffer
				tail = buf_inc(tail);
				character = buffer[tail];
			}
		}
		next_offset(); // next offset,
			// because frame buffer is also a ring bufer
		frame[offset] = character ? read_line(character, index) : 0;
			// actualy read the new line
	} else {
		--frame_count;
	}
	// multiplex the frame buffer to the display
	next_line(); // advance the line number
	put_address(line); // put the line number on the address pins
	put_data_offset(); // put the line content on the data pins
}

// Initialize the module
void scrolltext_init() {
	// Set the address pins to output, 0
	PORTB &= ~(1<<3|1<<4|1<<5);
	DDRB |= 1<<3|1<<4|1<<5;
	// Set the data pins to output, 0
	PORTD &= ~(1<<4|1<<5|1<<6|1<<7);
	DDRD |= 1<<4|1<<5|1<<6|1<<7;
	PORTC &= ~(1<<5);
	DDRC |= 1<<5;
	// Initialize the scrolltext state machine
	line = 6; 
	do {
		frame[line] = 0;
	} while (--line);
	//line = 0; // This happens implicitly
	// Initialize the strobe timer
	TCCR2 = 1<<WGM21 | 0<<WGM20 | 1<<CS22 | 1<<CS21 | 0<<CS20;
		// ctc mode, oc2 disconnected, clk/256
	OCR2 = TIMER2_OCR; // 111 ... 60 Hz @ 12 MHz clk
	TIMSK &= ~(1 << TOIE2); // overflow interrupt disabled
	TIMSK &= ~(1 << OCIE2); // compare match interrupt disabled
}

void scrolltext_putc(char const c) {
#if SCROLL_SLEEP
	if (buf_inc(head) == tail) {
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_enable();
		while (buf_inc(head) == tail) {
			sleep_cpu();
		}
		sleep_disable();
	}
#else // SCROLL_SLEEP
	while (buf_inc(head) == tail);
#endif // SCROLL_SLEEP
	head = buf_inc(head);
	buffer[head] = c;
	idle_count = 6; // reset idle count
	TIMSK |= (1 << OCIE2);
}

void scrolltext_puts(char const *s) {
	char c;
	while ((c = *(s++))) {
		scrolltext_putc(c);
	}
}

void scrolltext_puts_p(prog_char const *s) {
	char c;
	while ((c = pgm_read_byte(s++))) {
		scrolltext_putc(c);
	}
}

uint8_t scrolltext_full(void) {
	return buf_inc(head) == tail;
}

uint8_t scrolltext_empty(void) {
	return head == tail;
}

void scrolltext_wait(void) {
#if SCROLL_SLEEP
	if (idle_count > 1) {
		// Not (idle_count != 0) because this would risk a race condition,
	    // and the MCU could sleep forever! 
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_enable();
		while (idle_count > 1) {
			// Test again here _before_ going to sleep
			// to reduce risk of race condition
			// Race condition if TIMER2_COMP_vect interrupt is disabled
			// inside the ISR here, but it's (nearly?) impossible that _two_
			// TIMER2_COMP_vect interrupts happen here
			sleep_cpu();
			// Woken up by TIMER2_COMP_vect or any other interrupt
		}
		sleep_disable();
	}
#else // SCROLL_SLEEP
	while (idle_count > 1);
#endif // SCROLL_SLEEP
}
