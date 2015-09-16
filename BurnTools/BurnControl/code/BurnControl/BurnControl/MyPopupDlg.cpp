// MyPopupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BurnControl.h"
#include "MyPopupDlg.h"

#include "BurnControlDlg.h"

// CMyPopupDlg dialog

IMPLEMENT_DYNCREATE(CPopupDlg, CMFCDesktopAlertDialog)

CDialog * CPopupDlg::m_pParent=NULL;

CPopupDlg::CPopupDlg():
m_strFlag(STATE_DEFAULT),
m_strMsg(L"")
{

}

void CPopupDlg::DoDataExchange(CDataExchange* pDX)
{
    CMFCDesktopAlertDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_STATIC_MSG, m_staticMsg);
}


BEGIN_MESSAGE_MAP(CPopupDlg, CMFCDesktopAlertDialog)
    ON_WM_CLOSE()
    ON_WM_CTLCOLOR()
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_BUTTON1, &CPopupDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &CPopupDlg::OnBnClickedButton2)
    ON_WM_ERASEBKGND()
    ON_WM_CREATE()
END_MESSAGE_MAP()


// CMyPopupDlg message handlers

void CPopupDlg::OnClose()
{
    // TODO: Add your message handler code here and/or call default

    CMFCDesktopAlertDialog::OnClose();
}

//class CBurnControlDlg;

HBRUSH CPopupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CMFCDesktopAlertDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    // TODO:  Change any attributes of the DC here
    UINT id=pWnd->GetDlgCtrlID();

    if(CTLCOLOR_STATIC == nCtlColor)
    {
        switch (id)
        {
        case IDC_STATIC_MSG:
            pDC->SetBkMode(TRANSPARENT);
            hbr = (HBRUSH) ::GetStockObject(NULL_BRUSH);
            break;

        default:
            break;
        }
    }

    // TODO:  Return a different brush if the default is not desired
    return hbr;
}

void CPopupDlg::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // TODO: Add your message handler code here
    // Do not call CMFCDesktopAlertDialog::OnPaint() for painting messages
    // 
}

void CPopupDlg::OnBnClickedButton1()
{
    // TODO: Add your control notification handler code here
    if (m_pParent != NULL)
    {
        if (m_pParent->GetSafeHwnd() != NULL)
        {
            if (EXTRA_STATE_RTBURN_BACKUP_YES == m_strFlag)
            {
                ::PostMessageW(m_pParent->GetSafeHwnd(),WMUSER_BURNCONTROLDLG_BURNBACKUPYES,0,0);
            } 
            else if(EXTRA_STATE_RTBURN_BACKUP_RESTART_YES == m_strFlag)
            {
                ::PostMessageW(m_pParent->GetSafeHwnd(),WMUSER_BURNCONTROLDLG_BURNBACKUPAFTERRESTARTYES,0,0);
            }
        }
    }

    ::PostMessageW(GetParent()->GetSafeHwnd(),WM_CLOSE,0,0);
}

void CPopupDlg::OnBnClickedButton2()
{
    // TODO: Add your control notification handler code here
    if (m_pParent != NULL)
    {
        if (m_pParent->GetSafeHwnd() != NULL)
        {
            if (EXTRA_STATE_RTBURN_BACKUP_YES == m_strFlag)
            {
                ::PostMessageW(m_pParent->GetSafeHwnd(),WMUSER_BURNCONTROLDLG_BURNBACKUPNO,0,0);
            }
            else if(EXTRA_STATE_RTBURN_BACKUP_RESTART_YES == m_strFlag)
            {
                ::PostMessageW(m_pParent->GetSafeHwnd(),WMUSER_BURNCONTROLDLG_BURNBACKUPAFTERRESTARTNO,0,0);
            }
        }
    }

    ::PostMessageW(GetParent()->GetSafeHwnd(),WM_CLOSE,0,0);
}

CDialog * CPopupDlg::Parent()
{ 
    return m_pParent;
}

void CPopupDlg::Parent(CDialog * val)
{ 
    m_pParent = val;
}

BOOL CPopupDlg::OnInitDialog()
{
    CMFCDesktopAlertDialog::OnInitDialog();

    // TODO:  Add extra initialization here

    m_strFlag=((CBurnControlDlg *)m_pParent)->GetAlertFlag();
    m_strMsg=((CBurnControlDlg *)m_pParent)->GetAlertMsg();

    DisplayMsg();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CPopupDlg::DisplayMsg()
{
    if (m_pParent != NULL)
    {
        if (m_pParent->GetSafeHwnd() != NULL)
        {
            BOOL bShowBtn=FALSE;

            if (STATE_CONNECT_SERVER_FAILED == m_strFlag)
            {
            }
            else if (STATE_SINGLE_DEV_ERR_JOB_FAILED == m_strFlag)
            {
            }
            else if (STATE_SINGLE_DEV_ERR == m_strFlag)
            {
            }
            else if (STATE_BURN_ERR == m_strFlag)
            {
            }
            else if (STATE_DEFAULT == m_strFlag)
            {
            }
            else if (STATE_BURNING == m_strFlag)
            {
            }
            else if (STATE_SINGLE_DISC_BURNED == m_strFlag)
            {
            }
            else if (STATE_DOWNLOADING == m_strFlag)
            {
            }
            else if (STATE_WILL_CLOSE_DISC == m_strFlag)
            {
            }
            else if (STATE_CHANGE_DISC == m_strFlag)
            {
            }
            else if (STATE_TASK_OVER == m_strFlag)
            {
            }
            else if (STATE_JOB_OVER == m_strFlag)
            {
            }
            else if (STATE_JOB_FINISHED == m_strFlag)
            {
            }
            else if (STATE_RTJOB_PAUSED == m_strFlag)
            {
            }
            else if (STATE_RTJOB_STOPPED == m_strFlag)
            {
            }
            else if (EXTRA_STATE_RTBURN_BACKUP_YES == m_strFlag)
            {
                bShowBtn=TRUE;
            }
            else if (EXTRA_STATE_RTBURN_BACKUP_RESTART_YES == m_strFlag)
            {
                bShowBtn=TRUE;
            }

            GetDlgItem(IDC_BUTTON1)->ShowWindow(bShowBtn ? SW_SHOW : SW_HIDE);
            GetDlgItem(IDC_BUTTON2)->ShowWindow(bShowBtn ? SW_SHOW : SW_HIDE);

            GetDlgItem(IDC_STATIC_MSG)->SetWindowText(m_strMsg);
        }
    }
}

BOOL CPopupDlg::PreTranslateMessage(MSG* pMsg)
{
    // TODO: Add your specialized code here and/or call the base class
    
    return CMFCDesktopAlertDialog::PreTranslateMessage(pMsg);
}

BOOL CPopupDlg::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default


    return CMFCDesktopAlertDialog::OnEraseBkgnd(pDC);
}

int CPopupDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMFCDesktopAlertDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  Add your specialized creation code here

    return 0;
}

BOOL CPopupDlg::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Add your specialized code here and/or call the base class

    return CMFCDesktopAlertDialog::PreCreateWindow(cs);
}
