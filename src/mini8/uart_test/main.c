// Test the UART at high speeds.
// At reset a welcome message is sent, then loopback is performed.
// For test of the U2X flag with the 'uart' module.
// Tested up to 115200/8N1 with USB-to-serial-converter.

#include "mini8/scrolltext.h"
#include "mini8/uart.h"
#include "mini8/key.h"

#include <stdio.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

void reset(void) {
	wdt_enable(WDTO_15MS);
	while (1);
}

int main() {
	scrolltext_init();
	uart_init(BAUDRATE_U2X(115200));
	stdout = stdin = stderr = &uart_stream;
	key_init();
	sei();
	scrolltext_puts_p(PSTR("UART TEST MINI8"));
	uart_puts_p(PSTR("uart_test_mini8\n"));
	while (1) {
/*
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_enable();
		sleep_mode();
		sleep_disable();
*/
		if (uart_rx()) {
			char c = uart_getc();
			uart_putc(c);
		}
		if (key2_pressed() || key1_pressed()) {
			reset();
		}
	}
	return 0;
}

