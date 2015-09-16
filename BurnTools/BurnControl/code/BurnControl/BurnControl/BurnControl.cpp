
// BurnControl.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "BurnControl.h"
#include "BurnControlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBurnControlApp

BEGIN_MESSAGE_MAP(CBurnControlApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CBurnControlApp ����

CBurnControlApp::CBurnControlApp()
//: m_bConsole(false)
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}

CBurnControlApp::~CBurnControlApp()
{

}


// Ψһ��һ�� CBurnControlApp ����

ZLog theLog;
CBurnControlApp theApp;

BOOL CALLBACK EnumWindowsProc(HWND hWnd,LPARAM lParam)
{
    CWnd *pWnd=CWnd::FromHandle(hWnd);
    if (pWnd != NULL &&
        pWnd->GetSafeHwnd() != NULL)
    {
        CStringW strWindowText;
        pWnd->GetWindowText(strWindowText);

        if (L"BurnControl" == strWindowText)
        {
            pWnd->ShowWindow(SW_SHOW);
        }
    }

    return TRUE;
}


// CBurnControlApp ��ʼ��

BOOL CBurnControlApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

    //////////////////////////////////////////////////////////////////////////
    HANDLE hMutex = CreateMutex(NULL,FALSE,L"BURNCONTROL_2014_02_18");  
    if(hMutex != NULL &&
        ERROR_ALREADY_EXISTS == ::GetLastError())
    {
        EnumWindows(EnumWindowsProc,NULL);

        return FALSE;
    }

    LOG_SET_PATH(BURNCONTROL_LOG_PATH);
    LOG_SET_LEVEL(ZLog::LOG_INFORMATION);

    InitTaskModel();
    InterfaceServer::Initialize("C:\\CS\\BurnControl\\Log\\Ncx\\");

    {
        CBurnControlDlg dlg;
        m_pMainWnd = &dlg;
        INT_PTR nResponse = dlg.DoModal();
        if (nResponse == IDOK)
        {
            // TODO: �ڴ˷��ô����ʱ��
            //  ��ȷ�������رնԻ���Ĵ���
        }
        else if (nResponse == IDCANCEL)
        {
            // TODO: �ڴ˷��ô����ʱ��
            //  ��ȡ�������رնԻ���Ĵ���
        }
    }

    InterfaceServer::UnInitialize();
    UninitTaskModel();
    //////////////////////////////////////////////////////////////////////////


    // ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
    //  ����������Ӧ�ó������Ϣ�á�

	return FALSE;
}
