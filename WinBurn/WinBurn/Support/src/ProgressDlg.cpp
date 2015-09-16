// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ProgressDlg.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "./Common/StringUtil.h"
#include "Messages.h"
#include "Log/Log.h"


/*CProgressDlg * g_pProgressDlg;*/

// CProgressDlg dialog

CProgressDlg::CProgressDlg()
	: m_ucPercent(0)
	, m_bCancelled(false)
{
    m_nBurnState=0;
}

CProgressDlg::~CProgressDlg()
{
}

// CProgressDlg message handlers

void CProgressDlg::set_progress(unsigned char percent)
{
	//TRACE(_T("percent %c\n", percent));
	if (percent < 0)
		percent = 0;
	else if (percent > 100)
		percent = 100;

	// Make sure that the progress does not go in the wrong direction.
	if (percent < m_ucPercent && percent != 0)
		return;

	// Only redraw when we have to.
	if (m_ucPercent != percent)
	{
		m_ucPercent = percent;
	}
}

void CProgressDlg::set_status(const TCHAR *szStatus,...)
{
	// Prepare the string.
	TCHAR szStatusStr[256];
	lstrcpy(szStatusStr,lngGetString(PROGRESS_STATUS));

	unsigned int uiFreeSpace = sizeof(szStatusStr)/sizeof(TCHAR) - lstrlen(szStatusStr) - 1;
	if ((unsigned int)lstrlen(szStatus) > uiFreeSpace)
		lstrncat(szStatusStr,szStatus,uiFreeSpace);
	else
		lstrcat(szStatusStr,szStatus);

	// Parse the variable argument list.
	va_list args;
	va_start(args,szStatus);

#ifdef UNICODE
	_vsnwprintf(m_szStringBuffer,PROGRESS_STRINGBUFFER_SIZE - 1,szStatusStr,args);
#else
	_vsnprintf(m_szStringBuffer,PROGRESS_STRINGBUFFER_SIZE - 1,szStatusStr,args);
#endif

}

void CProgressDlg::notify(ckcore::Progress::MessageType Type,const TCHAR *szMessage,...)
{
//	m_sProgressMsg += _T("\n");
	// 获取时间.
	SYSTEMTIME st;
	GetLocalTime(&st);
	TCHAR szTime[9];	// xx:yy:zz
	_stprintf(szTime,_T("%.2d:%.2d:%.2d"),st.wHour,st.wMinute,st.wSecond);
	szTime[8] = _T('\0');

	// Convert the log type to an index.
	int iImageIndex = 0;
	switch (Type)
	{
	case ckcore::Progress::ckINFORMATION:
		iImageIndex = 0;

        SetBurnState(1);

		break;
	case ckcore::Progress::ckWARNING:
		iImageIndex = 1;

        SetBurnState(1);

		break;
	case ckcore::Progress::ckERROR:
		iImageIndex = 2;

        SetBurnState(-1);

		break;
	case ckcore::Progress::ckEXTERNAL:
		iImageIndex = 3;

        SetBurnState(1);//这个状态有待验证

		break;
	}

	// Parse the variable argument list.
	va_list args;
	va_start(args,szMessage);

#ifdef UNICODE
	_vsnwprintf(m_szStringBuffer,PROGRESS_STRINGBUFFER_SIZE - 1,szMessage,args);
#else
	_vsnprintf(m_szStringBuffer,PROGRESS_STRINGBUFFER_SIZE - 1,szMessage,args);
#endif

}

bool CProgressDlg::cancelled()
{
	return m_bCancelled;
}

void CProgressDlg::NotifyCompleted()
{
    wprintf_s(_T("刻录结束了...\r\n"));

	// 获取时间.
	SYSTEMTIME st;
	GetLocalTime(&st);
	TCHAR szTime[9];	// xx:yy:zz
	_stprintf(szTime,_T("%.2d:%.2d:%.2d"),st.wHour,st.wMinute,st.wSecond);
	szTime[8] = _T('\0');

	CString sMessage(_T("刻录结束"));

}

void CProgressDlg::SetRealMode(bool bRealMode)
{

}

void CProgressDlg::AllowReload()
{

}

void CProgressDlg::AllowCancel(bool bAllow)
{

}

bool CProgressDlg::RequestNextDisc()
{
    return true;
}

// Starts the smoke effect.
void CProgressDlg::StartSmoke()
{
	TRACE(_T("[CProgressDlg::StartSmoke]\n"));
}

void CProgressDlg::SetDevice(const TCHAR *szDevice)
{
	ckcore::tstring DeviceStr = lngGetString(PROGRESS_DEVICE);
	DeviceStr += szDevice;
	CString sMessage = CString(DeviceStr.c_str());

	// 获取时间.
	SYSTEMTIME st;
	GetLocalTime(&st);
	TCHAR szTime[9];	// xx:yy:zz
	_stprintf(szTime,_T("%.2d:%.2d:%.2d"),st.wHour,st.wMinute,st.wSecond);
	szTime[8] = _T('\0');

}

void CProgressDlg::SetDevice(ckmmc::Device &Device)
{
	SetDevice( NDeviceUtil::GetDeviceName(Device).c_str() );
}


void CProgressDlg::OnNMRClickListProgressmsg(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	*pResult = 0;
}

void CProgressDlg::OnBurnclearlog()
{
}

int CProgressDlg::GetBurnState() const 
{ 
    return m_nBurnState;
}

void CProgressDlg::SetBurnState(int val)
{ 
    m_nBurnState = val; 
}


const wchar_t * CProgressDlg::GetBurnStateDescription() const
{
    return m_szStringBuffer;
}

void CProgressDlg::SetBurnStateDescription(const wchar_t *s)
{
    wcscpy_s(m_szStringBuffer,sizeof(m_szStringBuffer)/sizeof(TCHAR),s);
}

