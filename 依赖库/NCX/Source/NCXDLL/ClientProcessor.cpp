#include "ClientProcessor.h"
//////////////////////////////////////////////////////////////////////////
ClientProcessorListener::ClientProcessorListener()
{
    //
}

ClientProcessorListener::~ClientProcessorListener()
{
    //
}

BOOL ClientProcessorListener::OnReceiveCompleteProtocol(ClientProcessor *pClientProcessor, const char *sProtocol, int nProtocolLength)
{
    return TRUE;
}
//////////////////////////////////////////////////////////////////////////
const int ClientProcessor::msc_nSendWaitMilliSecond = 3000;
const int ClientProcessor::msc_nEachTimeSendBytes = 1024;
const int ClientProcessor::msc_nEachTimeRecvBytes = 1024;
const int ClientProcessor::msc_nInitAllocateCount = 10*ClientProcessor::msc_nEachTimeRecvBytes;
const int ClientProcessor::msc_nSessionTimeoutMilliSecond = 30*1000;

ClientProcessor::ClientProcessor()
: ZOSThread("ClientProcessorThread")
, m_bIsBeUsed(false)
, m_pClientProcessorListener(NULL)
, m_tcpSocket()
, m_bGetProtocol(false)
, m_pReceiveData(NULL)
, m_nReceiveDataCount(0)
, m_nReceiveBufferSize()
{
    memset(m_sOuterMostTagName, 0, MAX_TAGNAME_LENGTH);
    m_nLastAliveTime = ZOS::milliseconds();
}

ClientProcessor::~ClientProcessor()
{
    //
}

bool ClientProcessor::Create()
{
    if (ZOSThread::Start())
    {
        m_pReceiveData = new char[msc_nInitAllocateCount];
        if (m_pReceiveData != NULL)
        {
            m_nReceiveBufferSize = msc_nInitAllocateCount;
            return true;
        }
    }

    return false;
}

bool ClientProcessor::Close(BOOL bWait)
{
    ClearSession();

    if (m_pReceiveData != NULL)
    {
        delete m_pReceiveData;
        m_pReceiveData = NULL;
    }

    ZOSThread::Stop(bWait);

    return true;
}

void ClientProcessor::SetListener(ClientProcessorListener *pListener)
{
    m_pClientProcessorListener = pListener;
}

void ClientProcessor::SetOuterMostTagName(const char *sTagName)
{
    strncpy(m_sOuterMostTagName, sTagName, MAX_TAGNAME_LENGTH);
    m_sOuterMostTagName[MAX_TAGNAME_LENGTH-1] = '\0';
}

bool ClientProcessor::IsBeUsed()
{
    return m_bIsBeUsed;
}

int ClientProcessor::SetClient(int nClientSocket, UINT nClientAddr, UINT nClientPort)
{
    int nReturn = -1;

    if (!m_bIsBeUsed)
    {
        struct  sockaddr_in remoteAddr;
        ::memset(&remoteAddr,0,sizeof(remoteAddr));
        remoteAddr.sin_family       = AF_INET;
        remoteAddr.sin_port         = htons(nClientPort);
        remoteAddr.sin_addr.s_addr  = htonl(nClientAddr);

        if (m_tcpSocket.Attach(nClientSocket, &remoteAddr))
        {
            m_tcpSocket.SetRecvBufferSize(DEFAULT_TCP_RECV_BUFFER_SIZE);
            m_tcpSocket.SetNonBlocking(TRUE);
            m_bIsBeUsed = true;
            m_bGetProtocol = false;
            m_nReceiveDataCount = 0;
            m_nLastAliveTime = ZOS::milliseconds();
            nReturn = 0;
        }
    }

    return nReturn;
}

