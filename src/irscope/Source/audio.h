
#include "mmsystem.h"
#include "wavfile.h"

class CAudio
{
private:
	UINT m_uChecksum;

	UINT m_uSampleRate;
	UINT m_uSampleBytes;
	UINT m_uChannels;

#ifdef USE_FP_MATH
	double m_dPhase;
	double m_dPhaseIncrement;
#else
	UINT m_uPhase;
	UINT m_uPhaseIncrement;
	UINT m_uPhaseTotal;
	UINT m_uPhaseHalf;
#endif

	UINT m_uCarrierFrequency;
	UINT m_uMaxCarrierFrequency;

	BOOL m_bFile;
	CFile m_file;

	BYTE* m_pbyAudioBuf;
	BYTE* m_pbyWaveData;
	UINT m_uAudioBuf;
	UINT m_uAudioLen;
	UINT m_uAudioBufferLimit;

	RIFF_HDR		riff_hdr;
	RIFF_HDR		fmt_chunk_hdr;
	WAVE_FMT_CHUNK	fmt_chunk;
	WAVE_FMT_MSPCM	fmt_mspcm;
	RIFF_HDR		data_chunk_hdr;

	HWAVEOUT		m_hWaveOut;
	WAVEFORMATEX	m_wf;
	WAVEHDR			m_hdr[2];
	WAVEHDR*		m_pHdr;
	BOOL			m_bPending[2];
	BOOL*			m_pbPending;

	void RiffHeader(void);
	void FlushAudioOutput(void);

public:
	CAudio();
	~CAudio();
	int Open(LPCSTR sFile=NULL);
	void Close(void);

	void SetSampleRate(UINT uSampleRate, BOOL b16Bit=TRUE, UINT uChannels=2);
	UINT GetSampleRate(void) { return m_uSampleRate; };
	UINT GetMaxCarrierFrequency(void) { return m_uMaxCarrierFrequency; };
	int SetCarrierFrequency(UINT freq);

	UINT Silence(const UINT uSamples);
	UINT Sine(const UINT uHalfCycleCount);
	void Pulse(int iTime);
};
