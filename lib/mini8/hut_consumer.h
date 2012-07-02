// HID consumer page (0x0C) controls
#ifndef HUT_CONSUMER_H
#define HUT_CONSUMER_H

enum {

	CC_POWER               = 0x30,
	CC_RESET               = 0x31,
	CC_SLEEP               = 0x32,

	CC_MENU                = 0x40,
	CC_MENU_PICK           = 0x41,
	CC_MENU_UP             = 0x42,
	CC_MENU_DOWN           = 0x43,
	CC_MENU_LEFT           = 0x44,
	CC_MENU_RIGHT          = 0x45,
	CC_MENU_ESCAPE         = 0x46,
	CC_MENU_VALUE_INCREASE = 0x47,
	CC_MENU_VALUE_DECREASE = 0x48,

	CC_PLAY                = 0xB0,
	CC_PAUSE               = 0xB1,
	CC_RECORD              = 0xB2,
	CC_FAST_FORWARD        = 0xB3,
	CC_REWIND              = 0xB4,
	CC_SCAN_NEXT_TRACK     = 0xB5,
	CC_SCAN_PREVIOUS_TRACK = 0xB6,
	CC_STOP                = 0xB7,
	CC_EJECT               = 0xB8,
	CC_RANDOM_PLAY         = 0xB9,

	CC_MUTE                = 0xE2,

	CC_VOLUME_INCREMENT    = 0xE9,
	CC_VOLUME_DECREMENT    = 0xEA

};

#endif // HUT_CONSUMER_H

