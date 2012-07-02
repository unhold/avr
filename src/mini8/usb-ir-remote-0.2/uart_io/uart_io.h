/**
 * \file
 *  _     _      _    _                    
 * | |__ (_)_ __| | _| | ___ _ __ ___  ___ 
 * | '_ \| | '__| |/ / |/ _ \ '__/ __|/ _ \
 * | |_) | | |  |   <| |  __/ | _\__ \  __/
 * |_.__/|_|_|  |_|\_\_|\___|_|(_)___/\___|
 *         
 * 
 * Public interface to the uart io subsystem
 * 
 * Copyright 2007 J�rgen Birkler
 * jorgen.birkler@gmail.com
 * 
 */

#ifndef UART_IO_H_
#define UART_IO_H_

#include <stdio.h>

/**
 * Initialize the UART, interrupts, pin and software inverter.
 */
FILE* uart_io_init(void);


/**
 * Returns 1 if ok to disable CPU clock 0 if not
 * 
 * @note Not implemented; always return 1
 */
char uart_io_ok_to_powersave(void);

#endif /*UART_IO_H_*/
