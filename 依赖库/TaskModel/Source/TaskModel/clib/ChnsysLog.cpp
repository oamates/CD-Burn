#include "ChnsysLog.h"
#include "ZLog.h"
#include "ZOSMemory.h"

CHNSYS_INT OS_LOG_Init()
{
    g_pLog = NEW ZLog;
    return 0;
}

CHNSYS_INT OS_LOG_UnInit()
{
    if(g_pLog != NULL)
    {
        SAFE_DELETE(g_pLog);
    }
    return 0;
}
