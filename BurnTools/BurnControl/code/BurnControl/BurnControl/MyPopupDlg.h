#pragma once
#include "afxwin.h"


// CMyPopupDlg dialog

class CPopupDlg : public CMFCDesktopAlertDialog
{
    DECLARE_DYNCREATE(CPopupDlg)

public:
	CPopupDlg();   // standard constructor

// Dialog Data
	enum { IDD = IDD_DIALOG_POPUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnClose();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnPaint();
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();

    static CDialog * Parent();
    static void Parent(CDialog * val);

    virtual BOOL OnInitDialog();
private:
        void DisplayMsg();
private:
    CStatic         m_staticMsg;
    static CDialog  *m_pParent;
    std::string     m_strFlag;
    CStringW        m_strMsg;
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};
