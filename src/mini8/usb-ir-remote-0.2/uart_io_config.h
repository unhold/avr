/**
 *  _     _      _    _                    
 * | |__ (_)_ __| | _| | ___ _ __ ___  ___ 
 * | '_ \| | '__| |/ / |/ _ \ '__/ __|/ _ \
 * | |_) | | |  |   <| |  __/ | _\__ \  __/
 * |_.__/|_|_|  |_|\_\_|\___|_|(_)___/\___|
 *         
 * 
 * Config for the UART IO pin
 * 
 * Copyright 2007 Jörgen Birkler
 * jorgen.birkler@gmail.com
 * 
 */

#ifndef UART_IO_CONFIG_H_
#define UART_IO_CONFIG_H_


///Port for output
#define UART_IO_TX_OUTPORT D

///Pin for output
#define UART_IO_TX_OUTPIN 3



#ifndef F_CPU 
///Clock frequency so that baud rate register can be calculated
#define F_CPU 12000000L
#endif


///Baudrate for uart
#define UART_IO_BAUDRATE 19200

#define UART_IO_BUFFER_SIZE_FACTOR 7

#endif /*UART_IO_CONFIG_H_*/
