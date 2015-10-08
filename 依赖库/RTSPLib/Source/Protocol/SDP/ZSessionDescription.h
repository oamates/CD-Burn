///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZRTCPTask	Header File
	Create		20110321		ZHAOTT
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZSESSIONDESCRIPTION_H_
#define	_ZSESSIONDESCRIPTION_H_
///////////////////////////////////////////////////////////////////////////////
#include "ZSession.h"
#include "ZOSArray.h"
//#include "ZMedia.h"
///////////////////////////////////////////////////////////////////////////////
class	ZHeaderParser;
class	ZSessionDescription;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ZSessionDescription : public ZObject
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZSessionDescription();
	virtual ~ZSessionDescription();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	InitDescription(char* sBuffer,int nBuffer,char* sURI);
	virtual	BOOL	ParseDescription(char* sBuffer,int nBuffer,char* sURI);
///////////////////////////////////////////////////////////////////////////////
public:
typedef	struct	_SDP_HEADER_
{
	DWORD		m_nVersion;
	DWORD		m_nAddress;
	DWORD		m_nTimeToLive;
	DWORD		m_nStartTime;
	DWORD		m_nEndTime;
}SDP_HEADER;
typedef	struct	_SDP_MEDIA_
{
	STREAM_TYPE	m_nMediaType;
	DWORD		m_nMediaSourceAddr;
	DWORD		m_nMediaDestinationsAddr;
	DWORD		m_nMediaDestinationsPort;
	DWORD		m_nMediaTimeToLive;
	DWORD		m_nMediaPayloadType;
	CHAR		m_sMediaPayloadName[MAX_KEY_LABEL+4];
	DWORD		m_nMediaFrequency;
	CHAR		m_sMediaControlName[MAX_KEY_LABEL+4];
	DWORD		m_nMediaTrackID;
}SDP_MEDIA;
typedef	ZOSArray<SDP_MEDIA*>	SDPStreamArray;
///////////////////////////////////////////////////////////////////////////////
public:
	DWORD			GetStreamCount();
	SDP_MEDIA*		GetStream(DWORD nIndex);
	DWORD           GetMediaDstAddr();
	DWORD           GetMediaDstPort(DWORD nIndex);
///////////////////////////////////////////////////////////////////////////////
public:
	BOOL			AddMedia();
///////////////////////////////////////////////////////////////////////////////
protected:
	BOOL			ParseVersion(ZHeaderParser* pHeader,int nLine,int* nPos);
	BOOL			ParseConnection(ZHeaderParser* pHeader,int nLine,int* nPos);
	BOOL			ParseBandwidth(ZHeaderParser* pHeader,int nLine,int* nPos);
	BOOL			ParseTimes(ZHeaderParser* pHeader,int nLine,int* nPos);
	BOOL			ParseAttributes(ZHeaderParser* pHeader,int nLine,int* nPos);
	BOOL			ParseMedia(ZHeaderParser* pHeader,int nLine,int* nPos);
///////////////////////////////////////////////////////////////////////////////
protected:
	CHAR			m_sURI[MAX_URI_PATH+4];
	SDP_HEADER		m_SDPHeader;
	SDPStreamArray	m_SDPStream;
	DWORD			m_nSDPStream;
protected:
	//value of session-level a=Control.
	CHAR			m_sControlNameBase[MAX_KEY_LABEL+4];
///////////////////////////////////////////////////////////////////////////////
protected:
	static	const	char	m_sSDPHeaderFormat[];
	static	const	char	m_sSDPMediaFormat[];
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZSESSIONDESCRIPTION_H_
///////////////////////////////////////////////////////////////////////////////
