#include <stdio.h>

#include "mini8/servo.h"
#include "mini8/key.h"
#include "mini8/scrolltext.h"
#include "mini8/error.h"

static int8_t position;

void output(void) {
	//scrolltext_putc('D');
	servo2_pos(position);
	if (scrolltext_empty()) {
		char s[8];
		sprintf(s, "%d  ", position);
		scrolltext_puts(s);
	}
}

int main(void) {	
	servo_init();
	key_init();
	scrolltext_init();
	sei();
	scrolltext_puts_p(PSTR("SERVO TEST"));
	while (1) {
		while (key1_pressed()) {
			//scrolltext_puts_p(PSTR("KEY1"));
			position -= (position == -128) ? 0 : 1;
			servo_wait(key2_pressed() ? 2 : 10);
			output();
		}
		while (key2_pressed()) {
			//scrolltext_puts_p(PSTR("KEY2"));
			position += (position == 127) ? 0 : 1;
			servo_wait(key1_pressed() ? 2 : 10);
			output();
		}
	}
	return 0;
}

