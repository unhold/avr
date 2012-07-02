#include "mini8/dotmatrix.h"
#include <avr/sleep.h>

int main() {
	dotmatrix_init();
	dotmatrix_set_callback(dotmatrix_walker);
	sei();
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();
	while(1) {
		sleep_cpu();
	}
}

