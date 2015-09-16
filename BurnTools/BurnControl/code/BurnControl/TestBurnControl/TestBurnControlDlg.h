
// TestBurnControlDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CTestBurnControlDlg 对话框
class CTestBurnControlDlg : public CDialog
{
// 构造
public:
	CTestBurnControlDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_TESTBURNCONTROL_DIALOG };

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
    afx_msg void OnBnClickedSendBurnXml();
    CListCtrl m_ListCtrl;
    CImageList m_ImageList;
    afx_msg void OnDestroy();
    CEdit m_editIP;
    CEdit m_editPort;
    CEdit m_editTag;
};
