
#define USE_FP_MATH

#include <stdafx.h>
#include "audio.h"


#ifdef USE_FP_MATH
#include "math.h"
const double dPi=3.1415927;
const double dPi2=dPi*2;
#endif

const UINT c_uAudioBufferSize=60*1024;


/*
int main()
{
	CAudio a;

	//a.SetSampleRate(44100);
	a.SetSampleRate(48000);

	const UINT max_carrier = a.GetSampleRate()*10/11;
	TRACE1("Max carrier frequency: %i Hz\n",max_carrier);

	for(UINT n=0; n<num_codes; ++n) {

		const powercode &c=*powerCodes[n];

		if(c.freq > max_carrier) {
			TRACE2("Code %s skipped - freq = %i Hz\n",codeNames[n],c.freq);
			continue;
		}

		CString s;
		s.Format("%03i_%s.wav",n,codeNames[n]);
		a.Open(s);

		a.SetFrequency(c.freq/2);

		UINT u=0;
		while(c.codes[u].offTime) {
			a.Pulse(c.codes[u].onTime*10);
			a.Pulse(c.codes[u].offTime*-10);
			++u;
		}
		a.Pulse(c.codes[u].onTime*10);
		a.Pulse(-100*1000);

		a.Close();
	}

	return 0;
}
*/





CAudio::CAudio()
{
	ASSERT(sizeof(short int)==2);
	ASSERT(sizeof(UINT)==4);
	ASSERT(sizeof(DWORD)==4);

	m_bFile=FALSE;
	m_hWaveOut=NULL;
	m_pbyAudioBuf=NULL;

	SetSampleRate(48000,TRUE);
}

CAudio::~CAudio()
{
	Close();
}

int CAudio::Open(LPCSTR sFile)
{
	m_bFile=(sFile!=NULL);

	delete[] m_pbyAudioBuf;
	m_pbyAudioBuf=new BYTE[c_uAudioBufferSize];
	m_uAudioBuf=m_uAudioLen=0;
	m_pbyWaveData=m_pbyAudioBuf;

	if(m_bFile) {
		TRACE1("File: %s\n",sFile);
		m_file.Open(sFile,CFile::modeWrite|CFile::modeCreate);
		m_uAudioBufferLimit=c_uAudioBufferSize;
		RiffHeader();
	} else {
		// MMRESULT waveOutOpen(LPHWAVEOUT phwo, UINT uDeviceID, LPWAVEFORMATEX pwfx,
		//		DWORD dwCallback, DWORD dwCallbackInstance, DWORD fdwOpen);

		waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &m_wf, 0, 0, CALLBACK_NULL | WAVE_ALLOWSYNC);

		m_uAudioBufferLimit=c_uAudioBufferSize>>1;

		m_bPending[0]=m_bPending[1]=FALSE;
		m_pHdr=m_hdr;
		m_pbPending=m_bPending;
	}

	return 0;
}

void CAudio::Close(void)
{
	FlushAudioOutput();
	FlushAudioOutput();	// Both waveOut buffers

	if(m_bFile) {
		RiffHeader();
		m_file.Close();
	} else {
		if(m_hWaveOut) waveOutClose(m_hWaveOut);
	}

	delete[] m_pbyAudioBuf;
	m_pbyAudioBuf=NULL;
	m_bFile=FALSE;
	m_hWaveOut=NULL;
}

void CAudio::RiffHeader(void)
{
	memcpy(riff_hdr.id,"RIFF",4);

	memcpy(fmt_chunk_hdr.id,"fmt ",4);
	fmt_chunk_hdr.len=sizeof(fmt_chunk)+sizeof(fmt_mspcm);

	memcpy(data_chunk_hdr.id,"data",4);

	m_file.Seek(0,CFile::begin);

	riff_hdr.len=4+sizeof(fmt_chunk_hdr)+sizeof(fmt_chunk)+
			sizeof(fmt_mspcm)+sizeof(data_chunk_hdr)+m_uAudioLen;
	m_file.Write(&riff_hdr,sizeof(riff_hdr));

	m_file.Write("WAVE",4);

	m_file.Write(&fmt_chunk_hdr,sizeof(fmt_chunk_hdr));

	m_file.Write(&fmt_chunk,sizeof(fmt_chunk));

	m_file.Write(&fmt_mspcm,sizeof(fmt_mspcm));

	data_chunk_hdr.len=m_uAudioLen;
	m_file.Write(&data_chunk_hdr,sizeof(data_chunk_hdr));

}

