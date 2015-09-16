//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include <io.h>
#include <share.h>
#include "Log.h"
#include "StringConversion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CLogFile theLog;

void AddLogTextV(LOG_LEVEL logLevel, LPCTSTR pszLine, va_list argptr)
{
	ASSERT(pszLine != NULL);

	TCHAR szLogLine[1000];
	if (_vsntprintf(szLogLine, ARRSIZE(szLogLine), pszLine, argptr) == -1)
		szLogLine[ARRSIZE(szLogLine) - 1] = _T('\0');

	//TRACE(_T("App Log: %s\n"), szLogLine);

	TCHAR szFullLogLine[1160];
	CString sHeader;
	if (logLevel == LOG_DEBUG)
	{
		sHeader = _T("[DEBUG]\t\t");
	}
	else if (logLevel == LOG_INFO)
	{
		sHeader = _T("[INFO]\t\t");
	}
	else if (logLevel == LOG_WARNING)
	{
		sHeader = _T("[WARNING]\t");
	}
	else if (logLevel == LOG_ERROR)
	{
		sHeader = _T("[ERROR]\t\t");
	}

	int iLen = _sntprintf(szFullLogLine, ARRSIZE(szFullLogLine), _T("%s- %s %s\r\n"), sHeader, CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S")), szLogLine);
	if (iLen >= 0)
	{
		theLog.Log(logLevel, szFullLogLine, iLen);
	}
}

void LogV(LOG_LEVEL logLevel, LPCTSTR pszFmt, va_list argp)
{
	AddLogTextV(logLevel, pszFmt, argp);
}

///////////////////////////////////////////////////////////////////////////////
// 

void LogDebug(LPCTSTR pszFmt, ...)
{
	va_list argp;
	va_start(argp, pszFmt);
	LogV(LOG_DEBUG, pszFmt, argp);
	va_end(argp);
}

void LogInfo(LPCTSTR pszFmt, ...)
{
	va_list argp;
	va_start(argp, pszFmt);	
	LogV(LOG_INFO, pszFmt, argp);
	va_end(argp);
}

void LogWarning(LPCTSTR pszFmt, ...)
{
	va_list argp;
	va_start(argp, pszFmt);	
	LogV(LOG_WARNING, pszFmt, argp);
	va_end(argp);
}

void LogError(LPCTSTR pszFmt, ...)
{
	va_list argp;
	va_start(argp, pszFmt);	
	LogV(LOG_ERROR, pszFmt, argp);
	va_end(argp);
}

///////////////////////////////////////////////////////////////////////////////
// CLogFile

CLogFile::CLogFile()
{
	(void)m_strFilePath;
	m_uMaxFileSize = (UINT)-1;
	m_uBytesWritten = 0;
	m_tStarted = 0;
	m_fp = NULL;
	m_bInOpenCall = false;
	m_logLevel = LOG_ERROR;
	m_eFileFormat = Utf8;
}

CLogFile::~CLogFile()
{
	Close();
}

const CString& CLogFile::GetFilePath() const
{
	return m_strFilePath;
}

bool CLogFile::SetFilePath(LPCTSTR pszFilePath)
{
	if (IsOpen())
		return false;
	m_strFilePath = pszFilePath;
	return true;
}

void CLogFile::SetMaxFileSize(UINT uMaxFileSize)
{
	if (uMaxFileSize == 0)
		uMaxFileSize = (UINT)-1;
	else if (uMaxFileSize < 0x10000)
		uMaxFileSize = 0x10000;
	m_uMaxFileSize = uMaxFileSize;
}

bool CLogFile::SetFileFormat(ELogFileFormat eFileFormat)
{
	if (eFileFormat != Unicode && eFileFormat != Utf8){
		ASSERT(0);
		return false;
	}
	if (m_fp != NULL)
		return false; // can't change file format on-the-fly
	m_eFileFormat = eFileFormat;
	return true;
}

bool CLogFile::IsOpen() const
{
	return m_fp != NULL;
}

bool CLogFile::Create(LPCTSTR pszFilePath, UINT uMaxFileSize, ELogFileFormat eFileFormat)
{
	Close();
	m_strFilePath = pszFilePath;
	m_uMaxFileSize = uMaxFileSize;
	m_eFileFormat = eFileFormat;
	return Open();
}

bool CLogFile::Open()
{
	if (m_fp != NULL)
		return true;

	m_fp = _tfsopen(m_strFilePath, _T("a+b"), _SH_DENYWR);
	if (m_fp != NULL)
	{
		m_tStarted = time(NULL);
		m_uBytesWritten = _filelength(fileno(m_fp));
		if (m_uBytesWritten == 0)
		{
			if (m_eFileFormat == Unicode)
			{
				// write Unicode byte-order mark 0xFEFF
				fputwc(0xFEFF, m_fp);
			}
			else
			{
				ASSERT( m_eFileFormat == Utf8 );
				; // could write UTF-8 header..
			}
		}
		else if (m_uBytesWritten >= sizeof(WORD))
		{
			// check for Unicode byte-order mark 0xFEFF
			WORD wBOM;
			if (fread(&wBOM, sizeof(wBOM), 1, m_fp) == 1)
			{
				if (wBOM == 0xFEFF && m_eFileFormat == Unicode)
				{
					// log file already in Unicode format
					fseek(m_fp, 0, SEEK_END); // actually not needed because file is opened in 'Append' mode..
				}
				else if (wBOM != 0xFEFF && m_eFileFormat != Unicode)
				{
					// log file already in UTF-8 format
					fseek(m_fp, 0, SEEK_END); // actually not needed because file is opened in 'Append' mode..
				}
				else
				{
					// log file does not have the required format, create a new one (with the req. format)
					ASSERT( (m_eFileFormat==Unicode && wBOM!=0xFEFF) || (m_eFileFormat==Utf8 && wBOM==0xFEFF) );

					ASSERT( !m_bInOpenCall );
					if (!m_bInOpenCall) // just for safety
					{
						m_bInOpenCall = true;
						StartNewLogFile();
						m_bInOpenCall = false;
					}
				}
			}
		}
	}
	return m_fp != NULL;
}

bool CLogFile::Close()
{
	if (m_fp == NULL)
		return true;
	bool bResult = (fclose(m_fp) == 0);
	m_fp = NULL;
	m_tStarted = 0;
	m_uBytesWritten = 0;
	return bResult;
}

bool CLogFile::Log(LOG_LEVEL logLevel, LPCTSTR pszMsg, int iLen)
{
	if (m_fp == NULL)
		return false;

	// ignore the low log level
	if (logLevel > m_logLevel)
	{
		return true;
	}	

	size_t uWritten;
	if (m_eFileFormat == Unicode)
	{
		// don't use 'fputs' + '_filelength' -- gives poor performance
		size_t uToWrite = ((iLen == -1) ? _tcslen(pszMsg) : (size_t)iLen)*sizeof(TCHAR);
		uWritten = fwrite(pszMsg, 1, uToWrite, m_fp);
	}
	else
	{
#ifdef _UNICODE
		TUnicodeToUTF8<2048> utf8(pszMsg, iLen);
		uWritten = fwrite((LPCSTR)utf8, 1, utf8.GetLength(), m_fp);
#else
		uWritten = fwrite((LPCSTR)pszMsg, 1, iLen, m_fp);
#endif // _UNICODE
	}
	bool bResult = !ferror(m_fp);
	m_uBytesWritten += uWritten;

	if (m_uBytesWritten >= m_uMaxFileSize)
		StartNewLogFile();
	else
		fflush(m_fp);

	return bResult;
}

bool CLogFile::Logf(LOG_LEVEL logLevel, LPCTSTR pszFmt, ...)
{
	if (m_fp == NULL)
		return false;

	va_list argp;
	va_start(argp, pszFmt);

	TCHAR szMsg[1024];
	_vsntprintf(szMsg, ARRSIZE(szMsg), pszFmt, argp);

	TCHAR szFullMsg[1060];
	int iLen = _sntprintf(szFullMsg, ARRSIZE(szFullMsg), _T("%s: %s\r\n"), CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S")), szMsg);
	va_end(argp);
	return Log(logLevel, szFullMsg, iLen);
}

bool CLogFile::LogEndLine()
{
	TCHAR szEndLine[5] = _T("\r\n");
	int iLen = _tcslen(szEndLine);
	if (m_fp == NULL)
		return false;

	size_t uWritten;
	if (m_eFileFormat == Unicode)
	{
		// don't use 'fputs' + '_filelength' -- gives poor performance
		size_t uToWrite = ((iLen == -1) ? _tcslen(szEndLine) : (size_t)iLen)*sizeof(TCHAR);
		uWritten = fwrite(szEndLine, 1, uToWrite, m_fp);
	}
	else
	{
#ifdef _UNICODE
		TUnicodeToUTF8<2048> utf8(szEndLine, iLen);
		uWritten = fwrite((LPCSTR)utf8, 1, utf8.GetLength(), m_fp);
#else
		uWritten = fwrite((LPCSTR)szEndLine, 1, iLen, m_fp);
#endif // _UNICODE
	}
	bool bResult = !ferror(m_fp);
	m_uBytesWritten += uWritten;

	if (m_uBytesWritten >= m_uMaxFileSize)
		StartNewLogFile();
	else
		fflush(m_fp);

	return bResult;
}

void CLogFile::StartNewLogFile()
{
	time_t tStarted = m_tStarted;
	Close();

	TCHAR szDateLogStarted[40];
	_tcsftime(szDateLogStarted, ARRSIZE(szDateLogStarted), _T("%Y.%m.%d %H.%M.%S"), localtime(&tStarted));

	TCHAR szDrv[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szNam[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(m_strFilePath, szDrv, szDir, szNam, szExt);

	CString strLogBakNam;
	strLogBakNam = szNam;
	strLogBakNam += _T(" - ");
	strLogBakNam += szDateLogStarted;

	TCHAR szLogBakFilePath[MAX_PATH];
	_tmakepathlimit(szLogBakFilePath, szDrv, szDir, strLogBakNam, szExt);

	if (_trename(m_strFilePath, szLogBakFilePath) != 0)
		_tremove(m_strFilePath);

	Open();
}

void CLogFile::SetLogLevel( LOG_LEVEL logLevel )
{
	m_logLevel = logLevel;
}