#include "ZRTSPSession.h"
#include "ZRTPSession.h"
#include "ZSessionDescription.h"
#include "ZTCPSocket.h"
#include "ZRTPStream.h"
#include "RTSPClientListener.h"
#include "RTSPServerListener.h"
#include "SDPUtil.h"
///////////////////////////////////////////////////////////////////////////////
//#define	ZRTSPSESSION_DEBUG	1
//#define	DEFAULT_RTSP_OUTPUT	1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZRTSPSession::ZRTSPSession(SESSION_SUBTYPE eSubType)
:ZSession(SESSION_TYPE_RTSP,eSubType)
,m_SessionStream()
,m_Request(NULL,&m_SessionStream)
,m_Response(NULL,&m_SessionStream)
,m_RTPSession(eSubType)
,m_nTransport(0)
,m_nTransportParamter(0)
,m_eProtocolTransportType(RTSP_PROTOCOL_TRANSPORT_UDP)
,m_pRTSPClientListener(NULL)
,m_pRTSPServerListener(NULL)
,m_bDoDescribe(FALSE)
,m_aSessionDescription()
,m_nStreamSetup(0)
,m_nPlayStartTime(0)
,m_nSessionStopTime(0)
,m_nLastRTCPTime(0)
,m_SessionStatus(RTSP_STATUS_OK)
,m_nSessionTimeoutMilliSecond(DEFAULT_SESSION_TIMEOUT)
,m_RtpInfo(NULL)
,m_pRangeString(NULL)
,m_bFirstCommand(TRUE)
{
	m_Request.SetSession(this);
	m_Response.SetSession(this);
	m_RTPSession.SetRTSPSession(this);
	m_RTPSession.SetRTSPSessionStream(&m_SessionStream);
	m_nStreamType[0] = 0;
	m_nStreamType[1] = 0;
#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("RTSP New Session\r\n"));
#endif	//ZRTSPSESSION_DEBUG
}
ZRTSPSession::~ZRTSPSession()
{
	ZRTSPSession::Close();
#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("RTSP Release Session\r\n"));
#endif	//ZRTSPSESSION_DEBUG

	SAFE_DELETE_ARRAY(m_pRangeString);
	SAFE_DELETE_ARRAY(m_RtpInfo);
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTSPSession::Create()
{
	ZSession::Create();

	SetRTSPSessionStatus(RTSP_STATUS_OK);

	if(m_nSessionSubType == SESSION_TYPE_SERVER)
	{
		SetSessionState(SESSION_STATE_READ_REQUEST);
	}else if(m_nSessionSubType == SESSION_TYPE_CLIENT)
	{
		m_Request.SetTransportType(m_eProtocolTransportType);
		m_Request.SetTransportParamter(RTSP_PROTOCOL_TRANSPORT_UNICAST);
		SetSessionState(SESSION_STATE_SEND_RESPONSE);
		AddEvent(ZTask::TASK_START_EVENT);
	}
	m_RTPSession.Create();
    m_nSessionStopTime = 0;

#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::Create\r\n"));
#endif	//ZRTSPSESSION_DEBUG
	return TRUE;
}
BOOL ZRTSPSession::Close()
{
    //LOG_DEBUG(("[ZRTSPSession::Close]\r\n"));
    //LOG_DEBUG(("[ZRTSPSession::Close] %s\r\n", m_UsedTaskThread->GetObjectName()));
	if(m_nSessionSubType == SESSION_TYPE_SERVER)
	{
        //
	}
    else if(m_nSessionSubType == SESSION_TYPE_CLIENT)
	{
		m_Request.SendRequest(RTSP_PROTOCOL_TEARDOWN);
		m_SessionStream.RequestEvent();
	}
	ZSession::Close();

	m_RTPSession.Close();

    m_bDoDescribe = FALSE;

	m_nStreamSetup	= 0;

	m_SessionStream.RemoveEvent();
	m_SessionStream.AttachStream(NULL);

    m_nSessionStopTime = ZOS::milliseconds();
#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::Close\r\n"));
#endif	//ZRTSPSESSION_DEBUG
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
VOID ZRTSPSession::SetRTSPClientListener(RTSPClientListener *pListener)
{
    m_pRTSPClientListener = pListener;
}
VOID ZRTSPSession::SetRTSPServerListener(RTSPServerListener *pListener)
{
    m_pRTSPServerListener = pListener;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTSPSession::SetSessionStream(ZBaseStream* pStream)
{
	if(pStream != NULL)
	{
		m_SessionStream.AttachStream(pStream);
	}
#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::SetSessionStream\r\n"));
#endif	//ZRTSPSESSION_DEBUG
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
UINT ZRTSPSession::GetDataStreamCount()
{
    return m_RTPSession.GetDataStreamCount();
}
ZBaseStream* ZRTSPSession::GetDataStream(int i)
{
    return m_RTPSession.GetDataStream(i);
}
ZDataPin* ZRTSPSession::GetDataPin(int i)
{
    return m_RTPSession.GetDataPin(i);
}

char* ZRTSPSession::GetTrackPath()
{
	static char sTrackPath[MAX_FILE_NAME+4] = {0};

	sprintf(sTrackPath,"%s",GetSessionURI());
	if(GetContentType() != NULL)
	{
		if(strcasecmp(GetContentType(),"application/sdp") == 0)
		{
			if(m_nStreamSetup < m_aSessionDescription.GetStreamCount())
			{
				if(m_aSessionDescription.GetStream(m_nStreamSetup)->m_nMediaPayloadType != 0)
				{
					if(m_aSessionDescription.GetStream(m_nStreamSetup)->m_nMediaTrackID == 0xFFFFFFFF)
					{
						sprintf(sTrackPath,"%s",m_aSessionDescription.GetStream(m_nStreamSetup)->m_sMediaControlName);
					}
					else
					{
						char *sBase = m_Response.GetContentBase();
						char *sControlName = m_aSessionDescription.GetStream(m_nStreamSetup)->m_sMediaControlName;
						if (strlen(sBase)<=0)
						{
							//content base is not exist, use request url
							sBase = GetSessionURI();
						}

						if (strlen(sControlName)>0 
							&& sControlName[0] == '/')
						{
							sControlName++;
						}

						if (sBase[strlen(sBase)-1] == '/')
						{
							sprintf(sTrackPath,"%s%s",sBase,sControlName);
						}
						else
						{
							sprintf(sTrackPath,"%s/%s",sBase,sControlName);
						}
					}
				}
			}
		}
	}
	return sTrackPath;
}
///////////////////////////////////////////////////////////////////////////////

DWORD ZRTSPSession::GetTransport()
{
	return(m_nTransport);
}
DWORD ZRTSPSession::SetTransport(DWORD nTransport)
{
	m_nTransport	= nTransport;
	m_Request.SetTransportType(nTransport);
	return(m_nTransport);
}
DWORD ZRTSPSession::GetTransportParamter()
{
	return(m_nTransportParamter);
}
DWORD ZRTSPSession::SetTransportParamter(DWORD nTransportParamter)
{
	m_nTransportParamter	= nTransportParamter;
	return(m_nTransport);
}
RTSP_PROTOCOL_TRANSPORT	ZRTSPSession::GetProtocolTransportType()
{
	return m_eProtocolTransportType;
}
RTSP_PROTOCOL_TRANSPORT ZRTSPSession::SetProtocolTransportType(RTSP_PROTOCOL_TRANSPORT eProtocolTransportType)
{
	m_eProtocolTransportType = eProtocolTransportType;
	m_Request.SetTransportType(eProtocolTransportType);
	return m_eProtocolTransportType;
}
void ZRTSPSession::SetSessionTimeoutMilliSecond(UINT64 nTimeoutMilliSecond)
{
    m_nSessionTimeoutMilliSecond = nTimeoutMilliSecond;
    m_RTPSession.SetSessionTimeoutMilliSecond(nTimeoutMilliSecond);
    AddEvent(ZTask::TASK_UPDATE_EVENT);
}
int ZRTSPSession::SetEachTimeSendCount(int nSendCount)
{
    m_RTPSession.SetTaskCount(nSendCount);
    return 0;
}
///////////////////////////////////////////////////////////////////////////////
int ZRTSPSession::GetSessionAddr()
{
	UINT	nRemoteAddr		= 0;
	if(m_SessionStream.GetStream() != NULL)
	{
		((ZTCPSocket*)m_SessionStream.GetStream())->GetRemoteAddr(&nRemoteAddr,NULL);
	}
	return nRemoteAddr;
}
int ZRTSPSession::GetSessionPort()
{
    UINT    nRemotePort     = 0;
    if (m_SessionStream.GetStream() != NULL)
    {
        ((ZTCPSocket*)m_SessionStream.GetStream())->GetRemoteAddr(NULL,&nRemotePort);
    }

    return nRemotePort;
}
UINT ZRTSPSession::GetSessionDuration()
{
    if (m_nSessionStopTime != 0)
    {
        return (UINT)((m_nSessionStopTime-m_nPlayStartTime)/1000);
    }
    else
    {
        return (UINT)((ZOS::milliseconds()-m_nPlayStartTime)/1000);
    }
}
UINT64 ZRTSPSession::GetSessionTotalPacketCount()
{
    return m_RTPSession.GetSessionTotalPacketCount();
}
UINT64 ZRTSPSession::GetSessionLostPacketCount()
{
    return m_RTPSession.GetSessionLostPacketCount();
}
DWORD ZRTSPSession::GetMediaDstAddr()
{
	return m_aSessionDescription.GetMediaDstAddr();
}
DWORD ZRTSPSession::GetMediaDstPort(DWORD index)
{
	return m_aSessionDescription.GetMediaDstPort(index);
}
BOOL ZRTSPSession::OnDataPinClose()
{
	if (GetSessionSubType() == SESSION_TYPE_SERVER)
	{
		SetSessionState(SESSION_STATE_ERROR);
		DEBUG_OUT(("[ZRTSPSession::OnDataPinClose] upper pin(channel or source) is close, disconnect with client!\r\n"));
	}
	return TRUE;
}
RTSP_PROTOCOL_STATUS ZRTSPSession::GetRTSPSessionStatus()
{
	return(m_SessionStatus);
}
RTSP_PROTOCOL_STATUS ZRTSPSession::SetRTSPSessionStatus(RTSP_PROTOCOL_STATUS nStatus)
{
	m_SessionStatus	= nStatus;
	return(m_SessionStatus);
}
///////////////////////////////////////////////////////////////////////////////
int ZRTSPSession::ServerRun(UINT nEvent)
{
	int		nTaskTime	= 0;
	BOOL	bBreak		= FALSE;
	int		nError		= 0;

	if(nEvent&TASK_READ_EVENT)
	{
		while(IsLiveSession())
		{
			switch(GetSessionState())
			{
			case SESSION_STATE_READ_REQUEST:
				{
					if(nEvent&TASK_READ_EVENT)
					{
#ifdef	DEFAULT_RTSP_OUTPUT
							LOG_DEBUG(("RTSP SESSION_STATE_READ_REQUEST START\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
						nError	= m_Request.ReadRequest();
						if(nError == SESSION_MESSAGE_STATE_COMPLETE)
						{
							ZTimeoutTask::RefreshTimeout();
							ZRTSPSession::ProcessRequest(&m_Request);
#ifdef	DEFAULT_RTSP_OUTPUT
							LOG_DEBUG(("RTSP SESSION_STATE_READ_REQUEST OK\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT

						}else if(nError == SESSION_MESSAGE_STATE_READ)
						{
							m_SessionStream.RequestEvent();
							bBreak		= TRUE;
#ifdef	DEFAULT_RTSP_OUTPUT
							LOG_DEBUG(("RTSP SESSION_STATE_READ_REQUEST ...\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
						}else if(nError == SESSION_MESSAGE_STATE_EOF)
						{
							//close
							SetSessionState(SESSION_STATE_ERROR);
							bBreak		= TRUE;
#ifdef	DEFAULT_RTSP_OUTPUT
							LOG_DEBUG(("RTSP SESSION_STATE_READ_REQUEST ERROR(PEER CLOSE)\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
						}else{
							//error
							SetSessionState(SESSION_STATE_ERROR);
							bBreak		= TRUE;
#ifdef	DEFAULT_RTSP_OUTPUT
							LOG_DEBUG(("RTSP SESSION_STATE_READ_REQUEST ERROR(UNKNOWN)\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
						}
					}else{
						bBreak	= TRUE;
					}
				}
				break;
			case SESSION_STATE_READ_OPTIONS:
				{
					nError	= m_Response.SendResponse((RTSP_PROTOCOL_METHOD)m_Request.GetMethod());
					if(nError == SESSION_MESSAGE_STATE_COMPLETE)
					{
						SetSessionState(SESSION_STATE_READ_REQUEST);
					}
					else
					{
						LOG_ERROR(("ErrorCode:%s [ZRTSPSession::ServerRun] ERROR(OPTION FAILED)\r\n", GetErrorCodeString(24014)));
						SetSessionState(SESSION_STATE_ERROR);
						bBreak		= TRUE;
					}
#ifdef	DEFAULT_RTSP_OUTPUT
					LOG_DEBUG(("RTSP SESSION_STATE_READ_OPTIONS OK\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
					//bBreak	= TRUE;
				}
				break;
			case SESSION_STATE_READ_DESCRIBE:
				{
					nError	= m_Response.SendResponse((RTSP_PROTOCOL_METHOD)m_Request.GetMethod());
					if(nError == SESSION_MESSAGE_STATE_COMPLETE)
					{
                        if (GetRTSPSessionStatus() == RTSP_STATUS_CLIENTNOTFOUND)
                        {
                            LOG_DEBUG(("[ZRTSPSession::ServerRun] RTSP_STATUS_CLIENTNOTFOUND, set SESSION_STATE_ERROR\r\n"));
                            SetSessionState(SESSION_STATE_ERROR);
                            bBreak		= TRUE;
                        }
                        else
                        {
                            SetSessionState(SESSION_STATE_READ_REQUEST);
                        }
					}
					else
					{
						LOG_ERROR(("ErrorCode:%s [ZRTSPSession::ServerRun] ERROR(DESCRIBE FAILED)\r\n", GetErrorCodeString(24015)));
						SetSessionState(SESSION_STATE_ERROR);
						bBreak		= TRUE;
					}
#ifdef	DEFAULT_RTSP_OUTPUT
					LOG_DEBUG(("RTSP SESSION_STATE_READ_DESCRIBE OK\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
				}
				break;
			case SESSION_STATE_READ_SETUP:
				{
					nError	= m_Response.SendResponse((RTSP_PROTOCOL_METHOD)m_Request.GetMethod());
					if(nError == SESSION_MESSAGE_STATE_COMPLETE)
					{
						SetSessionState(SESSION_STATE_READ_REQUEST);
					}
					else
					{
						LOG_ERROR(("ErrorCode:%s [ZRTSPSession::ServerRun] ERROR(SETUP FAILED)\r\n", GetErrorCodeString(24016)));
						SetSessionState(SESSION_STATE_ERROR);
						bBreak		= TRUE;
					}
#ifdef	DEFAULT_RTSP_OUTPUT
					LOG_DEBUG(("RTSP SESSION_STATE_READ_SETUP OK\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
				}
				break;
			case SESSION_STATE_READ_PLAY:
				{
					nError	= m_Response.SendResponse((RTSP_PROTOCOL_METHOD)m_Request.GetMethod());
					if(nError == SESSION_MESSAGE_STATE_COMPLETE)
					{
						SetSessionState(SESSION_STATE_READ_REQUEST);

                        if (m_Response.GetTransportParamter() == RTSP_PROTOCOL_TRANSPORT_MULTICAST)
                        {
                            SetSessionState(SESSION_STATE_TIMEOUT);
                        }
                        else
                        {
                            m_RTPSession.Play();
                        }
						
                        m_nPlayStartTime = ZOS::milliseconds();
						nTaskTime	= DEFAULT_NET_MAX_MTIMEOUT;
					}
					else
					{
						LOG_ERROR(("ErrorCode:%s [ZRTSPSession::ServerRun] ERROR(PLAY FAILED)\r\n", GetErrorCodeString(24017)));
						SetSessionState(SESSION_STATE_ERROR);
						bBreak		= TRUE;
					}
#ifdef	DEFAULT_RTSP_OUTPUT
					LOG_DEBUG(("RTSP SESSION_STATE_READ_PLAY OK\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
				}
				break;
			case SESSION_STATE_READ_PAUSE:
				{
					nError	= m_Response.SendResponse((RTSP_PROTOCOL_METHOD)m_Request.GetMethod());
					if(nError == SESSION_MESSAGE_STATE_COMPLETE)
					{
						SetSessionState(SESSION_STATE_READ_REQUEST);
						//m_SessionStream.RequestEvent();
						nTaskTime	= DEFAULT_NET_MAX_MTIMEOUT;
					}else{
						//error
						SetSessionState(SESSION_STATE_ERROR);
						bBreak		= TRUE;
					}
#ifdef	DEFAULT_RTSP_OUTPUT
					LOG_DEBUG(("RTSP SESSION_STATE_READ_PAUSE OK\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
				}
				break;
			case SESSION_STATE_READ_STOP:
				{
					nError	= m_Response.SendResponse((RTSP_PROTOCOL_METHOD)m_Request.GetMethod(),TRUE);
					if(nError != SESSION_MESSAGE_STATE_COMPLETE)
					{
						//error
						SetSessionState(SESSION_STATE_ERROR);
					}
					else
					{
						SetSessionState(SESSION_STATE_CLOSE);
					}
					bBreak		= TRUE;
#ifdef	DEFAULT_RTSP_OUTPUT
					LOG_DEBUG(("RTSP SESSION_STATE_READ_TEARDOWN OK\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
				}
				break;

			default:
				{
					nError	= m_Response.SendResponse((RTSP_PROTOCOL_METHOD)m_Request.GetMethod());
					if(nError == SESSION_MESSAGE_STATE_COMPLETE)
					{
						SetSessionState(SESSION_STATE_READ_REQUEST);
						//m_SessionStream.RequestEvent();
						//bBreak		= TRUE;
					}else{
						//error
						SetSessionState(SESSION_STATE_ERROR);
						bBreak		= TRUE;
					}
#ifdef	DEFAULT_RTSP_OUTPUT
					LOG_DEBUG(("RTSP %d OK\r\n",m_Request.GetMethod()));
#endif	//DEFAULT_RTSP_OUTPUT
				}
				break;
			}
			if(bBreak)
			{
				break;
			}
		}
	}
	return nTaskTime;
}
int ZRTSPSession::ClientRun(UINT nEvent)
{
	int		nTaskTime	= 0;
	BOOL	bBreak		= FALSE;
	int		nError		= 0;

    if (m_nSessionTimeoutMilliSecond != GetTimeout())
    {
        SetTimeout(m_nSessionTimeoutMilliSecond);
    }

	while(IsLiveSession())
	{
		switch(GetSessionState())
		{
		case SESSION_STATE_SEND_RESPONSE:
			{
#ifdef	DEFAULT_RTSP_OUTPUT
				LOG_DEBUG(("RTSP SESSION_STATE_SEND_OPTIONS\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT

				m_Request.SendRequest(RTSP_PROTOCOL_OPTIONS);
				SetSessionState(SESSION_STATE_SEND_OPTIONS);
				m_SessionStream.RequestEvent();
				bBreak	= TRUE;
			}
			break;
		case SESSION_STATE_SEND_OPTIONS:
			{
				if(nEvent&TASK_READ_EVENT)
				{
					nError	= m_Response.ReadResponse();
					if(nError == SESSION_MESSAGE_STATE_COMPLETE)
					{
						if(m_Response.GetStatusCode() == 200)
						{
#ifdef	DEFAULT_RTSP_OUTPUT
							LOG_DEBUG(("RTSP SESSION_STATE_SEND_OPTIONS OK\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
                            if (m_pRTSPClientListener != NULL)
                            {
                                m_pRTSPClientListener->OnReceiveResponse_OPTION(0);
                            }
							m_Request.SendRequest(RTSP_PROTOCOL_DESCRIBE);
							SetSessionState(SESSION_STATE_SEND_DESCRIBE);
							m_SessionStream.RequestEvent();
						}
						else if(m_Response.GetStatusCode() == 401)
						{
#ifdef	DEFAULT_RTSP_OUTPUT
							LOG_DEBUG(("RTSP SESSION_STATE_SEND_DESCRIBE Authorization\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
							if(ZSession::Authorization())
							{
								m_Request.SendRequest(RTSP_PROTOCOL_OPTIONS);
								SetSessionState(SESSION_STATE_SEND_OPTIONS);
								m_SessionStream.RequestEvent();
							}else{
								ZRTSPSession::SetSessionState(SESSION_STATE_ERROR);
								m_SessionStream.RemoveEvent();
							}
						}
					}
					else if(nError == SESSION_MESSAGE_STATE_READ)
					{
						m_SessionStream.RequestEvent();
						DEBUG_OUT(("ZRTSPSession::ClientRun(SESSION_STATE_SEND_OPTIONS SESSION_MESSAGE_STATE_READ)\r\n"));
					}
					else
					{
                        if (m_pRTSPClientListener != NULL)
                        {
                            m_pRTSPClientListener->OnReceiveResponse_OPTION(-1);
                        }
						LOG_ERROR(("ErrorCode:%s [ZRTSPSession::ClientRun] ERROR(OPTION FAILED) errorcode = %d!\r\n", 
							GetErrorCodeString(24010),nError));
					}
				}
				bBreak	= TRUE;
			}
			break;
		case SESSION_STATE_SEND_DESCRIBE:
			{
				if(nEvent&TASK_READ_EVENT)
				{
					nError	= m_Response.ReadResponse();
					if(nError == SESSION_MESSAGE_STATE_COMPLETE)
					{
						if(m_Response.GetStatusCode() == 200)
						{
#ifdef	DEFAULT_RTSP_OUTPUT
							LOG_DEBUG(("RTSP SESSION_STATE_SEND_DESCRIBE OK\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
                            if (m_pRTSPClientListener != NULL)
                            {
                                m_pRTSPClientListener->OnReceiveResponse_DESCRIBE(0, GetContent(), 
                                    GetContentLength());
                            }
							m_SessionStream.RequestEvent();
							ZRTSPSession::ProcessDescribe();
							ZRTSPSession::SetupStream();
							m_Request.SendRequest(RTSP_PROTOCOL_SETUP);
							SetSessionState(SESSION_STATE_SEND_SETUP);
						}else if(m_Response.GetStatusCode() == 401)
						{
#ifdef	DEFAULT_RTSP_OUTPUT
							LOG_DEBUG(("RTSP SESSION_STATE_SEND_DESCRIBE Authorization\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
							if(ZSession::Authorization())
							{
								m_SessionStream.RequestEvent();
								m_Request.SendRequest(RTSP_PROTOCOL_DESCRIBE);
							}else{
								m_SessionStream.RemoveEvent();
								ZRTSPSession::SetSessionState(SESSION_STATE_ERROR);
							}
						}else{
                            if (m_pRTSPClientListener != NULL)
                            {
                                m_pRTSPClientListener->OnReceiveResponse_DESCRIBE(0, NULL, 0);
                            }
							m_SessionStream.RemoveEvent();
							ZRTSPSession::SetSessionState(SESSION_STATE_ERROR);
						}
					}
					else if(nError == SESSION_MESSAGE_STATE_READ)
					{
						m_SessionStream.RequestEvent();
						DEBUG_OUT(("ZRTSPSession::ClientRun(SESSION_STATE_SEND_DESCRIBE SESSION_MESSAGE_STATE_READ)\r\n"));
					}
					else
					{
                        if (m_pRTSPClientListener != NULL)
                        {
                            m_pRTSPClientListener->OnReceiveResponse_DESCRIBE(0, NULL, 0);
                        }
						LOG_ERROR(("ErrorCode:%s [ZRTSPSession::ClientRun] ERROR(DESCRIBE FAILED) errorcode = %d!\r\n", 
							GetErrorCodeString(24011),nError));
					}
				}
				bBreak	= TRUE;
			}
			break;
		case SESSION_STATE_SEND_SETUP:
			{
				if(nEvent&TASK_READ_EVENT)
				{
					nError	= m_Response.ReadResponse();
					if(nError == SESSION_MESSAGE_STATE_COMPLETE)
					{
						if(m_Response.GetStatusCode() == 200)
						{
#ifdef	DEFAULT_RTSP_OUTPUT
							LOG_DEBUG(("RTSP SESSION_STATE_SEND_SETUP OK\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
                            if (m_pRTSPClientListener != NULL)
                            {
                                m_pRTSPClientListener->OnReceiveResponse_SETUP(0, m_nStreamSetup);
                            }
							if(ZRTSPSession::ProcessSetup())
							{
								ZRTSPSession::SetupStream();
								m_Request.SendRequest(RTSP_PROTOCOL_SETUP);
								SetSessionState(SESSION_STATE_SEND_SETUP);
							}else{
								m_Request.SendRequest(RTSP_PROTOCOL_PLAY);
								SetSessionState(SESSION_STATE_SEND_PLAY);
							}
						}
						m_SessionStream.RequestEvent();
					}
					else if(nError == SESSION_MESSAGE_STATE_READ)
					{
						m_SessionStream.RequestEvent();
						DEBUG_OUT(("ZRTSPSession::ClientRun(SESSION_STATE_SEND_SETUP SESSION_MESSAGE_STATE_READ)\r\n"));
					}
					else
					{
                        if (m_pRTSPClientListener != NULL)
                        {
                            m_pRTSPClientListener->OnReceiveResponse_SETUP(-1, m_nStreamSetup);
                        }
						LOG_ERROR(("ErrorCode:%s [ZRTSPSession::ClientRun] ERROR(SETUP FAILED) errorcode = %d!\r\n", 
							GetErrorCodeString(24012),nError));
					}
				}
				bBreak	= TRUE;
			}
			break;
		case SESSION_STATE_SEND_PLAY:
			{
				if(nEvent&TASK_READ_EVENT)
				{
					nError	= m_Response.ReadResponse();
					if(nError == SESSION_MESSAGE_STATE_COMPLETE)
					{
						if(m_Response.GetStatusCode() == 200)
						{
#ifdef	DEFAULT_RTSP_OUTPUT
							LOG_DEBUG(("RTSP SESSION_STATE_SEND_PLAY OK\r\n"));
#endif	//DEFAULT_RTSP_OUTPUT
                            if (m_pRTSPClientListener != NULL)
                            {
                                m_pRTSPClientListener->OnReceiveResponse_PLAY(0);
                            }
							ZRTSPSession::ProcessPlay();
							nTaskTime	= 9000;
							ZTimeoutTask::RefreshTimeout();
						}
						m_SessionStream.RequestEvent();
					}
					else if(nError == SESSION_MESSAGE_STATE_READ)
					{
						m_SessionStream.RequestEvent();
						DEBUG_OUT(("ZRTSPSession::ClientRun(SESSION_STATE_SEND_PLAY SESSION_MESSAGE_STATE_READ)\r\n"));
					}
                    else if (nError == SESSION_MESSAGE_STATE_ERROR)
                    {
                        if (m_pRTSPClientListener != NULL)
                        {
                            m_pRTSPClientListener->OnReceiveResponse_PLAY(0);
                        }
                        ZRTSPSession::ProcessPlay();
                        nTaskTime	= 9000;
                        ZTimeoutTask::RefreshTimeout();
                        m_SessionStream.RequestEvent();
                    }
					else
					{
                        if (m_pRTSPClientListener != NULL)
                        {
                            m_pRTSPClientListener->OnReceiveResponse_PLAY(-1);
                        }
						LOG_ERROR(("ErrorCode:%s [ZRTSPSession::ClientRun] ERROR(PLAY FAILED) errorcode = %d!\r\n", 
							GetErrorCodeString(24013),nError));
					}
				}
				bBreak	= TRUE;
			}
			break;
		case SESSION_STATE_SEND_PAUSE:
			{
				if(nEvent&TASK_READ_EVENT)
				{
					nError	= m_Response.ReadResponse();
					m_SessionStream.RequestEvent();
				}
				bBreak	= TRUE;
			}
			break;
		case SESSION_STATE_SEND_STOP:
			{
				if(nEvent&TASK_READ_EVENT)
				{
					nError	= m_Response.ReadResponse();
				}
				bBreak	= TRUE;
			}
			break;
		case SESSION_STATE_PLAY:
			{
				bBreak		= TRUE;
			}
			break;
		default:
			{
				bBreak	= TRUE;
			}
			break;
		}
		if(bBreak)
		{
			break;
		}
	}
	return nTaskTime;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTSPSession::ProcessRequest(ZRTSPRequest* pRTSPRequest)
{
	if(pRTSPRequest != NULL)
	{
        if (m_bFirstCommand)
        {
			BOOL bOutNumber = FALSE;
            if (m_pRTSPServerListener != NULL)
            {
                m_pRTSPServerListener->OnFirstCommand(this, pRTSPRequest->GetURI(),&bOutNumber);
            }
            m_bFirstCommand = FALSE;
			if(bOutNumber == TRUE)
			{
                LOG_DEBUG(("[ZRTSPSession::ProcessRequest] bOutNumber == TRUE %s \r\n", pRTSPRequest->GetURI()));
				if (m_pRTSPServerListener != NULL)
				{
					m_pRTSPServerListener->OnNotifySessionTimeout(this);
				}
				//ZRTSPSession::SetRTSPSessionStatus(RTSP_STATUS_CLIENTFORBIDDEN);
				SetSessionState(SESSION_STATE_READ_STOP);
				return TRUE;
			}
        }
		switch(pRTSPRequest->GetMethod())
		{
		case RTSP_PROTOCOL_OPTIONS:
			{
                if (m_pRTSPServerListener != NULL)
                {
                    m_pRTSPServerListener->OnNotifyOption(this, pRTSPRequest->GetURI());
                }
                RefreshTimeout();
                m_RTPSession.RefreshTimeout();
				ZRTSPSession::SetSessionState(SESSION_STATE_READ_OPTIONS);
			}
			break;
		case RTSP_PROTOCOL_DESCRIBE:
			{
				if(!DoDescribe())
				{
					ZRTSPSession::SetRTSPSessionStatus(RTSP_STATUS_CLIENTNOTFOUND);
				}
				ZRTSPSession::SetSessionState(SESSION_STATE_READ_DESCRIBE);
			}
			break;
		case RTSP_PROTOCOL_SETUP:
			{
				if(!DoSetup())
				{
					ZRTSPSession::SetRTSPSessionStatus(RTSP_STATUS_CLIENTNOTFOUND);
				}
				ZRTSPSession::SetSessionState(SESSION_STATE_READ_SETUP);
			}
			break;
		case RTSP_PROTOCOL_PLAY:
			{
				if(!DoPlay())
				{
					ZRTSPSession::SetRTSPSessionStatus(RTSP_STATUS_CLIENTNOTFOUND);
				}
				ZRTSPSession::SetSessionState(SESSION_STATE_READ_PLAY);
			}
			break;
		case RTSP_PROTOCOL_PAUSE:
			{
				if(!DoPause())
				{
					ZRTSPSession::SetRTSPSessionStatus(RTSP_STATUS_CLIENTNOTFOUND);
				}
				ZRTSPSession::SetSessionState(SESSION_STATE_READ_PAUSE);
			}
			break;
		case RTSP_PROTOCOL_TEARDOWN:
			{
				if(!DoTearDown())
				{
				}
				ZRTSPSession::SetSessionState(SESSION_STATE_READ_STOP);
			}
			break;
		case RTSP_PROTOCOL_ANNOUNCE:
			{
				if(!DoAnnounce())
				{
				}
                ZRTSPSession::SetSessionState(SESSION_STATE_READ_OTHERS);
			}
            break;
		case RTSP_PROTOCOL_GET_PARAMETER:
            {
                if (!DoGetParameter())
                {
                }
                ZRTSPSession::SetSessionState(SESSION_STATE_READ_OTHERS);
            }
            break;
		case RTSP_PROTOCOL_SET_PARAMETER:
            {
                if (!DoSetParameter())
                {
                }
                ZRTSPSession::SetSessionState(SESSION_STATE_READ_OTHERS);
            }
            break;
		case RTSP_PROTOCOL_REDIRECT:
		case RTSP_PROTOCOL_RECORD:
			{
				ZRTSPSession::SetSessionState(SESSION_STATE_READ_OTHERS);
			}
			break;
		case RTSP_PROTOCOL_DATA:
			{
				if(!DoData())
				{
				}
			}
			break;
		case RTSP_PROTOCOL_METHOD_INVALID:
			{
				ZRTSPSession::SetRTSPSessionStatus(RTSP_STATUS_SERVERNOTIMPLEMENTED);
				ZRTSPSession::SetSessionState(SESSION_STATE_READ_OTHERS);
			}
			break;
		default:
			{
				ZRTSPSession::SetRTSPSessionStatus(RTSP_STATUS_SERVERNOTIMPLEMENTED);
				ZRTSPSession::SetSessionState(SESSION_STATE_READ_OTHERS);
			}
			break;
		}
		return TRUE;
	}

	return FALSE;
}
BOOL ZRTSPSession::ProcessDescribe()
{
	BOOL bReturn = FALSE;
	if(GetContentType() != NULL)
	{
		if(strcasecmp(GetContentType(),"application/sdp") == 0)
		{
			bReturn = ZRTSPSession::ParseSDP();
		}
	}
	else
	{
		if(ZRTSPSession::ParseSDP())
		{
			SetContentType("application/sdp");
			bReturn = TRUE;
		}
	}
	return bReturn;
}
BOOL ZRTSPSession::SetupStream()
{
	if(GetContentType() != NULL)
	{
		if(strcasecmp(GetContentType(),"application/sdp") == 0)
		{
			if (m_nStreamSetup < m_aSessionDescription.GetStreamCount())
			{
				return m_RTPSession.AddClientStream(&m_Request);
			}
		}
	}else{
		if(m_nStreamSetup == 0)
		{
			return m_RTPSession.AddClientStream(&m_Request);
		}
	}
	return FALSE;
}
BOOL ZRTSPSession::ProcessSetup()
{
	m_nStreamSetup	++;
	m_RTPSession.SetupClientStream(&m_Response);

	if(GetContentType() != NULL)  
	{
		if(strcasecmp(GetContentType(),"application/sdp") == 0)
		{
			return (m_nStreamSetup < m_aSessionDescription.GetStreamCount());
		}
	}
	return FALSE;
}
BOOL ZRTSPSession::ProcessPlay()
{
	BOOL	bReturn	= FALSE;

	m_nPlayStartTime	= ZOS::milliseconds();
	m_nLastRTCPTime		= m_nPlayStartTime;

	bReturn	= m_RTPSession.Play();

	SetSessionState(SESSION_STATE_PLAY);

	return bReturn;
}
BOOL ZRTSPSession::ProcessTearDown()
{
	BOOL	bReturn	= FALSE;
	
	bReturn	= m_RTPSession.TearDown();
    m_nSessionStopTime = ZOS::milliseconds();
	
	SetSessionState(SESSION_STATE_STOP);

	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTSPSession::DoDescribe()
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_SessionMutex);

#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::DoDescribe\r\n"));
#endif	//ZRTSPSESSION_DEBUG
	if(ZRTSPSession::InitRTPSession())
	{
        bReturn = ZRTSPSession::CreateSDP();
	}

    m_bDoDescribe = TRUE;

	return bReturn;
}
BOOL ZRTSPSession::DoSetup()
{
	BOOL			bReturn			= FALSE;
    int             nSendType       = 0;//0-unicast, 1-multicast
    ZDataPin        *pDataPin       = NULL;
    //ZSessionDescription::SDP_MEDIA       *pSDPMedia      = NULL;
    DWORD           nStreamIndex    = 0;
	ZOSMutexLocker	locker(&m_SessionMutex);

#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::DoSetup\r\n"));
#endif	//ZRTSPSESSION_DEBUG
	if(ZRTSPSession::InitRTPSession())
	{
        if (
            (!m_bDoDescribe)
            && (
                (GetContent()==NULL)
                || (GetContentLength() <= 0)
                )
            )
        {// used for hik-type player rtsp. it has no describe request, first command is setup.
            bReturn = CreateSDP();
            m_bDoDescribe = TRUE;
        }
        else
        {
            bReturn = TRUE;
        }

        nStreamIndex = ZRTSPSession::GetStream(&m_Request);
        LOG_DEBUG(("[ZRTSPSession::DoSetup] nStreamIndex %d \r\n", nStreamIndex));
        m_Response.SetClientPort(m_Request.GetClientPortA(),m_Request.GetClientPortB());
        m_Response.SetChannel(m_Request.GetChannelA(),m_Request.GetChannelB());
        m_Response.SetTransportType(m_Request.GetTransportType());
        if (m_pRTSPServerListener != NULL)
        {
            m_pRTSPServerListener->OnNotifySetup(this, m_Request.GetURI(), nStreamIndex, &nSendType, &pDataPin);
            m_Response.SetTransportParamter(nSendType);
			if(nSendType == 0)
			{
				if (pDataPin != NULL)
				{
					if(m_nStreamType[nStreamIndex] == 100)
					{
						m_RTPSession.AddServerStream(&m_Request, pDataPin,3200);
					}
					else if(m_nStreamType[nStreamIndex] == 200)
					{
						m_RTPSession.AddServerStream(&m_Request, pDataPin,200);
					}
					else if(m_nStreamType[nStreamIndex] == 201)
					{
						m_RTPSession.AddServerStream(&m_Request, pDataPin,200);
					}
					else if(m_nStreamType[nStreamIndex] == 300)
					{
						m_RTPSession.AddServerStream(&m_Request, pDataPin,200);
					}
					else if(m_nStreamType[nStreamIndex] == 301)
					{
						m_RTPSession.AddServerStream(&m_Request, pDataPin,200);
					}
					m_RTPSession.SetupServerStream(&m_Request, &m_Response);
				}
			}
        }
        m_nStreamSetup ++;

		ZRTSPSession::SetPause(FALSE);
	}
	return bReturn;
}
BOOL ZRTSPSession::DoPlay()
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_SessionMutex);

	//LOG_DEBUG(("[ZRTSPSession::DoPlay] before\r\n"));
#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::DoPlay\r\n"));
#endif	//ZRTSPSESSION_DEBUG
    if (m_pRTSPServerListener != NULL)
    {
        m_pRTSPServerListener->OnNotifyPlay(this, (int)m_nStartTime);
    }
	ZRTSPSession::SetPause(FALSE);
	bReturn		= TRUE;

	//LOG_DEBUG(("[ZRTSPSession::DoPlay] after\r\n"));
	return bReturn;
}
BOOL ZRTSPSession::DoPause()
{
	BOOL			bReturn	= FALSE;
    int             nCurSecond = 0;
	ZOSMutexLocker	locker(&m_SessionMutex);

#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::DoPause\r\n"));
#endif	//ZRTSPSESSION_DEBUG
    if (m_pRTSPServerListener != NULL)
    {
        m_pRTSPServerListener->OnNotifyPause(this, &nCurSecond);
        m_nStartTime = (FLOAT64)nCurSecond;
    }
	//m_RTPSession.Pause();
	ZRTSPSession::SetPause(TRUE);
	bReturn		= TRUE;

	return bReturn;
}
BOOL ZRTSPSession::DoTearDown()
{
	BOOL			bReturn	= FALSE;
    {
        ZOSMutexLocker	locker(&m_SessionMutex);

#ifdef	ZRTSPSESSION_DEBUG
        LOG_DEBUG(("ZRTSPSession::DoTearDown\r\n"));
#endif	//ZRTSPSESSION_DEBUG

        bReturn	= m_RTPSession.TearDown();

        SetSessionState(SESSION_STATE_CLOSE);
        m_nSessionStopTime = ZOS::milliseconds();
#ifdef	ZRTSPSESSION_DEBUG
        LOG_DEBUG(("ZRTSPSession::DoTearDown OK\r\n"));
#endif	//ZRTSPSESSION_DEBUG
    }

    if (m_pRTSPServerListener != NULL)
    {
        //LOG_DEBUG(("[ZRTSPSession::DoTearDown] before OnNotifyTearDown\r\n"));
        m_pRTSPServerListener->OnNotifyTearDown(this);
        //LOG_DEBUG(("[ZRTSPSession::DoTearDown] after OnNotifyTearDown\r\n"));
    }

	return bReturn;
}
BOOL ZRTSPSession::DoAnnounce()
{
	RefreshTimeout();

	m_RTPSession.RefreshTimeout();

#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::DoAnnounce OK\r\n"));
#endif	//ZRTSPSESSION_DEBUG

	return TRUE;
}
BOOL ZRTSPSession::DoGetParameter()
{
    RefreshTimeout();

    m_RTPSession.RefreshTimeout();

    return TRUE;
}
BOOL ZRTSPSession::DoSetParameter()
{
    RefreshTimeout();

    m_RTPSession.RefreshTimeout();

    return TRUE;
}
BOOL ZRTSPSession::DoData()
{
	//MESSAGE_OUT(("[ZRTSPSession::DoData] RefreshTimeout\r\n"));
	RefreshTimeout();
	m_RTPSession.RefreshTimeout();

#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::DoData OK\r\n"));
#endif	//ZRTSPSESSION_DEBUG

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTSPSession::DoTaskKill()
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_SessionMutex);

#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::DoTaskKill\r\n"));
#endif	//ZRTSPSESSION_DEBUG
	
	bReturn	= m_RTPSession.TearDown();

	SetSessionState(SESSION_STATE_CLOSE);

#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::DoTaskKill OK\r\n"));
#endif	//ZRTSPSESSION_DEBUG
	return bReturn;
}
BOOL ZRTSPSession::DoTimeOut()
{
	BOOL			bReturn	= FALSE;

    {
        ZOSMutexLocker	locker(&m_SessionMutex);

        //#ifdef	ZRTSPSESSION_DEBUG

        m_nSessionStopTime = ZOS::milliseconds();

        //#endif	//ZRTSPSESSION_DEBUG

        bReturn	= m_RTPSession.TearDown();
        //DEBUG_OUT(("[ZRTSPSession::DoTimeOut] SESSION_STATE_TIMEOUT\r\n"));
        SetSessionState(SESSION_STATE_TIMEOUT);
    }

    {
        if (GetSessionSubType() == SESSION_TYPE_SERVER)
        {
            LOG_DEBUG(("[ZRTSPSession::DoTimeOut] SESSION_TYPE_SERVER\r\n"));
            if (m_pRTSPServerListener != NULL)
            {
                m_pRTSPServerListener->OnNotifySessionTimeout(this);
            }
        }
        else if (GetSessionSubType() == SESSION_TYPE_CLIENT)
        {
            LOG_DEBUG(("[ZRTSPSession::DoTimeOut] SESSION_TYPE_CLIENT\r\n"));
        }
    }

#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::DoTimeOut OK\r\n"));
#endif	//ZRTSPSESSION_DEBUG
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTSPSession::InitRTPSession()
{
	BOOL		bReturn	= FALSE;

	bReturn	= TRUE;

#ifdef	ZRTSPSESSION_DEBUG
	LOG_DEBUG(("ZRTSPSession::CreateRTPSession(%s)\r\n",(bReturn?"OK":"ERROR")));
#endif	//ZRTSPSESSION_DEBUG
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
// used when is SESSION_TYPE_SERVER
BOOL ZRTSPSession::CreateSDP()
{
	BOOL	bReturn		= FALSE;
	int		nHeader		= 0;
	char	sHeader[(DEFAULT_SESSION_BUFFER_SIZE + 4)];

	nHeader	= sizeof(sHeader);
	memset(sHeader,0,nHeader);

    if (m_pRTSPServerListener != NULL)
    {
        if (m_pRTSPServerListener->OnNotifyDescribe(this, m_Request.GetURI(), sHeader, &nHeader))
        {
            bReturn = TRUE;
        }
    }
	if (bReturn)
	{
		m_aSessionDescription.ParseDescription(sHeader,nHeader,m_pSessionURI);
		ZRTSPSession::SetContent(sHeader,nHeader);
		m_nStreamType[0] = SDP_GetStreamType(sHeader,nHeader,0);
		m_nStreamType[1] = SDP_GetStreamType(sHeader,nHeader,1);
	}
	else
	{
		ZRTSPSession::SetContent(NULL, 0);
	}
	return bReturn;
}
BOOL ZRTSPSession::ParseSDP()
{
	return m_aSessionDescription.ParseDescription(m_pContentData,m_nContentData,m_pSessionURI);
}
///////////////////////////////////////////////////////////////////////////////
char * ZRTSPSession::GetSessionURI()
{
	if (
		(m_Request.GetMethod()==RTSP_PROTOCOL_PLAY)
		|| (m_Request.GetMethod()==RTSP_PROTOCOL_TEARDOWN)
		)
	{
		if (strlen(m_Response.GetContentBase()) > 0)
		{
			return m_Response.GetContentBase();
		}
		else
		{
			return ZSession::GetSessionURI();
		}
	}
	else
	{
		return ZSession::GetSessionURI();
	}
}
///////////////////////////////////////////////////////////////////////////////
ZSession* ZRTSPSession::CreateSession(SESSION_SUBTYPE eSubType,ZSocket* pSocket,char* sURI,
									  RTSP_PROTOCOL_TRANSPORT eProtocolTransport,char* sUser,char* sPassword)
{
	ZRTSPSession*	pSession	= NULL;
	if(pSocket != NULL)
	{
		pSession	= NEW ZRTSPSession(eSubType);

		if(pSession != NULL)
		{
			pSocket->SetTask(pSession);
			pSession->SetSessionStream(pSocket);
			pSession->SetSessionURI(sURI);
			pSession->SetProtocolTransportType(eProtocolTransport);
			pSession->SetSessionUserPassword(sUser,sPassword);

			if(!pSession->Create())
			{
				SAFE_DELETE(pSession);
			}
		}
	}

	return pSession;
}
char* ZRTSPSession::GetRangeString()
{
	if(m_pRangeString == NULL)
	{
		m_pRangeString	= NEW char[64];
		if(m_pRangeString != NULL)
		{
			memset(m_pRangeString,0,64);
			sprintf(m_pRangeString,"npt=%.3f-%.3f",m_nStartTime,m_nStopTime);
		}
	}
	return (m_pRangeString);
}

void ZRTSPSession::SetRtpInfo(char* streamUrl)
{
	

}

DWORD ZRTSPSession::GetStream(ZRTSPRequest *pRTSPRequest)
{
    DWORD	nStreamIndex	= 0;
    DWORD	nStream			= 0;

    if(pRTSPRequest != NULL)
    {
        if (m_aSessionDescription.GetStreamCount() > 0)
        {
            nStream = 0xFFFFFFFF;
            for(nStreamIndex = 0; nStreamIndex < m_aSessionDescription.GetStreamCount(); nStreamIndex ++)
            {
                if(m_aSessionDescription.GetStream(nStreamIndex)->m_nMediaPayloadType != 0)
                {
                    CHAR*	sControl	= m_aSessionDescription.GetStream(nStreamIndex)->m_sMediaControlName;
                    if(m_aSessionDescription.GetStream(nStreamIndex)->m_nMediaTrackID == 0xFFFFFFFF)
                    {
                        if(strcmp(sControl,pRTSPRequest->m_sURI) == 0)
                        {
                            nStream		= nStreamIndex;
                            break;
                        }
                    }else{
                        if(strcmp(sControl,pRTSPRequest->m_sTrackPath) == 0)
                        {
                            nStream		= nStreamIndex;
                            break;
                        }
                    }
                }
            }
        }
    }
    return nStream;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
