#ifndef _MULTICASTPIN_H_
#define _MULTICASTPIN_H_

#include "ZTask.h"
#include "ZRTPSession.h"

class MulticastPin
    : public ZTask
{
public:
    void SetIPPort(char *sMulticastIP, int nMulticastPort);
    // if nMulticastPort=0, calculate port use m_nMulticastPort and nStream
    // if nMulticastPort>0, use it.
    BOOL AddStream(ZDataPin *pDataPin, int nStreamNo, int nRtpSize, int nMulticastPort = 0);
    BOOL Start();
    BOOL Stop();
    ZDataPin *GetDataPin(int nStreamNo);
public:
    virtual	int Run(int nEvent = 0);
public:
    MulticastPin();
    ~MulticastPin();
private:
    ZRTPSession     m_rtpSession;
    UINT            m_nMulticastIP;
    UINT            m_nMulticastPort;
};

#endif //_MULTICASTPIN_H_
