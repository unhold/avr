
#pragma pack(push,1)

struct RIFF_HDR {     
	char  id[4];  	// identifier string = "RIFF"
	DWORD len;    	// remaining length after this header
};

struct WAVE_FMT_CHUNK {
	WORD wFormatTag;         // Format category
	WORD wChannels;          // Number of channels
	DWORD dwSamplesPerSec;    // Sampling rate
	DWORD dwAvgBytesPerSec;   // For buffer estimation
	WORD wBlockAlign;        // Data block size
};

struct WAVE_FMT_MSPCM {
	WORD wBitsPerSample;	// Sample size
};

struct AU_HDR {
	int magic;               // magic number SND_MAGIC
	int dataLocation;        // offset or pointer to the data
	int dataSize;            // number of bytes of data
	int dataFormat;          // the data format code
	int samplingRate;        // the sampling rate
	int channelCount;        // the number of channels
	char info[4];            // optional text information
};


struct AIFF_HDR {
	char	id[4];			// "COMM", "SSND", etc...
	DWORD	dwLen;			// length of chunk
};

struct AIFF_COMM {
	WORD	wChannels;		// number of channels
	DWORD	dwSampleFrames;	// number of sample frames
	WORD	wSampleSize;	// number of bits per sample
	WORD	wSampleRateM;	// number of frames per second
	WORD	wSampleRate;
	DWORD	dwSampleRateZ;
	WORD	wSampleRateZ;
};

struct AIFF_SSND {
	DWORD	dwOffset;		// offset to sound data
	DWORD	dwBlockSize;	// size of alignment blocks
};

#pragma pack(pop)
