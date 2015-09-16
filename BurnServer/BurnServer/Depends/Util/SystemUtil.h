#ifndef _SYSTEM_UTIL_H_
#define _SYSTEM_UTIL_H_

#ifndef UINT64
#ifdef WIN32
typedef unsigned __int64    UINT64;
#else
typedef unsigned long long	UINT64;
#endif
#endif//UINT64

#ifndef INT64
#ifdef WIN32
typedef __int64				INT64;
#else
typedef signed long long	INT64;
#endif
#endif//INT64

class SystemUtil
{
public:
    static UINT64 GetMemoryTotalSize();
    static UINT64 GetMemoryFreeSize();
    static double GetCPUUtilization(int nSleepMilliSecond = 1000);
private:
    static bool GetCPUTimeState(double &dIdleTime, double &dKernelTime, double &dUserTime);
private:
    SystemUtil();
    ~SystemUtil();
};

#endif//_SYSTEM_UTIL_H_
