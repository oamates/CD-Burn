#ifndef _NCXSERVER_H_
#define _NCXSERVER_H_

#include "TaskModel.h"
#include "NCX.h"
#include "ClientProcessor.h"
#include <vector>

class NCXServer
    : public ZOSThread
    , public ClientProcessorListener
{
public:
    void SetListenPort(int nPort);
    void SetClientProcessorCount(int nCount);
    void SetOuterMostTagName(const char *sTagName);
    int SetCallBack(NCXEventCallBack ncxEventCallBack, void *pContext);
    BOOL Create();
    BOOL Close();
    int SendProtocolResponse(NCXServerCBParam ncxServerCBParam, const char *sResponse, int nResponseLength);
protected:
    int StartClientProcessorThreadPool();
    int StopClientProcessorThreadPool();
    virtual BOOL OnThreadStart();
    virtual BOOL OnThreadEntry();
    virtual BOOL OnThreadStop();
    bool PorcessAccept();

    virtual BOOL OnReceiveCompleteProtocol(ClientProcessor *pClientProcessor, const char *sProtocol, int nProtocolLength);
public:
    NCXServer();
    virtual ~NCXServer();
private:
    int                 m_nListenPort;
    ZTCPSocket          m_tcpSocketListen;

    int                 m_nClientProcessorCount;

    char                m_sOuterMostTagName[MAX_TAGNAME_LENGTH];

    NCXEventCallBack    m_cbFunction;
    void                *m_pContext;

    std::vector<ClientProcessor*>       m_vectClientProcessor;
    ZOSMutex                            m_mutexClientProcessorVect;
};

#endif //_NCXSERVER_H_
//////////////////////////////////////////////////////////////////////////
