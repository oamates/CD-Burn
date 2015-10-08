#include "RemoteRTSPClient.h"
#include "ZRTSPSession.h"

RemoteRTSPClient::RemoteRTSPClient()
: m_Socket()
, m_pRTSPSession(NULL)
, m_pRTSPServerListener(NULL)
{

}

RemoteRTSPClient::~RemoteRTSPClient()
{
    //
}

BOOL RemoteRTSPClient::Create(const int nSocket,const struct sockaddr_in* pRemoteAddr)
{
    BOOL bReturn = FALSE;

    m_Socket.Attach(nSocket, pRemoteAddr);
    m_pRTSPSession = (ZRTSPSession *)ZRTSPSession::CreateSession(SESSION_TYPE_SERVER, &m_Socket, NULL, 
        RTSP_PROTOCOL_TRANSPORT_UDP, NULL, NULL);
    if (m_pRTSPSession != NULL)
    {
        m_Socket.StreamCreate();
        m_Socket.SetNonBlocking();
        m_Socket.SetNoDelay();
        m_Socket.SetKeepAlive();
        m_Socket.SetSendBufferSize(DEFAULT_TCP_SEND_BUFFER_SIZE);
        m_Socket.SetRecvBufferSize(DEFAULT_TCP_RECV_BUFFER_SIZE);
        m_Socket.SetTimeOut(DEFAULT_TCP_SEND_TIMEOUT,DEFAULT_TCP_RECV_TIMEOUT);
        m_pRTSPSession->SetRTSPServerListener(m_pRTSPServerListener);
        

        bReturn = TRUE;
    }

    return bReturn;
}

BOOL RemoteRTSPClient::Start()
{
    m_Socket.RequestEvent(ZEvent::EVENT_READ);
    return TRUE;
}

VOID RemoteRTSPClient::SetRTSPProtocolListener(RTSPServerListener *pListener)
{
    m_pRTSPServerListener = pListener;
}

BOOL RemoteRTSPClient::Close()
{
    if (m_pRTSPSession != NULL)
    {
        m_Socket.StreamClose();
        m_Socket.SetTask(NULL);
        ZSession::CloseInstance(m_pRTSPSession);
        m_Socket.RemoveEvent(ZEvent::EVENT_READ);
        m_Socket.Close();
        m_pRTSPSession = NULL;
    }

    return TRUE;
}

BOOL RemoteRTSPClient::IsMe(ZRTSPSession *pRTSPSession)
{
    if (m_pRTSPSession == pRTSPSession)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL RemoteRTSPClient::IsValid()
{
    if (m_pRTSPSession->IsLiveSession())
    {
        return TRUE;
    }
    return FALSE;
}

CHAR *RemoteRTSPClient::GetRemoteIP()
{
    if (m_pRTSPSession != NULL)
    {
        return ZSocket::ConvertAddr(m_pRTSPSession->GetSessionAddr());
    }
    return NULL;
}

UINT RemoteRTSPClient::GetRemotePort()
{
    if (m_pRTSPSession != NULL)
    {
        return m_pRTSPSession->GetSessionPort();
    }

    return 0;
}

ZRTSPSession *RemoteRTSPClient::GetRTSPSession()
{
    return m_pRTSPSession;
}
