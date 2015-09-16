// LogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LogDlg.h"


// CLogDlg dialog

CLogDlg::CLogDlg()
{
}

CLogDlg::~CLogDlg()
{
}


void CLogDlg::print(const tchar *format,...)
{
	TCHAR szStringBuffer[256];
	va_list argp;
	va_start(argp, format);	
	LogV(LOG_INFO, format, argp);

#ifdef UNICODE
	_vsnwprintf(szStringBuffer,255,format,argp);
#else
	_vsnprintf(szStringBuffer,255,format,argp);
#endif

	va_end(argp);
}

void CLogDlg::print_line(const tchar *format,...)
{
	TCHAR szStringBuffer[256];
	va_list argp;
	va_start(argp, format);	
	LogV(LOG_INFO, format, argp);

#ifdef UNICODE
	_vsnwprintf(szStringBuffer,255,format,argp);

    wprintf_s(L"%s\r\n",szStringBuffer);
#else
	_vsnprintf(szStringBuffer,255,format,argp);
#endif

	va_end(argp);
}