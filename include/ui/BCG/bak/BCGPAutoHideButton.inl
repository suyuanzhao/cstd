
//
// BCGPAutoHideButton.cpp: implementation of the CBCGPAutoHideButton class.
//
//////////////////////////////////////////////////////////////////////

//#include "BCGCBPro.h"
#include "BCGPAutoHideButton.h"
#include "BCGPDockingControlBar.h"
#include "BCGGlobals.h"
#include "BCGPGlobalUtils.h"
#include "BCGPVisualManager.h"
#include "BCGPDockBarRow.h"

IMPLEMENT_DYNCREATE(CBCGPAutoHideButton, CObject)

int CBCGPAutoHideButton::m_nBorderSize = 1;
int CBCGPAutoHideButton::m_nMarginSize = 2; // from border to icon
int CBCGPAutoHideButton::m_nTextMargin = 2;
int CBCGPAutoHideButton::m_nOverlappedTextMargin = 10;
int CBCGPAutoHideButton::m_nTextSizeNoIcon = 0; // additinal spacing, which can be set if there is no icon
BOOL CBCGPAutoHideButton::m_bOverlappingTabs = TRUE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPAutoHideButton::CBCGPAutoHideButton()
{
	m_bTop				= FALSE;
	m_pParentBar		= NULL;
	m_pAutoHideWindow	= NULL;
	m_dwAlignment		= 0;
	m_bVisible			= FALSE;
	m_bHighlighted		= FALSE; 
	m_rect.SetRectEmpty();
}
//****
CBCGPAutoHideButton::~CBCGPAutoHideButton()
{
}
//****//
BOOL CBCGPAutoHideButton::Create (CBCGPAutoHideToolBar* pParentBar, 
								CBCGPDockingControlBar* pAutoHideWnd, DWORD dwAlignment)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pAutoHideWnd);
	ASSERT (dwAlignment & CBRS_ALIGN_ANY);

	m_pParentBar		= pParentBar;
	m_pAutoHideWindow	= pAutoHideWnd;
	m_dwAlignment		= dwAlignment;

	pAutoHideWnd->SetAutoHideParents (pParentBar, this);

	CSize size = GetSize();
	m_rect.SetRect (0, 0, size.w, size.h);

	m_bVisible = TRUE;

	return TRUE;
}
//****//
void CBCGPAutoHideButton::Move (int nOffset)
{
	ASSERT_VALID (this);
	// nOffset in pixels
	IsHorizontal() ? m_rect.OffsetRect (nOffset, 0) : m_rect.OffsetRect (0, nOffset);
}
//****//
CSize CBCGPAutoHideButton::GetSize() const
{
	ASSERT_VALID (this);

	const BOOL bIsOverlapped = 
		(CBCGPVisualManager::GetInstance()->HasOverlappedAutoHideButtons()) || 
		CBCGPAutoHideButton::m_bOverlappingTabs;

	CSize size (m_nMarginSize + 2 * m_nBorderSize, m_nMarginSize + 2 * m_nBorderSize);
	int nTextMargin = bIsOverlapped ? m_nOverlappedTextMargin : m_nTextMargin;

	if (m_pAutoHideWindow != NULL)
	{
		BOOL bHorz = IsHorizontal();
		CBitmap* hIcon = m_pAutoHideWindow->GetBarIcon (FALSE);
		CSize sizeText = GetTextSize();

		CSize sizeIcon (0, 0);
		if (hIcon != NULL || bIsOverlapped)
		{
			sizeIcon = globalData.m_sizeSmallIcon;
		}

		if (hIcon == NULL && !bIsOverlapped)
		{
			sizeIcon = CSize (0, 0);
		}

		int nSpacing = 0;

		
		if ((hIcon != NULL || bIsOverlapped) && 
			(sizeText.w > 0 && bHorz || sizeText.h > 0 && !bHorz))
		{
			nSpacing = nTextMargin;
			nSpacing += IsHorizontal() ? sizeIcon.w : sizeIcon.h; 
		}
		else if (hIcon == NULL)
		{
			nSpacing += m_nMarginSize + m_nTextSizeNoIcon;
		}
		else 
		{
			nSpacing += m_nMarginSize + 1;
		}
		

		if (bHorz)
		{
			size.w += (bIsOverlapped ? sizeIcon.w : 0) + sizeText.w + nSpacing;
			size.h += max (sizeIcon.h, sizeText.h);
		}
		else
		{
			size.w += max (sizeIcon.w, sizeText.w);
			size.h += (bIsOverlapped ? sizeIcon.h : 0) + sizeText.h + nSpacing;
		}

		CBCGPDockBarRow* pParentRow = m_pParentBar->GetDockBarRow();
		if (pParentRow != NULL)
		{
			int nExtraSpace = pParentRow->GetExtraSpace();
			int nMaxBarSize = pParentRow->GetMaxBarSize() - nExtraSpace;

			bHorz ? size.h = max (size.h, nMaxBarSize) : 
					size.w = max (size.w, nMaxBarSize);
		}
	}

	return size;
}
//****//
void CBCGPAutoHideButton::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);

	const BOOL bIsOverlapped = 
		(CBCGPVisualManager::GetInstance()->HasOverlappedAutoHideButtons()) || m_bOverlappingTabs;
	const int nTextMargin = bIsOverlapped ? (IsHorizontal() ? 5 : 7) : 
							m_nTextMargin;

	CSize size = GetSize();
	m_rect.SetRect (0, 0, size.w, size.h);

	// calculate border size and draw the border
	CRect rectBorderSize (m_nBorderSize, 0, m_nBorderSize, m_nBorderSize);

	switch (m_dwAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_RIGHT:
		globalUtils.FlipRect (rectBorderSize, 90);
		break;
	case CBRS_ALIGN_BOTTOM:
		globalUtils.FlipRect (rectBorderSize, 180);
		break;
	case CBRS_ALIGN_LEFT:
		globalUtils.FlipRect (rectBorderSize, -90);
		break;
	}

	if (bIsOverlapped && !m_pParentBar->m_bFirstInGroup)
	{
		CRect rectPrev = m_rect;

		switch (m_dwAlignment & CBRS_ALIGN_ANY)
		{
		case CBRS_ALIGN_RIGHT:
		case CBRS_ALIGN_LEFT:
			rectPrev.OffsetRect (0, -m_rect.Height() + size.w / 2);
			break;

		case CBRS_ALIGN_TOP:
		case CBRS_ALIGN_BOTTOM:
			rectPrev.OffsetRect (-m_rect.Width() + size.h / 2, 0);
			break;
		}

		OnFillBackground (pDC, rectPrev);
		OnDrawBorder (pDC, rectPrev, rectBorderSize);
	}

	OnFillBackground (pDC, m_rect);
	OnDrawBorder (pDC, m_rect, rectBorderSize);

	if (m_pAutoHideWindow == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pAutoHideWindow);

	CRect rectDraw = m_rect;

	if (!bIsOverlapped)
	{
		rectDraw.DeflateRect (rectBorderSize);
	}

	rectDraw.DeflateRect (m_nMarginSize, m_nMarginSize);

	if (m_pAutoHideWindow != NULL)
	{
		// draw the icon (if any)
		CBitmap* hIcon = m_pAutoHideWindow->GetBarIcon (FALSE);
		if (hIcon != NULL || bIsOverlapped)
		{
			CSize sizeIcon (globalData.m_sizeSmallIcon);

			int dx = IsHorizontal() ? 0 : (rectDraw.Width() - sizeIcon.w) / 2;
			int dy = IsHorizontal() ? (rectDraw.Height() - sizeIcon.h) / 2 : 0;

			if (bIsOverlapped && !m_bOverlappingTabs)
			{
				if (IsHorizontal()	)
				{
					dx += rectDraw.Height() / 2 + 1;
				}
				else
				{
					dy += rectDraw.Width() / 2 + 1;
				}
			}

			if (hIcon != NULL)
			{
				::DrawIconEx (pDC, 
							rectDraw.l + dx, rectDraw.t + dy, hIcon,
							sizeIcon.w, sizeIcon.h, 0, NULL, DI_NORMAL);
			}
			else
			{
				#ifndef BCGP_EXCLUDE_RIBBON
					CBCGPVisualManager::GetInstance()->
						OnDrawDefaultRibbonImage (pDC, 
							CRect (CPoint (rectDraw.l + dx, rectDraw.t + dy), sizeIcon));
				#endif
			}
			if (IsHorizontal())
			{
				rectDraw.l += nTextMargin + sizeIcon.w + dx;
			}
			else
			{
				rectDraw.t += nTextMargin + sizeIcon.h + dy;
			}
		}
		else
		{
			if (IsHorizontal())
			{
				rectDraw.l += m_nMarginSize;
			}
			else
			{
				rectDraw.t += m_nMarginSize;
			}
		}

		// Draw text:
		CString strText;
		m_pAutoHideWindow->GetWindowText (strText);

		if (!strText.IsEmpty() && m_pParentBar->m_bActiveInGroup || !m_bOverlappingTabs)
		{
			int nOldMode = pDC->SetBkMode (TRANSPARENT);

			CFont* pFontOld = (CFont*) pDC->SelectObject (IsHorizontal() ? 
										&globalData.fontRegular : &globalData.fontVert);
			ASSERT(pFontOld != NULL);

			pDC->SetTextColor (
				CBCGPVisualManager::GetInstance()->GetAutoHideButtonTextColor (this));

			if (IsHorizontal())
			{
				pDC->DrawText (strText, &rectDraw, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
			}
			else
			{
				TEXTMETRIC tm;
				pDC->GetTextMetrics (&tm);

				CRect rectText = rectDraw;

				rectText.l = rectText.r - (rectDraw.Width() - tm.tmHeight + 1) / 2;
				rectText.b = rectDraw.t + nTextMargin;

				pDC->DrawText (strText, &rectText, DT_SINGLELINE | DT_NOCLIP);
			}

			pDC->SelectObject (pFontOld);
			pDC->SetBkMode (nOldMode);
		}
	}
}
//****
void CBCGPAutoHideButton::OnFillBackground (CDC* pDC, CRect rect)
{
	CBCGPVisualManager::GetInstance()->OnFillAutoHideButtonBackground (pDC, rect, this);
}
//****
void CBCGPAutoHideButton::OnDrawBorder (CDC* pDC, CRect rectBounds, CRect rectBorderSize)
{
	CBCGPVisualManager::GetInstance()->OnDrawAutoHideButtonBorder (pDC, rectBounds, rectBorderSize, this);
}
//****
CSize CBCGPAutoHideButton::GetTextSize() const
{
	CSize size (0, 0);

	if (m_pAutoHideWindow != NULL && m_pParentBar != NULL)
	{
		CString strText;
		m_pAutoHideWindow->GetWindowText (strText);	

		if (!strText.IsEmpty())
		{
			CWindowDC dc (m_pParentBar);

			CFont* pFontOld = (CFont*) dc.SelectObject (IsHorizontal() ? 
										&globalData.fontRegular : &globalData.fontVert);
			ASSERT (pFontOld != NULL);
			size = dc.GetTextExtent (strText);
			size.w += m_nMarginSize;
			size.h += m_nMarginSize;

			dc.SelectObject (pFontOld);

			if (!IsHorizontal())
			{
				int n = size.h;
				size.h = size.w;
				size.w = n;
			}

		}

		if (!m_pParentBar->m_bActiveInGroup && m_bOverlappingTabs)
		{
			IsHorizontal() ? size.w = 0 : size.h = 0;
		}	
	}
	return size;
}
//****
BOOL CBCGPAutoHideButton::IsHorizontal() const
{
	return (m_dwAlignment & CBRS_ALIGN_TOP || m_dwAlignment & CBRS_ALIGN_BOTTOM);
}
//****
void CBCGPAutoHideButton::ShowAttachedWindow (BOOL bShow)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pAutoHideWindow);
	
	// conllapse window out only if it is visible
	// expand window only id it is not visible

	if (bShow && !m_pAutoHideWindow->IsWindowVisible() ||
		!bShow && m_pAutoHideWindow->IsWindowVisible())
	{
		m_pAutoHideWindow->Slide (bShow);
	}

	m_pParentBar->SetActiveInGroup (bShow);

}
//****
void CBCGPAutoHideButton::ShowButton (BOOL bShow)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParentBar);

	m_bVisible = bShow;
}
//****
void CBCGPAutoHideButton::HighlightButton (BOOL bHighlight)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParentBar);

	m_bHighlighted = bHighlight;
}
//****
void CBCGPAutoHideButton::UnSetAutoHideMode (CBCGPDockingControlBar* pFirstBarInGroup)
{
	if (m_pAutoHideWindow != NULL)
	{
		m_pAutoHideWindow->UnSetAutoHideMode (pFirstBarInGroup);
	}
}
//****
void CBCGPAutoHideButton::ReplaceControlBar (CBCGPDockingControlBar* pNewBar)
{
	ASSERT_VALID (pNewBar);
	m_pAutoHideWindow = pNewBar;
	pNewBar->SetAutoHideParents (m_pParentBar, this);
}