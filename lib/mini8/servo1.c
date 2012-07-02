////////////////////////////////////////////////////////////////////////////////
// file       : servo1.c
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, RC-servos on PB1/OC1A and PB2/OC1B
// description: position two RC-servos
//            : PWM generation using Timer1
////////////////////////////////////////////////////////////////////////////////

#include "servo1.h"

volatile uint8_t servo_flags;
volatile uint16_t servo1_dest;
volatile uint16_t servo2_dest;

volatile static uint16_t wait_time;

#define ABS(_VAL_) \
	((_VAL_ < 0) ? -_VAL_ : _VAL_)

void servo_init(void) {
	servo_flags = SERVO_DEFAULT_FLAGS;
	TCCR1A = 1<<COM1A1 | 0<<COM1A0 | // Clear OC1A on compare match, set at bottom
		1<<COM1B1 | 0<<COM1B0 | // Clear OC1B on compare match, set at bottom
		1<<WGM11 | 0<<WGM10; // ->
	TCCR1B = 1<<WGM13 | 1<<WGM12 | // Fast PWM, top = ICR1
		0<<CS12 | 0<<CS11 | 0<<CS10; // Timer stopped
	OCR1A = ((uint16_t)(SERVO1_ZERO));
	OCR1B = ((uint16_t)(SERVO2_ZERO));
	ICR1 = (uint16_t)(30000-1); // 20ms
	DDRB |= 1<<PB1|1<<PB2;
}

void servo_wait(uint16_t time) {
	wait_time = time;
	TIMSK |= 1<<TOIE1;
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();
	while (wait_time != 0) { // wait until time elapsed
		sleep_cpu();
	}
	sleep_disable();
}

uint16_t servo1_val(int8_t pos) {
	if (servo_flags & (1<<SERVO_REV1_FLAG)) pos = -pos;
	if (pos < 0) {
		return (uint16_t)(SERVO1_ZERO) - ((((uint16_t)(SERVO1_ZERO) - (SERVO1_LEFT))>>1) * (-pos) >> 6);
	} else if (pos > 0) {
		return (uint16_t)(SERVO1_ZERO) + ((((uint16_t)(SERVO1_RIGHT) - (SERVO1_ZERO))>>1) * pos >> 6);
	} else { // (pos == 0)
		return (SERVO1_ZERO);
	}
}

uint16_t servo2_val(int8_t pos) {
	if (servo_flags & (1<<SERVO_REV2_FLAG)) pos = -pos;
	if (pos < 0) {
		return (uint16_t)(SERVO2_ZERO) - ((((uint16_t)(SERVO2_ZERO) - (SERVO2_LEFT))>>1) * (-pos) >> 6);
	} else if (pos > 0) {
		return (uint16_t)(SERVO2_ZERO) + ((((uint16_t)(SERVO2_RIGHT) - (SERVO2_ZERO))>>1) * (pos) >> 6);
	} else { // (pos == 0)
		return (SERVO2_ZERO);
	}
}

ISR(TIMER1_COMPA_vect) {
	if (ABS(OCR1A - servo1_dest) <= (uint16_t)(SERVO1_STEP)) {
		OCR1A = servo1_dest;
		TIMSK &= ~(1<<OCIE1A);
	} else if (OCR1A < servo1_dest) {
		OCR1A += (SERVO1_STEP);
	} else { // (OCR1A > servo1_dest)
		OCR1A -= (SERVO1_STEP);
	}	
	
}

ISR(TIMER1_COMPB_vect) {
	if (ABS(OCR1B - servo2_dest) <= (uint16_t)SERVO2_STEP) {
		OCR1B = servo2_dest;
		TIMSK &= ~(1<<OCIE1B);
	} else if (OCR1B < servo2_dest) {
		OCR1B += SERVO2_STEP;
	} else { // (OCR1B > servo2_dest)
		OCR1B -= SERVO2_STEP;
	}		
}

ISR(TIMER1_OVF_vect) {
	if (wait_time == 0) {
		TIMSK &= ~(1<<TOIE1);
	} else {
		--wait_time;
	}
}

void servo1_wait(void) {
	set_sleep_mode(SLEEP_MODE_IDLE); 
	sleep_enable();
	while (TIMSK & 1<<OCIE1A) {
		sleep_cpu();
	}
	sleep_disable();
}

void servo2_wait(void) {
	set_sleep_mode(SLEEP_MODE_IDLE); 
	sleep_enable();
	while (TIMSK & 1<<OCIE1B) {
		sleep_cpu();
	}
	sleep_disable();
}
