// IrScopeConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIrScopeConfigDlg dialog

class CIrScopeConfigDlg : public CDialog
{
// Construction
public:
	CIrScopeConfigDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIrScopeConfigDlg)
	enum { IDD = IDD_IR_SCOPE_CONFIG };
	BOOL	m_bShowPulseTimes;
	UINT	m_uPixelTime;
	BOOL	m_bShowElapsedTime;
	BOOL	m_bShowPulseCounts;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIrScopeConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIrScopeConfigDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
