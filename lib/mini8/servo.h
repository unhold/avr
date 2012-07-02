////////////////////////////////////////////////////////////////////////////////
// file       : servo.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, RC-servos on PB1/OC1A and PB2/OC1B
// description: position two RC-servos
//            : PWM generation using Timer1
// hints      : Timer1 overflows every 20 ms and can be used for timekeeping
//            : or delay routines, example wait routine provided
////////////////////////////////////////////////////////////////////////////////

#ifndef _SERVO_INCLUDED_
#define _SERVO_INCLUDED_

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// Calculate the timer count from a given pulse width (in (double)seconds)
#define SERVO_COUNT(_double_time_) \
	((uint16_t)((_double_time_)*(F_CPU)/(SERVO_PRESCALER)))

// Calculate the pulse with (in (double)seconds) from a given timer count
#define SERVO_TIME(_count_) \
	((double)(_count_)*(SERVO_PRESCALER)/(F_CPU))

// Maximum value (plus 1) that Timer1 will reach
#define SERVO_CYCLE SERVO_COUNT(20.0E-3)

#ifdef LIBCONF
#	include "libconf.h"
#else // LIBCONF
	// Set the used prescaler for Timer1
#	define SERVO_PRESCALER 64
	// Set the pulse width values for servo operation
#	define SERVO_LEFT  SERVO_COUNT(0.60E-3)
#	define SERVO_ZERO  SERVO_COUNT(1.50E-3)
#	define SERVO_RIGHT SERVO_COUNT(2.40E-3)
#endif // LIBCONF

// Initialize Timer1 for servo operation
void servo_init(void);

// Calculate the OCR1X value of a servo
uint16_t servo_count(uint8_t position);

// Move a servo to a given position, wave is changed immediately
#define servo1_pos(_pos_) \
	do{OCR1A=servo_count(_pos_)-1;}while(0)
#define servo2_pos(_pos_) \
	do{OCR1B=servo_count(_pos_)-1;}while(0)

// Example for a timekeeping routine using TIMER1_OVF_vect
// Wait for time * 20 ms
void servo_wait(uint16_t time);

#endif // _SERVO_INCLUDED_
