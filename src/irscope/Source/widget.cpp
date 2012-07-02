
/*
	IR Widget comm and processing

    Copyright (C) 2007 Kevin Timmerman

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "stdafx.h"
#include "widget.h"
#include "IRScopeWnd.h"


static HANDLE comm_handle=NULL;

int OpenPort(CString port, UINT mode)
{
	port="\\\\.\\"+port;

	comm_handle=CreateFile(port,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);

	if(comm_handle==INVALID_HANDLE_VALUE) {
		const DWORD err=GetLastError();
		TRACE2("Error opening port: %s (%i)\n",port,err);
		comm_handle=NULL;
		return (int)err;
	}
	TRACE2("Port % s open - handle: %08X\n",port,comm_handle);

	// SetupComm(m_hComm,RxSize,TxSize);	// Setup Rx/Tx buffer sizes (optional)

	DCB dcb;
	dcb.DCBlength=sizeof(dcb);
	GetCommState(comm_handle,&dcb);
	dcb.BaudRate=115200;	// bit rate 
	dcb.ByteSize=8;			// number of bits/byte, 4-8 
	dcb.Parity=0;			// 0-4=no,odd,even,mark,space 
	dcb.StopBits=0;			// 0,1,2 = 1, 1.5, 2 
	dcb.fBinary=1;			// binary mode, no EOF check
	dcb.fParity=0;			// enable parity checking 
	dcb.fDtrControl=DTR_CONTROL_DISABLE; // DTR flow control type 
	dcb.fRtsControl=RTS_CONTROL_DISABLE; // RTS flow control 
	dcb.fOutxCtsFlow=0;		// CTS output flow control 
	dcb.fOutxDsrFlow=0;		// DSR output flow control 
	dcb.fDsrSensitivity=0;	// DSR sensitivity 
	dcb.fTXContinueOnXoff=0;// XOFF continues Tx 
	dcb.fOutX=0;			// XON/XOFF out flow control 
	dcb.fInX=0;				// XON/XOFF in flow control 
	dcb.fNull=0;			// enable null stripping 
	dcb.fAbortOnError=0;	// abort reads/writes on error 
	dcb.fErrorChar=0;		// enable error replacement 
	dcb.ErrorChar=(char)0xDE; // error replacement character
	//dcb.XonLim;			// transmit XON threshold 
	//dcb.XoffLim;			// transmit XOFF threshold 
	//dcb.XonChar;			// Tx and Rx XON character 
	//dcb.XoffChar;			// Tx and Rx XOFF character 
	//dcb.EofChar;			// end of input character 
	//dcb.EvtChar;			// received event character 
	SetCommState(comm_handle,&dcb);

	
	COMMTIMEOUTS to;
	GetCommTimeouts(comm_handle,&to);
	//to.ReadIntervalTimeout=(mode<2)?50:600;
	to.ReadIntervalTimeout=50;
	//to.ReadTotalTimeoutConstant=3000;
	to.ReadTotalTimeoutConstant=100;
	//to.ReadTotalTimeoutMultiplier=1;
	to.ReadTotalTimeoutMultiplier=0;
	//to.WriteTotalTimeoutMultiplier=;
	//to.WriteTotalTimeoutConstant=;
	SetCommTimeouts(comm_handle,&to);

	SetCommMask(comm_handle,0);

	//Sleep(500);

	switch(mode) {
		case 0: // IR Widget Pulse
		case 2: // Discrete Pulse
			EscapeCommFunction(comm_handle,SETDTR);
			Sleep(200);
			EscapeCommFunction(comm_handle,SETRTS);
			break;
		case 1:	// MiniPOV3 Pulse
			EscapeCommFunction(comm_handle,SETRTS);
			Sleep(200);
			EscapeCommFunction(comm_handle,SETDTR);
			break;

		case 3:	// IR Widget Time
			EscapeCommFunction(comm_handle,SETRTS);
			EscapeCommFunction(comm_handle,SETDTR);
			break;
		case 4: // MiniPOV3 Time
			EscapeCommFunction(comm_handle,SETDTR);
			EscapeCommFunction(comm_handle,SETRTS);
			break;
	}

	return 0;
}

void ClosePort(void)
{
	if(!comm_handle) return;

	EscapeCommFunction(comm_handle,CLRDTR);
	EscapeCommFunction(comm_handle,CLRRTS);

	CloseHandle(comm_handle);

	comm_handle=NULL;
}

UINT ReadPort(UINT len, BYTE* data, DWORD duration)
{
	PurgeComm(comm_handle,PURGE_RXCLEAR);

	DWORD err;
	ClearCommError(comm_handle,&err,NULL);
	TRACE1("Comm error: %08X\n",err);
/*
	winbase.h
	CE_RXOVER           0x0001  // Receive Queue overflow
	CE_OVERRUN          0x0002  // Receive Overrun Error
	CE_RXPARITY         0x0004  // Receive Parity Error
	CE_FRAME            0x0008  // Receive Framing error
	CE_BREAK            0x0010  // Break Detected
	CE_TXFULL           0x0100  // TX Queue is full
*/

	DWORD timeout=GetTickCount()+3000;
	DWORD read;
	DWORD total_read=0;
	BYTE* px=data;
	while(total_read < len && static_cast<short>(timeout-GetTickCount()) >= 0) {
		TRACE1("t- %i\n",static_cast<short>(timeout-GetTickCount()));
		read=len-total_read;
		if(read > 4096) read=4096;
		ReadFile(comm_handle,px,read,&read,NULL);
		//if(read==0) break;
		if(total_read==0 && read!=0) timeout=GetTickCount()+duration;
		TRACE1("%i ",read);
		px+=read;
		total_read+=read;
	}
	read=total_read;

	ClearCommError(comm_handle,&err,NULL);
	TRACE1("Comm error: %08X\n",err);

	return read;
}