void CAudio::SetSampleRate(UINT uSampleRate, BOOL b16Bit, UINT uChannels)
{
	m_uSampleRate=uSampleRate;
	m_uSampleBytes=b16Bit?2:1;
	m_uChannels=uChannels;

	m_uMaxCarrierFrequency=uSampleRate*10/11;

	TRACE1("Sample Rate: %u\n",m_uSampleRate);
	TRACE1("Sample Bytes: %u\n",m_uSampleBytes);
	TRACE1("Channels: %u\n",m_uChannels);

	// File
	fmt_chunk.wFormatTag=1;						// PCM linear
	fmt_chunk.wChannels=m_uChannels;
	fmt_chunk.dwSamplesPerSec=m_uSampleRate;	// Sample rate
	fmt_chunk.dwAvgBytesPerSec=m_uSampleRate*m_uSampleBytes*m_uChannels;
	fmt_chunk.wBlockAlign=m_uChannels*m_uSampleBytes;

	fmt_mspcm.wBitsPerSample=8*m_uSampleBytes;

	// Sound card
	m_wf.wFormatTag=WAVE_FORMAT_PCM;
	m_wf.nChannels=m_uChannels;
	m_wf.nSamplesPerSec=m_uSampleRate;
	m_wf.wBitsPerSample=8*m_uSampleBytes;
	m_wf.nBlockAlign=m_uChannels*m_uSampleBytes;
	m_wf.nAvgBytesPerSec=m_uSampleRate*m_uSampleBytes*m_uChannels;
	m_wf.cbSize=0;
}

int CAudio::SetCarrierFrequency(UINT freq)
{
	if(freq>GetMaxCarrierFrequency()) return 1;

	m_uCarrierFrequency=freq;

#ifdef USE_FP_MATH
	m_dPhase=0.0;
	m_dPhaseIncrement=m_uCarrierFrequency*dPi/m_uSampleRate;
#else
	m_uPhase=0;
	m_uPhaseTotal=256*(1<<20);
	m_uPhaseHalf=m_uPhaseTotal/2;
	m_uPhaseIncrement=static_cast<UINT>(static_cast<unsigned __int64>(m_uCarrierFrequency)*(m_uPhaseTotal/2)/m_uSampleRate);
	TRACE("Phase Increment Whole: %i\n",m_uPhaseIncrement>>20);
	TRACE("Phase Increment Fractional: %i\n",m_uPhaseIncrement&0x000FFFFF);
	TRACE("Frequency: %i Hz\n",static_cast<UINT>(static_cast<unsigned __int64>(m_uSampleRate)*m_uPhaseIncrement/m_uPhaseTotal));
#endif

	return 0;
}

