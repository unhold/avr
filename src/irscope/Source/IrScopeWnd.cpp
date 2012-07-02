// IrScopeWnd.cpp : implementation file
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
#include "IrScopeWnd.h"
#include "IrScopeConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIrScopeWnd

BEGIN_MESSAGE_MAP(CIrScopeWnd, CWnd)
	//{{AFX_MSG_MAP(CIrScopeWnd)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CString CIrScopeWnd::m_sWndClass;
CFont CIrScopeWnd::m_fontSmall;

CIrScopeWnd::CIrScopeWnd()
{
	m_uTimesCnt=0;
	m_piTimes=NULL;
	m_counts=NULL;
	m_uTotalTime=0;

	m_SmallFontSize.cy=0;
	m_LargeFontSize.cy=0;

	m_uPixelTime=20;
	m_bShowElapsedTime=TRUE;
	m_bShowPulseTimes=TRUE;
	m_bShowPulseCounts=TRUE;
}

CIrScopeWnd::~CIrScopeWnd()
{
	delete[] m_piTimes;
	delete[] m_counts;
}

int CIrScopeWnd::RegisterWndClass(void)
{
	if(!m_sWndClass.IsEmpty()) return 1;

	m_sWndClass=AfxRegisterWndClass(
		CS_HREDRAW | CS_DBLCLKS | CS_GLOBALCLASS,
		LoadCursor(NULL,IDC_ARROW),
		NULL,
		::LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_IRSCOPE)));

	m_fontSmall.CreatePointFont(90,"Courier New");

	return 0;
}

int CIrScopeWnd::CloseAll(void)
{
	ASSERT(!m_sWndClass.IsEmpty());
	HWND hwnd;
	int i=0;
	while(hwnd=::FindWindowEx(::GetDesktopWindow(),NULL,m_sWndClass,NULL)) {
		::DestroyWindow(hwnd);
		++i;
	}
	return i;
}

/////////////////////////////////////////////////////////////////////////////
// CIrScopeWnd message handlers

