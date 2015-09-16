#include "UDPServer.h"

const int UDPServer::msc_nUDPBufferSize = 64*1024;

UDPServer::UDPServer()
: m_udpSocket()
, m_CallBackFunc(NULL)
, m_pContext(NULL)
, m_nBindPort(0)
{
    //
}

UDPServer::~UDPServer()
{
    //
}

int UDPServer::Start(int nBindPort, UdpServerCallBackFunc cbf, void *pContext)
{
    int nReturn;

    if (m_udpSocket.Create())
    {
        if (m_udpSocket.Bind(0, nBindPort))
        {
            m_nBindPort = nBindPort;
            m_CallBackFunc = cbf;
            m_pContext = pContext;
            nReturn = 0;
            ZOSThread::Start();
        }
        else
        {
            nReturn = -1;
        }
    }
    else
    {
        nReturn = -2;
    }

    return nReturn;
}

int UDPServer::Stop()
{
    m_udpSocket.Close();
    ZOSThread::Stop();

    return 0;
}

int UDPServer::GetBindPort()
{
    return m_nBindPort;
}

int UDPServer::Send(const char *sIP, int nPort, const char *sData, int nData)
{
    int     nBroadcast = 1;
    if ((int)m_udpSocket.GetSocket() != INVALID_FILE_HANDLE)
    {
        setsockopt(m_udpSocket.GetHandle(), SOL_SOCKET, SO_BROADCAST, (char *)&nBroadcast, sizeof(int));
        return m_udpSocket.SendTo(sData, nData, ZSocket::ConvertAddr(sIP), nPort);
    }
    else
    {
        return -2;
    }
}

BOOL UDPServer::OnThreadStart()
{
    return TRUE;
}

BOOL UDPServer::OnThreadEntry()
{
    if (!m_bStop)
    {
        DoReceive();

        ZOSThread::Sleep(1);
    }

    return TRUE;
}

BOOL UDPServer::OnThreadStop()
{
    return TRUE;
}

int UDPServer::DoReceive()
{
    UINT    nRemoteAddr;
    UINT    nRemotePort;
    char    sData[msc_nUDPBufferSize];
    int     nData = msc_nUDPBufferSize;

    nData = m_udpSocket.RecvFrom(sData, nData, &nRemoteAddr, &nRemotePort);
    if (nData > 0)
    {
        if (m_CallBackFunc != NULL)
        {
            m_CallBackFunc(ZSocket::ConvertAddr(nRemoteAddr), nRemotePort, sData, nData, m_pContext);
        }
    }

    return 0;
}
