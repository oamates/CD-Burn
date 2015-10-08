#include "ChnsysThread.h"
#include "ZOSMutex.h"
#include "ZOSCond.h"
#include "CLibThread.h"

OS_MUTEX_HANDLE OS_MUTEX_CreateInstance()
{
    ZOSMutex *pOSMutex = NEW ZOSMutex("CLibMutex");
    if (pOSMutex != NULL)
    {
        return (OS_MUTEX_HANDLE)pOSMutex;
    }

    return NULL;
}

VOID OS_MUTEX_DestroyInstance(OS_MUTEX_HANDLE hOSMutex)
{
    if (hOSMutex != NULL)
    {
        ZOSMutex *pOSMutex = (ZOSMutex *)hOSMutex;
        SAFE_DELETE(pOSMutex);
    }
}

VOID OS_MUTEX_Lock(OS_MUTEX_HANDLE hOSMutex)
{
    if (hOSMutex != NULL)
    {
        ZOSMutex *pOSMutex = (ZOSMutex *)hOSMutex;
        pOSMutex->Lock();
    }
}

VOID OS_MUTEX_Unlock(OS_MUTEX_HANDLE hOSMutex)
{
    if (hOSMutex != NULL)
    {
        ZOSMutex *pOSMutex = (ZOSMutex *)hOSMutex;
        pOSMutex->Unlock();
    }
}

OS_CONDITION_HANDLE OS_CONDITION_CreateInstance()
{
    ZOSCond *pOSCond = NEW ZOSCond;
    if (pOSCond != NULL)
    {
        return (OS_CONDITION_HANDLE)pOSCond;
    }

    return NULL;
}

VOID OS_CONDITION_DestroyInstance(OS_CONDITION_HANDLE hOSCondition)
{
    if (hOSCondition != NULL)
    {
        ZOSCond *pOSCond = (ZOSCond *)hOSCondition;
        SAFE_DELETE(pOSCond);
    }
}

VOID OS_CONDITION_Wait(OS_CONDITION_HANDLE hOSCondition, OS_MUTEX_HANDLE hOSMutex, CHNSYS_INT nTimeoutMilliSeconds)
{
    if (
        (hOSCondition != NULL)
        && (hOSMutex != NULL)
        )
    {
        ZOSCond *pOSCond = (ZOSCond *)hOSCondition;
        pOSCond->Wait((ZOSMutex*)hOSMutex, nTimeoutMilliSeconds);
    }
}

VOID OS_CONDITION_Signal(OS_CONDITION_HANDLE hOSCondition)
{
    if (hOSCondition != NULL)
    {
        ZOSCond *pOSCond = (ZOSCond *)hOSCondition;
        pOSCond->Signal();
    }
}

OS_THREAD_HANDLE OS_THREAD_CreateInstance()
{
    CLibThread *pCLibThread = NEW CLibThread("CLibThread");
    if (pCLibThread != NULL)
    {
        return (OS_THREAD_HANDLE)pCLibThread;
    }

    return NULL;
}

VOID OS_THREAD_DestroyInstance(OS_THREAD_HANDLE hOSThread)
{
    if (hOSThread != NULL)
    {
        CLibThread *pCLibThread = (CLibThread *)hOSThread;
        SAFE_DELETE(pCLibThread);
    }
}

VOID OS_THREAD_SetCallBack(OS_THREAD_HANDLE hOSThread, OS_THREAD_CallBackFunc cbf, VOID *pContext)
{
    if (hOSThread != NULL)
    {
        CLibThread *pCLibThread = (CLibThread *)hOSThread;
        pCLibThread->SetCallBack(cbf, pContext);
    }
}

CHNSYS_BOOL OS_THREAD_Start(OS_THREAD_HANDLE hOSThread)
{
    if (hOSThread != NULL)
    {
        CLibThread *pCLibThread = (CLibThread *)hOSThread;
        return pCLibThread->Start();
    }

    return FALSE;
}

CHNSYS_BOOL OS_THREAD_Stop(OS_THREAD_HANDLE hOSThread, CHNSYS_BOOL bWait)
{
    if (hOSThread != NULL)
    {
        CLibThread *pCLibThread = (CLibThread *)hOSThread;
        return pCLibThread->Stop(bWait);
    }

    return FALSE;
}

