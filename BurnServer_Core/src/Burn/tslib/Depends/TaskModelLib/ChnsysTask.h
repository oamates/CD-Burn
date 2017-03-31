#ifndef _CHNSYSTASK_H_
#define _CHNSYSTASK_H_

#include "ChnsysTypes.h"
#include "ChnsysSocket.h"

#define TASK_EVENT_READ         (0x01<<0x00)
#define TASK_EVENT_WRITE        (0x01<<0x01)
#define TASK_EVENT_START        (0x01<<0x02)
#define TASK_EVENT_STOP         (0x01<<0x03)
#define TASK_EVENT_TIMEOUT      (0x01<<0x04)
#define TASK_EVENT_KILL         (0x01<<0x0D)
#define TASK_EVENT_UPDATE       (0x01<<0x0E)
#define TASK_EVENT_IDLE         (0x01<<0x0F)

typedef VOID *TASK_TASK_HANDLE;

VOID TASK_POOL_Init(CHNSYS_INT nTotalCount, CHNSYS_INT nExclusiveCount = 1);
VOID TASK_POOL_Uninit();

typedef CHNSYS_INT (*TASK_TASK_CallBackFunc)(CHNSYS_INT nEventType, VOID* pContext);
TASK_TASK_HANDLE TASK_TASK_CreateInstance(CHNSYS_INT nThreadBeginNo = -1, CHNSYS_INT nThreadEndNo = -1);
VOID TASK_TASK_DestroyInstance(TASK_TASK_HANDLE hTask);
VOID TASK_TASK_SetCallBack(TASK_TASK_HANDLE hTask, TASK_TASK_CallBackFunc cbf, VOID* pContext);
CHNSYS_BOOL TASK_TASK_Create(TASK_TASK_HANDLE hTask);
CHNSYS_BOOL TASK_TASK_Close(TASK_TASK_HANDLE hTask);

CHNSYS_BOOL TASK_TASK_AddEvent(TASK_TASK_HANDLE hTask, CHNSYS_INT nTaskEvent);

CHNSYS_BOOL TASK_TASK_AddIdleTask(TASK_TASK_HANDLE hTask, CHNSYS_INT nMilliSec);
CHNSYS_BOOL TASK_TASK_RemoveIdleTask(TASK_TASK_HANDLE hTask);

CHNSYS_BOOL TASK_TASK_SetTimeout(TASK_TASK_HANDLE hTask, CHNSYS_INT nMilliSec);
CHNSYS_BOOL TASK_TASK_RefreshTimeout(TASK_TASK_HANDLE hTask);

#define SOCKET_EVENT_READ   (1)
#define SOCKeT_EVENT_WRITE  (2)

CHNSYS_BOOL OS_SOCKET_SetTask(OS_SOCKET_HANDLE hOSSocket, TASK_TASK_HANDLE hTask);
CHNSYS_BOOL OS_SOCKET_RequestEvent(OS_SOCKET_HANDLE hOSSocket, CHNSYS_INT nEventType);

#endif //_CHNSYSTASK_H_
