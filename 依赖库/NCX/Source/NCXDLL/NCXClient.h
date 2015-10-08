#ifndef _NCXCLIENT_H_
#define _NCXCLIENT_H_

#include "TaskModel.h"
#include "TagUtillity.h"

class NCXClient
{
public:
    NCXClient();
    ~NCXClient();
public:
    BOOL Create();
    BOOL Close();
    void SetOuterMostTagName(const char *sTagName);
    BOOL ConnectServer(const char* sServerIP, int nServerPort, int nWaitSecond);
    //-1 send failed
    //-2 receive wait timeout
    //-3 server close or socket error. but data receive not finish
    int Communicate(const char* sProtocol, int nProtocol, int nWaitSecond);
    char * GetRespondProtocol();
protected:
    int EnsureReceiveBufferEnough();
private:
    ZTCPSocket                  m_tcpClient;

    char                        m_sOuterMostTagName[MAX_TAGNAME_LENGTH];
    char                        *m_pReceiveData;
    int                         m_nReceiveDataCount;
    int                         m_nReceiveBufferSize;

    static const int            msc_nEachTimeSendBytes;
    static const int            msc_nEachTimeRecvBytes;
    static const int            msc_nInitAllocateCount;

    //for debug
    UINT64                      m_nStartTime;
};

#endif //_NCXCLIENT_H_
//////////////////////////////////////////////////////////////////////////
