
int OpenPort(CString port, UINT mode);
void ClosePort(void);
UINT ReadPort(UINT len, BYTE* data, DWORD duration);
int ProcessPulseData(UINT len, BYTE* data, int& count, int*& times, short int*& counts, UINT interval, UINT& freq);
int ProcessTimeData(UINT len, BYTE* data, int& count, int*& times);
UINT WriteFile(CString file_name, int count, int* times, short int* counts, UINT freq=UINT_MAX);
