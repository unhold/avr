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

volatile bool on;

#define switch_pressed() \
	(~PINB & 1<<SWITCH_N)

ISR(INT0_vect) {
	PORTB &= ~(1<<POWER_N);
	PORTB |= 1<<LED;
	do _delay_ms(100); while (switch_pressed());
	GIMSK = 1<<PCIE;
	GIFR = 0;
	PCMSK = 1<<POWEROFF | 1<<SWITCH_N;
	on = true;
}

void poweroff(void) {
	PORTB |= 1<<POWER_N;
	PORTB &= ~(1<<LED);
	GIMSK = 0;
	on = false;
}

ISR(PCINT0_vect) {
	if (switch_pressed()) {
		char loop = 30;
		PORTB |= 1<<SHUTDOWN;
		do {
			_delay_ms(100);
			if (loop) --loop;
			else if (on) poweroff();
		} while (switch_pressed());
	}
	if (PINB & 1<<POWEROFF) {
		poweroff();
		_delay_ms(100);
	}
	GIFR = 0;
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
	//while (!on);
	//while (on);
	goto main;
	return 0;
}
