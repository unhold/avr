
/*
 *  _   _ ____  ____    ___        ____                      _
 * | | | / ___|| __ )  |_ _|_ __  |  _ \ ___ _ __ ___   ___ | |_ ___
 * | | | \___ \|  _ \   | || '__| | |_) / _ \ '_ ` _ \ / _ \| __/ _ \
 * | |_| |___) | |_) |  | || |    |  _ <  __/ | | | | | (_) | ||  __/
 *  \___/|____/|____/  |___|_|    |_| \_\___|_| |_| |_|\___/ \__\___|
 *
 * Copyright 2007 Jorgen Birkler
 * jorgen.birkler)a(gmail.com
 * USB HID device for IR receiver
 * License: GNU GPL v2 (see License.txt) or proprietary (contact author)
 */

/**
 * @mainpage
 *
 * \image html logo_big.png
 * - Main project site: http://usb-ir-remote.sourceforge.net
 * - Releases: http://sourceforge.net/projects/usb-ir-remote/files/
 * - GIT: git://usb-ir-remote.git.sourceforge.net/gitroot/usb-ir-remote/usb-ir-remote (read-only)
 * .
 *
 * \section intro Introduction
 * This project includes a USB device that receives Ir light from a remote control (Sony and NEC protocol)
 * and report the keys pressed on the remote control to the PC using the standardized USB HID device.
 * There is s special class for remote controls and this firmware will register as such a device.
 *
 * Main features:
 * - All source code and hardware design available
 * - Very low cost
 * - All development tools are free, including the hardware and programmer to flash the chip
 * - Can be extended to other uses
 * - Device is programmable to accept different types of remotes
 * .
 *
 * \section dev_environment Development Environment
 * I started to develop on Windows (http://winavr.sourceforge.net). I now moved to Linux.
 * Tools (for Ubuntu Linux):
 * - AVR GCC ("$ apt-get install avr-gcc")
 * - Eagle CAD (http://www.cadsoftusa.com)
 * - AVR Dude ("$ apt-get install avrdude")
 * - Doxygen (http://www.doxygen.org "$apt-get install doxygen")
 * .
 * \section user_guide User Guide
 *
 * Welcome to the USB Ir Remote Device. When plugged into the computer
 * the device will register it self as an USB Consumer HID device.
 * It will act as a keyboard and a consumer device that can send
 * play, stop, pause, etc commands to applications.
 *
 * Windows Media Player have built-in support for these commands and the HID
 * driver is included Windows. No drivers are needed! No girder or WinLIRC
 * or similar is needed.
 *
 * The device will react to NEC/SONY compatible IR codes. The NEC protocols is
 * used by a number of manufacturers including Pioneer, Onkyo and Goldstar.
 *
 * To learn the codes for the different commands do this:
 * - Open a notepad windows (Start->Run->"notepad")
 * - Plug-in the device
 * - Press the program button on the device
 * - "IR Keyboard..." should be printed in Notepad
 * - "Press button" will appear and then the button to press
 * - Press each button requested by the device
 * - "DONE" will appear when all programmable ir buttons have been programmed.
 * .
 *
 * Your computer should now be ready to control Windows Media Player.
 * Start your favorite movie and try the start, stop and pause buttons on the
 * remote.
 *
 * \section implementation_guide Implementation
 * - \ref software
 * - \ref hardware
 * .
 *
 * (c) 2007 Jorgen Birkler (jorgen.birkler)a(gmail.com)
 *
 * USB driver
 *
 * (c) 2006 by OBJECTIVE DEVELOPMENT Software GmbH
 */
/**
 * \page software Software
 * Uses the firmware only USB low speed driver from http://obdev.at.
 * The USB device is configured as a Remote Control HID device.
 *
 * \section tips Tips about HID development
 * General tips about HID development:
 *
 * 1. HID device class is cached by Windows; change USB_CFG_DEVICE_ID if you change USAGE_PAGE
 *    class to another. It took me several weeks to find this info. I copied the use page for the
 *    remote but it never work until I changed the USB_CFG_DEVICE_ID to another number so that the
 *    device was rediscovered by Windows.
 *
 * 2. Added usbconfig.h manually to the dependencies in the make file to all .o files.
 *    WinAVR .d files doesn't seem to work for subdirs
 *
 * Ir is received by ICP interrupt:
 * \include irrx.h
 *
 * Main loop translated the ir codes received and handles the main USB look:
 * \include main.c
 */
