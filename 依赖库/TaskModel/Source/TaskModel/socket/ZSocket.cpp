#include	"ZSocket.h"
///////////////////////////////////////////////////////////////////////////////
#if	(defined(_WIN32_)||defined(_MINGW_))
#include	<mswsock.h>
#include	<ws2tcpip.h>
#if	(defined(_WIN32_))
#pragma comment(lib,"wsock32.lib")
#pragma comment(lib,"ws2_32.lib")
#endif	//(defined(_WIN32_)
#else
#include	<sys/socket.h>
#include	<sys/select.h>
#include	<sys/time.h>
#include	<sys/uio.h>
#include	<sys/ioctl.h>
#include	<netinet/tcp.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>
#endif	//!(defined(_WIN32_)||defined(_MINGW_))
///////////////////////////////////////////////////////////////////////////////
//#define	ZSOCKETEVENTTHREAD_DEBUG	1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZEventThread* ZSocket::m_pEventThread	= NULL;
///////////////////////////////////////////////////////////////////////////////
ZSocket::ZSocket(ZTask* pTask)
:ZEvent(pTask,m_pEventThread)
,m_hSocket(INVALID_FILE_HANDLE)
,m_nAddrFamily(AF_INET)
,m_bNonBlocking(FALSE)
{
	memset(&m_LocalAddr,0,sizeof(sockaddr_in));
	memset(&m_DestAddr,0,sizeof(sockaddr_in));
}
ZSocket::~ZSocket()
{
	ZSocket::Close();
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZSocket::StreamSeek(UINT64 nPos)
{
	return(FALSE);
}
///////////////////////////////////////////////////////////////////////////////
void ZSocket::Initialize()
{
#ifdef	_WIN32_
	WORD	wsVersion	= MAKEWORD(1,1);
	WSADATA	wsData;
	(void)::WSAStartup(wsVersion,&wsData);
#endif	//_WIN32_
	if(m_pEventThread == NULL)
	{
		m_pEventThread	= NEW ZSocketEventThread();
	}
}
void ZSocket::Uninitialize()
{
	SAFE_DELETE(m_pEventThread);
}
void ZSocket::StartEventThread()
{
	if(m_pEventThread != NULL)
	{
		m_pEventThread->Start();
	}
}
void ZSocket::StopEventThread()
{
	if(m_pEventThread != NULL)
	{
		m_pEventThread->Stop();
	}
}
ZEventThread* ZSocket::GetEventThread()
{
	return(m_pEventThread);
};
///////////////////////////////////////////////////////////////////////////////
BOOL ZSocket::Create(UINT nType, INT nAddrFamily)
{
	ZSocket::Close();

    m_nAddrFamily   = nAddrFamily;
	m_hSocket	= ::socket(nAddrFamily, nType,0);
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		//LOG_DEBUG(("[ZSocket::Create] m_hSocket %d\r\n", m_hSocket));
		ZEvent::Create(m_hSocket);
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZSocket::Create] ERROR(%d)!\r\n", GetErrorCodeString(20032), ZOSThread::GetError()));
	}
	return (m_hSocket!=INVALID_FILE_HANDLE);
}
BOOL ZSocket::Close()
{
	int	nError	= 0;

	ZEvent::Close();
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
#if	defined(_WIN32_)||defined(_MINGW_)
		shutdown(m_hSocket,SD_BOTH);
		nError	= ::closesocket(m_hSocket);
#else
		nError	= ::close(m_hSocket);
#endif	//defined(_WIN32_)||defined(_MINGW_)
	}
	TMASSERT((nError==0));
	m_hSocket	= INVALID_FILE_HANDLE;
	return (nError==0);
}
BOOL ZSocket::Bind(UINT nAddr,UINT nPort)
{
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
#if	defined(_WIN32_)||defined(_MINGW_)
		int			nLocalLen	= sizeof(m_LocalAddr);
#else
		socklen_t	nLocalLen	= sizeof(m_LocalAddr);
#endif	//(_WIN32_)||defined(_MINGW_)
		::memset(&m_LocalAddr,0,sizeof(m_LocalAddr));
		m_LocalAddr.sin_family		= m_nAddrFamily;
		m_LocalAddr.sin_port		= htons(nPort);
		m_LocalAddr.sin_addr.s_addr	= htonl(nAddr);

        int	nError	= ::bind(m_hSocket,(sockaddr *)&m_LocalAddr,sizeof(m_LocalAddr));

		if(nError == 0)
		{
			::getsockname(m_hSocket,(sockaddr *)&m_LocalAddr,&nLocalLen);
			return TRUE;
		}
		else
		{
			LOG_ERROR(("ErrorCode:%s [ZSocket::Bind] ERROR (Port = %d Code = %d)!\r\n", GetErrorCodeString(20033),nPort,ZOSThread::GetError()));
		}
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZSocket::Bind] ERROR (m_hSocket INVALID_FILE_HANDLE)!\r\n", GetErrorCodeString(20033)));
	}
	return FALSE;
}
BOOL ZSocket::Unbind()
{
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZSocket::SetNonBlocking(BOOL bNonBlocking)
{
	int	nError	= 0;
	TMASSERT((m_hSocket != INVALID_FILE_HANDLE));
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
#if	defined(_WIN32_)||defined(_MINGW_)
		u_long	nFlag	= 0;
		if(bNonBlocking)
		{
			nFlag	= 1;
		}
		m_bNonBlocking	= bNonBlocking;
		nError	= ::ioctlsocket(m_hSocket,FIONBIO,&nFlag);
		if(nError == SOCKET_ERROR)
		{
			LOG_ERROR(("ErrorCode:%s [ZSocket::SetNonBlocking] ERROR(%d)!\r\n", GetErrorCodeString(20034),WSAGetLastError()));
		}
#else
		int	nFlag	= 0;
		nFlag	= ::fcntl(m_hSocket,F_GETFL,0);
		if(bNonBlocking)
		{
			nFlag	|= O_NONBLOCK;
		}else{
			nFlag	&= ~O_NONBLOCK;
		}
		nError	= ::fcntl(m_hSocket,F_SETFL,nFlag);
		if(nError == -1)
		{
			DEBUG_OUT(("ZSocket::SetNonBlocking ERROR(%d)\r\n",ZOSThread::GetError()));
		}
#endif	//defined(_WIN32_)||defined(_MINGW_)
		TMASSERT((nError==0));
		return (nError==0);
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZSocket::SetNonBlocking] ERROR(m_hSocket INVALID_FILE_HANDLE)!\r\n", GetErrorCodeString(20035)));
	}

	return FALSE;
}
BOOL ZSocket::SetReuseAddr(BOOL bReuseAddr)
{
	int	nError	= 0;
	int	nFlag	= 0;
	TMASSERT((m_hSocket != INVALID_FILE_HANDLE));
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		nFlag	= (bReuseAddr ? 1 : 0);
		nError= ::setsockopt(m_hSocket,SOL_SOCKET,SO_REUSEADDR,(char*)&nFlag,sizeof(int));
		TMASSERT((nError==0));
		return (nError==0);
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZSocket::SetReuseAddr] ERROR(m_hSocket INVALID_FILE_HANDLE)!\r\n", GetErrorCodeString(20036)));
	}
	return FALSE;
}
BOOL ZSocket::SetNoDelay(BOOL bNoDelay)
{
	int	nError	= 0;
	int	nFlag	= 0;
	TMASSERT((m_hSocket != INVALID_FILE_HANDLE));
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		nFlag	= (bNoDelay ? 1 : 0);
		nError	= ::setsockopt(m_hSocket,IPPROTO_TCP,TCP_NODELAY,(char*)&nFlag,sizeof(int));
		if (nError != 0)
		{
			LOG_ERROR(("ErrorCode:%s [ZSocket::SetNoDelay] system setsockopt fail!\r\n", GetErrorCodeString(20037)));
		}
		TMASSERT((nError==0));
		return (nError==0);
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZSocket::SetNoDelay] ERROR(m_hSocket INVALID_FILE_HANDLE)!\r\n", GetErrorCodeString(20037)));
	}
	return FALSE;
}
BOOL ZSocket::SetKeepAlive(BOOL bKeepAlive)
{
	int	nError	= 0;
	int	nFlag	= 0;
	TMASSERT((m_hSocket != INVALID_FILE_HANDLE));
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		nFlag	= (bKeepAlive ? 1 : 0);
		nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_KEEPALIVE,(char*)&nFlag,sizeof(int));
		if (nError != 0)
		{
			LOG_ERROR(("ErrorCode:%s [ZSocket::SetKeepAlive] system setsockopt fail!\r\n", GetErrorCodeString(20038)));
		}
		TMASSERT((nError==0));
		return (nError==0);
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZSocket::SetKeepAlive] ERROR(m_hSocket INVALID_FILE_HANDLE)!\r\n", GetErrorCodeString(20038)));
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZSocket::SetSendBufferSize(int nValue)
{
	int	nError	= 0;
	TMASSERT((m_hSocket != INVALID_FILE_HANDLE));
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		if(nValue > 0)
		{
			nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_SNDBUF,(char*)&nValue,sizeof(int));
		}
		if (nError != 0)
		{
			LOG_ERROR(("ErrorCode:%s [ZSocket::SetSendBufferSize] system setsockopt fail!\r\n", GetErrorCodeString(20039)));
		}
		TMASSERT((nError == 0));
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZSocket::SetSendBufferSize] ERROR(m_hSocket INVALID_FILE_HANDLE)!\r\n", GetErrorCodeString(20039)));
	}
	return TRUE;
}
BOOL ZSocket::SetRecvBufferSize(int nValue)
{
	int	nError	= 0;
	TMASSERT((m_hSocket != INVALID_FILE_HANDLE));
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		if(nValue > 0)
		{
			nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_RCVBUF,(char*)&nValue,sizeof(int));
		}
		if (nError != 0)
		{
			LOG_ERROR(("ErrorCode:%s [ZSocket::SetRecvBufferSize] system setsockopt fail!\r\n", GetErrorCodeString(20040)));
		}
		TMASSERT((nError == 0));
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZSocket::SetRecvBufferSize] ERROR(m_hSocket INVALID_FILE_HANDLE)!\r\n", GetErrorCodeString(20040)));
	}
	return TRUE;
}
BOOL ZSocket::SetTimeOut(int sndTime,int rcvTime)
{
	int	nError	= 0;
	TMASSERT((m_hSocket != INVALID_FILE_HANDLE));
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		if(rcvTime > 0)
		{
#if	defined(_WIN32_)||defined(_MINGW_)
			int	time		= (rcvTime);
#else
			struct	timeval	time;
			time.tv_sec		= (rcvTime/1000);
			time.tv_usec	= ((rcvTime%1000)*1000);
#endif	//
			nError		= ::setsockopt(m_hSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&time,sizeof(time));
		}
		if (nError != 0)
		{
			LOG_ERROR(("ErrorCode:%s [ZSocket::SetTimeOut] system setsockopt SO_RCVTIMEO fail!\r\n", GetErrorCodeString(20041)));
		}
		TMASSERT((nError == 0));
		if(sndTime > 0)
		{
#if	defined(_WIN32_)||defined(_MINGW_)
			int	time		= (sndTime);
#else
			struct	timeval	time;
			time.tv_sec		= (sndTime/1000);
			time.tv_usec	= ((sndTime%1000)*1000);
#endif	//
			nError		= ::setsockopt(m_hSocket,SOL_SOCKET,SO_SNDTIMEO,(char*)&time,sizeof(time));
		}
		if (nError != 0)
		{
			LOG_ERROR(("ErrorCode:%s [ZSocket::SetTimeOut] system setsockopt SO_SNDTIMEO fail!\r\n", GetErrorCodeString(20042)));
		}
		TMASSERT((nError == 0));
	}
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZSocket::IsBound()
{
	return(m_nState&SOCKET_BOUND);
}
BOOL ZSocket::IsConnected()
{
	return(m_nState&SOCKET_CONNECTED);
}
BOOL ZSocket::IsReadable(int nUsecond)
{
	fd_set	fd;
	timeval	tv;
	int		nSelectCode	= 0;

	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		FD_ZERO(&fd);
		FD_SET((unsigned int)m_hSocket,&fd);
		tv.tv_sec	= (nUsecond / 1000000);//0;
		tv.tv_usec	= (nUsecond > 0 ? (nUsecond % 1000000) : DEFAULT_NET_MIN_UTIMEOUT);
		if((nSelectCode = select(m_hSocket+1,&fd,NULL,NULL,&tv)) > 0)//timeout
		{
			if(FD_ISSET(m_hSocket,&fd))
			{
				return TRUE;
			}
		}else{
		}
	}
	return FALSE;
}
BOOL ZSocket::IsWriteable(int nUsecond)
{
	fd_set	fd;
	timeval	tv;
	int		nSelectCode	= 0;

	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		FD_ZERO(&fd);
		FD_SET((unsigned int)m_hSocket,&fd);
		tv.tv_sec	= (nUsecond / 1000000);//0;
		tv.tv_usec	= (nUsecond > 0 ? (nUsecond % 1000000) : DEFAULT_NET_MIN_UTIMEOUT);
		if((nSelectCode = select(m_hSocket+1,NULL,&fd,NULL,&tv)) > 0)//timeout
		{
			if(FD_ISSET(m_hSocket,&fd))
			{
				return TRUE;
			}
		}else{
			if (nSelectCode == -1)
			{
				LOG_DEBUG(("[ZSocket::IsWriteable] m_hSocket=%d %d\r\n",m_hSocket, ZOSThread::GetError()));
			}
			else
			{
				//LOG_DEBUG(("[ZSocket::IsWriteable] nSelectCode = 0 m_hSocket=%d\r\n", m_hSocket));
			}
		}
	}
	return FALSE;
}
BOOL ZSocket::IsError(int nUsecond)
{
	fd_set	fd;
	timeval	tv;
	int		nSelectCode	= 0;

	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		FD_ZERO(&fd);
		FD_SET((unsigned int)m_hSocket,&fd);
		tv.tv_sec	= (nUsecond / 1000000);//0;
		tv.tv_usec	= (nUsecond > 0 ? (nUsecond % 1000000) : DEFAULT_NET_MIN_UTIMEOUT);
		if((nSelectCode = select(m_hSocket+1,NULL,NULL,&fd,&tv)) > 0)//timeout
		{
			if(FD_ISSET(m_hSocket,&fd))
			{
				return TRUE;
			}
		}else{
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
UINT ZSocket::GetSocket()
{
	return(m_hSocket);
};
UINT ZSocket::GetLocalAddr()
{
	return(ntohl(m_LocalAddr.sin_addr.s_addr));
};
UINT ZSocket::GetLocalPort()
{
	return(ntohs(m_LocalAddr.sin_port));
};
///////////////////////////////////////////////////////////////////////////////
UINT ZSocket::ConvertAddr(const char* sAddr)
{
	if(sAddr != NULL)
	{
		return ntohl((::inet_addr(sAddr)));
	}
	return 0;
}
char* ZSocket::ConvertAddr(UINT nAddr)
{
	UINT	nTempAddr	= ntohl(nAddr);
	return ::inet_ntoa((const struct in_addr&)nTempAddr);
}
BOOL ZSocket::IsMulticastAddr(UINT nAddr)
{
	return (((nAddr>>8) & 0x00f00000) == 0x00e00000); 
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZSocketEventThread::ZSocketEventThread()
:ZEventThread()
,m_nMonitorUSeconds((9000))
,m_nMonitorSetFromSelect(0)
,m_nMonitorSetProcessed(0)
,m_nMonitorSetRead(0)
,m_nMonitorSetHandle(0)
,m_SocketEventMutex("SocketEventMutex")
,m_MonitorMaxSet(0)
{
	FD_ZERO(&m_ReadSet);
	FD_ZERO(&m_WriteSet);
	FD_ZERO(&m_MonitorReadSet);
	FD_ZERO(&m_MonitorWriteSet);

}
ZSocketEventThread::~ZSocketEventThread()
{
	ZOSThread::Sleep(m_nMonitorUSeconds/1000);
#if	defined(_WIN32_)||defined(_MINGW_)
	TMASSERT((m_ReadSet.fd_count==0));
	TMASSERT((m_WriteSet.fd_count==0));
#endif	//defined(_WIN32_)||defined(_MINGW_)
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZSocketEventThread::AddEvent(ZEvent* pEvent,UINT nEvent)
{
	BOOL			bReturn	= FALSE;

#ifdef	ZSOCKETEVENTTHREAD_DEBUG
	MESSAGE_OUT(("ZSocketEventThread::AddEvent(Owner 0x%08X pEvent 0x%08X Event %d)\r\n",this,pEvent,nEvent));
#endif	//ZSOCKETEVENTTHREAD_DEBUG
	TMASSERT((pEvent != NULL));
	if(pEvent != NULL)
	{
		bReturn		= ZEventThread::AddEvent(pEvent,nEvent);
		if(pEvent->GetHandle() != INVALID_FILE_HANDLE)
		{
			ZOSMutexLocker	locker(&m_SocketEventMutex);
			if(nEvent&ZEvent::EVENT_READ)
			{
				if(!FD_ISSET((unsigned int)pEvent->GetHandle(),&m_ReadSet))
				{
					FD_SET((unsigned int)pEvent->GetHandle(),&m_ReadSet);
				}
			}else{
				FD_CLR((unsigned int)pEvent->GetHandle(),&m_ReadSet);
			}
			if(nEvent&ZEvent::EVENT_WRITE)
			{
				if(!FD_ISSET((unsigned int)pEvent->GetHandle(),&m_WriteSet))
				{
					FD_SET((unsigned int)pEvent->GetHandle(),&m_WriteSet);
				}
			}else{
				FD_CLR((unsigned int)pEvent->GetHandle(),&m_WriteSet);
			}
			if(pEvent->GetHandle() > m_MonitorMaxSet)
			{
				m_MonitorMaxSet	= pEvent->GetHandle();
			}
		}
	}
	return bReturn;
}
BOOL ZSocketEventThread::RemoveEvent(ZEvent* pEvent,UINT nEvent)
{
	BOOL			bReturn	= FALSE;

	TMASSERT((pEvent != NULL));
	if(pEvent != NULL)
	{
		bReturn		= ZEventThread::RemoveEvent(pEvent,nEvent);

		if(pEvent->GetHandle() != INVALID_FILE_HANDLE)
		{
			ZOSMutexLocker	locker(&m_SocketEventMutex);
			FD_CLR((unsigned int)pEvent->GetHandle(),&m_ReadSet);
			FD_CLR((unsigned int)pEvent->GetHandle(),&m_WriteSet);
		}
	}
#ifdef	ZSOCKETEVENTTHREAD_DEBUG
	MESSAGE_OUT(("ZSocketEventThread::RemoveEvent(Owner 0x%08X pEvent 0x%08X Event %d)\r\n",this,pEvent,nEvent));
#endif	//ZSOCKETEVENTTHREAD_DEBUG
	return bReturn;
}
BOOL ZSocketEventThread::WaitForEvent()
{
	BOOL	bIsSet	= FALSE;

	if(!m_bStop)
	{
		//
		if(m_nMonitorSetProcessed < m_nMonitorSetFromSelect)
		{
			if(m_nMonitorSetRead)
			{
				{
					ZOSMutexLocker	locker(&m_SocketEventMutex);
					while((!(bIsSet = FD_ISSET(m_nMonitorSetHandle,&m_MonitorReadSet))))
					{
						m_nMonitorSetHandle	++;
						if(m_nMonitorSetHandle > m_MonitorMaxSet)
						{
							break;
						}
						//WIN32 Sleep???
                        #if _WIN32_
						if(m_MonitorMaxSet > 0xFFFFFFF0)
						{
							break;
						}
                        #endif //_WIN32_
					}
					if(bIsSet)
					{
						FD_CLR((unsigned int)m_nMonitorSetHandle,&m_MonitorReadSet);
						FD_CLR((unsigned int)m_nMonitorSetHandle,&m_ReadSet);
					}else{
						m_nMonitorSetRead	= 0;
						m_nMonitorSetHandle	= 0;
					}
				}
				if(bIsSet)
				{
					ZEventThread::OnEvent(m_nMonitorSetHandle,ZEvent::EVENT_READ);
#ifdef	ZSOCKETEVENTTHREAD_DEBUG
						MESSAGE_OUT(("ZSocketEventThread::WaitForEvent(Read = 0x%08X)\r\n",m_nMonitorSetHandle));
#endif	//ZSOCKETEVENTTHREAD_DEBUG
					return TRUE;
				}
			}
			if(!m_nMonitorSetRead)
			{
				{
					ZOSMutexLocker	locker(&m_SocketEventMutex);
					while((!(bIsSet = FD_ISSET((unsigned int)m_nMonitorSetHandle,&m_MonitorWriteSet))))
					{
						m_nMonitorSetHandle	++;
						if(m_nMonitorSetHandle > m_MonitorMaxSet)
						{
							break;
						}
						//WIN32 Sleep???
                        #if _WIN32_
						if(m_MonitorMaxSet > 0xFFFFFFF0)
						{
							break;
						}
                        #endif//_WIN32_
					}
					if(bIsSet)
					{
						FD_CLR((unsigned int)m_nMonitorSetHandle,&m_MonitorWriteSet);
						FD_CLR((unsigned int)m_nMonitorSetHandle,&m_WriteSet);
					}else{
						m_nMonitorSetProcessed	= m_nMonitorSetFromSelect;
					}
				}
				if(bIsSet)
				{
					ZEventThread::OnEvent(m_nMonitorSetHandle,ZEvent::EVENT_WRITE);
#ifdef	ZSOCKETEVENTTHREAD_DEBUG
					//MESSAGE_OUT(("ZSocketEventThread::WaitForEvent(Write = 0x%08X)\r\n",pEvent));
#endif	//ZSOCKETEVENTTHREAD_DEBUG
					return TRUE;
				}
			}
		}
		//
		if(m_nMonitorSetProcessed > 0)
		{
			m_nMonitorSetFromSelect	= 0;
			m_nMonitorSetProcessed	= 0;
			m_nMonitorSetRead		= 0;
			m_nMonitorSetHandle		= 0;
		}
		//
		{
			struct	timeval	tv;
			tv.tv_usec	= (m_nMonitorUSeconds);
			tv.tv_sec	= 0;
			{
				ZOSMutexLocker	locker(&m_SocketEventMutex);
				memcpy(&m_MonitorReadSet,&m_ReadSet,sizeof(m_ReadSet));
				memcpy(&m_MonitorWriteSet,&m_WriteSet,sizeof(m_WriteSet));
			}
#if	defined(_WIN32_)||defined(_MINGW_)
			if((m_ReadSet.fd_count==0)&&(m_WriteSet.fd_count==0))
			{
				ZOSThread::Sleep(m_nMonitorUSeconds/1000);
			}
#endif	//defined(_WIN32_)||defined(_MINGW_)

			m_nMonitorSetFromSelect = ::select((m_MonitorMaxSet+1),&m_MonitorReadSet,&m_MonitorWriteSet,NULL,&tv);
			m_nMonitorSetRead		= 1;
#ifdef	ZSOCKETEVENTTHREAD_DEBUG
			if(m_nMonitorSetFromSelect > 0)
			{
				MESSAGE_OUT(("ZSocketEventThread::WaitForEvent(select = %d)\r\n",m_nMonitorSetFromSelect));
			}
#endif	//ZSOCKETEVENTTHREAD_DEBUG
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
