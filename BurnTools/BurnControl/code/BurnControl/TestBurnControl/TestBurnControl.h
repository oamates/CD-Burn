
// TestBurnControl.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTestBurnControlApp:
// �йش����ʵ�֣������ TestBurnControl.cpp
//

class CTestBurnControlApp : public CWinAppEx
{
public:
	CTestBurnControlApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTestBurnControlApp theApp;