void CAudio::FlushAudioOutput(void)
{
	if(m_bFile) {
		if(m_uAudioBuf==0) return;
		m_file.Write(m_pbyWaveData,m_uAudioBuf);
	} else {
		if(!m_hWaveOut) return;
		if(m_uAudioBuf) {
			m_pHdr->lpData=(char*)m_pbyWaveData;
			m_pHdr->dwBufferLength=m_uAudioBuf;
			m_pHdr->dwBytesRecorded=0;
			m_pHdr->dwUser=0;
			m_pHdr->dwFlags=0;
			m_pHdr->dwLoops=0;
			m_pHdr->lpNext=0;
			m_pHdr->reserved=0;

			TRACE2("Writing %u bytes to buffer %i\n",
				m_pHdr->dwBufferLength,m_pHdr==&m_hdr[0]?0:1);

			waveOutPrepareHeader(m_hWaveOut, m_pHdr, sizeof(m_hdr));
			waveOutWrite(m_hWaveOut, m_pHdr, sizeof(m_hdr));

			*m_pbPending=TRUE;
		} else
			*m_pbPending=FALSE;

		if(m_pHdr==&m_hdr[0]) {
			ASSERT(m_pbyWaveData==m_pbyAudioBuf);
			m_pHdr=&m_hdr[1];
			m_pbPending=&m_bPending[1];
			m_pbyWaveData=m_pbyAudioBuf+m_uAudioBufferLimit;
		} else {
			ASSERT(m_pHdr==&m_hdr[1]);
			ASSERT(m_pbyWaveData==m_pbyAudioBuf+m_uAudioBufferLimit);
			m_pHdr=&m_hdr[0];
			m_pbPending=&m_bPending[0];
			m_pbyWaveData=m_pbyAudioBuf;
		}
		TRACE("Switch to buffer %i\n",m_pHdr==&m_hdr[0]?0:1);
		if(*m_pbPending) {
			while(((volatile)m_pHdr->dwFlags & WHDR_DONE)==0) Sleep(50);
			waveOutUnprepareHeader(m_hWaveOut, m_pHdr, sizeof(m_hdr));
			TRACE("Buf %i done, ready for more data\n",m_pHdr==&m_hdr[0]?0:1);
			*m_pbPending=FALSE;
		}
	}
	m_uAudioLen+=m_uAudioBuf;
	m_uAudioBuf=0;
}


UINT CAudio::Silence(const UINT uSamples)
{
	for(UINT u=0; u<uSamples; ++u) {
		if(m_uSampleBytes==1) {
			m_pbyWaveData[m_uAudioBuf++]=0x80;
			if(m_uChannels==2) {
				m_pbyWaveData[m_uAudioBuf++]=0x80;
			}
		} else {
			*((short int*)(&m_pbyWaveData[m_uAudioBuf]))=0;
			m_uAudioBuf+=2;
			if(m_uChannels==2) {
				*((short int*)(&m_pbyWaveData[m_uAudioBuf]))=0;
				m_uAudioBuf+=2;
			}
		}
		if(m_uAudioBuf>=m_uAudioBufferLimit) {
			ASSERT(m_uAudioBuf==m_uAudioBufferLimit);
			FlushAudioOutput();
		}
	}
	return uSamples;
}

#ifdef USE_FP_MATH

UINT CAudio::Sine(const UINT uHalfCycleCount)
{
	m_dPhase=0.0;
	UINT uSamples=0;
	UINT uHalfCycle=0;
	do {
		const double dEndPhase=(uHalfCycle+1==uHalfCycleCount)?dPi:dPi2;
		do {
			if(m_uSampleBytes==1) {
				int i=(int)(sin(m_dPhase)*120);
				m_pbyWaveData[m_uAudioBuf++]=128+i;
				if(m_uChannels==2) {
					i=(int)(sin(m_dPhase+dPi)*120);
					m_pbyWaveData[m_uAudioBuf++]=128+i;
				}
			} else {
				*((short int*)(&m_pbyWaveData[m_uAudioBuf]))=(int)(sin(m_dPhase)*32000);
				m_uAudioBuf+=2;
				if(m_uChannels==2) {
					*((short int*)(&m_pbyWaveData[m_uAudioBuf]))=(int)(sin(m_dPhase+dPi)*32000);
					m_uAudioBuf+=2;
				}
			}
			++uSamples;
			if(m_uAudioBuf>=m_uAudioBufferLimit) {
				ASSERT(m_uAudioBuf==m_uAudioBufferLimit);
				FlushAudioOutput();
			}
			m_dPhase+=m_dPhaseIncrement;
		} while(m_dPhase < dEndPhase);
		m_dPhase-=dPi2;
		uHalfCycle+=2;
	} while(uHalfCycle < uHalfCycleCount);
	return uSamples;
}

#else

