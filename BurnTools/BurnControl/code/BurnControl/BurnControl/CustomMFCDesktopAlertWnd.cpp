#include "StdAfx.h"
#include "CustomMFCDesktopAlertWnd.h"

CCustomMFCDesktopAlertWnd::CCustomMFCDesktopAlertWnd()
{
}

CCustomMFCDesktopAlertWnd::~CCustomMFCDesktopAlertWnd(void)
{
}

BOOL CCustomMFCDesktopAlertWnd::PreTranslateMessage(MSG* pMsg)
{
    // TODO: Add your specialized code here and/or call the base class

    return CMFCDesktopAlertWnd::PreTranslateMessage(pMsg);
}
BEGIN_MESSAGE_MAP(CCustomMFCDesktopAlertWnd, CMFCDesktopAlertWnd)
    ON_WM_ERASEBKGND()
    ON_WM_CREATE()
END_MESSAGE_MAP()

BOOL CCustomMFCDesktopAlertWnd::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default

    return CMFCDesktopAlertWnd::OnEraseBkgnd(pDC);
}

int CCustomMFCDesktopAlertWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMFCDesktopAlertWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  Add your specialized creation code here
    //WS_EX_NOPARENTNOTIFY

    return 0;
}

BOOL CCustomMFCDesktopAlertWnd::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Add your specialized code here and/or call the base class

    return CMFCDesktopAlertWnd::PreCreateWindow(cs);
}
