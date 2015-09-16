#include "TCPClient.h"
#include "TaskModel.h"

const int TCPClient::msc_nEachTimeSendBytes = 1024;
const int TCPClient::msc_nEachTimeRecvBytes = 1024;
const int TCPClient::msc_nInitAllocateCount = 10*TCPClient::msc_nEachTimeRecvBytes;

TCPClient::TCPClient()
: m_pReceiveData(NULL)
, m_nReceiveDataCount(0)
, m_nReceiveBufferSize(0)
{
}

TCPClient::~TCPClient()
{
    if (m_pReceiveData != NULL)
    {
        delete m_pReceiveData;
        m_pReceiveData = NULL;
    }
}

int TCPClient::DoShortConnectCommunicate(const char *sIP, int nPort, const char *sSendData, int nSendData,
                              int nConnectWaitMilliSecond, int nCommunicateWaitMilliSecond)
{
    if (
        (sIP == NULL)
        || (nPort <= 0)
        || (sSendData == NULL)
        || (nSendData <= 0)
        )
    {
        return -1;
    }

    int nReturn = 0;
    ZTCPSocket      tcpSocket;
    int             nRealSend;
    UINT64          nStartTime;
    bool            bSendSuccess = true;
    int             nRealRecv;
    bool            bRecvSuccess = true;

    if (tcpSocket.Create())
    {
        tcpSocket.SetNonBlocking(TRUE);
        if (!tcpSocket.Connect(ZSocket::ConvertAddr(sIP), nPort))
        {
            //LOG_DEBUG(("[TCPClient::DoShortConnectCommunicate] first connect failed\r\n"));
        }
        if (tcpSocket.ConnectAble(nConnectWaitMilliSecond*1000))
        {// connect success
            // send data
            // send msc_nEachTimeSendBytes data
            nStartTime = ZOS::milliseconds();
            bSendSuccess = true;
            for (int i = 0; i < nSendData/msc_nEachTimeSendBytes; i ++)
            {
                nRealSend = tcpSocket.Send(sSendData+i*msc_nEachTimeSendBytes, msc_nEachTimeSendBytes);
                if (nRealSend != msc_nEachTimeSendBytes)
                {
                    if (nRealSend == 0 || nRealSend == -2)
                    {
                        nReturn = -3;
                        bSendSuccess = false;
                        break;
                    }
                    else
                    {//nRealSend == -1 or (nRealSend > 0 but less msc_nEachTimeSendBytes)
                        if (nRealSend == -1)
                        {
                            nRealSend = 0;
                        }
                        int nLeft = msc_nEachTimeSendBytes - nRealSend;
                        int nAlreadySend = nRealSend;
                        while (nLeft > 0)
                        {
                            ZOSThread::Sleep(10);
                            nRealSend = tcpSocket.Send(sSendData+i*msc_nEachTimeSendBytes+nAlreadySend, nLeft);
                            if (nRealSend > 0)
                            {
                                nAlreadySend += nRealSend;
                                nLeft -= nRealSend;
                            }
                            else
                            {
                                if (nRealSend == 0 || nRealSend == -2)
                                {
                                    break;
                                }

                                if (ZOS::milliseconds()-nStartTime > (UINT64)nCommunicateWaitMilliSecond)
                                {
                                    nReturn = -4;
                                    bSendSuccess = false;
                                    break;
                                }
                            }
                        }
                        if (nRealSend == 0 || nRealSend == -2 || nRealSend == -1)
                        {
                            nReturn = -3;
                            bSendSuccess = false;
                            break;
                        }
                    }
                }
            }

            //send left data
            if (bSendSuccess)
            {
                if (nSendData%msc_nEachTimeSendBytes > 0)
                {
                    nRealSend = tcpSocket.Send(sSendData+(nSendData/msc_nEachTimeSendBytes)*msc_nEachTimeSendBytes, nSendData%msc_nEachTimeSendBytes);
                    if (nRealSend != nSendData%msc_nEachTimeSendBytes)
                    {
                        if (nRealSend == 0 || nRealSend == -2)
                        {
                            nReturn = -3;
                            bSendSuccess = false;
                        }
                        else
                        {//nRealSend == -1 or (nRealSend>0 but less msc_nEachTimeSendBytes)
                            if (nRealSend == -1)
                            {
                                nRealSend = 0;
                            }
                            int nLeft = nSendData%msc_nEachTimeSendBytes - nRealSend;
                            int nAlreadySend = nRealSend;
                            while (nLeft > 0)
                            {
                                ZOSThread::Sleep(10);
                                nRealSend = tcpSocket.Send(sSendData+(nSendData/msc_nEachTimeSendBytes)*msc_nEachTimeSendBytes+nAlreadySend, nLeft);
                                if (nRealSend > 0)
                                {
                                    nAlreadySend += nRealSend;
                                    nLeft -= nRealSend;
                                }
                                else
                                {
                                    if (nRealSend == 0 || nRealSend == -2)
                                    {
                                        break;
                                    }

                                    if (ZOS::milliseconds()-nStartTime > (UINT64)nCommunicateWaitMilliSecond)
                                    {
                                        nReturn = -4;
                                        bSendSuccess = false;
                                        break;
                                    }
                                }
                            }
                            if (nRealSend == 0 || nRealSend == -2 || nRealSend == -1)
                            {
                                nReturn = -3;
                                bSendSuccess = false;
                            }
                        }
                    }
                }
                while (
                    (nRealSend == -1)
                    && (ZOS::milliseconds()-nStartTime < (UINT64)nCommunicateWaitMilliSecond)
                    )
                {
                    ZOSThread::Sleep(10);
                    nRealSend = tcpSocket.Send(sSendData+(nSendData/msc_nEachTimeSendBytes)*msc_nEachTimeSendBytes, nSendData%msc_nEachTimeSendBytes);
                }
                if (nRealSend == -2)
                {
                    nReturn = -3;
                    bSendSuccess = false;
                    LOG_ERROR(("[TCPClient::DoShortConnectCommunicate] last send, socket send error. %s %d\r\n", sIP, nPort));
                }
                else if (ZOS::milliseconds()-nStartTime >= (UINT64)nCommunicateWaitMilliSecond)
                {
                    nReturn = -4;
                    bSendSuccess = false;
                    LOG_ERROR(("[TCPClient::DoShortConnectCommunicate] last send, send wait timeout. %s %d\r\n", sIP, nPort));
                }
            }
            else
            {
                LOG_ERROR(("[TCPClient::DoShortConnectCommunicate] send failed in send for loop, realSend = %d timeout %d\r\n",
                    nRealSend, ZOS::milliseconds()-nStartTime > (UINT64)nCommunicateWaitMilliSecond));
            }

            if (bSendSuccess)
            {
                // receive data
                if (m_pReceiveData != NULL)
                {
                    delete m_pReceiveData;
                    m_pReceiveData = NULL;
                }
                m_pReceiveData = new char [msc_nInitAllocateCount];
                if (m_pReceiveData != NULL)
                {
                    m_nReceiveBufferSize = msc_nInitAllocateCount;
                    // do first time receive
                    bRecvSuccess = false;
                    m_nReceiveDataCount = 0;
                    nRealRecv = tcpSocket.Recv(m_pReceiveData, msc_nEachTimeRecvBytes);
                    while (
                        (nRealRecv == -1)
                        && (ZOS::milliseconds()-nStartTime < (UINT64)nCommunicateWaitMilliSecond)
                        )
                    {
                        nRealRecv = tcpSocket.Recv(m_pReceiveData+m_nReceiveDataCount, msc_nEachTimeRecvBytes);
                    }
                    if (nRealRecv > 0)
                    {
                        bRecvSuccess = true;
                        m_nReceiveDataCount += nRealRecv;
                    }
                    else if (ZOS::milliseconds()-nStartTime >= (UINT64)nCommunicateWaitMilliSecond)
                    {
                        nReturn = -4;
                        LOG_ERROR(("[TCPClient::DoShortConnectCommunicate] first receive, receive wait timeout. %s %d\r\n", sIP, nPort));
                    }
                    else if (nRealRecv == 0 || nRealRecv == -2)
                    {
                        nReturn = -5;
                        LOG_ERROR(("[TCPClient::DoShortConnectCommunicate] first receive, receive error. %s %d %d\r\n", sIP, nPort, nRealRecv));
                    }

                    // do subsequent receive
                    if (bRecvSuccess)
                    {
                        nRealRecv = tcpSocket.Recv(m_pReceiveData+m_nReceiveDataCount, msc_nEachTimeRecvBytes);
                        while (nRealRecv > 0 && ZOS::milliseconds()-nStartTime < (UINT64)nCommunicateWaitMilliSecond)
                        {
                            m_nReceiveDataCount += nRealRecv;
                            if (EnsureReceiveBufferEnough() != 0)
                            {
                                nReturn = -7;
                                LOG_ERROR(("[TCPClient::DoShortConnectCommunicate] EnsureReceiveBufferEnough failed.\r\n"));
                                break;
                            }
                            nRealRecv = tcpSocket.Recv(m_pReceiveData+m_nReceiveDataCount, msc_nEachTimeRecvBytes);
                        }
                        if (nRealRecv == -1)
                        {
                            nReturn = 0;
                        }
                        else if (ZOS::milliseconds()-nStartTime >= (UINT64)nCommunicateWaitMilliSecond)
                        {
                            nReturn = -4;
                            LOG_ERROR(("[TCPClient::DoShortConnectCommunicate] receive loop, data too much, receive timeout\r\n"));
                        }
                        else if (nRealRecv == 0 || nRealRecv == -2)
                        {
                            nReturn = -5;
                            LOG_ERROR(("[TCPClient::DoShortConnectCommunicate] receive loop, receive error. %s %d %d\r\n", sIP, nPort, nRealRecv));
                        }
                    }
                }
                else
                {
                    nReturn = -7;
                    LOG_ERROR(("[TCPClient::DoShortConnectCommunicate] new failed\r\n"));
                }
            }
            else
            {
                LOG_ERROR(("[TCPClient::DoShortConnectCommunicate] send failed in send left data, realSend = %d timeout %d\r\n",
                    nRealSend, ZOS::milliseconds()-nStartTime > (UINT64)nCommunicateWaitMilliSecond));
            }
        }
        else
        {
            nReturn = -2;
            LOG_ERROR(("[TCPClient::DoShortConnectCommunicate] connect %s %d failed\r\n", sIP, nPort));
        }

        tcpSocket.Close();
    }
    else
    {
        nReturn = -6;
        LOG_ERROR(("[TCPClient::DoShortConnectCommunicate] socket not init\r\n"));
    }
    

    return nReturn;
}

