///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project				ZMediaServer
	ZTCPListenerSocket	Header File
	Create				20100617		ZHAOTT
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZTCPLISTENERSOCKET_H_
#define	_ZTCPLISTENERSOCKET_H_
///////////////////////////////////////////////////////////////////////////////
#include "ZTCPSocket.h"
#include "ZIdleTask.h"
///////////////////////////////////////////////////////////////////////////////
class	ZTCPListenerEvent;
class	ZTCPListenerSocket;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef	struct	_NET_LISTENER_
{
	char	m_sName[MAX_KEY_LABEL+4];
	int		m_nAddress;
	int		m_nPort;
}NET_LISTENER,*PNET_LISTENER;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ZTCPListenerEvent
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZTCPListenerEvent();
	virtual	~ZTCPListenerEvent();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	OnListenerEvent(const int hAccept,const struct sockaddr_in* pLocalAddr,const struct sockaddr_in* pRemoteAddr)	= 0;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
class ZTCPListenerSocket : public ZTCPSocket , public ZIdleTask
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZTCPListenerSocket();
	virtual ~ZTCPListenerSocket();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	Create(UINT nAddr,UINT nPort, INT nAddrFamily = AF_INET);
	virtual	BOOL	Close();
	virtual	int		Accept();
///////////////////////////////////////////////////////////////////////////////
public:
	void				SlowSpeed();
	void				NormalSpeed();
///////////////////////////////////////////////////////////////////////////////
public:
	int					GetAcceptSocket();
	struct sockaddr_in*	GetAcceptAddr();
///////////////////////////////////////////////////////////////////////////////
public:
	ZTCPListenerEvent*	GetListenerEvent();
	ZTCPListenerEvent*	SetListenerEvent(ZTCPListenerEvent* pListenerEvent);
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	int	Run(int nEvent = 0);
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual BOOL ProcessEvent(UINT nEvent);
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	BOOL	Listen(UINT nListenCount);
///////////////////////////////////////////////////////////////////////////////
protected:
	BOOL				m_bSlowAccept;
	UINT				m_nSecondsBetweenAccepts;
	UINT				m_nListenCount;
	int					m_hAcceptSocket;
	struct	sockaddr_in	m_AcceptAddr;
///////////////////////////////////////////////////////////////////////////////
protected:
	ZOSMutex			m_ListenerMutex;
	ZTCPListenerEvent*	m_pListenerEvent;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZTCPLISTENERSOCKET_H_
///////////////////////////////////////////////////////////////////////////////
