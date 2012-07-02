/**
 * \file
 *  _   _ ____  ____    ___        ____                      _
 * | | | / ___|| __ )  |_ _|_ __  |  _ \ ___ _ __ ___   ___ | |_ ___
 * | | | \___ \|  _ \   | || '__| | |_) / _ \ '_ ` _ \ / _ \| __/ _ \
 * | |_| |___) | |_) |  | || |    |  _ <  __/ | | | | | (_) | ||  __/
 *  \___/|____/|____/  |___|_|    |_| \_\___|_| |_| |_|\___/ \__\___|
 *
 *
 * Copyright 2007 J�rgen Birkler
 * jorgen.birkler@gmail.com
 * USB HID device for IR receiver
 * License: GNU GPL v2 (see License.txt) or proprietary (contact author)
 *  *
 * Bit length IR receiver (Suitable for Panasonic remotes)
 *
 * Copyright 2006 J�rgen Birkler
 * mailto:jorgen.birkler@gmail.com
 */

#ifndef IRRX_H_
#define IRRX_H_


#include <inttypes.h>

// ***********************
// * External API
// ***********************

/**
 * Ir code received
 */
typedef uint32_t irrx_code_t;


///No code received
#define IRRX_NO_CODE (0)


/**
 * Get received ir code
 * @retal Ircode Ir code received
 * @retval IRRX_NO_CODE if not ir code received
 */
irrx_code_t irrx_getcode(void);



/**
 * Initialize hardware ports for ir receiver. Call once during reset
 */
void irrx_init(void);


/**
 * Start reception of ir codes by giving power to ir receiver
 * and init timers and interrupts
 */
void irrx_power_on(void);


/**
 * Disables power to the ir receiver and disables interrupts
 */
void irrx_power_off(void);





#endif /*IRRX_H_*/
