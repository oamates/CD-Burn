#include "ZTCPSocket.h"
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
ZTCPSocket::ZTCPSocket(ZTask* pTask)
:ZSocket(pTask)
,m_bRemoteAddr(FALSE)
,m_bStreamSocket(FALSE)
{
	::memset(&m_RemoteAddr,0,sizeof(m_RemoteAddr));
}
ZTCPSocket::~ZTCPSocket()
{
	ZTCPSocket::Close();
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZTCPSocket::Create(INT nAddrFamily)
{
	ZTCPSocket::Close();
	return(ZSocket::Create(SOCK_STREAM, nAddrFamily));
}
BOOL ZTCPSocket::Close()
{
	m_nState= 0;
	return(ZSocket::Close());
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZTCPSocket::Connect(UINT nAddr,UINT nPort)
{
	BOOL	bReturn	= FALSE;

	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		if(!(m_nState&SOCKET_CONNECTED))
		{
			if(nAddr != 0 && nPort != 0)
			{
				::memset(&m_RemoteAddr,0,sizeof(m_RemoteAddr));
				m_RemoteAddr.sin_family			= m_nAddrFamily;
				m_RemoteAddr.sin_port			= htons(nPort);
				m_RemoteAddr.sin_addr.s_addr	= htonl(nAddr);
			}
			int	nError	= ::connect(m_hSocket,(sockaddr *)&m_RemoteAddr,sizeof(m_RemoteAddr));
			if(nError == 0)
			{
				m_nState		|= SOCKET_CONNECTED;

				m_bRemoteAddr	= TRUE;

				int	nValue		= 1;
				int	nBufSize	= DEFAULT_TCP_SEND_BUFFER_SIZE;
				nError	= ::setsockopt(m_hSocket,IPPROTO_TCP,TCP_NODELAY,(char*)&nValue,sizeof(int));
				TMASSERT((nError==0));
				nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_KEEPALIVE,(char*)&nValue,sizeof(int));
				TMASSERT((nError==0));
				nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_SNDBUF,(char*)&nBufSize,sizeof(int));
				TMASSERT((nError==0));

				bReturn	= TRUE;
			}else{
				nError	= ZOSThread::GetError();
				if(nError == EAGAIN)
				{
					if(ZTCPSocket::IsWriteable(9000))
					{
						m_nState		|= SOCKET_CONNECTED;

						m_bRemoteAddr	= TRUE;

						int	nValue		= 1;
						int	nBufSize	= DEFAULT_TCP_SEND_BUFFER_SIZE;
						nError	= ::setsockopt(m_hSocket,IPPROTO_TCP,TCP_NODELAY,(char*)&nValue,sizeof(int));
						TMASSERT((nError==0));
						nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_KEEPALIVE,(char*)&nValue,sizeof(int));
						TMASSERT((nError==0));
						nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_SNDBUF,(char*)&nBufSize,sizeof(int));
						TMASSERT((nError==0));

						bReturn	= TRUE;
					}
					else
					{
						m_bRemoteAddr	= FALSE;
						//m_RemoteAddr.sin_family			= 0;
						//m_RemoteAddr.sin_port			= 0;
						//m_RemoteAddr.sin_addr.s_addr	= 0;
						LOG_ERROR(("ErrorCode:%s [ZTCPSocket::Connect] EAGAIN Connect ERROR(%d)!\r\n", GetErrorCodeString(20049), nError));
					}
				}
				else if(nError == EINPROGRESS)
				{
						if(ZTCPSocket::IsWriteable(9000))
						{
							int	nValue		= 0;
							int	nSize		= sizeof(int);
							nError	= getsockopt(m_hSocket,SOL_SOCKET,SO_ERROR,(char*)&nValue,(socklen_t*)&nSize);
							if(nError == 0 && nValue == 0)
							{
								m_nState		|= SOCKET_CONNECTED;

								m_bRemoteAddr	= TRUE;

								int	nValue		= 1;
								int	nBufSize	= DEFAULT_TCP_SEND_BUFFER_SIZE;
								nError	= ::setsockopt(m_hSocket,IPPROTO_TCP,TCP_NODELAY,(char*)&nValue,sizeof(int));
								TMASSERT((nError==0));
								nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_KEEPALIVE,(char*)&nValue,sizeof(int));
								TMASSERT((nError==0));
								nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_SNDBUF,(char*)&nBufSize,sizeof(int));
								TMASSERT((nError==0));

								bReturn	= TRUE;
							}
							else
							{	
								m_bRemoteAddr	= FALSE;
								//m_RemoteAddr.sin_family			= 0;
								//m_RemoteAddr.sin_port			= 0;
								//m_RemoteAddr.sin_addr.s_addr	= 0;
								LOG_ERROR(("ErrorCode:%s [ZTCPSocket::Connect] EINPROGRESS Connect ERROR(%d)!\r\n", GetErrorCodeString(20050), nError));
							}
						}
						else
						{
							m_bRemoteAddr	= FALSE;
							//m_RemoteAddr.sin_family			= 0;
							//m_RemoteAddr.sin_port			= 0;
							//m_RemoteAddr.sin_addr.s_addr	= 0;
							LOG_ERROR(("ErrorCode:%s [ZTCPSocket::Connect] EINPROGRESS time eclipse Connect ERROR(%d)!\r\n", GetErrorCodeString(20050), nError));
						}
					}else{
					m_bRemoteAddr	= FALSE;
					//m_RemoteAddr.sin_family			= 0;
					//m_RemoteAddr.sin_port			= 0;
					//m_RemoteAddr.sin_addr.s_addr	= 0;
					LOG_ERROR(("ErrorCode:%s [ZTCPSocket::Connect] Connect ERROR(%d)!\r\n", GetErrorCodeString(20051), nError));
				}
			}
		}else{
			LOG_ERROR(("ErrorCode:%s [ZTCPSocket::Connect] ERROR(connected)!\r\n", GetErrorCodeString(20052)));
		}
	}else{
		DEBUG_OUT(("ZTCPSocket::Connect ERROR(m_hSocket NULL)\r\n"));
	}
	return bReturn;
}
BOOL ZTCPSocket::ConnectAble(UINT nUsecond)
{
	BOOL	bReturn	= FALSE;
	int		nError	= 0;

	if(ZTCPSocket::IsWriteable(nUsecond))
	{
		int	nValue		= 0;
		int	nSize		= sizeof(int);
        struct	sockaddr_in remoteAddr;
#if	defined(_WIN32_)||defined(_MINGW_)
        int			nLocalLen	= sizeof(remoteAddr);
#else
        socklen_t	nLocalLen	= sizeof(remoteAddr);
#endif	//(_WIN32_)||defined(_MINGW_)

        if (::getpeername(m_hSocket,(sockaddr *)&remoteAddr,&nLocalLen) == 0)
        {
            nError	= getsockopt(m_hSocket,SOL_SOCKET,SO_ERROR,(char*)&nValue,(socklen_t*)&nSize);
            if(nError == 0 && nValue == 0)
            {
                m_nState		|= SOCKET_CONNECTED;

                m_bRemoteAddr	= TRUE;

                int	nValue		= 1;
                int	nBufSize	= DEFAULT_TCP_SEND_BUFFER_SIZE;
                nError	= ::setsockopt(m_hSocket,IPPROTO_TCP,TCP_NODELAY,(char*)&nValue,sizeof(int));
                TMASSERT((nError==0));
                nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_KEEPALIVE,(char*)&nValue,sizeof(int));
                TMASSERT((nError==0));
                nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_SNDBUF,(char*)&nBufSize,sizeof(int));
                TMASSERT((nError==0));

                bReturn	= TRUE;
            }
            else
            {
                LOG_DEBUG(("[ZTCPSocket::ConnectAble] writeable but may be RST\r\n"));
            }
        }
        else
        {
            LOG_ERROR(("[ZTCPSocket::ConnectAble] getpeername false\r\n"));
        }
	}
    else
    {
        LOG_ERROR(("[ZTCPSocket::ConnectAble] IsWriteable false\r\n"));
    }

	return bReturn;
}
BOOL ZTCPSocket::Attach(const int hSocket,const struct sockaddr_in* pRemoteAddr)
{
	BOOL	bReturn	= FALSE;

	if(m_hSocket == INVALID_FILE_HANDLE)
	{
		if(hSocket != INVALID_FILE_HANDLE)
		{
#if	defined(_WIN32_)||defined(_MINGW_)
			int			nLocalLen	= sizeof(m_LocalAddr);
#else
			socklen_t	nLocalLen	= sizeof(m_LocalAddr);
#endif	//(_WIN32_)||defined(_MINGW_)

			m_hSocket		= hSocket;
			if(pRemoteAddr != NULL)
			{
				m_RemoteAddr	= *pRemoteAddr;
				m_bRemoteAddr	= TRUE;
			}
			m_nState		|= SOCKET_BOUND;
			m_nState		|= SOCKET_CONNECTED;
			::getsockname(m_hSocket,(sockaddr *)&m_LocalAddr,&nLocalLen);
			int	nValue		= 1;
			int	nBufSize	= DEFAULT_TCP_SEND_BUFFER_SIZE;
			int	nError		= 0;
			nError	= ::setsockopt(m_hSocket,IPPROTO_TCP,TCP_NODELAY,(char*)&nValue,sizeof(int));
			if(nError != 0)
			{
#ifdef	_WIN32_
				DEBUG_OUT(("ZTCPSocket::Attach ERROR(Error Code = %d\n)",::GetLastError()));
#endif	//_WIN32_
			}
			TMASSERT((nError==0));
			nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_KEEPALIVE,(char*)&nValue,sizeof(int));
			if(nError != 0)
			{
#ifdef	_WIN32_
				DEBUG_OUT(("ZTCPSocket::Attach ERROR(Error Code = %d\n)",::GetLastError()));
#endif	//_WIN32_
			}
			TMASSERT((nError==0));
			nError	= ::setsockopt(m_hSocket,SOL_SOCKET,SO_SNDBUF,(char*)&nBufSize,sizeof(int));
			if(nError != 0)
			{
#ifdef	_WIN32_
				DEBUG_OUT(("ZTCPSocket::Attach ERROR(Error Code = %d\n)",::GetLastError()));
#endif	//_WIN32_
			}
			TMASSERT((nError==0));

			ZEvent::Create(m_hSocket);

			bReturn	= TRUE;
		}else{
			DEBUG_OUT(("ZTCPSocket::Attach ERROR(hSocket NULL)\r\n"));
		}
	}else{
		DEBUG_OUT(("ZTCPSocket::Attach ERROR(m_hSocket NOT NULL)\r\n"));
	}
	return bReturn;
}
BOOL ZTCPSocket::Detach()
{
	m_hSocket		= INVALID_FILE_HANDLE;
	m_nState		= 0;
	::memset(&m_RemoteAddr,0,sizeof(m_RemoteAddr));
	::memset(&m_LocalAddr,0,sizeof(m_LocalAddr));
	m_bRemoteAddr	= FALSE;
	ZEvent::Close();

	return TRUE;
}
BOOL ZTCPSocket::Listen(UINT nListenCount)
{
    if(m_hSocket != INVALID_FILE_HANDLE)
    {
        int	nError	= ::listen(m_hSocket,nListenCount);

        if(nError == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}
int ZTCPSocket::Accept(UINT *pRemoteAddr, UINT *pRemotePort)
{
    int		hAcceptSocket;
    struct	sockaddr_in	acceptAddr;

    hAcceptSocket = INVALID_FILE_HANDLE;
    memset(&acceptAddr,0,sizeof(acceptAddr));
    
    if(m_hSocket != INVALID_FILE_HANDLE)
    {
#if	defined(_WIN32_)||defined(_MINGW_)
        int			size	= sizeof(acceptAddr);
#else
        socklen_t	size	= sizeof(acceptAddr);
#endif	//(_WIN32_)||defined(_MINGW_)
        hAcceptSocket = ::accept(m_hSocket,(struct sockaddr*)&acceptAddr,&size);
        if (hAcceptSocket != INVALID_FILE_HANDLE)
        {
            if (pRemoteAddr != NULL)
            {
                *pRemoteAddr = ntohl(acceptAddr.sin_addr.s_addr);
            }
            if (pRemotePort != NULL)
            {
                *pRemotePort = ntohs(acceptAddr.sin_port);
            }
            return hAcceptSocket;
        }
    }

    return INVALID_FILE_HANDLE;
}
///////////////////////////////////////////////////////////////////////////////
int ZTCPSocket::Send(const void* sdata,const int ndata,int* nSend)
{
	int	nReturn	= ZSOCKET_CLOSED;

	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		if(m_nState&SOCKET_CONNECTED)
		{
			if(sdata != NULL && ndata > 0)
			{
				int	nSendLen	= ::send(m_hSocket,(const char*)sdata,ndata,0);
				if(nSend != NULL)
				{
					*nSend	= nSendLen;
				}
				if(nSendLen < 0)
				{
					if(ZOSThread::GetError() != EAGAIN)
					{
						if(m_bRemoteAddr)
						{
							LOG_DEBUG((	"ZTCPSocket::Send ERROR(IP=%s;Port=%d), errorcode = %d\r\n",
										ZSocket::ConvertAddr(	(int)ntohl(m_RemoteAddr.sin_addr.s_addr)),
																(short)htons(m_RemoteAddr.sin_port),
																ZOSThread::GetError()
																));
						}else{
							LOG_DEBUG(("ZTCPSocket::Send ERROR\r\n"));
						}
						nReturn	= ZSOCKET_ERROR;
					}else{
						nReturn	= ZSOCKET_TIMEOUT;
					}
				}else if(nSendLen == 0)
				{
					if(m_bRemoteAddr)
					{
						LOG_DEBUG((	"ZTCPSocket::Send ERROR(IP=%s;Port=%d)\r\n",
									ZSocket::ConvertAddr(	(int)ntohl(m_RemoteAddr.sin_addr.s_addr)),
															(short)htons(m_RemoteAddr.sin_port)));
					}else{
						LOG_DEBUG(("ZTCPSocket::Send ERROR\r\n"));
					}
					nReturn	= ZSOCKET_CLOSED;
				}else{
					nReturn	= nSendLen;
				}
			}else{
				LOG_DEBUG(("[ZTCPSocket::Send] ERROR(Parameters)\r\n"));
			}
		}else{
			//LOG_DEBUG(("[ZTCPSocket::Send] ERROR(Disconnect)\r\n"));
		}
	}else{
		LOG_DEBUG(("[ZTCPSocket::Send] ERROR(m_hSocket NULL)\r\n"));
	}
	return nReturn;
}
int ZTCPSocket::Recv(void* sdata,const int ndata,int* nRead)
{
	int	nReturn	= ZSOCKET_CLOSED;
	int	nError	= 0;

	if(m_hSocket != INVALID_FILE_HANDLE)
	{
		if(m_nState&SOCKET_CONNECTED)
		{
			if(sdata != NULL && ndata > 0)
			{
				int	nReadLen	= ::recv(m_hSocket,(char*)sdata,ndata,0);
				if(nRead != NULL)
				{
					*nRead	= nReadLen;
				}
				if(nReadLen < 0)
				{
					nError	= ZOSThread::GetError();
					if(nError != EAGAIN)
					{
						if(m_bRemoteAddr)
						{
							LOG_DEBUG((	"ZTCPSocket::Recv ERROR(IP=%s;Port=%d) Error Code = %d\r\n",
										ZSocket::ConvertAddr((int)ntohl(m_RemoteAddr.sin_addr.s_addr)),
										htons(m_RemoteAddr.sin_port),
										nError));
						}else{
							LOG_DEBUG(("ZTCPSocket::Recv ERROR Error Code = %d\r\n",nError));
						}
						m_nState	&= ~SOCKET_CONNECTED;
						nReturn		= ZSOCKET_ERROR;
					}else{
						nReturn	= ZSOCKET_TIMEOUT;
					}
				}else if(nReadLen == 0)
				{
/*
					if(m_bRemoteAddr)
					{
						DEBUG_OUT((	"ZTCPSocket::Recv Close(IP=%s;Port=%d)\r\n",
									ZSocket::ConvertAddr((int)ntohl(m_RemoteAddr.sin_addr.s_addr)),
									(short)htons(m_RemoteAddr.sin_port)));
					}else{
						DEBUG_OUT(("ZTCPSocket::Recv Close\r\n"));
					}
*/
					m_nState	&= ~SOCKET_CONNECTED;
					nReturn		= ZSOCKET_CLOSED;
				}else{
					nReturn	= nReadLen;
				}
			}else{
				//DEBUG_OUT(("ZTCPSocket::Recv ERROR(Parameters)\r\n"));
			}
		}else{
			//DEBUG_OUT(("ZTCPSocket::Recv ERROR(Disconnect)\r\n"));
		}
	}else{
		//DEBUG_OUT(("ZTCPSocket::Recv ERROR(m_hSocket NULL)\r\n"));
	}

	return nReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZTCPSocket::GetRemoteAddr(UINT* RemoteAddr,UINT* RemotePort)
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
///////////////////////////////////////////////////////////////////////////////
int	ZTCPSocket::StreamCreate()
{
	m_bStreamSocket	= TRUE;
	return 0;
}
int	ZTCPSocket::StreamClose()
{
	m_bStreamSocket	= FALSE;
	return 0;
}
int	ZTCPSocket::StreamRead(char* sdata,int ndata)
{
	int	nread	= 0;
	int	nbyte	= 0;

	if(m_bStreamSocket)
	{
		if(m_bNonBlocking)
		{
			nread	= ZTCPSocket::Recv((sdata),ndata);
		}else{
			do{
				nbyte	= MIN((ndata-nread),DEFAULT_TCP_RECV_STREAM_SIZE);
				nbyte	= ZTCPSocket::Recv((sdata+nread),nbyte);
				if(nbyte > 0)
				{
					nread	+= nbyte;
				}else{
					if(nread == 0)
					{
						nread	= nbyte;
					}
					break;
				}
			}while(nread < ndata);
		}
	}else{
		nread	= ZTCPSocket::Recv((sdata),ndata);
	}
	return nread;//
}
int	ZTCPSocket::StreamWrite(const char* sdata,int ndata)
{
	return ZTCPSocket::Send(sdata,ndata);
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZTCPSocket::StreamReadable()
{
	return ZTCPSocket::IsReadable(DEFAULT_NET_MIN_UTIMEOUT);
}
BOOL ZTCPSocket::StreamWriteable()
{
	return ZTCPSocket::IsWriteable(DEFAULT_NET_MIN_UTIMEOUT);
}
BOOL ZTCPSocket::StreamError()
{
	return ZSocket::IsError(DEFAULT_NET_MIN_TIMEOUT*9999);
}
///////////////////////////////////////////////////////////////////////////////
int ZTCPSocket::TCPSend(int nSocket,const void* sdata,const int ndata)
{
	if(nSocket != INVALID_FILE_HANDLE)
	{
		int	nSendLen	= ::send(nSocket,(const char*)sdata,ndata,0);
		return nSendLen;
	}
	return 0;
}
int ZTCPSocket::TCPRecv(int nSocket,const void* sdata,const int ndata)
{
	if(nSocket != INVALID_FILE_HANDLE)
	{
		int	nReadLen	= ::recv(nSocket,(char*)sdata,ndata,0);
		return nReadLen;
	}

	return 0;
}
int ZTCPSocket::TCPClose(int nSocket)
{
	int	nError	= 0;
	if(nSocket != INVALID_FILE_HANDLE)
	{
#if	defined(_WIN32_)||defined(_MINGW_)
		nError	= ::closesocket(nSocket);
#else
		nError	= ::close(nSocket);
#endif	//defined(_WIN32_)||defined(_MINGW_)
	}

	return nError;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
