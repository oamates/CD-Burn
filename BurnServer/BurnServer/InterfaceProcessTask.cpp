#include "InterfaceProcessTask.h"
#include "InterfaceProtocol.h"

InterfaceProcessTask::InterfaceProcessTask()
: ZOSThread("InterfaceProcessTask")
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
    if (ZOSThread::Start())
    {
        return TRUE;
    }

    return FALSE;
}

BOOL InterfaceProcessTask::Close()
{
    int i = 0;
    BOOL bReturn = FALSE;

    if (ZOSThread::Stop(TRUE))
    {
        bReturn=TRUE;
    }

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

    return bReturn;
}

BOOL InterfaceProcessTask::OnThreadStart()
{
    return TRUE;
}

BOOL InterfaceProcessTask::OnThreadEntry()
{
    if (!m_bStop)
    {
        ProcessRequest();

        ZOSThread::Sleep(10);
    }
    
    return TRUE;
}

BOOL InterfaceProcessTask::OnThreadStop()
{
    return TRUE;
}

BOOL InterfaceProcessTask::ProcessRequest()
{
    NCXServerCBParam    *pCBParam;
    ZOSMutexLocker      locker(&m_mutexArrayCBParam);

    if (m_arrayCBParam.Size() > 0)
    {
        pCBParam = m_arrayCBParam[0];
        if (pCBParam != NULL)
        {
            m_arrayCBParam.Remove(0);
            locker.Unlock();
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
