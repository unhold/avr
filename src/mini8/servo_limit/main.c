#include "mini8/dotmatrix.h"
#include "mini8/key.h"

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#define SERVO_CYCLE 28169
#define SERVO_ZERO   2131
#define SERVO_MIN    1222  
#define SERVO_MAX    3060

inline void servo_init(void) {
	OCR1A = (uint16_t)(SERVO_CYCLE-1);
	OCR1B = (uint16_t)SERVO_ZERO;
	DDRB |= 1<<PB2; // Set OC1B to output
	TCCR1A = 0<<COM1A1 | 0<<COM1A0 | // OC1A disconnected
		1<<COM1B1 | 0<<COM1B0 | // Clear OC1B on compare match, set at bottom
		1<<WGM11 | 1<<WGM10; // ->
	TCCR1B = 1<<ICNC1 | // Noise canceler on input capture enabled
		1<<ICES1 | // Input capture detects rising edge
		1<<WGM13 | 1<<WGM12 | // Fast PWM, top = OCR1A
		0<<CS12 | 1<<CS11 | 0<<CS10; // Prescaler 8
	TIMSK |= 1<<TICIE1 | // Enable input capture interrupt
		1<<OCIE1B; // Enable output compare B match interrupt
}

static int16_t servo_low_limit;
static int16_t servo_high_limit;

static inline uint16_t transform(int16_t time) {
	if      (time < servo_low_limit)  return servo_low_limit;
	else if (time > servo_high_limit) return servo_high_limit;
	else                              return time;
}

void set_servo_limit(uint8_t servo_level) {
	switch (servo_level) {
	case 0:
		servo_low_limit  =  0;
		servo_high_limit =  0x7FFF;
		break;
	case 1:
		servo_low_limit  =  1647;
		servo_high_limit =  2674;
		break;
	case 2:
		servo_low_limit  =  1727;
		servo_high_limit =  2585;
		break;
	case 3:
		servo_low_limit  =  1794;
		servo_high_limit =  2510;
		break;
	case 4:
		servo_low_limit  =  1849;
		servo_high_limit =  2447;
		break;
	case 5:
		servo_low_limit  =  1896;
		servo_high_limit =  2395;
		break;
	case 6:
		servo_low_limit  =  1935;
		servo_high_limit =  2352;
		break;
	case 7:
		servo_low_limit  =  1967;
		servo_high_limit =  2315;
		break;
	case 8:
		servo_low_limit  =  1994;
		servo_high_limit =  2285;
		break;
	case 9:
		servo_low_limit  =  2017;
		servo_high_limit =  2259;
		break;
	}
}

static uint8_t servo_read_valid;
static uint8_t isr_sync;

ISR(TIMER1_CAPT_vect) {
	static uint16_t last_rise, last_fall;
	register uint16_t now = ICR1;
	register uint8_t tccr1b = TCCR1B;
	TCCR1B = tccr1b ^ 1<<ICES1;
	if (tccr1b & 1<<ICES1) { // Rising edge detectet
		last_rise = now;
	} else { // Falling edge detected
		register int16_t time = now - last_fall;
		last_fall = now;
		if (time < 0) time += SERVO_CYCLE; // time = period
		if (time < SERVO_CYCLE*0.9 && time > SERVO_CYCLE*0.1) {
			servo_read_valid = 0;
			OCR1B = (uint16_t)SERVO_ZERO;
		} else {
			time = now - last_rise; // time = high_time
			if (time < 0) time += SERVO_CYCLE;
			if (time < SERVO_MIN || time > SERVO_MAX) {
				servo_read_valid = 0;
				OCR1B = (uint16_t)SERVO_ZERO;
			} else {
				if (servo_read_valid < 2) {
					++servo_read_valid;
					OCR1B = (uint16_t)SERVO_ZERO;
				} else {
					OCR1B = transform(time);
				}
			}
		}
		isr_sync = 0;
	}
}

ISR(TIMER1_COMPB_vect) {
	if (isr_sync > 2) {
		OCR1B = (uint16_t)SERVO_ZERO;
		servo_read_valid = 0;
	}
	isr_sync++;
}

static uint8_t servo_level = 8;
#define SERVO_LEVEL_EEADR (uint8_t *)510

int main(void) {
	servo_level = eeprom_read_byte(SERVO_LEVEL_EEADR);
	set_servo_limit(servo_level);
	key_init();
	servo_init();
	sei();
	dotmatrix_init();
	dotmatrix_putfont_p(font_num[servo_level]);
	_delay_ms(500);
	dotmatrix_off();
	while(1) {
		uint8_t save = 0;
		if (key1_pressed()) {
			if (servo_level > 0) --servo_level;
			set_servo_limit(servo_level);
			dotmatrix_putfont_p(font_num[servo_level]);
			dotmatrix_on();
			while (key1_pressed());
			save = 1;
			_delay_ms(500);
			dotmatrix_off();
		}
		if (key2_pressed()) {
			if (servo_level < 9) ++servo_level;
			set_servo_limit(servo_level);
			dotmatrix_putfont_p(font_num[servo_level]);
			dotmatrix_on();
			while (key2_pressed());
			save = 1;
			key2_wait_release();
			_delay_ms(500);
			dotmatrix_off();
		}
		if (save && !key1_pressed() && !key2_pressed()) {
			save = 0;
			eeprom_write_byte(SERVO_LEVEL_EEADR, servo_level);
		}
	}
	return 0;
}

