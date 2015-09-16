
// TestBurnControlDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CTestBurnControlDlg �Ի���
class CTestBurnControlDlg : public CDialog
{
// ����
public:
	CTestBurnControlDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TESTBURNCONTROL_DIALOG };

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
    afx_msg void OnBnClickedSendBurnXml();
    CListCtrl m_ListCtrl;
    CImageList m_ImageList;
    afx_msg void OnDestroy();
    CEdit m_editIP;
    CEdit m_editPort;
    CEdit m_editTag;
};
