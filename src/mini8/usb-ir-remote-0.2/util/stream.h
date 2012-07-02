
/*
 * Copyright 2010 Jorgen Birkler
 * jorgen.birkler)a(gmail.com
 *
 * fifo stream for input/output
 */

#ifndef __stream_h__
#define __stream_h__


#include <stdint.h>

#define STREAM_FLAG_OVERWRITE 0x01

typedef struct {
	uint8_t flags;
	uint8_t bufmask;
	uint8_t read;
	uint8_t write;
	uint8_t data[];
} __attribute__((__packed__)) stream_buffer_t;



int stream_putchar(stream_buffer_t* s, char c);
int stream_getchar_nowait(stream_buffer_t* s);
int stream_getchar_wait(stream_buffer_t* s);

#endif //__stream_h__

