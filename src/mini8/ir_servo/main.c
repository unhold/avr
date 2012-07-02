#include <stdio.h>

#include "mini8/rc5rx.h"
#include "mini8/error.h"
#include "mini8/scrolltext.h"

int main() {
	uint8_t pos = 255;
	uint16_t old, rc5 = 0;
	char s[32];
	scrolltext_init();
	rc5rx_init();
	servo_pos(pos);
	sei();
	scrolltext_puts_p(PSTR("RC5RX SERVO TEST"));
	while (1) {
		if (RC5RX_VALID(rc5rx_data)) {
			old = rc5;
			rc5 = rc5rx_data;
			RC5RX_CLEAR(rc5rx_data);
			if (rc5 != old && RC5RX_ADDRESS(rc5) == 30) {
				switch (RC5RX_COMMAND(rc5)) {
					case 1: case 2:	case 3:
					case 4: case 5: case 6:
					case 7: case 8: case 9:
							case 0:
						pos = ~(255/9*RC5RX_COMMAND(rc5));
						break;
					case 20: // Up
						pos = (pos < 10) ? 0 : pos - 10;
						break;
					case 21: // Down
						pos = (pos > 245) ? 255 : pos + 10;
						break;
					case 23: // Right
						if (pos != 0) pos -= 1;
						break;
					case 22: // Left
						if (pos != 255) pos += 1;
						break;
					default:
						break;
				}
				servo_pos(pos);
				if (scrolltext_empty()) {
					sprintf(s, "%d ", 255-pos);
					scrolltext_puts(s);
				}
			}
		} else {
			set_sleep_mode(SLEEP_MODE_IDLE);
			sleep_enable();
			sleep_cpu();
			sleep_disable();
		}
	}
	return 0;
}

