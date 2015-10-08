#ifndef _REMOTERTSPCLIENT_H_
#define _REMOTERTSPCLIENT_H_

#include "ZTCPSocket.h"

class RTSPServerListener;
class ZRTSPSession;

class RemoteRTSPClient
{
public:
    BOOL Create(const int nSocket,const struct sockaddr_in* pRemoteAddr);
    BOOL Start();
    VOID SetRTSPProtocolListener(RTSPServerListener *pListener);
    BOOL Close();
    BOOL IsMe(ZRTSPSession *pRTSPSession);
    BOOL IsValid();
    CHAR *GetRemoteIP();
    UINT GetRemotePort();
    ZRTSPSession *GetRTSPSession();
public:
    RemoteRTSPClient();
    ~RemoteRTSPClient();
protected:
    ZTCPSocket      m_Socket;
    ZRTSPSession    *m_pRTSPSession;
    RTSPServerListener  *m_pRTSPServerListener;
};


#endif //_REMOTERTSPCLIENT_H_
