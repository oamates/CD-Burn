
// AuthDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Auth.h"
#include "AuthDlg.h"
#include "md5.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAuthDlg �Ի���




CAuthDlg::CAuthDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAuthDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAuthDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_editPwd);
	DDX_Control(pDX, IDOK, m_btnEnter);
	DDX_Control(pDX, IDC_STATIC_TIP, m_staticTip);
}

BEGIN_MESSAGE_MAP(CAuthDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CAuthDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAuthDlg ��Ϣ�������

BOOL CAuthDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	ShowWindow(SW_SHOWMINIMIZED);
	InitAutoRun();
	std::string strNullMd5 = NULLMD5;
	transform(m_strPwdMd5.begin(),m_strPwdMd5.end(),m_strPwdMd5.begin(),tolower);
    RegisterDsoframerOcx();
	if (m_strPwdMd5 == strNullMd5)
	{
		StartPlayBack();
		SendMessage(WM_CLOSE);
	}
	ShowWindow(SW_SHOWNORMAL);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CAuthDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CAuthDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAuthDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	ShowWindow(SW_SHOWMINIMIZED);
	//std::string strPwdMd5 = GetPwdMd5();
	CString csPwd;
	m_editPwd.GetWindowText(csPwd);
	CString csInputPwdMd5 = Md5(csPwd);
	std::string strInputPwdMd5 = csInputPwdMd5.GetBuffer();

 	if (m_strPwdMd5 != strInputPwdMd5)
 	{
		ShowWindow(SW_SHOWNORMAL);
		m_staticTip.SetWindowText(_T("����������������룡"));
 		return;
 	}
	StartPlayBack();
	OnOK();
}

CString CAuthDlg::Md5(CString csInputPwd)
{
	MD5 md5(csInputPwd.GetBuffer());
	CString csInputPwdMd5;
	std::string strInputPwdMd5 = md5.toString().c_str();
	csInputPwdMd5 = strInputPwdMd5.c_str();

	return csInputPwdMd5;
}

