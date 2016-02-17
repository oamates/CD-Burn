
// AuthDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "Ini.h"
#include <string>

#define NULLMD5 "d41d8cd98f00b204e9800998ecf8427e"

// CAuthDlg 对话框
class CAuthDlg : public CDialog
{
// 构造
public:
	CAuthDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_AUTH_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CString Md5(CString csPwd);//返回md5后的密文
    bool RegisterDsoframerOcx();//注册dsoframer.ocx
	void StartPlayBack();//自动播放视频
	void GetPlaylistFilePath(CString & csPLfilePath);//得到playlist路径
    void GetPlaylistAndPlayBackFilePath(CString & csPLfilePath);//在csPLfilePath路径的子文件（夹）下找PlayBack与Playlist的地址
	void GetAppFold();//获取应用程序的目录
	void GetAutoRunFilePath();//获取自动运行文件的路径
	void GetPwdMd5();//获取光盘密码的密文
	CEdit m_editPwd;
	CButton m_btnEnter;
	CString m_csAppFold;
	CString m_csAutoRunFilePath;
	std::string m_strPwdMd5;//inf文件中的密文
	CStatic m_staticTip;
	CString m_csPlaylistFilePath;//
	CString m_csPlayBackFilePath;

public:
	void InitAutoRun();//初始化自动运行
};
