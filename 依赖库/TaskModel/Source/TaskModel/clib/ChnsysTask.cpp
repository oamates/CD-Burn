#include "ChnsysTask.h"
#include "CLibTask.h"
#include "ZSocket.h"

VOID TASK_POOL_Init(CHNSYS_INT nTotalCount, CHNSYS_INT nExclusiveCount)
{
    ZOS::Initialize();
    ZOSThread::Initialize();
    ZIdleTask::Initialize();
    ZTaskThreadPool::Initialize(nTotalCount, nExclusiveCount);
    ZIdleTask::StartIdleTaskThread();
    ZTaskThreadPool::StartTaskThread();
    ZSocket::StartEventThread();
}

VOID TASK_POOL_Uninit()
{
    ZSocket::StopEventThread();
    ZIdleTask::StopIdleTaskThread();
    ZTaskThreadPool::StopTaskThread();
    ZTaskThreadPool::Uninitialize();
    ZIdleTask::Uninitialize();
    ZOSThread::Uninitialize();
    ZOS::Uninitialize();
}

TASK_TASK_HANDLE TASK_TASK_CreateInstance(CHNSYS_INT nThreadBeginNo, CHNSYS_INT nThreadEndNo)
{
    CLibTask *pCLibTask = NEW CLibTask;
    if (
        (nThreadBeginNo > 0)
        && (nThreadEndNo > 0)
        && (nThreadEndNo > nThreadBeginNo)
        )
    {
        pCLibTask->SetTaskThreadBegin(nThreadBeginNo);
        pCLibTask->SetTaskThreadBound(nThreadEndNo);
    }
    return (TASK_TASK_HANDLE)pCLibTask;
}

VOID TASK_TASK_DestroyInstance(TASK_TASK_HANDLE hTask)
{
    if (hTask != NULL)
    {
        CLibTask *pCLibTask = (CLibTask *)hTask;
        SAFE_DELETE(pCLibTask);
    }
}

VOID TASK_TASK_SetCallBack(TASK_TASK_HANDLE hTask, TASK_TASK_CallBackFunc cbf, VOID* pContext)
{
    if (hTask != NULL)
    {
        CLibTask *pCLibTask = (CLibTask *)hTask;
        pCLibTask->SetCallBack(cbf, pContext);
    }
}

CHNSYS_BOOL TASK_TASK_Create(TASK_TASK_HANDLE hTask)
{
    if (hTask != NULL)
    {
        CLibTask *pCLibTask = (CLibTask *)hTask;
        return pCLibTask->Create();
    }

    return FALSE;
}

CHNSYS_BOOL TASK_TASK_Close(TASK_TASK_HANDLE hTask)
{
    if (hTask != NULL)
    {
        CLibTask *pCLibTask = (CLibTask *)hTask;
        return pCLibTask->Close();
    }

    return FALSE;
}

CHNSYS_BOOL TASK_TASK_AddEvent(TASK_TASK_HANDLE hTask, CHNSYS_INT nTaskEvent)
{
    if (hTask != NULL)
    {
        CLibTask *pCLibTask = (CLibTask *)hTask;
        return pCLibTask->AddEvent((ZTask::TASK_EVENT)nTaskEvent);
    }

    return FALSE;
}

CHNSYS_BOOL TASK_TASK_AddIdleTask(TASK_TASK_HANDLE hTask, CHNSYS_INT nMilliSec)
{
    if (hTask != NULL)
    {
        CLibTask *pCLibTask = (CLibTask *)hTask;
        return pCLibTask->AddIdleTask(nMilliSec);
    }

    return FALSE;
}

CHNSYS_BOOL TASK_TASK_RemoveIdleTask(TASK_TASK_HANDLE hTask)
{
    if (hTask != NULL)
    {
        CLibTask *pCLibTask = (CLibTask *)hTask;
        return pCLibTask->RemoveIdleTask();
    }

    return FALSE;
}

CHNSYS_BOOL TASK_TASK_SetTimeout(TASK_TASK_HANDLE hTask, CHNSYS_INT nMilliSec)
{
    if (hTask != NULL)
    {
        CLibTask *pCLibTask = (CLibTask *)hTask;
        return pCLibTask->SetTimeout(nMilliSec);
    }

    return FALSE;
}

CHNSYS_BOOL TASK_TASK_RefreshTimeout(TASK_TASK_HANDLE hTask)
{
    if (hTask != NULL)
    {
        CLibTask *pCLibTask = (CLibTask *)hTask;
        return pCLibTask->RefreshTimeout();
    }

    return FALSE;
}

CHNSYS_BOOL OS_SOCKET_SetTask(OS_SOCKET_HANDLE hOSSocket, TASK_TASK_HANDLE hTask)
{
    if (
        (hOSSocket != NULL)
        && (hTask != NULL)
        )
    {
        ZSocket *pSocket = (ZSocket *)hOSSocket;
        ZTask *pTask = (ZTask *)hTask;

        pSocket->SetTask(pTask);
        return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_SOCKET_RequestEvent(OS_SOCKET_HANDLE hOSSocket, CHNSYS_INT nEventType)
{
    if (hOSSocket != NULL)
    {
        ZSocket *pSocket = (ZSocket *)hOSSocket;
        return pSocket->RequestEvent(nEventType);
    }

    return FALSE;
}

