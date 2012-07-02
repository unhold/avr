#include <stdlib.h>
#include <avr/wdt.h>

#include "mini8/error.h"
#include "mini8/key.h"
#include "mini8/temp.h"
#include "mini8/scrolltext.h"

__attribute__((noreturn)) int main() {
	int8_t temp;
	char str[8];
	wdt_disable();
	key_init();
	ls1_init();
	if (key2_pressed()) {
		// Jump to bootloader if present
		reset();
	} else if (key1_pressed() || ls1_active()) {
		// Show temperature on display
		temp_init();
		temp = temp_read();
		itoa(temp, str, 10);
		scrolltext_init();
		sei();
		scrolltext_puts(str);
		scrolltext_wait();
	}
	if (ls1_active()) wdt_enable(WDTO_1S);
	else              wdt_enable(WDTO_250MS);
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_mode();
	while (1);
}
