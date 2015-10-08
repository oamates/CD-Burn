///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZRTSPProtocol	Header File
	Create			20100721		ZHAOTT		RTSP
	Modify			20110602		ZHAOTT		RTCP
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZRTSPPROTOCOL_H_
#define	_ZRTSPPROTOCOL_H_
///////////////////////////////////////////////////////////////////////////////
#include "ZProtocol.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//METHOD
typedef	enum	_RTSP_PROTOCOL_METHOD_
{
	RTSP_PROTOCOL_OPTIONS,			//protocol options
	RTSP_PROTOCOL_DESCRIBE,			//description
	RTSP_PROTOCOL_ANNOUNCE,			//announce
	RTSP_PROTOCOL_SETUP,			//specifies the transport mechanism
	RTSP_PROTOCOL_PLAY,				//start sending data
	RTSP_PROTOCOL_PAUSE,			//pause stream delivery
	RTSP_PROTOCOL_TEARDOWN,			//stop stream delivery
	RTSP_PROTOCOL_GET_PARAMETER,	//retrieves the value of a parameter
	RTSP_PROTOCOL_SET_PARAMETER,	//set the value of a parameter
	RTSP_PROTOCOL_REDIRECT,			//redirect to new server
	RTSP_PROTOCOL_RECORD,			//recording a range of media data
	RTSP_PROTOCOL_DATA,				//rtp/rtcp
	RTSP_PROTOCOL_METHOD_INVALID	= PROTOCOL_METHOD_INVALID
}RTSP_PROTOCOL_METHOD;
///////////////////////////////////////////////////////////////////////////////
//TRANSPORT
typedef	enum	_RTSP_PROTOCOL_TRANSPORT_
{
	RTSP_PROTOCOL_TRANSPORT_UDP	= 0,	//RTP/AVP/UDP
	RTSP_PROTOCOL_TRANSPORT_TCP,		//RTP/AVP/TCP
	RTSP_TRANSPORTTYPE_RELIABLE_UDP		//RTP/AVP/UDP
}RTSP_PROTOCOL_TRANSPORT;
typedef	enum	_RTSP_PROTOCOL_TRANSPORT_PARAMTER_
{
	RTSP_PROTOCOL_TRANSPORT_UNICAST	= 0,	//UNICAST
	RTSP_PROTOCOL_TRANSPORT_MULTICAST,		//MULTICAST
}RTSP_PROTOCOL_TRANSPORT_PARAMTER;
///////////////////////////////////////////////////////////////////////////////
typedef	enum	_RTSP_PROTOCOL_HEADERS_
{
	RTSP_HEADERS_ACCEPT								= 0,	//"Accept",
	RTSP_HEADERS_CSEQ								= 1,	//"Cseq",
	RTSP_HEADERS_USERAGENT							= 2,	//"User-Agent",
	RTSP_HEADERS_TRANSPORT							= 3,	//"Transport",
	RTSP_HEADERS_SESSION							= 4,	//"Session",
	RTSP_HEADERS_RANGE								= 5,	//"Range",
	RTSP_HEADERS_MAIN								= 6,	//
	//
	RTSP_HEADERS_ACCEPTENCODING						= 6,	//"Accept-Encoding",
	RTSP_HEADERS_ACCEPTLANGUAGE						= 7,	//"Accept-Language",
	RTSP_HEADERS_AUTHORIZATION						= 8,	//"Authorization",
	RTSP_HEADERS_BANDWIDTH							= 9,	//"Bandwidth",
	RTSP_HEADERS_BLOCKSIZE							= 10,	//"Blocksize",
	RTSP_HEADERS_CACHECONTROL						= 11,	//"Cache-Control",
	RTSP_HEADERS_CONFERENCE							= 12,	//"Conference",
	RTSP_HEADERS_CONNECTION							= 13,	//"Connection",
	RTSP_HEADERS_CONTENTBASE						= 14,	//"Content-Base",
	RTSP_HEADERS_CONTENTENCODING					= 15,	//"Content-Encoding",
	RTSP_HEADERS_CONTENTLANGUAGE					= 16,	//"Content-Language",
	RTSP_HEADERS_CONTENTLENGTH						= 17,	//"Content-length",
	RTSP_HEADERS_CONTENTLOCATION					= 18,	//"Content-Location",
	RTSP_HEADERS_CONTENTTYPE						= 19,	//"Content-Type",
	RTSP_HEADERS_DATE								= 20,	//"Date",
	RTSP_HEADERS_EXPIRES							= 21,	//"Expires",
	RTSP_HEADERS_FROM								= 22,	//"From",
	RTSP_HEADERS_HOST								= 23,	//"Host",
	RTSP_HEADERS_IFMATCH							= 24,	//"If-Match",
	RTSP_HEADERS_ISMODIFIEDSINCE					= 25,	//"If-Modified-Since",
	RTSP_HEADERS_LASTMODIFIED						= 26,	//"Last-Modified",
	RTSP_HEADERS_LOCATION							= 27,	//"Location",
	RTSP_HEADERS_PROXYAUTHENTICATE					= 28,	//"Proxy-Authenticate",
	RTSP_HEADERS_PROXYREQUIRE						= 29,	//"Proxy-Require",
	RTSP_HEADERS_REFERER							= 30,	//"Referer",
	RTSP_HEADERS_RETRYAFTER							= 31,	//"Retry-After",
	RTSP_HEADERS_REQUIRE							= 32,	//"Require",
	RTSP_HEADERS_RTPINFO							= 33,	//"RTP-Info",
	RTSP_HEADERS_SCALE								= 34,	//"Scale",
	RTSP_HEADERS_SPEED								= 35,	//"Speed",
	RTSP_HEADERS_TIMESTAMP							= 36,	//"Timestamp",
	RTSP_HEADERS_VARY								= 37,	//"Vary",
	RTSP_HEADERS_VIA								= 38,	//"Via",
	RTSP_HEADERS_REQUEST							= 39,	//
	//
	RTSP_HEADERS_ALLOW								= 39,	//"Allow",
	RTSP_HEADERS_PUBLIC								= 40,	//"Public",
	RTSP_HEADERS_SERVER								= 41,	//"Server",
	RTSP_HEADERS_UNSUPPORTED						= 42,	//"Unsupported",
	RTSP_HEADERS_WWWAUTHENTICATE					= 43,	//,"WWW-Authenticate",
	RTSP_HEADERS_SAMEASLAST							= 44,	//",",
	RTSP_HEADERS_EXTENSION							= 45,	//
	//
	RTSP_HEADERS_XRETRANSMIT						= 45,	//"x-Retransmit",
	RTSP_HEADERS_XACCEPTRETRANSMIT					= 46,	//"x-Accept-Retransmit",
	RTSP_HEADERS_XRTPMETAINFO						= 47,	//"x-RTP-Meta-Info",
	RTSP_HEADERS_XTRANSPORTOPTIONS					= 48,	//"x-Transport-Options",
	RTSP_HEADERS_XPACKETRANGE						= 49,	//"x-Packet-Range",
	RTSP_HEADERS_XPREBUFFER							= 50,	//"x-Prebuffer",
	RTSP_HEADERS_XDYNAMICRATE						= 51,	//"x-Dynamic-Rate",
	RTSP_HEADERS_XACCEPTDYNAMICRATE					= 52,	//"x-Accept-Dynamic-Rate",
	//
	RTSP_HEADERS_XRANDOMDATASIZE					= 53,	//"x-Random-Data-Size",
	//3gpp
	RTSP_HEADERS_3GPPLINKCHAR						= 54,	//"3GPP-Link-Char",
	RTSP_HEADERS_3GPPADAPTATION						= 55,	//"3GPP-Adaptation",
	RTSP_HEADERS_3GPPQOEFEEDBACK					= 56,	//"3GPP-QoE-Feedback",
	RTSP_HEADERS_3GPPQOEMETRICS						= 57,	//"3GPP-QoE-Metrics",
	//Annex G
	RTSP_HEADERS_XPREDECBUFSIZE						= 58,	//"x-predecbufsize",
	RTSP_HEADERS_XINITPREDECBUFPERIOD				= 59,	//"x-initpredecbufperiod",
	RTSP_HEADERS_XINITPOSTDEBUFPERIOD				= 60,	//"x-initpostdecbufperiod",
	RTSP_HEADERS_3GPPVIDEOPOSTDECBUFSIZE			= 61,	//"3gpp-videopostdecbufsize",
	//
	RTSP_HEADERS_COUNT								= 62,	//
	RTSP_HEADERS_INVALID							= PROTOCOL_METHOD_INVALID

}RTSP_PROTOCOL_HEADERS;
typedef	enum	_RTSP_PROTOCOL_STATUS_
{
	RTSP_STATUS_CONTINUE							= 0,	//"100",	//Continue
	RTSP_STATUS_OK									= 1,	//"200",	//SuccessOK
	RTSP_STATUS_CREATE								= 2,	//"201",	//SuccessCreated
	RTSP_STATUS_ACCEPTED							= 3,	//"202",	//SuccessAccepted
	RTSP_STATUS_NOCONTENT							= 4,	//"204",	//SuccessNoContent
	RTSP_STATUS_PARTIALCONTENT						= 5,	//"206",	//SuccessPartialContent
	RTSP_STATUS_LOWONSTORAGE						= 6,	//"250",	//SuccessLowOnStorage
	RTSP_STATUS_MULTIPLECHOICES						= 7,	//"300",	//MultipleChoices
	RTSP_STATUS_REDIRECTPERMMOVED					= 8,	//"301",	//RedirectPermMoved
	RTSP_STATUS_REDIRECTTEMPMOVED					= 9,	//"302",	//RedirectTempMoved
	RTSP_STATUS_REDIRECTSEEOTHER					= 10,	//"303",	//RedirectSeeOther
	RTSP_STATUS_REDIRECTNOTMODIFIED					= 11,	//"304",	//RedirectNotModified
	RTSP_STATUS_USEPROXY							= 12,	//"305",	//UseProxy
	RTSP_STATUS_CLIENTBADREQUEST					= 13,	//"400",	//ClientBadRequest
	RTSP_STATUS_CLIENTUNAUTHORIZED					= 14,	//"401",	//ClientUnAuthorized
	RTSP_STATUS_PAYMENTREQUIRED						= 15,	//"402",	//PaymentRequired
	RTSP_STATUS_CLIENTFORBIDDEN						= 16,	//"403",	//ClientForbidden
	RTSP_STATUS_CLIENTNOTFOUND						= 17,	//"404",	//ClientNotFound
	RTSP_STATUS_CLIENTMETHODNOTALLOWED				= 18,	//"405",	//ClientMethodNotAllowed
	RTSP_STATUS_NOTACCEPTABLE						= 19,	//"406",	//NotAcceptable
	RTSP_STATUS_PROXYAUTHENTICATIONREQUIRED			= 20,	//"407",	//ProxyAuthenticationRequired
	RTSP_STATUS_REQUESTTIMEOUT						= 21,	//"408",	//RequestTimeout
	RTSP_STATUS_CLIENTCONFLICT						= 22,	//"409",	//ClientConflict
	RTSP_STATUS_GONE								= 23,	//"410",	//Gone
	RTSP_STATUS_LENGTHREQUIRED						= 24,	//"411",	//LengthRequired
	RTSP_STATUS_PRECONDITIONFAILED					= 25,	//"412",	//PreconditionFailed
	RTSP_STATUS_REQUESTENTITYTOOLARGE				= 26,	//"413",	//RequestEntityTooLarge
	RTSP_STATUS_REQUESTURITOOLARGE					= 27,	//"414",	//RequestURITooLarge
	RTSP_STATUS_UNSUPPORTEDMEDIATYPE				= 28,	//"415",	//UnsupportedMediaType
	RTSP_STATUS_CLIENTPARAMETERNOTUNDERSTOOD		= 29,	//"451",	//ClientParameterNotUnderstood
	RTSP_STATUS_CLIENTCONFERENCENOTFOUND			= 30,	//"452",	//ClientConferenceNotFound
	RTSP_STATUS_CLIENTNOTENOUGHBANDWIDTH			= 31,	//"453",	//ClientNotEnoughBandwidth
	RTSP_STATUS_CLIENTSESSIONNOTFOUND				= 32,	//"454",	//ClientSessionNotFound
	RTSP_STATUS_CLIENTMETHODNOTVALIDINSTATE			= 33,	//"455",	//ClientMethodNotValidInState
	RTSP_STATUS_CLIENTHEADERFIELDNOTVALID			= 34,	//"456",	//ClientHeaderFieldNotValid
	RTSP_STATUS_CLIENTINVALIDRANGE					= 35,	//"457",	//ClientInvalidRange
	RTSP_STATUS_CLIENTREADONLYPARAMTER				= 36,	//"458",	//ClientReadOnlyParameter
	RTSP_STATUS_CLIENTAGGREGATEOPTIONNOTALLOWED		= 37,	//"459",	//ClientAggregateOptionNotAllowed
	RTSP_STATUS_CLIENTAGGREGATEOPTIONALLOWED		= 38,	//"460",	//ClientAggregateOptionAllowed
	RTSP_STATUS_CLIENTUNSUPPORTEDTRANSPORT			= 39,	//"461",	//ClientUnsupportedTransport
	RTSP_STATUS_CLIENTDESTINATIONUNREACHABLE		= 40,	//"462",	//ClientDestinationUnreachable
	RTSP_STATUS_SERVERINTERNAL						= 41,	//"500",	//ServerInternal
	RTSP_STATUS_SERVERNOTIMPLEMENTED				= 42,	//"501",	//ServerNotImplemented
	RTSP_STATUS_SERVERBADGATEWAY					= 43,	//"502",	//ServerBadGateway
	RTSP_STATUS_SERVERUNAVAILABLE					= 44,	//"503",	//ServerUnavailable
	RTSP_STATUS_SERVERGATEWAYTIMEOUT				= 45,	//"504",	//ServerGatewayTimeout
	RTSP_STATUS_RTSPVERSIONNOTSUPPORTED				= 46,	//"505",	//RTSPVersionNotSupported
	RTSP_STATUS_SERVEROPTIONNOTSUPPORTED			= 47,	//"551",	//ServerOptionNotSupported
}RTSP_PROTOCOL_STATUS;
///////////////////////////////////////////////////////////////////////////////
class ZRTSPProtocol : public ZProtocol
{
///////////////////////////////////////////////////////////////////////////////
public:
	static	CHAR*	GetProtocolName();
	static	CHAR*	GetProtocolAccept();
	static	CHAR*	GetProtocolUserAgent();
	static	CHAR*	GetProtocolPublic();
	static	CHAR*	GetProtocolServer();
	static	BOOL	IsProtocol(CONST CHAR* sProtocolName);
	static	CHAR*	GetProtocolMethod(CONST UINT nMethod);
	static	UINT	GetProtocolMethod(CONST CHAR* sMethod);
	static	CHAR*	GetProtocolHeader(CONST UINT nHeader);
	static	UINT	GetProtocolHeader(CONST CHAR* sHeader);
	static	CHAR*	GetProtocolStatusCode(CONST UINT nStatus);
	static	CHAR*	GetProtocolStatusLabel(CONST UINT nStatus);
	static	CHAR*	GetProtocolTransport(CONST UINT nTransport);
	static	CHAR*	GetProtocolTransportParamter(CONST UINT nTransportParamter);
///////////////////////////////////////////////////////////////////////////////
protected:
	static	const	char*	m_sRTSPProtocolName;
	static	const	char*	m_sRTSPProtocolUserAgent;
	static	const	char*	m_sRTSPProtocolServer;
	static	const	char*	m_sRTSPProtocolAccept;
	static	const	char*	m_sRTSPProtocolPublic;
	static	const	char*	m_sRTSPTransport[];
	static	const	char*	m_sRTSPTransportParamter[];
	static	const	char*	m_sRTSPProtocolMethods[];
	static	const	char*	m_sRTSPProtocolHeaders[];
	static	const	char*	m_sRTSPStatusCodes[];
	static	const	char*	m_sRTSPStatusLabel[];
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZRTSPPROTOCOL_H_
///////////////////////////////////////////////////////////////////////////////
