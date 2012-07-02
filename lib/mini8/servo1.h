////////////////////////////////////////////////////////////////////////////////
// file       : servo1.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8, RC-servos on PB1/OC1A and PB2/OC1B
// description: position two RC-servos
//            : PWM generation using Timer1
////////////////////////////////////////////////////////////////////////////////

#ifndef SERVO_H
#define SERVO_H

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#if F_CPU != 12000000
	#warning "Servo values are set for 12MHz but actual clock differs"
#endif

extern volatile uint16_t servo1_dest;
extern volatile uint16_t servo2_dest;
extern volatile uint8_t servo_flags;

#define SERVO_REV1_FLAG 0
#define SERVO_REV2_FLAG 1
#define SERVO_DEFAULT_FLAGS (0<<SERVO_REV1_FLAG|0<<SERVO_REV2_FLAG)

// Set offset to adjust zero position
#define SERVO1_OFFSET -30
#define SERVO2_OFFSET 0

// Set maximum left, maximum right and zero value in clock ticks/8
#define SERVO1_ZERO  2250+(SERVO1_OFFSET) // default 2250 --> 1.5ms
#define SERVO1_LEFT  1600+(SERVO1_OFFSET) // min     1500 --> 1.0ms
#define SERVO1_RIGHT 2850+(SERVO1_OFFSET) // max     3000 --> 2.0ms
#define SERVO2_ZERO  2250+(SERVO2_OFFSET) // default 2250 --> 1.5ms
#define SERVO2_LEFT  1500+(SERVO2_OFFSET) // min     1500 --> 1.0ms
#define SERVO2_RIGHT 3000+(SERVO2_OFFSET) // max     3000 --> 2.0ms

// Set the timer compare step value for move functions
#define SERVO1_STEP 15
#define SERVO2_STEP 15

// Initialize timer1 for servo operation
void servo_init(void);

// Wait for time * 20 ms
void servo_wait(uint16_t time);

// Activate servo waveform generation by starting timer1
#define servo_on() \
	(TCCR1B = (TCCR1B & ~(1<<CS12|1<<CS10)) | (1<<CS11))

// Deactivate servo waveform generation by stopping timer1
#define servo_off() \
	(TCCR1B &= ~(1<<CS12|1<<CS11|1<<CS10))

// Calculate the OCR1X value of a servo
uint16_t servo1_val(int8_t pos);
uint16_t servo2_val(int8_t pos);

// Move a servo to a given position, wave is changed immediately
#define servo1_pos(_pos_) \
	OCR1A = servo1_val(_pos_)
#define servo2_pos(_pos_) \
	OCR1B = servo2_val(_pos_)

// Move a servo to a given position, with a speed of 1 timer value step per 20ms
#define servo1_move(_pos_) \
	servo1_dest = servo1_val(_pos_); \
	(TIMSK |= 1<<OCIE1A); \
	servo1_wait();
#define servo2_move(_pos_) \
	servo2_dest = servo2_val(_pos_); \
	(TIMSK |= 1<<OCIE1B); \
	servo2_wait();

// Wait until the last move command is finished
void servo1_wait(void);

void servo2_wait(void);

// Set non-reverted movement
#define servo1_normal() \
	(servo_flags &= ~(1<<REV1_FLAG))
#define servo2_normal() \
	(servo_flags &= ~(1<<REV2_FLAG))

// Set reverted movement
#define servo1_revert() \
	(servo_flags |= 1<<SERVO_REV1_FLAG)
#define servo2_revert() \
	(servo_flags |= 1<<SERVO_REV2_FLAG)

#endif // SERVO_H
