
// WinBurnDlg.h : 头文件
//

#pragma once

#include "afxcmn.h"

#include "WinBurnSDK.h"
#include "afxwin.h"

// CWinBurnDlg 对话框
class CWinBurnDlg : public CDialog
{
// 构造
public:
	CWinBurnDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_WINBURN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