/**
 * \page hardware Hardware
 *
 * Schematic:
 * \image html ir_hid.sch.png Schematic
 *
 * Partlist:
 * \verbinclude ir_hid.sch.parts.txt
 *
 * Board (for protoboards):
 * \image html ir_hid.brd.png
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include <string.h>

#include "usbdrv.h"
#include "irrx.h"
#include "oddebug.h"
#include "uart_io.h"
#include "usbstream.h"

#define elements_of(array) (sizeof(array) / sizeof(array[0]))

#define LED_RED_CHANGE() PORTB ^= _BV(PB2)
#define LED_RED_ON() PORTB |= _BV(PB2)
#define LED_RED_OFF() PORTB &= ~_BV(PB2)
#define LED_RED_INIT() DDRB |= _BV(PB2) | _BV(PB1);PORTB &= ~_BV(PB1);LED_RED_OFF()

#define LED_GREEN_CHANGE() PORTC ^= _BV(PC3)
#define LED_GREEN_ON() PORTC |= _BV(PC3)
#define LED_GREEN_OFF() PORTC &= ~_BV(PC3)
#define LED_GREEN_INIT() PORTC |= _BV(PC3);LED_RED_OFF()

#define SWITCH_PGM_IS_PRESSED() bit_is_clear(PINC,4)
#define SWITCH_PGM_INIT() DDRC &=~_BV(PC4);PORTC|=_BV(PC4)

/*
 * help macros
 */
#define STATIC_ASSERT(expr) extern char static_assert[ (!!(expr))*2 - 1]

typedef struct
{
	uint8_t put;
	uint8_t get;
	uint8_t data[128];
} virtual_keyboard_buffer_t;

static int virtual_keyboard_ioputchar(char c, FILE *stream);

static virtual_keyboard_buffer_t virtual_keyboard_buffer =
{ 0, 0,
{ 0 } };

STATIC_ASSERT(sizeof(virtual_keyboard_buffer.data) == 128 || sizeof(virtual_keyboard_buffer.data) == 256 || sizeof(virtual_keyboard_buffer.data) == 64);

static FILE virtual_keyboard_file_io= FDEV_SETUP_STREAM(virtual_keyboard_ioputchar, NULL, _FDEV_SETUP_WRITE);

static int virtual_keyboard_ioputchar(char c, FILE *stream)
{
	uint8_t next = (virtual_keyboard_buffer.put + 1)
			& (sizeof(virtual_keyboard_buffer.data) - 1);
	if (c == '\n')
	{
		c = '\r';
	}
	if (next == virtual_keyboard_buffer.get)
	{
		return 0; //Full
	}
	virtual_keyboard_buffer.data[virtual_keyboard_buffer.put] = c;
	virtual_keyboard_buffer.put = next;
	return 0;
}

static int virtual_keyboard_buffer_not_empty(void)
{
	return (virtual_keyboard_buffer.put != virtual_keyboard_buffer.get);
}

static int virtual_keyboard_iogetchar(void)
{
	if (virtual_keyboard_buffer.put == virtual_keyboard_buffer.get)
	{
		return -1; //Empty
	}
	else
	{
		int c;
		c = virtual_keyboard_buffer.data[virtual_keyboard_buffer.get];
		virtual_keyboard_buffer.get = (virtual_keyboard_buffer.get + 1)
				& (sizeof(virtual_keyboard_buffer.data) - 1);
		return c;
	}
}

#define LEFT_SHIFT_BIT 0x80

#define USB_HID_KEY_a_A 0x04
#define USB_HID_KEY_0 0x27
#define USB_HID_KEY_1 0x1E
#define USB_HID_KEY_RETURN 0x28
#define USB_HID_KEY_SPACEBAR 0x2C
#define USB_HID_KEY_DOT 0x37
#define USB_HID_KEY_COMMA 0x36

//!#%,.


/**
 * Translate a character to something that can be sent as a HID event
 * This function work together with the HID descriptor and assumes that the
 * keyboard layout have a-z on roughly the same as an english keyboard
 */
