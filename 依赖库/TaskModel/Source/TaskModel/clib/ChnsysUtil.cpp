#include "ChnsysUtil.h"
#include "ZOS.h"
#include "ZOSThread.h"

CHNSYS_UINT64 OS_UTIL_Milliseconds()
{
    return ZOS::milliseconds();
}

CHNSYS_UINT64 OS_UTIL_Microseconds()
{
    return ZOS::microseconds();
}

VOID OS_UTIL_SleepMilliseconds(CHNSYS_INT nMilliSecond)
{
    ZOSThread::Sleep(nMilliSecond);
}

VOID OS_UTIL_SleepMicroseconds(CHNSYS_INT nMicroSecond)
{
    ZOS::sleepmicrosec(nMicroSecond);
}

CHNSYS_CHAR *OS_UTIL_Lower(CHNSYS_CHAR *sValue)
{
    if (sValue != NULL)
    {
        return ZOS::lower(sValue);
    }
    return NULL;
}

CHNSYS_CHAR *OS_UTIL_Upper(CHNSYS_CHAR *sValue)
{
    if (sValue != NULL)
    {
         return ZOS::upper(sValue);
    }
   return NULL;
}

CHNSYS_INT OS_UTIL_CheckDir(CONST CHNSYS_CHAR *sDir)
{
    if (sDir != NULL)
    {
        return ZOS::checkdir(sDir);
    }
    return -1;
}

CHNSYS_INT OS_UTIL_RecursiveMakeDir(CONST CHNSYS_CHAR *sDir)
{
    if (sDir != NULL)
    {
        return ZOS::recursivemakedir(sDir);
    }
    return -1;
}

CHNSYS_BOOL OS_UTIL_DecodeURL(CHNSYS_CHAR *sURL, CHNSYS_CHAR **sProtocol,
                              CHNSYS_CHAR **sHost, CHNSYS_CHAR **sPort,
                              CHNSYS_CHAR **sUser, CHNSYS_CHAR **sPass,
                              CHNSYS_CHAR **sPath)
{
    if(sProtocol != NULL )
    {
        return ZOS::DecodeURL(sURL,sProtocol,sHost,sPort,sUser,sPass,sPath);
    }
    return FALSE;
}
