

/*
 * Copyright 2010 Jorgen Birkler
 * jorgen.birkler)a(gmail.com
 *
 * fifo stream for input/output
 */
#ifndef __usb_stream_h__
#define __usb_stream_h__


extern FILE usb_out_stream;
extern FILE usb_in_stream;


//Implement these functions instead, these will be called when the requests are not for the streams.
usbMsgLen_t usbFunctionSetup2(uchar data[8]);
uchar usbFunctionRead2(uchar *data, uchar len);
uchar usbFunctionWrite2(uchar *data, uchar len);


#endif //#ifndef __usb_stream_h__