static uint8_t virtual_keyboard_translate_to_hid(int c)
{
	if (c <= 0)
	{
		return 0;
	}
	else if ('a' <= c && c <= 'z')
	{
		return (c - 'a' + USB_HID_KEY_a_A);
	}
	else if ('A' <= c && c <= 'Z')
	{
		return (c - 'A' + USB_HID_KEY_a_A) | LEFT_SHIFT_BIT;
	}
	else if ('1' <= c && c <= '9')
	{
		return (c - '1' + USB_HID_KEY_1);
	}
	else if ('0' == c)
	{
		return (USB_HID_KEY_0);
	}
	else if ('.' == c)
	{
		return (USB_HID_KEY_DOT);
	}
	else if (',' == c)
	{
		return (USB_HID_KEY_COMMA);
	}
	else if ('\r' == c)
	{
		return (USB_HID_KEY_RETURN);
	}
	else if (' ' == c || '\t' == c)
	{
		return (USB_HID_KEY_SPACEBAR);
	}
	else
	{
		return (USB_HID_KEY_SPACEBAR);
	}
}

static uchar vtLastKey = 0;

/* ----------------------- hardware I/O abstraction ------------------------ */

/* pin assignments:
 PB0	Key 1
 PB1	Key 2
 PB2	Key 3
 PB3	Key 4
 PB4	Key 5
 PB5 Key 6

 PC0	Key 7
 PC1	Key 8
 PC2	Key 9
 PC3	Key 10
 PC4	Key 11
 PC5	Key 12

 PD0	USB-
 PD1	debug tx
 PD2	USB+ (int0)
 PD3	Key 13
 PD4	Key 14
 PD5	Key 15
 PD6	Key 16
 PD7	Key 17
 */

static void hardwareInit(void)
{
	uchar i, j;
	PORTD = 0xfa;
	/* 1111 1010 bin: activate pull-ups except on USB lines */
	DDRD = 0x07;
	/* 0000 0111 bin: all pins input except USB (-> USB reset) */
	j = 0;
	while (--j)
	{ /* USB Reset by device only required on Watchdog Reset */
		i = 0;
		while (--i)
			; /* delay >10ms for USB reset */
	}
	DDRD = 0x02;
	/* 0000 0010 bin: remove USB reset condition */
	/* configure timer 0 for a rate of 12M/(1024 * 256) = 45.78 Hz (~22ms) */
	TCCR0 = 5;
	/* timer 0 prescaler: 1024 */
}

/* ------------------------------------------------------------------------- */

#define NUM_HARDWARE_KEYS    17

#define NUM_KEYS (NUM_HARDWARE_KEYS + NUM_IR_KEYS)

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

static uchar reportBuffer[2]; /* buffer for HID reports */
static uchar idleRate; /* in 4 ms units */

//Names of buttons

const char label00[] PROGMEM = "Press button\n";
const char label01[] PROGMEM = "Scan Next Track";
const char label02[] PROGMEM = "Scan Previous Track";
const char label03[] PROGMEM = "Rewind";
const char label04[] PROGMEM = "Play";
const char label05[] PROGMEM = "Fast Forward";
const char label06[] PROGMEM = "Pause";
const char label07[] PROGMEM = "Stop";
const char label08[] PROGMEM = "Record";
const char label09[] PROGMEM = "Menu";
const char label10[] PROGMEM = "Menu Escape";
const char label11[] PROGMEM = "Menu Up";
const char label12[] PROGMEM = "Menu Down";
const char label13[] PROGMEM = "Menu Left";
const char label14[] PROGMEM = "Menu Right";
const char label15[] PROGMEM = "Menu Pick";
const char label16[] PROGMEM = "Menu Value Decrease";
const char label17[] PROGMEM = "Menu Value Increase";
const char label18[] PROGMEM = "Volume Down";
const char label19[] PROGMEM = "Volume Up";
const char label20[] PROGMEM = "Mute";
const char label21[] PROGMEM = "Channel Decrement";
const char label22[] PROGMEM = "Channel Increment";
const char label23[] PROGMEM = "Closed Caption Select";
const char label24[] PROGMEM = "Closed Caption";
const char label25[] PROGMEM = "DONE\n\n";

PGM_P button_labels[] PROGMEM =
{
	label00,
	label01,
	label02,
	label03,
	label04,
	label05,
	label06,
	label07,
	/*,
	label08,
	label09,
	label10,
	label11,
	label12,
	label13,
	label14,
	label15,
	label16,
	label17,
	label18,
	label19,
	label20,
	label21,
	label22,
	label23,
	label24,*/
	label25
};

