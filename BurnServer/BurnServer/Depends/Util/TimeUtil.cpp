#include "TimeUtil.h"
#ifdef WIN32
#define		WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include	<unistd.h>
#include	<sys/time.h>
#endif

std::string TimeUtil::GetTime_0()
{
    char szTime[256];
    memset(szTime, 0, 256);
#ifdef WIN32
    SYSTEMTIME	theTime;
    ::GetLocalTime(&theTime);
    sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", theTime.wYear, theTime.wMonth, theTime.wDay,
        theTime.wHour, theTime.wMinute, theTime.wSecond);
#else
    struct timeval	time;
    ::gettimeofday(&time,NULL);
    struct tm*	theTime;
    theTime	= ::localtime(&time.tv_sec);
    sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", theTime->tm_year+1900, theTime->tm_mon+1, theTime->tm_mday,
        theTime->tm_hour, theTime->tm_min, theTime->tm_sec);
#endif//WIN32

    return szTime;
}


std::string TimeUtil::GetTime_1()
{
    char szTime[256];
    memset(szTime, 0, 256);
#ifdef WIN32
    SYSTEMTIME	theTime;
    ::GetLocalTime(&theTime);
    sprintf(szTime, "%04d-%02d-%02d", theTime.wYear, theTime.wMonth, theTime.wDay);
#else
    struct timeval	time;
    ::gettimeofday(&time,NULL);
    struct tm*	theTime;
    theTime	= ::localtime(&time.tv_sec);
    sprintf(szTime, "%04d-%02d-%02d", theTime->tm_year+1900, theTime->tm_mon+1, theTime->tm_mday);
#endif//WIN32

    return szTime;
}

std::string TimeUtil::GetTime_2()
{
    char szTime[256];
    memset(szTime, 0, 256);
#ifdef WIN32
    SYSTEMTIME	theTime;
    ::GetLocalTime(&theTime);
    sprintf(szTime, "%02d:%02d:%02d", theTime.wHour, theTime.wMinute, theTime.wSecond);
#else
    struct timeval	time;
    ::gettimeofday(&time,NULL);
    struct tm*	theTime;
    theTime	= ::localtime(&time.tv_sec);
    sprintf(szTime, "%02d:%02d:%02d", theTime->tm_hour, theTime->tm_min, theTime->tm_sec);
#endif//WIN32

    return szTime;
}
