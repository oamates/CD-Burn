#include "stdafx.h"
#include "DialogCustom.h"
#include "../../resource.h"

// ensure not conflict with ID in dialog
#define IDC_CUSTOMDLG_BUTTON_MIN    10000
#define IDC_CUSTOMDLG_BUTTON_MAX    10001
#define IDC_CUSTOMDLG_BUTTON_CLOSE  10002

CDialogCustom::CDialogCustom(CWnd* pParent)
: CDialogSK(pParent)
{
    InitParameter();
}

CDialogCustom::CDialogCustom(UINT uResourceID, CWnd* pParent)
: CDialogSK(uResourceID, pParent)
{
    InitParameter();
}

CDialogCustom::CDialogCustom(LPCTSTR pszResourceID, CWnd* pParent)
: CDialogSK(pszResourceID, pParent)
{
    InitParameter();
}

CDialogCustom::~CDialogCustom()
{

}

void CDialogCustom::SetMaxButtonVisible(BOOL bVisible)
{
    m_bShowButtonMax   = bVisible;
}

void CDialogCustom::SetMinButtonVisible(BOOL bVisible)
{
    m_bShowButtonMin   = bVisible;
}

void CDialogCustom::SetCloseButtonVisible(BOOL bVisible)
{
    m_bShowButtonClose = bVisible;
}

void CDialogCustom::SetIgnoreESC(BOOL bIgnore)
{
    m_bIgnoreESC       = bIgnore;
}

void CDialogCustom::SetIgnoreReturn(BOOL bIgnore)
{
    m_bIgnoreReturn    = bIgnore;
}

void CDialogCustom::ShowMaximize()
{
    if (IsZoomed())
    {
        SendMessage(WM_SYSCOMMAND, SC_RESTORE, NULL);
        m_btnMax.SetIcon(IDI_ICON_MAX_OVER, IDI_ICON_MAX);
    }
    else
    {
        SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, NULL);
        m_btnMax.SetIcon(IDI_ICON_RESTORE_OVER, IDI_ICON_RESTORE);
    }
}

void CDialogCustom::MoveCenterWindow(CRect rect)
{
    HWND hParentOrOwner;
    RECT rc, rc2;
    int x,y;
    hParentOrOwner=::GetParent(GetSafeHwnd());
    if(hParentOrOwner == NULL)
    {
        SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
    }
    else
    {
        ::GetClientRect(hParentOrOwner, &rc);
    }
//     if (rect.IsRectEmpty())
//     {
//         ::GetWindowRect(GetSafeHwnd(), &rc2);
//     }
//     else
    {
        rc2.left = rect.left;
        rc2.top = rect.top;
        rc2.right = rect.right;
        rc2.bottom = rect.bottom;
    }
    x = ((rc.right-rc.left) - (rc2.right-rc2.left)) / 2 +rc.left;
    y = ((rc.bottom-rc.top) - (rc2.bottom-rc2.top)) / 2 +rc.top;

    UINT uFlags;
    if (IsWindowVisible())
    {
        uFlags = SWP_SHOWWINDOW;
    }
    else
    {
        uFlags = SWP_HIDEWINDOW;
    }
    ::SetWindowPos(GetSafeHwnd(),HWND_TOPMOST,x, y,rc2.right-rc2.left, rc2.bottom-rc2.top,uFlags);
}

void CDialogCustom::InitParameter()
{
    m_rectBtnMin              = CRect(0, 0, 0, 0);
    m_rectBtnMax              = CRect(0, 0, 0, 0);
    m_rectBtnClose            = CRect(0, 0, 0, 0);

    m_bShowButtonMin          = TRUE;
    m_bShowButtonMax          = TRUE;
    m_bShowButtonClose        = TRUE;

    m_bIgnoreESC              = FALSE;
    m_bIgnoreReturn           = FALSE;

    m_rgbInitialBk            = RGB(85,116,175);

    //m_rgbFrame                = RGB(0,89,169);
    m_rgbFrame                = RGB(0,75,140);
    //m_rgbFrameOther           = RGB(60,150,230);
	m_rgbFrameOther           = RGB(0,75,140);
    //m_rgbBk                   = RGB(237,246,255);
    //m_rgbBk                   = RGB(230,246,255);
	m_rgbBk                   = RGB(255,255,255);
    m_rgbBkOther              = RGB(255,255,255);

    m_pointFrameAngle.x       = 10;
    m_pointFrameAngle.y       = 10;
    m_pointBkAngle.x          = 10; 
    m_pointBkAngle.y          = 10;
    m_nBkTop                  = 30;

	m_nBkLeft                 = 6;
	m_nBkBottom               = 6;

    m_nGradientLeft           = 10;
	m_nGradientTop            = 0;
    m_nGradientBottom         = 10; 
    m_nBkReduce               = 2;
}

