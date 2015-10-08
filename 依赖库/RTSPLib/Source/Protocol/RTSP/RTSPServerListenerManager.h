#ifndef _RTSPSERVERLISTENERMANAGER_H_
#define _RTSPSERVERLISTENERMANAGER_H_

#include "ZOSArray.h"
#include "ZSocket.h"
#include "RTSPServerListener.h"
#include "RTSPServerSource.h"
#include "ZTCPListenerSocket.h"
#include "RemoteRTSPClient.h"

//////////////////////////////////////////////////////////////////////////
class ListenerAgent
{
    ///////////////////////////////////////////////////////////////////////////////
public:
    ListenerAgent(int hAccept,const struct sockaddr_in* pLocalAddr,const struct sockaddr_in* pRemoteAddr);
    virtual	~ListenerAgent();
    ///////////////////////////////////////////////////////////////////////////////
public:
    int             m_hAccept;
    sockaddr_in     m_LocalAddr;
    sockaddr_in     m_RemoteAddr;
    ///////////////////////////////////////////////////////////////////////////////
protected:
public:
    friend class	RTSPServerListenerManager;
    ///////////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////
typedef ZOSArray<RemoteRTSPClient*>     RemoteRTSPClientArray;
typedef ZOSArray<ListenerAgent*>        ListenerAgentArray;

class RTSPServerListenerManager
    : public RTSPServerListener
    , public ZTCPListenerEvent
    , public ZTask
{
public:
    void SetRequestCallBack(RTSP_SERVER_RequestCallBackFunc rcbf, void *pContext);
public:
    virtual BOOL OnFirstCommand(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl, BOOL *bOutNumber);
//////////////////////////////////////////////////////////////////////////
protected:
    RemoteRTSPClient * GetRemoteRTSPClient(ZRTSPSession *pRTSPSession);
    BOOL DeleteRemoteRTSPClient(ZRTSPSession *pRTSPSession);
//////////////////////////////////////////////////////////////////////////
public:
    virtual	BOOL OnListenerEvent(const int hAccept,const struct sockaddr_in* pLocalAddr,const struct sockaddr_in* pRemoteAddr);
public:
    static RTSPServerListenerManager* GetInstance();
    static void Initialize();
    static void Uninitialize();
public:
    RTSPServerListenerManager();
    virtual ~RTSPServerListenerManager();
private:
    static RTSPServerListenerManager    *m_pInstance;
private:
	RemoteRTSPClientArray           m_arrayRemoteRTSPClient;
    ZOSMutex                        m_mutexRemoteRTSPClientArray;
    ZTCPListenerSocket              m_Listener;

    ListenerAgentArray              m_arrayListenerAgent;
    ZOSMutex                        m_ListenerMutex;

protected:
    virtual	int Run(int nEvent);
private:
    RTSP_SERVER_RequestCallBackFunc     m_RequestCallBackFunc;
    void                                *m_prcbfContext;
};

#endif //_RTSPSERVERLISTENERMANAGER_H_
