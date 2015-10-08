#include "RTSPServerListenerManager.h"
#include "RemoteRTSPClient.h"

//////////////////////////////////////////////////////////////////////////
ListenerAgent::ListenerAgent(int hAccept,const struct sockaddr_in* pLocalAddr,const struct sockaddr_in* pRemoteAddr)
:m_hAccept(hAccept)
{
    if(pLocalAddr != NULL)
    {
        memcpy(&m_LocalAddr,pLocalAddr,sizeof(m_LocalAddr));
    }
    if(pRemoteAddr != NULL)
    {
        memcpy(&m_RemoteAddr,pRemoteAddr,sizeof(m_RemoteAddr));
    }
}
ListenerAgent::~ListenerAgent()
{
}
//////////////////////////////////////////////////////////////////////////
RTSPServerListenerManager * RTSPServerListenerManager::m_pInstance = NULL;

RTSPServerListenerManager::RTSPServerListenerManager()
: ZTask("RTSPServerListenerManagerTask")
, m_arrayRemoteRTSPClient(100)
, m_mutexRemoteRTSPClientArray("Mutex_RemoteRTSPClientArray")
, m_Listener()
, m_arrayListenerAgent(100)
, m_ListenerMutex("ListenerMutex")
, m_RequestCallBackFunc(NULL)
, m_prcbfContext(NULL)
{
    m_Listener.Create(0, 554);
    m_Listener.SetListenerEvent(this);
}

RTSPServerListenerManager::~RTSPServerListenerManager()
{
    m_Listener.Close();
}

RTSPServerListenerManager* RTSPServerListenerManager::GetInstance()
{
    if (m_pInstance == NULL)
    {
        m_pInstance = NEW RTSPServerListenerManager;
        m_pInstance->Create();
        m_pInstance->AddEvent(ZTask::TASK_UPDATE_EVENT);
    }

    return m_pInstance;
}

void RTSPServerListenerManager::Initialize()
{
    RTSPServerListenerManager::GetInstance();
}

void RTSPServerListenerManager::Uninitialize()
{
    if (m_pInstance != NULL)
    {
        m_pInstance->Close();
    }
    SAFE_DELETE(m_pInstance);
}

int RTSPServerListenerManager::Run(int nEvent)
{
    int		nTaskTime	= 1000;
    UINT	nLocalEvent	= 0;

    nLocalEvent	= GetEvent(nEvent);

    ZTask::Run(nLocalEvent);

    if (nLocalEvent&TASK_UPDATE_EVENT)
    {
        ListenerAgent   *pListenerAgent = NULL;
        //ZOSHeapElement  *pElement = NULL;

        //get pListenerAgent from array
        {
            ZOSMutexLocker  locker(&m_ListenerMutex);
            //LOG_DEBUG(("[RTSPServerListenerManager::Run] #-count %d\r\n", m_arrayListenerAgent.Count()));
            if (m_arrayListenerAgent.Count() > 0)
            {
			    pListenerAgent = m_arrayListenerAgent[0];
				m_arrayListenerAgent.Remove(0);
            }
        }

        //create RemoteRTSPClient and add to array
        if (pListenerAgent != NULL)
        {
            nTaskTime = 1;
            //LOG_DEBUG(("[RTSPServerListenerManager::Run] before create\r\n"));
            RemoteRTSPClient *pClient = NEW RemoteRTSPClient;
            if (pClient != NULL)
            {
                pClient->SetRTSPProtocolListener(this);
                //LOG_DEBUG(("[RTSPServerListenerManager::Run] --set listener first\r\n"));
                if (pClient->Create(pListenerAgent->m_hAccept, &(pListenerAgent->m_RemoteAddr)))
                {
                    ZOSMutexLocker  locker(&m_mutexRemoteRTSPClientArray);
                    //LOG_DEBUG(("[RTSPServerListenerManager::Run] before add %d\r\n", m_arrayRemoteRTSPClient.Size()));
                    m_arrayRemoteRTSPClient.Add(pClient);
                    //LOG_DEBUG(("[RTSPServerListenerManager::Run] after add\r\n"));
                    locker.Unlock();
                    pClient->Start();
                }
                else
                {
                    ZTCPSocket::TCPClose(pListenerAgent->m_hAccept);
                    LOG_ERROR(("[RTSPServerListenerManager::Run] reject\r\n"));
                }
            }
            SAFE_DELETE(pListenerAgent);
        }

        // check and delete invalid 
        {
            ZOSMutexLocker  locker(&m_mutexRemoteRTSPClientArray);
            for (int i = 0; i < m_arrayRemoteRTSPClient.Count(); i ++)
            {
                if (
                    (m_arrayRemoteRTSPClient[i] != NULL)
                    && (!m_arrayRemoteRTSPClient[i]->IsValid())
                    )
                {
                    RemoteRTSPClient *pRemoteClient = m_arrayRemoteRTSPClient[i];
                    m_arrayRemoteRTSPClient.Remove(i);
                    locker.Unlock();
                    LOG_DEBUG(("[RTSPServerListenerManager::Run] delete remote %s %d\r\n", 
                        pRemoteClient->GetRemoteIP(), pRemoteClient->GetRemotePort()));
                    pRemoteClient->Close();
                    DEL pRemoteClient;
                    nTaskTime = 10;
                    break;
                }
            }
        }
    }

    return nTaskTime;
}

