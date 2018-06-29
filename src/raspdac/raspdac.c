#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define SHUTDOWN 3
#define POWEROFF 4
#define SWITCH_N 2
#define LED 1
#define POWER_N 0

#define switch_pressed() \
	(~PINB & 1<<SWITCH_N)

// about 500 ms at prescaler 1024
#define BLINK_COUNT_MAX 2

volatile static bool on;
volatile static bool blink_state;
volatile static char blink_count;

ISR(INT0_vect) {
	PORTB &= ~(1<<POWER_N);
	PORTB |= 1<<LED;
	do _delay_ms(100); while (switch_pressed());
	GIMSK = 1<<PCIE;
	GIFR = 0;
	PCMSK = 1<<POWEROFF | 1<<SWITCH_N;
	on = true;
}

ISR(TIMER0_OVF_vect) {
	blink_count++;
	if (blink_count == BLINK_COUNT_MAX) {
		blink_count = 0;
		if (blink_state) {
			PORTB &= ~(1<<SHUTDOWN);
			PORTB &= ~(1<<LED);
		} else {
			PORTB |= 1<<SHUTDOWN;
			PORTB |= 1<<LED;
		}
		blink_state = !blink_state;
	}
}

static inline void shutdown(void) {
	PORTB |= 1<<SHUTDOWN;
	PORTB |= 1<<LED;

	// init timer
	TCCR0B = 0; // stop timer
	GTCCR = 1<<PSR0; // reset prescaler
	
	TIFR = 0; // clear timer interrupts
	TIMSK = 1<<TOIE0; // overflow interrupt enable
	TCCR0A = 0; // normal mode, OC0A/OC0B disconnected
	TCCR0B = 1<<CS02|0<<CS01|1<<CS00; // start, clkIO/1024

	blink_count = 0;
	blink_state = true;
}

static inline void poweroff(void) {
		PORTB |= 1<<POWER_N;
		PORTB &= ~(1<<LED);
		TCCR0B = 0; // stop timer
		GIMSK = 0;
		on = false;
}

ISR(PCINT0_vect) {
	if (switch_pressed()) {
		char loop = 30;
		shutdown();
		do {
			_delay_ms(100);
			if (loop) loop--;
			else if (on) poweroff();
		} while (switch_pressed());
	}
	if (PINB & 1<<POWEROFF) {
		poweroff();
		_delay_ms(100);
	}
}

int main() {
main:
	cli();
	PORTB = 0<<SHUTDOWN | 0<<POWEROFF | 1<<SWITCH_N | 0<<LED | 1<<POWER_N;
	DDRB  = 1<<SHUTDOWN | 0<<POWEROFF | 0<<SWITCH_N | 1<<LED | 1<<POWER_N;
	_delay_ms(100);
	GIMSK = 1<<INT0;
	GIFR = 0;
	sei();
	do {
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode();
	} while (!on);
	do {
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();
	} while (on);
	goto main;
	return 0;
}
