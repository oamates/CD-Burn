#include "ZRTSPProtocol.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const char*	ZRTSPProtocol::m_sRTSPProtocolName		= "RTSP/1.0";
//const char*	ZRTSPProtocol::m_sRTSPProtocolUserAgent	= "ZMedia/1.1(ZMedia Lib)";
const char*	ZRTSPProtocol::m_sRTSPProtocolUserAgent	= "LibVLC/2.1.3 (LIVE555 Streaming Media v2014.01.21z)";
const char*	ZRTSPProtocol::m_sRTSPProtocolServer	= "ZMedia Server(2.0)";
const char*	ZRTSPProtocol::m_sRTSPProtocolAccept	= "application/sdp";
const char*	ZRTSPProtocol::m_sRTSPProtocolPublic	= "OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN, GET_PARAMETER";
///////////////////////////////////////////////////////////////////////////////
const char*	ZRTSPProtocol::m_sRTSPTransport[]	=
{
	"RTP/AVP",			//UDP
	"RTP/AVP/TCP",		//TCP
};
const char*	ZRTSPProtocol::m_sRTSPTransportParamter[]	=
{
	"unicast",			//UNICAST
	"multicast",		//MULTICAST
};
///////////////////////////////////////////////////////////////////////////////
const char*	ZRTSPProtocol::m_sRTSPProtocolMethods[]	=
{
	"OPTIONS",			//protocol options
	"DESCRIBE",			//description
	"ANNOUNCE",			//announce
	"SETUP",			//specifies the transport mechanism
	"PLAY",				//start sending data
	"PAUSE",			//pause stream delivery
	"TEARDOWN",			//stop stream delivery
	"GET_PARAMETER",	//retrieves the value of a parameter
	"SET_PARAMETER",	//set the value of a parameter
	"REDIRECT",			//redirect to new server
	"RECORD",			//recording a range of media data
};
///////////////////////////////////////////////////////////////////////////////
const char*	ZRTSPProtocol::m_sRTSPProtocolHeaders[]	=
{
	"Accept",
	"CSeq",
	"User-Agent",
	"Transport",
	"Session",
	"Range",
	//
	"Accept-Encoding",
	"Accept-Language",
	"Authorization",
	"Bandwidth",
	"Blocksize",
	"Cache-Control",
	"Conference",
	"Connection",
	"Content-Base",
	"Content-Encoding",
	"Content-Language",
	"Content-Length",
	"Content-Location",
	"Content-Type",
	"Date",
	"Expires",
	"From",
	"Host",
	"If-Match",
	"If-Modified-Since",
	"Last-Modified",
	"Location",
	"Proxy-Authenticate",
	"Proxy-Require",
	"Referer",
	"Retry-After",
	"Require",
	"RTP-Info",
	"Scale",
	"Speed",
	"timestamp",//"Timestamp",
	"Vary",
	"Via",
	"Allow",
	"Public",
	"Server",
	"Unsupported",
	"WWW-Authenticate",
	",",
	"x-Retransmit",
	"x-Accept-Retransmit",
	"x-RTP-Meta-Info",
	"x-Transport-Options",
	"x-Packet-Range",
	"x-Prebuffer",
	"x-Dynamic-Rate",
	"x-Accept-Dynamic-Rate",
	//
	"x-Random-Data-Size",
	//3gpp
	"3GPP-Link-Char",
	"3GPP-Adaptation",
	"3GPP-QoE-Feedback",
	"3GPP-QoE-Metrics",
	//Annex G
	"x-predecbufsize",
	"x-initpredecbufperiod",
	"x-initpostdecbufperiod",
	"3gpp-videopostdecbufsize",
};
///////////////////////////////////////////////////////////////////////////////
const char*	ZRTSPProtocol::m_sRTSPStatusCodes[]	= 
{
	"100",	//Continue
	"200",	//SuccessOK
	"201",	//SuccessCreated
	"202",	//SuccessAccepted
	"204",	//SuccessNoContent
	"206",	//SuccessPartialContent
	"250",	//SuccessLowOnStorage
	"300",	//MultipleChoices
	"301",	//RedirectPermMoved
	"302",	//RedirectTempMoved
	"303",	//RedirectSeeOther
	"304",	//RedirectNotModified
	"305",	//UseProxy
	"400",	//ClientBadRequest
	"401",	//ClientUnAuthorized
	"402",	//PaymentRequired
	"403",	//ClientForbidden
	"404",	//ClientNotFound
	"405",	//ClientMethodNotAllowed
	"406",	//NotAcceptable
	"407",	//ProxyAuthenticationRequired
	"408",	//RequestTimeout
	"409",	//ClientConflict
	"410",	//Gone
	"411",	//LengthRequired
	"412",	//PreconditionFailed
	"413",	//RequestEntityTooLarge
	"414",	//RequestURITooLarge
	"415",	//UnsupportedMediaType
	"451",	//ClientParameterNotUnderstood
	"452",	//ClientConferenceNotFound
	"453",	//ClientNotEnoughBandwidth
	"454",	//ClientSessionNotFound
	"455",	//ClientMethodNotValidInState
	"456",	//ClientHeaderFieldNotValid
	"457",	//ClientInvalidRange
	"458",	//ClientReadOnlyParameter
	"459",	//ClientAggregateOptionNotAllowed
	"460",	//ClientAggregateOptionAllowed
	"461",	//ClientUnsupportedTransport
	"462",	//ClientDestinationUnreachable
	"500",	//ServerInternal
	"501",	//ServerNotImplemented
	"502",	//ServerBadGateway
	"503",	//ServerUnavailable
	"504",	//ServerGatewayTimeout
	"505",	//RTSPVersionNotSupported
	"551",	//ServerOptionNotSupported
};
///////////////////////////////////////////////////////////////////////////////
const char*	ZRTSPProtocol::m_sRTSPStatusLabel[]	=
{
	"Continue",								//Continue
	"OK",									//SuccessOK
	"Created",								//SuccessCreated
	"Accepted",								//SuccessAccepted
	"No Content",							//SuccessNoContent
	"Partial Content",						//SuccessPartialContent
	"Low on Storage Space",					//SuccessLowOnStorage
	"Multiple Choices",						//MultipleChoices
	"Moved Permanently",					//RedirectPermMoved
	"Found",								//RedirectTempMoved
	"See Other",							//RedirectSeeOther
	"Not Modified",							//RedirectNotModified
	"Use Proxy",							//UseProxy
	"Bad Request",							//ClientBadRequest
	"Unauthorized",							//ClientUnAuthorized
	"Payment Required",						//PaymentRequired
	"Forbidden",							//ClientForbidden
	"Not Found",							//ClientNotFound
	"Method Not Allowed",					//ClientMethodNotAllowed
	"Not Acceptable",						//NotAcceptable
	"Proxy Authentication Required",		//ProxyAuthenticationRequired
	"Request Time-out",						//RequestTimeout
	"Conflict",								//ClientConflict
	"Gone",									//Gone
	"Length Required",						//LengthRequired
	"Precondition Failed",					//PreconditionFailed
	"Request Entity Too Large",				//RequestEntityTooLarge
	"Request-URI Too Large",				//RequestURITooLarge
	"Unsupported Media Type",				//UnsupportedMediaType
	"Parameter Not Understood",				//ClientParameterNotUnderstood
	"Conference Not Found",					//ClientConferenceNotFound
	"Not Enough Bandwidth",					//ClientNotEnoughBandwidth
	"Session Not Found",					//ClientSessionNotFound
	"Method Not Valid in this State",		//ClientMethodNotValidInState
	"Header Field Not Valid For Resource",	//ClientHeaderFieldNotValid
	"Invalid Range",						//ClientInvalidRange
	"Parameter Is Read-Only",				//ClientReadOnlyParameter
	"Aggregate Option Not Allowed",			//ClientAggregateOptionNotAllowed
	"Only Aggregate Option Allowed",		//ClientAggregateOptionAllowed
	"Unsupported Transport",				//ClientUnsupportedTransport
	"Destination Unreachable",				//ClientDestinationUnreachable
	"Internal Server Error",				//ServerInternal
	"Not Implemented",						//ServerNotImplemented
	"Bad Gateway",							//ServerBadGateway
	"Service Unavailable",					//ServerUnavailable
	"Gateway Timeout",						//ServerGatewayTimeout
	"RTSP Version not supported",			//RTSPVersionNotSupported
	"Option Not Supported",					//ServerOptionNotSupported
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CHAR* ZRTSPProtocol::GetProtocolName()
{
	return (CHAR*)ZRTSPProtocol::m_sRTSPProtocolName;
}
CHAR* ZRTSPProtocol::GetProtocolAccept()
{
	return (CHAR*)ZRTSPProtocol::m_sRTSPProtocolAccept;
}
CHAR* ZRTSPProtocol::GetProtocolUserAgent()
{
	return (CHAR*)ZRTSPProtocol::m_sRTSPProtocolUserAgent;
}
CHAR* ZRTSPProtocol::GetProtocolPublic()
{
	return (CHAR*)ZRTSPProtocol::m_sRTSPProtocolPublic;
}
CHAR* ZRTSPProtocol::GetProtocolServer()
{
	return (CHAR*)ZRTSPProtocol::m_sRTSPProtocolServer;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTSPProtocol::IsProtocol(CONST CHAR* sProtocolName)
{
	if(sProtocolName != NULL)
	{
		return (strcasecmp(ZRTSPProtocol::m_sRTSPProtocolName,sProtocolName) == 0);
	}
	return FALSE;
}
CHAR* ZRTSPProtocol::GetProtocolMethod(CONST UINT nMethod)
{
	if(nMethod >= 0 && nMethod < COUNT_OF(ZRTSPProtocol::m_sRTSPProtocolMethods))
	{
		return (CHAR*)(ZRTSPProtocol::m_sRTSPProtocolMethods[nMethod]);
	}
	return NULL;
}
UINT ZRTSPProtocol::GetProtocolMethod(CONST CHAR* sMethod)
{
	int		nIndex	= 0;
	int		nCount	= COUNT_OF(ZRTSPProtocol::m_sRTSPProtocolMethods);
	UINT	nCheck	= PROTOCOL_METHOD_INVALID;
	UINT	nReturn	= PROTOCOL_METHOD_INVALID;

	if(sMethod != NULL)
	{
		switch(sMethod[0])
		{
		case	'O':
		case	'o':
			{
				nCheck	= RTSP_PROTOCOL_OPTIONS;
			}
			break;
		case	'D':
		case	'd':
			{
				nCheck	= RTSP_PROTOCOL_DESCRIBE;
			}
			break;
		case	'S':
		case	's':
			{
				nCheck	= RTSP_PROTOCOL_SETUP;
			}
			break;
		case	'A':
		case	'a':
			{
				nCheck	= RTSP_PROTOCOL_ANNOUNCE;
			}
			break;
		case	'T':
		case	't':
			{
				nCheck	= RTSP_PROTOCOL_TEARDOWN;
			}
			break;
        case     'G':
        case     'g':
            {
                nCheck  = RTSP_PROTOCOL_GET_PARAMETER;
            }
            break;
		default:
			break;
		}
		if(nCheck != PROTOCOL_METHOD_INVALID)
		{
			if(strcasecmp(ZRTSPProtocol::m_sRTSPProtocolMethods[nCheck],sMethod) == 0)
			{
				nReturn	= (UINT)nCheck;
			}

		}
		if(nReturn == PROTOCOL_METHOD_INVALID)
		{
			for(nIndex = 0; nIndex < nCount; nIndex ++)
			{
				if(strcasecmp(ZRTSPProtocol::m_sRTSPProtocolMethods[nIndex],sMethod) == 0)
				{
					nReturn	= (UINT)nIndex;
					break;
				}
			}
		}
	}

	return nReturn;
}
///////////////////////////////////////////////////////////////////////////////
CHAR* ZRTSPProtocol::GetProtocolHeader(CONST UINT nHeader)
{
	if(nHeader >= 0 && nHeader < COUNT_OF(ZRTSPProtocol::m_sRTSPProtocolHeaders))
	{
		return (CHAR*)(ZRTSPProtocol::m_sRTSPProtocolHeaders[nHeader]);
	}
	return NULL;
}
UINT ZRTSPProtocol::GetProtocolHeader(CONST CHAR* sHeader)
{
	int		nIndex	= 0;
	int		nCount	= COUNT_OF(ZRTSPProtocol::m_sRTSPProtocolHeaders);
	UINT	nCheck	= PROTOCOL_METHOD_INVALID;
	UINT	nReturn	= PROTOCOL_METHOD_INVALID;

	if(sHeader != NULL)
	{
		switch(sHeader[0])
		{
		case	'A':
		case	'a':
			{
				nCheck	= RTSP_HEADERS_ACCEPT;
			}
			break;
		case	'C':
		case	'c':
			{
				nCheck	= RTSP_HEADERS_CSEQ;
			}
			break;
		case	'R':
		case	'r':
			{
				nCheck	= RTSP_HEADERS_RANGE;
			}
			break;
		case	'S':
		case	's':
			{
				nCheck	= RTSP_HEADERS_SESSION;
			}
			break;
		case	'T':
		case	't':
			{
				nCheck	= RTSP_HEADERS_TRANSPORT;
			}
			break;
		case	'U':
		case	'u':
			{
				nCheck	= RTSP_HEADERS_USERAGENT;
			}
			break;
		case	'X':
		case	'x':
			{
				nCheck	= RTSP_HEADERS_XRETRANSMIT;
			}
			break;
		default:
			break;
		}
		if(nCheck != PROTOCOL_METHOD_INVALID)
		{
			if(strcasecmp(ZRTSPProtocol::m_sRTSPProtocolHeaders[nCheck],sHeader) == 0)
			{
				nReturn	= (UINT)nCheck;
			}

		}
		if(nReturn == PROTOCOL_METHOD_INVALID)
		{
			for(nIndex = 0; nIndex < nCount; nIndex ++)
			{
				if(strcasecmp(ZRTSPProtocol::m_sRTSPProtocolHeaders[nIndex],sHeader) == 0)
				{
					nReturn	= (UINT)nIndex;
					break;
				}
			}
		}
	}

	return nReturn;
}
///////////////////////////////////////////////////////////////////////////////
CHAR* ZRTSPProtocol::GetProtocolStatusCode(CONST UINT nStatus)
{
	if(nStatus >= 0 && nStatus < COUNT_OF(ZRTSPProtocol::m_sRTSPStatusCodes))
	{
		return (CHAR*)(ZRTSPProtocol::m_sRTSPStatusCodes[nStatus]);
	}
	return NULL;
}
CHAR* ZRTSPProtocol::GetProtocolStatusLabel(CONST UINT nStatus)
{
	if(nStatus >= 0 && nStatus < COUNT_OF(ZRTSPProtocol::m_sRTSPStatusLabel))
	{
		return (CHAR*)(ZRTSPProtocol::m_sRTSPStatusLabel[nStatus]);
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
CHAR* ZRTSPProtocol::GetProtocolTransport(CONST UINT nTransport)
{
	if(nTransport >= 0 && nTransport < COUNT_OF(ZRTSPProtocol::m_sRTSPTransport))
	{
		return (CHAR*)(ZRTSPProtocol::m_sRTSPTransport[nTransport]);
	}
	return NULL;
}
CHAR* ZRTSPProtocol::GetProtocolTransportParamter(CONST UINT nTransportParamter)
{
	if(nTransportParamter >= 0 && nTransportParamter < COUNT_OF(ZRTSPProtocol::m_sRTSPTransportParamter))
	{
		return (CHAR*)(ZRTSPProtocol::m_sRTSPTransportParamter[nTransportParamter]);
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
