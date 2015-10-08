#include "ZSessionDescription.h"
#include "ZHeaderParser.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const	char	ZSessionDescription::m_sSDPHeaderFormat[]	=
"v=0"DEFAULT_HEADER_EOL
"o=- 5A4D45444941 5A4D454449413031 IN IP4 %s"DEFAULT_HEADER_EOL
"c=IN IP4 0.0.0.0"DEFAULT_HEADER_EOL;
///////////////////////////////////////////////////////////////////////////////
const	char	ZSessionDescription::m_sSDPMediaFormat[]	=
"m=%s 0 RTP/AVP %d"DEFAULT_HEADER_EOL
"a=rtpmap:%d %s/%d%s"DEFAULT_HEADER_EOL
"a=fmtp:"DEFAULT_HEADER_EOL;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZSessionDescription::ZSessionDescription()
:ZObject("SessionDescription")
,m_SDPStream(2)
,m_nSDPStream(0)
{
	memset(m_sURI,0,sizeof(m_sURI));
	memset(&m_SDPHeader,0,sizeof(m_SDPHeader));
	memset(m_sControlNameBase,0,sizeof(m_sControlNameBase));
}
ZSessionDescription::~ZSessionDescription()
{
	int	i	= 0;
	for(i = 0; i < m_SDPStream.Count(); i ++)
	{
		if(m_SDPStream[i] != NULL)
		{
			delete m_SDPStream[i];
		}
	}
	m_nSDPStream		= 0;
	m_SDPStream.RemoveAll();
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZSessionDescription::InitDescription(char* sBuffer,int nBuffer,char* sURI)
{
	if(sURI != NULL)
	{
		strncpy(m_sURI,sURI,MAX_URI_PATH);
	}

//	sprintf(sdata,m_sSDPHeaderFormat,0);

	return FALSE;
}
BOOL ZSessionDescription::ParseDescription(char* sBuffer,int nBuffer,char* sURI)
{
	if(sURI != NULL)
	{
		strncpy(m_sURI,sURI,MAX_URI_PATH);
	}
	// for reuse clean last time value
	for (int i = 0; i < m_SDPStream.Count(); i ++)
	{
		if (m_SDPStream[i] != NULL)
		{
			delete m_SDPStream[i];
		}
	}
	m_nSDPStream	= 0;
	m_SDPStream.RemoveAll();
	memset(&m_SDPHeader,0,sizeof(m_SDPHeader));

	if(sBuffer != NULL && nBuffer > 0)
	{
		ZHeaderParser	Parser(sBuffer,nBuffer);
		int				nLine	= 0;
		int				nPos	= 0;
		if(Parser.GetHeaderSize() > 0)
		{
			for(nLine = 0; nLine < (int)Parser.GetLineCount(); nLine ++)
			{
				nPos	= 0;
				Parser.SkipSpace(nLine,&nPos);
				switch(Parser.GetChar(nLine,&nPos))
				{
				case 'v':
					ZSessionDescription::ParseVersion(&Parser,nLine,&nPos);
					break;
				case 'c':
					ZSessionDescription::ParseConnection(&Parser,nLine,&nPos);
					break;
				case 't':
					ZSessionDescription::ParseTimes(&Parser,nLine,&nPos);
					break;
				case 'a':
					ZSessionDescription::ParseAttributes(&Parser,nLine,&nPos);
					break;
				case 'm':
					ZSessionDescription::ParseMedia(&Parser,nLine,&nPos);
					break;
				default:
					break;
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZSessionDescription::GetStreamCount()
{
	return m_nSDPStream;
}
ZSessionDescription::SDP_MEDIA* ZSessionDescription::GetStream(DWORD nIndex)
{
	if(nIndex >= 0 && nIndex < m_nSDPStream)
	{
		return m_SDPStream[nIndex];
	}
	return NULL;
}
DWORD ZSessionDescription::GetMediaDstAddr()
{
	return m_SDPHeader.m_nAddress;
}
DWORD ZSessionDescription::GetMediaDstPort(DWORD nIndex)
{
	return m_SDPStream[nIndex]->m_nMediaDestinationsPort;	
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZSessionDescription::ParseVersion(ZHeaderParser* pHeader,int nLine,int* nPos)
{
	if(pHeader != NULL)
	{
		pHeader->SkipChar(nLine,nPos,'=');
		pHeader->SkipSpace(nLine,nPos);
		//ver
		m_SDPHeader.m_nVersion	= pHeader->GetIntegerNumber(nLine,nPos);
		return TRUE;
	}
	return FALSE;
}
BOOL ZSessionDescription::ParseConnection(ZHeaderParser* pHeader,int nLine,int* nPos)
{
	DWORD	nAddr	= 0;
	DWORD	nTTL	= 0;

	if(pHeader != NULL)
	{
		pHeader->SkipChar(nLine,nPos,'=');
		pHeader->SkipSpace(nLine,nPos);
		//net type
		TMASSERT((strcmp(pHeader->GetString(nLine,nPos,' '),"IN")==0));
		pHeader->SkipSpace(nLine,nPos);
		//addr type
		TMASSERT((strcmp(pHeader->GetString(nLine,nPos,' '),"IP4")==0));
		pHeader->SkipSpace(nLine,nPos);
		nAddr	= ZSocket::ConvertAddr(pHeader->GetString(nLine,nPos,'/'));
		if(pHeader->IsLineEnd(nLine,*nPos))
		{
			pHeader->SkipSpace(nLine,nPos);
			//ttl
			nTTL	= pHeader->GetIntegerNumber(nLine,nPos);
			TMASSERT((nTTL>0&&nTTL<0xFFFF));
		}
		if(m_nSDPStream > 0)
		{
			m_SDPStream[(m_nSDPStream-1)]->m_nMediaDestinationsAddr	= nAddr;
			m_SDPStream[(m_nSDPStream-1)]->m_nMediaTimeToLive		= nTTL;
		}else{
			m_SDPHeader.m_nAddress		= nAddr;
			m_SDPHeader.m_nTimeToLive	= nTTL;
		}
		return TRUE;
	}
	return FALSE;
}
BOOL ZSessionDescription::ParseTimes(ZHeaderParser* pHeader,int nLine,int* nPos)
{
	if(pHeader != NULL)
	{
		pHeader->SkipChar(nLine,nPos,'=');
		pHeader->SkipSpace(nLine,nPos);
		//start
		m_SDPHeader.m_nStartTime	= pHeader->GetIntegerNumber(nLine,nPos);
		//end
		pHeader->SkipSpace(nLine,nPos);
		m_SDPHeader.m_nEndTime		= pHeader->GetIntegerNumber(nLine,nPos);

		return TRUE;
	}
	return FALSE;
}
BOOL ZSessionDescription::ParseAttributes(ZHeaderParser* pHeader,int nLine,int* nPos)
{
	CHAR*		sValue	= NULL;

	if(pHeader != NULL)
	{
		pHeader->SkipChar(nLine,nPos,'=');
		pHeader->SkipSpace(nLine,nPos);
		sValue	= pHeader->GetWords(nLine,nPos);
		if(m_nSDPStream > 0)
		{
			if(strcasecmp(sValue,"rtpmap")==0)
			{
				if(strlen(m_SDPStream[m_nSDPStream-1]->m_sMediaPayloadName)==0)
				{
					pHeader->SkipChar(nLine,nPos,':');
					pHeader->SkipSpace(nLine,nPos);
					pHeader->GetDigit(nLine,nPos);
					pHeader->SkipSpace(nLine,nPos);
					strncpy(m_SDPStream[m_nSDPStream-1]->m_sMediaPayloadName,pHeader->GetString(nLine,nPos,' '),MAX_KEY_LABEL);
				}
			}
			if(strcasecmp(sValue,"control")==0)
			{
				if(strlen(m_SDPStream[m_nSDPStream-1]->m_sMediaControlName)==0)
				{
					pHeader->SkipChar(nLine,nPos,':');
					pHeader->SkipSpace(nLine,nPos);

					char sControlName[MAX_KEY_LABEL+4];
					memset(sControlName, 0, sizeof(sControlName));
					strncpy(sControlName,pHeader->GetString(nLine,nPos,'\r'),MAX_KEY_LABEL);
					if (strstr(sControlName, "://"))
					{// control name is absolute path
						strncpy(m_SDPStream[m_nSDPStream-1]->m_sMediaControlName,sControlName,MAX_KEY_LABEL);
						m_SDPStream[m_nSDPStream-1]->m_nMediaTrackID = 0xFFFFFFFF;
					}
					else if (
						(strlen(m_sControlNameBase) > 0)
						&& (strcmp(m_sControlNameBase, "*")!=0)
						)
					{// control name is relative path and the session-level control name point the base path
						char *sTmpName = sControlName;
						if (
							strlen(sTmpName)>0
							&& sTmpName[0] == '/'
							)
						{
							sTmpName ++;
						}

						if (m_sControlNameBase[strlen(m_sControlNameBase)-1] == '/')
						{
							sprintf(m_SDPStream[m_nSDPStream-1]->m_sMediaControlName,"%s%s",m_sControlNameBase, sTmpName);
						}
						else
						{
							sprintf(m_SDPStream[m_nSDPStream-1]->m_sMediaControlName,"%s/%s",m_sControlNameBase, sTmpName);
						}
						m_SDPStream[m_nSDPStream-1]->m_nMediaTrackID = 0xFFFFFFFF;
					}
					else
					{// control name is relative path and the base path should get from content-base or request url
						strncpy(m_SDPStream[m_nSDPStream-1]->m_sMediaControlName,sControlName,MAX_KEY_LABEL);
					}
				}
			}
		}
		else
		{
			if(strcasecmp(sValue,"control")==0)
			{
				pHeader->SkipChar(nLine,nPos,':');
				pHeader->SkipSpace(nLine,nPos);
				strncpy(m_sControlNameBase,pHeader->GetString(nLine,nPos,'\r'),MAX_KEY_LABEL);
			}
		}
		return TRUE;
	}
	return FALSE;
}
BOOL ZSessionDescription::ParseMedia(ZHeaderParser* pHeader,int nLine,int* nPos)
{
	CHAR*		sValue	= NULL;
	INT			nValue	= 0;
	SDP_MEDIA*	pMedia	= NULL;

	if(pHeader != NULL)
	{
		pHeader->SkipChar(nLine,nPos,'=');
		pHeader->SkipSpace(nLine,nPos);

		pMedia	= (SDP_MEDIA*)NEW SDP_MEDIA;
		if(pMedia)
		{
			memset(pMedia,0,sizeof(SDP_MEDIA));
			m_SDPStream.Add(pMedia);

			m_SDPStream[m_nSDPStream]->m_nMediaDestinationsAddr	= m_SDPHeader.m_nAddress;
			m_SDPStream[m_nSDPStream]->m_nMediaTimeToLive		= m_SDPHeader.m_nTimeToLive;
			sValue	= pHeader->GetWords(nLine,nPos);
			if(strcasecmp(sValue,"audio")==0)
			{
				m_SDPStream[m_nSDPStream]->m_nMediaPayloadType	= STREAM_AUDIO;
				pHeader->SkipSpace(nLine,nPos);
				nValue	= pHeader->GetIntegerNumber(nLine,nPos);
				if((nValue>0&&nValue<0xFFFF))
				{
					m_SDPStream[m_nSDPStream]->m_nMediaDestinationsPort	= nValue;
				}
				pHeader->SkipSpace(nLine,nPos);
				sValue	= pHeader->GetString(nLine,nPos,' ');
				m_SDPStream[m_nSDPStream]->m_nMediaTrackID			= (m_nSDPStream+1);
				m_nSDPStream	++;
			}
			if(strcasecmp(sValue,"video")==0)
			{
				m_SDPStream[m_nSDPStream]->m_nMediaPayloadType	= STREAM_VIDEO;
				pHeader->SkipSpace(nLine,nPos);
				nValue	= pHeader->GetIntegerNumber(nLine,nPos);
				if((nValue>0&&nValue<0xFFFF))
				{
					m_SDPStream[m_nSDPStream]->m_nMediaDestinationsPort	= nValue;
				}
				pHeader->SkipSpace(nLine,nPos);
				sValue	= pHeader->GetString(nLine,nPos,' ');
				m_SDPStream[m_nSDPStream]->m_nMediaTrackID			= (m_nSDPStream+1);
				m_nSDPStream	++;
			}
			return TRUE;
		}
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
