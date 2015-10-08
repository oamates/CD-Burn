#ifndef _RTSPCLIENTLISTENER_H_
#define _RTSPCLIENTLISTENER_H_

#include "Common.h"

class RTSPClientListener
{
public:
    virtual BOOL OnReceiveResponse_OPTION(int nState);
    virtual BOOL OnReceiveResponse_DESCRIBE(int nState, CHAR *sSDP, int nSDP);
    virtual BOOL OnReceiveResponse_SETUP(int nState, int nStreamNo);
    virtual BOOL OnReceiveResponse_PLAY(int nState);

    virtual BOOL DisconnectWithServer();
    virtual BOOL BeforeReconnect();
    virtual BOOL AfterReconnect(BOOL bReconnectOK);
    virtual BOOL GiveupReconnect();

    virtual BOOL DetectPacketLost(int nLostCount);
public:
    RTSPClientListener();
    virtual ~RTSPClientListener();
};

#endif //_RTSPCLIENTLISTENER_H_
