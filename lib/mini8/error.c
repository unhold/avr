////////////////////////////////////////////////////////////////////////////////
// file       : error.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, 5*7 dotmatrix with row decoder (mini8)
// description: Signal error states, perform hardware reset
//            : Uses the Watchdog Timer
////////////////////////////////////////////////////////////////////////////////

#include "error.h"

ISR(BADISR_vect, ISR_NAKED) {
	// An interrupt witout a coresponding ISR occured
	error(ERROR_BADISR);
}

__attribute__((noreturn)) void error(uint8_t number) {
	// Disable interrupts
	cli();
	// Set the dotmatrix pins to output
	DDRB = 1<<3|1<<4|1<<5;
	DDRC = 1<<5;
	DDRD = 1<<4|1<<5|1<<6|1<<7;
	// Set the dotmatrix adress pins to 0
	PORTB = 0;
	// Set the dotmatrix data pins according to number
	PORTC = (uint8_t)((uint8_t)number<<1)&(uint8_t)(1<<5);
	PORTD = (uint8_t)((uint8_t)number<<4)&(uint8_t)(1<<4|1<<5|1<<6|1<<7);
	#ifdef ERROR_TIMEOUT
		// Enable the WDT with the defined timeout
		wdt_enable(ERROR_TIMEOUT);
	#else // ERROR_TIMEOUT
		// Disable the WDT
		// This is not working if the WDT is always enabled by the WDTON fuse,
		// so set the timeout to the highest possible value (2 sec) first
		wdt_enable(WDTO_2S);
		wdt_disable();
	#endif // ERROR_TIMEOUT
	// Go into power down mode, waiting for WDT timeout if set
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_mode();
	while(1);
}