char * TCPClient::GetReceiveData()
{
    return m_pReceiveData;
}

int TCPClient::GetReceiveDataCount()
{
    return m_nReceiveDataCount;
}

int TCPClient::DoTcpOnlySend(const char *sIP, int nPort, const char *sSendData, int nSendData, 
                  int nConnectWaitMilliSecond, int nSendWaitMilliSecond)
{
    if (
        (sIP == NULL)
        || (nPort <= 0)
        || (sSendData == NULL)
        || (nSendData <= 0)
        )
    {
        return -1;
    }

    int nReturn = 0;
    ZTCPSocket      tcpSocket;
    int             nRealSend;
    UINT64          nStartTime;
    bool            bSendSuccess = true;

    if (tcpSocket.Create())
    {
        tcpSocket.SetNonBlocking(TRUE);
        if (!tcpSocket.Connect(ZSocket::ConvertAddr(sIP), nPort))
        {
            //LOG_DEBUG(("[TCPClient::DoTcpOnlySend] first connect failed\r\n"));
        }
        if (tcpSocket.ConnectAble(nConnectWaitMilliSecond*1000))
        {// connect success
            // send data
            // send msc_nEachTimeSendBytes data
            nStartTime = ZOS::milliseconds();
            bSendSuccess = true;
            for (int i = 0; i < nSendData/msc_nEachTimeSendBytes; i ++)
            {
                nRealSend = tcpSocket.Send(sSendData+i*msc_nEachTimeSendBytes, msc_nEachTimeSendBytes);
                if (nRealSend != msc_nEachTimeSendBytes)
                {
                    if (nRealSend == 0 || nRealSend == -2)
                    {
                        nReturn = -3;
                        bSendSuccess = false;
                        break;
                    }
                    else
                    {//nRealSend == -1 or (nRealSend > 0 but less msc_nEachTimeSendBytes)
                        if (nRealSend == -1)
                        {
                            nRealSend = 0;
                        }
                        int nLeft = msc_nEachTimeSendBytes - nRealSend;
                        int nAlreadySend = nRealSend;
                        while (nLeft > 0)
                        {
                            ZOSThread::Sleep(10);
                            nRealSend = tcpSocket.Send(sSendData+i*msc_nEachTimeSendBytes+nAlreadySend, nLeft);
                            if (nRealSend > 0)
                            {
                                nAlreadySend += nRealSend;
                                nLeft -= nRealSend;
                            }
                            else
                            {
                                if (nRealSend == 0 || nRealSend == -2)
                                {
                                    break;
                                }

                                if (ZOS::milliseconds()-nStartTime > (UINT64)nSendWaitMilliSecond)
                                {
                                    nReturn = -4;
                                    bSendSuccess = false;
                                    break;
                                }
                            }
                        }
                        if (nRealSend == 0 || nRealSend == -2 || nRealSend == -1)
                        {
                            nReturn = -3;
                            bSendSuccess = false;
                            break;
                        }
                    }
                }
            }

            //send left data
            if (bSendSuccess)
            {
                if (nSendData%msc_nEachTimeSendBytes > 0)
                {
                    nRealSend = tcpSocket.Send(sSendData+(nSendData/msc_nEachTimeSendBytes)*msc_nEachTimeSendBytes, nSendData%msc_nEachTimeSendBytes);
                    if (nRealSend != nSendData%msc_nEachTimeSendBytes)
                    {
                        if (nRealSend == 0 || nRealSend == -2)
                        {
                            nReturn = -3;
                            bSendSuccess = false;
                        }
                        else
                        {//nRealSend == -1 or (nRealSend>0 but less msc_nEachTimeSendBytes)
                            if (nRealSend == -1)
                            {
                                nRealSend = 0;
                            }
                            int nLeft = nSendData%msc_nEachTimeSendBytes - nRealSend;
                            int nAlreadySend = nRealSend;
                            while (nLeft > 0)
                            {
                                ZOSThread::Sleep(10);
                                nRealSend = tcpSocket.Send(sSendData+(nSendData/msc_nEachTimeSendBytes)*msc_nEachTimeSendBytes+nAlreadySend, nLeft);
                                if (nRealSend > 0)
                                {
                                    nAlreadySend += nRealSend;
                                    nLeft -= nRealSend;
                                }
                                else
                                {
                                    if (nRealSend == 0 || nRealSend == -2)
                                    {
                                        break;
                                    }

                                    if (ZOS::milliseconds()-nStartTime > (UINT64)nSendWaitMilliSecond)
                                    {
                                        nReturn = -4;
                                        bSendSuccess = false;
                                        break;
                                    }
                                }
                            }
                            if (nRealSend == 0 || nRealSend == -2 || nRealSend == -1)
                            {
                                nReturn = -3;
                                bSendSuccess = false;
                            }
                        }
                    }

                    if (!bSendSuccess)
                    {
                        LOG_ERROR(("[TCPClient::DoTcpOnlySend] send failed in send left data, realSend = %d timeout %d\r\n",
                            nRealSend, ZOS::milliseconds()-nStartTime > (UINT64)nSendWaitMilliSecond));
                    }
                }
            }
            else
            {
                LOG_ERROR(("[TCPClient::DoTcpOnlySend] send failed in send for loop, realSend = %d timeout %d\r\n",
                    nRealSend, ZOS::milliseconds()-nStartTime > (UINT64)nSendWaitMilliSecond));
            }
        }
        else
        {
            nReturn = -2;
            LOG_ERROR(("[TCPClient::DoTcpOnlySend] connect %s %d failed\r\n", sIP, nPort));
        }

        tcpSocket.Close();
    }
    else
    {
        nReturn = -5;
        LOG_ERROR(("[TCPClient::DoTcpOnlySend] socket not init\r\n"));
    }

    return nReturn;
}

int TCPClient::EnsureReceiveBufferEnough()
{
    char *pTemp = NULL;
    int nReturn = -1;

    if (m_pReceiveData != NULL)
    {
        if (m_nReceiveDataCount+msc_nEachTimeRecvBytes >= m_nReceiveBufferSize)
        {
            pTemp = new char[m_nReceiveBufferSize*3];
            if (pTemp != NULL)
            {
                memmove(pTemp, m_pReceiveData, m_nReceiveBufferSize);
                delete m_pReceiveData;
                m_pReceiveData = pTemp;
                m_nReceiveBufferSize = m_nReceiveBufferSize*3;

                nReturn = 0;
            }
        }
        else
        {
            nReturn = 0;
        }
    }

    return nReturn;
}
