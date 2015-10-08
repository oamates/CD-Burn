#include "NCXClient.h"

const int NCXClient::msc_nEachTimeSendBytes = 1024;
const int NCXClient::msc_nEachTimeRecvBytes = 1024;
const int NCXClient::msc_nInitAllocateCount = 10*NCXClient::msc_nEachTimeRecvBytes;

NCXClient::NCXClient()
: m_tcpClient()
, m_pReceiveData(NULL)
, m_nReceiveDataCount(0)
, m_nReceiveBufferSize(0)
{
    memset(m_sOuterMostTagName, 0, MAX_TAGNAME_LENGTH);
    m_nStartTime = ZOS::milliseconds();
}

NCXClient::~NCXClient()
{
    Close();
    //LOG_DEBUG(("[NCXClient::~NCXClient] %"F_NUM_64"d\r\n", ZOS::milliseconds()-m_nStartTime));
}

BOOL NCXClient::Create()
{
    if (m_tcpClient.Create())
    {
        m_tcpClient.SetNonBlocking(TRUE);
        m_tcpClient.SetReuseAddr(TRUE);
        return TRUE;
    }
    
    return FALSE;
}

BOOL NCXClient::Close()
{
    m_tcpClient.Close();
    if (m_pReceiveData != NULL)
    {
        delete m_pReceiveData;
        m_pReceiveData = NULL;
    }

    return TRUE;
}

void NCXClient::SetOuterMostTagName(const char *sTagName)
{
    strncpy(m_sOuterMostTagName, sTagName, MAX_TAGNAME_LENGTH);
}

BOOL NCXClient::ConnectServer(const char* sServerIP, int nServerPort, int nWaitSecond)
{
    if (!m_tcpClient.Connect(ZSocket::ConvertAddr(sServerIP), nServerPort))
    {
        //
    }

    if (m_tcpClient.ConnectAble(nWaitSecond*1000*1000))
    {
        m_tcpClient.SetRecvBufferSize(DEFAULT_TCP_RECV_BUFFER_SIZE);
        return TRUE;
    }
    else
    {
        LOG_ERROR(("[NCXClient::ConnectServer] connect %s %d failed\r\n", sServerIP, nServerPort));
        return FALSE;
    }
}

int NCXClient::Communicate(const char* sProtocol, int nProtocol, int nWaitSecond)
{
    int nReturn = -1;
    UINT64 nTimeout = ZOS::milliseconds() + nWaitSecond*1000;

    // do send
    int nRealSend;
    bool bSendSuccess = true;
    int nRealRecv;
    int nProtocolStartPos;
    int nProtocolLength;

    // send loop
    for (int i = 0; i < nProtocol/msc_nEachTimeSendBytes; i ++)
    {
        nRealSend = m_tcpClient.Send(sProtocol+i*msc_nEachTimeSendBytes, msc_nEachTimeSendBytes);
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
                    nRealSend = m_tcpClient.Send(sProtocol+i*msc_nEachTimeSendBytes+nAlreadySend, nLeft);
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
        }//if (nRealSend != msc_nEachTimeSendBytes)

        if (ZOS::milliseconds() > nTimeout)
        {
            bSendSuccess = false;
            break;
        }
    }//for (int i = 0; i < nProtocol/msc_nEachTimeSendBytes; i ++)

    // send left
    if (bSendSuccess)
    {
        if (nProtocol%msc_nEachTimeSendBytes > 0)
        {
            nRealSend = m_tcpClient.Send(sProtocol+(nProtocol/msc_nEachTimeSendBytes)*msc_nEachTimeSendBytes, nProtocol%msc_nEachTimeSendBytes);
            if (nRealSend != nProtocol%msc_nEachTimeSendBytes)
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
                    int nLeft = nProtocol%msc_nEachTimeSendBytes - nRealSend;
                    int nAlreadySend = nRealSend;
                    while (nLeft > 0)
                    {
                        ZOSThread::Sleep(10);
                        nRealSend = m_tcpClient.Send(sProtocol+(nProtocol/msc_nEachTimeSendBytes)*msc_nEachTimeSendBytes+nAlreadySend, nLeft);
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
        }//if (nProtocol%msc_nEachTimeSendBytes > 0)
    }
    else
    {
        LOG_ERROR(("[NCXClient::Communicate] send loop failed, nRealSend = %d timeout = %d\r\n",
            nRealSend, ZOS::milliseconds() > nTimeout));
    }

    if (bSendSuccess)
    {//send success, start receive
        m_pReceiveData = new char[msc_nInitAllocateCount];
        if (m_pReceiveData != NULL)
        {
            m_nReceiveBufferSize = msc_nInitAllocateCount;
            m_nReceiveDataCount = 0;
            do 
            {
                if (ZOS::milliseconds() > nTimeout)
                {// recv timeout.
                    nReturn = -2;
                    LOG_ERROR(("[NCXClient::Communicate] recv timeout\r\n"));
                    break;
                }

                EnsureReceiveBufferEnough();
                nRealRecv = m_tcpClient.Recv(m_pReceiveData+m_nReceiveDataCount, msc_nEachTimeRecvBytes);
                if (nRealRecv > 0)
                {
                    m_nReceiveDataCount += nRealRecv;
                    continue;
                }
                m_pReceiveData[m_nReceiveDataCount] = '\0';
                if (TagUtillity::IsProtocolComplete(m_pReceiveData, m_sOuterMostTagName, &nProtocolStartPos, &nProtocolLength))
                {
                    // receive success.
                    nReturn = 0;
                    break;
                }
                
                if (nRealRecv == -1)
                {
                    ZOSThread::Sleep(10);
                }
                else if (nRealRecv == 0)
                {
                    // server close.
                    nReturn = -3;
                    LOG_ERROR(("[NCXClient::Communicate] server close.\r\n"));
                    break;
                }
                else if (nRealRecv == -2)
                {
                    // socket error.
                    nReturn = -3;
                    LOG_ERROR(("[NCXClient::Communicate] socket error.\r\n"));
                    break;
                }
            } while (true);
        }
    }
    else
    {
        LOG_ERROR(("[NCXClient::Communicate] send left data failed, nRealSend = %d timeout = %d\r\n",
            nRealSend, ZOS::milliseconds() > nTimeout));
    }

    m_tcpClient.Close();

    return nReturn;
}

CHAR * NCXClient::GetRespondProtocol()
{
	return m_pReceiveData;
}

int NCXClient::EnsureReceiveBufferEnough()
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

//////////////////////////////////////////////////////////////////////////
