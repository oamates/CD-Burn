#pragma once
#include "afxwin.h"


// CPopDialog dialog

class CPopDialog : public CDialogCustom
{
	DECLARE_DYNAMIC(CPopDialog)

public:
	CPopDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPopDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_POPUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    int PopMsg(int nMsgType, CString strMsgText);

    void SetMsgTargetWnd(CWnd *pWnd);
private:
    CWnd *m_pMsgTargetWnd;

    CRect m_rectPopPos;

public:
    virtual BOOL OnInitDialog();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    CStatic m_staticMsgText;
};
