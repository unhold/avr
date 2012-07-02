////////////////////////////////////////////////////////////////////////////////
// file       : rc5rx.c
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, IR Receiver with demodulator and low-active data line
//            : (like the Vishay TSOP... series) on pin PB0/ICP
// description: Receive RC5 signals from IR remote controls
//            : Fully interrupt driven with low CPU consumption
//            : Uses Timer1 with input capture interrupt
// hints      : Timer1 is running thru (prescaler 256)
//            : May be used for timekeeping purposes
////////////////////////////////////////////////////////////////////////////////

#include "rc5rx.h"

#define _COUNT \
	((double)(F_CPU)*64/((double)(RC5RX_FREQUENCY)*(RC5RX_PRESCALER)))
#define _ERROR ((double)(_COUNT)*(RC5RX_ERROR)/100)
#define _MAXCOUNT ((uint16_t)((_COUNT)+(_ERROR)+0.5))
#define _MINCOUNT ((uint16_t)((_COUNT)-(_ERROR)+0.5))

volatile uint16_t rc5rx_data;

#if RX5RX_CALLBACK
volatile rc5rx_callback_t rc5rx_callback;
#endif // RX5RX_CALLBACK

#if RC5RX_SERVO // With servo function
void rc5rx_init(void) {
	OCR1B = SERVO_ZERO;
//	OCR1A = SERVO_CYCLE-1; // 20ms
	OCR1A = 30000;
	TCCR1A = 0<<COM1A1 | 0<<COM1A0 | // OC1A disconnected
		1<<COM1B1 | 0<<COM1B0 | // Clear OC1B on compare match, set at bottom
		1<<WGM11 | 1<<WGM10; // ->
	TCCR1B = 1<<WGM13 | 1<<WGM12 | // Fast PWM, top = OCR1A
		1<<ICNC1 | // Input noise canceler activated
		0<<ICES1 | // Detect falling edge
#if (RC5RX_PRESCALER == 64)
		0<<CS12 | 1<<CS11 | 1<<CS10; // Prescaler 64
#else // RC5RX_PRESCALER
#	error "Invalid prescaler setting"
#endif // RC5RX_PRESCALER
	TIMSK = (TIMSK & ~(1<<TICIE1|1<<OCIE1A|1<<OCIE1B|1<<TOIE1)) |
		1<<TICIE1; // Only input capure interrupt acitvated
	DDRB |= 1<<PB2; // Servo pin 2 to output
	rc5rx_data = 0;
#if RC5RX_CALLBACK
	rx5rx_callback = 0;
#endif // RC5RX_CALLBACK
}
#else // RC5RX_SERVO // Without servo function
void rc5rx_init(void) {
	TCCR1A = 0; // OC1A/OC1B disconnected
	TCCR1B = 1<<ICNC1 | // Input noise canceler activated
		0<<ICES1 | // Detect falling edge
#if (RC5RX_PRESCALER == 64)
		0<<CS12 | 1<<CS11 | 1<<CS10; // Prescaler 64
#elif (RC5RX_PRESCALER == 256)
		1<<CS12 | 0<<CS11 | 0<<CS10; // Prescaler 256
#else // RC5RX_PRESCALER
#	error "Invalid prescaler setting"
#endif // RC5RX_PRESCALER
	TIMSK = (TIMSK & ~(1<<TICIE1|1<<OCIE1A|1<<OCIE1B|1<<TOIE1)) |
		1<<TICIE1; // Only input capure interrupt acitvated
	rc5rx_data = 0;
#if RC5RX_CALLBACK
	rx5rx_callback = 0;
#endif // RC5RX_CALLBACK
}
#endif // RC5RX_SERVO

ISR(TIMER1_CAPT_vect) {
	static uint16_t rc5rx_tmp;
	static uint16_t oldcount;
	uint16_t count;
	TCCR1B ^= 1<<ICES1; // Revert edge sensitivity
	count = ICR1 - oldcount; // Calculate differrence between counts
#if RC5RX_SERVO
	// Macro RC5RX_TIMER1MAX may contain double expression,
	// can't be handled by CPP
	// Timer not running thru, handle timer overflow
	count %= RC5RX_TIMER1MAX+1;
#elif (RC5RX_TIMER1MAX != 0) && (RC5RX_TIMER1MAX != 0xFFFF)
	// Timer not running thru, handle timer overflow
	count %= RC5RX_TIMER1MAX+1;
#endif
	if (count < _MINCOUNT/2 || count >_MAXCOUNT) {
		// Pulse too short or too long
		rc5rx_tmp = 0;
	}
	if (!rc5rx_tmp || count > _MINCOUNT) { // Start or long pulse
		oldcount = ICR1; // Start counting from now on		rc5rx_tmp <<= 1;
		if (TCCR1B&(1<<ICES1)) { // Falling edge detected
			rc5rx_tmp |= 1;
		}
		if (rc5rx_tmp&(1<<13)) { // 14 Bits received
			rc5rx_data = rc5rx_tmp;
			rc5rx_tmp = 0;
#if RC5RX_CALLBACK
			rx5rx_callback();
#endif // RC5RX_CALLBACK
		}
	}
}

