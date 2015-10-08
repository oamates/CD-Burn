#include "ZAuthenticator.h"
#include "ZOS.h"
#include "ZMath.h"
#include "ZMD5.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_AUTHENTICATION_BASIC_FORMAT		"Authorization: Basic %s"
#define	DEFAULT_AUTHENTICATION_DIGEST_FORMAT	"Authorization: Digest "\
												"username=\"%s\", realm=\"%s\", "\
												"nonce=\"%s\", uri=\"%s\", response=\"%s\""
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZAuthenticator::ZAuthenticator()
:m_bAuthenticationFlag(FALSE)
,m_nAuthenticationType(AUTH_NULL)
{
	memset(m_sUser,0,sizeof(m_sUser));
	memset(m_sPassword,0,sizeof(m_sPassword));

	memset(m_sAuthenticationRealm,0,sizeof(m_sAuthenticationRealm));
	memset(m_sAuthenticationNonce,0,sizeof(m_sAuthenticationNonce));
	memset(m_sAuthenticationMethod,0,sizeof(m_sAuthenticationMethod));
	memset(m_sAuthenticationURL,0,sizeof(m_sAuthenticationURL));


	memset(m_sAuthenticationData,0,sizeof(m_sAuthenticationData));
}
ZAuthenticator::~ZAuthenticator()
{
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZAuthenticator::Authorization()
{
	if(!m_bAuthenticationFlag)
	{
		return (m_nAuthenticationType!=AUTH_NULL);
	}
	return FALSE;
}
BOOL ZAuthenticator::Authentication()
{
	if(m_bAuthenticationFlag)
	{
		ZAuthenticator::GetDigestAuthentication();
	}else{
		ZAuthenticator::GetBasicAuthentication();
		ZAuthenticator::GetDigestAuthentication();

		m_bAuthenticationFlag	= (m_nAuthenticationType!=AUTH_NULL);
	}
	return m_bAuthenticationFlag;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZAuth::AUTH_TYPE ZAuthenticator::GetAuthType()
{
	return m_nAuthenticationType;
}
ZAuth::AUTH_TYPE ZAuthenticator::SetAuthType(AUTH_TYPE nType)
{
	m_nAuthenticationType	= nType;

	return m_nAuthenticationType;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZAuthenticator::GetAuthData(CHAR* sAuth,UINT* nAuth)
{
	BOOL	bReturn		= FALSE;

	if(m_bAuthenticationFlag)
	{
		if(sAuth != NULL && nAuth != NULL)
		{
			if(((UINT)*nAuth) > ((UINT)strlen(m_sAuthenticationData)))
			{
				strcpy(sAuth,m_sAuthenticationData);
				bReturn	= TRUE;
			}
			*nAuth	= strlen(m_sAuthenticationData);
		}
	}
	return bReturn;
}
BOOL ZAuthenticator::CheckAuthData(CHAR* sAuth,UINT nAuth)
{
	if(sAuth != NULL && nAuth > 0)
	{
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZAuthenticator::SetUserNamePassword(CHAR* sUser,CHAR* sPassword)
{
	memset(m_sUser,0,sizeof(m_sUser));
	memset(m_sPassword,0,sizeof(m_sPassword));

	if(sUser != NULL)
	{
		snprintf(m_sUser,MAX_KEY_LABEL,"%s",sUser);
	}
	if(sPassword != NULL)
	{
		snprintf(m_sPassword,MAX_KEY_LABEL,"%s",sPassword);
	}

	return TRUE;
}
BOOL ZAuthenticator::SetRealmNonce(CHAR* sRealm,CHAR* sNonce)
{
	memset(m_sAuthenticationRealm,0,sizeof(m_sAuthenticationRealm));
	memset(m_sAuthenticationNonce,0,sizeof(m_sAuthenticationNonce));

	if(sRealm != NULL)
	{
		snprintf(m_sAuthenticationRealm,MAX_KEY_LABEL,"%s",sRealm);
		if(sNonce != NULL)
		{
			snprintf(m_sAuthenticationNonce,MAX_KEY_LABEL,"%s",sNonce);
		}
		return TRUE;
	}

	return FALSE;
}
BOOL ZAuthenticator::SetMethodURL(CHAR* sMethod,CHAR* sURL)
{
	memset(m_sAuthenticationMethod,0,sizeof(m_sAuthenticationMethod));
	memset(m_sAuthenticationURL,0,sizeof(m_sAuthenticationURL));

	if(sMethod != NULL && sURL != NULL)
	{
		snprintf(m_sAuthenticationMethod,MAX_KEY_LABEL,"%s",sMethod);
		snprintf(m_sAuthenticationURL,MAX_KEY_LABEL,"%s",sURL);
		ZAuthenticator::GetBasicAuthentication();
		ZAuthenticator::GetDigestAuthentication();

		return TRUE;
	}

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZAuthenticator::GetBasicAuthentication()
{
	CHAR	sLocalData[MAX_KEY_LABEL*2+4];
	CHAR	sLocalBase[MAX_KEY_LABEL*4+4];

	if(m_nAuthenticationType == AUTH_BASIC)
	{
		sprintf(sLocalData,"%s:%s",m_sUser,m_sPassword);
        //LOG_DEBUG(("[ZAuthenticator::GetBasicAuthentication] user %s pass %s\r\n",
        //    m_sUser, m_sPassword));
		Common_Base64Encode(sLocalData,strlen(sLocalData),sLocalBase,MAX_KEY_LABEL*4);
		sprintf(m_sAuthenticationData,DEFAULT_AUTHENTICATION_BASIC_FORMAT,sLocalBase);
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
//MD5(MD5(<username>:<realm>:<password>):<nonce>:MD5(<method>:<url>))
///////////////////////////////////////////////////////////////////////////////
BOOL ZAuthenticator::GetDigestAuthentication()
{
	CHAR	sLocalData[MAX_KEY_LABEL*4+4];
	CHAR	sLocalBase[MAX_KEY_LABEL+4];
	CHAR	sLocalString1[MAX_KEY_LABEL+4];
	CHAR	sLocalString2[MAX_KEY_LABEL+4];
	CHAR	sLocalString3[MAX_KEY_LABEL+4];
	ZMD5	md5;

	if(m_nAuthenticationType == AUTH_DIGEST)
	{
		sprintf(sLocalData,"%s:%s:%s",m_sUser,m_sAuthenticationRealm,m_sPassword);
		memcpy(sLocalBase,md5.MD5String(sLocalData),16);
		binarytolowerstring(sLocalBase,16,sLocalString1,MAX_KEY_LABEL);
		printf("%s\r\n",sLocalString1);
		sprintf(sLocalData,"%s:%s",m_sAuthenticationMethod,m_sAuthenticationURL);
		memcpy(sLocalBase,md5.MD5String(sLocalData),16);
		binarytolowerstring(sLocalBase,16,sLocalString2,MAX_KEY_LABEL);
		printf("%s\r\n",sLocalString2);
		sprintf(sLocalData,"%s:%s:%s",sLocalString1,m_sAuthenticationNonce,sLocalString2);
		memcpy(sLocalBase,md5.MD5String(sLocalData),16);
		binarytolowerstring(sLocalBase,16,sLocalString3,MAX_KEY_LABEL);
		printf("%s\r\n",sLocalString3);
		sprintf(m_sAuthenticationData,DEFAULT_AUTHENTICATION_DIGEST_FORMAT,m_sUser,m_sAuthenticationRealm,m_sAuthenticationNonce,m_sAuthenticationURL,sLocalString3);

		return TRUE;
	}

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
