////////////////////////////////////////////////////////////////////////////////
// file       : temp.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8 with KTY81-120 linearized by R 2k7 Ohm on ADC0
// description: read temperature from a KTY81-120 silicon temperature sensor
//            : connected to VCC over a 2k7 Ohm resistor
//            : proximity is in the range of 1-2°C
//            : see KTY81 datasheet for details
////////////////////////////////////////////////////////////////////////////////

#ifndef _TEMP_INCLUDED_
#define _TEMP_INCLUDED_

#define ADC_LOW_RES 0
#include "adc.h"
#include <inttypes.h>

// T[°C] = ADC_Result / ADC_Max * TEMP_LIN - TEMP_OFF
#define TEMP_LIN (646.61952)
#define TEMP_OFF (149.88675+2.06250)

#if (ADC_LOW_RES == 0)
#	define TEMP_ADC_RES 10
#elif (ADC_LOW_RES == 1)
#	define TEMP_ADC_RES 8
#else // ADC_LOW_RES
#	error "ADC_LOW_RES undefined"
#endif // ADC_LOW_RES

#if defined (__AVR_ATmega8__)
#	define ADC0_PORT PORTC
#	define ADC0_DDR DDRC
#	define ADC0_BIT 0
#else
#	error "Location of ADC0 unknown"
#endif

static inline void temp_init(void) {
	ADC0_PORT &= ~(1<<ADC0_BIT);
	ADC0_DDR |= 1<<ADC0_BIT; // Set ADC0 to output, driving ground
		// This avoids a constant current thru the KTY81 that could heat it up
}

int8_t temp_read(void);

#endif // _TEMP_INCLUDED_
