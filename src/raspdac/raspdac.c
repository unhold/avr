#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

// Port B pin numbers
#define SHUTDOWN 3 // OUT: Shutdown request to RaspberryPi.
#define POWEROFF 4 // IN : Poweroff confirmation from RaspberryPi.
#define SWITCH_N 2 // IN : Low-active power switch signal.
#define LED 1      // OUT: LED on power switch.
#define POWER_N 0  // OUT: Low-active RaspberryPi power enable.

#define SWITCH_PRESSED() \
	(~PINB & 1<<SWITCH_N)

// About 500 ms at prescaler 1024
#define BLINK_COUNT_MAX 2

volatile static bool on;
volatile static bool blink_state;
volatile static char blink_count;

// Interrupt on power switch when RaspberryPi is off.
// Power on RaspberryPi and LED.
// Disable INT0. Enable PCINT0.
ISR(INT0_vect) {
	PORTB &= ~(1<<POWER_N);
	PORTB |= 1<<LED;
	do _delay_ms(100); while (SWITCH_PRESSED());
	GIMSK = 1<<PCIE;
	GIFR = 0;
	PCMSK = 1<<POWEROFF | 1<<SWITCH_N;
	on = true;
}

// Interrupt on timer overflow, used to blink LED.
// Pulse shutdown request to make sure an edge-sensitive interrupt is triggered.
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

// Send shutdown request to RaspberryPi and start blinking LED.
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

// Power off RaspberryPi.
static inline void poweroff(void) {
		PORTB |= 1<<POWER_N;
		PORTB &= ~(1<<LED);
		TCCR0B = 0; // stop timer
		GIMSK = 0;
		on = false;
}

// Interrupt on power switch when RaspberryPi is on,
// or RaspberryPi confirmes poweroff.
ISR(PCINT0_vect) {
	// Short press to send shutdown request to RaspberryPi.
	// Long press for hard shutdown.
	if (SWITCH_PRESSED()) {
		char loop = 30;
		shutdown();
		do {
			_delay_ms(100);
			if (loop) loop--;
			else if (on) poweroff();
		} while (SWITCH_PRESSED());
	}
	// Power off if RaspberryPi confirmes poweroff.
	if (PINB & 1<<POWEROFF) {
		poweroff();
		_delay_ms(100);
	}
}

// Timed sequence to avoid race condition.
#define SLEEP_WHILE(_condition, _sleep_mode) \
	do { \
		set_sleep_mode(_sleep_mode); \
		cli(); \
		if (_condition) { \
			sleep_enable(); \
			sei(); \
			sleep_cpu(); \
			sleep_disable(); \
		} \
	} while(_condition)

int main() {
	while (true) {
		cli();
		PORTB = 0<<SHUTDOWN | 0<<POWEROFF | 1<<SWITCH_N | 0<<LED | 1<<POWER_N;
		DDRB  = 1<<SHUTDOWN | 0<<POWEROFF | 0<<SWITCH_N | 1<<LED | 1<<POWER_N;
		_delay_ms(100);
		GIMSK = 1<<INT0;
		GIFR = 0;
		sei();
		SLEEP_WHILE(!on, SLEEP_MODE_PWR_DOWN);
		SLEEP_WHILE(on, SLEEP_MODE_IDLE);
	}
	return 0;
}
