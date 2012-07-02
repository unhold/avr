////////////////////////////////////////////////////////////////////////////////
// file       : servo.c
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, RC-servos on PB1/OC1A and PB2/OC1B
// description: position two RC-servos
//            : PWM generation using Timer1
// hints      : Timer1 overflows every 20 ms and can be used for timekeeping
//            : or delay routines, example wait routine provided
////////////////////////////////////////////////////////////////////////////////

#include "servo.h"

volatile static uint16_t wait_time;

void servo_init(void) {
	OCR1A = SERVO_ZERO;
	OCR1B = SERVO_ZERO;
	ICR1 = SERVO_CYCLE-1; // 20ms
	TCCR1A = 1<<COM1A1 | 0<<COM1A0 |
			// Clear OC1A on compare match, set at bottom
		1<<COM1B1 | 0<<COM1B0 | // Clear OC1B on compare match, set at bottom
		1<<WGM11 | 0<<WGM10; // ->
	TCCR1B = 1<<WGM13 | 1<<WGM12 | // Fast PWM, top = ICR1
#	if (SERVO_PRESCALER == 8)
			0<<CS12 | 1<<CS11 | 0<<CS10; // Prescaler 8
#	elif (SERVO_PRESCALER == 64)
			0<<CS12 | 1<<CS11 | 1<<CS10; // Prescaler 64
#	else // SERVO_PRESCALER
#		error "Invalid prescaler setting"
#	endif // SERVO_PRESCALER
	DDRB |= 1<<PB1|1<<PB2; // Servo pins to output
}

uint16_t servo_count(uint8_t position) {
	// Beware of integer overflows in this expression
	return (uint16_t)(SERVO_LEFT) +
		((uint16_t)(((SERVO_RIGHT) - (SERVO_LEFT)) >> 2) * position >> 6);
}

void servo_wait(uint16_t time) {
	wait_time = time;
	TIMSK |= 1<<TOIE1; // Enable Timer1 overflow interrupt
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();
	while (wait_time != 0) { // Wait until time elapsed
		sleep_cpu();
	}
	sleep_disable();
}

ISR(TIMER1_OVF_vect) {
	if (wait_time == 0) {
		TIMSK &= ~(1<<TOIE1); // Self disable
	} else {
		--wait_time;
	}
}