void RTSPServerListenerManager::SetRequestCallBack(RTSP_SERVER_RequestCallBackFunc rcbf, void *pContext)
{
    m_RequestCallBackFunc = rcbf;
    m_prcbfContext = pContext;
}

BOOL RTSPServerListenerManager::OnFirstCommand(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl, BOOL *bOutNumber)
{
    RTSP_SERVER_REQUESTCBDATA rcbd;
    RTSP_SERVER_HANDLE hRTSPServer = NULL;
    RTSPServerSource *pServerSource = NULL;

    //LOG_DEBUG(("[RTSPServerListenerManager::OnFirstCommand] in---\r\n"));
    if (m_RequestCallBackFunc != NULL)
    {
        strncpy(rcbd.sRequestURL, sRequestUrl, 1024);
        rcbd.sRequestURL[1023] = '\0';
        rcbd.phRTSPServer = &hRTSPServer;
        //LOG_DEBUG(("[RTSPServerListenerManager::OnFirstCommand] before callback\r\n"));
        m_RequestCallBackFunc(rcbd);
        //LOG_DEBUG(("[RTSPServerListenerManager::OnFirstCommand] after callback\r\n"));

        // know this ZRTSPSession is bind with which RTSPServerSource
        if (hRTSPServer != NULL)
        {
            ZOSMutexLocker  locker(&m_mutexRemoteRTSPClientArray);
            RemoteRTSPClient *pRemoteClient = GetRemoteRTSPClient(pRTSPSession);
            if (pRemoteClient != NULL)
            {
                DeleteRemoteRTSPClient(pRTSPSession);
                locker.Unlock();
                pServerSource = (RTSPServerSource *)hRTSPServer;
                pRTSPSession->SetRTSPServerListener(pServerSource);
                pServerSource->AddRemoteRTSPClient(pRemoteClient);
                if(pServerSource->GetClientCount() > pServerSource->GetMaxClientCount())
                {
                    *bOutNumber = TRUE;
                }
            }
        }
    }

    return TRUE;
}

RemoteRTSPClient *RTSPServerListenerManager::GetRemoteRTSPClient(ZRTSPSession *pRTSPSession)
{
    int i = 0;
    ZOSMutexLocker  locker(&m_mutexRemoteRTSPClientArray);

    for (i = 0; i < m_arrayRemoteRTSPClient.Count(); i ++)
    {
        if (
            (m_arrayRemoteRTSPClient[i] != NULL)
            && m_arrayRemoteRTSPClient[i]->IsMe(pRTSPSession)
            )
        {
            return m_arrayRemoteRTSPClient[i];
        }
    }

    return NULL;
}

BOOL RTSPServerListenerManager::DeleteRemoteRTSPClient(ZRTSPSession *pRTSPSession)
{
    int i = 0;
    BOOL bReturn = FALSE;
    ZOSMutexLocker  locker(&m_mutexRemoteRTSPClientArray);

    for (i = 0; i < m_arrayRemoteRTSPClient.Count(); i ++)
    {
        if (
            (m_arrayRemoteRTSPClient[i] != NULL)
            && m_arrayRemoteRTSPClient[i]->IsMe(pRTSPSession)
            )
        {
            m_arrayRemoteRTSPClient.Remove(i);
            bReturn = TRUE;
        }
    }

    return bReturn;
}

BOOL RTSPServerListenerManager::OnListenerEvent(const int hAccept,const struct sockaddr_in* pLocalAddr,const struct sockaddr_in* pRemoteAddr)
{
    BOOL bReturn = FALSE;

    if (hAccept > 0)
    {
        //LOG_DEBUG(("[RTSPServerListenerManager::OnListenerEvent] before m_ListenerMutex\r\n"));
        ZOSMutexLocker locker(&m_ListenerMutex);
        //LOG_DEBUG(("[RTSPServerListenerManager::OnListenerEvent] after m_ListenerMutex\r\n"));
        if (m_arrayListenerAgent.Count() < 100)
        {
            //LOG_DEBUG(("[RTSPServerListenerManager::OnListenerEvent] hAccept %d\r\n", hAccept));
            ListenerAgent *pAgent = NEW ListenerAgent(hAccept, pLocalAddr, pRemoteAddr);
            if (pAgent != NULL)
            {
                //LOG_DEBUG(("[RTSPServerListenerManager::OnListenerEvent] -#-insert\r\n"));
                m_arrayListenerAgent.Add(pAgent);

                bReturn = TRUE;
            }
            else
            {
                LOG_ERROR(("[RTSPServerListenerManager::OnListenerEvent] NEW ListenerAgent failed\r\n"));
            }
        }
        else
        {
            LOG_ERROR(("[RTSPServerListenerManager::OnListenerEvent] over 100 %d\r\n", m_arrayListenerAgent.Count()));
        }
    }
    else
    {
        LOG_ERROR(("[RTSPServerListenerManager::OnListenerEvent] hAccept = %d\r\n", hAccept));
    }
    ZTask::AddEvent(TASK_UPDATE_EVENT);

    return bReturn;


}
