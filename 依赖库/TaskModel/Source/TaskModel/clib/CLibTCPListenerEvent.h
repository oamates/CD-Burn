#ifndef _CLIBTCPLISTENEREVENT_H_
#define _CLIBTCPLISTENEREVENT_H_

#include "ZTCPListenerSocket.h"

typedef BOOL (*TCPListenerEvent_CallBackFunc)(INT hAccept, UINT nLocalAddr, UINT nLocalPort, UINT nRemoteAddr, UINT nRemotePort, VOID *pContext);

class CLibTCPListenerEvent : public ZTCPListenerEvent
{
public:
    CLibTCPListenerEvent();
    virtual ~CLibTCPListenerEvent();
public:
    virtual	BOOL	OnListenerEvent(const int hAccept,const struct sockaddr_in* pLocalAddr,const struct sockaddr_in* pRemoteAddr);
    void SetCallBack(TCPListenerEvent_CallBackFunc cbf, void *pContext);
private:
    TCPListenerEvent_CallBackFunc       m_funcCallBack;
    void*                               m_pCallBackContext;
};

#endif //_CLIBTCPLISTENEREVENT_H_