int CDialogCustom::CalculateRect()
{
    int nButtonWidth          = 24;
    int nButtonHeight         = 25;
	int nButtonTop            = 0;
	int nButtonCloseWidth     = 42;
	int nButtonCloseHeight    = 25;

    CRect   rectWindow;
    GetClientRect(rectWindow);

    m_rectBtnClose.right      = rectWindow.Width() - 5;
    m_rectBtnClose.left       = m_rectBtnClose.right - nButtonCloseWidth;
    m_rectBtnClose.top        = nButtonTop;
    m_rectBtnClose.bottom     = m_rectBtnClose.top + nButtonCloseHeight;

    if (m_bShowButtonMax)
    {
        m_rectBtnMax.right    = m_rectBtnClose.left;
        m_rectBtnMax.left     = m_rectBtnMax.right - nButtonWidth;
        m_rectBtnMax.top      = nButtonTop;
        m_rectBtnMax.bottom   = m_rectBtnMax.top + nButtonHeight;
    }
    else
    {
        m_rectBtnMax.right    = m_rectBtnClose.left;
        m_rectBtnMax.left     = m_rectBtnMax.right - 0;
        m_rectBtnMax.top      = nButtonTop;
        m_rectBtnMax.bottom   = m_rectBtnMax.top + nButtonHeight;
    }

    if (m_bShowButtonMin)
    {
        m_rectBtnMin.right    = m_rectBtnMax.left;
        m_rectBtnMin.left     = m_rectBtnMin.right - nButtonWidth;
        m_rectBtnMin.top      = nButtonTop;
        m_rectBtnMin.bottom   = m_rectBtnMin.top + nButtonHeight;
    }
    else
    {
        m_rectBtnMin.right    = m_rectBtnMax.left;
        m_rectBtnMin.left     = m_rectBtnMin.right - 0;
        m_rectBtnMin.top      = nButtonTop;
        m_rectBtnMin.bottom   = m_rectBtnMin.top + nButtonHeight;
    }

    if (!m_bShowButtonClose)
    {
        m_rectBtnClose        = CRect(0, 0, 0, 0);
    }

    return 0;
}

void CDialogCustom::Relayout()
{
    CalculateRect();

    if (m_btnMin.GetSafeHwnd()   != NULL)
    {
        m_btnMin.MoveWindow(m_rectBtnMin);
    }

    if (m_btnMax.GetSafeHwnd()   != NULL)
    {
        m_btnMax.MoveWindow(m_rectBtnMax);
    }

    if (m_btnClose.GetSafeHwnd() != NULL)
    {
        m_btnClose.MoveWindow(m_rectBtnClose);
    }

    Invalidate();
}

