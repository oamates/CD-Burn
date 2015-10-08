#include "ZRTPSession.h"
#include "ZRTPPacket.h"
#include "ZRTCPPacket.h"
#include "ZRTPStream.h"
#include "ZRTSPSession.h"
#include "ZHeaderParser.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//#define	ZRTPSESSION_DEBUG		1
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_IDLE_TIME		(DEFAULT_SESSION_TIMEIDLE)
#define	DEFAULT_TASK_TIME		3
#define	DEFAULT_TASK_COUNT		3
#define	DEFAULT_SEND_TASK_TIME		1
#define	DEFAULT_SEND_TASK_COUNT		7
#define	DEFAULT_RECV_TASK_TIME		3
#define	DEFAULT_RECV_TASK_COUNT		30
///////////////////////////////////////////////////////////////////////////////
ZRTPSession::ZRTPSession(SESSION_SUBTYPE eSubType)
:ZSession(SESSION_TYPE_RTP,eSubType)
,m_pRTSPSession(NULL)
,m_pRTSPSessionStream(NULL)
,m_pCurrentStream(NULL)
,m_nStreamCount(0)
,m_StreamArray(2)
,m_nChannel(-1)
,m_RTPPacketMutex("SessionRTPPakcetMutex")
,m_RTPPacketPool(0)
,m_nStartTime(0)
,m_nTotalTime(0)
,m_nLastTime(0)
,m_nSessionTimeoutMilliSecond(DEFAULT_SESSION_TIMEOUT)
,m_nIdleTime(DEFAULT_IDLE_TIME)
,m_nTaskTime(DEFAULT_TASK_TIME)
,m_nTaskCount(DEFAULT_TASK_COUNT)
,m_bUpdate(FALSE)
,m_nMultiAddr(0)
,m_nLastPrintTime(0)
,m_nInServerRun(0)
{
#ifdef	ZRTPSESSION_DEBUG
	MESSAGE_OUT(("RTP New Session\r\n"));
#endif	//ZRTPSESSION_DEBUG
	if (eSubType == SESSION_TYPE_CLIENT)
	{
		m_nTaskTime = m_nRecvFreq;
		m_nTaskCount = m_nRecvCount;
	}
	else if (eSubType == SESSION_TYPE_SERVER)
	{
		m_nTaskTime = m_nSendFreq;
		m_nTaskCount = m_nSendCount;
	}
    m_RTPPacketPool.SetSize(1200);
}
ZRTPSession::~ZRTPSession()
{
#ifdef	ZRTPSESSION_DEBUG
	MESSAGE_OUT(("RTP Release Session\r\n"));
#endif	//ZRTPSESSION_DEBUG
	ZRTPSession::Close();
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPSession::Create()
{
#ifdef	ZRTPSESSION_DEBUG
	MESSAGE_OUT(("ZRTPSession::Create\r\n"));
#endif	//ZRTPSESSION_DEBUG
	ZRTPSession::Close();

	m_pCurrentStream	= NULL;
	m_nStreamCount		= 0;
	m_bUpdate			= FALSE;

	return ZSession::Create();
}
BOOL ZRTPSession::Close()
{
#ifdef	ZRTPSESSION_DEBUG
	MESSAGE_OUT(("ZRTPSession::Close\r\n"));
#endif	//ZRTPSESSION_DEBUG
    //LOG_DEBUG(("[ZRTPSession::Close] before TearDown\r\n"));
	ZRTPSession::TearDown();
    //LOG_DEBUG(("[ZRTPSession::Close] after TearDown\r\n"));

	return ZSession::Close();
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPSession::SetSessionStream(ZBaseStream* pStream)
{
	return FALSE;
}
UINT ZRTPSession::GetDataStreamCount()
{
	ZOSMutexLocker	locker(&m_SessionMutex);

	return(m_nStreamCount);
}
ZBaseStream* ZRTPSession::GetDataStream(int i)
{
	ZOSMutexLocker	locker(&m_SessionMutex);

	if(i >= 0 && i< m_StreamArray.Count())
	{
		return((ZBaseStream*)m_StreamArray[i]);
	}
	return(NULL);
}
ZDataPin* ZRTPSession::GetDataPin(int i)
{
	ZOSMutexLocker	locker(&m_SessionMutex);

	if(i >= 0 && i< m_StreamArray.Count())
	{
		return((ZDataPin*)m_StreamArray[i]);
	}
	return(NULL);
}
BOOL ZRTPSession::OnDataPinClose()
{
	if (m_pRTSPSession != NULL)
	{
		m_pRTSPSession->OnDataPinClose();
	}
	return TRUE;
}
UINT64 ZRTPSession::GetSessionTotalPacketCount()
{
    int i = 0;
    UINT64 nTotal = 0;
    ZOSMutexLocker  locker(&m_SessionMutex);

    for (i = 0; i < m_StreamArray.Count(); i ++)
    {
        if (m_StreamArray[i] != NULL)
        {
            nTotal += m_StreamArray[i]->GetTotalPacketCount();
        }
    }

    return nTotal;
}
UINT64 ZRTPSession::GetSessionLostPacketCount()
{
    int i = 0;
    UINT64 nLost = 0;
    ZOSMutexLocker  locker(&m_SessionMutex);

    for (i = 0; i < m_StreamArray.Count(); i ++)
    {
        if (m_StreamArray[i] != NULL)
        {
            nLost += m_StreamArray[i]->GetLostPacketCount();
        }
    }

    return nLost;
}

void ZRTPSession::SetSessionTimeoutMilliSecond(int nTimeoutMilliSecond)
{
    m_nSessionTimeoutMilliSecond = nTimeoutMilliSecond;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ZRTPSession::SetRTSPSession(ZRTSPSession* pRTSPSession)
{
	m_pRTSPSession		= pRTSPSession;
}
void ZRTPSession::SetRTSPSessionStream(ZSessionStream* pSessionStream)
{
	m_pRTSPSessionStream	= pSessionStream;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPSession::AddClientStream(ZRTSPRequest* pRTSPRequest)
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_SessionMutex);

	DWORD nMediaAddr = 0;
	if(m_pRTSPSession != NULL)
	{
		//nMediaAddr = ntohl(m_pRTSPSession->GetMediaDstAddr());
		nMediaAddr = m_pRTSPSession->GetMediaDstAddr();
	}

	if(nMediaAddr >  0xE00000FF && nMediaAddr <= 0xEFFFFFFF)
	{
		if(pRTSPRequest != NULL)
		{
			pRTSPRequest->SetTransportParamter(RTSP_PROTOCOL_TRANSPORT_MULTICAST);

			ZRTPStream*	pRTPStream	= NEW ZRTPStream(this);
			//LOG_DEBUG(("[ZRTPSession::AddClientStream] %d\r\n", (int)pRTPStream));
			if(pRTPStream != NULL)
			{
				pRTPStream->Pause();
				pRTSPRequest->SetChannel((m_nChannel+1),(m_nChannel+2));
				if(pRTPStream->SetupClientStreamMulticast(pRTSPRequest,m_pRTSPSession->GetMediaDstAddr(),m_pRTSPSession->GetMediaDstPort(m_nStreamCount)))
				{
					m_pCurrentStream	= pRTPStream;
					m_StreamArray.Add(pRTPStream);
					m_nStreamCount		++;
					m_nChannel   += 2;
					bReturn				= TRUE;
				}
				else
				{
					LOG_ERROR(("ErrorCode:%s [ZRTPSession::AddClientStream] ERROR(pRTPStream SetupClientStream fail)!\r\n", GetErrorCodeString(24009)));
					SAFE_DELETE(pRTPStream);
				}
			}
			else
			{
				LOG_ERROR(("ErrorCode:%s [ZRTPSession::AddClientStream] ERROR(can not new ZRTPStream)!\r\n", GetErrorCodeString(24008)));
			}
		}
	}
	else
	{

		if(pRTSPRequest != NULL)
		{
			ZRTPStream*	pRTPStream	= NEW ZRTPStream(this);
			//LOG_DEBUG(("[ZRTPSession::AddClientStream] %d\r\n", (int)pRTPStream));
			if(pRTPStream != NULL)
			{
				pRTPStream->Pause();
				pRTSPRequest->SetChannel((m_nChannel+1),(m_nChannel+2));
				if(pRTPStream->SetupClientStream(pRTSPRequest))
				{
					m_pCurrentStream	= pRTPStream;
					m_StreamArray.Add(pRTPStream);
					m_nStreamCount		++;
					m_nChannel   += 2;
                    if (m_nChannel >= 100)
                    {
                        m_nChannel = 0;
                    }
					bReturn				= TRUE;
				}
				else
				{
					LOG_ERROR(("ErrorCode:%s [ZRTPSession::AddClientStream] ERROR(pRTPStream SetupClientStream fail)!\r\n", GetErrorCodeString(24009)));
					SAFE_DELETE(pRTPStream);
				}
			}
			else
			{
				LOG_ERROR(("ErrorCode:%s [ZRTPSession::AddClientStream] ERROR(can not new ZRTPStream)!\r\n", GetErrorCodeString(24008)));
			}
		}
	}
	return bReturn;
}
BOOL ZRTPSession::SetupClientStream(ZRTSPResponse* pRTSPResponse)
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_SessionMutex);

	if(pRTSPResponse != NULL)
	{
		if(m_pCurrentStream != NULL)
		{
			m_pCurrentStream->SetRemotePort(pRTSPResponse->GetServerPortA(),pRTSPResponse->GetServerPortB());
			m_pCurrentStream->SetChannel(pRTSPResponse->GetChannelA(),pRTSPResponse->GetChannelB());
			bReturn	= TRUE;
		}
	}
	m_pCurrentStream	= NULL;

	return bReturn;
}
BOOL ZRTPSession::AddServerStream(ZRTSPRequest* pRTSPRequest,ZDataPin* pDataPin,int nRtpSize)
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_SessionMutex);

	if(pRTSPRequest != NULL)
	{
		ZRTPStream*	pRTPStream	= NEW ZRTPStream(this,(ZDataOut*)pDataPin);
		if(pRTPStream != NULL)
		{
			pRTPStream->Pause();
			if(pRTPStream->SetupServerStream(pRTSPRequest,nRtpSize))
			{
				m_pCurrentStream	= pRTPStream;
				m_StreamArray.Add(pRTPStream);
				m_nStreamCount		++;
				bReturn				= TRUE;
			}else{
				DEBUG_OUT(("ZRTPSession::AddStream ERROR(SETUP)\r\n"));
				SAFE_DELETE(pRTPStream);
			}
		}else{
			DEBUG_OUT(("ZRTPSession::AddStream ERROR(MEMORY)\r\n"));
		}
	}
	return bReturn;
}
BOOL ZRTPSession::AddServerStreamMulticast(CHAR *sMultiIP, int nPortA, int nPortB, int nRtpSize, ZDataPin *pDataPin)
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_SessionMutex);

	if (pDataPin != NULL)
	{
		ZRTPStream*	pRTPStream	= NEW ZRTPStream(this,(ZDataOut*)pDataPin);
		if(pRTPStream != NULL)
		{
			pRTPStream->Pause();
			if(pRTPStream->SetupServerStreamMulticast(sMultiIP, nPortA, nPortB,nRtpSize))
			{
				m_pCurrentStream	= pRTPStream;
				m_StreamArray.Add(pRTPStream);
				m_nStreamCount		++;
				bReturn				= TRUE;
			}
			else
			{
				DEBUG_OUT(("ZRTPSession::AddServerStreamMulticast ERROR(SETUP)\r\n"));
				SAFE_DELETE(pRTPStream);
			}
		}
		else
		{
			DEBUG_OUT(("ZRTPSession::AddServerStreamMulticast ERROR(MEMORY)\r\n"));
		}
	}

	return bReturn;
}
BOOL ZRTPSession::SetupServerStream(ZRTSPRequest* pRTSPRequest,ZRTSPResponse* pRTSPResponse)
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_SessionMutex);

	if(pRTSPRequest != NULL && pRTSPResponse != NULL)
	{
		if(m_pCurrentStream != NULL)
		{
			m_pCurrentStream->SetChannel(pRTSPRequest->GetChannelA(),pRTSPRequest->GetChannelB());
			pRTSPResponse->SetServerPort(m_pCurrentStream->GetLocalPortA(),m_pCurrentStream->GetLocalPortB());
		}
	}
	m_pCurrentStream	= NULL;

	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPSession::RecvRTPPackets()
{
	int				i		= 0;
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_SessionMutex);

	for(i = 0; i < m_StreamArray.Count(); i ++)
	{
		if(m_StreamArray[i] != NULL)
		{
			if(m_StreamArray[i]->OnRecvRTPPacket())
			{
				bReturn	= TRUE;
			}
		}
	}

	return bReturn;
};
BOOL ZRTPSession::RecvRTCPPackets()
{
	int				i		= 0;
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_SessionMutex);

	for(i = 0; i < m_StreamArray.Count(); i ++)
	{
		if(m_StreamArray[i] != NULL)
		{
			if(m_StreamArray[i]->OnRecvRTCPPacket())
			{
				bReturn	= TRUE;
			}
		}
	}

	return bReturn;
}
BOOL ZRTPSession::SendRTPPackets()
{
	int				i		= 0;
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_SessionMutex);


    ZRTPPacket*     pPacketUsedBegin = NULL;

    DWORD nSSRC = 0;

    {
        ZOSMutexLocker	locker(&m_RTPPacketMutex);
        pPacketUsedBegin = m_RTPPacketPool.GetUsedBegin();
        if (pPacketUsedBegin != NULL)
        {
            nSSRC = pPacketUsedBegin->GetPacketSSRC();
        }
    }

    if (nSSRC != 0)
    {
        for (i = 0; i < m_StreamArray.Count(); i ++)
        {
            if (m_StreamArray[i] != NULL)
            {
                if (m_StreamArray[i]->GetServerSSRC() == nSSRC)
                {
                    if (m_StreamArray[i]->OnSendRTPPacket())
                    {
                        bReturn = TRUE;
                    }
                    break;
                }
            }
        }
    }

	return bReturn;
}
BOOL ZRTPSession::SendRTCPPackets()
{
	int				i		= 0;
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_SessionMutex);

	for(i = 0; i < m_StreamArray.Count(); i ++)
	{
		if(m_StreamArray[i] != NULL)
		{
			if(m_StreamArray[i]->OnSendRTCPPacket())
			{
				bReturn	= TRUE;
			}
		}
	}

	return bReturn;
}

