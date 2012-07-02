////////////////////////////////////////////////////////////////////////////////
// file       : ir.c
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

#include "ir.h"

////////////////////////////////////////////////////////////////////////////////
#if defined (__AVR_ATmega8__)
// Device ATmega8: 16 bit Timer 1 with Input Capture Pin

#define IR_PRESCALER 256 // Prescaler value

typedef uint16_t count_t;

static inline void ir_handler(const count_t newcount);

static inline void timer_init(void) {
	TCCR1A = 0; // OC1A/OC1B disconnected
	TCCR1B = 1<<ICNC1 | // Input noise canceler activated
		0<<ICES1 | // Detect falling edge
#if (IR_PRESCALER == 64)
		0<<CS12 | 1<<CS11 | 1<<CS10; // Prescaler 64
#elif (IR_PRESCALER == 256)
		1<<CS12 | 0<<CS11 | 0<<CS10; // Prescaler 256
#else // IR_PRESCALER
#	error "Invalid prescaler setting"
#endif // IR_PRESCALER
	TIMSK = (TIMSK & ~(1<<TICIE1|1<<OCIE1A|1<<OCIE1B|1<<TOIE1)) |
		1<<TICIE1; // Only input capure interrupt acitvated
}

ISR(TIMER1_CAPT_vect) {
	TIMSK &= ~(1<<TICIE1);
	sei();
	{
		register count_t count;
		TCCR1B ^= 1<<ICES1; // Revert edge sensitivity
		count = ICR1;
		ir_handler(count);
	}
	cli();
	TIMSK |= 1<<TICIE1;
}

#define IS_PULSE_START() (TCCR1B&(1<<ICES1))
#define IS_PULSE_STOP() (!IS_PULSE_START())
	// Get the edge polarity for which the handler was called

////////////////////////////////////////////////////////////////////////////////
#elif defined (__AVR_ATtiny45__)
// Device ATtiny45: 8 bit Timer 0 and Pin Change Interrupt

#define IR_PRESCALER 1024 // Prescaler value

typedef uint8_t count_t;

static inline void ir_handler(const count_t newcount);

static inline void timer_init(void) {
	// Init Timer 0
	//TCCR0A = 0; // Normal operation
	TCCR0B = 1<<CS02 | 0<<CS01 | 1<<CS00; // Prescaler 1024
	//OCR0A = 0;
	//OCR0B = 0;
	// Init Pin Change Interrupt
	GIMSK |= 1<<PCIE; // Enable Pin Change Interrupt
	PCMSK = 1<<PCINT4;
}

static uint8_t rising;

ISR(PCINT0_vect) {
	GIMSK &= ~(1<<PCIE);
	sei();
	{
		register count_t count;
		count = TCNT0;
		rising = PINB & 1<<PB4;
		ir_handler(count);
	}
	cli();
	GIMSK |= 1<<PCIE;
}

#define IS_PULSE_START() (!rising)
#define IS_PULSE_STOP() (rising)
	// Get the edge polarity for which the handler was called

#endif
////////////////////////////////////////////////////////////////////////////////
// RC5 specific

#define _RC5_COUNT \
	(64.0*(F_CPU)/((double)(RC5_FREQUENCY)*(IR_PRESCALER)))
#define _RC5_ERROR ( (double)(_RC5_COUNT) * (RC5_ERROR) / 100 )
#define _RC5_MAXCOUNT ( (count_t) ((_RC5_COUNT)+(_RC5_ERROR)+0.5) )
#define _RC5_MINCOUNT ( (count_t) (((_RC5_COUNT)-(_RC5_ERROR)+0.5)/2) )
#define _RC5_COUNT075 ( (count_t) ((_RC5_COUNT)*0.75+0.5) )

volatile uint16_t rc5_data;

static inline void rc5_init(void) {
	rc5_data = 0;
}