UINT CAudio::Sine(const UINT uHalfCycleCount)
{
	static const BYTE bySin[512]={
		128,	130,	133,	136,	139,	142,	145,	148,
		151,	154,	157,	160,	162,	165,	168,	171,
		173,	176,	179,	181,	184,	187,	189,	192,
		194,	197,	199,	201,	204,	206,	208,	210,
		212,	214,	216,	218,	220,	222,	224,	226,
		227,	229,	230,	232,	233,	235,	236,	237,
		238,	239,	240,	241,	242,	243,	244,	245,
		245,	246,	246,	247,	247,	247,	247,	247,
		247,	247,	247,	247,	247,	247,	246,	246,
		245,	245,	244,	243,	242,	241,	240,	239,
		238,	237,	236,	235,	233,	232,	230,	229,
		227,	226,	224,	222,	220,	218,	216,	214,
		212,	210,	208,	206,	204,	201,	199,	197,
		194,	192,	189,	187,	184,	181,	179,	176,
		173,	171,	168,	165,	162,	160,	157,	154,
		151,	148,	145,	142,	139,	136,	133,	130,
		128,	126,	123,	120,	117,	114,	111,	108,
		105,	102,	99,		96,		94,		91,		88,		85,
		83,		80,		77,		75,		72,		69,		67,		64,
		62,		59,		57,		55,		52,		50,		48,		46,
		44,		42,		40,		38,		36,		34,		32,		30,
		29,		27,		26,		24,		23,		21,		20,		19,
		18,		17,		16,		15,		14,		13,		12,		11,
		11,		10,		10,		9,		9,		9,		9,		9,
		9,		9,		9,		9,		9,		9,		10,		10,
		11,		11,		12,		13,		14,		15,		16,		17,
		18,		19,		20,		21,		23,		24,		26,		27,
		29,		30,		32,		34,		36,		38,		40,		42,
		44,		46,		48,		50,		52,		55,		57,		59,
		62,		64,		67,		69,		72,		75,		77,		80,
		83,		85,		88,		91,		94,		96,		99,		102,
		105,	108,	111,	114,	117,	120,	123,	126,
		128,	130,	133,	136,	139,	142,	145,	148,
		151,	154,	157,	160,	162,	165,	168,	171,
		173,	176,	179,	181,	184,	187,	189,	192,
		194,	197,	199,	201,	204,	206,	208,	210,
		212,	214,	216,	218,	220,	222,	224,	226,
		227,	229,	230,	232,	233,	235,	236,	237,
		238,	239,	240,	241,	242,	243,	244,	245,
		245,	246,	246,	247,	247,	247,	247,	247,
		247,	247,	247,	247,	247,	247,	246,	246,
		245,	245,	244,	243,	242,	241,	240,	239,
		238,	237,	236,	235,	233,	232,	230,	229,
		227,	226,	224,	222,	220,	218,	216,	214,
		212,	210,	208,	206,	204,	201,	199,	197,
		194,	192,	189,	187,	184,	181,	179,	176,
		173,	171,	168,	165,	162,	160,	157,	154,
		151,	148,	145,	142,	139,	136,	133,	130,
		128,	126,	123,	120,	117,	114,	111,	108,
		105,	102,	99,		96,		94,		91,		88,		85,
		83,		80,		77,		75,		72,		69,		67,		64,
		62,		59,		57,		55,		52,		50,		48,		46,
		44,		42,		40,		38,		36,		34,		32,		30,
		29,		27,		26,		24,		23,		21,		20,		19,
		18,		17,		16,		15,		14,		13,		12,		11,
		11,		10,		10,		9,		9,		9,		9,		9,
		9,		9,		9,		9,		9,		9,		10,		10,
		11,		11,		12,		13,		14,		15,		16,		17,
		18,		19,		20,		21,		23,		24,		26,		27,
		29,		30,		32,		34,		36,		38,		40,		42,
		44,		46,		48,		50,		52,		55,		57,		59,
		62,		64,		67,		69,		72,		75,		77,		80,
		83,		85,		88,		91,		94,		96,		99,		102,
		105,	108,	111,	114,	117,	120,	123,	126
	};
	static const short int iSin[512]={
		0,	785,	1570,	2354,	3136,	3917,	4695,	5470,
		6242,	7011,	7775,	8534,	9289,	10037,	10780,	11516,
		12245,	12967,	13681,	14387,	15084,	15772,	16451,	17119,
		17778,	18425,	19062,	19687,	20300,	20901,	21489,	22065,
		22627,	23175,	23710,	24230,	24736,	25227,	25702,	26162,
		26607,	27035,	27447,	27842,	28221,	28583,	28927,	29254,
		29564,	29855,	30129,	30384,	30622,	30840,	31041,	31222,
		31385,	31528,	31653,	31759,	31845,	31913,	31961,	31990,
		31999,	31990,	31961,	31913,	31845,	31759,	31653,	31528,
		31385,	31222,	31040,	30840,	30622,	30384,	30129,	29855,
		29564,	29254,	28927,	28583,	28221,	27842,	27447,	27035,
		26607,	26162,	25702,	25227,	24736,	24230,	23710,	23175,
		22627,	22065,	21489,	20901,	20300,	19687,	19062,	18425,
		17778,	17119,	16451,	15772,	15084,	14387,	13681,	12967,
		12245,	11516,	10780,	10037,	9289,	8534,	7775,	7011,
		6242,	5470,	4695,	3917,	3136,	2354,	1570,	785,
		0,		-785,	-1570,	-2354,	-3136,	-3917,	-4695,	-5470,
		-6242,	-7011,	-7775,	-8534,	-9289,	-10037,	-10780,	-11516,
		-12245,	-12967,	-13681,	-14387,	-15084,	-15772,	-16451,	-17119,
		-17778,	-18425,	-19062,	-19687,	-20300,	-20901,	-21489,	-22065,
		-22627,	-23175,	-23710,	-24230,	-24736,	-25227,	-25702,	-26162,
		-26607,	-27035,	-27447,	-27842,	-28221,	-28583,	-28927,	-29254,
		-29564,	-29855,	-30129,	-30384,	-30622,	-30840,	-31041,	-31222,
		-31385,	-31528,	-31653,	-31759,	-31845,	-31913,	-31961,	-31990,
		-31999,	-31990,	-31961,	-31913,	-31845,	-31759,	-31653,	-31528,
		-31385,	-31222,	-31040,	-30840,	-30622,	-30384,	-30129,	-29855,
		-29564,	-29254,	-28927,	-28583,	-28221,	-27842,	-27447,	-27035,
		-26607,	-26162,	-25702,	-25227,	-24736,	-24230,	-23710,	-23175,
		-22627,	-22065,	-21489,	-20901,	-20300,	-19687,	-19062,	-18425,
		-17778,	-17119,	-16451,	-15772,	-15084,	-14387,	-13681,	-12967,
		-12245,	-11516,	-10780,	-10037,	-9289,	-8534,	-7775,	-7011,
		-6242,	-5470,	-4695,	-3917,	-3136,	-2354,	-1570,	-785,
		0,		785,	1570,	2354,	3136,	3917,	4695,	5470,
		6242,	7011,	7775,	8534,	9289,	10037,	10780,	11516,
		12245,	12967,	13681,	14387,	15084,	15772,	16451,	17119,
		17778,	18425,	19062,	19687,	20300,	20901,	21489,	22065,
		22627,	23175,	23710,	24230,	24736,	25227,	25702,	26162,
		26607,	27035,	27447,	27842,	28221,	28583,	28927,	29254,
		29564,	29855,	30129,	30384,	30622,	30840,	31041,	31222,
		31385,	31528,	31653,	31759,	31845,	31913,	31961,	31990,
		31999,	31990,	31961,	31913,	31845,	31759,	31653,	31528,
		31385,	31222,	31040,	30840,	30622,	30384,	30129,	29855,
		29564,	29254,	28927,	28583,	28221,	27842,	27447,	27035,
		26607,	26162,	25702,	25227,	24736,	24230,	23710,	23175,
		22627,	22065,	21489,	20901,	20300,	19687,	19062,	18425,
		17778,	17119,	16451,	15772,	15084,	14387,	13681,	12967,
		12245,	11516,	10780,	10037,	9289,	8534,	7775,	7011,
		6242,	5470,	4695,	3917,	3136,	2354,	1570,	785,
		0,		-785,	-1570,	-2354,	-3136,	-3917,	-4695,	-5470,
		-6242,	-7011,	-7775,	-8534,	-9289,	-10037,	-10780,	-11516,
		-12245,	-12967,	-13681,	-14387,	-15084,	-15772,	-16451,	-17119,
		-17778,	-18425,	-19062,	-19687,	-20300,	-20901,	-21489,	-22065,
		-22627,	-23175,	-23710,	-24230,	-24736,	-25227,	-25702,	-26162,
		-26607,	-27035,	-27447,	-27842,	-28221,	-28583,	-28927,	-29254,
		-29564,	-29855,	-30129,	-30384,	-30622,	-30840,	-31041,	-31222,
		-31385,	-31528,	-31653,	-31759,	-31845,	-31913,	-31961,	-31990,
		-31999,	-31990,	-31961,	-31913,	-31845,	-31759,	-31653,	-31528,
		-31385,	-31222,	-31040,	-30840,	-30622,	-30384,	-30129,	-29855,
		-29564,	-29254,	-28927,	-28583,	-28221,	-27842,	-27447,	-27035,
		-26607,	-26162,	-25702,	-25227,	-24736,	-24230,	-23710,	-23175,
		-22627,	-22065,	-21489,	-20901,	-20300,	-19687,	-19062,	-18425,
		-17778,	-17119,	-16451,	-15772,	-15084,	-14387,	-13681,	-12967,
		-12245,	-11516,	-10780,	-10037,	-9289,	-8534,	-7775,	-7011,
		-6242,	-5470,	-4695,	-3917,	-3136,	-2354,	-1570,	-785
	};

	m_uPhase=0;
	UINT uSamples=0;
	UINT uHalfCycle=0;
	do {
		const UINT uEndPhase=(uHalfCycle+1==uHalfCycleCount)?m_uPhaseHalf:m_uPhaseTotal;
		do {
			if(m_uSampleBytes==1) {
				BYTE by=bySin[m_uPhase>>20];
				m_pbyWaveData[m_uAudioBuf++]=by;
				if(m_uChannels==2) {
					by=bySin[(m_uPhase+m_uPhaseHalf)>>20];
					m_pbyWaveData[m_uAudioBuf++]=by;
				}
			} else {
				short int i=iSin[m_uPhase>>20];
				*((short int*)(&m_pbyWaveData[m_uAudioBuf]))=i;
				m_uAudioBuf+=2;
				if(m_uChannels==2) {
					i=iSin[(m_uPhase+m_uPhaseHalf)>>20];
					*((short int*)(&m_pbyWaveData[m_uAudioBuf]))=i;
					m_uAudioBuf+=2;
				}
			}
			++uSamples;
			if(m_uAudioBuf>=m_uAudioBufferLimit) {
				ASSERT(m_uAudioBuf==m_uAudioBufferLimit);
				FlushAudioOutput();
			}
			m_uPhase+=m_uPhaseIncrement;
		} while(m_uPhase < uEndPhase);
		m_uPhase-=uEndPhase;
		uHalfCycle+=2;
	} while(uHalfCycle < uHalfCycleCount);
	return uSamples;
}

#endif	// USE_FP_MATH


void CAudio::Pulse(int iTime)
{
	if(iTime>0) {
		Sine(static_cast<UINT>(static_cast<__int64>(iTime)*m_uCarrierFrequency/1000000));
	} else if(iTime<0) {
		Silence(static_cast<UINT>(static_cast<__int64>(-iTime)*m_uSampleRate/1000000));
	}
}


#if 0
#include <math.h>

void MakeSineTable(void)
{
	const UINT table_size=256;

	const double dPi=3.1415927;
	const double dPi2=dPi*2;

	UINT u=0;
	while(u < table_size) {
		// -- 8 bit unsigned
		//const int x=0x80+(int)(sin(dPi2*(double)u/table_size)*120);
		// -- 16 bit unsigned
		//const int x=0x8000+(int)(sin(dPi2*(double)u/table_size)*32000);
		// -- 16 bit signed
		const int x=(int)(sin(dPi2*(double)u/table_size)*32000);
		//printf("%s%i,",(u%8)?"\t":"\n\t\t",x);
		TRACE2("%s%i,",(u%8)?"\t":"\n\t\t",x);
		++u;
	}
}
#endif
