// Photinus consanguineus

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "firefly.h"

#define DDR DDRB
#define PIN PINB
#define PORT PORTB
#define A PB0 // OC0A
#define K PB2

/* Pin configuration
        +----------+
   PB5--| 1*     8 |-- VCC -->
   PB3--| 2      7 |-- PB2 -----+----.
|--PB4--| 3      6 |-- PB1      | \\| )
|--GND--| 4      5 |-- PB0 -----+----°
        +----------+
*/

#define GLOW_TIME   60
#define SLEEP_TIME 600

static inline void glow(uint8_t scale) {
	prog_uint8_t const* p = firefly;
	uint8_t data, loop;
	DDR = 1<<A|1<<K;
	PRR = 1<<PRADC; // Enable timer by clearing PRTIM0
	// Timer/Counter0 with clkI/O.
	// Fast PWM mode, 0 to 255.
	// Clear OC0A on compare match, set at TOP.
	TCCR0A = 1<<WGM01|1<<WGM00 | 1<<COM0A1|0<<COM0A0;
	TCCR0B = 0<<WGM02 | 0<<CS02|0<<CS01|1<<CS00;
	TIMSK0 = 1<<TOIE0;
	do {
		data = pgm_read_byte(p++);
		OCR0A = data;
		loop = scale;
		while (loop--) {
			set_sleep_mode(SLEEP_MODE_IDLE);
			sleep_cpu();
			//while ( !(TIFR0&1<<TOV0) );
			//TIFR0 = 1<<TOV0;
			// Done by sleep_cpu().
		}
	} while (data);
	PRR = 1<<PRTIM0|1<<PRADC;
	//TCCR0B = 0; // Stop clk, disable timer. // Done by PRR.
	DDR = 0;
}

EMPTY_INTERRUPT(TIM0_OVF_vect);

static inline void wdt_init(void) {
	sleep_enable();
	sei();
}

static inline void wdt_sleep(uint8_t value) {
	WDTCR = 1<<WDCE;
	WDTCR = 1<<WDTIE|(value&0x8?1<<WDP3:0)|(value&(1<<WDP2|1<<WDP1|1<<WDP0));
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_cpu();
	WDTCR = 0;
}

EMPTY_INTERRUPT(WDT_vect);

static inline void sleep_2min(void) {
	uint8_t count = 15;
	WDTCR = 1<<WDCE;
	WDTCR = 1<<WDTIE|(WDTO_8S&0x8?1<<WDP3:0)|(WDTO_8S&(1<<WDP2|1<<WDP1|1<<WDP0));
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	while (count--) sleep_cpu();
	WDTCR = 0;
}

static inline uint8_t dark(void) {
	uint8_t result;
	PORT = 1<<K;
	DDR = 1<<A|1<<K;
	//_delay_us(100); // Done by wdt_sleep().
	wdt_sleep(WDTO_15MS);
	DDR = 1<<A;
	PORT = 1<<A;
	//_delay_ms(150); // Done by wdt_sleep().
	wdt_sleep(WDTO_120MS);
	result = (PIN&(1<<K)) != 0;
	DDR = 0;
	PORT = 0;
	return result;
}

// Takes approx. 5.7sec.
static inline void glow_phot_cons(uint8_t gender) {
	glow(gender+1);
	wdt_sleep(WDTO_250MS);
	wdt_sleep(WDTO_60MS);
	if (!gender) glow(gender+1);
	wdt_sleep(WDTO_4S);
	//wdt_sleep(WDTO_1S); // Photinus consanguineus would wait a second longer, but it looks better without.
}

int main(void) {
	uint8_t gender;
	PRR = 1<<PRTIM0|1<<PRADC;
	PORTB = 1<<PB4; // Pull-up on PB4.
	wdt_init();
	gender = (~PINB)>>PB4 & 1; // 0: male, 1: female.
	while (1) {
		{
			// Glow for 4h or until 10min bright.
			uint8_t bright_count = 0;
			uint8_t glow_count = 0;
			while ((glow_count < 120) && (bright_count < 5)) {
				uint8_t count = 21;
				while (count--) glow_phot_cons(gender);
				++bright_count;
				if (dark()) bright_count = 0;
			}
		}		{
			// Wait for 1h bright.
			uint8_t bright_count = 0;
			while (bright_count < 30) {
				sleep_2min();
				++bright_count;
				if (dark()) bright_count = 0;
			}
		}
		{
			// Wait for 30min dark.
			uint8_t dark_count = 0;
			while (dark_count < 15) {
				sleep_2min();
				++dark_count;
				if (!dark()) dark_count = 0;
			}
		}

	}
	return 0;
}

