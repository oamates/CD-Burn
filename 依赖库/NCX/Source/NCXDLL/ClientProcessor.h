#ifndef _CLIENTPROCESSOR_H_
#define _CLIENTPROCESSOR_H_

#include "TaskModel.h"
#include "TagUtillity.h"

class ClientProcessor;

//////////////////////////////////////////////////////////////////////////
class ClientProcessorListener
{
public:
    virtual BOOL OnReceiveCompleteProtocol(ClientProcessor *pClientProcessor, const char *sProtocol, int nProtocolLength);
public:
    ClientProcessorListener();
    virtual ~ClientProcessorListener();
};
//////////////////////////////////////////////////////////////////////////
class ClientProcessor
    : public ZOSThread
{
public:
    ClientProcessor();
    virtual ~ClientProcessor();
public:
    bool Create();
    bool Close(BOOL bWait = TRUE);
    void SetListener(ClientProcessorListener *pListener);
    void SetOuterMostTagName(const char *sTagName);
    bool IsBeUsed();
    int SetClient(int nClientSocket, UINT nClientAddr, UINT nClientPort);
    int SendRespond(const char *sProtocol, int nProtocolLength);
    char* GetRemoteIP();
protected:
    virtual BOOL OnThreadStart();
    virtual BOOL OnThreadEntry();
    virtual BOOL OnThreadStop();
    void ProcessReceive();
    int EnsureReceiveBufferEnough();
    int ClearSession();
private:
    bool                        m_bIsBeUsed;
    ClientProcessorListener     *m_pClientProcessorListener;
    char                        m_sOuterMostTagName[MAX_TAGNAME_LENGTH];

    ZTCPSocket                  m_tcpSocket;
    bool                        m_bGetProtocol;

    char                        *m_pReceiveData;
    int                         m_nReceiveDataCount;
    int                         m_nReceiveBufferSize;
    static const int            msc_nSendWaitMilliSecond;
    static const int            msc_nEachTimeSendBytes;
    static const int            msc_nEachTimeRecvBytes;
    static const int            msc_nInitAllocateCount;
    static const int            msc_nSessionTimeoutMilliSecond;

    UINT64                      m_nLastAliveTime;
};

#endif //_CLIENTPROCESSOR_H_
//////////////////////////////////////////////////////////////////////////
