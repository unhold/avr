#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "firefly.h"

#define DDR DDRB
#define PIN PINB
#define PORT PORTB
#define A PB4 // OC1B
#define K PB3
#define DSCHG 150

static inline uint8_t dark(void) {
	uint8_t result;
	PORT &= ~(1<<A);
	PORT |= 1<<K;
	_delay_ms(1);
	DDR &= ~(1<<K);
	PORT &= ~(1<<K);
	_delay_ms(DSCHG);
	result = (PIN&(1<<K)) != 0;
	DDR |= 1<<K;
	PORT &= ~(1<<K);
	return result;
}

/*
static inline void blink(uint8_t count) {
	while (count--) {
		uint8_t i;
		for (i = 0; i < 200; ++i) {
			PORT |= 1<<A;
			_delay_us(100);
			PORT &= ~(1<<A);
			_delay_us(100);
		}
		PORT &= ~(1<<A);
		if (i != 199) {
			_delay_ms(160);
		}
	}
}
*/

void play(void) {
	register uint8_t i = 0, d = 0;
	// init timer
	GTCCR = 1<<PWM1B|1<<COM1B1|0<<COM1B0; // set OC1B at compare match	
	TCCR1 = 1<<CS10; // enable, clk/1
	while ((d = pgm_read_byte(&firefly[i++]))) {
		uint8_t n;
		OCR1B = d;
		for (n = 0; n < 5; ++n) {
			// poll compare match
			// TODO: sleep/interrupt
			while(!(TIFR&1<<TOV1));
			TIFR &= 1<<TOV1;
		}
	}
	// disable timer
	TCCR1 = 0;
}

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
void wdt_init(void) {
	MCUSR = 0;
	wdt_disable();
}

ISR(WDT_vect, ISR_NAKED) {
	reti();
}

int main(void) {
	uint8_t i = 0;
	PRR = 1<<PRTIM1|1<<PRTIM0|1<<PRUSI|1<<PRADC;
	DDR = 1<<A|1<<K;
	sei();
	play();
	play();
	while (1) {
		WDTCR = 1<<WDIE|1<<WDP3|0<<WDP2|0<<WDP1|1<<WDP0; // 8s
		if (!i) {
			if (dark()) {
				PRR &= ~(1<<PRTIM1);
				play();
				PRR |= 1<<PRTIM1;
				WDTCR = 1<<WDIE|0<<WDP3|1<<WDP2|1<<WDP1|1<<WDP0; // 8s
			} else {
				i = 7;
			}
		} else {
			--i;
		}
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode();
	}
	return 0;
}