void CIrScopeWnd::OnPaint() 
{
	if(IsIconic()) return;

	CPaintDC dc(this); // device context for painting
	
	CRect rectClip,rectClient;
	dc.GetClipBox(&rectClip);
	GetClientRect(&rectClient);

	dc.FillSolidRect(&rectClip,RGB(255,255,255));

	CFont* fontOld=dc.SelectObject(CFont::FromHandle((HFONT)GetStockObject(SYSTEM_FONT)));
	if(m_SmallFontSize.cy==0 || m_LargeFontSize.cy==0) {
		m_LargeFontSize=dc.GetTextExtent("0",1);
		dc.SelectObject(&m_fontSmall);
		m_SmallFontSize=dc.GetTextExtent("0",1);
		dc.SelectObject(fontOld);
	}

	Layout(rectClient.Width(),rectClient.Height());

	int iY=4;

	dc.SetTextColor(0);
	dc.SetTextAlign(TA_TOP | TA_LEFT | TA_UPDATECP);
	dc.MoveTo(4,iY);
	if(m_TitleText.GetLength()) dc.TextOut(0,0,m_TitleText);

	const int summary_left_margin=dc.GetCurrentPosition().x+m_LargeFontSize.cx;
	const int summary_right_margin=rectClient.right-1-4;
	const int summary_top_margin=iY;
	const int summary_bottom_margin=iY+m_LargeFontSize.cy-4;
	iY+=(m_LargeFontSize.cy+7);

	const int summary_width=summary_right_margin - summary_left_margin;
	if(summary_width >= 128 && (summary_bottom_margin-summary_top_margin)>3) {
		UINT u=0;
		int total_time=0;
		UINT n=m_uTimesCnt;
		while(n-- && total_time<320000) total_time+=abs(m_piTimes[u++]);

		if(total_time > summary_width) {
			int div=total_time/summary_width;
			//TRACE1("%i uS/pixel\n",div);
			int t=0;
			n=u;
			u=0;
			BOOL skip=FALSE;
			int x=summary_left_margin;
			while(n--) {
				int tt=m_piTimes[u++];
				if(!skip) {
					if(tt>0) {
						dc.MoveTo(x,summary_bottom_margin);
						dc.LineTo(x,summary_top_margin);
					} else {
						dc.MoveTo(x,summary_top_margin);
						dc.LineTo(x,summary_bottom_margin);
					}
				}
				t+=abs(tt);
				const int xx=summary_left_margin+(t/div);
				const int y=tt>0?summary_top_margin:summary_bottom_margin;
				if(xx!=x) {
					if(skip) dc.MoveTo(x,y);
					dc.LineTo(xx,y);
					x=xx;
					skip=FALSE;
				} else {
					if(!skip) dc.SetPixel(x,y,0);
					skip=TRUE;
					//TRACE0("Skip!\n");
				}
			}
		}
	}

	int iYHigh=iY;
	int iYLow=iYHigh+m_iTraceHeight;

	UINT uXTime=0;
	UINT u=0;
	UINT uStartTime=m_uLineTime*GetScrollPos(SB_VERT);
	BOOL bHigh=FALSE;
	while((uXTime < uStartTime) &&  (u < m_uTimesCnt)) {
		uXTime+=abs(m_piTimes[u]);
		bHigh=m_piTimes[u]>0;
		++u;
	}
	uXTime-=uStartTime;

	CString s;
	if(m_bShowElapsedTime) {
		s.Format("%i",uStartTime);
		dc.SetTextAlign(TA_RIGHT | TA_TOP);
		dc.TextOut(m_iTextMargin,iYHigh,s);
	}

	int iX=m_iLeftTraceMargin;
	dc.MoveTo(iX,bHigh?iYHigh:iYLow);

	do {
		iX=m_iLeftTraceMargin + uXTime/m_uPixelTime;

		while(iX > m_iRightTraceMargin) {
			dc.LineTo(m_iRightTraceMargin,bHigh?iYHigh:iYLow);
			iYHigh+=m_iTraceSpacing;
			iYLow+=m_iTraceSpacing;
			if(iYHigh>rectClip.bottom) break;
			uStartTime+=m_uLineTime;
			if(m_bShowElapsedTime) {
				s.Format("%i",uStartTime);
				dc.SelectObject(fontOld);
				dc.SetTextAlign(TA_RIGHT | TA_TOP);
				dc.TextOut(m_iTextMargin,iYHigh,s);
			}
			dc.MoveTo(m_iLeftTraceMargin,bHigh?iYHigh:iYLow);
			uXTime-=m_uLineTime;
			iX=m_iLeftTraceMargin + uXTime/m_uPixelTime;
		}

		if(iYHigh>rectClip.bottom) break;

		dc.LineTo(iX,bHigh?iYHigh:iYLow);

		if(u >= m_uTimesCnt) break;

		const int uTime=abs(m_piTimes[u]);
		bHigh=m_piTimes[u]>0;
		uXTime+=uTime;
		if(m_bShowPulseTimes) {
			s.Format("%i",uTime);
			dc.SelectObject(&m_fontSmall);
			dc.SetTextAlign(TA_LEFT | TA_TOP);
			dc.TextOut(iX,iYLow+1,s);
		}
		if(m_bShowPulseCounts && m_counts!=NULL && m_counts[u]!=0) {
			s.Format("%i",m_counts[u]);
			dc.SelectObject(&m_fontSmall);
			dc.SetTextAlign(TA_LEFT | TA_TOP);
			dc.TextOut(iX+3,iYHigh+1,s);
		}

		if(bHigh) {
			dc.MoveTo(iX,iYLow);
			dc.LineTo(iX,iYHigh);
		} else {
			dc.MoveTo(iX,iYHigh);
			dc.LineTo(iX,iYLow);
		}

		++u;
	} while(TRUE);

	dc.SelectObject(fontOld);
	
	// Do not call CWnd::OnPaint() for painting messages
}

void CIrScopeWnd::OnDestroy() 
{
	CWnd::OnDestroy();
	
	delete this;
}

int CIrScopeWnd::Create(HWND hwndParent)
{
	ASSERT(!m_sWndClass.IsEmpty());
	ASSERT(!IsWindow(m_hWnd));
	return CreateEx(0,
		m_sWndClass,
		"IR Scope",
		WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_VSCROLL,
		20,50,720,480,
		//10,100,1400,1000,
		hwndParent,
		0);
}

