#include <avr/io.h>
#include <util/delay.h>

int main(void) {
	DDRB = 1<<PB2;
	while (1) {
		_delay_us(0.5/32768*1000*1000);
		PORTB = ~PORTB & 1<<PB2;
	}
	return 0;
}

