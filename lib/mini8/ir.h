////////////////////////////////////////////////////////////////////////////////
// file       : ir.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : IR receiver with demodulator and low-active data line
//            : (like the Vishay TSOP... series).
//            : ATmega8 (pin PB0/ICP) Timer1 or
//            : ATtiny45 (pin PBx/PCINTx) Timer0.
// description: Receive RC5 signals from IR remote controls.
//            : Fully interrupt driven with low CPU consumption.
// hints      : Timer is running thru and can be used
//            : for timekeeping or delay routines.
////////////////////////////////////////////////////////////////////////////////

#ifndef _IR_INCLUDED_
#define _IR_INCLUDED_

#include <inttypes.h>
#include <avr/interrupt.h>

////////////////////////////////////////////////////////////////////////////////
// Generic

void ir_init(void); // Initialize the module

////////////////////////////////////////////////////////////////////////////////
// RC5 specific

#define RC5_FREQUENCY 36000 // Frequency of the IR bursts
#define RC5_ERROR 20
	// Maximum allowed timing error (plus/minus) in percent
	// allowed before rejecting RC5 signals.
	// Be generous with this because most remotes use cheap R/C oscillators.
	// Value should not exceed 25.

extern volatile uint16_t rc5_data;
	// The received data.
	// Use the macros below to extract the data fields.
	// bit 15-14: unused, always zero
	// bit 13   : 1st start-bit (always 1)
	// bit 12   : 2nd start-bit (always 1, RC5) or inverted command bit 6 (RC5x)
	// bit 11   : toggle-bit, toggles on new keystroke, keeps value on repeat
	// bit 10-06: address bits (10:MSB)
	// bit 05-00: command bits (05:MSB)

#define RC5_VALID(_data_) ((_data_)&(1<<13|1<<12))
	// Indicates if _data_ is valid (RC5)

//#define RC5X_VALID(_data_) ((_data_)&1<<13) // Now buffered
#define RC5X_VALID(_data_) (_data_)
	// Indicates if _data_ is valid (RC5x)

#define RC5_CLEAR(_data_) do{_data_=0;}while(0)
	// Clear rc5_data

#define RC5_TOGGLE(_data_) ((((_data_)&1<<11)?1:0))
	// Extract toggle bit from _data_

#define RC5_ADDRESS(_data_) ((uint8_t)((_data_)>>6)&0x1F)
	// Extract address from _data_

#define RC5_COMMAND(_data_) ((uint8_t)(_data_)&0x3F)
	// Extract command from _data_ (RC5)

#define RC5X_COMMAND(_data_) \
	(RC5_COMMAND(_data_)|(((_data_)&1<<12)?0:1<<6))
	// Extract command from _data_ (RC5x)

////////////////////////////////////////////////////////////////////////////////
// SIRC specific

#define SIRC_ERROR 20
	// Maximum allowed timing error (plus/minus) in percent
	// allowed before rejecting SIRC signals.
	// Be generous with this because most remotes use cheap R/C oscillators.
	// Value should not exceed 25.

#define SIRC_BITS 15

typedef uint16_t sirc_t;
	// Must be big enough to hold SIRC_BITS bits.

extern volatile sirc_t sirc_data;

//#define SIRC_VALID(_data_) ((_data_)&1<<(SIRC_BITS)) // Now buffered
#define SIRC_VALID(_data_) (_data_)

#define SIRC_ADDRESS(_data_) ((_data_)&((1<<((SIRC_BITS)-7))-1))

#define SIRC_COMMAND(_data_) (((_data_)>>(SIRC_BITS-7))&(0x7F))

////////////////////////////////////////////////////////////////////////////////
// NEC specific

#define NEC_ERROR 20
	// Maximum allowed timing error (plus/minus) in percent
	// allowed before rejecting NEC signals.
	// Be generous with this because most remotes use cheap R/C oscillators.
	// Value should not exceed 25.

typedef union {
	struct {uint8_t b0, b1, b2, b3;} bytes;
	struct {uint16_t w1, w0;} words;
	uint32_t dword;
} nec_t;

extern volatile nec_t nec_data;

#define NEC_VALID(_data_) \
	(((uint8_t)(~(_data_.bytes.b0)) == (_data_.bytes.b1)) \
	&& ((uint8_t)(~(_data_.bytes.b2)) == (_data_.bytes.b3)))

#define NEC_ADDRESS(_data_) (_data_.bytes.b0)

#define NEC_COMMAND(_data_) (_data_.bytes.b2)

////////////////////////////////////////////////////////////////////////////////
// Log specific

#define LOG_SIZE 64

typedef uint16_t log_t;

extern volatile uint8_t log_overf;

uint8_t log_empty(void);

log_t log_pop(void);

#endif // _IR_INCLUDED_
