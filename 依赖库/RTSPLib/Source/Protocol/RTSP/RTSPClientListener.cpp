#include "RTSPClientListener.h"

RTSPClientListener::RTSPClientListener()
{

}

RTSPClientListener::~RTSPClientListener()
{

}

BOOL RTSPClientListener::OnReceiveResponse_OPTION(int nState)
{
    return TRUE;
}

BOOL RTSPClientListener::OnReceiveResponse_DESCRIBE(int nState, CHAR *sSDP, int nSDP)
{
    return TRUE;
}

BOOL RTSPClientListener::OnReceiveResponse_SETUP(int nState, int nStreamNo)
{
    return TRUE;
}

BOOL RTSPClientListener::OnReceiveResponse_PLAY(int nState)
{
    return TRUE;
}

BOOL RTSPClientListener::DisconnectWithServer()
{
    return TRUE;
}

BOOL RTSPClientListener::BeforeReconnect()
{
    return TRUE;
}

BOOL RTSPClientListener::AfterReconnect(BOOL bReconnectOK)
{
    return TRUE;
}

BOOL RTSPClientListener::GiveupReconnect()
{
    return TRUE;
}

BOOL RTSPClientListener::DetectPacketLost(int nLostCount)
{
    return TRUE;
}
