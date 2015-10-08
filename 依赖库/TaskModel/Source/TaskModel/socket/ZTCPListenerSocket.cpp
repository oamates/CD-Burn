#include "ZTCPListenerSocket.h"
///////////////////////////////////////////////////////////////////////////////
#if	!(defined(_WIN32_)||defined(_MINGW_))
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
///////////////////////////////////////////////////////////////////////////////
//#define	ZTCPLISTENERSOCKET_DEBUG	1
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_LISTEN_COUNT				128
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZTCPListenerEvent::ZTCPListenerEvent()
{
}
ZTCPListenerEvent::~ZTCPListenerEvent()
{
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZTCPListenerSocket::ZTCPListenerSocket()
:ZTCPSocket()
,ZIdleTask("TCPListenerSocketIdleTask")
,m_bSlowAccept(FALSE)
,m_nSecondsBetweenAccepts(1000)
,m_nListenCount(DEFAULT_LISTEN_COUNT)
,m_hAcceptSocket(INVALID_FILE_HANDLE)
,m_ListenerMutex("ListenerMutex")
,m_pListenerEvent(NULL)
{
	memset(&m_AcceptAddr,0,sizeof(m_AcceptAddr));
	SetTask(this);
	SetTaskThreadBegin(0);
	SetTaskThreadBound(1);
}
ZTCPListenerSocket::~ZTCPListenerSocket()
{
	SetTask(NULL);
	ZTCPListenerSocket::Close();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZTCPListenerSocket::Create(UINT nAddr,UINT nPort, INT nAddrFamily)
{
	BOOL			bReturn		= FALSE;

	ZTCPListenerSocket::Close();

	ZIdleTask::Create();
	{
		ZOSMutexLocker	locker(&m_ListenerMutex);
		if(ZTCPSocket::Create(nAddrFamily))
		{
			do{
				//ZSocket::SetReuseAddr();
                ZSocket::SetNonBlocking();

				if(!ZTCPSocket::Bind(nAddr,nPort))
				{
					LOG_ERROR(("ErrorCode:%s [ZTCPListenerSocket::Create] system bind fail! nAddr=%d, nPort=%d\r\n", 
						GetErrorCodeString(20043), nAddr, nPort));
					break;
				}
				if(!ZTCPListenerSocket::Listen(m_nListenCount))
				{
					LOG_ERROR(("ErrorCode:%s [ZTCPListenerSocket::Create] system listen fail! listencount=%d\r\n", 
						GetErrorCodeString(20044), m_nListenCount));
					break;
				}
				ZEvent::RequestEvent(ZEvent::EVENT_READ);
				bReturn	= TRUE;
			}while(FALSE);
            AddEvent(ZTask::TASK_START_EVENT);
		}
	}
	return bReturn;
}
BOOL ZTCPListenerSocket::Close()
{
	BOOL			bReturn		= FALSE;

	ZIdleTask::RemoveIdleTask();

	ZEvent::RemoveEvent(ZEvent::EVENT_READ);

	ZIdleTask::Close();

	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		ZOSMutexLocker	locker(&m_ListenerMutex);

		ZTCPSocket::TCPClose(m_hSocket);
		m_hSocket			= INVALID_FILE_HANDLE;
		m_hAcceptSocket		= INVALID_FILE_HANDLE;
		m_pListenerEvent	= NULL;
		memset(&m_AcceptAddr,0,sizeof(m_AcceptAddr));
		ZTCPSocket::Close();
	}
	bReturn	= TRUE;
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ZTCPListenerSocket::Accept()
{
	int				nReturn	= INVALID_FILE_HANDLE;
	ZOSMutexLocker	locker(&m_ListenerMutex);
	{
		m_hAcceptSocket	= INVALID_FILE_HANDLE;
		memset(&m_AcceptAddr,0,sizeof(m_AcceptAddr));

		if(m_hSocket != INVALID_FILE_HANDLE)
		{
#if	defined(_WIN32_)||defined(_MINGW_)
			int			size	= sizeof(m_AcceptAddr);
#else
			socklen_t	size	= sizeof(m_AcceptAddr);
#endif	//(_WIN32_)||defined(_MINGW_)
			m_hAcceptSocket	= ::accept(m_hSocket,(struct sockaddr*)&m_AcceptAddr,&size);
#ifdef	ZTCPLISTENERSOCKET_DEBUG
			MESSAGE_OUT(("ZTCPListenerSocket::Accept:hAccept=%08x;ip=%s;port=%d\r\n",m_hAcceptSocket,ZSocket::ConvertAddr((UINT)(m_AcceptAddr.sin_addr.s_addr)),ntohs(m_AcceptAddr.sin_port)));
#endif	//ZTCPLISTENERSOCKET_DEBUG
			if(m_hAcceptSocket == INVALID_FILE_HANDLE)
			{
				int	nError	= ZOSThread::GetError();
				if(nError == EAGAIN)
				{
					LOG_ERROR(("ErrorCode:%s [ZTCPListenerSocket::Accept] system accept fail! ERROR EAGAIN\r\n", GetErrorCodeString(20045)));
				}
				else if(nError == EMFILE)
				{
					LOG_ERROR(("ErrorCode:%s [ZTCPListenerSocket::Accept] system accept fail! ERROR EMFILE\r\n", GetErrorCodeString(20046)));
				}
				else if(nError == ENFILE)
				{
					LOG_ERROR(("ErrorCode:%s [ZTCPListenerSocket::Accept] system accept fail! ERROR ENFILE\r\n", GetErrorCodeString(20046)));
				}
				else
				{
					LOG_ERROR(("ErrorCode:%s [ZTCPListenerSocket::Accept] system accept fail! ERROR %d\r\n", GetErrorCodeString(20047), nError));
				}
			}
			else
			{
				if(m_pListenerEvent != NULL)
				{
					if(m_pListenerEvent->OnListenerEvent(m_hAcceptSocket,&m_LocalAddr,&m_AcceptAddr))
					{

						//DEBUG_OUT(("ZTCPListenerSocket::Accept OK\n"));
					}else{
#if	defined(_WIN32_)||defined(_MINGW_)
						::closesocket(m_hAcceptSocket);
#else
						::close(m_hAcceptSocket);
#endif	//defined(_WIN32_)||defined(_MINGW_)
						m_hAcceptSocket	= INVALID_FILE_HANDLE;
						//DEBUG_OUT(("ZTCPListenerSocket::Accept ERROR(OnListenerEvent=FALSE)\n"));
					}
				}else
				{
#if	defined(_WIN32_)||defined(_MINGW_)
					::closesocket(m_hAcceptSocket);
#else
					::close(m_hAcceptSocket);
#endif	//defined(_WIN32_)||defined(_MINGW_)
					m_hAcceptSocket	= INVALID_FILE_HANDLE;
					LOG_ERROR(("ErrorCode:%s [ZTCPListenerSocket::Accept] ERROR(m_pListenerEvent=NULL)!\r\n", GetErrorCodeString(20048)));
				}
			}
			nReturn	= m_hAcceptSocket;
		}
	}
	return nReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ZTCPListenerSocket::SlowSpeed()
{
	m_bSlowAccept=TRUE;
}
void ZTCPListenerSocket::NormalSpeed()
{
	m_bSlowAccept=FALSE;
}
///////////////////////////////////////////////////////////////////////////////
int ZTCPListenerSocket::GetAcceptSocket()
{
	return(m_hAcceptSocket);
}
struct sockaddr_in* ZTCPListenerSocket::GetAcceptAddr()
{
	return(&m_AcceptAddr);
}
///////////////////////////////////////////////////////////////////////////////
ZTCPListenerEvent* ZTCPListenerSocket::GetListenerEvent()
{
	return(m_pListenerEvent);
}
ZTCPListenerEvent* ZTCPListenerSocket::SetListenerEvent(ZTCPListenerEvent* pListenerEvent)
{
	m_pListenerEvent=pListenerEvent;
	return(m_pListenerEvent);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ZTCPListenerSocket::Run(int nEvent)
{
	UINT	nLocalEvent	= 0;

	nLocalEvent	= GetEvent(nEvent);

	if((nLocalEvent&TASK_READ_EVENT))
	{
		this->ProcessEvent(EVENT_READ);
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZTCPListenerSocket::ProcessEvent(UINT nEvent)
{

	TMASSERT((nEvent==ZEvent::EVENT_READ));

	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		Accept();

		if(m_bSlowAccept)
		{
			AddIdleTask(m_nSecondsBetweenAccepts);
		}else{
			ZEvent::RequestEvent(ZEvent::EVENT_READ);
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZTCPListenerSocket::Listen(UINT nListenCount)
{
    return ZTCPSocket::Listen(nListenCount);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