int ProcessPulseData(UINT len, BYTE* data, int& count, int*& times, short int*& counts, UINT interval, UINT& freq)
{
	// Find start of signal
	for(UINT n=0;n<len-1;n++)
		if(data[n]!=data[n+1]) break;
	const UINT pcl=len-1-n;
	TRACE2("Start: %i   Len: %i\n",n,pcl);
	if(!pcl)
		return 1;


	// Convert running count to absolute count
	BYTE* pc=new BYTE[pcl];
	if(!pc) return -1;
	BYTE* ppc=pc;
	for(;n<len-1;n++)
		*ppc++=(data[n+1]-data[n])&0x3F;


	// Count transitions and sum running counts
	BOOL state=(pc[0]==0);
	BYTE last=0;
	UINT tc=0;	// Times count
	int s=0;
	int t=0;
	for(n=0;n<pcl;n++) {
		const BYTE b=pc[n];
		const BOOL new_state=(b!=0);
		if(state ^ new_state) {
			if(state) {
				// On to off transition
				if(last) {
					s-=last;
					--t;
				}
			} else {
				// Off to on transition
				last=0;
			}
			++tc;
			state=new_state;
		} else if(state) {
			// Still on
			s+=b;
			last=b;
			++t;
		}
	}

	//if(t<10) { s=0; t=0; }

	TRACE1("Times: %i\n",tc);
	TRACE2("Sum of on counts: %i  Periods of on counts: %i\n",s,t);
	
	// Calculate carrier frequency in Hertz
	freq=(interval&&t)?static_cast<UINT>(static_cast<unsigned __int64>(s)*1000000/(interval*t)):0;
	TRACE1("Frequency: %i Hz\n",freq);
	
	// Calculate carrier period in microseconds
	const int period=s?t*interval/s:0;
	TRACE1("Period: %i uS\n",period);

	// Allocate memory for times and counts
	times=new int[tc];
	counts=new short int[tc];
	if(times==NULL || counts==NULL) {
		delete[] pc;
		delete[] times;
		delete[] counts;
		return -2;
	}

	// Calculate on time, on count and off time
	UINT i=0;
	state=(pc[0]==0);
	int time=0;
	int pulses=0;
	const int max_time=interval*3/2;
	for(n=0;n<pcl;n++) {
		const BYTE b=pc[n];
		const BOOL new_state=(b!=0);
		if(state ^ new_state) {
			if(state) {
				// On to off transition
				int on_time = last ? s ? t*interval*last/s : 0 : interval;
				const int off_time=(on_time > max_time) ? 0 : interval-on_time;
				times[i]=time-off_time;
				time=off_time+interval;
			} else {
				// Off to on transition
				int on_time = s ? t*interval*b/s : interval;
				if(on_time > max_time) on_time=interval;
				const int off_time=interval-on_time;
				times[i]=-(time+off_time);
				time=on_time;
				last=0;
			}
			counts[i]=pulses;
			pulses=b;
			if(n) ++i;
			state=new_state;
		} else {
			time+=interval;
			pulses+=b;
			last=b;
		}
	}
	counts[i]=pulses;
	times[i]=state?time:-time;
	ASSERT(i==tc-1);
	count=tc;

	delete[] pc;

	return 0;
}

int ProcessTimeData(UINT len, BYTE* data, int& count, int*& times)
{
	if(len<4) return 1;
	ASSERT((len&1)==0);
	count=0;
	times = new int[len/2];
	if(!times) return -1;
	for(UINT u=2; u<len; u+=2) {
		int t = data[u] | data[u+1]<<8;
		t = (t&0x8000) ? t&0x7FFF : -t;
		t *= 16;
		TRACE1("%5i\n",t);
		times[count++] = t;
	}
	ASSERT(count);
	if(times[count-1]>0) times[count++]=-200000;
	return 0;
}

UINT WriteFile(CString file_name, int count, int* times, short int* counts, UINT freq)
{
	if(!count) return 0;

	CFile file;
	if(file.Open(file_name,CFile::modeWrite | CFile::modeCreate)) {
		CString s("irscope 0\r\n");
		file.Write(s,s.GetLength());
		if(freq!=UINT_MAX) {
			s.Format("carrier_frequency %i\r\n",freq);
			file.Write(s,s.GetLength());
		}
		s.Format("sample_count %i\r\n",count);
		file.Write(s,s.GetLength());
		for(int i=0;i<count;i++) {
			if(counts!=NULL && counts[i])
				s.Format("%+i,%i\r\n",times[i],counts[i]);
			else
				s.Format("%+i\r\n",times[i]);
			file.Write(s,s.GetLength());
		}
	} else
		return GetLastError();

	return 0;
}