int CIrScopeWnd::SetData(UINT uTimesCnt, int* piTimes, short int* counts)
{
	m_uTimesCnt=uTimesCnt;
	m_piTimes=piTimes;
	m_counts=counts;
	CalcTotalTime();
	Invalidate();
	return 0;
}

int CIrScopeWnd::CopyData(UINT uTimesCnt, int* piTimes, short int* counts)
{
	m_uTimesCnt=uTimesCnt;

	m_piTimes=new int[m_uTimesCnt];
	memcpy(m_piTimes,piTimes,m_uTimesCnt*sizeof(int));

	if(counts) {
		m_counts=new short int[m_uTimesCnt];
		memcpy(m_counts,counts,m_uTimesCnt*sizeof(short int));
	} else {
		m_counts=NULL;
	}

	CalcTotalTime();

	Invalidate();

	return 0;
}

void CIrScopeWnd::CalcTotalTime(void)
{
	m_uTotalTime=0;
	for(UINT u=0; u<m_uTimesCnt; ++u) m_uTotalTime+=abs(m_piTimes[u]);
}

void CIrScopeWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	//TRACE0("Double Click!\n");

	CIrScopeConfigDlg dlg(this);

	dlg.m_uPixelTime=m_uPixelTime;
	dlg.m_bShowElapsedTime=m_bShowElapsedTime;
	dlg.m_bShowPulseTimes=m_bShowPulseTimes;
	dlg.m_bShowPulseCounts=m_bShowPulseCounts;
	if(dlg.DoModal()==IDOK) {
		m_uPixelTime=dlg.m_uPixelTime;
		m_bShowElapsedTime=dlg.m_bShowElapsedTime;
		m_bShowPulseTimes=dlg.m_bShowPulseTimes;
		m_bShowPulseCounts=dlg.m_bShowPulseCounts;
		Invalidate();
	}
}


void CIrScopeWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	//Layout(cx,cy);
}

void CIrScopeWnd::Layout(int cx, int cy)
{
	m_iTraceHeight=((m_bShowPulseCounts && m_counts!=NULL)?m_SmallFontSize.cy:10);
	m_iLineHeight=m_iTraceHeight+(m_bShowPulseTimes?m_SmallFontSize.cy:0)+5;
	m_iTraceSpacing=__max(m_iLineHeight,m_LargeFontSize.cy);
	m_iTextMargin=/*rectClient.left+*/(m_bShowElapsedTime?m_LargeFontSize.cx*9:0);
	m_iLeftTraceMargin=m_iTextMargin+4;
	//m_iRightTraceMargin=rectClient.right-1-4;
	m_iRightTraceMargin=cx-4;
	m_uLineTime=(m_iRightTraceMargin-m_iLeftTraceMargin+1)*m_uPixelTime;



	SCROLLINFO si;
	si.cbSize=sizeof(si);
	si.fMask=SIF_RANGE;
	si.nMin=0;
	si.nMax=(m_uTotalTime+m_uLineTime-1)/m_uLineTime-1;
	//si.nMax=10;;
	//si.nPage=1;
	//si.nPos=1;
	SetScrollInfo(SB_VERT,&si);
}

void CIrScopeWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	ASSERT(pScrollBar==NULL);

	int pos=GetScrollPos(SB_VERT);

	switch(nSBCode) {
		case SB_TOP:			// Scroll to top
			break;
		case SB_BOTTOM:			// Scroll to bottom
			break;
		case SB_LINEDOWN:		// Scroll one line down
			++pos;
			break;
		case SB_LINEUP:			// Scroll one line up
			--pos;
			break;
		case SB_PAGEDOWN:		// Scroll one page down
			pos+=3;
			break;
		case SB_PAGEUP:			// Scroll one page up
			pos-=3;
			break;
		case SB_THUMBPOSITION:	// Scroll to the absolute position. The current position is provided in nPos
			break;
		case SB_THUMBTRACK:		// Drag scroll box to specified position. The current position is provided in nPos
			pos=static_cast<int>(nPos);
			break;
		case SB_ENDSCROLL:		// End scroll
			break;
	}
	
	if(pos<0) pos=0;

	SetScrollPos(SB_VERT,pos);

	Invalidate();

	//CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}
