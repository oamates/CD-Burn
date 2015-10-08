#include "ZUDPSocket.h"
///////////////////////////////////////////////////////////////////////////////
#if	(defined(_WIN32_)||defined(_MINGW_))
#include	<mswsock.h>
#include	<ws2tcpip.h>
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
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZUDPSocket::ZUDPSocket(ZTask* pTask)
:ZSocket(pTask)
,m_bRemoteAddr(FALSE)
,m_bStreamSocket(FALSE)
{
	::memset(&m_RemoteAddr,0,sizeof(m_RemoteAddr));
}
ZUDPSocket::~ZUDPSocket()
{
	ZUDPSocket::Close();
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZUDPSocket::Create(INT nAddrFamily)
{
	return(ZSocket::Create(SOCK_DGRAM, nAddrFamily));
}
BOOL ZUDPSocket::Close()
{
	return(ZSocket::Close());
}
BOOL ZUDPSocket::Unbind()
{
	ZSocket::Close();
	return (ZSocket::Create(SOCK_DGRAM));
}
///////////////////////////////////////////////////////////////////////////////
int ZUDPSocket::SendTo(const void* sdata,const int ndata,UINT RemoteAddr,UINT RemotePort,int* nSend)
{
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		if(sdata != NULL && ndata > 0)
		{
			m_RemoteAddr.sin_family			= m_nAddrFamily;
			m_RemoteAddr.sin_port			= htons(RemotePort);
			m_RemoteAddr.sin_addr.s_addr	= htonl(RemoteAddr);
			m_bRemoteAddr					= TRUE;

			int	nError	= ::sendto(m_hSocket,(char*)sdata,ndata,0,(sockaddr*)&m_RemoteAddr,sizeof(m_RemoteAddr));
			if(nSend != NULL)
			{
				*nSend	= nError;
			}
			return nError;
		}
	}
	return 0;
}
int ZUDPSocket::RecvFrom(void* sdata,const int ndata,UINT* RemoteAddr,UINT* RemotePort,int* nRead)
{
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		if(sdata != NULL && ndata > 0)
		{
#if	defined(_WIN32_)||defined(_MINGW_)
			int			nLen	= sizeof(m_RemoteAddr);
#else
			socklen_t	nLen	= sizeof(m_RemoteAddr);
#endif	//(_WIN32_)||defined(_MINGW_)

			int	nError	= ::recvfrom(m_hSocket,(char*)sdata,ndata,0,(sockaddr*)&m_RemoteAddr,&nLen);
			if(RemoteAddr != NULL)
			{
				*RemoteAddr	= ntohl(m_RemoteAddr.sin_addr.s_addr);
			}
			if(RemotePort != NULL)
			{
				*RemotePort	= ntohs(m_RemoteAddr.sin_port);
			}
			if(nRead != NULL)
			{
				*nRead	= nError;
			}
			return nError;
		}
	}
	else
	{
		MESSAGE_OUT(("[ZUDPSocket::RecvFrom] m_hSocket = %d\r\n", m_hSocket));
	}
	return 0;
}
int ZUDPSocket::SendTo(const void* sdata,const int ndata,sockaddr_in* RemoteAddr,int* nSend)
{
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		if(sdata != NULL && ndata > 0)
		{
			m_RemoteAddr	= *RemoteAddr;
			m_bRemoteAddr					= TRUE;

			int	nError	= ::sendto(m_hSocket,(char*)sdata,ndata,0,(sockaddr*)&m_RemoteAddr,sizeof(m_RemoteAddr));
			if(nSend != NULL)
			{
				*nSend	= nError;
			}
			return nError;
		}
	}
	return 0;
}
int ZUDPSocket::RecvFrom(void* sdata,const int ndata,sockaddr_in* RemoteAddr,int* nRead)
{
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		if(sdata != NULL && ndata > 0)
		{
#if	defined(_WIN32_)||defined(_MINGW_)
			int			nLen	= sizeof(m_RemoteAddr);
#else
			socklen_t	nLen	= sizeof(m_RemoteAddr);
#endif	//(_WIN32_)||defined(_MINGW_)

			int	nError	= ::recvfrom(m_hSocket,(char*)sdata,ndata,0,(sockaddr*)&m_RemoteAddr,&nLen);
			if(RemoteAddr != NULL)
			{
				*RemoteAddr	= m_RemoteAddr;
			}
			if(nRead != NULL)
			{
				*nRead	= nError;
			}
			return nError;
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZUDPSocket::GetRemoteAddr(UINT* RemoteAddr,UINT* RemotePort)
{
	if(m_bRemoteAddr)
	{
		if(RemoteAddr != NULL)
		{
			*RemoteAddr	= ntohl(m_RemoteAddr.sin_addr.s_addr);
		}
		if(RemotePort != NULL)
		{
			*RemotePort	= ntohs(m_RemoteAddr.sin_port);
		}
	}
	return m_bRemoteAddr;
}
BOOL ZUDPSocket::SetRemoteAddr(UINT RemoteAddr,UINT RemotePort)
{
	m_RemoteAddr.sin_family			= m_nAddrFamily;
	m_RemoteAddr.sin_port			= htons(RemotePort);
	m_RemoteAddr.sin_addr.s_addr	= htonl(RemoteAddr);
	m_bRemoteAddr					= TRUE;
	return m_bRemoteAddr;
}
///////////////////////////////////////////////////////////////////////////////
int ZUDPSocket::StreamCreate()
{
	m_bStreamSocket	= TRUE;
	return 0;
}
int ZUDPSocket::StreamClose()
{
	m_bStreamSocket	= FALSE;
	return 0;
}
int	ZUDPSocket::StreamRead(char* sdata,int ndata)
{
	return ZUDPSocket::RecvFrom(sdata,ndata,&m_RemoteAddr);
}
int	ZUDPSocket::StreamWrite(const char* sdata,int ndata)
{
	return ZUDPSocket::SendTo(sdata,ndata,&m_RemoteAddr);
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZUDPSocket::StreamReadable()
{
	return ZUDPSocket::IsReadable(DEFAULT_NET_MIN_UTIMEOUT);
}
BOOL ZUDPSocket::StreamWriteable()
{
	return ZUDPSocket::IsWriteable(DEFAULT_NET_MIN_UTIMEOUT);
}
BOOL ZUDPSocket::StreamError()
{
	return ZUDPSocket::IsError(DEFAULT_NET_MIN_UTIMEOUT*9999);
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZUDPSocket::JoinMulticast(UINT RemoteAddr)
{
	int				nError	= 0;
	struct ip_mreq	MulticastAddr;

	TMASSERT((m_hSocket != INVALID_FILE_HANDLE));
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		MulticastAddr.imr_multiaddr.s_addr	= htonl(RemoteAddr);
		MulticastAddr.imr_interface.s_addr	= m_LocalAddr.sin_addr.s_addr;

		nError	= ::setsockopt(m_hSocket,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&MulticastAddr,sizeof(MulticastAddr));
		TMASSERT((nError == 0));
		if(nError != 0)
		{
#ifdef	_WIN32_
			LOG_ERROR(("ErrorCode:%s [ZUDPSocket::JoinMulticast] ERROR(Error Code = %d\n)!\r\n", GetErrorCodeString(20054), ::GetLastError()));
#endif	//_WIN32_
		}
		return (nError == 0);
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZUDPSocket::JoinMulticast] ERROR(m_hSocket INVALID_FILE_HANDLE)!\r\n", GetErrorCodeString(20055)));
	}
	return FALSE;
}
BOOL ZUDPSocket::LeaveMulticast(UINT RemoteAddr)
{
	int				nError	= 0;
	struct ip_mreq	MulticastAddr;

	TMASSERT((m_hSocket != INVALID_FILE_HANDLE));
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		MulticastAddr.imr_multiaddr.s_addr	= htonl(RemoteAddr);
		MulticastAddr.imr_interface.s_addr	= m_LocalAddr.sin_addr.s_addr;

		nError	= ::setsockopt(m_hSocket,IPPROTO_IP,IP_DROP_MEMBERSHIP,(char*)&MulticastAddr,sizeof(MulticastAddr));
		TMASSERT((nError == 0));
		if(nError != 0)
		{
#ifdef	_WIN32_
			LOG_ERROR(("ErrorCode:%s [ZUDPSocket::LeaveMulticast] ERROR(Error Code = %d\n)!\r\n", GetErrorCodeString(20056), ::GetLastError()));
#endif	//_WIN32_
		}
		return (nError == 0);
	}else{
		LOG_ERROR(("ErrorCode:%s [ZUDPSocket::LeaveMulticast] ERROR(m_hSocket INVALID_FILE_HANDLE)!\r\n", GetErrorCodeString(20057)));
	}
	return FALSE;
}
BOOL ZUDPSocket::SetMulticastInterface(UINT LocalAddr)
{
	int				nError	= 0;
	in_addr			InterfaceAddr;

	TMASSERT((m_hSocket != INVALID_FILE_HANDLE));
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		InterfaceAddr.s_addr	= LocalAddr;

		nError	= ::setsockopt(m_hSocket,IPPROTO_IP,IP_MULTICAST_IF,(char*)&InterfaceAddr,sizeof(InterfaceAddr));
		TMASSERT((nError == 0));
		if(nError != 0)
		{
#ifdef	_WIN32_
			LOG_ERROR(("ErrorCode:%s [ZUDPSocket::SetMulticastInterface] ERROR(Error Code = %d\n)!\r\n", GetErrorCodeString(20058), ::GetLastError()));
#endif	//_WIN32_
		}
		return (nError == 0);
	}else{
		LOG_ERROR(("ErrorCode:%s [ZUDPSocket::SetMulticastInterface] ERROR(m_hSocket INVALID_FILE_HANDLE)!\r\n", GetErrorCodeString(20059)));
	}
	return FALSE;
}
BOOL ZUDPSocket::SetMulticastTTL(UINT nTTL)
{
	int				nError			= 0;
	BYTE			nMulticastTTL	= 0;

	TMASSERT((m_hSocket != INVALID_FILE_HANDLE));
	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		nMulticastTTL	= nTTL;

		nError	= ::setsockopt(m_hSocket,IPPROTO_IP,IP_MULTICAST_TTL,(char*)&nMulticastTTL,sizeof(nMulticastTTL));
		TMASSERT((nError == 0));
		if(nError != 0)
		{
#ifdef	_WIN32_
			LOG_ERROR(("ErrorCode:%s [ZUDPSocket::SetMulticastTTL] ERROR(Error Code = %d\n)!\r\n", GetErrorCodeString(20060), ::GetLastError()));
#endif	//_WIN32_
		}
		return (nError == 0);
	}else{
		LOG_ERROR(("ErrorCode:%s [ZUDPSocket::SetMulticastTTL] ERROR(m_hSocket INVALID_FILE_HANDLE)!\r\n", GetErrorCodeString(20061)));
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
