/*
  Flashtool for AVRUSBBoot, an USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>

  Creation Date..: 2006-03-18
  Last change....: 2006-06-25
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include "libs-host/hiddata.h"
#define USB_GET_REPORT_IDS
#define USB_HOST_ONLY_GET_IDS
#include "../usbconfig.h"


static char *usbErrorMessage(int errCode);
static void adjustConsole(void);
static int kb_input(void);


typedef struct {
	usbDevice_t* device;
	int error;
} logdevice_t;



static int waitOpenDevice(logdevice_t* logdevice)
{
	unsigned char   rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
	char            vendorName[] = {USB_CFG_VENDOR_NAME, 0};
	char            productName[] = {USB_CFG_DEVICE_NAME, 0};
	int             vid = rawVid[0] + 256 * rawVid[1];
	int             pid = rawPid[0] + 256 * rawPid[1];
	const char* feedback = "|/-\\";
	unsigned char feedbackpos = 0;

	if (!logdevice->error && logdevice->device) return 1;
	if (logdevice->error) {
		if (logdevice->device) {
			usbhidCloseDevice(logdevice->device);
			logdevice->device = 0;
		}
		logdevice->error = 0;
	}

	while (1) {
		int res  = usbhidOpenDevice(&logdevice->device, vid, vendorName, pid, productName, 1);

		if (res == USBOPEN_SUCCESS) {
			printf("\nHID Device %04x:%04x \"%s\"/\"%s\" open.\n",vid,pid,vendorName,productName);
			printf("-----------------------------------------------\n");
			return 1;
		}
		else if (res == USBOPEN_ERR_NOTFOUND) {
			printf("\rWaiting HID Device %04x:%04x \"%s\"/\"%s\" %c",vid,pid,vendorName,productName,feedback[feedbackpos++ & 0x3]);
			usleep(200000);
		}
		else {
			//printf("Open HID Device failed. %d\n",res);
		}
	}
	return 0;
}



int main(int argc, char **argv) {
	logdevice_t logdevicedata = {0,0};
	logdevice_t* logdevice = &logdevicedata;
	int err;

	setvbuf (stdout,0,_IONBF ,0); //no buffering out
	setvbuf (stdin,0,_IONBF ,0);//no buffering in

	adjustConsole();


	char buffer[128+4];
	unsigned char read_pos = 0;

	while (waitOpenDevice(logdevice))
	{
		int readdata = 0;
		buffer[0] = report_id_buf_out_data;
		int len = 69;

		if((err = usbhidGetReport(logdevice->device,report_id_buf_out_data, &buffer[0], &len)) != 0)
		{
			printf("error reading data: %s\n", usbErrorMessage(err));
			logdevice->error = 1;
		}
		if (err == 0) {
			stream_buffer_t* stream_buffer = (stream_buffer_t*)&buffer[0];
			uint16_t buflen = (stream_buffer->write + stream_buffer->bufmask + 1 - stream_buffer->read) & stream_buffer->bufmask;

			stream_buffer->data[buflen] = '\0';

			//printf("resp=%d bufmask=0x%02x write=%d read=%d len=%d buffer:%s\n",len,stream_buffer->bufmask,stream_buffer->write,stream_buffer->read,buflen, &stream_buffer->data[0]);

			while (read_pos != stream_buffer->read) {
				read_pos = (read_pos + 1) & stream_buffer->bufmask;
				readdata = 1;
			}
			int i =0;
			while (read_pos != stream_buffer->write && i < 64){
				printf("%c",(int)stream_buffer->data[i]);
				i++;
				read_pos = (read_pos + 1) & stream_buffer->bufmask;
				readdata = 1;
			}

			//printf("read_pos: %d\n", read_pos);
			buffer[0] = report_id_buf_out_setreadpos;
			buffer[1] = read_pos&0xFF;
			buffer[2] = read_pos>>8;
			len = 3;

			if((err = usbhidSetReport(logdevice->device,buffer, len)) != 0)
			{
				printf("error writing data: %s\n", usbErrorMessage(err));
				logdevice->error = 1;
			}
		}

		int c;
		if ((c = kb_input()) >= 0) {
			//printf("Sending \"%c\" 0x%02x",c,c);
			if (c=='\r') {c = '\n';}
			buffer[0] = report_id_buf_in_data;
			buffer[1] = 1;
			buffer[2] = c;
			len = 10;

			if((err = usbhidSetReport(logdevice->device,buffer, len)) != 0)
			{
				printf("error writing data: %s\n", usbErrorMessage(err));
				logdevice->error = 1;
			}
		}
		else {
			//printf(".");
			if (!readdata) {

				usleep(50000);
			}
		}
	}

	usbhidCloseDevice(logdevice->device);
	logdevice->device = 0;
	return 0;
}




static char *usbErrorMessage(int errCode)
{
	static char buffer[80];

	switch(errCode){
	case USBOPEN_ERR_ACCESS:      return "Access to device denied";
	case USBOPEN_ERR_NOTFOUND:    return "The specified device was not found";
	case USBOPEN_ERR_IO:          return "Communication error with device";
	default:
		sprintf(buffer, "Unknown USB error %d", errCode);
		return buffer;
	}
	return NULL;    /* not reached */
}

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


VOID ErrorExit (LPSTR lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);
    ExitProcess(0);
}


#include <conio.h>
#include <stdio.h>



static int kb_input(void) {
	if (kbhit()) {
		return _getch();
	}
	else {
		return -1;
	}
}
static void adjustConsole(void) {
	    HANDLE hStdin;
	    DWORD fdwMode;
	    // Get the standard input handle.

	    hStdin = GetStdHandle(STD_INPUT_HANDLE);
	    if (hStdin == INVALID_HANDLE_VALUE)
	        ErrorExit("GetStdHandle");

	    // Get the current input mode

	    if (! GetConsoleMode(hStdin, &fdwMode) )
	        ErrorExit("GetConsoleMode");
	    fdwMode &= ~ENABLE_LINE_INPUT & ~ENABLE_ECHO_INPUT;

	    fdwMode |=  ENABLE_PROCESSED_INPUT;

	    if (! SetConsoleMode(hStdin, fdwMode) )
	        ErrorExit("SetConsoleMode");

}
#else
static void adjustConsole(void) {
	//TOFIX
}
static int kb_input(void) {
	//TOFIX
	return -1;
}

#endif //#ifdef _WIN32


