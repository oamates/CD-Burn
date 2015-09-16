// WinBurn.h : main header file for the WinBurn DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWinBurnApp
// See WinBurn.cpp for the implementation of this class
//

class CWinBurnApp : public CWinApp
{
public:
	CWinBurnApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
