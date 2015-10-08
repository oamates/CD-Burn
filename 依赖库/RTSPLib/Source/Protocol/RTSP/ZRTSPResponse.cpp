#include "ZRTSPResponse.h"
#include "ZRTSPSession.h"
#include "ZHeaderParser.h"
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_SEND_SERVER		1
///////////////////////////////////////////////////////////////////////////////
ZRTSPResponse::ZRTSPResponse(ZSession* pSession,ZSessionStream* pStream)
:ZRTSPMessage(pSession,pStream)
{
}
ZRTSPResponse::~ZRTSPResponse()
{
}
///////////////////////////////////////////////////////////////////////////////
int ZRTSPResponse::SendResponse(DWORD nMethod,BOOL bClose)
{
	int		nReturn		= 0;
	int		nError		= 0;
	char	sResponse[(DEFAULT_SESSION_BUFFER_SIZE * 2 + 4)];
	int		nResponse	= 0;

	m_bConnectClose		= bClose;
	nReturn				= SESSION_MESSAGE_STATE_INVALID;
	memset(sResponse,0,sizeof(sResponse));
	if(m_pSession != NULL && m_pSessionStream != NULL)
	{
		m_nMethod	= nMethod;
		nReturn		= SESSION_MESSAGE_STATE_COMPLETE;
		//Create Message
		switch(nMethod)
		{
		case RTSP_PROTOCOL_OPTIONS:
			{
				ZRTSPResponse::GetFirstLine(sResponse);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#ifdef	DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_SERVER);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#endif	//DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_PUBLIC);
				strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetEndLine(sResponse);
			}
			break;
		case RTSP_PROTOCOL_DESCRIBE:
			{
				ZRTSPResponse::GetFirstLine(sResponse);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#ifdef	DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_SERVER);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#endif	//DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetEndLine(sResponse);
			}
			break;
		case RTSP_PROTOCOL_ANNOUNCE:
        case RTSP_PROTOCOL_GET_PARAMETER:
        case RTSP_PROTOCOL_SET_PARAMETER:
			{
				ZRTSPResponse::GetFirstLine(sResponse);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#ifdef	DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_SERVER);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#endif	//DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sResponse,DEFAULT_HEADER_EOL);
                ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_SESSION,((ZRTSPSession*)m_pSession)->GetSessionID());
                strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetEndLine(sResponse);
			}
			break;
		case RTSP_PROTOCOL_SETUP:
			{
				ZRTSPResponse::GetFirstLine(sResponse);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#ifdef	DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_SERVER);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#endif	//DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_TIMESTAMP,((ZRTSPSession*)m_pSession)->GetTimeStamp());
				strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_SESSION,((ZRTSPSession*)m_pSession)->GetSessionID());
				strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetTransport(sResponse);
				strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetEndLine(sResponse);
			}
			break;
		case RTSP_PROTOCOL_PLAY:
			{
				ZRTSPResponse::GetFirstLine(sResponse);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#ifdef	DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_SERVER);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#endif	//DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_SESSION,((ZRTSPSession*)m_pSession)->GetSessionID());
				strcat(sResponse,DEFAULT_HEADER_EOL);
                ZRTSPResponse::GetRange(sResponse,((ZRTSPSession*)m_pSession)->GetStartTime(),((ZRTSPSession*)m_pSession)->GetStopTime());
                strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetEndLine(sResponse);
			}
			break;
		case RTSP_PROTOCOL_PAUSE:
			{
				ZRTSPResponse::GetFirstLine(sResponse);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#ifdef	DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_SERVER);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#endif	//DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_SESSION,((ZRTSPSession*)m_pSession)->GetSessionID());
				strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetEndLine(sResponse);
			}
			break;
		case RTSP_PROTOCOL_TEARDOWN:
			{
				ZRTSPResponse::GetFirstLine(sResponse);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#ifdef	DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_SERVER);
				strcat(sResponse,DEFAULT_HEADER_EOL);
#endif	//DEFAULT_SEND_SERVER
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetHeaders(sResponse,RTSP_HEADERS_SESSION,((ZRTSPSession*)m_pSession)->GetSessionID());
				strcat(sResponse,DEFAULT_HEADER_EOL);
				ZRTSPResponse::GetEndLine(sResponse);
			}
			break;
		case RTSP_PROTOCOL_REDIRECT:
		case RTSP_PROTOCOL_RECORD:
		default:
			{
			}
			break;
		}
		nResponse			= strlen(sResponse);
		if(nResponse > 0)
		{
			if(((ZRTSPSession*)m_pSession)->GetContentLength() > 0 && ((ZRTSPSession*)m_pSession)->GetContentLength() < DEFAULT_SESSION_BUFFER_SIZE)
			{
				memcpy(&sResponse[nResponse],((ZRTSPSession*)m_pSession)->GetContent(),((ZRTSPSession*)m_pSession)->GetContentLength());
				nResponse			+= ((ZRTSPSession*)m_pSession)->GetContentLength();
				((ZRTSPSession*)m_pSession)->SetContent(NULL,0);
			}
            if (
                (m_nMethod != RTSP_PROTOCOL_ANNOUNCE)
                && (m_nMethod != RTSP_PROTOCOL_GET_PARAMETER)
                && (m_nMethod != RTSP_PROTOCOL_SET_PARAMETER)
                )
            {
                DEBUG_OUT(("[ZRTSPResponse::SendResponse] %s\r\n", sResponse));
            }
			nError	= m_pSessionStream->SendStream(sResponse,nResponse);
			if(nError != SESSION_STREAM_STATE_COMPLETE)
			{
				nReturn		= SESSION_MESSAGE_STATE_ERROR;
			}
		}
	}
	return nReturn;
}
///////////////////////////////////////////////////////////////////////////////
int ZRTSPResponse::ReadResponse()
{
	int		nReturn		= 0;
	int		nError		= 0;
	char*	pBuffer		= NULL;
	int		nBuffer		= 0;
	char*	pResponse	= NULL;
	int		nResponse	= 0;
	int		nProcess	= 0;

	m_nStatusCode	= 0;
	m_bConnectClose	= FALSE;
	nReturn			= SESSION_MESSAGE_STATE_INVALID;
	if(m_pSession != NULL && m_pSessionStream != NULL)
	{
		do{
			nReturn	= SESSION_MESSAGE_STATE_READ;
			nError	= m_pSessionStream->ReadStream();
			if(SESSION_STREAM_PROCESS(nError))
			{
				pBuffer		= m_pSessionStream->GetBuffer();
				nBuffer		= m_pSessionStream->GetBufferLength();
				pResponse	= strstr(pBuffer,"RTSP");
				if(pResponse != NULL)
				{
					DEBUG_OUT(("[ZRTSPResponse::ReadResponse] %s\r\n", pResponse));
					nProcess	= (pResponse-pBuffer);
					nResponse	= (nBuffer-nProcess);
					ZHeaderParser	parser(pResponse,nResponse);
					if(parser.GetHeaderSize() > 0)
					{
						nProcess	+= parser.GetHeaderSize();
						m_pSessionStream->MoveBuffer(nProcess);
						if(ZRTSPResponse::Parse(&parser))
						{
							nReturn	= SESSION_MESSAGE_STATE_COMPLETE;
						}else{
							nReturn	= SESSION_MESSAGE_STATE_ERROR;
							MESSAGE_OUT(("ZRTSPResponse::ReadRTSPResponse(Parse)\r\n"));
						}
						break;
					}else{
						nReturn	= SESSION_MESSAGE_STATE_ERROR;
						MESSAGE_OUT(("ZRTSPResponse::ReadRTSPResponse(Header)\r\n"));
					}
				}else{
					m_pSessionStream->MoveBuffer(nBuffer);
					nReturn	= SESSION_MESSAGE_STATE_ERROR;
					MESSAGE_OUT(("ZRTSPResponse::ReadRTSPResponse(RTSP?)\r\n"));
				}
			}else{
				nReturn	= SESSION_STREAM_MESSAGE_STATE(nError);
				MESSAGE_OUT(("ZRTSPResponse::ReadRTSPResponse(%d)\r\n",nError));
			}
		}while(nReturn == SESSION_MESSAGE_STATE_READ);
	}
	return nReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTSPResponse::ParseFirstLine(ZHeaderParser* pHeader)
{
	int		nPos	= 0;

	if(pHeader != NULL)
	{
		pHeader->SkipSpace(0,&nPos);
		if(ZRTSPProtocol::IsProtocol(pHeader->GetBlocks(0,&nPos)))
		{
			pHeader->SkipSpace(0,&nPos);
			m_nStatusCode	= atoi(pHeader->GetDigit(0,&nPos));

			return TRUE;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
void ZRTSPResponse::GetFirstLine(char* sResponse)
{
	strcat(sResponse,ZRTSPProtocol::GetProtocolName());
	strcat(sResponse," ");
	strcat(sResponse,ZRTSPProtocol::GetProtocolStatusCode(((ZRTSPSession*)m_pSession)->GetRTSPSessionStatus()));
	strcat(sResponse," ");
	strcat(sResponse,ZRTSPProtocol::GetProtocolStatusLabel(((ZRTSPSession*)m_pSession)->GetRTSPSessionStatus()));
}
void ZRTSPResponse::GetEndLine(char* sResponse)
{
	if(sResponse != NULL)
	{
		if(((ZRTSPSession*)m_pSession)->GetContentLength() > 0)
		{
			ZRTSPMessage::GetHeaders(sResponse,RTSP_HEADERS_CONTENTLENGTH,((ZRTSPSession*)m_pSession)->GetContentLength());
			strcat(sResponse,DEFAULT_HEADER_EOL);
		}
		if(m_bConnectClose)
		{

			strcat(sResponse,"Connection: Close");
			strcat(sResponse,DEFAULT_HEADER_EOL);
		}
		strcat(sResponse,DEFAULT_HEADER_EOL);
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
