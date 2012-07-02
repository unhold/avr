
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "stream.h"

int stream_putchar(stream_buffer_t* s, char c)
{
	uint8_t newWritePos = (s->write + 1) & s->bufmask;
	if (newWritePos == s->read) {
		if (s->flags & STREAM_FLAG_OVERWRITE) {
			s->read++;
			s->read &= s->bufmask;
		}
		else {
			return -1;
		}
	}
	s->data[s->write] = c;
	s->write = newWritePos;
	return c;
}


int stream_getchar_nowait(stream_buffer_t* s)
{
	int res = -1;
	if (s->read != s->write) {
		res = s->data[s->read++];
		s->read &= s->bufmask;
	}
	return res;
}

int stream_getchar_wait(stream_buffer_t* s)
{

	int res;
	do {
		res = stream_getchar_nowait(s);
	} while (res == -1);
	return res;
}