int ClientProcessor::SendRespond(const char *sProtocol, int nProtocolLength)
{
    bool bSendSuccess = true;
    int nRealSend = 0;
    UINT64 nTimeout = 0;

    if (m_tcpSocket.GetSocket() != 0xFFFFFFFF)
    {
        nTimeout = ZOS::milliseconds()+msc_nSendWaitMilliSecond;

        for (int i = 0; i < nProtocolLength/msc_nEachTimeSendBytes; i ++)
        {
            nRealSend = m_tcpSocket.Send(sProtocol+i*msc_nEachTimeSendBytes, msc_nEachTimeSendBytes);
            if (nRealSend != msc_nEachTimeSendBytes)
            {
                if (nRealSend == 0 || nRealSend == -2)
                {
                    bSendSuccess = false;
                    break;
                }
                else
                {//nRealSend == -1 or (nRealSend>0 but less msc_nEachTimeSendBytes)
                    if (nRealSend == -1)
                    {
                        nRealSend = 0;
                    }
                    int nLeft = msc_nEachTimeSendBytes - nRealSend;
                    int nAlreadySend = nRealSend;
                    while (nLeft > 0)
                    {
                        ZOSThread::Sleep(10);
                        nRealSend = m_tcpSocket.Send(sProtocol+i*msc_nEachTimeSendBytes+nAlreadySend, nLeft);
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

                            if (ZOS::milliseconds() > nTimeout)
                            {
                                bSendSuccess = false;
                                break;
                            }
                        }
                    }
                    if (nRealSend == 0 || nRealSend == -2 || nRealSend == -1)
                    {
                        bSendSuccess = false;
                        break;
                    }
                }
            }
        }

        if (bSendSuccess)
        {
            if (nProtocolLength%msc_nEachTimeSendBytes > 0)
            {
                nRealSend = m_tcpSocket.Send(sProtocol+(nProtocolLength/msc_nEachTimeSendBytes)*msc_nEachTimeSendBytes, nProtocolLength%msc_nEachTimeSendBytes);
                if (nRealSend != nProtocolLength%msc_nEachTimeSendBytes)
                {
                    if (nRealSend == 0 || nRealSend == -2)
                    {
                        bSendSuccess = false;
                    }
                    else
                    {//nRealSend == -1 or (nRealSend>0 but less msc_nEachTimeSendBytes)
                        if (nRealSend == -1)
                        {
                            nRealSend = 0;
                        }
                        int nLeft = nProtocolLength%msc_nEachTimeSendBytes - nRealSend;
                        int nAlreadySend = nRealSend;
                        while (nLeft > 0)
                        {
                            ZOSThread::Sleep(10);
                            nRealSend = m_tcpSocket.Send(sProtocol+(nProtocolLength/msc_nEachTimeSendBytes)*msc_nEachTimeSendBytes+nAlreadySend, nLeft);
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

                                if (ZOS::milliseconds() > nTimeout)
                                {
                                    bSendSuccess = false;
                                    break;
                                }
                            }
                        }
                        if (nRealSend == 0 || nRealSend == -2 || nRealSend == -1)
                        {
                            bSendSuccess = false;
                        }
                    }
                }//if (nRealSend != nProtocol%msc_nEachTimeSendBytes)

                if (ZOS::milliseconds() > nTimeout)
                {
                    bSendSuccess = false;
                }
            }
        }
        else
        {
            LOG_ERROR(("[ClientProcessor::SendRespond] send loop, nRealSend = %d timeout = %d\r\n", 
                nRealSend, ZOS::milliseconds()>nTimeout));
        }
    }
    else
    {
        LOG_WARNING(("[ClientProcessor::SendRespond] m_tcpSocket.GetSocket() == -1\r\n"));
        bSendSuccess = false;
    }

    if (bSendSuccess)
    {
        return 0;
    }
    else
    {
        LOG_ERROR(("[ClientProcessor::SendRespond] send left failed, nRealSend = %d timeout = %d\r\n", 
            nRealSend, ZOS::milliseconds()>nTimeout));
        return -1;
    }
}

char* ClientProcessor::GetRemoteIP()
{
    UINT    nAddr,nPort;
    if (m_tcpSocket.GetSocket() != 0xFFFFFFFF)
    {
        m_tcpSocket.GetRemoteAddr(&nAddr,&nPort);
        return ZSocket::ConvertAddr(nAddr);
    }
    else
    {
        return NULL;
    }
}

BOOL ClientProcessor::OnThreadStart()
{
    return TRUE;
}

BOOL ClientProcessor::OnThreadEntry()
{
    if(!m_bStop)
    {
        if (m_bIsBeUsed)
        {
            ProcessReceive();

            if ((int)(ZOS::milliseconds() - m_nLastAliveTime) > msc_nSessionTimeoutMilliSecond)
            {
                LOG_ERROR(("[ClientProcessor::OnThreadEntry] timeout\r\n"));
                ClearSession();
            }
        }

        ZOSThread::Sleep(10);
    }

    return TRUE;
}

BOOL ClientProcessor::OnThreadStop()
{
    return TRUE;
}

void ClientProcessor::ProcessReceive()
{
    int nRealRecv;
    int nProtocolStartPos;
    int nProtocolLength;

    if (m_tcpSocket.GetSocket() != 0xFFFFFFFF)
    {
        do
        {
            EnsureReceiveBufferEnough();
            nRealRecv = m_tcpSocket.Recv(m_pReceiveData+m_nReceiveDataCount, msc_nEachTimeRecvBytes);
            if (nRealRecv > 0)
            {
                m_nReceiveDataCount += nRealRecv;
                m_pReceiveData[m_nReceiveDataCount] = '\0';
                m_nLastAliveTime = ZOS::milliseconds();
            }
            else
            {
                if (nRealRecv == -1)
                {
                    //
                }
                else if (nRealRecv == 0)
                {
                    //LOG_INFO(("[ClientProcessor::ProcessReceive] client close session\r\n"));
                    ClearSession();
                }
                else if (nRealRecv == -2)
                {
                    LOG_ERROR(("[ClientProcessor::ProcessReceive] client close session unnormal\r\n"));
                    ClearSession();
                }
                else
                {
                    LOG_ERROR(("[ClientProcessor::ProcessReceive] error unknow\r\n"));
                }
            }
        } while (nRealRecv > 0);
    }

    if (!m_bGetProtocol)
    {
        if (m_nReceiveDataCount > 0 && TagUtillity::IsProtocolComplete(m_pReceiveData, m_sOuterMostTagName, &nProtocolStartPos, &nProtocolLength))
        {// get complete format string
            m_bGetProtocol = true;
            // call upper caller callback
            if (m_pClientProcessorListener != NULL)
            {
                m_pClientProcessorListener->OnReceiveCompleteProtocol(this, m_pReceiveData+nProtocolStartPos, nProtocolLength);
            }
        }
    }

}

int ClientProcessor::EnsureReceiveBufferEnough()
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

int ClientProcessor::ClearSession()
{
    m_tcpSocket.Close();
    m_bIsBeUsed = false;
    m_bGetProtocol = false;
    if (m_pReceiveData != NULL)
    {
        memset(m_pReceiveData, 0, m_nReceiveBufferSize);
    }
    m_nReceiveDataCount = 0;

    return 0;
}
//////////////////////////////////////////////////////////////////////////
