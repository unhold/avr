#include "mini8/ir.h"

#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// LED on Pin5/PB0.
#define LED_INIT() DDRB |= 1<<PB0
#define LED_ON() PORTB |= 1<<PB0
#define LED_OFF() PORTB &= ~(1<<PB0)

// Low-active key on Pin2/PB3/PCINT3.
#define KEY_INIT() PORTB |= 1<<PB3
#define KEY_ACTIVE() (!(PINB & 1<<PB3))

static void blink(uint8_t count) {
	LED_OFF();
	while (count != 0) {
		--count;
		LED_ON();
		_delay_ms(200);
		LED_OFF();
		_delay_ms(300);
	}
}

static void morse(uint8_t data) {
	uint8_t i = 7;
	// Find the highest set bit in data.
	while ( ((data & 1<<i) == 0) && (i != 0) ) --i;
	// i is now the number of the highest set bit
	// or 0 if data is 0 (no bits set).
	do {
		LED_ON();
		if (data & 1<<i) {
			_delay_ms(500);
			LED_OFF();
			_delay_ms(300);
		} else {
			_delay_ms(100);
			LED_OFF();
			_delay_ms(700);
		}
	} while (i--);
}

static __attribute__((noreturn)) void reset(void) {
	cli();
	static void const (*null)(void) = (void const (*)(void))0;
	null();
	//wdt_enable(WDTO_15MS);
	//while (1);
}

ISR(BADISR_vect, ISR_NAKED) {
	blink(5);
	reset();
}

__attribute__((noreturn)) int main() {
	wdt_disable();
	ir_init();
	LED_INIT();
	LED_ON();
	KEY_INIT();
	// PCINT is already used by ir.c.
	// Add PCINT3 to the sensitivity of PCINT,
	// to allow wakeup from standby.
	// This may confuse the ir.c interrupt handler,
	// but this is no problem here because it triggers a reset then.
	PCMSK |= 1<<PCINT3;
	sei();
	while (1) {
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_enable();
		sleep_mode();
		sleep_disable();
		if (rc5_data) {
			uint16_t data = rc5_data;
			morse(RC5_ADDRESS(data));
			_delay_ms(1600);
			morse(RC5X_COMMAND(data));
			RC5_CLEAR(rc5_data);
		}
		if (KEY_ACTIVE()) {
			reset();
		}
	}
}
