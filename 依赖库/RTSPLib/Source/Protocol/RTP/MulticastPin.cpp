#include "MulticastPin.h"

MulticastPin::MulticastPin()
: ZTask("MulticastTask")
, m_rtpSession(SESSION_TYPE_SERVER)
, m_nMulticastIP(0)
, m_nMulticastPort(0)
{
    m_rtpSession.Create();
}

MulticastPin::~MulticastPin()
{
    m_rtpSession.Close();
}

void MulticastPin::SetIPPort(char *sMulticastIP, int nMulticastPort)
{
    m_nMulticastIP = ZSocket::ConvertAddr(sMulticastIP);
    m_nMulticastPort = nMulticastPort;
}

BOOL MulticastPin::AddStream(ZDataPin *pDataPin, int nStreamNo, int nRtpSize, int nMulticastPort)
{
    if (nMulticastPort == 0)
    {
        return m_rtpSession.AddServerStreamMulticast(ZSocket::ConvertAddr(m_nMulticastIP), 
            m_nMulticastPort+2*nStreamNo, m_nMulticastPort+2*nStreamNo+1,nRtpSize,
            pDataPin);
    }
    else
    {
        return m_rtpSession.AddServerStreamMulticast(ZSocket::ConvertAddr(m_nMulticastIP), 
            nMulticastPort, nMulticastPort+1,nRtpSize,
            pDataPin);
    }
}

BOOL MulticastPin::Start()
{
    ZTask::Create();
    ZTask::AddEvent(ZTask::TASK_START_EVENT);

    return m_rtpSession.Play();
}

BOOL MulticastPin::Stop()
{
    m_rtpSession.Close();
    ZTask::Close();

    return TRUE;
}

ZDataPin* MulticastPin::GetDataPin(int nStreamNo)
{
    return m_rtpSession.GetDataPin(nStreamNo);
}

int MulticastPin::Run(int nEvent)
{
    int		nTaskTime	= 1000;
    UINT	nLocalEvent	= 0;

    nLocalEvent	= GetEvent(nEvent);
    ZTask::Run(nLocalEvent);

    if (nLocalEvent&TASK_UPDATE_EVENT)
    {
        if (m_rtpSession.GetSessionState() == SESSION_STATE_PLAY)
        {
            m_rtpSession.RefreshTimeout();
        }
    }

    return nTaskTime;
}


