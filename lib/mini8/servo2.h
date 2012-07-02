////////////////////////////////////////////////////////////////////////////////
// file       : servo2.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, RC-servo on PB2/OC1B, RC-servo PWM input on PB0/ICP
// description: position one RC-servo, read an RC-servo PWM signal
//            : PWM generation and input capturing using Timer1
////////////////////////////////////////////////////////////////////////////////

#ifndef SERVO_H
#define SERVO_H

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#if F_CPU != 12000000
	#warning Servo values are set for 12MHz but actual clock differs
#endif

extern volatile uint16_t servo_dest;
extern volatile uint16_t servo_read;
extern volatile uint8_t servo_flags;

#define SERVO_REV_FLAG 0
#define SERVO_VAL_INVALID_FLAG 1
#define SERVO_DEFAULT_FLAGS 0

// Set maximum left, maximum right and zero value in clock ticks/8
#define SERVO_ZERO  2250 // default 2250 --> 1.5ms
#define SERVO_LEFT  1800 // min     1500 --> 1.0ms
#define SERVO_RIGHT 2700 // max     3000 --> 2.0ms
#define ICP_ZERO  	2100 // default 2250 --> 1.5ms
#define ICP_LEFT  	1200 // min     1500 --> 1.0ms
#define ICP_RIGHT 	3000 // max     3000 --> 2.0ms

// Set the timer compare step value for move function
#define SERVO_STEP 15

// Initialize timer1 for servo operation
inline void servo_init(void);

// Activate servo waveform generation by starting timer1
#define servo_on() \
	(TCCR1B = (TCCR1B & ~(1<<CS12|1<<CS10)) | (1<<CS11))

// Deactivate servo waveform generation by stopping timer1
#define servo_off() \
	(TCCR1B &= ~(1<<CS12|1<<CS11|1<<CS10))

// Calculate the OCR1A value of the servo
uint16_t servo_val(int8_t pos);

// Get the value from the input reading (servo_read)
int8_t servo_lav(uint32_t val);

// Move a servo to a given position, wave is changed immediately
#define servo_pos(_pos_) \
	OCR1B = servo_val(_pos_)

// Move a servo to a given position, with a speed of 1 timer value step per 20ms
// (Speed is set by SERVO_STEP)
#define servo_move(_pos_) \
	servo_dest = servo_val(_pos_); \
	(TIMSK |= 1<<OCIE1B)

// Wait until the last move command is finished
#define servo_wait() \
	while (TIMSK & 1<<OCIE1B) {}

// Set non-reverted movement
#define servo_normal() \
	(servo_flags &= ~(1<<SERVO_REV_FLAG))

// Set reverted movement
#define servo_revert() \
	(servo_flags |= 1<<SERVO_REV_FLAG)

// Read the value from the input
#define servo_in() \
	(servo_lav(servo_read))

// Get the state of the switch on the R/C remote control
#define SERVO_SWITCH_UP 3
#define SERVO_SWITCH_MIDDLE 2
#define SERVO_SWITCH_DOWN 1
#define SERVO_SWITCH_ERROR 0
uint8_t servo_switch(void); 

#endif // SERVO_H
