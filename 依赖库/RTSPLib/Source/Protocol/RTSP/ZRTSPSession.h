///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project			ZMediaServer
	ZRTSPSession	Header File
	Create			20100721		ZHAOTT		RTSP
	Modify			20110602		ZHAOTT		RTCP
	Modify			20110708		ZHAOTT		RTP Over RTSP
	Modify			20120226		ZHAOTT		Reliable UDP
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZRTSPSESSION_H_
#define	_ZRTSPSESSION_H_
///////////////////////////////////////////////////////////////////////////////
#include "ZSession.h"
#include "ZRTPSession.h"
#include "ZRTSPRequest.h"
#include "ZRTSPResponse.h"
#include "ZSessionDescription.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class	ZSource;
//class	ZChannel;
class	ZRTPSession;
class	ZRTSPSession;
class	ZRTSPMessage;
class   RTSPClientListener;
class   RTSPServerListener;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ZRTSPSession : public ZSession
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZRTSPSession(SESSION_SUBTYPE eSubType);
	virtual ~ZRTSPSession();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	Create();
	virtual	BOOL	Close();
///////////////////////////////////////////////////////////////////////////////
public:
    VOID SetRTSPClientListener(RTSPClientListener *pListener);
    VOID SetRTSPServerListener(RTSPServerListener *pListener);
public:
	virtual	BOOL	SetSessionStream(ZBaseStream* pStream);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	UINT			GetDataStreamCount();
	virtual	ZBaseStream*	GetDataStream(int i);
	virtual	ZDataPin*		GetDataPin(int i);
	//virtual BOOL			Status(STATUS_TYPE	eRequestType, char *sStatus);
	virtual BOOL			OnDataPinClose();
///////////////////////////////////////////////////////////////////////////////
public:
	//virtual BOOL	IsLiveSession();
///////////////////////////////////////////////////////////////////////////////
public:
	char*			GetTrackPath();

	char*           GetRangeString();
	void            SetRtpInfo(char* streamUrl);
///////////////////////////////////////////////////////////////////////////////
public:
    DWORD           GetStream(ZRTSPRequest *pRTSPRequest);
public:
	DWORD			GetTransport();
	DWORD			SetTransport(DWORD nTransport);
	DWORD			GetTransportParamter();
	DWORD			SetTransportParamter(DWORD nTransportParamter);
	RTSP_PROTOCOL_TRANSPORT	GetProtocolTransportType();
	RTSP_PROTOCOL_TRANSPORT	SetProtocolTransportType(RTSP_PROTOCOL_TRANSPORT eProtocolTransportType);
    void            SetSessionTimeoutMilliSecond(UINT64 nTimeoutMilliSecond);
    int             SetEachTimeSendCount(int nSendCount);
///////////////////////////////////////////////////////////////////////////////
public:
	int				GetSessionAddr();
    int             GetSessionPort();
    UINT            GetSessionDuration();
    UINT64          GetSessionTotalPacketCount();
    UINT64          GetSessionLostPacketCount();
	DWORD           GetMediaDstAddr();
	DWORD           GetMediaDstPort(DWORD index);
///////////////////////////////////////////////////////////////////////////////
public:
	RTSP_PROTOCOL_STATUS
					GetRTSPSessionStatus();
	RTSP_PROTOCOL_STATUS
					SetRTSPSessionStatus(RTSP_PROTOCOL_STATUS nStatus);
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	int		ServerRun(UINT nEvent);
	virtual	int		ClientRun(UINT nEvent);
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	BOOL	ProcessRequest(ZRTSPRequest* pRTSPRequest);
	virtual	BOOL	ProcessDescribe();
	virtual	BOOL	SetupStream();
	virtual	BOOL	ProcessSetup();
	virtual	BOOL	ProcessPlay();
	virtual	BOOL	ProcessTearDown();
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	BOOL	DoDescribe();
	virtual	BOOL	DoSetup();
	virtual	BOOL	DoPlay();
	virtual	BOOL	DoPause();
	virtual	BOOL	DoTearDown();
	virtual	BOOL	DoAnnounce();
    virtual BOOL    DoGetParameter();
    virtual BOOL    DoSetParameter();
	virtual	BOOL	DoData();
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	BOOL	DoTaskKill();
	virtual	BOOL	DoTimeOut();
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	BOOL	InitRTPSession();
///////////////////////////////////////////////////////////////////////////////
protected:
	BOOL			CreateSDP();
	BOOL			ParseSDP();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual char *	GetSessionURI();
///////////////////////////////////////////////////////////////////////////////
protected:
	ZSessionStream	m_SessionStream;
	ZRTSPRequest	m_Request;
	ZRTSPResponse	m_Response;
///////////////////////////////////////////////////////////////////////////////
protected:
	ZRTPSession		m_RTPSession;
///////////////////////////////////////////////////////////////////////////////
protected:
	DWORD			m_nTransport;
	DWORD			m_nTransportParamter;
	RTSP_PROTOCOL_TRANSPORT		m_eProtocolTransportType;
	DWORD                       m_nStreamType[2];
protected:
    RTSPClientListener      *m_pRTSPClientListener;
    RTSPServerListener      *m_pRTSPServerListener;
///////////////////////////////////////////////////////////////////////////////
protected:
    BOOL                    m_bDoDescribe;
///////////////////////////////////////////////////////////////////////////////
protected:
	ZSessionDescription		m_aSessionDescription;
	DWORD					m_nStreamSetup;
///////////////////////////////////////////////////////////////////////////////
protected:
	UINT64					m_nPlayStartTime;
    UINT64                  m_nSessionStopTime;
	UINT64					m_nLastRTCPTime;
	RTSP_PROTOCOL_STATUS	m_SessionStatus;

    int                     m_nSessionTimeoutMilliSecond;

	char*                   m_RtpInfo;
	char*                   m_pRangeString;
protected:
    BOOL                    m_bFirstCommand;
///////////////////////////////////////////////////////////////////////////////
public:
	static	ZSession*	CreateSession(SESSION_SUBTYPE eSubType,ZSocket* pSocket,char* sURI,
		RTSP_PROTOCOL_TRANSPORT eProtocolTransport,char* sUser,char* sPassword);
///////////////////////////////////////////////////////////////////////////////
public:
	friend	class	ZRTSPMessage;
	friend	class	ZRTSPRequest;
	friend	class	ZRTSPResponse;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZRTSPSESSION_H_
///////////////////////////////////////////////////////////////////////////////
