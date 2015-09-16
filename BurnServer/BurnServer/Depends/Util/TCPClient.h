#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

class TCPClient
{
public:
    /**
     *
     * tcp short connect communication process:
     * 1 connect, until success or nConnectWaitMilliSecond timeout
     * 2 send, until send all or nCommunicateWaitMilliSecond timeout.
     * 3 receive, first receive wait until nCommunicateWaitMilliSecond timeout, 
     *   subsequent receive not wait, until receive timeout(receive = -1)
     * 4 close
     *
     * return -1; parameter is not right
     * return -2; connect failed
     * return -3; send failed
     * return -4; send or receive timeout. 
     * return -5; receive error. the tcp session is abnormal.
     * return -6: socket not init
     * return -7: memory allocate failed
     */
    int DoShortConnectCommunicate(const char *sIP, int nPort, const char *sSendData, int nSendData,
        int nConnectWaitMilliSecond = 3000, int nCommunicateWaitMilliSecond = 5000);

    char *GetReceiveData();
    
    int GetReceiveDataCount();

    /**
     * process:
     * 1 connect, until success or nConnectWaitMilliSecond timeout
     * 2 send, until send all or nSendWaitMilliSecond timeout.
     * 3 close
     * 
     * return -1; parameter is not right
     * return -2; connect failed
     * return -3; send failed
     * return -4; send timeout. 
     * return -5: socket not init
     */
    static int DoTcpOnlySend(const char *sIP, int nPort, const char *sSendData, int nSendData, 
        int nConnectWaitMilliSecond = 3000, int nSendWaitMilliSecond = 1000);

protected:
    int EnsureReceiveBufferEnough();
public:
    TCPClient();
    ~TCPClient();
private:
    char                *m_pReceiveData;
    int                 m_nReceiveDataCount;
    int                 m_nReceiveBufferSize;
    static const int    msc_nEachTimeSendBytes;
    static const int    msc_nEachTimeRecvBytes;
    static const int    msc_nInitAllocateCount;
};

#endif //_TCP_CLIENT_H_
