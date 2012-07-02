// IRScopeDlg.cpp : implementation file
//

/*
    Copyright (C) 2007,2008  Kevin Timmerman

	irwidget [@t] compendiumarcana [d0t] com

	http://www.compendiumarcana.com/irwidget

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#include "stdafx.h"
#include "IRScope.h"
#include "IRScopeDlg.h"
#include "IRScopeWnd.h"
#include "widget.h"
#include "audio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	HANDLE h=FindResource(NULL,MAKEINTRESOURCE(IDR_LICENSE),"LICENSE");
	if(h) h=LoadResource(NULL,FindResource(NULL,MAKEINTRESOURCE(IDR_LICENSE),"LICENSE"));
	if(h) {
		LPCSTR s=static_cast<LPCSTR>(LockResource(h));
		if(s) GetDlgItem(IDC_LICENSE)->SetWindowText(s);
		FreeResource(h);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIRScopeDlg dialog

CIRScopeDlg::CIRScopeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIRScopeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIRScopeDlg)
	m_Duration = 700;
	m_Port = _T("COM5");
	m_Hardware = 0;
	m_SaveICT = TRUE;
	m_SaveTVBG = FALSE;
	m_SaveWav = FALSE;
	m_AskName = FALSE;
	m_ClearBeforeCapture = FALSE;
	m_ShowWaveform = TRUE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIRScopeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIRScopeDlg)
	DDX_Control(pDX, IDC_DECODE, m_Decode);
	DDX_Text(pDX, IDC_DURATION, m_Duration);
	DDV_MinMaxUInt(pDX, m_Duration, 500, 10000);
	DDX_CBString(pDX, IDC_PORT, m_Port);
	DDX_CBIndex(pDX, IDC_HARDWARE, m_Hardware);
	DDX_Check(pDX, IDC_SAVE_ICT, m_SaveICT);
	DDX_Check(pDX, IDC_SAVE_TVBG, m_SaveTVBG);
	DDX_Check(pDX, IDC_SAVE_WAV, m_SaveWav);
	DDX_Check(pDX, IDC_ASK_NAME, m_AskName);
	DDX_Check(pDX, IDC_CLEAR_BEFORE, m_ClearBeforeCapture);
	DDX_Check(pDX, IDC_SHOW_WAVE, m_ShowWaveform);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIRScopeDlg, CDialog)
	//{{AFX_MSG_MAP(CIRScopeDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CAPTURE, OnCapture)
	ON_BN_CLICKED(IDC_VIEW, OnView)
	ON_BN_CLICKED(IDC_CLEAR_DECODE, OnClearDecode)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIRScopeDlg message handlers

BOOL CIRScopeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	CString strAboutMenu;
	strAboutMenu.LoadString(IDS_ABOUTBOX);
	if (!strAboutMenu.IsEmpty())
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// Add extra initialization here
	m_Decode.InsertColumn(0,"Frequency",LVCFMT_LEFT,64);
	m_Decode.InsertColumn(1,"Protocol",LVCFMT_LEFT,64);
	m_Decode.InsertColumn(2,"Device",LVCFMT_LEFT,64);
	m_Decode.InsertColumn(3,"Key",LVCFMT_LEFT,48);
	m_Decode.InsertColumn(4,"Hex",LVCFMT_LEFT,80);
	m_Decode.InsertColumn(5,"Misc",LVCFMT_LEFT,64);
	m_Decode.InsertColumn(6,"Error",LVCFMT_LEFT,64);
	

	m_hDecodeLib=LoadLibrary("DecodeIR.DLL");
	if(m_hDecodeLib) {
		m_GetDecodeVersion=reinterpret_cast<typeGetVersion>(GetProcAddress(m_hDecodeLib,"Version"));
		m_DecodeIR=reinterpret_cast<typeDecodeIR>(GetProcAddress(m_hDecodeLib,"DecodeIR"));
		if(m_DecodeIR) {
			char version[64];
			m_GetDecodeVersion(version);
			CString s("IR Decoder Version ");
			s+=version;
			GetDlgItem(IDC_DECODE_REV)->SetWindowText(s);
		}
	} else {
		m_GetDecodeVersion=NULL;
		m_DecodeIR=NULL;
		RECT rect,rectDecode;
		GetWindowRect(&rect);
		GetDlgItem(IDC_DECODE)->GetWindowRect(&rectDecode);
		rect.bottom=rectDecode.top;
		MoveWindow(&rect,FALSE);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CIRScopeDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	if(m_hDecodeLib) {
		FreeLibrary(m_hDecodeLib);
	}	
}

void CIRScopeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIRScopeDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIRScopeDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CIRScopeDlg::OnCapture() 
{
	if(!UpdateData())
		return;

	CWaitCursor wait;

	if(m_ClearBeforeCapture) m_Decode.DeleteAllItems();
	
	int err=OpenPort(m_Port,m_Hardware);

	CString s;
	if(err) {
		s.Format("Unable to open %s\nError %i",m_Port,err);
		MessageBox(s);
		return;
	}

	const UINT sample_byte_count = m_Duration * 1000 / 100;

	BYTE* data=new BYTE[sample_byte_count];
	if(!data) return;

	MessageBeep(MB_OK);

	const UINT read=ReadPort(sample_byte_count,data,(m_Hardware<2) ? m_Duration+400 : m_Duration);
	TRACE("%i Bytes Read\n",read);

	ClosePort();

	if((m_Hardware<2)?read!=sample_byte_count:read==0) {
		if(read)
			s.Format("Expected %i bytes, only got %i",sample_byte_count,read);
		else
			s="No data received from IR Widget\n\nMake sure the correct COM port is selected\nPress the remote button after the beep";
		MessageBox(s);
		if(read<100) {
			delete[] data;
			return;
		}
	}

	int count;
	int* times;
	short int* counts=NULL;
	UINT freq=0;
	err = (m_Hardware<3) ?
		ProcessPulseData(read,data,count,times,counts,(m_Hardware==2)?139:100,freq)
		: ProcessTimeData(read,data,count,times);
	if(err==0) {
		CString file_name;
		file_name=CTime::GetCurrentTime().Format("IR%Y%m%d%H%M%S");
		BOOL save=TRUE;
		if(m_AskName) {
			CFileDialog dlg(FALSE,NULL,file_name);
			if(dlg.DoModal()==IDOK) {
				file_name=dlg.GetPathName();
			} else {
				save=FALSE;
			}
		}
		if(save && (m_SaveICT || m_SaveTVBG || m_SaveWav)) {
			if(m_SaveICT)
				WriteFile(file_name+".ict",count,times,counts,freq);
			if(m_SaveTVBG)
				WriteTVBGFile(file_name+"tvbg.c",count,times,freq);
			if(m_SaveWav)
				WriteWavFile(file_name+".wav",count,times,freq);
		}
		if(m_ShowWaveform) {
			CIrScopeWnd* wnd=new CIrScopeWnd;
			wnd->Create(::GetDesktopWindow());
			wnd->SetData(count,times,counts);
			if(freq) {
				CString fs;
				fs.Format("%i Hz",freq);
				wnd->SetText(fs);
			}
		}
		Decode(count,times,freq);
	} else if(err==1) {
		MessageBox("No signal found");
	}

	delete[] data;
}



UINT CIRScopeDlg::WriteTVBGFile(LPCSTR filename, int count, int *times, UINT freq)
{
	if(!count) return 0;

	CFile file;
	if(file.Open(filename,CFile::modeWrite | CFile::modeCreate)) {
		CString s("const struct powercode xCode PROGMEM = {\r\n");
		file.Write(s,s.GetLength());
		s.Format("\tfreq_to_timerval(%i),\r\n\t{\r\n",freq);
		file.Write(s,s.GetLength());
		int t1,t2,frac=0;
		for(int i=0;i<count;) {
			t1 = abs(times[i++])+frac;
			frac = t1%10;
			t1 /= 10;
			if(i<count) {
				t2 = abs(times[i++])+frac;
				frac = t2%10;
				t2 /= 10;
			} else
				t2=0;
			if(i<count)
				s.Format("\t\t{ %i,\t%i },\r\n",t1,t2);
			else
				s.Format("\t\t{ %i,\t0 }\r\n",t1);
			file.Write(s,s.GetLength());
		}
		s.Format("\t}\r\n};\r\n",freq);
		file.Write(s,s.GetLength());
	} else
		return GetLastError();

	return 0;
}

int CIRScopeDlg::WriteWavFile(LPCSTR filename, int count, int *times, UINT freq)
{
	if(!count) return 0;

	CAudio a;

	int err;
	err=a.SetCarrierFrequency(freq);
	if(err) {
		AfxMessageBox("Can not create audio file\nCarrier frequency out of range");
		return err;
	}

	err=a.Open(filename);
	if(err) return err;

	for(int i=0;i<count;) {
		a.Pulse(times[i++]);
	}
	
	a.Close();

	return 0;
}


void CIRScopeDlg::Decode(int count, int *times, UINT freq)
{
	int item=m_Decode.GetItemCount()-1;

	if(0) { //m_GetDecodeVersion) {
		char version[64];
		m_GetDecodeVersion(version);
		TRACE1("Version: %s\n",version);
	}
	if(m_DecodeIR) {
		int* decode_times=new int[count];
		if(!decode_times) return;
		UINT u=0;
		while(u<static_cast<UINT>(count)) {
			decode_times[u]=abs(times[u]);
			++u;
		}

		char protocol[64];
		int context[2],device,subdevice,obc,hex[4];
		char misc[64];
		char error[64];

		memset(protocol,0,sizeof(protocol));
		context[0]=context[1]=0;
		device=subdevice=obc=-1;
		hex[0]=hex[1]=hex[2]=hex[3]=-1;
		memset(misc,0,sizeof(misc));
		memset(error,0,sizeof(error));

		do {
			m_DecodeIR(context,decode_times,freq,count/2,0,protocol,&device,&subdevice,&obc,hex,misc,error);

			if(!*protocol) break;	/// Is this the correct way to determine decoding is complete???

			CString s;
#if 0
			s.Format("Context: %i %i",context[0],context[1]);
			AfxMessageBox(s);
#endif
#if 0
			TRACE2("Context: %i %i\n",context[0],context[1]);
			TRACE2("Context: %08X %08X\n",context[0],context[1]);
			TRACE("Protocol: %s Device: %i SubDevice:%i OBC: %i\n",protocol,device,subdevice,obc);
			TRACE("Hex: %02X %02X %02X %02X\n",hex[0],hex[1],hex[2],hex[3]);
			TRACE2("Error: %s Misc: %s\n",error,misc);
			TRACE0("\n");
#endif
			s.Format("%i",freq);
			m_Decode.InsertItem(++item,s);

			m_Decode.SetItemText(item,1,*protocol?protocol:"UNKNOWN");
			if(device!=-1) {
				if(subdevice==-1)
					s.Format("%i",device);
				else
					s.Format("%i.%i",device,subdevice);
				m_Decode.SetItemText(item,2,s);
			}
			if(obc!=-1) {
				s.Format("%i",obc);
				m_Decode.SetItemText(item,3,s);
			}
			if(hex[0]!=-1) {
				s.Empty();
				CString ss;
				int x=0;
				while(x<4 && hex[x]!=-1) {
					ss.Format(x?" %02X":"%02X",hex[x]);
					s+=ss;
					++x;
				}
				m_Decode.SetItemText(item,4,s);
			}
			if(*misc) m_Decode.SetItemText(item,5,misc);
			if(*error) m_Decode.SetItemText(item,6,error);
		} while(TRUE);
	
		delete[] decode_times;
	}

	m_Decode.EnsureVisible(item,FALSE);
}


void CIRScopeDlg::OnView() 
{
	CFileDialog dlg(TRUE,"*.ict",NULL,OFN_HIDEREADONLY|OFN_PATHMUSTEXIST,
		"IR Capture Text Files (*.ict)|*.ict||");

	if(dlg.DoModal()==IDOK) {
		CString file_name=dlg.GetPathName();
		CStdioFile file;
		if(file.Open(file_name,CFile::modeRead|CFile::shareExclusive)) {
			CString s;
			if(!file.ReadString(s) || s!="irscope 0") {
				AfxMessageBox("This is not an IR Scope Capture Text file");
				return;
			}
			UINT sample_count=0;
			UINT u=0;
			int *times=NULL;
			UINT freq=0;
			short int *counts=NULL;
			while(file.ReadString(s)) {
				if(s[0]=='+' || s[0]=='-' || isdigit(s[0])) {
					if((u<sample_count) && times && counts) {
						const int comma=s.Find(',');
						if(comma==-1) {
							times[u]=atoi(s);
							counts[u]=0;
						} else {
							times[u]=atoi(s.Left(comma));
							counts[u]=atoi(s.Mid(comma+1));
						}
						++u;
					}
				} else if(s.Left(17).CompareNoCase("carrier_frequency")==0) {
					freq=atoi(s.Mid(18));
				} else if(s.Left(12).CompareNoCase("sample_count")==0) {
					sample_count=atoi(s.Mid(13));
					times=new int[sample_count];
					counts=new short int[sample_count];
				} else {
					TRACE1("wtf: %s\n",s);
				}
			}
			if(sample_count && times && counts) {
				CIrScopeWnd* wnd=new CIrScopeWnd;
				wnd->Create(::GetDesktopWindow());
				wnd->SetData(u,times,counts);
				if(freq) {
					CString fs;
					fs.Format("%i Hz",freq);
					wnd->SetText(fs);
				}
			}
		} else {
			AfxMessageBox("Could not open file");
		}
	}	
}

void CIRScopeDlg::OnClearDecode() 
{
	m_Decode.DeleteAllItems();
}
