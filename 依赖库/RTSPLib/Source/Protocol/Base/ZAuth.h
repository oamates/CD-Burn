#ifndef	_ZAUTH_HEADER_
#define	_ZAUTH_HEADER_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ZAuth
{
///////////////////////////////////////////////////////////////////////////////
public: 
	enum	AUTH_TYPE
	{
		AUTH_NULL	= 0,
		AUTH_BASIC,
		AUTH_DIGEST,
	};
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	Authorization()										= 0;
	virtual	BOOL	Authentication()									= 0;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	AUTH_TYPE	GetAuthType()									= 0;
	virtual	AUTH_TYPE	SetAuthType(AUTH_TYPE nType)					= 0;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	GetAuthData(CHAR* sAuth,UINT* nAuth)				= 0;
	virtual	BOOL	CheckAuthData(CHAR* sAuth,UINT nAuth)				= 0;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	SetUserNamePassword(CHAR* sUser,CHAR* sPassword)	= 0;
	virtual	BOOL	SetRealmNonce(CHAR* sRealm,CHAR* sNonce)			= 0;
	virtual	BOOL	SetMethodURL(CHAR* sMethod,CHAR* sURL)				= 0;
///////////////////////////////////////////////////////////////////////////////
inline char* binarytolowerstring(char* s,int m,char* t,int n)
{
	static	const	char	shex[]	= "0123456789abcdef";
	int	i	= 0;

	if(s != NULL && t != NULL && n > (2 * m))
	{
		for(i = 0; i < m; i ++)
		{
			t[i+i]		= shex[((s[i]&0xF0)>>4)];
			t[i+i+1]	= shex[(s[i]&0x0F)];
		}
		t[m+m]	= 0;
	}
	return NULL;
}
inline char* binarytoupperstring(char* s,int m,char* t,int n)
{
	static	const	char	shex[]	= "0123456789ABCDEF";
	int	i	= 0;

	if(s != NULL && t != NULL && n > (2 * m))
	{
		for(i = 0; i < m; i ++)
		{
			t[i+i]		= shex[((s[i]&0xF0)>>4)];
			t[i+i+1]	= shex[(s[i]&0x0F)];
		}
		t[m+m]	= 0;
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
};
#endif	//_ZAUTH_HEADER_
