#include "SystemUtil.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#endif

SystemUtil::SystemUtil()
{
    //
}

SystemUtil::~SystemUtil()
{
    //
}

UINT64 SystemUtil::GetMemoryTotalSize()
{
#ifdef WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex))
    {
        return statex.ullTotalPhys;
    }

    return 0;
#else
    return 0;
#endif
}

UINT64 SystemUtil::GetMemoryFreeSize()
{
#ifdef WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex))
    {
        return statex.ullAvailPhys;
    }

    return 0;
#else
    return 0;
#endif
}

double SystemUtil::GetCPUUtilization(int nSleepMilliSecond)
{
#ifdef WIN32
    double dLastIdleTime;
    double dLastKernelTime;
    double dLastUserTime;
    double dLastSystemTime;
    double dCurrentIdleTime;
    double dCurrentKernelTime;
    double dCurrentUserTime;
    double dCurrentSystemTime;

    if (GetCPUTimeState(dLastIdleTime, dLastKernelTime, dLastUserTime))
    {
        dLastSystemTime = dLastKernelTime+dLastUserTime;
        Sleep(nSleepMilliSecond);
        if (GetCPUTimeState(dCurrentIdleTime, dCurrentKernelTime, dCurrentUserTime))
        {
            dCurrentSystemTime = dCurrentKernelTime+dCurrentUserTime;
            return ((dCurrentSystemTime-dLastSystemTime)-(dCurrentIdleTime-dLastIdleTime))/(dCurrentSystemTime-dLastSystemTime);
        }
    }

    return 0;
#else
    return 0;
#endif
}

bool SystemUtil::GetCPUTimeState(double &dIdleTime, double &dKernelTime, double &dUserTime)
{
#ifdef WIN32
    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;

    if (GetSystemTimes(&idleTime, &kernelTime, &userTime))
    {
        // get idleTime
        UINT64 nTemp = idleTime.dwHighDateTime;
        nTemp = nTemp<<32;
        nTemp = nTemp|idleTime.dwLowDateTime;
        dIdleTime = (double)nTemp;

        // get kernelTime
        nTemp = kernelTime.dwHighDateTime;
        nTemp = nTemp<<32;
        nTemp = nTemp|kernelTime.dwLowDateTime;
        dKernelTime = (double)nTemp;

        // get userTime
        nTemp = userTime.dwHighDateTime;
        nTemp = nTemp<<32;
        nTemp = nTemp|userTime.dwLowDateTime;
        dUserTime = (double)nTemp;

        return true;
    }

    return false;

#else
    return false;
#endif
}
