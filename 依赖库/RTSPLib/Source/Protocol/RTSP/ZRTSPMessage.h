///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project			ZMediaServer
	ZRTSPMessage	Header File
	Create			20100721		ZHAOTT		RTSP
	Modify			20110602		ZHAOTT		RTCP
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZRTSPMESSAGE_H_
#define	_ZRTSPMESSAGE_H_
///////////////////////////////////////////////////////////////////////////////
#include "ZSessionMessage.h"
#include "ZRTSPProtocol.h"
///////////////////////////////////////////////////////////////////////////////
class	ZSessionMessage;
class	ZRTSPSession;
class	ZSessionStream;
class	ZHeaderParser;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class	ZRTSPMessage : public ZSessionMessage
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZRTSPMessage(ZSession* pSession,ZSessionStream* pSessionStream);
	virtual	~ZRTSPMessage();
///////////////////////////////////////////////////////////////////////////////
public:
	DWORD	GetTransportType();
	DWORD	SetTransportType(DWORD nType);
	DWORD	GetTransportParamter();
	DWORD	SetTransportParamter(DWORD nParamter);
///////////////////////////////////////////////////////////////////////////////
public:
	int		GetClientAddr();
	int		GetClientPortA();
	int		GetClientPortB();
	int		GetServerAddr();
	int		GetServerPortA();
	int		GetServerPortB();
	int		GetSessionAddr();
	int		GetChannelA();
	int		GetChannelB();
	void	SetClientPort(int nPortA,int nPortB);
	void	SetServerPort(int nPortA,int nPortB);
	void	SetChannel(int nChannelA,int nChannelB);
	void	SetServerAddr(int nServerAddr);
    char*   GetURI();
	char*	GetHost();
///////////////////////////////////////////////////////////////////////////////
public:
	char*	GetContentBase();
///////////////////////////////////////////////////////////////////////////////
public:
	int		GetStatusCode();
///////////////////////////////////////////////////////////////////////////////
public:
	BOOL	Parse(ZHeaderParser* pHeader);
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	BOOL	ParseFirstLine(ZHeaderParser* pHeader)	= 0;
///////////////////////////////////////////////////////////////////////////////
protected:
	BOOL	ParseHeaders(ZHeaderParser* pHeader);
///////////////////////////////////////////////////////////////////////////////
protected:
	BOOL	ParseCSeq(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseTransport(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseSession(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseRange(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseSpeed(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseContentLength(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseConnection(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseContentBase(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseContentType(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseAuthorization(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseAuthentication(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseBasicAuthorization(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseDigestAuthorization(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseBasicAuthentication(ZHeaderParser* pHeader,int nLine,int nPos);
	BOOL	ParseDigestAuthentication(ZHeaderParser* pHeader,int nLine,int nPos);
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	void	GetFirstLine(char*)	= 0;
	virtual	void	GetEndLine(char*)	= 0;
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	VOID	GetAuthorization(CHAR*);
	virtual	VOID	GetAuthentication(CHAR*);
///////////////////////////////////////////////////////////////////////////////
protected:
	void	GetHeaders(char* sMessage,RTSP_PROTOCOL_HEADERS nHeader);
	void	GetHeaders(char* sMessage,RTSP_PROTOCOL_HEADERS nHeader,const char* sValue);
	void	GetHeaders(char* sMessage,RTSP_PROTOCOL_HEADERS nHeader,int nValue);
	void	GetHeaders(char* sMessage,RTSP_PROTOCOL_HEADERS nHeader,float nValue);
	void	GetHeaders(char* sMessage,RTSP_PROTOCOL_HEADERS nHeader,double nValue);
	void	GetTransport(char* sMessage);
    void    GetRange(char *sMessage, double nStart, double nStop);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
protected:
	int		m_nStatusCode;
	BOOL	m_bConnectClose;
///////////////////////////////////////////////////////////////////////////////
protected:
	DWORD	m_nTransport;
	DWORD	m_nTransportParamter;
///////////////////////////////////////////////////////////////////////////////
private:
	int		m_nClientAddr;
	int		m_nClientPortA;
	int		m_nClientPortB;
	int		m_nServerAddr;
	int		m_nServerPortA;
	int		m_nServerPortB;
	int		m_nChannelA;
	int		m_nChannelB;
///////////////////////////////////////////////////////////////////////////////
protected:
    char    m_sURI[(MAX_URI_PATH+4)];
	char	m_sHost[(MAX_FILE_NAME+4)];
	char	m_sUser[(MAX_KEY_LABEL+4)];
	char	m_sPass[(MAX_KEY_LABEL+4)];
	char	m_sChannel[(MAX_URI_PATH+4)];
	char	m_sFilePath[(MAX_FILE_PATH+4)];
    char	m_sTrackPath[(MAX_FILE_PATH+4)];
///////////////////////////////////////////////////////////////////////////////
protected:
	char	m_sContentBase[(MAX_KEY_LABEL+4)];
///////////////////////////////////////////////////////////////////////////////
public:
	friend	class	ZRTSPSession;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZRTSPMESSAGE_H_
///////////////////////////////////////////////////////////////////////////////
