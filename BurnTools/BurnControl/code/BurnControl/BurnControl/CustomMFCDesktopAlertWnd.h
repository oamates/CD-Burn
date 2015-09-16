#pragma once
#include "afxdesktopalertwnd.h"

class CCustomMFCDesktopAlertWnd :
    public CMFCDesktopAlertWnd
{
public:
    CCustomMFCDesktopAlertWnd();
    ~CCustomMFCDesktopAlertWnd(void);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    DECLARE_MESSAGE_MAP()
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};
