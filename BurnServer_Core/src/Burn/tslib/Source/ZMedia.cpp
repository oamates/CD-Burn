#include "ZMedia.h"
#include "TSMedia.h"
#include "ZOSMemory.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const ZMedia::MEDIA_FACTORY_DATA ZMedia::m_MediaFactory[]	=
{
	{0}
	//MEDIA_INSTANCE_REGIST(ASFMedia)
	//MEDIA_INSTANCE_REGIST(TSMedia)
};
const int ZMedia::m_nMediaFactory	= COUNT_OF(m_MediaFactory);

ZMediaArray ZMedia::m_ZMediaArray(200);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZMedia::ZMedia()
:ZObject()
,m_eMediaType(MEDIA_UNKNOWN)
,m_nMediaError(0)
,m_nUseCount(0)
{
	memset(m_sMediaURL,0,sizeof(m_sMediaURL));
}
ZMedia::~ZMedia()
{
	ZMedia::Close();
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZMedia::Open(const char* sFileName)
{
	if(sFileName != NULL)
	{
		strncpy(m_sMediaURL,sFileName,MAX_URI_PATH);
		return TRUE;
	}
	return FALSE;
}
BOOL ZMedia::Create(const char* sFileName,const char* sFileType)
{
	if(sFileName != NULL)
	{
		strncpy(m_sMediaURL,sFileName,MAX_URI_PATH);
		return TRUE;
	}
	return FALSE;
}
BOOL ZMedia::Close()
{
	memset(m_sMediaURL,0,sizeof(m_sMediaURL));
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
MEDIA_TYPE ZMedia::GetMediaType() CONST
{
	return(m_eMediaType);
}
CONST CHAR* ZMedia::GetMediaURL() CONST
{
	return(m_sMediaURL);
}
int ZMedia::GetMediaError() CONST
{
	return(m_nMediaError);
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZMedia::IsMediaFile()
{
	return ((m_eMediaType > MEDIA_FILE_START) && (m_eMediaType < MEDIA_FILE_END));
}
BOOL ZMedia::IsMediaList()
{
	return ((m_eMediaType > MEDIA_LIST_START) && (m_eMediaType < MEDIA_LIST_END));
}
///////////////////////////////////////////////////////////////////////////////
MEDIA_TYPE ZMedia::SetMediaType(MEDIA_TYPE eType)	
{
	m_eMediaType=eType;
	return(m_eMediaType);
}
int ZMedia::SetMediaError(int nError)
{
	m_nMediaError=nError;
	return(m_nMediaError);
}
///////////////////////////////////////////////////////////////////////////////
ZMedia*	ZMedia::OpenMedia(const char* sFileName,const char* sFileType)
{
	ZMedia*	pMedia	= NULL;

	int	i	= 0;

	if(strncasecmp(sFileType,"MP4",strlen(sFileType)) == 0)
	{
		for(i = 0; i < m_ZMediaArray.Size(); i++)
		{
			MESSAGE_OUT(("[ZMedia::OpenMedia] %s %s\r\n", m_ZMediaArray[i]->GetMediaURL(), sFileName));
			if(strncasecmp(m_ZMediaArray[i]->GetMediaURL(),sFileName,strlen(sFileName)) == 0)
			{
				m_ZMediaArray[i]->m_nUseCount++;
				pMedia = m_ZMediaArray[i];
				break;
			}
		}
	}

	if(pMedia != NULL)
	{
		return pMedia;
	}
	for(i = 0; i < m_nMediaFactory; i ++)
	{
		pMedia	= m_MediaFactory[i].m_pOpenMedia(sFileName,sFileType);
		if(pMedia != NULL)
		{
			if(strncasecmp(sFileType,"MP4",strlen(sFileType)) == 0)
			{
				pMedia->m_nUseCount++;
				m_ZMediaArray.Add(pMedia);
			}
			break;
		}
	}

	return pMedia;
}
///////////////////////////////////////////////////////////////////////////////
ZMedia*	ZMedia::CreateMedia(const char* sFileName,const char* sFileType)
{
	ZMedia*	pMedia	= NULL;

	int	i	= 0;

	for(i = 0; i < m_nMediaFactory; i ++)
	{
		pMedia	= m_MediaFactory[i].m_pCreateMedia(sFileName,sFileType);
		if(pMedia != NULL)
		{
			break;
		}
	}

	return pMedia;
}
ZMedia*	ZMedia::CloseMedia(ZMedia* pMedia)
{
	if(pMedia != NULL)
	{
		if (pMedia->GetMediaType() == MEDIA_MP4)
		{
			if (pMedia->m_nUseCount > 0)
			{
				pMedia->m_nUseCount --;
			}
			if (pMedia->m_nUseCount == 0)
			{
				for(int i = 0; i <ZMedia::m_ZMediaArray.Size(); i++)
				{
					if(pMedia == ZMedia::m_ZMediaArray[i])
					{
						ZMedia::m_ZMediaArray.Remove(i);
						break;
					}
				}
				pMedia->Close();
				SAFE_DELETE(pMedia);
			}
		}
		else
		{
			pMedia->Close();
			SAFE_DELETE(pMedia);
		}
	}

	return pMedia;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
