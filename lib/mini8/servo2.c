////////////////////////////////////////////////////////////////////////////////
// file       : servo2.c
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, RC-servo on PB2/OC1B, RC-servo PWM input on PB0/ICP
// description: position one RC-servo, read an RC-servo PWM signal
//            : PWM generation and input capturing using Timer1
////////////////////////////////////////////////////////////////////////////////

#include "servo2.h"

volatile uint16_t servo_dest;
volatile uint16_t servo_read;
volatile uint8_t servo_flags;

#define ABS(_VAL_) \
	((_VAL_ < 0) ? -_VAL_ : _VAL_)

inline void servo_init(void) {
	servo_flags = SERVO_DEFAULT_FLAGS;
	OCR1A = (uint16_t)(30000-1); // 20ms
	OCR1B = (uint16_t)SERVO_ZERO;
	DDRB |= 1<<PB2; // Set OC1B to output
	TCCR1A = 0<<COM1A1 | 0<<COM1A0 | // OC1A disconnected
		1<<COM1B1 | 0<<COM1B0 | // Clear OC1B on compare match, set at bottom
		1<<WGM11 | 1<<WGM10; // ->
	TCCR1B = 1<<ICNC1 | // Noise canceler on input capture enabled
		1<<ICES1 | // Input capture detects rising edge
		1<<WGM13 | 1<<WGM12 | // Fast PWM, top = OCR1A
		0<<CS12 | 0<<CS11 | 0<<CS10; // Timer stopped
	TIMSK |= 1<<TICIE1; // Enable input capture interrupt
}

uint16_t servo_val(int8_t pos) {
	if (servo_flags & SERVO_REV_FLAG) pos -= pos;
	if (pos < 0) {
		return (uint16_t)SERVO_ZERO - ((((uint16_t)SERVO_ZERO - SERVO_LEFT)>>1) * (-pos) >> 6);
	} else if (pos > 0) {
		return (uint16_t)SERVO_ZERO + ((((uint16_t)SERVO_RIGHT - SERVO_ZERO)>>1) * (pos) >> 6);
	} else { // (pos == 0)
		return SERVO_ZERO;
	}
}

int8_t servo_lav(uint32_t val) {
	if (val < ICP_LEFT || val > ICP_RIGHT) { // val invalid
		servo_flags |= 1<<SERVO_VAL_INVALID_FLAG;
		return 0;
	} else { // val valid
		servo_flags &= ~(1<<SERVO_VAL_INVALID_FLAG);
		if (val < ICP_ZERO) {
			val = ICP_ZERO - val;
			return -((val << 4) / (((uint16_t)ICP_ZERO - ICP_LEFT) >> 3));
		} else if (val > ICP_ZERO) {
			val -= ICP_ZERO;
			return (val << 4) / (((uint16_t)ICP_RIGHT - ICP_ZERO) >> 3);
		} else { // val == ICP_ZERO
			return 0;
		}
	}
}

uint8_t servo_switch(void) {
	int8_t c;
	c = servo_lav(servo_read);
	if (servo_flags & SERVO_VAL_INVALID_FLAG) {
		return SERVO_SWITCH_ERROR;
	}
	if (c > 85) { // on: bachward
		return SERVO_SWITCH_DOWN;
	} else if ( c < -85) { // on: forward
		return SERVO_SWITCH_UP;
	} else { // off
		return SERVO_SWITCH_MIDDLE;
	}
}

ISR(TIMER1_COMPB_vect) {
	if (ABS(OCR1B - servo_dest) <= (uint16_t)SERVO_STEP) {
		OCR1B = servo_dest;
		TIMSK &= ~(1<<OCIE1B);
	} else if (OCR1B < servo_dest) {
		OCR1B += SERVO_STEP;
	} else { // (OCR1B > servo_dest)
		OCR1B -= SERVO_STEP;
	}	
}

ISR(TIMER1_CAPT_vect) {
	static uint16_t last_val;
	if (TCCR1B & 1<<ICES1) { // Falling edge detected
		last_val = ICR1;
	} else { // Rising edge detected	
		if (ICR1 < last_val) { // TOP reached since rising edge
			servo_read = ICR1 + (30000-1) - last_val;
		} else {
			servo_read = ICR1 - last_val;
		}
	}
	TCCR1B ^= 1<<ICES1;
}