#define NUM_IR_KEYS elements_of(button_labels)

static irrx_code_t ir_codes[NUM_IR_KEYS];

static uchar get_ir_key(irrx_code_t ircode)
{
	uchar i;
	for (i = 0; i < elements_of(ir_codes); i++)
	{
		if (ircode != IRRX_NO_CODE && ir_codes[i] == ircode)
			return i + 1;
	}
	return 0;
}

static void clear_ir_codes(void)
{
	memset(ir_codes, 0, sizeof(ir_codes));
}

static irrx_code_t ir_code= IRRX_NO_CODE; //Currently received ir code
static uchar ir_key_pressed = 0;

enum
{
	ProgramingMode_Not = 0,
	ProgramingMode_LAST = NUM_IR_KEYS

};

/**
 * The 2 byte report looks like this:
 *
 * ------------------------------------------------------------------------------------------
 * | Shift 0/1 | Keyboard (0-127) | IR consumer code (0-0xFF)                             |
 * ------------------------------------------------------------------------------------------
 *
 */
#define ReportDescriptor usbHidReportDescriptor

PROGMEM
#include "ir_keyboard_2.hid.h"

STATIC_ASSERT(sizeof(usbHidReportDescriptor) == USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH);

static void print_button_label(uint8_t programming_mode_type)
{
	//This code from the avrlibc FAQ:
	PGM_P p;
	memcpy_P(&p, &button_labels[programming_mode_type], sizeof(PGM_P));
	fprintf_P(&virtual_keyboard_file_io,p);
}

typedef enum
{
	report_id_none = 0,
	report_id_irremote = 1,
	report_id_virtualkeyboard = 2
} report_id_t;

static void buildReport(report_id_t id, uchar irkey, uchar vtLastKey)
{
	reportBuffer[0] = id;
	if (id == report_id_irremote)
	{
		reportBuffer[1] = irkey;
	}
	else if (id == report_id_virtualkeyboard)
	{
		reportBuffer[1] = vtLastKey;
	}
}

uchar usbFunctionSetup2(uchar data[8])
{
	usbRequest_t* rq = (void *)data;
	usbMsgPtr = reportBuffer;
	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS)
	{
		/* class request type */
		if(rq->bRequest == USBRQ_HID_GET_REPORT)
		{
			/* wValue: ReportType (highbyte), ReportID (lowbyte) */
			/* we only have one report type, so don't look at wValue */
			buildReport(rq->wValue.bytes[0],ir_key_pressed,vtLastKey);
			return sizeof(reportBuffer);
		}
		else if(rq->bRequest == USBRQ_HID_GET_IDLE)
		{
			usbMsgPtr = &idleRate;
			return 1;
		}
		else if(rq->bRequest == USBRQ_HID_SET_IDLE)
		{
			idleRate = rq->wValue.bytes[1];
		}
	}
	else
	{
		/* no vendor specific requests implemented */
	}
	return 0;
}

/* ------------------------------------------------------------------------- */

#define IR_TIMER_TIMEOUT 10 //10 x 22ms
#define BLINK_TIMER_TIMEOUT 30 //5 x 22ms

