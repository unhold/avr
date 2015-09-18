////////////////////////////////////////////////////////////////////////////////
// file       : key.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// hardware   : ATmega8, push-buttons on PC3 and PC4 (mini8),
//              optionally switches or jumpers on PC1 and PC2
//              (named LS1 and LS2 on the mini8).
// description: Read the status of the buttons/switches/jumpers,
//              idle-wait for pressed buttons/switches/jumpers.
////////////////////////////////////////////////////////////////////////////////

#ifndef _KEY_H_INCLUDED_
#define _KEY_H_INCLUDED_

#include <avr/io.h>

// Initialize Key1 and Key2.
// Beware: PC4 is also connected to Pin 3 of the ISP plug (GND from programmer).
// Because of this, Key2 will read active when the programmer is plugged in.
#define key_init() \
	DDRC &= ~(1<<PC3|1<<PC4); \
	(PORTC |= (1<<PC3|1<<PC4))

// Initialize Key1 only.
#define key1_init() \
	DDRC &= ~(1<<PC3); \
	(PORTC |= 1<<PC3)

// Initialize Key2 only.
// Beware: PC4 is also connected to Pin 3 of the ISP plug (GND from programmer).
// Because of this, Key2 will read active when the programmer is plugged in.
#define key2_init() \
	DDRC &= ~(1<<PC4); \
	(PORTC |= 1<<PC4)

// Initialize LS1 as a low active input (switch or jumper to ground).
#define ls1_init() \
	DDRC &= ~(1<<PC1); \
	(PORTC |= 1<<PC1)

// Initialize LS2 as a low active input (switch or jumper to ground).
#define ls2_init() \
	DDRC &= ~(1<<PC2); \
	(PORTC |= 1<<PC2)

// Returns (logically) if Key1 is pressed.
#define key1_pressed() \
	(!(PINC & 1<<PC3))

// Returns (logically) if Key2 is pressed.
#define key2_pressed() \
	(!(PINC & 1<<PC4))

// Returns (logically) if LS1 is active.
#define ls1_active() \
	(!(PINC & 1<<PC1))

// Returns (logically) if LS2 is active.
#define ls2_active() \
	(!(PINC & 1<<PC2))

// Wait until Key1 is pressed.
#define key1_wait() \
	while (PINC & 1<<PC3)

// Wait until Key2 is pressed.
#define key2_wait() \
	while (PINC & 1<<PC4)

// Wait until Key1 is released.
#define key1_wait_release() \
	while (!(PINC & 1<<PC3))

// Wait until Key2 is released.
#define key2_wait_release() \
	while (!(PINC & 1<<PC4))
	
// Wait until LS1 is active.
#define ls1_wait() \
	while (PINC & 1<<PC1)

// Wait until LS2 is active.
#define ls2_wait() \
	while (PINC & 1<<PC2)

#endif // _KEY_H_INCLUDED_
