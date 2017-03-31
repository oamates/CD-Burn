#ifndef _CHNSYSUTIL_H_
#define _CHNSYSUTIL_H_

#include <stdlib.h>
#include "ChnsysTypes.h"

CHNSYS_UINT64 OS_UTIL_Milliseconds();
CHNSYS_UINT64 OS_UTIL_Microseconds();

VOID OS_UTIL_SleepMilliseconds(CHNSYS_INT nMilliSecond);
VOID OS_UTIL_SleepMicroseconds(CHNSYS_INT nMicroSecond);

CHNSYS_CHAR *OS_UTIL_Lower(CHNSYS_CHAR *sValue);
CHNSYS_CHAR *OS_UTIL_Upper(CHNSYS_CHAR *sValue);

CHNSYS_INT OS_UTIL_CheckDir(CONST CHNSYS_CHAR *sDir);
CHNSYS_INT OS_UTIL_RecursiveMakeDir(CONST CHNSYS_CHAR *sDir);

CHNSYS_BOOL OS_UTIL_DecodeURL(CHNSYS_CHAR *sURL, CHNSYS_CHAR **sProtocol,
                              CHNSYS_CHAR **sHost = NULL, CHNSYS_CHAR **sPort = NULL,
                              CHNSYS_CHAR **sUser = NULL, CHNSYS_CHAR **sPass = NULL,
                              CHNSYS_CHAR **sPath = NULL);

#endif //_CHNSYSUTIL_H_
