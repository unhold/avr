// IRScopeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIRScopeDlg dialog

typedef void (_stdcall *typeGetVersion)(char *);

typedef void (_stdcall* typeDecodeIR)(int* Context, /* [2] */
				      int* bursts,
				      const int freq,
				      const int singleBurstCount,
				      const int repeatBurstCount,
				      char* protocol,	/* [64] */
				      int* device,
				      int* subDevice,
				      int* OBC,
				      int* hex,			/* [4] */
				      char* misc,		/* [64] */
				      char* error);		/* [64] */

class CIRScopeDlg : public CDialog
{
// Construction
public:
	CIRScopeDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CIRScopeDlg)
	enum { IDD = IDD_IRSCOPE_DIALOG };
	CListCtrl	m_Decode;
	UINT	m_Duration;
	CString	m_Port;
	int		m_Hardware;
	BOOL	m_SaveICT;
	BOOL	m_SaveTVBG;
	BOOL	m_SaveWav;
	BOOL	m_AskName;
	BOOL	m_ClearBeforeCapture;
	BOOL	m_ShowWaveform;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIRScopeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	HINSTANCE m_hDecodeLib;
	typeGetVersion m_GetDecodeVersion;
	typeDecodeIR m_DecodeIR;

	UINT WriteTVBGFile(LPCSTR filename, int count, int *times, UINT freq);
	int WriteWavFile(LPCSTR filename, int count, int *times, UINT freq);
	void Decode(int count, int *times, UINT freq);

	// Generated message map functions
	//{{AFX_MSG(CIRScopeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnCapture();
	afx_msg void OnView();
	afx_msg void OnClearDecode();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
