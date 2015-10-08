#include "ZRTPStream.h"
#include "ZTCPSocket.h"
#include "ZUDPSocket.h"
#include "ZRTPSocketPool.h"
#include "ZRTPProtocol.h"
#include "ZRTPSession.h"
#include "ZRTSPSession.h"
#include "ZRTSPRequest.h"
#include "ZRTSPResponse.h"
#include "ZRTPPacket.h"
#include "ZRTCPPacket.h"
#include "ZRTCPTask.h"
///////////////////////////////////////////////////////////////////////////////
#define	ZRTPSTREAM_DEBUG		1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZRTPStream::ZRTPStream(ZRTPSession* pRTPSession,ZDataOut* pInputDataPin)
:ZDataInOut(pInputDataPin)
,m_pSession(pRTPSession)
,m_pUDPSocketPair(NULL)
,m_pRTCPTask(NULL)
,m_pRTPPacket(NULL)
,m_RTPPacketMutex("RTPPacketMutex")
,m_RTPPacketPool(0)
,m_RTCPPacketMutex("RTCPPacketMutex")
,m_RTCPPacketPool(0)
,m_bStreamPackets(FALSE)
,m_bServerStream(FALSE)
,m_nServerSSRC(0)
,m_nClientSSRC(0)
,m_nSequence(0)
,m_nLastTimestamp(0)
,m_nTransport(0)
,m_nRemoteAddr(0)
,m_nRemotePortA(0)
,m_nRemotePortB(0)
,m_nChannelA(-1)
,m_nChannelB(-1)
,m_nTotalPacketCount(0)
,m_nLostPacketCount(0)
{
	//LOG_DEBUG(("[ZRTPStream::ZRTPStream] in construct\r\n"));
}
ZRTPStream::~ZRTPStream()
{
	//LOG_DEBUG(("[ZRTPStream::ZRTPStream] in destruct\r\n"));
	ZDataInOut::SetDataPin(NULL);

	if(m_pRTCPTask != NULL)
	{
		m_pRTCPTask->UnRegisterStream(this);
		m_pRTCPTask = NULL;
	}

	ZRTPSocketPool::ReleaseUDPSocketPair(m_pUDPSocketPair);
	{
		ZOSMutexLocker	locker(&m_RTPPacketMutex);
		m_RTPPacketPool.SetSize(0);
	}
	{
		ZOSMutexLocker	locker(&m_RTCPPacketMutex);
		m_RTCPPacketPool.SetSize(0);
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPStream::OnCommand(int nCommand)
{
	ZDataInOut::OnCommand(nCommand);

	if (nCommand == DEFAULT_COMMAND_CLOSE && m_pSession != NULL)
	{
		m_pSession->OnDataPinClose();
	}
	return TRUE;
}
BOOL ZRTPStream::OnHeader(int nFlag,void* pHeader,int nHeader)
{
	return FALSE;
}
BOOL ZRTPStream::OnData(int nFlag,void* pData,int nData)
{
	if(nData > 0)
	{
        m_nTotalPacketCount ++;
		ZRTPStream::OnRecvRTPData(nFlag,pData,nData);
	}
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSObject* ZRTPStream::GetObject()
{
	return &m_StreamObject;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ZRTPStream::GetTransport()
{
	return m_nTransport;
}
int ZRTPStream::GetLocalPortA()
{
	if(m_pUDPSocketPair != NULL)
	{
		return m_pUDPSocketPair->GetUDPSocketA()->GetLocalPort();
	}
	return 0;
}
int ZRTPStream::GetLocalPortB()
{
	if(m_pUDPSocketPair != NULL)
	{
		return m_pUDPSocketPair->GetUDPSocketB()->GetLocalPort();
	}
	return 0;
}
UINT64 ZRTPStream::GetTotalPacketCount()
{
    return m_nTotalPacketCount;
}
UINT64 ZRTPStream::GetLostPacketCount()
{
    return m_nLostPacketCount;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ZRTPStream::SetTransport(int nTransport)
{
	m_nTransport	= nTransport;
}
///////////////////////////////////////////////////////////////////////////////
void ZRTPStream::SetRemotePort(int nPortA,int nPortB)
{
	m_nRemotePortA	= nPortA;
	m_nRemotePortB	= nPortB;
}
///////////////////////////////////////////////////////////////////////////////
void ZRTPStream::SetChannel(int nChannelA,int nChannelB)
{
	m_nChannelA	= nChannelA;
	m_nChannelB	= nChannelB;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPStream::OnSendRTPPacket()
{
	BOOL				bReturn		= FALSE;

	if(m_bStreamPackets)
	{
		ZRTPPacket*		pPacket		= NULL;
		char*			pData		= NULL;
		int				nData		= 0;

		if(m_pRTPPacket == NULL)
		{
			ZOSMutexLocker locker(&m_RTPPacketMutex);
			pPacket = (ZRTPPacket*)m_RTPPacketPool.GetUsed();
		}
		else
		{
			pPacket = m_pRTPPacket;
			m_pRTPPacket = NULL;
		}

		if(pPacket != NULL)
		{
			pData	= pPacket->GetPacketPointer();
			nData	= pPacket->GetPacketLength();
			if(m_pUDPSocketPair != NULL)
			{
				if (m_pSession != NULL && m_pSession->IsMulticast())
				{
					if (m_pUDPSocketPair->UDPSocketASendTo((char*)pData,nData,m_pSession->GetMulticastIP(),m_nRemotePortA,NULL) != nData)
					{
						m_pRTPPacket = pPacket;
						pPacket = NULL;
					}
				}
				else
				{
					if (m_pUDPSocketPair->UDPSocketASendTo((char*)pData,nData,m_nRemoteAddr,m_nRemotePortA,NULL) != nData)
					{
						m_pRTPPacket = pPacket;
						pPacket = NULL;
						//INT64 nCurTime = ZOS::microseconds();
						//LOG_DEBUG(("[ZRTPStream::OnSendRTPPacket] Send RTP Packet failed!\r\n"));
						//LOG_DEBUG(("log time used %d\r\n", ZOS::microseconds()-nCurTime));
					}
				}
			}
			else
			{
				if(m_pSession != NULL)
				{
					m_pSession->InterleavedSend((char*)pData,nData,m_nChannelA);
				}
			}
			bReturn	= TRUE;
			
			{
				ZOSMutexLocker locker(&m_RTPPacketMutex);
				m_RTPPacketPool.SetFree(pPacket);
			}
		}
	}

	return bReturn;
}
BOOL ZRTPStream::OnSendRTCPPacket()
{
	BOOL	bReturn	= FALSE;

	if(m_bStreamPackets)
	{
		if(m_bServerStream)
		{
			ZRTCPSRPacket	Packet(DEFAULT_RTCP_BUFFER_USED);

			Packet.SetPacketSSRC(m_nServerSSRC);
			Packet.AddReportBlock(m_nClientSSRC,0,0,m_nSequence,m_nLastTimestamp,0);
			if(m_pUDPSocketPair != NULL)
			{
				if (m_pSession != NULL && m_pSession->IsMulticast())
				{
					//MESSAGE_OUT(("[ZRTPStream::OnSendRTCPPacket] rtcp ip %s %d\r\n", ZSocket::ConvertAddr(m_pSession->GetMulticastIP()), m_nRemotePortB));
					m_pUDPSocketPair->UDPSocketBSendTo((char*)Packet.GetPacket(),Packet.GetPacketLength(),m_pSession->GetMulticastIP(),(m_nRemotePortB),NULL);
				}
				else
				{
					m_pUDPSocketPair->UDPSocketBSendTo((char*)Packet.GetPacket(),Packet.GetPacketLength(),(m_nRemoteAddr),(m_nRemotePortB),NULL);
				}
				bReturn	= TRUE;
			}else{
				if(m_pSession != NULL)
				{
					m_pSession->InterleavedSend((char*)Packet.GetPacket(),Packet.GetPacketLength(),m_nChannelB);
					bReturn	= TRUE;
				}
			}
		}else{
			CHAR			sBuffer[DEFAULT_RTCP_BUFFER_USED];
			UINT			nBuffer	= 0;
			ZRTCPRRPacket	Packet0(DEFAULT_RTCP_BUFFER_USED);
			ZRTCPSDESPacket	Packet1(DEFAULT_RTCP_BUFFER_USED);

			Packet0.SetPacketSSRC(m_nClientSSRC);
			Packet0.AddReportBlock(m_nServerSSRC,0,0,m_nSequence,m_nLastTimestamp,0);

			Packet1.SetPacketSSRC(m_nClientSSRC);
			Packet1.AddReportBlock(m_nClientSSRC,"ZMedia");

			nBuffer	= 0;
			memcpy(&sBuffer[nBuffer],Packet0.GetPacket(),Packet0.GetPacketLength());
			nBuffer	+= Packet0.GetPacketLength();
			memcpy(&sBuffer[nBuffer],Packet1.GetPacket(),Packet1.GetPacketLength());
			nBuffer	+= Packet1.GetPacketLength();
			if(m_pUDPSocketPair != NULL)
			{
				m_pUDPSocketPair->UDPSocketBSendTo(sBuffer,nBuffer,(m_nRemoteAddr),(m_nRemotePortB),NULL);
				bReturn	= TRUE;
			}else{
				if(m_pSession != NULL)
				{
					m_pSession->InterleavedSend(sBuffer,nBuffer,m_nChannelB);
					bReturn	= TRUE;
				}
			}
		}
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPStream::OnRecvRTPPacket()
{
	BOOL	bReturn	= FALSE;

	if(m_bStreamPackets)
	{
		char	sData[DEFAULT_RTP_BUFFER_MAX];
		int		nData	= DEFAULT_RTP_BUFFER_MAX;

		if(m_pUDPSocketPair != NULL)
		{
			nData	= m_pUDPSocketPair->UDPSocketARecvFrom((char*)sData,nData,NULL,NULL,NULL);
			//LOG_DEBUG(("[ZRTPStream::OnRecvRTPPacket] data size %d\r\n", nData));
		}else{
			if(m_pSession != NULL)
			{
				nData	= m_pSession->InterleavedRead(sData,&nData,m_nChannelA);
			}
		}
		if(nData > 0)
		{
			ZRTPPacket	Packet;
			Packet.SetPacketData(sData,nData);
			ZRTPStream::OnProcessPacket(&Packet);

            m_nTotalPacketCount ++;

			ZDataInOut::SendData(1,sData,nData);
			bReturn	= TRUE;
		}
	}
	return bReturn;
}
BOOL ZRTPStream::OnRecvRTCPPacket()
{
	BOOL	bReturn	= FALSE;

	if(m_bStreamPackets)
	{
		char	sData[DEFAULT_RTP_BUFFER_MAX];
		int		nData	= DEFAULT_RTP_BUFFER_MAX;
		int		nRead	= 0;

		if(m_pUDPSocketPair != NULL)
		{
			if(m_pRTCPTask != NULL)
			{
				ZOSMutexLocker	locker(&m_RTCPPacketMutex);
				ZRTCPPacket*	pPacket	= NULL;

				pPacket	= (ZRTCPPacket*)m_RTCPPacketPool.GetUsed();
				if(pPacket != NULL)
				{
					nRead	= pPacket->GetPacketLength();
					m_RTCPPacketPool.SetFree(pPacket);
					//LOG_DEBUG(("[ZRTPStream::OnRecvRTCPPacket] IP:%s, Port:%d, nData:%d\r\n",
					//	ZSocket::ConvertAddr(m_nRemoteAddr), m_nRemotePortB, nData));

					pPacket = m_RTCPPacketPool.GetUsed();
					while (pPacket != NULL)
					{
						m_RTCPPacketPool.SetFree(pPacket);
						pPacket = m_RTCPPacketPool.GetUsed();
					}
				}
				else
				{
					nRead	= 0;
					//LOG_DEBUG(("[ZRTPStream::OnRecvRTCPPacket] Packet = NULL, IP:%s, Port:%d, nData:%d\r\n",
					//	ZSocket::ConvertAddr(m_nRemoteAddr), m_nRemotePortB, nData));
				}
			}else{
				nRead	= m_pUDPSocketPair->UDPSocketBRecvFrom((char*)sData,nData,NULL,NULL,NULL);
			}
		}else{
			if(m_pSession != NULL)
			{
				if (!m_bServerStream)
				{
					nRead	= m_pSession->InterleavedRead(sData,&nData,m_nChannelB);
				}
			}
		}
		if(nRead > 0)
		{
			ZRTCPPacket	Packet;
			Packet.SetPacketData(sData,nRead);
			ZRTPStream::OnProcessPacket(&Packet);
			bReturn	= TRUE;
		}
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPStream::SetupServerStream(ZRTSPRequest* pRTSPRequest,int nRtpSize)
{
	BOOL	bReturn	= FALSE;

	m_bStreamPackets	= FALSE;
	if(pRTSPRequest != NULL)
	{
		RTSP_PROTOCOL_TRANSPORT	Type	= (RTSP_PROTOCOL_TRANSPORT)pRTSPRequest->GetTransportType();
		if(Type == RTSP_PROTOCOL_TRANSPORT_UDP)
		{
			m_pUDPSocketPair	= ZRTPSocketPool::GetUDPSocketPair();
			if(m_pUDPSocketPair != NULL)
			{
				m_nRemoteAddr	= pRTSPRequest->GetSessionAddr();
				m_nRemotePortA	= pRTSPRequest->GetClientPortA();
				m_nRemotePortB	= pRTSPRequest->GetClientPortB();
				bReturn			= TRUE;
			}
		}else if(Type == RTSP_PROTOCOL_TRANSPORT_TCP)
		{
			ASSERT((m_pUDPSocketPair==NULL));
			bReturn			= TRUE;
		}
		m_bServerStream	= TRUE;
		m_StreamObject.Set((m_nRemoteAddr<<16|m_nRemotePortB),this);
		MESSAGE_OUT(("[ZRTPStream::SetupServerStream] remote addr %s remote port %d\r\n", 
			ZSocket::ConvertAddr(m_nRemoteAddr), m_nRemotePortB));
		m_RTPPacketPool.SetSize(nRtpSize);
		m_RTCPPacketPool.SetSize(10);
		m_pRTCPTask	= ZRTCPTask::GetRTCPTask();
		if(m_pRTCPTask != NULL)
		{
			m_pRTCPTask->RegisterStream(this);
		}
	}

	return bReturn;
}
BOOL ZRTPStream::SetupServerStreamMulticast(CHAR *sMultiIP, int nPortA, int nPortB, int nRtpSize)
{
	BOOL	bReturn	= FALSE;

	m_bStreamPackets	= FALSE;
	if(sMultiIP != NULL)
	{
		m_pUDPSocketPair	= ZRTPSocketPool::GetUDPSocketPair();
		if(m_pUDPSocketPair != NULL)
		{
			m_nRemoteAddr	= ZSocket::ConvertAddr(sMultiIP);
			m_nRemotePortA	= nPortA;
			m_nRemotePortB	= nPortB;
			bReturn			= TRUE;
		}

		m_bServerStream	= TRUE;
		m_StreamObject.Set((m_nRemoteAddr<<16|m_nRemotePortB),this);
		MESSAGE_OUT(("[ZRTPStream::SetupServerStreamMulticast] remote addr %s remote port %d\r\n", 
			ZSocket::ConvertAddr(m_nRemoteAddr), m_nRemotePortB));
		m_RTPPacketPool.SetSize(nRtpSize);
		m_RTCPPacketPool.SetSize(10);
		m_pRTCPTask	= ZRTCPTask::GetRTCPTask();
		if(m_pRTCPTask != NULL)
		{
			m_pRTCPTask->RegisterStream(this);
		}
	}

	return bReturn;
}

BOOL ZRTPStream::SetupClientStream(ZRTSPRequest* pRTSPRequest)
{
	BOOL	bReturn	= FALSE;

	m_bStreamPackets	= FALSE;
	if(pRTSPRequest != NULL)
	{
		RTSP_PROTOCOL_TRANSPORT	Type	= (RTSP_PROTOCOL_TRANSPORT)pRTSPRequest->GetTransportType();
		if(Type == RTSP_PROTOCOL_TRANSPORT_UDP)
		{
			m_pUDPSocketPair	= ZRTPSocketPool::SetupUDPSocketPair(0);
			if(m_pUDPSocketPair != NULL)
			{
				m_pUDPSocketPair->GetUDPSocketA()->SetNonBlocking();
				m_pUDPSocketPair->GetUDPSocketB()->SetNonBlocking();
				m_pUDPSocketPair->GetUDPSocketA()->SetRecvBufferSize(DEFAULT_UDP_RECV_BUFFER_SIZE);
				m_pUDPSocketPair->GetUDPSocketA()->SetTimeOut(DEFAULT_UDP_SEND_TIMEOUT,DEFAULT_UDP_RECV_TIMEOUT);
				pRTSPRequest->SetClientPort(ZRTPStream::GetLocalPortA(),ZRTPStream::GetLocalPortB());
				m_nRemoteAddr		= pRTSPRequest->GetSessionAddr();
				m_bStreamPackets	= TRUE;
				bReturn				= TRUE;
			}
		}else if(Type == RTSP_PROTOCOL_TRANSPORT_TCP)
		{
			ASSERT((m_pUDPSocketPair==NULL));
			m_nChannelA   = pRTSPRequest->GetChannelA();
			m_nChannelB   = pRTSPRequest->GetChannelB();
			m_bStreamPackets	= TRUE;
			bReturn				= TRUE;
		}
		m_bServerStream	= FALSE;
	}

	return bReturn;
}

BOOL ZRTPStream::SetupClientStreamMulticast(ZRTSPRequest* pRTSPRequest,DWORD nAddr,DWORD nPort)
{
	BOOL	bReturn	= FALSE;

	m_bStreamPackets	= FALSE;
	if(pRTSPRequest != NULL)
	{
		RTSP_PROTOCOL_TRANSPORT	Type	= (RTSP_PROTOCOL_TRANSPORT)pRTSPRequest->GetTransportType();
		if(Type == RTSP_PROTOCOL_TRANSPORT_UDP)
		{
			m_pUDPSocketPair	= ZRTPSocketPool::SetupUDPSocketPairMulticast(nAddr,nPort);
			if(m_pUDPSocketPair != NULL)
			{
				m_pUDPSocketPair->GetUDPSocketA()->SetNonBlocking();
				m_pUDPSocketPair->GetUDPSocketB()->SetNonBlocking();
				m_pUDPSocketPair->GetUDPSocketA()->SetRecvBufferSize(DEFAULT_UDP_RECV_BUFFER_SIZE);
				m_pUDPSocketPair->GetUDPSocketA()->SetTimeOut(DEFAULT_UDP_SEND_TIMEOUT,DEFAULT_UDP_RECV_TIMEOUT);
				pRTSPRequest->SetClientPort(nPort,nPort + 1);
				m_nRemoteAddr		= pRTSPRequest->GetSessionAddr();
				m_bStreamPackets	= TRUE;
				bReturn				= TRUE;
			}
		}
		m_bServerStream	= FALSE;
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPStream::OnProcessPacket(ZRTPPacket* pPacket)
{
	BOOL	bRTP	= FALSE;

	if(pPacket != NULL)
	{
		if(pPacket->IsValid())
		{
			bRTP	= TRUE;
			if(m_nServerSSRC == 0)
			{
				m_nServerSSRC	= pPacket->GetPacketSSRC();
			}else{
				if(m_nServerSSRC != pPacket->GetPacketSSRC())
				{
					bRTP	= FALSE;
#ifdef	ZRTPSTREAM_DEBUG
					MESSAGE_OUT(("ZRTPStream::OnRecvRTPPacket BAD SSRC\r\n"));
#endif	//ZRTPSTREAM_DEBUG
				}
			}
			m_nSequence	= pPacket->GetSequence();
		}else{
#ifdef	ZRTPSTREAM_DEBUG
			MESSAGE_OUT(("ZRTPStream::OnRecvRTPPacket BAD Packet\r\n"));
#endif	//ZRTPSTREAM_DEBUG
		}
		if(bRTP)
		{

		}
	}
	return TRUE;
}
BOOL ZRTPStream::OnProcessPacket(ZRTCPPacket* pPacket)
{
	BOOL	bReturn		= FALSE;
	
	if(pPacket != NULL )
	{
		if(pPacket->IsValid())
		{
			bReturn		= TRUE;
			switch(pPacket->GetPacketType())
			{
			case ZRTCPPacket::RTCP_PACKET_SR:
				{
					ZRTCPSRPacket	Packet(pPacket->GetPacketLength(),pPacket->GetBuffer());
					m_nLastTimestamp = Packet.GetRTPTimestamp();
                    
                    ZDataInOut::SendUserData(1, &Packet);
				}
				break;
			case ZRTCPPacket::RTCP_PACKET_RR:
				{
				}
				break;
			case ZRTCPPacket::RTCP_PACKET_SDES:
				{
				}
				break;
			case ZRTCPPacket::RTCP_PACKET_BYE:
				{
				}
				break;
			case ZRTCPPacket::RTCP_PACKET_APP:
				{
				} 
				break;
			default:
				{
				}
				break;
			}
		}
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPStream::Pause()
{
	m_bStreamPackets	= FALSE;
	return TRUE;
}
BOOL ZRTPStream::Resume()
{
	m_bStreamPackets	= TRUE;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPStream::OnRecvRTPData(int nFlag,void* pData,int nData)
{
	if(m_bStreamPackets)
	{
		ZRTPPacket*		pPacket = NULL;
		
		{
			ZOSMutexLocker	locker(&m_RTPPacketMutex);
			pPacket	= (ZRTPPacket*)m_RTPPacketPool.GetFree();
		}
		
		if(pPacket != NULL)
		{
			//LOG_DEBUG(("[ZRTPStream::OnRecvRTPData] OnRecvRTPData m_RTPPacketPool Size = %d nData= %d\r\n", 
			//	m_RTPPacketPool.GetFreeSize(), nData));
			pPacket->SetPacketData((char*)pData,nData);
			
			{
				ZOSMutexLocker	locker(&m_RTPPacketMutex);
				m_RTPPacketPool.SetUsed(pPacket);
			}
		}
		else
		{
			LOG_DEBUG(("[ZRTPStream::OnRecvRTPData] Pool GetFree = NULL\r\n"));
		}
		return (pPacket != NULL);
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPStream::OnRecvRTCPData(int nFlag,void* pData,int nData)
{
	if(m_bStreamPackets)
	{
		ZRTCPPacket*	pPacket = NULL;

		{
			ZOSMutexLocker	locker(&m_RTCPPacketMutex);
			pPacket	= (ZRTCPPacket*)m_RTCPPacketPool.GetFree();
		}

		if(pPacket != NULL)
		{
			//LOG_DEBUG(("[ZRTPStream::OnRecvRTCPData] put rtcp packet in pool! ip:%s  port:%d  nData %d\r\n",
			//	ZSocket::ConvertAddr(m_nRemoteAddr), m_nRemotePortB, nData));
			pPacket->SetPacketData((char*)pData,nData);

			{
				ZOSMutexLocker	locker(&m_RTCPPacketMutex);
				m_RTCPPacketPool.SetUsed(pPacket);
			}
		}
		return (pPacket != NULL);
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
