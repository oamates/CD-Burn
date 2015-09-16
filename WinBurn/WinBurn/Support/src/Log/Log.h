#pragma once

/************************************************************************/
/* 
#include "../LibCommon/Log.h"
CLogFile theLog;
VERIFY( theLog.SetFilePath(_T("D:\\app.log")));
theLog.SetMaxFileSize(0x100000);
#ifdef _UNICODE
theLog.SetFileFormat(Unicode);
#else
theLog.SetFileFormat(Utf8);
#endif // _UNICODE
theLog.SetLogLevel(LOG_DEBUG);
theLog.Open();
theLog.LogEndLine();

LogDebug  ("%s", "debug");
LogInfo   ("%s", "info");
LogWarning("%s", "warning");
LogError  ("%s", "error");                                        */
/************************************************************************/

#define ARRSIZE(x)	(sizeof(x)/sizeof(x[0]))

// Log message type enumeration
typedef enum LOG_LEVEL_T 
{
	LOG_ERROR	= 0,
	LOG_WARNING	= 1,
    LOG_INFO	= 2,
    LOG_DEBUG	= 3,
} LOG_LEVEL;

void LogDebug  (LPCTSTR pszLine, ...);
void LogInfo   (LPCTSTR pszLine, ...);
void LogWarning(LPCTSTR pszLine, ...);
void LogError  (LPCTSTR pszLine, ...);

///////////////////////////////////////////////////////////////////////////////
// CLogFile

enum ELogFileFormat
{
	Unicode = 0,
	Utf8
};

class CLogFile
{
public:
	CLogFile();
	~CLogFile();

	bool IsOpen() const;
	const CString& GetFilePath() const;
	bool SetFilePath(LPCTSTR pszFilePath);
	void SetMaxFileSize(UINT uMaxFileSize);

	bool Create(LPCTSTR pszFilePath, UINT uMaxFileSize = 1024*1024, ELogFileFormat eFileFormat = Unicode);
	bool Open();
	bool Close();
	bool Log(LOG_LEVEL logLevel, LPCTSTR pszMsg, int iLen = -1);
	bool Logf(LOG_LEVEL logLevel, LPCTSTR pszFmt, ...);
	bool LogEndLine();
	void StartNewLogFile();
	void SetLogLevel(LOG_LEVEL logLevel);

	bool SetFileFormat(ELogFileFormat eFileFormat);

protected:
	FILE* m_fp;
	time_t m_tStarted;
	CString m_strFilePath;
	UINT m_uBytesWritten;
	UINT m_uMaxFileSize;
	bool m_bInOpenCall;
	ELogFileFormat m_eFileFormat;
	LOG_LEVEL m_logLevel;
};

extern CLogFile theLog;

void LogV(LOG_LEVEL logLevel, LPCTSTR pszFmt, va_list argp);