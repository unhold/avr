// IrScopeConfigDlg.cpp : implementation file
//

/*
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
#include "IRScope.h"
#include "IrScopeConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIrScopeConfigDlg dialog


CIrScopeConfigDlg::CIrScopeConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIrScopeConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIrScopeConfigDlg)
	m_bShowPulseTimes = FALSE;
	m_uPixelTime = 0;
	m_bShowElapsedTime = FALSE;
	m_bShowPulseCounts = FALSE;
	//}}AFX_DATA_INIT
}


void CIrScopeConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIrScopeConfigDlg)
	DDX_Check(pDX, IDC_PULSE_TIMES, m_bShowPulseTimes);
	DDX_Text(pDX, IDC_PIXEL_TIME, m_uPixelTime);
	DDV_MinMaxUInt(pDX, m_uPixelTime, 1, 2000);
	DDX_Check(pDX, IDC_ELAPSED_TIME, m_bShowElapsedTime);
	DDX_Check(pDX, IDC_PULSE_COUNTS, m_bShowPulseCounts);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIrScopeConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CIrScopeConfigDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIrScopeConfigDlg message handlers
