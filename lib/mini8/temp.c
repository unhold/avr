////////////////////////////////////////////////////////////////////////////////
// file       : temp.c
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8 with KTY81-120 linearized by R 2k7 Ohm on ADC0
// description: read temperature from a KTY81-120 silicon temperature sensor
//            : connected to VCC over a 2k7 Ohm resistor
//            : proximity is in the range of 1-2°C
//            : see KTY81 datasheet for details
////////////////////////////////////////////////////////////////////////////////

#include "temp.h"

#include "uart.h"
#include <stdlib.h>

#if 0
float temp_read(void) {
	// Floating point version
	// Used for developing the fixed point version
	float temp;
	ADC0_DDR &= ~(1<<ADC0_BIT); // Set ADC0 to input
	adc_set(ADC_REF_AVCC, 0); // read from ADC0
	adc_read(temp);
	ADC0_DDR |= ~(1<<ADC0_BIT); // Set ADC0 to output (driving ground)
	temp *= (float)TEMP_LIN/(1<<TEMP_ADC_RES);
	temp -= (float)TEMP_OFF;
	return temp;
}
#endif

int8_t temp_read(void) {
	// Fixed point version
	int32_t temp;
	ADC0_DDR &= ~(1<<ADC0_BIT); // Set ADC0 to input
	adc_set(ADC_REF_AVCC, 0);
	adc_read(temp);
	ADC0_DDR |= ~(1<<ADC0_BIT); // Set ADC0 to output, driving ground
		// This avoids a constant current thru the KTY81 that could heat it up
	temp *= (int32_t)((TEMP_LIN)*(1<<4)+0.5);
	temp -= (int32_t)((TEMP_OFF)*(1<<(TEMP_ADC_RES+4))+0.5);
	return temp>>(TEMP_ADC_RES+4);
}
