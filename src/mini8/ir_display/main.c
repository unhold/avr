// Receive IR signals and send the timestamps to the serial line (115200/8N1).
// For use with 'irscope': visualize an IR protocol.
// Use the "Hardware and Mode" setting "MiniPOV3 Time" or "IR Widget Time".
// Format: 1 bit edge flag, 15 bit time since last event in 16us resolution.
// first byte transmitted:
//   bit 7: edge flag (0: falling edge, 1: rising edge)
//   bits 6-0: time diff bits 14-8
// second byte transmitted:
//   bits 7-0: time diff bits 7-0

#define CLEARTEXT 0 // for debugging

#include "mini8/error.h"
#include "mini8/ir.h"
#include "mini8/scrolltext.h"
#include "mini8/uart.h"
#include "mini8/key.h"

#include <stdio.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

#if CLEARTEXT
#	include <stdio.h>
#endif // CLEARTEXT

static inline void send(log_t time) {
	#if CLEARTEXT
		char s[16];
	#endif // CLEARTEXT
	static log_t old;
	static uint8_t edge;
	uint16_t diff;
	time &= 0x7FFF;
	diff = time - old;
	if (edge) diff |= 0x8000;
	#if CLEARTEXT
		sprintf(s, "%02x %02x\n", diff&0xFF, diff>>8);
		uart_puts(s);
	#else // CLEARTEXT
		uart_putc(diff);
		uart_putc(diff>>8);
	#endif // CLEARTEXT
	old = time;
	edge = !edge;
}

int main() {
	char s[32];
	scrolltext_init();
	ir_init();
	uart_init(BAUDRATE_U2X(115200));
	stdout = stdin = stderr = &uart_stream;
	key2_init();
	sei();
	scrolltext_puts_p(PSTR("IR DISPLAY"));
	while (1) {
		/* Disabled because it prevents detection of keys 
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_enable();
		sleep_mode();
		sleep_disable();*/
		if (scrolltext_empty()) {		
			if (RC5X_VALID(rc5_data)) {
				uint16_t d = rc5_data;
				sprintf(s, "RC5:%d:%d:%d ", RC5_ADDRESS(d), RC5X_COMMAND(d),
					RC5_TOGGLE(d) ? 1 : 0);
				scrolltext_puts(s);
			} else if (SIRC_VALID(sirc_data)) {
				sirc_t d = sirc_data;
				sprintf(s, "SIRC:%d:%d ", SIRC_ADDRESS(d), SIRC_COMMAND(d));
				scrolltext_puts(s);
			} else if (NEC_VALID(nec_data)) {
				nec_t d = nec_data;
				sprintf(s, "NEC:%d:%d ", NEC_ADDRESS(d), NEC_COMMAND(d));
				/*sprintf(s, "NEC:%d %d:%d %d:%d",
					nec_data.bytes.b0, nec_data.bytes.b1,
					nec_data.bytes.b2, nec_data.bytes.b3,
					NEC_VALID(nec_data) ? 1 : 0);*/
				scrolltext_puts(s);
			}
		} else {
			rc5_data = 0;
			sirc_data = 0;
			nec_data.dword = 0;
		}
		if (!log_empty()) {
			log_t data = log_pop();
			send(data);
		}
		if (key2_pressed()) {
			reset();
		}
	}
	return 0;
}