BOOL CDialogCustom::OwnerDrawBackground(CDC* pDC)
{
//     CDC dcMem;
//     if (dcMem.CreateCompatibleDC(pDC))
//     {
//         CRect rect;
//         GetClientRect(rect);
// 
//         //内存中承载临时图象的位图
//         CBitmap bmp;
//         //创建兼容位图
//         bmp.CreateCompatibleBitmap(pDC,rect.Width(),rect.Height());
//         CBitmap* pOldBmp = dcMem.SelectObject(&bmp); 
//         //用颜色m_rgbInitialBk填充客户区
//         dcMem.FillSolidRect(rect,m_rgbInitialBk);
//         
//         CBrush  cbrFrame(m_rgbFrame);
//         CPen    cpen;
// 		cpen.CreatePen(PS_SOLID,1,RGB(0,0,0));
// 
//         //创建边框DC
//         CBrush* pOldBr   = dcMem.SelectObject(&cbrFrame);  
//         CPen*   pOldPen  = dcMem.SelectObject(&cpen);
// 
//         dcMem.RoundRect(&rect,m_pointFrameAngle);
// 
//         //创建内部底色DC
//         CBrush cbrBk(m_rgbBk);
//         dcMem.SelectObject(cbrBk);
// 
//         //内部底色CRect对象
//         CRect rectBk = CRect(rect.left+m_nBkLeft,rect.top+m_nBkTop,rect.right-m_nBkLeft,rect.bottom-m_nBkBottom);
// 
//         dcMem.RoundRect(&rectBk,m_pointBkAngle);
// 
//         CDrawingManager dm(dcMem);
// 
//         //画内部底色渐变效果
//         dm.FillGradient(CRect(rectBk.left+m_nBkReduce,rectBk.top+m_nBkReduce,rectBk.right-m_nBkReduce,rectBk.bottom-rectBk.Height()/m_nBkReduce),m_rgbBkOther,m_rgbBk,TRUE);
//         dm.FillGradient(CRect(rectBk.left+m_nBkReduce,rectBk.bottom-rectBk.Height()/2,rectBk.right-m_nBkReduce,rectBk.bottom-m_nBkReduce),m_rgbBk,m_rgbBkOther,TRUE);
// 
//         //画上边框颜色渐变效果
// 		dm.FillGradient(CRect(m_nGradientLeft,m_nGradientTop,rect.Width()-m_nGradientLeft,m_nGradientBottom),m_rgbFrame,m_rgbFrameOther,TRUE);
// 
//         COLORREF clr = ::GetPixel(dcMem.GetSafeHdc(), m_nGradientLeft, m_nGradientTop + 1);
//         dm.FillGradient(CRect(m_nGradientLeft-2,m_nGradientTop+1,m_nGradientLeft,m_nGradientBottom),m_rgbFrame,clr,TRUE);
//         dm.FillGradient(CRect(rect.Width()-m_nGradientLeft,m_nGradientTop+1,rect.Width()-m_nGradientLeft+2,m_nGradientBottom),m_rgbFrame,clr,TRUE);
// 
//         pDC->BitBlt(rect.left,rect.top,rect.Width(),rect.Height(),&dcMem,0,0,SRCCOPY);
// 
//         dcMem.SelectObject(pOldPen);
//         dcMem.SelectObject(pOldBr);
//         dcMem.SelectObject(pOldBmp);
// 
//         return TRUE;		
//     }
// 
//     ASSERT(FALSE);
//     return FALSE;
	CDC dcMem;
	if (dcMem.CreateCompatibleDC(pDC))
	{
		CRect rect;
		GetClientRect(rect);

		//内存中承载临时图象的位图
		CBitmap bmp;
		//创建兼容位图
		bmp.CreateCompatibleBitmap(pDC,rect.Width(),rect.Height());
		CBitmap* pOldBmp = dcMem.SelectObject(&bmp); 
		//用颜色m_rgbInitialBk填充客户区
		dcMem.FillSolidRect(rect,m_rgbInitialBk);

		CBrush  cbrFrame(m_rgbFrame);
		CPen    cpen;
		cpen.CreatePen(PS_SOLID,1,m_rgbFrame);

		//创建边框DC
		CBrush* pOldBr   = dcMem.SelectObject(&cbrFrame);  
		CPen*   pOldPen  = dcMem.SelectObject(&cpen);

		dcMem.Rectangle(&rect);

		//创建内部底色DC
		CBrush cbrBk(m_rgbBk);
		dcMem.SelectObject(cbrBk);

		//内部底色CRect对象
		CRect rectBk = CRect(rect.left+m_nBkLeft,rect.top+m_nBkTop,rect.right-m_nBkLeft,rect.bottom-m_nBkBottom);

		dcMem.Rectangle(&rectBk);

		CDrawingManager dm(dcMem);

		//画内部底色渐变效果
		dm.FillGradient(CRect(rectBk.left,rectBk.top,rectBk.right,rectBk.bottom-rectBk.Height()/2),m_rgbBkOther,m_rgbBk,TRUE);
		dm.FillGradient(CRect(rectBk.left,rectBk.bottom-rectBk.Height()/2,rectBk.right,rectBk.bottom),m_rgbBk,m_rgbBkOther,TRUE);

		//画上边框颜色渐变效果
		dm.FillGradient(CRect(m_nGradientLeft,m_nGradientTop,
			rect.Width() - m_nGradientLeft,m_nGradientBottom),
			m_rgbFrame,m_rgbFrameOther,TRUE);

		COLORREF clr = ::GetPixel(dcMem.GetSafeHdc(), m_nGradientLeft, m_nGradientTop);
		dm.FillGradient(CRect(0,m_nGradientTop,m_nGradientLeft,m_nGradientBottom),m_rgbFrame,clr,TRUE);
		dm.FillGradient(CRect(rect.Width() - m_nGradientLeft,m_nGradientTop,rect.Width(),m_nGradientBottom),m_rgbFrame,clr,TRUE);

		pDC->BitBlt(rect.left,rect.top,rect.Width(),rect.Height(),&dcMem,0,0,SRCCOPY);

		//dcMem.SelectObject(pOldPen);
		dcMem.SelectObject(pOldBr);
		dcMem.SelectObject(pOldBmp);

		return TRUE;		
	}

	ASSERT(FALSE);
	return FALSE;
}

