; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CIRScopeDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "irscope.h"
LastPage=0

ClassCount=5
Class1=CIRScopeApp
Class2=CIrScopeConfigDlg
Class3=CAboutDlg
Class4=CIRScopeDlg
Class5=CIrScopeWnd

ResourceCount=3
Resource1=IDD_IRSCOPE_DIALOG
Resource2=IDD_IR_SCOPE_CONFIG
Resource3=IDD_ABOUTBOX

[CLS:CIRScopeApp]
Type=0
BaseClass=CWinApp
HeaderFile=IRScope.h
ImplementationFile=IRScope.cpp

[CLS:CIrScopeConfigDlg]
Type=0
BaseClass=CDialog
HeaderFile=IRScopeConfigDlg.h
ImplementationFile=IRScopeConfigDlg.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=IRScopeDlg.cpp
ImplementationFile=IRScopeDlg.cpp
LastObject=CAboutDlg

[CLS:CIRScopeDlg]
Type=0
BaseClass=CDialog
HeaderFile=IRScopeDlg.h
ImplementationFile=IRScopeDlg.cpp
Filter=D
VirtualFilter=dWC
LastObject=CIRScopeDlg

[CLS:CIrScopeWnd]
Type=0
BaseClass=CWnd
HeaderFile=IrScopeWnd.h
ImplementationFile=IrScopeWnd.cpp

[DLG:IDD_IR_SCOPE_CONFIG]
Type=1
Class=CIrScopeConfigDlg
ControlCount=7
Control1=IDC_PIXEL_TIME,edit,1350631552
Control2=IDC_STATIC,static,1342308352
Control3=IDC_ELAPSED_TIME,button,1342242819
Control4=IDC_PULSE_TIMES,button,1342242819
Control5=IDC_PULSE_COUNTS,button,1342242819
Control6=IDOK,button,1342242817
Control7=IDCANCEL,button,1342242816

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=5
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_LICENSE,edit,1344342148

[DLG:IDD_IRSCOPE_DIALOG]
Type=1
Class=CIRScopeDlg
ControlCount=19
Control1=IDC_STATIC,static,1342308352
Control2=IDC_PORT,combobox,1344339970
Control3=IDC_STATIC,static,1342308352
Control4=IDC_DURATION,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_HARDWARE,combobox,1344339971
Control8=IDC_STATIC,button,1342177287
Control9=IDC_SAVE_ICT,button,1342242819
Control10=IDC_SAVE_WAV,button,1342242819
Control11=IDC_SAVE_TVBG,button,1342242819
Control12=IDC_ASK_NAME,button,1342242819
Control13=IDC_VIEW,button,1342242816
Control14=IDC_CAPTURE,button,1342242817
Control15=IDC_DECODE,SysListView32,1350631429
Control16=IDC_CLEAR_DECODE,button,1342242816
Control17=IDC_CLEAR_BEFORE,button,1342242819
Control18=IDC_SHOW_WAVE,button,1342242819
Control19=IDC_DECODE_REV,static,1342308352

