#include "ZLog.h"
#include "ZOSMutex.h"
#include "ZOSMemory.h"
///////////////////////////////////////////////////////////////////////////////
#include	<stdlib.h>
#include	<stdarg.h>
#include	<fcntl.h>
#ifdef _WIN32_
#include	<io.h>
#else
#include	<unistd.h>
#include	<sys/time.h>
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static	const	ZOSMutex	gLogMutex("LogMutex");
///////////////////////////////////////////////////////////////////////////////
ZLog*	g_pLog								= NULL;
///////////////////////////////////////////////////////////////////////////////
ZOSMutex*	ZLog::m_pLogMutex					= (ZOSMutex*)&gLogMutex;
#ifdef _WIN32_
char		ZLog::m_sFilePath[MAX_FILE_PATH+4]	= "C:/TaskModel/log";
#else
char		ZLog::m_sFilePath[MAX_FILE_PATH+4]	= "/home/TaskModel/log";
#endif
int			ZLog::m_nLogLevel					= LOG_DEBUG;
///////////////////////////////////////////////////////////////////////////////
ZLog::ZLog()
{
	memset(m_sFileName,0,sizeof(m_sFileName));
	m_nLogFile	= INVALID_FILE_HANDLE;
	m_hFile		= NULL;

    m_bSingleLoop = FALSE;
    m_nLoopThresholdSize = 512*1024;//512k
    strcpy(m_sFixName, "fixname");

    memset(m_sStackInfoFile, 0, sizeof(m_sStackInfoFile));
    memset(m_sStackInfoFunction, 0, sizeof(m_sStackInfoFunction));
    m_nStackInfoLine = -1;

	g_pLog		= this;
}
ZLog::ZLog(const char* sLogPath,const int nLogLevel)
{
	if(sLogPath != NULL)
	{
		strncpy(m_sFilePath,sLogPath,MAX_FILE_PATH);
	}
	m_nLogLevel	= nLogLevel;
	memset(m_sFileName,0,sizeof(m_sFileName));
	m_nLogFile	= INVALID_FILE_HANDLE;
	m_hFile		= NULL;

    m_bSingleLoop = FALSE;
    m_nLoopThresholdSize = 512*1024;//512k
    strcpy(m_sFixName, "fixname");

	g_pLog		= this;
}
ZLog::~ZLog()
{
	g_pLog		= NULL;
	ZLog::CloseLogFile();
}
///////////////////////////////////////////////////////////////////////////////
void ZLog::Logout(const char* sFormat,...)
{
	char			sLogs[MAX_MESSAGE_DATA+4];
	va_list			arglist;
	ZOSMutexLocker	lock(m_pLogMutex);
	LOG_LEVEL		eLogLevel = ZLog::LOG_DEBUG;


	if (m_nLogLevel >= eLogLevel)
	{
		ZLog::CheckLogFile(eLogLevel);

		if(sFormat != NULL)
		{
			va_start(arglist,sFormat);
			vsnprintf(sLogs,MAX_MESSAGE_DATA,sFormat,arglist);
			va_end(arglist);
            if (strlen(sLogs) > 0)
            {
                if (sLogs[strlen(sLogs) - 1] != '\n')
                {
                    strcat(sLogs, "\r\n");
                }
            }
			if(m_nLogLevel >= eLogLevel)
			{
#ifdef	_WIN32_
				::OutputDebugStringA(sLogs);
#else
				printf(sLogs);
#endif	//
			}
			if(m_hFile != NULL)
			{
				fwrite(sLogs,1,strlen(sLogs),m_hFile);
				fflush(m_hFile);
			}else{
				if(m_nLogFile != INVALID_FILE_HANDLE)
				{
					::write(m_nLogFile,sLogs,strlen(sLogs));
				}
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
void ZLog::LOGError(const char* sFormat,...)
{
	char			sLogs[MAX_MESSAGE_DATA+1];
	va_list			arglist;
	ZOSMutexLocker	lock(m_pLogMutex);
	LOG_LEVEL		eLogLevel = ZLog::LOG_ERROR;


	if (m_nLogLevel >= eLogLevel)
	{
		ZLog::CheckLogFile(eLogLevel);

		if(sFormat != NULL)
		{
			va_start(arglist,sFormat);
			vsnprintf(sLogs,MAX_MESSAGE_DATA,sFormat,arglist);
			va_end(arglist);
            if (strlen(sLogs) > 0)
            {
                if (sLogs[strlen(sLogs) - 1] != '\n')
                {
                    strcat(sLogs, "\r\n");
                }
            }
            if(m_nLogLevel >= eLogLevel)
			{
#ifdef	_WIN32_
				::OutputDebugStringA(sLogs);
				printf(sLogs);
#else
				printf(sLogs);
#endif	//
			}
			if(m_hFile != NULL)
			{
				fwrite(sLogs,strlen(sLogs),1,m_hFile);
				fflush(m_hFile);
			}else{
				if(m_nLogFile != INVALID_FILE_HANDLE)
				{
					::write(m_nLogFile,sLogs,strlen(sLogs));
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void ZLog::LOGWarning(const char* sFormat,...)
{
	char			sLogs[MAX_MESSAGE_DATA+1];
	va_list			arglist;
	ZOSMutexLocker	lock(m_pLogMutex);
	LOG_LEVEL		eLogLevel = ZLog::LOG_WARNING;


	if (m_nLogLevel >= eLogLevel)
	{
		ZLog::CheckLogFile(eLogLevel);

		if(sFormat != NULL)
		{
			va_start(arglist,sFormat);
			vsnprintf(sLogs,MAX_MESSAGE_DATA,sFormat,arglist);
			va_end(arglist);
            if (strlen(sLogs) > 0)
            {
                if (sLogs[strlen(sLogs) - 1] != '\n')
                {
                    strcat(sLogs, "\r\n");
                }
            }
			if(m_nLogLevel >= eLogLevel)
			{
#ifdef	_WIN32_
				::OutputDebugStringA(sLogs);
				printf(sLogs);
#else
				printf(sLogs);
#endif	//
			}
			if(m_hFile != NULL)
			{
				fwrite(sLogs,strlen(sLogs),1,m_hFile);
				fflush(m_hFile);
			}else{
				if(m_nLogFile != INVALID_FILE_HANDLE)
				{
					::write(m_nLogFile,sLogs,strlen(sLogs));
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void ZLog::LOGInfo(const char* sFormat,...)
{
	char			sLogs[MAX_MESSAGE_DATA+1];
	va_list			arglist;
	ZOSMutexLocker	lock(m_pLogMutex);
	LOG_LEVEL		eLogLevel = ZLog::LOG_INFORMATION;


	if (m_nLogLevel >= eLogLevel)
	{
		ZLog::CheckLogFile(eLogLevel);

		if(sFormat != NULL)
		{
			va_start(arglist,sFormat);
			vsnprintf(sLogs,MAX_MESSAGE_DATA,sFormat,arglist);
			va_end(arglist);
            if (strlen(sLogs) > 0)
            {
                if (sLogs[strlen(sLogs) - 1] != '\n')
                {
                    strcat(sLogs, "\r\n");
                }
            }
			if(m_nLogLevel >= eLogLevel)
			{
#ifdef	_WIN32_
				::OutputDebugStringA(sLogs);
				printf(sLogs);
#else
				printf(sLogs);
#endif	//
			}
			if(m_hFile != NULL)
			{
				fwrite(sLogs,strlen(sLogs),1,m_hFile);
				fflush(m_hFile);
			}else{
				if(m_nLogFile != INVALID_FILE_HANDLE)
				{
					::write(m_nLogFile,sLogs,strlen(sLogs));
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void ZLog::LOGDebug(const char* sFormat,...)
{
	char			sLogs[MAX_MESSAGE_DATA+1];
	va_list			arglist;
	ZOSMutexLocker	lock(m_pLogMutex);
	LOG_LEVEL		eLogLevel = ZLog::LOG_DEBUG;


	if (m_nLogLevel >= eLogLevel)
	{
		ZLog::CheckLogFile(eLogLevel);

		if(sFormat != NULL)
		{
			va_start(arglist,sFormat);
			vsnprintf(sLogs,MAX_MESSAGE_DATA,sFormat,arglist);
			va_end(arglist);
            if (strlen(sLogs) > 0)
            {
                if (sLogs[strlen(sLogs) - 1] != '\n')
                {
                    strcat(sLogs, "\r\n");
                }
            }
			if(m_nLogLevel >= eLogLevel)
			{
#ifdef	_WIN32_
				::OutputDebugStringA(sLogs);
				printf(sLogs);
#else
				printf(sLogs);
#endif	//
			}
			if(m_hFile != NULL)
			{
				fwrite(sLogs,strlen(sLogs),1,m_hFile);
				fflush(m_hFile);
			}else{
				if(m_nLogFile != INVALID_FILE_HANDLE)
				{
					::write(m_nLogFile,sLogs,strlen(sLogs));
				}
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
int ZLog::OpenLogFile(const char* sFileName)
{
	char	sFullName[2000];

	if(m_nLogFile == INVALID_FILE_HANDLE)
	{
		if(sFileName != 0)
		{
			sprintf(sFullName,"%s/%s",m_sFilePath,sFileName);
			ZOS::recursivemakedir(m_sFilePath);
            m_nLogFile	= ::open(sFullName,(O_CREAT|O_WRONLY),0666);
            //
			if(m_nLogFile != INVALID_FILE_HANDLE)
			{
				m_hFile	= ::fdopen(m_nLogFile,"wb");
				if(m_hFile != NULL)
				{
					strcpy(m_sFileName,sFileName);
                    fseek(m_hFile, 0, SEEK_END);
				}
			}
		}
	}
	return m_nLogFile;
}
int ZLog::CloseLogFile()
{
	if(m_hFile != NULL)
	{
		::fclose(m_hFile);
	}
	if(m_nLogFile != INVALID_FILE_HANDLE)
	{
		if(m_hFile == NULL)
		{
			::close(m_nLogFile);
		}
	}
	m_hFile		= NULL;
	m_nLogFile	= INVALID_FILE_HANDLE;

	return 0;
}
///////////////////////////////////////////////////////////////////////////////
int	ZLog::CheckLogFile(LOG_LEVEL eLogLevel)
{
	char	sTime[256];
	char	sFileName[256];
	char	sLevel[30];
    char    sStackInfo[1024];

#ifdef	_WIN32_
	SYSTEMTIME	Time;
	::GetLocalTime(&Time);
	sprintf(sTime,"%04d-%02d-%02d %02d:%02d:%02d:%03d\t",Time.wYear,Time.wMonth,Time.wDay,Time.wHour,Time.wMinute,Time.wSecond,Time.wMilliseconds);
	sprintf(sFileName,"%04d%02d%02d.log",Time.wYear,Time.wMonth,Time.wDay);
#else
	struct timeval	time;
	::gettimeofday(&time,NULL);
	struct tm*	Time;
	Time	= ::localtime(&time.tv_sec);
	sprintf(sTime,"%04d-%02d-%02d %02d:%02d:%02d:%03d\t",(Time->tm_year+1900),(Time->tm_mon+1),Time->tm_mday,Time->tm_hour,Time->tm_min,Time->tm_sec,(int)(time.tv_usec%1000000/1000));
	sprintf(sFileName,"%04d%02d%02d.log",(Time->tm_year+1900),(Time->tm_mon+1),Time->tm_mday);
#endif	//_WIN32_

    if (m_bSingleLoop)
    {
        strcpy(sFileName, m_sFixName);
        if (strcmp(sFileName, "") == 0)
        {
            strcpy(sFileName, "fixname");
        }
    }

	if (eLogLevel == LOG_ERROR)
	{
		strcpy(sLevel, "[ERROR  ]\t");
	}
	else if (eLogLevel == LOG_WARNING)
	{
		strcpy(sLevel, "[WARNING]\t");
	}
	else if (eLogLevel == LOG_INFORMATION)
	{
		strcpy(sLevel, "[INFO   ]\t");
	}
	else if (eLogLevel == LOG_DEBUG)
	{
		strcpy(sLevel, "[DEBUG  ]\t");
	}

    sprintf(sStackInfo, "[%s]\t", m_sStackInfoFunction);

	if(strcmp(m_sFileName,sFileName)!=0)
	{
		ZLog::CloseLogFile();
		ZLog::OpenLogFile(sFileName);
	}
    if (m_bSingleLoop)
    {
        if (m_hFile != NULL)
        {
            long nCurrentPos = ftell(m_hFile);
            if (nCurrentPos >= m_nLoopThresholdSize)
            {
                rewind(m_hFile);
            }
        }
    }
	if(m_hFile != NULL)
	{
		fwrite(sLevel,1,strlen(sLevel),m_hFile);
		fwrite(sTime,1,strlen(sTime),m_hFile);
        fwrite(sStackInfo,1,strlen(sStackInfo),m_hFile);
		fflush(m_hFile);
	}else{
		if(m_nLogFile != INVALID_FILE_HANDLE)
		{
			::write(m_nLogFile,sLevel,strlen(sLevel));
			::write(m_nLogFile,sTime,strlen(sTime));
            ::write(m_nLogFile,sStackInfo,strlen(sStackInfo));
		}
	}
	if(m_nLogLevel > 0)
	{
#ifdef	_WIN32_
		::OutputDebugStringA(sLevel);
		::OutputDebugStringA(sTime);
        ::OutputDebugStringA(sStackInfo);
#else
		printf(sLevel);
		printf(sTime);
        printf(sStackInfo);
#endif	//
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
void ZLog::MessageOut(const char* sFormat,...)
{
	char			sMessage[MAX_MESSAGE_DATA*8];
	char			sTime[256];
	va_list			arglist;
	ZOSMutexLocker	lock(m_pLogMutex);

 	if(sFormat != NULL)
	{
		if(m_nLogLevel > 0)
		{
#ifdef	_WIN32_
			SYSTEMTIME	Time;
			::GetLocalTime(&Time);
			sprintf(sTime,"%04d-%02d-%02d %02d:%02d:%02d:%03d\t",Time.wYear,Time.wMonth,Time.wDay,Time.wHour,Time.wMinute,Time.wSecond,Time.wMilliseconds);
#else
			struct timeval	time;
			::gettimeofday(&time,NULL);
			struct tm*	Time;              
			Time	= ::localtime(&time.tv_sec); 
			sprintf(sTime,"%04d-%02d-%02d %02d:%02d:%02d:%03d\t",(Time->tm_year+1900),(Time->tm_mon+1),Time->tm_mday,Time->tm_hour,Time->tm_min,Time->tm_sec,(int)(time.tv_usec%1000000/1000));
#endif	//_WIN32_
#ifdef	_WIN32_
			::OutputDebugStringA(sTime);
#else
			::printf(sTime);
#endif	//
		}
		if(m_nLogLevel > 0)
		{
			va_start(arglist,sFormat);
			vsnprintf(sMessage,MAX_MESSAGE_DATA,sFormat,arglist);
			va_end(arglist);
#ifdef	_WIN32_
			::OutputDebugStringA(sMessage);
#else
			::printf(sMessage);
#endif	//
		}
	}
}

void ZLog::SetStackInfo(const char *sFile, const char *sFunction, int nLine)
{
    strncpy(m_sStackInfoFile, sFile, 256);
    strncpy(m_sStackInfoFunction, sFunction, 256);
    m_nStackInfoLine = nLine;
}

///////////////////////////////////////////////////////////////////////////////
char* ZLog::GetLogPath()
{
	ZOSMutexLocker	lock(m_pLogMutex);
	return(m_sFilePath);
}
int ZLog::GetLogLevel()
{
	ZOSMutexLocker	lock(m_pLogMutex);
	return(m_nLogLevel);
}
void ZLog::SetLogPath(const char* sLogPath)
{
	ZOSMutexLocker	lock(m_pLogMutex);

	if(sLogPath != NULL)
	{
		strncpy(m_sFilePath,sLogPath,MAX_FILE_PATH);
	}
}
void ZLog::SetLogLevel(int nLogLevel)
{
	ZOSMutexLocker	lock(m_pLogMutex);
	m_nLogLevel	= nLogLevel;
}
void ZLog::SetSingleLoop(BOOL bLoop)
{
    m_bSingleLoop = bLoop;
}
void ZLog::SetLoopThreshold(int nThresholdSize)
{
    m_nLoopThresholdSize = nThresholdSize;
}
void ZLog::SetFixName(const char* sLogName)
{
    strcpy(m_sFixName, sLogName);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
