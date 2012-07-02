////////////////////////////////////////////////////////////////////////////////
// file       : adc.h
// project    : mini8
// author     : Christian Unhold (christian.unhold@gmx.at)
// compiler   : WinAVR GCC
// hardware   : ATmega8
// description: read analog values from AD0 to AD5
////////////////////////////////////////////////////////////////////////////////

#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

// Set ADC resolution
// 1:  8 bit
// 0: 10 bit
#ifndef ADC_LOW_RES
#	define ADC_LOW_RES 1
#endif // ADC_LOW_RES 

// Set ADC clock prescaler
#define ADC_PRESC (1<<ADPS2|1<<ADPS1|1<<ADPS0) // F_CPU/128

// Defines for ADC reference selection
#define ADC_REF_AREF  (0<<REFS1|0<<REFS0) // AREF, internal Vref off
#define ADC_REF_AVCC  (0<<REFS1|1<<REFS0) // AVcc, external capacitor at AREF
#define ADC_REF_256MV (1<<REFS1|1<<REFS0) // Internal 2.56V, ext. cap. at AREF

// Defines for the ADC input channel selection
#define ADC_CHAN_123MV 14 // Internal 1.23V fixed bandgap refernce
#define ADC_CHAN_GND   15 // Internal 0V (ground)

// Set the ADC voltage reference and input channel
// Use the reference selection defines above as argument '_ref'
// Use the number of the channel (0 to 5)
// or one of the channel defines above as argument '_channel_'
#define adc_set(_ref_, _channel_) \
	(ADMUX = _ref_ | ADC_LOW_RES<<5 | (_channel_&15))

// Enable the ADC
#define adc_on() \
	(ADCSRA |= 1<<ADEN|ADC_PRESC)

// Disable the ADC
#define adc_off() \
	(ADCSRA &= ~(1<<ADEN|1<<ADFR))

// Start a single conversion
// ADC has to be enabled before
#define adc_start() \
	(ADCSRA |= 1<<ADSC)

// Start free running mode
// ADC has to be enabled before
#define adc_free() \
	(ADCSRA |= 1<<ADFR|1<<ADSC)

// Wait until single conversion finished
#define adc_wait() \
	while (ADCSRA & 1<<ADSC) {}

// Get the ADC result
#if ADC_LOW_RES == 1
	#define adc_result() ADCH
#else
	#define adc_result() ADC
#endif

// Enables the ADC, starts a single conversion,
// waits for the result, writes the result into '_val_'
// then disables the ADC
#define adc_read(_val_) \
	ADCSRA = 1<<ADEN|1<<ADSC|ADC_PRESC; \
	adc_wait(); \
	_val_ = adc_result(); \
	adc_off()

#endif // ADC_H
