#include "ZAuthorization.h"
#include "ZOS.h"
#include "ZMD5.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_AUTHORIZATION_BASIC_FORMAT		"WWW-Authenticate: Basic realm=\"%s\""
#define	DEFAULT_AUTHORIZATION_DIGEST_FORMAT		"WWW-Authenticate: Digest realm=\"%s\",nonce=\"%s\""
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZAuthorization::ZAuthorization()
:m_bAuthorizationFlag(FALSE)
,m_bAuthenticationFlag(FALSE)
,m_nAuthorizationType(AUTH_NULL)
{
	memset(m_sUser,0,sizeof(m_sUser));
	memset(m_sPassword,0,sizeof(m_sPassword));

	memset(m_sAuthorizationRealm,0,sizeof(m_sAuthorizationRealm));
	memset(m_sAuthorizationNonce,0,sizeof(m_sAuthorizationNonce));
	memset(m_sAuthenticationMethod,0,sizeof(m_sAuthenticationMethod));
	memset(m_sAuthenticationURL,0,sizeof(m_sAuthenticationURL));

	memset(m_sAuthorizationData,0,sizeof(m_sAuthorizationData));

	memset(&m_AuthorizationSeedData,0,sizeof(m_AuthorizationSeedData));

	ZAuthorization::SetRealmNonce("/",NULL);
}
ZAuthorization::~ZAuthorization()
{
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZAuthorization::Authorization()
{
	if(!m_bAuthorizationFlag)
	{
		ZAuthorization::GetBasicAuthentication();
		ZAuthorization::GetDigestAuthentication();
		m_bAuthorizationFlag	= TRUE;
		return m_bAuthorizationFlag;
	}
	return FALSE;
}
BOOL ZAuthorization::Authentication()
{
	if(m_nAuthorizationType!=0)
	{
		return m_bAuthenticationFlag;
	}
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZAuth::AUTH_TYPE ZAuthorization::GetAuthType()
{
	return m_nAuthorizationType;
}
ZAuth::AUTH_TYPE ZAuthorization::SetAuthType(AUTH_TYPE nType)
{
	m_nAuthorizationType	= nType;
	return m_nAuthorizationType;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZAuthorization::GetAuthData(CHAR* sAuth,UINT* nAuth)
{
	BOOL	bReturn		= FALSE;

	if(sAuth != NULL && nAuth != NULL)
	{
		if(((UINT)*nAuth) > ((UINT)strlen(m_sAuthorizationData)))
		{
			strcpy(sAuth,m_sAuthorizationData);
			bReturn	= TRUE;
		}
		*nAuth	= strlen(m_sAuthorizationData);
	}

	return bReturn;
}
BOOL ZAuthorization::CheckAuthData(CHAR* sAuth,UINT nAuth)
{
	m_bAuthorizationFlag	= FALSE;
	m_bAuthenticationFlag	= TRUE;
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZAuthorization::SetUserNamePassword(CHAR* sUser,CHAR* sPassword)
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
BOOL ZAuthorization::SetRealmNonce(CHAR* sRealm,CHAR* sNonce)
{
	CHAR	sLocalBase[16];

	memset(m_sAuthorizationRealm,0,sizeof(m_sAuthorizationRealm));
	memset(m_sAuthorizationNonce,0,sizeof(m_sAuthorizationNonce));

	if(sRealm != NULL)
	{
		snprintf(m_sAuthorizationRealm,MAX_KEY_LABEL,"%s",sRealm);
		if(sNonce != NULL)
		{
			snprintf(m_sAuthorizationNonce,MAX_KEY_LABEL,"%s",sNonce);
		}else{
			ZMD5	md5;
			m_AuthorizationSeedData.nTime	= ZOS::milliseconds();
			m_AuthorizationSeedData.nCount	++;
			memcpy(sLocalBase,md5.MD5Memory((BYTE*)&m_AuthorizationSeedData,sizeof(m_AuthorizationSeedData)),16);
			binarytolowerstring(sLocalBase,16,m_sAuthorizationNonce,sizeof(m_sAuthorizationNonce));

		}
		return TRUE;
	}

	return FALSE;
}
BOOL ZAuthorization::SetMethodURL(CHAR* sMethod,CHAR* sURL)
{
	memset(m_sAuthenticationMethod,0,sizeof(m_sAuthenticationMethod));
	memset(m_sAuthenticationURL,0,sizeof(m_sAuthenticationURL));

	if(sMethod != NULL && sURL != NULL)
	{
		snprintf(m_sAuthenticationMethod,MAX_KEY_LABEL,"%s",sMethod);
		snprintf(m_sAuthenticationURL,MAX_KEY_LABEL,"%s",sURL);
		return TRUE;
	}

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZAuthorization::GetBasicAuthentication()
{
	if(m_nAuthorizationType == AUTH_BASIC)
	{
		sprintf(m_sAuthorizationData,DEFAULT_AUTHORIZATION_BASIC_FORMAT,m_sAuthorizationRealm);
		return TRUE;
	}
	return FALSE;
}
BOOL ZAuthorization::GetDigestAuthentication()
{
	if(m_nAuthorizationType == AUTH_DIGEST)
	{
		sprintf(m_sAuthorizationData,DEFAULT_AUTHORIZATION_DIGEST_FORMAT,m_sAuthorizationRealm,m_sAuthorizationNonce);
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
