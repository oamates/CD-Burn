#include "ZRTCPTask.h"
#include "ZUDPSocket.h"
#include "ZRTPStream.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZRTCPTask*	ZRTCPTask::m_pRTCPTask	= NULL;
///////////////////////////////////////////////////////////////////////////////
ZRTCPTask::ZRTCPTask()
:ZTask("RTCPTask")
,m_StreamMutex("StreamMutex")
,m_StreamSocket(0)
,m_StreamTable(0)
{
}
ZRTCPTask::~ZRTCPTask()
{
	ZRTCPTask::Close();
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTCPTask::Create()
{
	ZTask::Create();
	ZTask::AddEvent(TASK_READ_EVENT);
	return TRUE;
}
BOOL ZRTCPTask::Close()
{
	ZTask::Close();
	m_StreamSocket.RemoveAll();
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTCPTask::AddSocket(ZUDPSocketPair* pSocketPair)
{
	ZOSMutexLocker	theLocket(&m_StreamMutex);
	m_StreamSocket.Add(pSocketPair);
	return TRUE;
}
BOOL ZRTCPTask::RegisterStream(ZRTPStream* pStream)
{
	ZOSMutexLocker	theLocket(&m_StreamMutex);
	if(pStream != NULL)
	{
		return m_StreamTable.Register(pStream->GetObject());
	}
	return FALSE;
}
BOOL ZRTCPTask::UnRegisterStream(ZRTPStream* pStream)
{
	ZOSMutexLocker	theLocket(&m_StreamMutex);
	if(pStream != NULL)
	{
		return m_StreamTable.Unregister(pStream->GetObject());
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
int ZRTCPTask::Run(int nEvent)
{
	int		nTaskTime	= 0;
	UINT	nLocalEvent	= 0;

	nLocalEvent	= GetEvent(nEvent);

	ZTask::Run(nLocalEvent);

	if(nLocalEvent&TASK_KILL_EVENT)
	{
		nTaskTime	= 0;
	}else
	if(nLocalEvent&TASK_TIMEOUT_EVENT)
	{
		nTaskTime	= 0;
	}else
	if(nLocalEvent&TASK_READ_EVENT)
	{
		OnReciveRTCPPacket();
		nTaskTime	= 90;
	}
	else if(nLocalEvent&TASK_UPDATE_EVENT)
	{
		OnReciveRTCPPacket();
		nTaskTime	= 90;
	}

	return nTaskTime;
 }
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTCPTask::OnReciveRTCPPacket()
{
	char		sData[DEFAULT_RTP_BUFFER_MAX];
	int			nData			= DEFAULT_RTP_BUFFER_MAX;
	UINT		nRemoteAddr		= 0;
	UINT		nRemotePort		= 0;
	UINT		nKey			= 0;
	ZOSObject*	pObject			= NULL;
	ZRTPStream*	pStream			= NULL;
	int			hSocket			= 0;
	fd_set		fd;
	timeval		tv;
	int			nSelectCode		= 0;

	{
		ZOSMutexLocker	theLocket(&m_StreamMutex);

		FD_ZERO(&fd);

		for(int i = 0; i < m_StreamSocket.Count(); i ++)
		{
			if(m_StreamSocket[i] != NULL && m_StreamSocket[i]->GetUDPSocketB()->GetHandle() != -1)
			{
				FD_SET((unsigned int)m_StreamSocket[i]->GetUDPSocketB()->GetHandle(),&fd);
				hSocket	= MAX(hSocket,m_StreamSocket[i]->GetUDPSocketB()->GetHandle());
			}
		}
		tv.tv_sec	= 0;
		tv.tv_usec	= DEFAULT_NET_MIN_UTIMEOUT;
		if((nSelectCode = select(hSocket+1,&fd,NULL,NULL,&tv)) > 0)//timeout
		{
			for(int i = 0; i < m_StreamSocket.Count(); i ++)
			{
				if(m_StreamSocket[i] != NULL && m_StreamSocket[i]->GetUDPSocketB()->GetHandle() != -1)
				{
					if(FD_ISSET(m_StreamSocket[i]->GetUDPSocketB()->GetHandle(),&fd))
					{
						do 
						{
							nData = DEFAULT_RTP_BUFFER_MAX;
							nData	= m_StreamSocket[i]->UDPSocketBRecvFrom((char*)sData,nData,&nRemoteAddr,&nRemotePort,NULL);
							if(nData > 0)
							{
								//LOG_DEBUG(("[ZRTCPTask::OnReciveRTCPPacket] %s %d %d\r\n", 
								//	ZSocket::ConvertAddr(nRemoteAddr), nRemotePort, nData));
								nKey	= (nRemoteAddr<<16|nRemotePort);
								ZOSObjectKey	Key(nKey);
								pObject	= m_StreamTable.GetObject(&Key);
								if(pObject != NULL)
								{
									pStream	= (ZRTPStream*)pObject->GetObject();
									if(pStream != NULL)
									{
										pStream->OnRecvRTCPData(0,sData,nData);
									}
								}
							}
							//MESSAGE_OUT(("[ZRTCPTask::OnReciveRTCPPacket] port %d nData %d\r\n", 
							//	m_StreamSocket[i]->GetUDPSocketB()->GetLocalPort(), nData));
						} while (nData>0);
					}
				}
			}
		}
	}
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
void ZRTCPTask::Initialize()
{
	if(m_pRTCPTask == NULL)
	{
		m_pRTCPTask	= NEW ZRTCPTask();
		if(m_pRTCPTask != NULL)
		{
			m_pRTCPTask->Create();
		}
	}
}
void ZRTCPTask::Uninitialize()
{
	if(m_pRTCPTask != NULL)
	{
		m_pRTCPTask->Close();
	}
	SAFE_DELETE(m_pRTCPTask);
}
///////////////////////////////////////////////////////////////////////////////
ZRTCPTask* ZRTCPTask::GetRTCPTask()
{
	return m_pRTCPTask;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
