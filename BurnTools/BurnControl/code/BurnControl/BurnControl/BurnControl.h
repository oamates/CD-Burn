
// BurnControl.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

// CBurnControlApp:
// �йش����ʵ�֣������ BurnControl.cpp
//

class CBurnControlApp : public CWinAppEx
{
public:
    CBurnControlApp();
    ~CBurnControlApp();

    // ��д
public:
    virtual BOOL InitInstance();


    // ʵ��

    DECLARE_MESSAGE_MAP()

private:
    //bool m_bConsole;
};

extern CBurnControlApp theApp;