BEGIN_MESSAGE_MAP(CDialogCustom, CDialogSK)
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_CUSTOMDLG_BUTTON_MIN, &CDialogCustom::OnBnClickedButtonMin)
    ON_BN_CLICKED(IDC_CUSTOMDLG_BUTTON_MAX, &CDialogCustom::OnBnClickedButtonMax)
    ON_BN_CLICKED(IDC_CUSTOMDLG_BUTTON_CLOSE, &CDialogCustom::OnBnClickedButtonClose)
END_MESSAGE_MAP()

BOOL CDialogCustom::OnInitDialog()
{
    CDialogSK::OnInitDialog();

    // hide caption
    LONG lStyle   = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
    lStyle        = lStyle|WS_MINIMIZEBOX;
    lStyle        = lStyle&~(WS_CAPTION);
    SetWindowLong(GetSafeHwnd(), GWL_STYLE, lStyle);
    // 
    LONG lExStyle = GetWindowLong(GetSafeHwnd(), GWL_EXSTYLE);
    lExStyle      = lExStyle&~(WS_EX_DLGMODALFRAME);
    SetWindowLong(GetSafeHwnd(), GWL_EXSTYLE, lExStyle);

    // create min button
    m_btnMin.Create(_T(""), WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0, 0, 0, 0), this, IDC_CUSTOMDLG_BUTTON_MIN);
    m_btnMin.DrawBorder(FALSE);
    m_btnMin.DrawTransparent();
    m_btnMin.SetIcon(IDI_ICON_MIN_OVER, IDI_ICON_MIN);

    // create max button
    m_btnMax.Create(_T(""), WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0, 0, 0, 0), this, IDC_CUSTOMDLG_BUTTON_MAX);
    m_btnMax.DrawBorder(FALSE);
    m_btnMax.DrawTransparent();
    m_btnMax.SetIcon(IDI_ICON_MAX_OVER, IDI_ICON_MAX);

    // create close button
    m_btnClose.Create(_T(""), WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0, 0, 0, 0), this, IDC_CUSTOMDLG_BUTTON_CLOSE);
    m_btnClose.DrawBorder(FALSE);
    m_btnClose.DrawTransparent();
    m_btnClose.SetIcon(IDI_ICON_CLOSE_OVER, IDI_ICON_CLOSE);

	SetTransparentColor(m_rgbInitialBk);

    Relayout();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDialogCustom::OnEraseBkgnd(CDC* pDC)
{
//     TRACE("[CDialogCustom::OnEraseBkgnd] %d\r\n", GetTickCount());
    if (GetBitmap() == NULL)
    {
        return CDialogCustom::OwnerDrawBackground(pDC);
    }
    else
    {
        return CDialogSK::OnEraseBkgnd(pDC);
    }
    
}

void CDialogCustom::OnSize(UINT nType, int cx, int cy)
{
    CDialogSK::OnSize(nType, cx, cy);

    if (nType == SIZE_MAXIMIZED)
    {
        CRect rectWorkArea;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWorkArea, 0);
        MoveWindow(rectWorkArea);
    }
    Relayout();
}

void CDialogCustom::OnBnClickedButtonMin()
{
    SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, NULL);
}

void CDialogCustom::OnBnClickedButtonMax()
{
    ShowMaximize();
}

void CDialogCustom::OnBnClickedButtonClose()
{
    SendMessage(WM_SYSCOMMAND, SC_CLOSE, NULL);
}

BOOL CDialogCustom::PreTranslateMessage(MSG* pMsg)
{
    if (m_bIgnoreESC && (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
        && pMsg->wParam == VK_ESCAPE)
    {
        return TRUE;
    }

    if (m_bIgnoreReturn && (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
        && pMsg->wParam == VK_RETURN)
    {
        return TRUE;
    }

    return CDialogSK::PreTranslateMessage(pMsg);
}
