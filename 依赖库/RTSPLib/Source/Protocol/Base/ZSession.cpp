#include "ZSession.h"
#include "ZEvent.h"
#include "ZRequestStream.h"
#include "ZResponseStream.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "ZRTSPSession.h"
///////////////////////////////////////////////////////////////////////////////
static	const	char*	SESSION_NAME[]	=
{
	"HTTP",	//HTTP SESSION
	"RTSP",	//RTSP SESSION
	"RTP",	//RTP SESSION
	"RTCP",	//RTCP SESSIOM
	"RTMP",	//RTMP SESSIOM
	"SCP",	//SCP SESSION
};
///////////////////////////////////////////////////////////////////////////////
CONST ZSession::SESSION_FACTORY_DATA ZSession::m_aSessionFactory[]	=
{
	SESSION_INSTANCE_REGIST(ZRTSPSession,	(CHAR*)SESSION_NAME[SESSION_TYPE_RTSP],	SESSION_TYPE_RTSP)
	//SESSION_INSTANCE_REGIST(ZRTMPSession,	(CHAR*)SESSION_NAME[SESSION_TYPE_RTMP],	SESSION_TYPE_RTMP)
	//SESSION_INSTANCE_REGIST(ZSCPSession,	(CHAR*)SESSION_NAME[SESSION_TYPE_SCP],	SESSION_TYPE_SCP)
};
CONST INT ZSession::m_nSessionFactory	= COUNT_OF(m_aSessionFactory);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//#define	ZSESSION_DEBUG	1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZSession::ZSession(SESSION_TYPE eType,SESSION_SUBTYPE eSubType)
:ZTimeoutTask("SessionTimeoutTask")
,m_SessionMutex("SessionMutex")
,m_SessionAuthorization()
,m_SessionAuthenticator()
,m_nSessionType(eType)
,m_nSessionSubType(eSubType)
,m_nSessionState(SESSION_STATE_READY)
,m_pSessionID(NULL)
,m_pSessionURI(NULL)
,m_pContentType(NULL)
,m_pContentData(NULL)
,m_nContentData(0)
,m_nSequence(0)
,m_nStartTime(0)
,m_nStopTime(0)
,m_bPause(TRUE)
{
	SetTimeoutTask(this);
}
ZSession::~ZSession()
{
	TMASSERT((m_nSessionState == SESSION_STATE_CLOSE));
	SAFE_DELETE_ARRAY(m_pSessionID);
	SAFE_DELETE_ARRAY(m_pSessionURI);
	SAFE_DELETE_ARRAY(m_pContentType);
	SAFE_DELETE_ARRAY(m_pContentData);
	m_nContentData		= 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZSession::Create()
{
	ZTimeoutTask::Create();
	SetTimeout(DEFAULT_SESSION_TIMEOUT);

	if (m_nSessionSubType == SESSION_TYPE_SERVER)
	{
		GenerateSessionID();
	}
	m_nSequence		= 0;
	m_nStartTime	= 0;
	m_nStopTime		= 0;

	SetSessionState(SESSION_STATE_OPEN);

	return TRUE;
}
BOOL ZSession::Close()
{
	ZTimeoutTask::Close();

	SetSessionState(SESSION_STATE_CLOSE);

	SAFE_DELETE_ARRAY(m_pContentData);
	m_nContentData		= 0;

	SAFE_DELETE_ARRAY(m_pSessionID);

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZSession::GetDataStreamCount()
{
	return 0;
}
ZBaseStream* ZSession::GetDataStream(int i)
{
	return NULL;
}
ZDataPin* ZSession::GetDataPin(int i)
{
	return NULL;
}
//BOOL ZSession::Status(STATUS_TYPE	eRequestType, char *sStatus)
//{
//	strcpy(sStatus, " ");
//	return TRUE;
//}
BOOL ZSession::OnDataPinClose()
{
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZSession::SetSessionUserPassword(CHAR* sUser,CHAR* sPassword)
{
    //LOG_DEBUG(("[ZSession::SetSessionUserPassword] sUser %s sPassword %s\r\n",
    //    sUser, sPassword));
	m_SessionAuthorization.SetUserNamePassword(sUser,sPassword);
	m_SessionAuthenticator.SetUserNamePassword(sUser,sPassword);
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZSession::Authorization()
{
	return m_SessionAuthorization.Authorization();
}
BOOL ZSession::Authentication()
{
	return m_SessionAuthenticator.Authentication();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZSession::AuthorizationCheck(ZAuth::AUTH_TYPE nType,CHAR* sParam)
{
	if(m_SessionAuthorization.GetAuthType() == nType)
	{
		m_SessionAuthorization.CheckAuthData(sParam,strlen(sParam));
		return TRUE;
	}
	return FALSE;
}
BOOL ZSession::AuthenticationCheck(ZAuth::AUTH_TYPE nType,CHAR* sParam)
{
	m_SessionAuthenticator.SetAuthType(nType);
	m_SessionAuthenticator.CheckAuthData(sParam,strlen(sParam));
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
char* ZSession::GetSessionID()
{
	return(m_pSessionID);
}
char* ZSession::SetSessionID(const char* sSessionID)
{
	SAFE_DELETE_ARRAY(m_pSessionID);

	if(sSessionID != NULL)
	{
		int		nLen	= MIN(strlen(sSessionID),DEFAULT_SESSION_ID_SIZE);
		nLen			+= 4;
		m_pSessionID	= NEW char[nLen];
		if(m_pSessionID != NULL)
		{
			strncpy(m_pSessionID,sSessionID,nLen);
		}
	}
	return(m_pSessionID);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
char* ZSession::GetSessionURI()
{
	return(m_pSessionURI);
};
char* ZSession::SetSessionURI(const char* sURI)
{
	SAFE_DELETE_ARRAY(m_pSessionURI);

	if(sURI != NULL)
	{
		int		nLen	= MIN(strlen(sURI),MAX_URI_PATH);
		nLen			+= 4;
		m_pSessionURI	= NEW char[nLen];
		if(m_pSessionURI != NULL)
		{
			strncpy(m_pSessionURI,sURI,nLen);
		}
	}
	return(m_pSessionURI);
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SESSION_TYPE ZSession::GetSessionType()
{
	return(m_nSessionType);
}
SESSION_SUBTYPE ZSession::GetSessionSubType()
{
	return(m_nSessionSubType);
}
SESSION_STATE ZSession::GetSessionState()
{
	return(m_nSessionState);
};
SESSION_STATE ZSession::SetSessionState(SESSION_STATE nSessionState)
{
	ZOSMutexLocker	locker(&m_SessionMutex);

	m_nSessionState	= nSessionState;
	return(m_nSessionState);
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
char* ZSession::GetContent()
{
	return(m_pContentData);
};
CHAR* ZSession::GetContentType()
{
	return(m_pContentType);
};
int ZSession::GetContentLength()
{
	return(m_nContentData);
};
int ZSession::SetContentType(CONST CHAR* sContentType)
{
	UINT	nLen	= 0;

	SAFE_DELETE_ARRAY(m_pContentType);
	if(sContentType != NULL)
	{
		nLen			= (strlen(sContentType) + 4);
		m_pContentType	= NEW CHAR[nLen];
		if(m_pContentType != NULL)
		{
			strcpy(m_pContentType,sContentType);
		}
	}
	return nLen;
}
int ZSession::SetContent(const char* sContent,int nContent)
{
	ZOSMutexLocker	locker(&m_SessionMutex);
	int	nLen	= 0;

	LOG_DEBUG(("[ZSession::SetContent] %s\r\n", sContent));
	SAFE_DELETE_ARRAY(m_pContentData);
	m_nContentData		= 0;
	if((sContent != NULL)&&(nContent > 0))
	{
		nLen			= MIN(nContent,DEFAULT_SESSION_BUFFER_SIZE);
		m_pContentData	= NEW char[nLen+4];
		memset(m_pContentData, 0, nLen+4);
		if(m_pContentData != NULL)
		{
			memcpy(m_pContentData,sContent,nLen);
			m_nContentData	= nLen;
		}
	}
	return nLen;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ZSession::AddSequence()
{
	m_nSequence	++;
	return(m_nSequence);
}
int ZSession::GetSequence()
{
	return(m_nSequence);
};
int ZSession::SetSequence(int nSequence)
{
	m_nSequence	= nSequence;
	return(m_nSequence);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FLOAT64 ZSession::GetStartTime()
{
	return(m_nStartTime);
}
FLOAT64 ZSession::GetStopTime()
{
	return(m_nStopTime);
}
FLOAT64 ZSession::GetTimeStamp()
{
	return(m_nStopTime-m_nStartTime);
}
FLOAT64 ZSession::SetTime(FLOAT64 nStartTime,FLOAT64 nStopTime)
{
	m_nStartTime	= nStartTime;
	if(nStopTime > 0)
	{
		m_nStopTime		= nStopTime;
	}
	return(m_nStopTime-m_nStartTime);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZSession::SetPause(BOOL bPause)
{
	m_bPause	= bPause;
	return m_bPause;
}

///////////////////////////////////////////////////////////////////////////////
BOOL ZSession::IsLiveSession()
{
	return	(	m_nSessionState != SESSION_STATE_ERROR		&&
				m_nSessionState != SESSION_STATE_TIMEOUT	&&
				m_nSessionState != SESSION_STATE_CLOSE);
}
BOOL ZSession::IsPauseSession()
{
	return m_bPause;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ZSession::GenerateSessionID()
{
	CHAR	sSessionID[68];
	UINT	nMicroseconds	= (UINT)ZOS::milliseconds();
	UINT	nFirstRandom	= 0;
	UINT	nSecondRandom	= 0;
	UINT64	nSessionID		= 0;

	memset(sSessionID,0,sizeof(sSessionID));

    ::srand((unsigned int)nMicroseconds);

	nFirstRandom	= (UINT)(UINT64)this;
	
	nSecondRandom	= ::rand();

    ::srand((unsigned int)nFirstRandom);

	nSecondRandom	+= ::rand();

	nSessionID		= (UINT64)nFirstRandom;
	nSessionID		<<=32;
	nSessionID		+= (UINT64)nSecondRandom;

	sprintf(sSessionID,"%"F_NUM_64"d",nSessionID);
	ZSession::SetSessionID(sSessionID);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ZSession::Run(int nEvent)
{
	int		nTaskTime	= 0;
	UINT	nLocalEvent	= 0;

	nLocalEvent	= GetEvent(nEvent);

	ZTimeoutTask::Run(nLocalEvent);

	if(nLocalEvent&TASK_KILL_EVENT)
	{
		DoTaskKill();
		nTaskTime	= 0;
	}else
	if(nLocalEvent&TASK_TIMEOUT_EVENT)
	{
		DoTimeOut();
		nTaskTime	= 0;
	}else
	if(nLocalEvent&TASK_START_EVENT)
	{
		if(m_nSessionSubType == SESSION_TYPE_SERVER)
		{
			nTaskTime	= ServerRun(TASK_START_EVENT);
		}
		if(m_nSessionSubType == SESSION_TYPE_CLIENT)
		{
			nTaskTime	= ClientRun(TASK_START_EVENT);
		}
	}else
	if(nLocalEvent&TASK_READ_EVENT)
	{
		if(m_nSessionSubType == SESSION_TYPE_SERVER)
		{
			nTaskTime	= ServerRun(TASK_READ_EVENT);
		}
		if(m_nSessionSubType == SESSION_TYPE_CLIENT)
		{
			nTaskTime	= ClientRun(TASK_READ_EVENT);
		}
	}else
	if(nLocalEvent&TASK_UPDATE_EVENT)
	{
		if(m_nSessionSubType == SESSION_TYPE_SERVER)
		{
			nTaskTime	= ServerRun(TASK_UPDATE_EVENT);
		}
		if(m_nSessionSubType == SESSION_TYPE_CLIENT)
		{
			nTaskTime	= ClientRun(TASK_UPDATE_EVENT);
		}
	}
	return nTaskTime;
 }
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZSession* ZSession::CreateInstance(SESSION_TYPE eType,SESSION_SUBTYPE eSubType,ZSocket* pSocket,
								   char* sURI,RTSP_PROTOCOL_TRANSPORT eProtocolTransport,char* sUser,char* sPassword)
{
	ZSession*	pSession	= NULL;
	int			i			= 0;

	for(i = 0; i < m_nSessionFactory; i ++)
	{
		if(m_aSessionFactory[i].m_nSessionType == eType)
		{
			pSession	= (m_aSessionFactory[i].m_pCreateZSession)(eSubType,pSocket,sURI,eProtocolTransport,sUser,sPassword);
			break;
		}
	}

	return pSession;
}
ZSession* ZSession::CloseInstance(ZSession* pSession)
{
	if(pSession != NULL)
	{
		pSession->Close();
	}
	SAFE_DELETE(pSession);
	return pSession;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SESSION_TYPE ZSession::GetSessionType(CONST CHAR* sName)
{
	int				i		= 0;
	SESSION_TYPE	nType	= (SESSION_TYPE)SESSION_TYPE_ERROR;

	if(sName != NULL)
	{
		for(i = 0; i < m_nSessionFactory; i ++)
		{
			if(strcasecmp(m_aSessionFactory[i].m_sSessionLabel,sName) == 0)
			{
				nType	= m_aSessionFactory[i].m_nSessionType;
				break;
			}
		}

	}
	return nType;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
