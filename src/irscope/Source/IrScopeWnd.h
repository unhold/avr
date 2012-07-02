// IrScopeWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIrScopeWnd window

class CIrScopeWnd : public CWnd
{
// Construction
public:
	CIrScopeWnd();

// Attributes
public:
private:
	static CString m_sWndClass;
	static CFont m_fontSmall;
	UINT m_uTimesCnt;
	int* m_piTimes;
	short int* m_counts;
	UINT m_uTotalTime;
	CString m_TitleText;

	CSize m_SmallFontSize;
	CSize m_LargeFontSize;
	int m_iTraceHeight;
	int m_iLineHeight;
	int m_iTraceSpacing;
	int m_iTextMargin;
	int m_iLeftTraceMargin;
	int m_iRightTraceMargin;
	UINT m_uLineTime;

	UINT m_uPixelTime;
	BOOL m_bShowElapsedTime;
	BOOL m_bShowPulseTimes;
	BOOL m_bShowPulseCounts;

// Operations
public:
	static int RegisterWndClass(void);
	static int CloseAll(void);
	int SetData(UINT uTimesCnt, int* piTimes,short int* counts=NULL);
	int CopyData(UINT uTimesCnt, int* piTimes,short int* counts=NULL);
	void SetText(CString& s) { m_TitleText=s; Invalidate(); };
	int Create(HWND hwndParent);
private:
	void Layout(int cx, int cy);
	void CalcTotalTime(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIrScopeWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIrScopeWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CIrScopeWnd)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
