#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_

#include "TaskModel.h"

typedef int (*UdpServerCallBackFunc)(char *sRemoteIP, int nRemotePort, char *sData, int nData, void *pContext);

class UDPServer
    : public ZOSThread
{
public:
    /**
     * return -1; bind port failed, may be port is being used;
     * return -2; socket not init
     */
    int Start(int nBindPort, UdpServerCallBackFunc cbf, void *pContext);
    int Stop();
    int GetBindPort();
    /**
     *
     * 
     * return -1; send error;
     * return -2; socket not init
     */
    int Send(const char *sIP, int nPort, const char *sData, int nData);
private:
    virtual BOOL OnThreadStart();
    virtual BOOL OnThreadEntry();
    virtual BOOL OnThreadStop();
    int DoReceive();
public:
    UDPServer();
    ~UDPServer();
private:
    ZUDPSocket              m_udpSocket;
    UdpServerCallBackFunc   m_CallBackFunc;
    void                    *m_pContext;
    int                     m_nBindPort;

    static const int        msc_nUDPBufferSize;
};

#endif //_UDP_SERVER_H_
