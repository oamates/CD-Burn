
// WinBurnDlg.h : ͷ�ļ�
//

#pragma once

#include "afxcmn.h"

#include "WinBurnSDK.h"
#include "afxwin.h"

// CWinBurnDlg �Ի���
class CWinBurnDlg : public CDialog
{
// ����
public:
	CWinBurnDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_WINBURN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	DECLARE_MESSAGE_MAP()
public:
    BURNHANDLE m_hBurnHandle;

    std::vector<BURNHANDLE> m_vecBurnHandle;

    CListCtrl m_listctrlRecorder;

    int m_nCurDevSel;

    afx_msg void OnBnClickedButtonSearchRecorder();
    afx_msg void OnBnClickedButtonEjectdisc();
    afx_msg void OnBnClickedButtonHavedisc();
    afx_msg void OnBnClickedButtonLoaddisc();
    afx_msg void OnBnClickedButtonBurnable();
    afx_msg void OnBnClickedButtonBlankdisc();
    afx_msg void OnBnClickedButtonDiscinfo();
    afx_msg void OnBnClickedButtonBurn();
    afx_msg void OnBnClickedButtonNewproject();
    afx_msg void OnBnClickedButtonAddfile();
    afx_msg void OnBnClickedButtonAdddir();

    CStringW SelectDir();
    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    CEdit m_editBurnSYN;

    bool ToBurnSYN() const;
};
