// PopDialog.cpp : implementation file
//

#include "stdafx.h"
#include "BurnControl.h"
#include "PopDialog.h"


// CPopDialog dialog

IMPLEMENT_DYNAMIC(CPopDialog,CDialogCustom)

CPopDialog::CPopDialog(CWnd* pParent /*=NULL*/)
	: CDialogCustom(CPopDialog::IDD, pParent),
    m_pMsgTargetWnd(NULL)
{
    CDialogCustom::Create(CPopDialog::IDD);
}

CPopDialog::~CPopDialog()
{
}

void CPopDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogCustom::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_MSG, m_staticMsgText);
}


BEGIN_MESSAGE_MAP(CPopDialog, CDialogCustom)
    ON_WM_ERASEBKGND()
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CPopDialog message handlers

int CPopDialog::PopMsg(int nMsgType, CString strMsgText)
{
    m_staticMsgText.SetWindowText(strMsgText);

    switch (nMsgType)
    {
    case 1:
        {
            CWnd *pWnd;

            pWnd=GetDlgItem(IDOK);
            if (pWnd != NULL)
            {
                if (pWnd->GetSafeHwnd() != NULL)
                {
                    pWnd->ShowWindow(SW_SHOW);
                }
            }

            pWnd=GetDlgItem(IDCANCEL);
            if (pWnd != NULL)
            {
                if (pWnd->GetSafeHwnd() != NULL)
                {
                    pWnd->ShowWindow(SW_SHOW);
                }
            }
        }
    	break;

    case 2:
        {
            CWnd *pWnd;

            pWnd=GetDlgItem(IDOK);
            if (pWnd != NULL)
            {
                if (pWnd->GetSafeHwnd() != NULL)
                {
                    pWnd->ShowWindow(SW_SHOW);
                }
            }

            pWnd=GetDlgItem(IDCANCEL);
            if (pWnd != NULL)
            {
                if (pWnd->GetSafeHwnd() != NULL)
                {
                    pWnd->ShowWindow(SW_SHOW);
                }
            }
        }
        break;
    }

    SetWindowPos(NULL,m_rectPopPos.left,m_rectPopPos.top,m_rectPopPos.Width(),m_rectPopPos.Height(),SWP_NOACTIVATE);
    AnimateWindow(1000,AW_VER_NEGATIVE | AW_SLIDE);

    return 0;
}

void CPopDialog::SetMsgTargetWnd(CWnd *pWnd)
{
    m_pMsgTargetWnd=pWnd;
}

BOOL CPopDialog::OnInitDialog()
{
    SetMaxButtonVisible(FALSE);
    SetMinButtonVisible(FALSE);
    SetCloseButtonVisible(FALSE);

    CDialogCustom::OnInitDialog();

    // TODO:  Add extra initialization here


    CRect rectWorkArea;
    SystemParametersInfo(SPI_GETWORKAREA,0,&rectWorkArea,0);

    CRect rectClient;
    GetClientRect(&rectClient);
    
    int nLeft=rectWorkArea.right-rectClient.Width();
    int nTop=rectWorkArea.bottom-rectClient.Height();
    int nRight=rectWorkArea.right;
    int nBottom=rectWorkArea.bottom;

    m_rectPopPos.SetRect(nLeft,nTop,nRight,nBottom);

    m_nBkTop=15;
    m_nBkBottom=15;

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPopDialog::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default

    return CDialogCustom::OnEraseBkgnd(pDC);
}

HBRUSH CPopDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogCustom::OnCtlColor(pDC, pWnd, nCtlColor);

    // TODO:  Change any attributes of the DC here
    int id=pWnd->GetDlgCtrlID();
    if (CTLCOLOR_STATIC == nCtlColor)
    {
        switch (id)
        {
        case IDC_STATIC_MSG:
            pDC->SetTextColor(RGB(255,0,0));
            pDC->SetBkMode(TRANSPARENT);
            hbr = (HBRUSH) ::GetStockObject(NULL_BRUSH);
            break;
        }
    }

    // TODO:  Return a different brush if the default is not desired
    return hbr;
}
