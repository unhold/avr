#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#define USB_GET_REPORT_IDS
#include "usbdrv.h"

#include "stream.h"
#include "usbstream.h"

#ifndef USBSTREAM_DISABLE_RX
#define USBSTREAM_ENABLE_RX
#endif //#ifndef DISABLE_RX

#define noinline __attribute__((noinline))


typedef struct {
	uint16_t magic;
	stream_buffer_t base;
} usb_stream_buffer_internal_t;


typedef struct {
	usb_stream_buffer_internal_t internal;
	uint8_t data[256];
}usb_stream_buffer_out_t;

typedef struct {
	usb_stream_buffer_internal_t internal;
	uint8_t data[16];
}usb_stream_buffer_in_t;


static usb_stream_buffer_out_t usb_out_stream_buffer;
static uint8_t initialized = 0;


//Streams
///////////////////////////////////////


static int usb_stream_putchar(char c, FILE *stream);
static int usb_stream_getchar(FILE *stream);
#define FDEV_SETUP_STREAM2(p, g, f, u) \
	{ \
		.put = p, \
		.get = g, \
		.flags = f, \
		.udata = u, \
	}

FILE usb_out_stream = FDEV_SETUP_STREAM2(usb_stream_putchar, NULL, _FDEV_SETUP_WRITE,&usb_out_stream_buffer);
#ifdef USBSTREAM_ENABLE_RX
static usb_stream_buffer_in_t usb_in_stream_buffer;
FILE usb_in_stream = FDEV_SETUP_STREAM2(usb_stream_putchar,usb_stream_getchar, _FDEV_SETUP_READ|_FDEV_SETUP_WRITE,&usb_in_stream_buffer);
#else
FILE usb_in_stream = FDEV_SETUP_STREAM2(NULL,usb_stream_getchar, _FDEV_SETUP_READ,NULL);
#endif

#define USB_STREAM_MAGIX 0xBABE
#define USB_STREAM_OVERWRITE 0x01

static noinline void usb_stream_init(usb_stream_buffer_internal_t* s)
{
	if (initialized == 0) {
		usb_out_stream_buffer.internal.base.bufmask = 0x7F;
		usb_out_stream_buffer.internal.base.flags = USB_STREAM_OVERWRITE;
#ifdef USBSTREAM_ENABLE_RX
		usb_in_stream_buffer.internal.base.bufmask = 0x0F;
		usb_in_stream_buffer.internal.base.flags = 0;
#endif
		initialized = 1;
	}
	if (s->magic != USB_STREAM_MAGIX) {
		s->magic = USB_STREAM_MAGIX;
		s->base.read = 0;
		s->base.write = 0;
	}
}

static noinline int usb_stream_putchar(char c, FILE *stream)
{
	usb_stream_buffer_internal_t* s = fdev_get_udata(stream);
	usb_stream_init(s);
	return stream_putchar(&s->base,c);
}

static noinline int usb_stream_getchar(FILE *stream)
{
	usb_stream_buffer_internal_t* s = fdev_get_udata(stream);
	usb_stream_init(s);
	return stream_getchar_nowait(&s->base);
}

static uchar usb_buf_state;
static uchar usb_buf_report_id;


usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (void *)data;
	usb_buf_report_id = rq->wValue.bytes[0];
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */

        	switch (rq->wValue.bytes[0]) {
        		case report_id_buf_out_data:
        			usb_buf_state = 0;
        			PORTB |= _BV(PB2);
        			return USB_NO_MSG;
#ifdef USBSTREAM_ENABLE_RX
        		case report_id_buf_in_getwritepos:
        			usbMsgPtr = (void*)&usb_in_stream_buffer.internal.base.write;
        			return sizeof(usb_in_stream_buffer.internal.base.write);
#endif //#ifdef USBSTREAM_ENABLE_RX
        		default:
        			break;
        	}
        }else if(rq->bRequest == USBRQ_HID_SET_REPORT){
        	switch (rq->wValue.bytes[0]) {
        		case report_id_buf_out_setreadpos:
        			return USB_NO_MSG;

        		case report_id_buf_in_data:
        			usb_buf_state = 0;
        			return USB_NO_MSG;

        		default:
        			break;
        	}
        }
    }
    return usbFunctionSetup2(data);
}


uchar usbFunctionRead(uchar *data, uchar len)
{
	if (usb_buf_report_id == report_id_buf_out_data) {
		uchar i;
		for (i = 0;i<len;i++) {
			if (usb_buf_state < sizeof(usb_out_stream_buffer.internal.base))
			{
				data[i] = ((uchar*)&(usb_out_stream_buffer.internal.base))[usb_buf_state];
			}
			else {
				uchar read_pos = usb_out_stream_buffer.internal.base.read + usb_buf_state - sizeof(usb_out_stream_buffer.internal.base);
				read_pos &= usb_out_stream_buffer.internal.base.bufmask;
				data[i] = usb_out_stream_buffer.internal.base.data[read_pos];
			}
			usb_buf_state++;
		}
		return len;
	}
	return usbFunctionRead2(data,len);
}

#ifdef USBSTREAM_ENABLE_RX
static uchar usb_stream_buffer_write_len;
#endif


uchar usbFunctionWrite(uchar *data, uchar len)
{
	if (usb_buf_report_id == report_id_buf_out_setreadpos)
	{
		usb_out_stream_buffer.internal.base.read = *((uint16_t*)&data[1]);
		return len;
	}
#ifdef USBSTREAM_ENABLE_RX
	else if (usb_buf_report_id == report_id_buf_in_data) {
		int i;
		for (i=0;i<len;i++) {
			if (usb_buf_state == 0){
				usb_buf_state++;
			}
			else if (usb_buf_state == 1) {
				usb_stream_buffer_write_len = data[i];
				usb_buf_state++;
			}
			else if (usb_stream_buffer_write_len > 0){
				fputc(data[i],&usb_in_stream);
				usb_stream_buffer_write_len--;
			}
		}
		return len;
	}
#endif //#ifdef USBSTREAM_ENABLE_RX
	return usbFunctionWrite2(data,len);
}

uchar __attribute__((__weak__)) usbFunctionRead2(uchar *data, uchar len)
{
	return 0;
}
uchar __attribute__((__weak__)) usbFunctionWrite2(uchar *data, uchar len)
{
	return 0;
}