static inline void rc5_handler(const count_t newcount) {
	static count_t oldcount;
	static uint16_t rc5_temp;
	count_t	count = newcount - oldcount; // Calculate differrence between counts
	if (count < _RC5_MINCOUNT || count > _RC5_MAXCOUNT) {
		// Pulse too short or too long
		rc5_temp = 0;
	}
	if (!rc5_temp || count > _RC5_COUNT075) { // Start or long pulse
		oldcount = newcount; // Start counting from now on
		rc5_temp <<= 1;
		if (IS_PULSE_START()) {
			rc5_temp |= 1;
		}
		if (rc5_temp&(1<<13)) { // 14 Bits received
			rc5_data = rc5_temp;
			rc5_temp = 0;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// SIRC specific

#define _SIRC_COUNT (600e-6*(F_CPU)/(IR_PRESCALER))
	// 600 us in timer counts, the base time of SIRC (double)
#define _SIRC_MIN(_times_) \
	((count_t)((_SIRC_COUNT*(_times_)*(100-(SIRC_ERROR))/100)+0.5))
#define _SIRC_MAX(_times_) \
	((count_t)((_SIRC_COUNT*(_times_)*(100+(SIRC_ERROR))/100)+0.5))

volatile sirc_t sirc_data;

static inline void sirc_init(void) {
	sirc_data = 0;
}

static inline void sirc_handler(const count_t count) {
	static sirc_t sirc_temp;
	if (IS_PULSE_STOP()) {
		if (sirc_temp == 0) { // Waiting for start
			// Start is 5 marks long: 4 active, 1 idle
			if (count > _SIRC_MIN(4) && count < _SIRC_MAX(4)) {
				sirc_temp = 1;
			}
		} else { // In transmission
			if (count > _SIRC_MIN(1) && count < _SIRC_MAX(1)) {
			// Bit 0 is 2 marks long: 1 active, 1 idle
				sirc_temp <<= 1;
			} else if (count > _SIRC_MIN(2) && count < _SIRC_MAX(2)) {
			// Bit 1 is 3 marks long: 2 active, 1 idle
				sirc_temp <<= 1;
				sirc_temp |= 1;
			} else { // Invalid, abort
				sirc_temp = 0;
			}
			if (sirc_temp & 1<<(SIRC_BITS)) { // Got valid code
				sirc_data = sirc_temp;
				sirc_temp = 0;
			}
		}
	} else { // IS_PULSE_START()
		if (!(count > _SIRC_MIN(1) && count < _SIRC_MAX(1))) {
			// Pause with the wrong duration, abort
			sirc_temp = 0;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// NEC specific

#define _NEC_COUNT (560e-6*(F_CPU)/(IR_PRESCALER))
	// 560 us in timer counts, the base time of NEC (double)
#define _NEC_MIN(_times_) \
	((count_t)((_NEC_COUNT*(_times_)*(100-(NEC_ERROR))/100)+0.5))
#define _NEC_MAX(_times_) \
	((count_t)((_NEC_COUNT*(_times_)*(100+(NEC_ERROR))/100)+0.5))
#define _NEC_WAS(_count_, _times_) \
	(_count_ > _NEC_MIN(_times_) && _count_ < _NEC_MAX(_times_))

#define NEC_DEBUG(c) // Disable

volatile nec_t nec_data;

static inline void nec_init(void) {
	nec_data.dword = 0;
}

static inline void nec_handler(const count_t count) {
	static uint32_t nec_temp;
	static uint8_t nec_state;
	if (IS_PULSE_START()) {
		if (nec_state > 1) {
			++nec_state;
			nec_temp >>= 1;
			if (_NEC_WAS(count, 3)) {
				// Long pause, 1
				NEC_DEBUG('1');
				nec_temp |= (uint32_t)1<<31;
			} else if (_NEC_WAS(count, 1)) {
				NEC_DEBUG('0');
				// Short pause, 0
				//nec_temp |= 0<<31;
			} else {
				NEC_DEBUG('A');
				// Invalid, abort
				nec_state = 0;
			}
		} else if (nec_state == 1) {
			if (_NEC_WAS(count, 8)) {
				// Extra long pause after start
				NEC_DEBUG('P');
				nec_state = 2;
				nec_temp = 0;
			} else if (_NEC_WAS(count, 4)) {
				// Shorter pause indicates repeat
				NEC_DEBUG('R');
				nec_state = 34;
			} else {
				// Invalid, abort
				NEC_DEBUG('C');
				nec_state = 0;
			}
		}
	} else { // IS_PULSE_STOP()
		if (nec_state > 1) {
			// Normal pulse is always 1 mark long
			if (!(_NEC_WAS(count, 1))) {
				// Invalid, abort
				NEC_DEBUG('B');
				nec_state = 0;
			} else if (nec_state == 34) {
				// Protocol finished
				NEC_DEBUG('F');
				nec_state = 0;
				nec_data.dword = nec_temp;
			}
		} else if (nec_state == 0) {
			if (_NEC_WAS(count, 16)) {
				// Long pulse at start
				NEC_DEBUG('S');
				nec_state = 1;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Log specific

// Log FIFO
static volatile log_t log_fifo[LOG_SIZE];
static volatile uint8_t log_headp;
static volatile uint8_t log_tailp;
// Enter at head, exit at tail (think of the snake)

volatile uint8_t log_overf;

static inline uint8_t log_incp(uint8_t p) {
	return (p + 1) % LOG_SIZE;
}

uint8_t log_empty(void) {
	return log_headp == log_tailp;
}

static inline uint8_t log_full(void) {
	return log_incp(log_headp) == log_tailp;
}

void log_push(const log_t data) {
	uint8_t oldp;
	while(log_full());
	// First increment head pointer, then put in data
	// This avoids race conditions without locking
	oldp = log_headp;
	log_headp = log_incp(log_headp);
	log_fifo[oldp] = data;
}

log_t log_pop(void) {
	log_t data;
	while (log_empty());
	// First take out data, then increment tail pointer
	// This avoids race conditions without locking
	data = log_fifo[log_tailp];
	log_tailp = log_incp(log_tailp);
	return data;
}

static inline void log_init(void) {
	log_headp = 0,
	log_tailp = 0,
	log_overf = 0;
}

static inline void log_handler(const count_t newcount) {
	if (log_full()) {
		log_overf = 1;
	} else {
		log_push(newcount);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Generic

void ir_init(void) {
	rc5_init();
	sirc_init();
	nec_init();
	log_init();
	timer_init();
}

static inline void ir_handler(const count_t newcount) {
	static count_t oldcount;
	const count_t count = newcount - oldcount;
	oldcount = newcount;
	rc5_handler(newcount);
	sirc_handler(count);
	nec_handler(count);
	log_handler(newcount);
}