bool CAuthDlg::RegisterDsoframerOcx()
{
    bool bRet = false;
    char szSystemDiretory[255];
    GetSystemDirectory(szSystemDiretory, sizeof(szSystemDiretory));
    if (szSystemDiretory != "")
    {
        strcat(szSystemDiretory, "\\dsoframer.ocx");
        CFileFind fileFind;
        BOOL bFileFind = fileFind.FindFile(szSystemDiretory);
        if (!bFileFind)
        {//ϵͳ�ļ�����û��dsoframer.ocx
            //�õ�dsoframer.ocx�Ĺ���·��
            CString strDsoframerPath = _T("");
            int nLen	= m_csPlayBackFilePath.GetLength();
            int nReturn = m_csPlayBackFilePath.ReverseFind('\\');
            if (nReturn != -1)
            {
                strDsoframerPath = m_csPlayBackFilePath.Left(nReturn + 1);
            }
            else
            {
                nReturn = m_csPlayBackFilePath.ReverseFind('/');
                if (nReturn != -1)
                {
                    strDsoframerPath = m_csPlayBackFilePath.Left(nReturn + 1);
                }
            }

            if (strDsoframerPath != _T(""))
            {
                HINSTANCE hLib;
                strDsoframerPath += _T("dsoframer.ocx");
                if (fileFind.FindFile(strDsoframerPath))
                {//�����ļ���dsoframer.ocx�Ƿ����
                    if (CopyFile(strDsoframerPath,szSystemDiretory,TRUE))
                    {
                        hLib = LoadLibrary(szSystemDiretory);
                        if (hLib >= (HINSTANCE)HINSTANCE_ERROR)
                        {
                            FARPROC lpDllEntryPoint;
                            //��ȡע�ắ��DllRegisterServer��ַ
                            lpDllEntryPoint = GetProcAddress(hLib,_T("DllRegisterServer"));
                            //����ע�ắ��DllRegisterServer
                            if(lpDllEntryPoint!=NULL)
                            {
                                if(FAILED((*lpDllEntryPoint)()))
                                {
                                    FreeLibrary(hLib);
                                }
                                else
                                {
                                    bRet = true;
                                }
                            }
                        }
                    }
                    if (!bRet)
                    {//û��ע��ɹ����ڹ���ע��
                        hLib = LoadLibrary(strDsoframerPath);
                        if (hLib >= (HINSTANCE)HINSTANCE_ERROR)
                        {
                            FARPROC lpDllEntryPoint;
                            //��ȡע�ắ��DllRegisterServer��ַ
                            lpDllEntryPoint = GetProcAddress(hLib,_T("DllRegisterServer"));
                            //����ע�ắ��DllRegisterServer
                            if(lpDllEntryPoint!=NULL)
                            {
                                if(FAILED((*lpDllEntryPoint)()))
                                {
                                    FreeLibrary(hLib);
                                }
                                else
                                {
                                    bRet = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return bRet;
}

void CAuthDlg::StartPlayBack()
{
	::ShellExecute(
		::GetDesktopWindow(), 
		_T("open"), 
		m_csPlayBackFilePath.GetBuffer(), 
		m_csPlaylistFilePath.GetBuffer(), 
		NULL,
		SW_SHOWNORMAL);	
}

void CAuthDlg::GetPlaylistFilePath(CString & csPLfilePath)
{
	CString csAutoRunFile;
	csAutoRunFile = m_csAppFold + _T("autorun.inf");

	CIni iniAutoRunFile;
	iniAutoRunFile.SetPathName(csAutoRunFile.GetBuffer());
	TCHAR chBuffer[MAX_PATH] = {0};
	iniAutoRunFile.GetString(_T("AutoRun"), _T("playlistpath"),chBuffer,MAX_PATH);
	csPLfilePath += chBuffer;
}

void CAuthDlg::GetPlaylistAndPlayBackFilePath(CString & csPLfilePath)
{
    CString strFindDir=csPLfilePath;

    if (strFindDir.GetAt(strFindDir.GetLength()-1) != _T('\\'))
    {
        strFindDir+="\\";
    }

    strFindDir+=CString(_T("*.*"));

    CFileFind fileFind;

    CString strExtension=_T(".playlist");
    CString strPlayBackName=_T("PlayBack.exe");

    BOOL bIsFileFound=fileFind.FindFile(strFindDir);
    while (bIsFileFound)
    {
        bIsFileFound=fileFind.FindNextFile();

        if (!fileFind.IsDirectory() &&
            !fileFind.IsDots())
        { 
            CString strFileName=fileFind.GetFileName();

            CString strSub=strFileName.Right(strExtension.GetLength());
            if (strExtension == strSub)
            {
                m_csPlaylistFilePath=fileFind.GetFilePath();
            }

            if (strFileName == strPlayBackName)
            {
                m_csPlayBackFilePath=fileFind.GetFilePath();
            }
        }
        else if (fileFind.IsDirectory() &&
            !fileFind.IsDots())
        {
            CString str=fileFind.GetFilePath();
            GetPlaylistAndPlayBackFilePath(str);
        }
    }

    fileFind.Close();
}

void CAuthDlg::InitAutoRun()
{
	GetAppFold();
	GetAutoRunFilePath();
	GetPwdMd5();

	CIni iniAutoRunFile;
	iniAutoRunFile.SetPathName(m_csAutoRunFilePath);
	TCHAR chBuffer[MAX_PATH] = {0};
	iniAutoRunFile.GetString(_T("AutoRun"), _T("playlistpath"),chBuffer,MAX_PATH);
	m_csPlaylistFilePath = chBuffer;
    if (m_csPlaylistFilePath != _T(""))
    {
        m_csPlaylistFilePath = m_csAppFold + m_csPlaylistFilePath;
    }
	ZeroMemory(chBuffer,MAX_PATH);
	iniAutoRunFile.GetString(_T("AutoRun"), _T("playbackpath"),chBuffer,MAX_PATH);
	m_csPlayBackFilePath = chBuffer;
    if (m_csPlayBackFilePath != _T(""))
    {
        m_csPlayBackFilePath = m_csAppFold + m_csPlayBackFilePath;
    }

    if (m_csPlayBackFilePath == _T("") || m_csPlaylistFilePath == _T(""))
    {//���Զ����������ļ���û���ҵ���������Playlist�ĵ�ַ
        GetPlaylistAndPlayBackFilePath(m_csAppFold);
    }
}

void CAuthDlg::GetAppFold()
{
	TCHAR szAppFold[MAX_PATH];
	GetModuleFileName(NULL, szAppFold, MAX_PATH - 1);
	_tcsrchr(szAppFold, _T('\\'))[1] = _T('\0');
	m_csAppFold = szAppFold;
}

void CAuthDlg::GetAutoRunFilePath()
{
	m_csAutoRunFilePath = m_csAppFold + _T("autorun.inf");
}

void CAuthDlg::GetPwdMd5()
{
	LPWSTR *szArgList;
	int nArgs;
	int i = 1;
	szArgList = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	CStringW csPwdMd5;
	csPwdMd5 = szArgList[i];

	TCHAR chBuffer[MAX_PATH] = {0};
	//����״̬�£��õ��Ĳ�������"?",�����޸ķ������
#ifdef _DEBUG
#else
	int nSize = WideCharToMultiByte(CP_ACP, NULL, csPwdMd5.GetBuffer(), -1, NULL, 0, NULL, FALSE);
	WideCharToMultiByte (CP_ACP, NULL, csPwdMd5.GetBuffer(), -1, chBuffer, nSize, NULL, FALSE);
#endif	
	std::string strPwdMd5 = chBuffer;

	//���ֶ������˳����ʱ�򣬳��Զ�ȡ.inf�е���������
	if (strPwdMd5.empty())
	{
		CIni iniAutoRunFile;
		iniAutoRunFile.SetPathName(m_csAutoRunFilePath.GetBuffer());
		TCHAR chBuffer[MAX_PATH] = {0};
		iniAutoRunFile.GetString(_T("AutoRun"), _T("open"), chBuffer, MAX_PATH);
		CString csPwdMd5 = chBuffer;
		int nPos = csPwdMd5.Find(_T(" "));
		int nLength = csPwdMd5.GetLength();
		csPwdMd5 = csPwdMd5.Right(nLength - (nPos + 1));
		strPwdMd5 = csPwdMd5.GetBuffer();
	}
	m_strPwdMd5 = strPwdMd5;
}