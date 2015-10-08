#include "CLibTCPListenerEvent.h"

CLibTCPListenerEvent::CLibTCPListenerEvent()
{
    m_funcCallBack = NULL;
    m_pCallBackContext = NULL;
}

CLibTCPListenerEvent::~CLibTCPListenerEvent()
{

}

BOOL CLibTCPListenerEvent::OnListenerEvent(const int hAccept,const struct sockaddr_in* pLocalAddr,const struct sockaddr_in* pRemoteAddr)
{
    if (m_funcCallBack != NULL)
    {
        return m_funcCallBack(hAccept, ntohl(pLocalAddr->sin_addr.s_addr), 
            ntohs(pLocalAddr->sin_port),
            ntohl(pRemoteAddr->sin_addr.s_addr),
            ntohs(pRemoteAddr->sin_port),
            m_pCallBackContext);
    }
    return FALSE;
}

void CLibTCPListenerEvent::SetCallBack(TCPListenerEvent_CallBackFunc cbf, void *pContext)
{
    m_funcCallBack = cbf;
    m_pCallBackContext = pContext;
}
