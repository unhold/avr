////////////////////////////////////////////////////////////////////////////////
// file       : rc5rx.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, IR Receiver with demodulator and low-active data line
//            : (like the Vishay TSOP... series) on pin PB0/ICP,
//            : RC-servo on PB2/OC1B
// description: Receive RC5 signals from IR remote controls,
//            : position an RC-servo
//            : Fully interrupt driven with low CPU consumption
//            : PWM generation and input capture using Timer1
// hints      : Timer1 is running thru (prescaler 256, RC5 only) or
//            : overflows every 20 ms (with servo) and can be used
//            : for timekeeping or delay routines,
//            : example wait routine provided
////////////////////////////////////////////////////////////////////////////////

#ifndef _RC5RX_INCLUDED_
#define _RC5RX_INCLUDED_

#include <inttypes.h>
#include <avr/interrupt.h>

#ifdef LIBCONF
#	include "libconf.h"
#else // LIBCONF
#	define RC5RX_FREQUENCY 36000 // Frequency of the IR bursts 
#	define RC5RX_ERROR 20
		// Maximum allowed timing error (plus/minus) in percent
		// before rejecting the signals
		// Be generous with this because most remotes use cheap R/C oscillators
		// Value should not exceed 25
#	define RC5RX_SERVO 0 // En/disable the servo function
#	if (!RC5RX_SERVO)
#		define RC5RX_PRESCALER 256 // Prescaler value
			// Must correspond to the config set in rc5_init()
#		define RC5RX_TIMER1MAX 0 // Maximum value of Timer1 before overflow
			// Set to 0 for default value (full 16 bit, 0xFFFF)
#	endif // RC5RX_SERVO
#	define RX5RX_CALLBACK 0 // En/disable callback function
		// when an RC5 signal is received
#endif // LIBCONF

#if RC5RX_SERVO
#	include "servo.h" // Included here, dont't include from application
#	define servo_init _servo_init_UNUSED // Disable
#	undef servo1_pos // Disable
#	define servo_pos servo2_pos // Add alias
#	define RC5RX_PRESCALER SERVO_PRESCALER
#	define RC5RX_TIMER1MAX SERVO_CYCLE
#endif // RC5RX_SERVO

void rc5rx_init(void); // Initialize the module (including servo if enabled)

extern volatile uint16_t rc5rx_data;
	// The received data
	// Use the macros below to extract the data fields
	// bit 13   : 1st start-bit (always 1)
	// bit 12   : 2nd start-bit (always 1, RC5) or inverted command bit 6 (RC5x)
	// bit 11   : toggle-bit, toggles on new keystroke, keeps value on repeat
	// bit 10-06: address bits (10:MSB)
	// bit 05-00: command bits (05:MSB)

#if RX5RX_CALLBACK
typedef void(*rc5rx_callback_t)(void); // Type of a callback function
extern volatile rc5rx_callback_t rc5rx_callback;
	// This callback function is called when an RC5 signal is received
	// Set to 0 to disable
	// The function is called from an ISR, so don't block
#endif // RX5RX_CALLBACK

#define RC5RX_VALID(_data_) ((_data_)&(1<<13|1<<12))
	// Indicates if _data_ is valid (RC5)

#define RC5RX_VALIDX(_data_) ((_data_)&1<<13)
	// Indicates if _data_ is valid (RC5x)

#define RC5RX_CLEAR(_data_) do{_data_=0;}while(0)
	// Clear rc5_data

#define RC5RX_TOGGLE(_data_) ((((_data_)&1<<11)?1:0))
	// Extract toggle bit from _data_

#define RC5RX_ADDRESS(_data_) ((uint8_t)((_data_)>>6)&0x1F)
	// Extract address from _data_

#define RC5RX_COMMAND(_data_) ((uint8_t)(_data_)&0x3F)
	// Extract command from _data_ (RC5)

#define RC5RX_COMMANDX(_data_) \
	(RC5RX_COMMAND(_data_)|(((_data_)&1<<12)?0:1<<6))
	// Extract command from _data_ (RC5x)

#endif // _RC5RX_INCLUDED_
