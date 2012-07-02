/* Name: main.c
 * Project: HID-Test
 * Author: Christian Starkjohann
 * Creation Date: 2006-02-02
 * Tabsize: 4
 * Copyright: (c) 2006 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt) or proprietary (CommercialLicense.txt)
 * This Revision: $Id: main.c 299 2007-03-29 17:07:19Z cs $
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "usbdrv/oddebug.h"

#include "mini8/key.h"
#include "mini8/rc5rx.h"
#include "mini8/rc5cmd.h"

#include "descriptor.h"

/* ----------------------- hardware I/O abstraction ------------------------ */
// Put an adress on the address lines
// Adress range is from 0 to 6
static inline void dotPutAddress(uint8_t adress) {
	PORTB = (PORTB & ~(1<<3|1<<4|1<<5)) | (((adress) << 3) & (1<<3|1<<4|1<<5));
}

// Put a data on the data lines
// The lower 5 bits are the used data
static inline void dotPutData(uint8_t data) {
	PORTD = (PORTD & ~(1<<4|1<<5|1<<6|1<<7)) |
	((data << 4) & (1<<4|1<<5|1<<6|1<<7)),
	PORTC = (PORTC & ~(1<<5)) | ((data<<1) & (1<<5));
}

void dotInit() {
	// Set the adress pins to output, 0
	PORTB &= ~(1<<3|1<<4|1<<5);
	DDRB |= 1<<3|1<<4|1<<5;
	// Set the data pins to output, 0
	PORTD &= ~(1<<4|1<<5|1<<6|1<<7);
	DDRD |= 1<<4|1<<5|1<<6|1<<7;
	PORTC &= ~(1<<5);
	DDRC |= 1<<5;
}

#define ledClear() ledSet(0)

static void hardwareInit(void) {
	/* force (re-)enumeration of USB device */
	usbDeviceDisconnect();
	_delay_ms(20.0);
	usbDeviceConnect();
	TCCR0 = 5; /* timer 0 prescaler: 1024 */
	key_init();
	rc5rx_init();
	dotInit();
	dotPutAddress(3);
}
/* ------------------------------------------------------------------------- */
/* Mapping of the RC5 commands to key values
*/
PROGMEM char keyMapping[] = {
	0,
	RC5_POWER,
	RC5_GO,
	RC5_MENU,
	RC5_OK,
	RC5_UP,
	RC5_DOWN,
	RC5_LEFT,
	RC5_RIGHT,
	RC5_BACK_EXIT,
	RC5_CH_UP,
	RC5_CH_DOWN,
	RC5_PLAY,
	RC5_PAUSE,
	RC5_RECORD,
	RC5_FORWARD,
	RC5_REWIND,
	RC5_NEXT,
	RC5_PREVIOUS,
	RC5_STOP,
	RC5_PREV_CH,
	RC5_MUTE,
	RC5_VOL_UP,
	RC5_VOL_DOWN
};

/* ------------------------------------------------------------------------- */
/* The following function returns an index for the first key pressed. It
 * returns 0 if no key is pressed.
 */
static uchar keyPressed(void) {
	static uint8_t toggle;
	if (key1_pressed()) return 23; /* Volume Down */
	if (key2_pressed()) return 22; /* Volume Up */
	if (RC5RX_VALID(rc5rx_data) && (RC5RX_ADDRESS(rc5rx_data)) == 30 &&
		(RC5RX_TOGGLE(rc5rx_data) != toggle)) {
		uchar idx, cmd;
		toggle = RC5RX_TOGGLE(rc5rx_data);
		cmd = RC5RX_COMMAND(rc5rx_data);
		RC5RX_CLEAR(rc5rx_data);
		for (idx = 1; idx < sizeof(keyMapping); ++idx) {
			if (cmd == pgm_read_byte(keyMapping+idx)) {
				return idx;
			}
		}
	}
	return 0;
}

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

static uchar    reportBuffer;       /* buffer for HID reports */
static uchar    idleRate;           /* in 4 ms units */

/*
static void buildReport(uchar key)
{
	reportBuffer = key;
}
*/

uchar	usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    usbMsgPtr = &reportBuffer;
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */
            reportBuffer = keyPressed(); /*buildReport(keyPressed());*/
            return sizeof(reportBuffer);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idleRate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idleRate = rq->wValue.bytes[1];
        }
    }else{
        /* no vendor specific requests implemented */
    }
	return 0;
}

/* ------------------------------------------------------------------------- */

int	main(void)
{
uchar   key, lastKey = 0, keyDidChange = 0;
uchar   idleCounter = 0;

	wdt_enable(WDTO_2S);
    hardwareInit();
	odDebugInit();
	usbInit();
	sei();
    DBG1(0x00, 0, 0);
	for(;;){	/* main event loop */
		wdt_reset();
		usbPoll();
        key = keyPressed();
        if(lastKey != key){
            lastKey = key;
            keyDidChange = 1;
            dotPutData(0b00100);
        }
        if(TIFR & (1<<TOV0)){   /* 22 ms timer */
            TIFR = 1<<TOV0;
            if(idleRate != 0){
                if(idleCounter > 4){
                    idleCounter -= 5;   /* 22 ms in units of 4 ms */
                }else{
                    idleCounter = idleRate;
                    keyDidChange = 1;
                }
            }
        }
        if(keyDidChange && usbInterruptIsReady()){
            keyDidChange = 0;
            /* use last key and not current key status in order to avoid lost
               changes in key status. */
            reportBuffer = lastKey; /*buildReport(lastKey);*/
            usbSetInterrupt(&reportBuffer, sizeof(reportBuffer));
            dotPutData(0);
        }
	}
	return 0;
}

/* ------------------------------------------------------------------------- */
