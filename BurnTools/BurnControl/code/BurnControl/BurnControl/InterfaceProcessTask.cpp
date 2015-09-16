#include "stdafx.h"

#include "InterfaceProcessTask.h"
#include "InterfaceProtocol.h"

InterfaceProcessTask::InterfaceProcessTask()
: ZTask("InterfaceProcessTask")
, m_arrayCBParam(100)
, m_mutexArrayCBParam("MutexArrayCBParam")
, m_hNCXServer(NULL)
{
}

InterfaceProcessTask::~InterfaceProcessTask()
{
    Close();
}

void InterfaceProcessTask::SetNCXServerHandle(NCXSERVERHANDLE hNCXServer)
{
    m_hNCXServer = hNCXServer;
}

BOOL InterfaceProcessTask::AddRequest(NCXServerCBParam * pNCXServerCBParam)
{
    NCXServerCBParam    *pParam = NULL;
    BOOL                bReturn = FALSE;
    ZOSMutexLocker  locker(&m_mutexArrayCBParam);

    if (pNCXServerCBParam != NULL)
    {
        pParam = NEW NCXServerCBParam;
        if (pParam != NULL)
        {
            strncpy(pParam->szRemoteIP, pNCXServerCBParam->szRemoteIP, 16);
            pParam->szRemoteIP[15] = '\0';
            pParam->pClientProcessor = pNCXServerCBParam->pClientProcessor;
            pParam->sProtocolContent = NEW char [pNCXServerCBParam->nProtocolLength+1];
            if (pParam->sProtocolContent != NULL)
            {
                strncpy(pParam->sProtocolContent, pNCXServerCBParam->sProtocolContent, pNCXServerCBParam->nProtocolLength+1);
                pParam->sProtocolContent[pNCXServerCBParam->nProtocolLength] = '\0';
                pParam->nProtocolLength = pNCXServerCBParam->nProtocolLength;

                m_arrayCBParam.Add(pParam);
                AddEvent(ZTask::TASK_UPDATE_EVENT);
                bReturn = TRUE;
            }
        }
    }

    if (!bReturn)
    {
        if (pParam!=NULL)
        {
            if (pParam->sProtocolContent != NULL)
            {
                SAFE_DELETE(pParam->sProtocolContent);
            }
            DEL pParam;
            pParam = NULL;
        }
    }

    return bReturn;
}

BOOL InterfaceProcessTask::Create()
{
    if (ZTask::Create() == 0)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL InterfaceProcessTask::Close()
{
    int i = 0;

    if (ZTask::Close() == 0)
    {
        for (i = 0; i < m_arrayCBParam.Size(); i ++)
        {
            if (m_arrayCBParam[i] != NULL)
            {
                if (m_arrayCBParam[i]->sProtocolContent != NULL)
                {
                    SAFE_DELETE(m_arrayCBParam[i]->sProtocolContent);
                }
                DEL m_arrayCBParam[i];
            }
        }
        m_arrayCBParam.RemoveAll();

        return TRUE;
    }

    return FALSE;
}

int InterfaceProcessTask::Run(int nEvent)
{
    int         nTaskTime   = 1000;
    UINT        nLocalEvent = 0;

    nLocalEvent = GetEvent(nEvent);

    if (nLocalEvent&TASK_KILL_EVENT)
    {
        //
    }
    if (nLocalEvent&TASK_TIMEOUT_EVENT)
    {
        //
    }
    if (nLocalEvent&TASK_START_EVENT)
    {
        //
    }
    if (nLocalEvent&TASK_READ_EVENT)
    {
        //
    }
    if (nLocalEvent&TASK_UPDATE_EVENT)
    {
        if (ProcessRequest())
        {
            nTaskTime = 5;
        }
    }

    return nTaskTime;
}

BOOL InterfaceProcessTask::ProcessRequest()
{
    NCXServerCBParam    *pCBParam;
    ZOSMutexLocker      locker(&m_mutexArrayCBParam);

    if (m_arrayCBParam.Size() > 0)
    {
        pCBParam = m_arrayCBParam[0];
        m_arrayCBParam.Remove(0);
        locker.Unlock();
        if (pCBParam != NULL)
        {
            CInterfaceProtocol::OnRequest(m_hNCXServer, pCBParam);
            if (pCBParam->sProtocolContent != NULL)
            {
                SAFE_DELETE(pCBParam->sProtocolContent);
            }
            DEL pCBParam;
            return TRUE;
        }
    }

    return FALSE;
}
