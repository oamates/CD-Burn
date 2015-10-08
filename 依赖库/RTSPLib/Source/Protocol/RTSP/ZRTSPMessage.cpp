#include "ZRTSPMessage.h"
#include "ZRTSPSession.h"
#include "ZHeaderParser.h"
///////////////////////////////////////////////////////////////////////////////
//#define	RTSPMESSAGE_DEBUG		1
///////////////////////////////////////////////////////////////////////////////
#define DEFAULT_READ_SDP_TIME		(3000)
///////////////////////////////////////////////////////////////////////////////
ZRTSPMessage::ZRTSPMessage(ZSession* pSession,ZSessionStream* pSessionStream)
:ZSessionMessage(pSession,pSessionStream)
,m_nStatusCode(0)
,m_bConnectClose(FALSE)
,m_nTransport(0)
,m_nTransportParamter(0)
,m_nClientAddr(0)
,m_nClientPortA(0)
,m_nClientPortB(0)
,m_nServerAddr(0)
,m_nServerPortA(0)
,m_nServerPortB(0)
,m_nChannelA(-1)
,m_nChannelB(-1)
{
    memset(m_sURI,0,sizeof(m_sURI));
	memset(m_sHost,0,sizeof(m_sHost));
	memset(m_sUser,0,sizeof(m_sUser));
	memset(m_sPass,0,sizeof(m_sPass));
	memset(m_sChannel,0,sizeof(m_sChannel));
	memset(m_sFilePath,0,sizeof(m_sFilePath));
    memset(m_sTrackPath,0,sizeof(m_sTrackPath));
	memset(m_sContentBase,0,sizeof(m_sContentBase));
}
ZRTSPMessage::~ZRTSPMessage()
{
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTSPMessage::GetTransportType()
{
	return(m_nTransport);
}
DWORD ZRTSPMessage::SetTransportType(DWORD nType)
{
	m_nTransport	= nType;
	return(m_nTransport);
}
DWORD ZRTSPMessage::GetTransportParamter()
{
	return(m_nTransportParamter);
}
DWORD ZRTSPMessage::SetTransportParamter(DWORD nParamter)
{
	m_nTransportParamter	= nParamter;
	return(m_nTransportParamter);
}
///////////////////////////////////////////////////////////////////////////////
int ZRTSPMessage::GetClientAddr()
{
	return(m_nClientAddr);
};
int ZRTSPMessage::GetClientPortA()
{
	return(m_nClientPortA);
};
int ZRTSPMessage::GetClientPortB()
{
	return(m_nClientPortB);
};
int ZRTSPMessage::GetServerAddr()
{
	return(m_nServerAddr);
};
int ZRTSPMessage::GetServerPortA()
{
	return(m_nServerPortA);
};
int ZRTSPMessage::GetServerPortB()
{
	return(m_nServerPortB);
};
int ZRTSPMessage::GetStatusCode()
{
	return(m_nStatusCode);
}
int ZRTSPMessage::GetSessionAddr()
{
	if(m_pSession!=NULL)
	{
		return (((ZRTSPSession*)m_pSession)->GetSessionAddr());
	}
	return 0;
}
int ZRTSPMessage::GetChannelA()
{
	return(m_nChannelA);
}
int ZRTSPMessage::GetChannelB()
{
	return(m_nChannelB);
}
void ZRTSPMessage::SetClientPort(int nPortA,int nPortB)
{
	m_nClientPortA	= nPortA;
	m_nClientPortB	= nPortB;
}
void ZRTSPMessage::SetServerPort(int nPortA,int nPortB)
{
	m_nServerPortA	= nPortA;
	m_nServerPortB	= nPortB;
}
void ZRTSPMessage::SetChannel(int nChannelA,int nChannelB)
{
	m_nChannelA		= nChannelA;
	m_nChannelB		= nChannelB;
}
void ZRTSPMessage::SetServerAddr(int nServerAddr)
{
	m_nServerAddr = nServerAddr;
}
char* ZRTSPMessage::GetURI()
{
    return m_sURI;
}
char* ZRTSPMessage::GetHost()
{
	return m_sHost;
}
///////////////////////////////////////////////////////////////////////////////
char* ZRTSPMessage::GetContentBase()
{
	return m_sContentBase;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTSPMessage::Parse(ZHeaderParser* pHeader)
{
	m_nStatusCode	= 0;
	if(ParseFirstLine(pHeader))
	{
		return ZRTSPMessage::ParseHeaders(pHeader);
	}
	return FALSE;
}
BOOL ZRTSPMessage::ParseHeaders(ZHeaderParser* pHeader)
{
	int		i		= 1;
	int		nPos	= 0;
	int		nHeader	= 0;

	if(pHeader != NULL)
	{
		for(i = 1; i < (int)pHeader->GetLineCount(); i ++)
		{
			nPos	= 0;
			pHeader->SkipSpace(i,&nPos);
			nHeader	= ZRTSPProtocol::GetProtocolHeader(pHeader->GetString(i,&nPos,':'));
			pHeader->SkipChar(i,&nPos,':');
			pHeader->SkipSpace(i,&nPos);
			switch(nHeader)
			{
			case RTSP_HEADERS_CSEQ:
				{
					ParseCSeq(pHeader,i,nPos);
				}
				break;
			case RTSP_HEADERS_TRANSPORT:
				{
					ParseTransport(pHeader,i,nPos);
				}
				break;
			case RTSP_HEADERS_SESSION:
				{
					ParseSession(pHeader,i,nPos);
				}
				break;
			case RTSP_HEADERS_RANGE:
				{
					ParseRange(pHeader,i,nPos);
				}
				break;
			case RTSP_HEADERS_SPEED:
				{
					ParseSpeed(pHeader,i,nPos);
				}
				break;
			case RTSP_HEADERS_CONTENTLENGTH:
				{
					ParseContentLength(pHeader,i,nPos);
				}
				break;
			case RTSP_HEADERS_CONNECTION:
				{
					ParseConnection(pHeader,i,nPos);
				}
				break;
			case RTSP_HEADERS_CONTENTBASE:
				{
					ParseContentBase(pHeader,i,nPos);
				}
				break;
			case RTSP_HEADERS_CONTENTTYPE:
				{
					ParseContentType(pHeader,i,nPos);
				}
				break;
			case RTSP_HEADERS_AUTHORIZATION:
				{
					ParseAuthorization(pHeader,i,nPos);
				}
				break;
			case RTSP_HEADERS_WWWAUTHENTICATE:
				{
					ParseAuthentication(pHeader,i,nPos);
				}
				break;
			}
		}
		return TRUE;
	}
	return FALSE;
}
//CSeq
BOOL ZRTSPMessage::ParseCSeq(ZHeaderParser* pHeader,int nLine,int nPos)
{
	if(pHeader != NULL)
	{
		int	nCseq	= 0;
		nCseq	= atoi(pHeader->GetDigit(nLine,&nPos));
		if(m_pSession != NULL)
		{
			((ZRTSPSession*)m_pSession)->SetSequence(nCseq);
		}
		return TRUE;
	}
	return FALSE;
}
//Transport
BOOL ZRTSPMessage::ParseTransport(ZHeaderParser* pHeader,int nLine,int nPos)
{
	char	sFirstTransport[(DEFAULT_SESSION_BLOCK_SIZE+4)];
	int		nCurrentParser	= 0;
	int		nPreParser		= 0;
	
	if(pHeader != NULL)
	{
		pHeader->SkipSpace(nLine,&nPos);
		//multiple transports,comma separated.use first.
		strcpy(sFirstTransport,pHeader->GetString(nLine,&nPos,','));

		if(strncasecmp(sFirstTransport,"RTP/AVP",7) == 0)
		{
			strcat(sFirstTransport,"\r\n");
			ZHeaderParser	Parser(sFirstTransport,strlen(sFirstTransport));
			nCurrentParser	= 0;
			nPreParser		= 0;
			strcpy(sFirstTransport,Parser.GetString(0,&nCurrentParser,';'));
			nCurrentParser	+= 1;
			do{
				strcat(sFirstTransport,"\r\n\r\n");
				if(strncasecmp(sFirstTransport,"RTP/AVP",7) == 0)
				{
					m_nTransport	= RTSP_PROTOCOL_TRANSPORT_UDP;
					if(strncasecmp(sFirstTransport,"RTP/AVP/TCP",11) == 0)
					{
						m_nTransport	= RTSP_PROTOCOL_TRANSPORT_TCP;
					}
					if(m_pSession != NULL)
					{
						((ZRTSPSession*)m_pSession)->SetTransport(m_nTransport);
					}
				}else if(strncasecmp(sFirstTransport,"unicast",7) == 0)
				{
					m_nTransportParamter	= RTSP_PROTOCOL_TRANSPORT_UNICAST;
					if(m_pSession != NULL)
					{
						((ZRTSPSession*)m_pSession)->SetTransportParamter(RTSP_PROTOCOL_TRANSPORT_UNICAST);
					}
				}else if(strncasecmp(sFirstTransport,"multicast",9) == 0)
				{
					m_nTransportParamter	= RTSP_PROTOCOL_TRANSPORT_MULTICAST;
					if(m_pSession != NULL)
					{
						((ZRTSPSession*)m_pSession)->SetTransportParamter(RTSP_PROTOCOL_TRANSPORT_MULTICAST);
					}
				}else if(strncasecmp(sFirstTransport,"destination",11) == 0)
				{
					ZHeaderParser	TempParser(sFirstTransport,strlen(sFirstTransport));
					int				nTempParser	= 11;
					TempParser.SkipSpace(0,&nTempParser);
					TempParser.SkipChar(0,&nTempParser,'=');
					TempParser.SkipSpace(0,&nTempParser);
					m_nClientAddr	= ZSocket::ConvertAddr(TempParser.GetString(0,&nTempParser,';'));
				}else if(strncasecmp(sFirstTransport,"source",6) == 0)
				{
					ZHeaderParser	TempParser(sFirstTransport,strlen(sFirstTransport));
					int				nTempParser	= 6;
					TempParser.SkipSpace(0,&nTempParser);
					TempParser.SkipChar(0,&nTempParser,'=');
					TempParser.SkipSpace(0,&nTempParser);
					m_nServerAddr	= ZSocket::ConvertAddr(TempParser.GetString(0,&nTempParser,';'));
				}else if(strncasecmp(sFirstTransport,"interleaved",11) == 0)
				{
					ZHeaderParser	TempParser(sFirstTransport,strlen(sFirstTransport));
					int				nTempParser	= 6;
					TempParser.SkipSpace(0,&nTempParser);
					TempParser.SkipChar(0,&nTempParser,'=');
					TempParser.SkipSpace(0,&nTempParser);
					m_nChannelA		= atoi(TempParser.GetDigit(0,&nTempParser));
					TempParser.SkipChar(0,&nTempParser,'-');
					m_nChannelB		= atoi(TempParser.GetDigit(0,&nTempParser));

				}else if(strncasecmp(sFirstTransport,"append",6) == 0)
				{
				}else if(strncasecmp(sFirstTransport,"ttl",3) == 0)
				{
				}else if(strncasecmp(sFirstTransport,"layers",6) == 0)
				{
				}else if(strncasecmp(sFirstTransport,"port",4) == 0)
				{
				}else if(strncasecmp(sFirstTransport,"client_port",11) == 0)
				{
					ZHeaderParser	TempParser(sFirstTransport,strlen(sFirstTransport));
					int				nTempParser	= 11;
					TempParser.SkipSpace(0,&nTempParser);
					TempParser.SkipChar(0,&nTempParser,'=');
					TempParser.SkipSpace(0,&nTempParser);
					m_nClientPortA	= atoi(TempParser.GetDigit(0,&nTempParser));
					TempParser.SkipChar(0,&nTempParser,'-');
					m_nClientPortB	= atoi(TempParser.GetDigit(0,&nTempParser));
				}else if(strncasecmp(sFirstTransport,"server_port",11) == 0)
				{
					ZHeaderParser	TempParser(sFirstTransport,strlen(sFirstTransport));
					int				nTempParser	= 11;
					TempParser.SkipSpace(0,&nTempParser);
					TempParser.SkipChar(0,&nTempParser,'=');
					TempParser.SkipSpace(0,&nTempParser);
					m_nServerPortA	= atoi(TempParser.GetDigit(0,&nTempParser));
					TempParser.SkipChar(0,&nTempParser,'-');
					m_nServerPortB	= atoi(TempParser.GetDigit(0,&nTempParser));
				}else if(strncasecmp(sFirstTransport,"ssrc",4) == 0)
				{
				}else if(strncasecmp(sFirstTransport,"mode",4) == 0)
				{
				}
				nPreParser		= nCurrentParser;
				strcpy(sFirstTransport,Parser.GetString(0,&nCurrentParser,';'));
				nCurrentParser	+= 1;
			}while(Parser.IsLineEnd(0,nPreParser));
#ifdef	RTSPMESSAGE_DEBUG
			MESSAGE_OUT(("ZRTSPMessage::ParseTransport(destination = %s)\r\n",ZSocket::ConvertAddr(m_nClientAddr)));
			MESSAGE_OUT(("ZRTSPMessage::ParseTransport(client_port = %d-%d)\r\n",m_nClientPortA,m_nClientPortB));
			MESSAGE_OUT(("ZRTSPMessage::ParseTransport(source = %s)\r\n",ZSocket::ConvertAddr(m_nServerAddr)));
			MESSAGE_OUT(("ZRTSPMessage::ParseTransport(server_port = %d-%d)\r\n",m_nServerPortA,m_nServerPortB));
#endif	//RTSPMESSAGE_DEBUG
			return TRUE;
		}
	}
	return FALSE;
}
//Session
BOOL ZRTSPMessage::ParseSession(ZHeaderParser* pHeader,int nLine,int nPos)
{
	char*	sValue	= NULL;
	int		nValue	= 0;

	if(pHeader != NULL)
	{
		sValue	= pHeader->GetString(nLine,&nPos,';');
		if(m_pSession != NULL)
		{
			((ZRTSPSession*)m_pSession)->SetSessionID(sValue);
		}
		pHeader->SkipChar(nLine,&nPos,';');
		pHeader->SkipSpace(nLine,&nPos);
		if(pHeader->Compare(nLine,nPos,"timeout") == 0)
		{
			nPos	+= strlen("timeout");
			pHeader->SkipSpace(nLine,&nPos);
			pHeader->SkipChar(nLine,&nPos,'=');
			pHeader->SkipSpace(nLine,&nPos);
			nValue	= pHeader->GetIntegerNumber(nLine,&nPos);
		}
#ifdef	RTSPMESSAGE_DEBUG
			MESSAGE_OUT(("ZRTSPMessage::ParseSession(session = %s)\r\n",sValue));
			MESSAGE_OUT(("ZRTSPMessage::ParseSession(timeout = %d)\r\n",nValue));
#endif	//RTSPMESSAGE_DEBUG
		return TRUE;
	}
	return FALSE;
}
//Range
BOOL ZRTSPMessage::ParseRange(ZHeaderParser* pHeader,int nLine,int nPos)
{
	FLOAT64	nStartTime	= 0;
	FLOAT64	nStopTime	= 0;

	if(pHeader != NULL)
	{
		pHeader->GetString(nLine,&nPos,'=');
		pHeader->SkipSpace(nLine,&nPos);
		nStartTime	= pHeader->GetTime(nLine,&nPos);
		if(pHeader->IsLineEnd(nLine,nPos))
		{
			pHeader->SkipSpace(nLine,&nPos);
			nStopTime	= pHeader->GetTime(nLine,&nPos);
		}
		if(m_pSession != NULL)
		{
			((ZRTSPSession*)m_pSession)->SetTime(nStartTime,nStopTime);
		}
		return TRUE;
	}
	return FALSE;
}
BOOL ZRTSPMessage::ParseSpeed(ZHeaderParser* pHeader,int nLine,int nPos)
{
	FLOAT64	nSpeed	= 0;

	if(pHeader != NULL)
	{
		pHeader->SkipSpace(nLine,&nPos);
		nSpeed	= pHeader->GetFloatNumber(nLine,&nPos);
		return TRUE;
	}
	return FALSE;
}
BOOL ZRTSPMessage::ParseContentLength(ZHeaderParser* pHeader,int nLine,int nPos)
{
	int	nContenLength	= 0;
	int	nError			= 0;
	UINT64 nReadStartTime;

	if(pHeader != NULL)
	{
		pHeader->SkipSpace(nLine,&nPos);
		nContenLength	= pHeader->GetIntegerNumber(nLine,&nPos);
		if(nContenLength > 0)
		{
			if(m_pSession != NULL)
			{
				nReadStartTime = ZOS::milliseconds();
				while(m_pSessionStream->GetBufferLength() < nContenLength)
				{
					if (ZOS::milliseconds() - nReadStartTime > DEFAULT_READ_SDP_TIME)
					{
						LOG_ERROR(("ErrorCode:%s [ZRTSPMessage::ParseContentLength] read SDP Timeout!\r\n", 
							GetErrorCodeString(24019)));
						break;
					}
					nError	= m_pSessionStream->ReadStream();
					if(SESSION_STREAM_PROCESS(nError))//BUG FIXED
					{
						if(m_pSessionStream->GetBufferLength() >= nContenLength)
						{
							break;
						}
						ZOSThread::Sleep(9);
					}
					else
					{
						LOG_ERROR(("ErrorCode:%s [ZRTSPMessage::ParseContentLength] read SDP error!\r\n", 
							GetErrorCodeString(24018)));
						break;
					}
				}
				TMASSERT((m_pSessionStream->GetBufferLength() >= nContenLength));
				if(m_pSessionStream->GetBufferLength() >= nContenLength)
				{
					((ZRTSPSession*)m_pSession)->SetContent(m_pSessionStream->GetBuffer(),nContenLength);
					m_pSessionStream->MoveBuffer(nContenLength);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}
BOOL ZRTSPMessage::ParseConnection(ZHeaderParser* pHeader,int nLine,int nPos)
{
	if(pHeader != NULL)
	{
		pHeader->SkipSpace(nLine,&nPos);
		if(strcasecmp(pHeader->GetBlocks(nLine,&nPos),"Close")==0)
		{
			m_bConnectClose	= TRUE;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL ZRTSPMessage::ParseContentBase(ZHeaderParser* pHeader,int nLine,int nPos)
{
	if (pHeader != NULL)
	{
		pHeader->SkipSpace(nLine,&nPos);
		strncpy(m_sContentBase, pHeader->GetBlocks(nLine,&nPos), MAX_KEY_LABEL);
		return TRUE;
	}
	return FALSE;
}
BOOL ZRTSPMessage::ParseContentType(ZHeaderParser* pHeader,int nLine,int nPos)
{
	if(pHeader != NULL)
	{
		pHeader->SkipSpace(nLine,&nPos);
		if(m_pSession != NULL)
		{
			m_pSession->SetContentType(pHeader->GetString(nLine,&nPos,'\n'));
		}
		return TRUE;
	}
	return FALSE;
}
BOOL ZRTSPMessage::ParseAuthorization(ZHeaderParser* pHeader,int nLine,int nPos)
{
	if(pHeader != NULL)
	{
		pHeader->SkipSpace(nLine,&nPos);
		if(pHeader->CompareNoCase(nLine,nPos,"Basic") == 0)
		{
			return ZRTSPMessage::ParseBasicAuthorization(pHeader,nLine,nPos);
		}else if(pHeader->CompareNoCase(nLine,nPos,"Digest") == 0)
		{
			return ZRTSPMessage::ParseDigestAuthorization(pHeader,nLine,nPos);
		}
		return TRUE;
	}
	return FALSE;
}
BOOL ZRTSPMessage::ParseAuthentication(ZHeaderParser* pHeader,int nLine,int nPos)
{
	if(pHeader != NULL)
	{
		pHeader->SkipSpace(nLine,&nPos);
		if(pHeader->CompareNoCase(nLine,nPos,"Basic") == 0)
		{
			return ZRTSPMessage::ParseBasicAuthentication(pHeader,nLine,nPos);
		}else if(pHeader->CompareNoCase(nLine,nPos,"Digest") == 0)
		{
			return ZRTSPMessage::ParseDigestAuthentication(pHeader,nLine,nPos);
		}
	}
	return FALSE;
}
BOOL ZRTSPMessage::ParseBasicAuthorization(ZHeaderParser* pHeader,int nLine,int nPos)
{
	if(pHeader != NULL)
	{
		if(m_pSession != NULL)
		{
			m_pSession->AuthorizationCheck(ZAuth::AUTH_BASIC,pHeader->GetRemain(nLine,&nPos));
			return TRUE;
		}
	}
	return FALSE;
}
BOOL ZRTSPMessage::ParseDigestAuthorization(ZHeaderParser* pHeader,int nLine,int nPos)
{
	if(pHeader != NULL)
	{
		if(m_pSession != NULL)
		{
			m_pSession->AuthorizationCheck(ZAuth::AUTH_DIGEST,pHeader->GetRemain(nLine,&nPos));
			return TRUE;
		}
	}
	return FALSE;
}
BOOL ZRTSPMessage::ParseBasicAuthentication(ZHeaderParser* pHeader,int nLine,int nPos)
{
	if(pHeader != NULL)
	{
		if(m_pSession != NULL)
		{
			m_pSession->AuthenticationCheck(ZAuth::AUTH_BASIC,pHeader->GetRemain(nLine,&nPos));
			return TRUE;
		}
	}
	return FALSE;
}
BOOL ZRTSPMessage::ParseDigestAuthentication(ZHeaderParser* pHeader,int nLine,int nPos)
{
	if(pHeader != NULL)
	{
		if(m_pSession != NULL)
		{
			m_pSession->AuthenticationCheck(ZAuth::AUTH_DIGEST,pHeader->GetRemain(nLine,&nPos));
			return TRUE;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
void ZRTSPMessage::GetHeaders(char* sMessage,RTSP_PROTOCOL_HEADERS nHeader)
{
	if(sMessage != NULL)
	{
		switch(nHeader)
		{
		case RTSP_HEADERS_ACCEPT:
			strcat(sMessage,"Accept: ");
			strcat(sMessage,ZRTSPProtocol::GetProtocolAccept());
			break;
		case RTSP_HEADERS_USERAGENT:
			strcat(sMessage,"User-Agent: ");
			strcat(sMessage,ZRTSPProtocol::GetProtocolUserAgent());
			break;
		case RTSP_HEADERS_SESSION:
			strcat(sMessage,"Session: ");
			break;
		case RTSP_HEADERS_PUBLIC:
			strcat(sMessage,"Public: ");
			strcat(sMessage,ZRTSPProtocol::GetProtocolPublic());
			break;
		case RTSP_HEADERS_SERVER:
			strcat(sMessage,"Server: ");
			strcat(sMessage,ZRTSPProtocol::GetProtocolServer());
			break;
		default:
			strcat(sMessage,ZRTSPProtocol::GetProtocolHeader(nHeader));
			strcat(sMessage,DEFAULT_HEADER_COLON" ");
			break;
		}
	}
}
void ZRTSPMessage::GetHeaders(char* sMessage,RTSP_PROTOCOL_HEADERS nHeader,const char* sValue)
{
	if(sMessage != NULL)
	{
		if(nHeader >= 0 && nHeader < RTSP_HEADERS_COUNT)
		{
			if(sValue != NULL)
			{
				strcat(sMessage,ZRTSPProtocol::GetProtocolHeader(nHeader));
				strcat(sMessage,DEFAULT_HEADER_COLON" ");
				strcat(sMessage,sValue);
			}
		}
	}
}
void ZRTSPMessage::GetHeaders(char* sMessage,RTSP_PROTOCOL_HEADERS nHeader,int nValue)
{
	char	sValue[64];
	sprintf(sValue,"%d",nValue);
	ZRTSPMessage::GetHeaders(sMessage,nHeader,sValue);
}
void ZRTSPMessage::GetHeaders(char* sMessage,RTSP_PROTOCOL_HEADERS nHeader,float nValue)
{
	char	sValue[64];
	sprintf(sValue,"%.3f",nValue);
	ZRTSPMessage::GetHeaders(sMessage,nHeader,sValue);
}
void ZRTSPMessage::GetHeaders(char* sMessage,RTSP_PROTOCOL_HEADERS nHeader,double nValue)
{
	char	sValue[64];
	sprintf(sValue,"%.3f",nValue);
	ZRTSPMessage::GetHeaders(sMessage,nHeader,sValue);
}
//Transport
void ZRTSPMessage::GetTransport(char* sMessage)
{
	const	char*	sFirstTransport				= ZRTSPProtocol::GetProtocolTransport(m_nTransport);
	const	char*	sFirstTransportParamter		= ZRTSPProtocol::GetProtocolTransportParamter(m_nTransportParamter);
	const	char*	sSSRC						= NULL;
			char	sTemp[256];

	if(sMessage != NULL)
	{
		//Transport
		strcat(sMessage,ZRTSPProtocol::GetProtocolHeader(RTSP_HEADERS_TRANSPORT));
		strcat(sMessage,DEFAULT_HEADER_COLON" ");
		//Transport/Profile/Lower-Transport
		if(sFirstTransport != NULL)
		{
			strcat(sMessage,sFirstTransport);
		}
		//Paramter
		if(sFirstTransportParamter != NULL)
		{
			strcat(sMessage,";");
			strcat(sMessage,sFirstTransportParamter);
		}
		//Destination
		if(m_nClientAddr != 0)
		{
			strcat(sMessage,";");
			strcat(sMessage,"destination=");
			strcat(sMessage,ZSocket::ConvertAddr(m_nClientAddr));
		}
		//Source
		if(m_nServerAddr != 0)
		{
			strcat(sMessage,";");
			strcat(sMessage,"source=");
			strcat(sMessage,ZSocket::ConvertAddr(m_nServerAddr));
		}
		if(m_nTransportParamter == RTSP_PROTOCOL_TRANSPORT_MULTICAST)
		{
			//TTL
		}
		//Client Port
		if(m_nClientPortA != 0)
		{
			strcat(sMessage,";");
			sprintf(sTemp,"client_port=%d-%d",m_nClientPortA,m_nClientPortB);
			strcat(sMessage,sTemp);
		}
		//Server Port
		if(m_nServerPortA != 0)
		{
			strcat(sMessage,";");
			sprintf(sTemp,"server_port=%d-%d",m_nServerPortA,m_nServerPortB);
			strcat(sMessage,sTemp);
		}
		if(m_nTransport == RTSP_PROTOCOL_TRANSPORT_TCP)
		{
			//
			if(m_nChannelA != -1)
			{
				strcat(sMessage,";");
				strcat(sMessage,"interleaved=");
				sprintf(sTemp,"%d",m_nChannelA);
				strcat(sMessage,sTemp);
				if(m_nChannelB != -1)
				{
					strcat(sMessage,"-");
					sprintf(sTemp,"%d",m_nChannelB);
					strcat(sMessage,sTemp);
				}
			}
		}
		//SSRC
		if(sSSRC != NULL)
		{
			strcat(sMessage,";");
			strcat(sMessage,"ssrc=");
			strcat(sMessage,sSSRC);
		}
	}
}

void ZRTSPMessage::GetRange(char *sMessage, double nStart, double nStop)
{
    CHAR sValue[64];

    strcat(sMessage,ZRTSPProtocol::GetProtocolHeader(RTSP_HEADERS_RANGE));
    strcat(sMessage,DEFAULT_HEADER_COLON" ");
    sprintf(sValue,"npt=%.3f",nStart);
    strcat(sMessage,sValue);
    strcat(sMessage,"-");
    if(nStop > 0)
    {
        sprintf(sValue,"%.3f",nStop);
        strcat(sMessage,sValue);
    }
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
VOID ZRTSPMessage::GetAuthorization(CHAR* sResponse)
{
	CHAR	sAuthorizationData[1024];
	UINT	nAuthorizationData	= 1024;

	if(sResponse != NULL)
	{
		if(((ZRTSPSession*)m_pSession) != NULL)
		{
			if(((ZRTSPSession*)m_pSession)->Authorization())
			{
				if(((ZRTSPSession*)m_pSession)->m_SessionAuthorization.GetAuthData(sAuthorizationData,&nAuthorizationData))
				{
					strcat(sResponse,sAuthorizationData);
					strcat(sResponse,DEFAULT_HEADER_EOL);
				}
			}
		}
	}
}
VOID ZRTSPMessage::GetAuthentication(CHAR* sRequest)
{
	CHAR	sAuthenticationData[1024];
	UINT	nAuthenticationData	= 1024;

	if(sRequest != NULL)
	{
		if(((ZRTSPSession*)m_pSession) != NULL)
		{
			((ZRTSPSession*)m_pSession)->m_SessionAuthenticator.SetMethodURL(ZRTSPProtocol::GetProtocolMethod(m_nMethod),((ZRTSPSession*)m_pSession)->GetSessionURI());
			if(((ZRTSPSession*)m_pSession)->Authentication())
			{
				if(((ZRTSPSession*)m_pSession)->m_SessionAuthenticator.GetAuthData(sAuthenticationData,&nAuthenticationData))
				{
					strcat(sRequest,sAuthenticationData);
					strcat(sRequest,DEFAULT_HEADER_EOL);
				}
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