BOOL ZRTPSession::PoolNearFull()
{
    int     i = 0;
    BOOL    bReturn = FALSE;

    ZOSMutexLocker  locker(&m_SessionMutex);
    for (i = 0; i < m_StreamArray.Count(); i ++)
    {
        if (m_StreamArray[i] != NULL && m_StreamArray[i]->RTPPoolUsePercent() > 0.85)
        {
            bReturn = TRUE;
        }
    }

    return bReturn;
}

BOOL ZRTPSession::ChannelExist(int nChannel)
{
    int				i		= 0;
    ZOSMutexLocker	locker(&m_SessionMutex);

    for (i = 0; i < m_StreamArray.Count(); i ++)
    {
        if (m_StreamArray[i] != NULL)
        {
            int nChannelA = -1;
            int nChannelB = -1;
            m_StreamArray[i]->GetChannel(&nChannelA, &nChannelB);
            if (nChannelA >= 0 && nChannelB >= 0)
            {
                if (nChannel == nChannelA || nChannel == nChannelB)
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
int ZRTPSession::InterleavedRead(void* sdata,int* ndata,const int nchannel)
{
	int		nError		= 0;
	char*	pBuffer		= NULL;
	int		nBuffer		= 0;
	int		nDataPacket	= 0;

	if(sdata != NULL && ndata != NULL)
	{
		if(m_pRTSPSessionStream != NULL)
		{
			pBuffer	= m_pRTSPSessionStream->GetBuffer();
			nBuffer	= m_pRTSPSessionStream->GetBufferLength();

            if (pBuffer != NULL && nBuffer > 6)
            {
                if((pBuffer[0]&0xFF) != '$')
                {//we are in a strange state, drop buffer data until next '$' or whole buffer
                    int nMoveCount = 0;
                    while (nMoveCount < nBuffer)
                    {
                        if ((pBuffer[nMoveCount]&0xFF) == '$')
                        {
                            break;
                        }
                        nMoveCount ++;
                    }
                    if (nMoveCount > 0)
                    {
                        LOG_DEBUG(("[ZRTPSession::InterleavedRead] drop count %d nBuffer %d\r\n", 
                            nMoveCount, nBuffer));
                        m_pRTSPSessionStream->MoveBuffer(nMoveCount);
                    }
                    *ndata = 0;
                    return 0;
                }
                else
                {
                    int nCurChannel = pBuffer[1]&0xFF;
                    unsigned char *ucpBuffer = (unsigned char*)pBuffer;
                    if (!ChannelExist(nCurChannel) || ((ucpBuffer[4]&0xC0)!=0x80))
                    {//though first char is '$', but channel is invalid, so drop this data.
                        m_pRTSPSessionStream->MoveBuffer(2);
                        *ndata = 0;
                        LOG_DEBUG(("[ZRTPSession::InterleavedRead] channel %d ucpBuffer[4] %x is not valid\r\n",
                            nCurChannel, (ucpBuffer[4]&0xC0)));
                        return 0;
                    }
                }
            }

			if(pBuffer != NULL && nBuffer > 4)
			{
				if((pBuffer[0]&0xFF) == '$')
				{
					nDataPacket	= GETUINT16(&pBuffer[2]);
					nDataPacket	= (ntohs(nDataPacket)+4);
					if(nDataPacket <= nBuffer)
					{
						if((pBuffer[1]&0xFF) == nchannel)
						{
                            //LOG_DEBUG(("[ZRTPSession::InterleavedRead] == $ nDataPacket %d nchannel %d\r\n",
                            //    nDataPacket, nchannel));
							if((nDataPacket-4) <= *ndata)
							{
								memcpy(sdata,(pBuffer+4),(nDataPacket-4));
								*ndata	= (nDataPacket-4);
							}
							else
							{
								*ndata	= 0;
								MESSAGE_OUT(("DATA %d %d\r\n",(pBuffer[1]&0xFF),nDataPacket));
							}
							m_pRTSPSessionStream->MoveBuffer(nDataPacket);
						}
						else
						{
							*ndata = 0;
						}
						return (*ndata);
					}
				}// if((pBuffer[0]&0xFF) == '$')
			}//if(pBuffer != NULL && nBuffer > 4)
			*ndata = 0;
			nError	= m_pRTSPSessionStream->ReadStream();
		}//if(m_pRTSPSessionStream != NULL)
		else
		{
			*ndata = 0;
		}
	}

	return 0;
}
int ZRTPSession::InterleavedSend(const void* sdata,const int ndata,const int nchannel)
{
	char	srtpdata[(DEFAULT_SESSION_STREAM_SIZE + 4)];
	int		nrtpdata	= 0;

	if(sdata != NULL && ndata > 0)
	{
		if(m_pRTSPSessionStream !=NULL)
		{
			srtpdata[(0)]	= '$';
			srtpdata[(1)]	= ((nchannel)&0xFF);
			srtpdata[(2)]	= ((ndata>>8)&0xFF);
			srtpdata[(3)]	= ((ndata)&0xFF);
			nrtpdata		= 4;
			if(ndata <= DEFAULT_SESSION_STREAM_SIZE)
			{
				memcpy(&srtpdata[4],sdata,ndata);
				nrtpdata	+= ndata;
                int nReturn = m_pRTSPSessionStream->SendStream(srtpdata,nrtpdata);
                if (nReturn == SESSION_STREAM_STATE_ERROR)
                {
                    if (m_nSendErrorStartTime == 0)
                    {
                        m_nSendErrorStartTime = ZOS::milliseconds();
                    }
                    if (ZOS::milliseconds() - m_nSendErrorStartTime > 5000)
                    {//5 sec send error, make sesstion time out.
                        LOG_DEBUG(("[ZRTPSession::InterleavedSend] 5 second send data error.set session timeout\r\n"));
                        ((ZRTSPSession*)m_pRTSPSession)->AddEvent(TASK_TIMEOUT_EVENT);
                        m_nSendErrorStartTime = 0;
                    }
                }
                else
                {
                    m_nSendErrorStartTime = 0;
                }
				return nReturn;
                //return m_pRTSPSessionStream->SendStreamNoWritableCheck(srtpdata,nrtpdata);
			}else{
                int nReturn;
				m_pRTSPSessionStream->SendStream(srtpdata,nrtpdata);
				nReturn = m_pRTSPSessionStream->SendStream((char*)sdata,ndata);
                if (nReturn == SESSION_STREAM_STATE_ERROR)
                {
                    if (m_nSendErrorStartTime == 0)
                    {
                        m_nSendErrorStartTime = ZOS::milliseconds();
                    }
                    if (ZOS::milliseconds() - m_nSendErrorStartTime > 5000)
                    {//5 sec send error, make sesstion time out.
                        LOG_DEBUG(("[ZRTPSession::InterleavedSend] 5 second send data error.set session timeout\r\n"));
                        ((ZRTSPSession*)m_pRTSPSession)->AddEvent(TASK_TIMEOUT_EVENT);
                    }
                }
                else
                {
                    m_nSendErrorStartTime = 0;
                }

                return nReturn;
                //m_pRTSPSessionStream->SendStreamNoWritableCheck(srtpdata,nrtpdata);
                //return m_pRTSPSessionStream->SendStreamNoWritableCheck((char*)sdata,ndata);
            }
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPSession::Play()
{
	int				i	= 0;
	ZOSMutexLocker	locker(&m_SessionMutex);

#ifdef	ZRTPSESSION_DEBUG
	MESSAGE_OUT(("ZRTPSession::Play\r\n"));
#endif	//ZRTPSESSION_DEBUG

	AddEvent(ZTask::TASK_START_EVENT);

	for(i = 0; i < m_StreamArray.Count(); i ++)
	{
		if(m_StreamArray[i] != NULL)
		{
			m_StreamArray[i]->Resume();
		}
	}
	m_nStartTime	= ZOS::milliseconds();
	m_nTotalTime	= 0;
	m_nLastTime		= m_nStartTime;

	SetSessionState(SESSION_STATE_PLAY);
	return FALSE;
}
BOOL ZRTPSession::Pause()
{
	ZOSMutexLocker	locker(&m_SessionMutex);

#ifdef	ZRTPSESSION_DEBUG
	MESSAGE_OUT(("ZRTPSession::Pause\r\n"));
#endif	//ZRTPSESSION_DEBUG

	SetSessionState(SESSION_STATE_PAUSE);

	return TRUE;
}
BOOL ZRTPSession::TearDown()
{
	int				i	= 0;
	ZOSMutexLocker	locker(&m_SessionMutex);

#ifdef	ZRTPSESSION_DEBUG
	MESSAGE_OUT(("ZRTPSession::TearDown\r\n"));
#endif	//ZRTPSESSION_DEBUG

	for(i = 0; i < m_StreamArray.Count(); i ++)
	{
		if(m_StreamArray[i] != NULL)
		{
			m_StreamArray[i]->Pause();
			delete m_StreamArray[i];
		}
	}
	m_StreamArray.RemoveAll();

	m_nStreamCount	= 0;

	SetSessionState(SESSION_STATE_STOP);
	return TRUE;
}
BOOL ZRTPSession::DoTaskKill()
{
#ifdef	ZRTPSESSION_DEBUG
	MESSAGE_OUT(("ZRTPSession::DoTaskKill\r\n"));
#endif	//ZRTPSESSION_DEBUG
	
	ZRTPSession::TearDown();

	SetSessionState(SESSION_STATE_STOP);
	
	return TRUE;
}
BOOL ZRTPSession::DoTimeOut()
{
//#ifdef	ZRTPSESSION_DEBUG
	MESSAGE_OUT(("ZRTPSession::DoTimeOut\r\n"));
//#endif	//ZRTPSESSION_DEBUG
	
	ZRTPSession::TearDown();

	SetSessionState(SESSION_STATE_TIMEOUT);
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
int ZRTPSession::ServerRun(UINT nEvent)
{
	UINT64	nCurrentTime	= 0;
	int		nTaskCount		= m_nTaskCount;

#if 0/*test for 1 sec send times*/
    if (m_nLastPrintTime == 0)
    {
        m_nLastPrintTime = ZOS::milliseconds();
    }
    m_nInServerRun ++;
    if (ZOS::milliseconds() - m_nLastPrintTime > 1000)
    {
        LOG_DEBUG(("[ZRTPSession::ServerRun] each second send count %d\r\n", m_nInServerRun));
        m_nLastPrintTime = ZOS::milliseconds();
        m_nInServerRun = 0;
    }
#endif

    nTaskCount = 250;
    //if (PoolNearFull())
    //{
    //    nTaskCount = 30;
    //    //LOG_DEBUG(("[ZRTPSession::ServerRun] PoolNearFull\r\n"));
    //}

	if(GetSessionState() == SESSION_STATE_PLAY)
	{
		//INT64 nCurTime = ZOS::microseconds();
		while(nTaskCount > 0)
		{
			if(ZRTPSession::SendRTPPackets())
			{
				nTaskCount	--;
			}
			else
			{
				break;
			}
		}
		//LOG_DEBUG(("[ZRTPSession::ServerRun] %d %d\r\n", nTaskCount, this));

		if(ZRTPSession::RecvRTCPPackets())
		{
			m_bUpdate		= TRUE;
			ZTimeoutTask::RefreshTimeout();
			if(m_pRTSPSession != NULL)
			{
				m_pRTSPSession->RefreshTimeout();
			}
			m_bUpdate		= FALSE;
			//LOG_DEBUG(("[ZRTPSession::ServerRun] RefreshTimeout\r\n"));
		}

		nCurrentTime	= ZOS::milliseconds();
		m_nTotalTime	= nCurrentTime - m_nStartTime;
		if(nCurrentTime - m_nLastTime > m_nIdleTime)
		{
			if(m_bUpdate)
			{
				ZTimeoutTask::RefreshTimeout();
				if(m_pRTSPSession != NULL)
				{
					m_pRTSPSession->RefreshTimeout();
				}
				//LOG_DEBUG(("[ZRTPSession::ServerRun] RefreshTimeout In > m_nIdleTime\r\n"));
			}
			//ZRTPSession::SendRTCPPackets();
			m_nLastTime		= nCurrentTime;
			m_bUpdate		= FALSE;
		}
	}

	return m_nTaskTime;
}
int ZRTPSession::ClientRun(UINT nEvent)
{
	UINT64	nCurrentTime	= 0;
	int		nTaskCount		= m_nTaskCount;

	if(GetSessionState() == SESSION_STATE_PLAY)
	{
		while(nTaskCount > 0)
		{
			if(ZRTPSession::RecvRTPPackets())
			{
				m_bUpdate	= TRUE;
				nTaskCount	--;
			}
			else
			{
				break;
			}
		}

		if(ZRTPSession::RecvRTCPPackets())
		{
			m_bUpdate		= TRUE;
			ZTimeoutTask::RefreshTimeout();
			if(m_pRTSPSession != NULL)
			{
                //LOG_DEBUG(("[ZRTPSession::ClientRun] recv rtcp RefreshTimeout\r\n"));
				m_pRTSPSession->RefreshTimeout();
			}
			m_bUpdate		= FALSE;
		}

		nCurrentTime	= ZOS::milliseconds();
		m_nTotalTime	= nCurrentTime - m_nStartTime;
		if(nCurrentTime - m_nLastTime > m_nIdleTime)
		{
			ZRTPSession::SendRTCPPackets();
			if(m_bUpdate)
			{
				//LOG_DEBUG(("[ZRTPSession::ClientRun] RefreshTimeout\r\n"));
                ZTimeoutTask::RefreshTimeout();
				if(m_pRTSPSession != NULL)
				{
					m_pRTSPSession->RefreshTimeout();
				}

                if (m_nSessionTimeoutMilliSecond != GetTimeout())
                {
                    SetTimeout(m_nSessionTimeoutMilliSecond);
                }
			}
			m_nLastTime		= nCurrentTime;
			m_bUpdate		= FALSE;
		}
	}

	return m_nTaskTime;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPSession::IsMulticast()
{
	ZOSMutexLocker	locker(&m_SessionMutex);
	return (m_nMultiAddr != 0);
}
///////////////////////////////////////////////////////////////////////////////
void ZRTPSession::SetMulticastIP(int nMultiAddr)
{
	ZOSMutexLocker	locker(&m_SessionMutex);
	m_nMultiAddr = nMultiAddr;
}
///////////////////////////////////////////////////////////////////////////////
int ZRTPSession::GetMulticastIP()
{
	return m_nMultiAddr;
}

int ZRTPSession::SetTaskCount(int nTaskCount)
{
    m_nTaskCount = nTaskCount;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
UINT ZRTPSession::m_nRecvFreq	= DEFAULT_RECV_TASK_TIME;
UINT ZRTPSession::m_nRecvCount	= DEFAULT_RECV_TASK_COUNT;
UINT ZRTPSession::m_nSendFreq	= DEFAULT_SEND_TASK_TIME;
UINT ZRTPSession::m_nSendCount	= DEFAULT_SEND_TASK_COUNT;

UINT ZRTPSession::GetRecvFreq()
{
	return m_nRecvFreq;
}

void ZRTPSession::SetRecvFreq(UINT nRecvFreq)
{
	m_nRecvFreq = nRecvFreq;
}

UINT ZRTPSession::GetRecvCount()
{
	return m_nRecvCount;
}

void ZRTPSession::SetRecvCount(UINT nRecvCount)
{
	m_nRecvCount = nRecvCount;
}
UINT ZRTPSession::GetSendFreq()
{
	return m_nSendFreq;
}

void ZRTPSession::SetSendFreq(UINT nSendFreq)
{
	m_nSendFreq = nSendFreq;
}

UINT ZRTPSession::GetSendCount()
{
	return m_nSendCount;
}

void ZRTPSession::SetSendCount(UINT nSendCount)
{
	m_nSendCount = nSendCount;
}


BOOL ZRTPSession::AddRTPPacket(const char *pRtpData, const int nRtpDataSize)
{
    ZRTPPacket*		pPacket = NULL;

    {
        ZOSMutexLocker	locker(&m_RTPPacketMutex);
        pPacket	= (ZRTPPacket*)m_RTPPacketPool.GetFree();
    }

    if (pPacket != NULL)
    {
        pPacket->SetPacketData(pRtpData, nRtpDataSize);

        {
            ZOSMutexLocker  locker(&m_RTPPacketMutex);
            m_RTPPacketPool.SetUsed(pPacket);
        }
    }
    else
    {
        LOG_DEBUG(("[ZRTPSession::AddRTPPacket] Pool GetFree = NULL\r\n"));
    }

    return (pPacket != NULL);
}

ZRTPPacket* ZRTPSession::GetUsedRTPPacket()
{
    ZOSMutexLocker	locker(&m_RTPPacketMutex);

    return (ZRTPPacket*)m_RTPPacketPool.GetUsed();
}

void ZRTPSession::SetRTPPacketFree(ZRTPPacket *pRTPPacket)
{
    if (pRTPPacket != NULL)
    {
        ZOSMutexLocker  locker(&m_RTPPacketMutex);

        m_RTPPacketPool.SetFree(pRTPPacket);
    }
}

///////////////////////////////////////////////////////////////////////////////