int main(void)
{
	uchar key = 0;
	uchar lastKey = 0;
	report_id_t keyDidChange = report_id_none;
	uchar idleCounter = 0;
	unsigned int virtual_keyboard_timer = 0;
	unsigned int ir_timer=0;
	unsigned int programming_mode_type = 0;
	unsigned int blink_timer=0;
	irrx_code_t last_ir_code = 0;
	hardwareInit();
	stdout = &usb_out_stream;

	usbDeviceDisconnect();
	wdt_enable(WDTO_2S);
  	odDebugInit();
	usbInit();
	irrx_init();
	irrx_power_on();
	LED_RED_INIT();
	LED_GREEN_INIT();
	SWITCH_PGM_INIT();
	sei();
	DBG1(0x00, 0, 0);
	eeprom_read_block(&ir_codes, 0, sizeof(ir_codes));

	//uart_io_init();
	usbDeviceConnect();
	/* main event loop */
	printf_P(PSTR("Booted!"));
	for (;;)
	{
		//Watchdog
		wdt_reset();

		//usb
		/////////////////////////////////////////////////////
		usbPoll();

		//IR Remote
		/////////////////////////////////////////////////////
		ir_code = irrx_getcode();
		if (ir_code != last_ir_code)
		{
			last_ir_code = ir_code;
			ir_key_pressed = get_ir_key(ir_code);
			if (last_ir_code != 0)
			{
				LED_RED_ON();
			}
			else
			{
				LED_RED_OFF();
			}
			if (programming_mode_type > ProgramingMode_Not)
			{
				if (ir_code != IRRX_NO_CODE)
				{
					fprintf_P(&virtual_keyboard_file_io,PSTR(" [0x%08lx]\r\n"),ir_code);
					if (ir_key_pressed == 0) //Is key already programmed?
					{
						//No,
						ir_codes[programming_mode_type - 1] = ir_code;

						programming_mode_type++;
						print_button_label(programming_mode_type);
						if (programming_mode_type >= ProgramingMode_LAST)
						{
							eeprom_write_block(&ir_codes, 0, sizeof(ir_codes));
							programming_mode_type = ProgramingMode_Not;
						}
					}
					else
					{
						//Yes
						fprintf_P(stdout,PSTR("\r\n"));
						print_button_label(programming_mode_type);
					}
				}
			}
			else
			{
				//fprintf_P(stdout,PSTR("A"));
				keyDidChange = report_id_irremote;
			}
		}

		//Key presses
		/////////////////////////////////////////////////////
		key = SWITCH_PGM_IS_PRESSED();
		if (lastKey != key)
		{
			if (key)
			{
				printf_P(PSTR("\nIR Keyboard (C) 2007 jorgen.birkler@gmail.com\n"));
				if (programming_mode_type == ProgramingMode_Not)
				{
					clear_ir_codes();
					fprintf_P(&virtual_keyboard_file_io,PSTR("\nIR Keyboard (C) 2007 jorgen.birkler@gmail.com\n"));
					print_button_label(programming_mode_type);
					programming_mode_type++;
					print_button_label(programming_mode_type);
				}
				else
				{
					programming_mode_type = ProgramingMode_Not;
					LED_RED_OFF();
				}
			}
			lastKey = key;
		}
		//Virtual keyboard (printf)
		/////////////////////////////////////////////////////
		if (virtual_keyboard_buffer_not_empty() && virtual_keyboard_timer == 0)
		{
			virtual_keyboard_timer = 4;
		}

		//USB interrupt
		/////////////////////////////////////////////////////
		if (keyDidChange && usbInterruptIsReady())
		{
			/* use last key and not current key status in order to avoid lost changes in key status. */
			buildReport(keyDidChange, ir_key_pressed, vtLastKey);
			usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
			keyDidChange = 0;
		}

		//Timer
		/////////////////////////////////////////////////////

		if (TIFR & _BV(TOV0))
		{
			/* 22 ms timer */
			TIFR = 1<<TOV0;
			if (programming_mode_type != ProgramingMode_Not)
			{
				if (blink_timer == 0)
				{
					LED_RED_CHANGE();
					blink_timer = BLINK_TIMER_TIMEOUT;
				}
				blink_timer--;
			}
			if (virtual_keyboard_timer > 0)
			{
				virtual_keyboard_timer--;
				if (virtual_keyboard_timer == 1)
				{
					vtLastKey = 0;
					keyDidChange = report_id_virtualkeyboard;
				}
				else if (virtual_keyboard_timer == 0)
				{
					keyDidChange = report_id_virtualkeyboard;
					vtLastKey
							= virtual_keyboard_translate_to_hid(virtual_keyboard_iogetchar());
					if (vtLastKey != 0)
					{
						virtual_keyboard_timer = 2;
					}
				}
			}

			if (ir_timer > 0)
			{
				ir_timer--;
				if (ir_timer == 0)
				{
					LED_RED_OFF();
					LED_GREEN_OFF();
					ir_key_pressed = 0;
					ir_code = IRRX_NO_CODE;
					keyDidChange = 1;
				}
			}
			if (idleRate != 0)
			{
				if (idleCounter > 4)
				{
					idleCounter -= 5; /* 22 ms in units of 4 ms */
				}
				else
				{
					idleCounter = idleRate;
					//keyDidChange = 1;
				}
			}
		}
	}
	return 0;
}

/* ------------------------------------------------------------------------- */

