
// AuthDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "Ini.h"
#include <string>

#define NULLMD5 "d41d8cd98f00b204e9800998ecf8427e"

// CAuthDlg �Ի���
class CAuthDlg : public CDialog
{
// ����
public:
	CAuthDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_AUTH_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CString Md5(CString csPwd);//����md5�������
    bool RegisterDsoframerOcx();//ע��dsoframer.ocx
	void StartPlayBack();//�Զ�������Ƶ
	void GetPlaylistFilePath(CString & csPLfilePath);//�õ�playlist·��
    void GetPlaylistAndPlayBackFilePath(CString & csPLfilePath);//��csPLfilePath·�������ļ����У�����PlayBack��Playlist�ĵ�ַ
	void GetAppFold();//��ȡӦ�ó����Ŀ¼
	void GetAutoRunFilePath();//��ȡ�Զ������ļ���·��
	void GetPwdMd5();//��ȡ�������������
	CEdit m_editPwd;
	CButton m_btnEnter;
	CString m_csAppFold;
	CString m_csAutoRunFilePath;
	std::string m_strPwdMd5;//inf�ļ��е�����
	CStatic m_staticTip;
	CString m_csPlaylistFilePath;//
	CString m_csPlayBackFilePath;

public:
	void InitAutoRun();//��ʼ���Զ�����
};
