#include "ZRTPSocketPool.h"
#include "ZUDPSocket.h"
#ifdef _LINUX_
#include <sys/time.h>
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//#define	ZRTPSOCKETPOOL_DEBUG	1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_RTP_UDP_PORT_BEGINE	0x1B44//6980;
#define	DEFAULT_RTP_UDP_PORT_END	0xFFFF//
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_RTP_UDP_PAIRS		1
///////////////////////////////////////////////////////////////////////////////
ZRTPSocketPool*	ZRTPSocketPool::m_pRTPSocketPool	= NULL;
///////////////////////////////////////////////////////////////////////////////
int ZUDPSocketPair::m_nPacketInterval =150;
UINT ZUDPSocketPair::m_nFilterPacketSize = 768;
///////////////////////////////////////////////////////////////////////////////
ZUDPSocketPair::ZUDPSocketPair()
:m_OSQueueElement()
,m_UDPSocketPairMutex("UDPSocketPairMutex")
,m_pUDPSocketA(NULL)
,m_pUDPSocketB(NULL)
,m_nUsedCount(0)
,m_nSendCount(0)
,m_bFrameStartFlag(TRUE)
,m_nFramePacCount(0)
{
	m_OSQueueElement.SetObject(this);
}
ZUDPSocketPair::~ZUDPSocketPair()
{
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZUDPSocketPair::Set(ZUDPSocket* pUDPSocketA,ZUDPSocket* pUDPSocketB)
{
	m_pUDPSocketA	= pUDPSocketA;
	m_pUDPSocketB	= pUDPSocketB;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
int ZUDPSocketPair::UDPSocketASendTo(const void* sdata,const int ndata,UINT RemoteAddr,UINT RemotePort,int* nSend)
{
	ZOSMutexLocker	locker(&m_UDPSocketPairMutex);

	if (m_pUDPSocketA != NULL)
	{
		bool bCanSend = false;

		if (ndata < (int)m_nFilterPacketSize)
		{
			bCanSend = true;
		}
		else
		{
			INT64 nLastMicroSec = ZOS::getlastmicrosec();
			INT64 nCurMicroSec = ZOS::microseconds();
			INT64 nDiff = nCurMicroSec - nLastMicroSec;
			if (nDiff > m_nPacketInterval || nDiff < 0)
			{
				bCanSend = true;
				ZOS::setlastmicrosec(nCurMicroSec);
			}
			else
			{
				//ZOS::sleepmicrosec(m_nPacketInterval-(int)nDiff);
				//printf("#:%d\n",m_nPacketInterval-(int)nDiff);
				nCurMicroSec = ZOS::microseconds();
				nDiff = nCurMicroSec - nLastMicroSec;
				if (nDiff > m_nPacketInterval || nDiff < 0)
				{
					bCanSend = true;
					ZOS::setlastmicrosec(nCurMicroSec);
				}
			}
		}

		if (bCanSend)
		{
			return m_pUDPSocketA->SendTo(sdata,ndata,RemoteAddr,RemotePort,nSend);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

/*int ZUDPSocketPair::UDPSocketASendTo(const void* sdata,const int ndata,UINT RemoteAddr,UINT RemotePort,int* nSend)
{
	ZOSMutexLocker	locker(&m_UDPSocketPairMutex);
	if (m_pUDPSocketA != NULL)
	{
		bool bCanSend = false;

		if (ndata < (int)m_nFilterPacketSize)
		{
			m_nSendCount = 0;
			bCanSend = true;
		}
		else
		{
			if(m_nSendCount > 0 && m_nSendCount % 4 == 0)
			{
				ZOS::sleepmicrosec(1000);
				m_nSendCount = 0;
				bCanSend = true;
			}
			else
			{
				m_nSendCount++;
			}

		}

		if (bCanSend)
		{
			return m_pUDPSocketA->SendTo(sdata,ndata,RemoteAddr,RemotePort,nSend);
		}
		else
		{
			return 0;
		}
	}
	return 0;
}*/

/*int ZUDPSocketPair::UDPSocketASendTo(const void* sdata,const int ndata,UINT RemoteAddr,UINT RemotePort,int* nSend)
{
	int nRet = 0;
	if (m_pUDPSocketA != NULL)
	{
		if(m_bFrameStartFlag == TRUE)
		{
			m_bFrameStartTime = ZOS::microseconds();
			m_nFramePacCount = 0;
			m_bFrameStartFlag = FALSE;
			nRet = m_pUDPSocketA->SendTo(sdata,ndata,RemoteAddr,RemotePort,nSend);
		}
		else
		{
			m_nFramePacCount++;
			INT64 nCurMicroSec = ZOS::microseconds();
			INT nDiff = (INT)(nCurMicroSec - m_bFrameStartTime);

			INT exP =  m_nFramePacCount * 1000;

			if (nDiff > exP || nDiff < 0)
			{
				nRet = m_pUDPSocketA->SendTo(sdata,ndata,RemoteAddr,RemotePort,nSend);
			}
			else
			{
				usleep(1);
			}
			
		}

		char* rtpHeader = (char*)sdata;
		DWORD	nHeader	= GETUINT32(&rtpHeader[0]);
		DWORD	nValue	= ntohl(nHeader);
		if((nValue&0x00800000UL) != 0)
		{
			m_bFrameStartFlag = TRUE;
		}
	}
	return nRet;
}*/

/*int ZUDPSocketPair::UDPSocketASendTo(const void* sdata,const int ndata,UINT RemoteAddr,UINT RemotePort,int* nSend)
{
	ZOSMutexLocker	locker(&m_UDPSocketPairMutex);
	if (m_pUDPSocketA != NULL)
	{
		//usleep(1);
		return m_pUDPSocketA->SendTo(sdata,ndata,RemoteAddr,RemotePort,nSend);

	}
	return 0;
}*/

int ZUDPSocketPair::UDPSocketBSendTo(const void* sdata,const int ndata,UINT RemoteAddr,UINT RemotePort,int* nSend)
{
	ZOSMutexLocker	locker(&m_UDPSocketPairMutex);
	if(m_pUDPSocketB != NULL)
	{
		return m_pUDPSocketB->SendTo(sdata,ndata,RemoteAddr,RemotePort,nSend);
	}
	return 0;
}
int ZUDPSocketPair::UDPSocketARecvFrom(void* sdata,const int ndata,UINT* RemoteAddr,UINT* RemotePort,int* nRecv)
{
	ZOSMutexLocker	locker(&m_UDPSocketPairMutex);
	if(m_pUDPSocketA != NULL)
	{
		return m_pUDPSocketA->RecvFrom(sdata,ndata,RemoteAddr,RemotePort,nRecv);
	}
	return 0;
}
int ZUDPSocketPair::UDPSocketBRecvFrom(void* sdata,const int ndata,UINT* RemoteAddr,UINT* RemotePort,int* nRecv)
{
	ZOSMutexLocker	locker(&m_UDPSocketPairMutex);
	if(m_pUDPSocketB != NULL)
	{
		return m_pUDPSocketB->RecvFrom(sdata,ndata,RemoteAddr,RemotePort,nRecv);
	}
	return 0;
}

int ZUDPSocketPair::GetPacketInterval()
{
	return m_nPacketInterval;
}

UINT ZUDPSocketPair::GetFilterPacketSize()
{
	return m_nFilterPacketSize;
}

void ZUDPSocketPair::SetPacketInterval(int nPacketInterval)
{
	m_nPacketInterval = nPacketInterval;
    LOG_DEBUG(("[ZUDPSocketPair::SetPacketInterval] m_nPacketInterval = %d\r\n",
        m_nPacketInterval));
}

void ZUDPSocketPair::SetFilterPacketSize(UINT nFilterPacketSize)
{
	m_nFilterPacketSize = nFilterPacketSize;
    LOG_DEBUG(("[ZUDPSocketPair::SetFilterPacketSize] m_nFilterPacketSize = %d\r\n",
        m_nFilterPacketSize));
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZRTCPTask* ZRTPSocketPool::m_pRTCPTask				= NULL;
ZOSMutex ZRTPSocketPool::m_RTPSocketPoolMutex("RTPSocketPoolMutex");
UINT ZRTPSocketPool::m_RTPPort		= DEFAULT_RTP_UDP_PORT_BEGINE;
UINT ZRTPSocketPool::m_nPairs		= DEFAULT_RTP_UDP_PAIRS;
UINT ZRTPSocketPool::m_nPairIndex	= 0;
///////////////////////////////////////////////////////////////////////////////
ZRTPSocketPool::ZRTPSocketPool()
:m_UDPSocketPairQueue("UDPSocketPairQueue")
{
}
ZRTPSocketPool::~ZRTPSocketPool()
{
	ZOSQueueElement*	pElement	= NULL;
	while((pElement = m_UDPSocketPairQueue.Pop()) != NULL)
	{
		ZRTPSocketPool::CloseUDPSocketPair((ZUDPSocketPair*)pElement->GetObject());
	}
}
///////////////////////////////////////////////////////////////////////////////
ZUDPSocketPair* ZRTPSocketPool::CreateUDPSocketPair(int nAddr,int nPort)
{
	ZUDPSocketPair*	pPair			= NULL;
	BOOL			bPair			= FALSE;
	ZUDPSocket*		pUDPSocketA		= NULL;
	ZUDPSocket*		pUDPSocketB		= NULL;
	int				nCurrentPortA	= 0;
	int				nCurrentPortB	= 0;

	{
		ZOSMutexLocker	locker(&m_RTPSocketPoolMutex);
		pUDPSocketA		= NEW ZUDPSocket();
		pUDPSocketB		= NEW ZUDPSocket();

		if(pUDPSocketA != NULL && pUDPSocketB != NULL)
		{
			if(pUDPSocketA->Create() && pUDPSocketB->Create())
			{
				if(nPort != 0)
				{
					nCurrentPortA	= nPort;
					nCurrentPortB	= (nPort + 1);
					if(pUDPSocketA->Bind(nAddr,nCurrentPortA) && pUDPSocketB->Bind(nAddr,nCurrentPortB))
					{
						pPair	= NEW ZUDPSocketPair();
						if(pPair != NULL)
						{
							pUDPSocketA->SetNonBlocking();
							pUDPSocketB->SetNonBlocking();
							pUDPSocketA->SetSendBufferSize(DEFAULT_UDP_SEND_BUFFER_SIZE);
							pUDPSocketB->SetSendBufferSize(DEFAULT_UDP_SEND_BUFFER_SIZE);
							pUDPSocketA->SetRecvBufferSize(DEFAULT_UDP_RECV_BUFFER_SIZE);
							pUDPSocketB->SetRecvBufferSize(DEFAULT_UDP_RECV_BUFFER_SIZE);
							pUDPSocketA->SetTimeOut(DEFAULT_UDP_SEND_TIMEOUT,DEFAULT_UDP_RECV_TIMEOUT);
							pUDPSocketB->SetTimeOut(DEFAULT_UDP_SEND_TIMEOUT,DEFAULT_UDP_RECV_TIMEOUT);
							pUDPSocketA->SetMulticastTTL(255);
							pUDPSocketB->SetMulticastTTL(255);
							pPair->Set(pUDPSocketA,pUDPSocketB);
							m_UDPSocketPairQueue.Push(&(pPair->m_OSQueueElement));
						}
					}
				}else{
					while(!bPair)
					{
						nCurrentPortA	= ZRTPSocketPool::GetRTPPort();
						nCurrentPortB	= (nCurrentPortA + 1);
						if(pUDPSocketA->Bind(nAddr,nCurrentPortA) && pUDPSocketB->Bind(nAddr,nCurrentPortB))
						{
							pPair	= NEW ZUDPSocketPair();
							if(pPair != NULL)
							{
								pUDPSocketA->SetNonBlocking();
								pUDPSocketB->SetNonBlocking();
								pUDPSocketA->SetSendBufferSize(DEFAULT_UDP_SEND_BUFFER_SIZE);
								pUDPSocketB->SetSendBufferSize(DEFAULT_UDP_SEND_BUFFER_SIZE);
								pUDPSocketA->SetRecvBufferSize(DEFAULT_UDP_RECV_BUFFER_SIZE);
								pUDPSocketB->SetRecvBufferSize(DEFAULT_UDP_RECV_BUFFER_SIZE);
								pUDPSocketA->SetTimeOut(DEFAULT_UDP_SEND_TIMEOUT,DEFAULT_UDP_RECV_TIMEOUT);
								pUDPSocketB->SetTimeOut(DEFAULT_UDP_SEND_TIMEOUT,DEFAULT_UDP_RECV_TIMEOUT);
								pUDPSocketA->SetMulticastTTL(255);
								pUDPSocketB->SetMulticastTTL(255);
								pPair->Set(pUDPSocketA,pUDPSocketB);
								m_UDPSocketPairQueue.Push(&(pPair->m_OSQueueElement));
								pPair->m_nUsedCount	++;
								break;
							}
						}
						pUDPSocketA->Unbind();
						pUDPSocketB->Unbind();
					}
				}
			}
		}
		if(pPair == NULL)
		{
			SAFE_DELETE(pUDPSocketA);
			SAFE_DELETE(pUDPSocketB);
		}
	}

	return pPair;
}
ZUDPSocketPair* ZRTPSocketPool::CloseUDPSocketPair(ZUDPSocketPair* pPair)
{
	if(pPair != NULL)
	{
		ZUDPSocket*	pUDPSocket	= NULL;
		{
			TMASSERT((pPair->m_nUsedCount >= 0))
			if(pPair->m_nUsedCount > 0)
			{
				pPair->m_nUsedCount	--;
			}
			if(pPair->m_nUsedCount==0)
			{
				m_UDPSocketPairQueue.Remove(&(pPair->m_OSQueueElement));
				pUDPSocket	= pPair->GetUDPSocketA();
				SAFE_DELETE(pUDPSocket);
				pUDPSocket	= pPair->GetUDPSocketB();
				SAFE_DELETE(pUDPSocket);
				SAFE_DELETE(pPair);
			}
		}
	}
	return pPair;
}
///////////////////////////////////////////////////////////////////////////////
ZUDPSocketPair* ZRTPSocketPool::GetUDPSocketPair()
{
	ZUDPSocketPair*	pPair	= NULL;

	if(m_pRTPSocketPool != NULL)
	{
		ZOSMutexLocker	locker(&m_RTPSocketPoolMutex);
		ZOSQueue::ELEMENT_FIND_HANDLE	handle;
		ZOSQueueElement*				pElement	= NULL;	
		int								nIndex		= (m_nPairIndex%m_nPairs);

		pElement	= m_pRTPSocketPool->m_UDPSocketPairQueue.GetFirst(&handle);
		while(nIndex > 0)
		{
			pElement	= m_pRTPSocketPool->m_UDPSocketPairQueue.GetNext(&handle);
			nIndex		--;
		}
		if(pElement != NULL)
		{
			pPair	= (ZUDPSocketPair*)pElement->GetObject();
			if(pPair != NULL)
			{
				pPair->m_nUsedCount	++;
			}
#ifdef	ZRTPSOCKETPOOL_DEBUG
			MESSAGE_OUT(("ZRTPSocketPool::GetUDPSocketPair(PortA %d PortB %d UsedCount %d)\r\n",pPair->GetUDPSocketA()->GetLocalPort(),pPair->GetUDPSocketB()->GetLocalPort(),pPair->m_nUsedCount));
#endif	//ZRTPSOCKETPOOL_DEBUG
		}
		m_nPairIndex	++;
	}

	return pPair;
}
ZUDPSocketPair*	ZRTPSocketPool::SetupUDPSocketPair(int nAddr)
{
	ZUDPSocketPair*	pPair			= NULL;
	BOOL			bPair			= FALSE;
	ZUDPSocket*		pUDPSocketA		= NULL;
	ZUDPSocket*		pUDPSocketB		= NULL;
	int				nCurrentPortA	= 0;
	int				nCurrentPortB	= 0;

	{
		ZOSMutexLocker	locker(&m_RTPSocketPoolMutex);
		pUDPSocketA		= NEW ZUDPSocket();
		pUDPSocketB		= NEW ZUDPSocket();

		if(pUDPSocketA != NULL && pUDPSocketB != NULL)
		{
			if(pUDPSocketA->Create() && pUDPSocketB->Create())
			{
				while(!bPair)
				{
					nCurrentPortA	= ZRTPSocketPool::GetRTPPort();
					nCurrentPortB	= (nCurrentPortA + 1);
					if(pUDPSocketA->Bind(nAddr,nCurrentPortA) && pUDPSocketB->Bind(nAddr,nCurrentPortB))
					{
						pPair	= NEW ZUDPSocketPair();
						if(pPair != NULL)
						{
							pPair->Set(pUDPSocketA,pUDPSocketB);
							break;
						}
					}
					pUDPSocketA->Unbind();
					pUDPSocketB->Unbind();
				}
			}
		}
		if(pPair == NULL)
		{
			SAFE_DELETE(pUDPSocketA);
			SAFE_DELETE(pUDPSocketB);
		}
	}

	return pPair;
}

ZUDPSocketPair* ZRTPSocketPool::SetupUDPSocketPairMulticast(DWORD nAddr,DWORD nPort)
{
	ZUDPSocketPair*	pPair			= NULL;
	BOOL			bPair			= FALSE;
	ZUDPSocket*		pUDPSocketA		= NULL;
	ZUDPSocket*		pUDPSocketB		= NULL;
	int				nCurrentPortA	= 0;
	int				nCurrentPortB	= 0;

	{
		ZOSMutexLocker	locker(&m_RTPSocketPoolMutex);
		pUDPSocketA		= NEW ZUDPSocket();
		pUDPSocketB		= NEW ZUDPSocket();

		if(pUDPSocketA != NULL && pUDPSocketB != NULL)
		{
			if(pUDPSocketA->Create() && pUDPSocketB->Create())
			{
				if(pUDPSocketA->JoinMulticast(nAddr) && pUDPSocketB->JoinMulticast(nAddr))
				{
					while(!bPair)
					{
						nCurrentPortA	= nPort;
						nCurrentPortB	= (nCurrentPortA + 1);
						if(pUDPSocketA->Bind(INADDR_ANY,nCurrentPortA) && pUDPSocketB->Bind(INADDR_ANY,nCurrentPortB))
						{
							pPair	= NEW ZUDPSocketPair();
							if(pPair != NULL)
							{
								pPair->Set(pUDPSocketA,pUDPSocketB);
								break;
							}
						}
						pUDPSocketA->Unbind();
						pUDPSocketB->Unbind();
					}
				}
			}
		}
		if(pPair == NULL)
		{
			SAFE_DELETE(pUDPSocketA);
			SAFE_DELETE(pUDPSocketB);
		}
		return pPair;
	}
}

BOOL ZRTPSocketPool::ReleaseUDPSocketPair(ZUDPSocketPair* pPair)
{
	if(pPair != NULL)
	{
		ZOSMutexLocker	locker(&m_RTPSocketPoolMutex);
		if(m_pRTPSocketPool != NULL)
		{
			m_pRTPSocketPool->CloseUDPSocketPair(pPair);
		}else{
			ZUDPSocket*	pUDPSocket	= NULL;
			TMASSERT((pPair->m_nUsedCount == 0))
			pUDPSocket	= pPair->GetUDPSocketA();
			SAFE_DELETE(pUDPSocket);
			pUDPSocket	= pPair->GetUDPSocketB();
			SAFE_DELETE(pUDPSocket);
			SAFE_DELETE(pPair);
		}
		return TRUE;
	}
	return FALSE;
}
UINT ZRTPSocketPool::GetPairCount()
{
	return m_nPairs;
}
void ZRTPSocketPool::SetPairCount(UINT nPairCount)
{
	m_nPairs = nPairCount;
}
///////////////////////////////////////////////////////////////////////////////
void ZRTPSocketPool::Initialize()
{
	UINT				i	= 0;
	ZUDPSocketPair*		p	= NULL;

	if(m_pRTPSocketPool == NULL)
	{
		m_pRTPSocketPool	= NEW ZRTPSocketPool();
		m_pRTCPTask			= ZRTCPTask::GetRTCPTask();
		if(m_pRTPSocketPool != NULL)
		{
			for(i = 0; i < m_nPairs; i ++)
			{
				p	= m_pRTPSocketPool->CreateUDPSocketPair(0,0);
				if(m_pRTCPTask != NULL)
				{
					if(p != NULL)
					{
						m_pRTCPTask->AddSocket(p);
					}
				}
			}
		}
	}
}
void ZRTPSocketPool::Uninitialize()
{
	SAFE_DELETE(m_pRTPSocketPool);
}
///////////////////////////////////////////////////////////////////////////////
UINT ZRTPSocketPool::GetRTPPort()
{
	UINT	nRTPPort	= m_RTPPort;
	m_RTPPort		+= 2;
	if(m_RTPPort > 0xFFF0)
	{
		m_RTPPort	= DEFAULT_RTP_UDP_PORT_BEGINE;
	}
	return nRTPPort;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
