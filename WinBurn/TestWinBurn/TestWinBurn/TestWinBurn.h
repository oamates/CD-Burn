
// WinBurn.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CWinBurnApp:
// �йش����ʵ�֣������ WinBurn.cpp
//

class CWinBurnApp : public CWinAppEx
{
public:
	CWinBurnApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CWinBurnApp theApp;