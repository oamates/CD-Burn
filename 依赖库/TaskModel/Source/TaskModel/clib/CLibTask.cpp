#include "CLibTask.h"

CLibTask::CLibTask()
: ZTimeoutTask("CLibTask")
{
    m_funcCallBack = NULL;
    m_pCallBackContext = NULL;
}

CLibTask::~CLibTask()
{
    CLibTask::Close();
}

BOOL CLibTask::Create()
{
    ZTimeoutTask::Create();
    return TRUE;
}

BOOL CLibTask::Close()
{
    ZTimeoutTask::Close();
    return TRUE;
}

void CLibTask::SetCallBack(Task_CallBackFunc cbf, void* pContext)
{
    m_funcCallBack = cbf;
    m_pCallBackContext = pContext;
}

int CLibTask::Run(int nEvent)
{
    int         nTaskTime = 0;
    UINT        nLocalEvent = 0;

    nLocalEvent = GetEvent(nEvent);
    ZTimeoutTask::Run(nLocalEvent);

    nTaskTime = m_funcCallBack(nLocalEvent, m_pCallBackContext);

    return nTaskTime;
}
