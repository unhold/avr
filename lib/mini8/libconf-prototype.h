// mini8 library configuration template
#ifndef _LIBCONF_INCLUDED_
#define _LIBCONF_INCLUDED_

// ERROR
//#	define ERROR_TIMEOUT WDTO_2S
		// Set the time the error state is displayed before MCU reset
		// For other possibilitys see <avr/wdt.h> or avr-libc documentation
		// Undefine to permanently display the error

// RC5RX
#	define RC5RX_FREQUENCY 36000 // Frequency of the IR bursts
#	define RC5RX_ERROR 25
		// Maximum allowed timing error (plus/minus) in percent
		// before rejecting the signals
		// Be generous with this because most remotes use cheap R/C oscillators
		// Value should not exceed 25
#	define RC5RX_SERVO 1 // En/disable the servo function
#	if (!RC5RX_SERVO)
#		define RC5RX_PRESCALER 256 // Prescaler value
			// Must correspond to the config set in rc5_init()
#		define RC5RX_TIMER1MAX 0 // Maximum value of Timer1 before overflow
			// Set to 0 for default value (full 16 bit, 0xFFFF)
#	endif // RC5RX_SERVO
#	define RX5RX_CALLBACK 0 // En/disable callback function
		// when an RC5 signal is received

// SCROLLTEXT
// Parameters, can be freely adjusted, but keep in mind that
// over/underflows could happen in the expressions where these are used
#	define SCROLL_REFRESH 120 // Refresh rate (in Hz)
#	define SCROLL_TIME 500 // Scroll time (in ms)
#	define SCROLL_BUF_SIZE 16 // Size of the character ring buffer (in byte)
#	define SCROLL_SLEEP 1 // Enable(1)/disable(0) sleep for power saving
#	define SCROLL_REVERSE 1 // Dotmatrix upside-down

// SERVO
	// Set the used prescaler for Timer1
#	define SERVO_PRESCALER 64
	// Set the pulse width values for servo operation
#	define SERVO_LEFT  SERVO_COUNT(0.60E-3)
#	define SERVO_ZERO  SERVO_COUNT(1.50E-3)
#	define SERVO_RIGHT SERVO_COUNT(2.40E-3)

// SMALLFONT
/* not working at the moment
#	define SMALLFONT_ALPHA 1 // Enable alphabetic characters
#	define SMALLFONT_DIGIT 1 // Enable digits
#	define SMALLFONT_PUNCT1 1 // Enable common punctuation characters
*/
#	define SMALLFONT_PUNCT2 0 // Enable seldom punctuation characters

#endif // _LIBCONF_INCLUDED_
