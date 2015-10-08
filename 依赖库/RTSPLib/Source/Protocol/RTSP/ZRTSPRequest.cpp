#include "ZRTSPRequest.h"
#include "ZRTSPSession.h"
#include "ZHeaderParser.h"
///////////////////////////////////////////////////////////////////////////////
#if	(defined(_WIN32_)||defined(_MINGW_))
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif	//!(defined(_WIN32_)||defined(_MINGW_))
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZRTSPRequest::ZRTSPRequest(ZSession* pSession,ZSessionStream* pStream)
:ZRTSPMessage(pSession,pStream)
{
}
ZRTSPRequest::~ZRTSPRequest()
{
}
///////////////////////////////////////////////////////////////////////////////
int ZRTSPRequest::SendRequest(DWORD nMethod)
{
	int		nReturn		= 0;
	int		nError		= 0;
	char	sRequest[(DEFAULT_SESSION_BUFFER_SIZE + 4)];

	nReturn	= SESSION_MESSAGE_STATE_INVALID;
	memset(sRequest,0,sizeof(sRequest));
	if(m_pSessionStream != NULL)
	{
		m_nMethod	= nMethod;
		nReturn		= SESSION_MESSAGE_STATE_COMPLETE;
		//Create Message
		switch(nMethod)
		{
		case RTSP_PROTOCOL_OPTIONS:
			{
				((ZRTSPSession*)m_pSession)->AddSequence();
				ZRTSPRequest::GetFirstLine(sRequest);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_USERAGENT);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				if (((ZRTSPSession*)m_pSession)->GetSessionID() != NULL)
				{
					ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_SESSION, ((ZRTSPSession*)m_pSession)->GetSessionID());
					strcat(sRequest,DEFAULT_HEADER_EOL);
				}
				ZRTSPRequest::GetAuthentication(sRequest);
				ZRTSPRequest::GetEndLine(sRequest);
			}
			break;
		case RTSP_PROTOCOL_DESCRIBE:
			{
				((ZRTSPSession*)m_pSession)->AddSequence();
				ZRTSPRequest::GetFirstLine(sRequest);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_ACCEPT);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_USERAGENT);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetAuthentication(sRequest);
				ZRTSPRequest::GetEndLine(sRequest);
			}
			break;
		case RTSP_PROTOCOL_ANNOUNCE:
			{
				((ZRTSPSession*)m_pSession)->AddSequence();
				ZRTSPRequest::GetFirstLine(sRequest);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_ACCEPT);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_USERAGENT);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetAuthentication(sRequest);
				ZRTSPRequest::GetEndLine(sRequest);
			}
			break;
		case RTSP_PROTOCOL_SETUP:
			{
				((ZRTSPSession*)m_pSession)->AddSequence();
				ZRTSPRequest::GetFirstLine(sRequest);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetTransport(sRequest);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_USERAGENT);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				if (((ZRTSPSession*)m_pSession)->GetSessionID() != NULL)
				{
					ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_SESSION, ((ZRTSPSession*)m_pSession)->GetSessionID());
					strcat(sRequest,DEFAULT_HEADER_EOL);
				}
				ZRTSPRequest::GetAuthentication(sRequest);
				ZRTSPRequest::GetEndLine(sRequest);
			}
			break;
		case RTSP_PROTOCOL_PLAY:
			{
				((ZRTSPSession*)m_pSession)->AddSequence();
				ZRTSPRequest::GetFirstLine(sRequest);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_SESSION,((ZRTSPSession*)m_pSession)->GetSessionID());
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_USERAGENT);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				strcat(sRequest, "Range: npt=0.000-");
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetAuthentication(sRequest);
				ZRTSPRequest::GetEndLine(sRequest);
			}
			break;
		case RTSP_PROTOCOL_PAUSE:
			{
				((ZRTSPSession*)m_pSession)->AddSequence();
				ZRTSPRequest::GetFirstLine(sRequest);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_SESSION,((ZRTSPSession*)m_pSession)->GetSessionID());
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_USERAGENT);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetAuthentication(sRequest);
				ZRTSPRequest::GetEndLine(sRequest);
			}
			break;
		case RTSP_PROTOCOL_TEARDOWN:
			{
				((ZRTSPSession*)m_pSession)->AddSequence();
				ZRTSPRequest::GetFirstLine(sRequest);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_SESSION,((ZRTSPSession*)m_pSession)->GetSessionID());
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_USERAGENT);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetAuthentication(sRequest);
				ZRTSPRequest::GetEndLine(sRequest);
			}
			break;
		case RTSP_PROTOCOL_GET_PARAMETER:
			{
				((ZRTSPSession*)m_pSession)->AddSequence();
				ZRTSPRequest::GetFirstLine(sRequest);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_CSEQ,((ZRTSPSession*)m_pSession)->GetSequence());
				strcat(sRequest,DEFAULT_HEADER_EOL);
				ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_USERAGENT);
				strcat(sRequest,DEFAULT_HEADER_EOL);
				if (((ZRTSPSession*)m_pSession)->GetSessionID() != NULL)
				{
					ZRTSPRequest::GetHeaders(sRequest,RTSP_HEADERS_SESSION, ((ZRTSPSession*)m_pSession)->GetSessionID());
					strcat(sRequest,DEFAULT_HEADER_EOL);
				}
				ZRTSPRequest::GetAuthentication(sRequest);
				ZRTSPRequest::GetEndLine(sRequest);
			}
			break;
		case RTSP_PROTOCOL_SET_PARAMETER:
		case RTSP_PROTOCOL_REDIRECT:
		case RTSP_PROTOCOL_RECORD:
		default:
			{
			}
			break;
		}
		//Send Message
		if(strlen(sRequest) > 0)
		{
			LOG_DEBUG(("[ZRTSPRequest::SendRequest] %s\r\n", sRequest));
			nError	= m_pSessionStream->SendStream(sRequest,strlen(sRequest));
			if(nError != SESSION_STREAM_STATE_COMPLETE)
			{
				nReturn	= SESSION_MESSAGE_STATE_ERROR;
			}
		}
	}
	return nReturn;
}
int ZRTSPRequest::ReadRequest()
{
	int		nReturn		= 0;
	int		nError		= 0;
	char*	pBuffer		= NULL;
	int		nBuffer		= 0;
	int		nDataPacket	= 0;

	nReturn	= SESSION_MESSAGE_STATE_INVALID;
	if(m_pSessionStream != NULL)
	{
		nReturn	= SESSION_MESSAGE_STATE_READ;
		nError	= m_pSessionStream->ReadStream();
		if(SESSION_STREAM_PROCESS(nError))
		{
			pBuffer	= m_pSessionStream->GetBuffer();
			nBuffer	= m_pSessionStream->GetBufferLength();
			if(*pBuffer == '$')
			{
				if(nBuffer > 4)
				{
					nDataPacket	= GETUINT16(&pBuffer[2]);
					nDataPacket	= (ntohs(nDataPacket)+4);
					if(nDataPacket <= nBuffer)
					{
						m_pSessionStream->MoveBuffer(nDataPacket);
						m_nMethod	= RTSP_PROTOCOL_DATA;
						nReturn		= SESSION_MESSAGE_STATE_COMPLETE;
					}
				}
			}else{
				if(nBuffer > 0)
				{
					ZHeaderParser	parser(pBuffer,nBuffer);
					if(parser.GetHeaderSize() > 0)
					{
						if(ZRTSPRequest::Parse(&parser))
						{
							nReturn	= SESSION_MESSAGE_STATE_COMPLETE;
						}else{
							nReturn	= SESSION_MESSAGE_STATE_ERROR;
						}
                        if (
                            (m_nMethod != RTSP_PROTOCOL_GET_PARAMETER)
                            && (m_nMethod != RTSP_PROTOCOL_SET_PARAMETER)
                            && (m_nMethod != RTSP_PROTOCOL_ANNOUNCE)
                            )
                        {
                            LOG_DEBUG(("[ZRTSPRequest::ReadRequest] %s\r\n", pBuffer));
                        }
                        m_pSessionStream->MoveBuffer(parser.GetHeaderSize());
					}else{
						nReturn	= SESSION_MESSAGE_STATE_ERROR;
					}
				}
			}
		}else{
			nReturn	= SESSION_STREAM_MESSAGE_STATE(nError);
		}
	}
	return nReturn;;
}
int ZRTSPRequest::CleanRequest()
{
	int		nReturn		= 0;
	int		nError		= 0;

	nReturn	= SESSION_MESSAGE_STATE_INVALID;

	if(m_pSessionStream != NULL)
	{
		nReturn	= SESSION_MESSAGE_STATE_COMPLETE;
		nError	= m_pSessionStream->CleanStream();
		if(nError != SESSION_STREAM_STATE_COMPLETE)
		{
			 nReturn	= SESSION_MESSAGE_STATE_ERROR;
		}
	}
	return nReturn;;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTSPRequest::ParseFirstLine(ZHeaderParser* pHeader)
{
	BOOL	bReturn	= FALSE;
	char*	sValue	= NULL;
	int		nPos	= 0;

	if(pHeader != NULL)
	{
		do{
			pHeader->SkipSpace(0,&nPos);
			sValue		= pHeader->GetWords(0,&nPos);
			if(sValue[0] == '$')
			{
				m_nMethod	= RTSP_PROTOCOL_DATA;
			}else{
				m_nMethod	= (RTSP_PROTOCOL_METHOD)ZRTSPProtocol::GetProtocolMethod(sValue);
				if(m_nMethod == PROTOCOL_METHOD_INVALID)
				{
					((ZRTSPSession*)m_pSession)->SetRTSPSessionStatus(RTSP_STATUS_CLIENTBADREQUEST);
					//DEBUG_OUT(("ZRTSPRequest::ParseFirstLine ERROR(Method=%s)\r\n",pHeader->GetLine(0)));
					break;
				}
				pHeader->SkipSpace(0,&nPos);
				if(!ZRTSPRequest::ParseURI(pHeader,&nPos))
				{
					break;
				}
				pHeader->SkipSpace(0,&nPos);
				if(!ZRTSPProtocol::IsProtocol(pHeader->GetBlocks(0,&nPos)))
				{
					((ZRTSPSession*)m_pSession)->SetRTSPSessionStatus(RTSP_STATUS_CLIENTBADREQUEST);
					break;
				}
				if(pHeader->IsLineEnd(0,nPos))
				{
					((ZRTSPSession*)m_pSession)->SetRTSPSessionStatus(RTSP_STATUS_CLIENTBADREQUEST);
					break;
				}
			}
			bReturn	= TRUE;
		}while(FALSE);
	}
	return bReturn;
}
void ZRTSPRequest::GetFirstLine(char* sRequest)
{
	if(sRequest != NULL && m_pSession != NULL && ((ZRTSPSession*)m_pSession)->GetSessionURI() != NULL)
	{
		if(m_nMethod == RTSP_PROTOCOL_SETUP)
		{
			strcat(sRequest,ZRTSPProtocol::GetProtocolMethod(m_nMethod));
			strcat(sRequest," ");
			strcat(sRequest,((ZRTSPSession*)m_pSession)->GetTrackPath());
			strcat(sRequest," ");
			strcat(sRequest,ZRTSPProtocol::GetProtocolName());
		}else{
			strcat(sRequest,ZRTSPProtocol::GetProtocolMethod(m_nMethod));
			strcat(sRequest," ");
			strcat(sRequest,((ZRTSPSession*)m_pSession)->GetSessionURI());
			strcat(sRequest," ");
			strcat(sRequest,ZRTSPProtocol::GetProtocolName());
		}
	}else{
		TMASSERT((m_pSession!=NULL));
	}
}
void ZRTSPRequest::GetEndLine(char* sRequest)
{
	if(sRequest != NULL)
	{
		strcat(sRequest,DEFAULT_HEADER_EOL);
	}
}
BOOL ZRTSPRequest::ParseURI(ZHeaderParser* pHeader,int* nPos)
{
	char	sURI[(DEFAULT_SESSION_BLOCK_SIZE + 4)];
	char*	pHost	= NULL;
	char*	pUser	= NULL;
	char*	pPass	= NULL;
	char*	pPath	= NULL;
	char*	pTemp	= NULL;

    memset(m_sURI,0,sizeof(m_sURI));
	memset(m_sHost,0,sizeof(m_sHost));
	memset(m_sUser,0,sizeof(m_sUser));
	memset(m_sPass,0,sizeof(m_sPass));
	memset(m_sChannel,0,sizeof(m_sChannel));
	memset(m_sFilePath,0,sizeof(m_sFilePath));
    memset(m_sTrackPath,0,sizeof(m_sTrackPath));

	if(pHeader != NULL)
	{
		strncpy(sURI,pHeader->GetBlocks(0,nPos),DEFAULT_SESSION_BLOCK_SIZE);
        strncpy(m_sURI, sURI, MAX_URI_PATH);
		//*
		if(sURI[0] == '*' && strlen(sURI) == 1)
		{
			return TRUE;
		}
		//rtsp://
		ZOS::DecodeURL(sURI,NULL,&pHost,NULL,&pUser,&pPass,&pPath);
		if(pHost != NULL)
		{
			strncpy(m_sHost,pHost,MAX_FILE_NAME);
		}
		if(pUser != NULL)
		{
			strncpy(m_sUser,pUser,MAX_KEY_LABEL);
		}
		if(pPass != NULL)
		{
			strncpy(m_sPass,pPass,MAX_KEY_LABEL);
		}
		if(pPath != NULL)
		{
			strncpy(m_sChannel,pPath,MAX_URI_PATH);
			pTemp	= m_sChannel;
			// use for compatibility with sms1.0 multicast.  
			if (strncmp(m_sChannel, "multicast", 9) != 0)
			{
				while(*pTemp != '\0')
				{
					if(*pTemp == '\\' || *pTemp == '/')
					{
						*pTemp	= '\0';
						strncpy(m_sFilePath,&pTemp[1],MAX_FILE_PATH);
                        strncpy(m_sTrackPath,&pTemp[1],MAX_FILE_PATH);
						break;
					}
					pTemp	++;
				}
                pTemp	= m_sFilePath;
                while(*pTemp != '\0')
                {
                    if(*pTemp == '\\' || *pTemp == '/')
                    {
                        strncpy(m_sTrackPath,&pTemp[1],MAX_FILE_PATH);
                    }
                    pTemp	++;
                }
			}
		}
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
