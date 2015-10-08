#include "ZOS.h"
#include "ZOSThread.h"
#include "ZOSMutex.h"
#include "ZOSMemory.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include	<math.h>
#ifdef _WIN32_
//header
#include	<time.h>
#include	<mmsystem.h>
#include	<process.h>
#include	<io.h>
#include	<direct.h>
#include    <sys/types.h>
#include    <sys/timeb.h>
#include    <Winsock2.h>
//lib
#ifndef	_MINGW_
#pragma comment(lib,"winmm.lib")
#endif	//_MINGW_
#endif	//_WIN32_
#ifndef _WIN32_
//header
#include	<unistd.h>
#include	<sys/time.h>
#include	<net/if.h>
#include	<sys/ioctl.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#ifndef	_MINGW_
#include	<pwd.h>
#endif	//_MINGW_
//lib
#endif	//_WIN32_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSMutex*	ZOS::m_sSystemMutex		= NULL;
ZOSMutex*	ZOS::m_sLastMillisMutex	= NULL;
ZOSMutex*	ZOS::m_sLastMicroMutex	= NULL;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
INT64		ZOS::m_sMsecSince1970	= 0;
INT64		ZOS::m_sInitialMsec		= 0;
INT64		ZOS::m_sInitialTime		= 0;
INT64		ZOS::m_sCompareWrap		= 0;
INT64		ZOS::m_sWrapMsec		= 0;
INT64		ZOS::m_sLastMsecMilli	= 0;
///////////////////////////////////////////////////////////////////////////////
UINT64		ZOS::m_nTimeFreq		= 0;
INT64		ZOS::m_nLastMicroSec	= 0;
///////////////////////////////////////////////////////////////////////////////
const char *ZOS::smonths[12]	=
{	
	"Jan",	//1
	"Feb",	//2
	"Mar",	//3
	"Apr",	//4
	"May",	//5
	"Jun",	//6
	"Jul",	//7
	"Aug",	//8
	"Sep",	//9
	"Oct",	//10
	"Nov",	//11
	"Dec"	//12
};
const char *ZOS::sweeks[7]	=
{
	"Sun",	//1
	"Mon",	//2
	"Tue",	//3
	"Wed",	//4
	"Thu",	//5
	"Fri",	//6
	"Sat",	//7
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ZOSBaseInitialize()
{
	ZOS::Initialize();
	ZOSThread::Initialize();
}
void ZOSBaseUninitialize()
{
	ZOSThread::Uninitialize();
	ZOS::Uninitialize();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ZOS::Initialize()
{
	//
	TMASSERT((m_sSystemMutex==NULL));
	if(m_sSystemMutex == NULL)
	{
		m_sSystemMutex	= NEW ZOSMutex("SystemMutex");
	}
	//
	TMASSERT((m_sInitialMsec==0));
	if(m_sInitialMsec == 0)
	{
		::tzset();
		m_sWrapMsec			= ((INT64) 0x00000001 << 32);
		m_sCompareWrap		= ((INT64) 0xffffffff << 32);
		m_sLastMsecMilli	= 0;
		m_sInitialMsec		= ZOS::milliseconds();
		m_sInitialTime		= ZOS::systemtime();
		m_sMsecSince1970	= ::time(NULL);
		m_sMsecSince1970	*= 1000;

		TMASSERT((m_sLastMillisMutex==NULL));
		if(m_sLastMillisMutex == NULL)
		{
			m_sLastMillisMutex	= NEW ZOSMutex("LastMillisMutex");
		}
	}

	TMASSERT((m_sLastMicroMutex==NULL));
	if(m_sLastMicroMutex == NULL)
	{
		m_sLastMicroMutex	= NEW ZOSMutex("LastMicroMutex");
	}

#ifdef _WIN32_
	LARGE_INTEGER tFreqTick;
	LARGE_INTEGER tLastTick;
	QueryPerformanceFrequency(&tFreqTick);
	QueryPerformanceCounter(&tLastTick);

	m_nTimeFreq = tFreqTick.QuadPart;
	m_nLastMicroSec = tLastTick.QuadPart*1000000/m_nTimeFreq;
#else
	struct timeval t;
	::gettimeofday(&t,NULL);
	m_nLastMicroSec = t.tv_sec*1000000 + t.tv_usec;
#endif
}
void ZOS::Uninitialize()
{
	m_sInitialMsec	= 0;

	SAFE_DELETE(m_sLastMicroMutex);

	SAFE_DELETE(m_sLastMillisMutex);

	SAFE_DELETE(m_sSystemMutex);
}
///////////////////////////////////////////////////////////////////////////////
unsigned int ZOS::systeminit(unsigned int* area,unsigned int val,int newval)
{
    ZOSMutexLocker	locker(m_sSystemMutex);
	if(area != NULL)
	{
		if(val == *area)
		{
			*area	= newval;
			return *area;
		}
	}
	return 0;
}
unsigned int ZOS::systemadd(unsigned int* area,int val)
{
    ZOSMutexLocker	locker(m_sSystemMutex);
	if(area != NULL)
	{
		*area	+= val;
		return *area;
	}
	return 0;
}
unsigned int ZOS::systemsub(unsigned int* area,int val)
{
	ZOSMutexLocker	locker(m_sSystemMutex);
	if(area != NULL)
	{
		*area	-= val;
		return *area;
	}
	return 0;
}
unsigned int ZOS::systemor(unsigned int* area,int val)
{
	unsigned	int	oval	= 0;

    ZOSMutexLocker	locker(m_sSystemMutex);
	if(area != NULL)
	{
		oval	= *area;
		oval	|= val;
		*area	= oval;
		return *area;
	}
	return 0;
}
unsigned int ZOS::systemand(unsigned int* area,int val)
{
	unsigned	int	oval	= 0;

    ZOSMutexLocker	locker(m_sSystemMutex);
	if(area != NULL)
	{
		oval	= *area;
		oval	&= val;
		*area	= oval;
		return *area;
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
#ifndef _WIN32_
#ifdef USE_SHADOW
#include	<shadow.h>
#endif	//USE_SHADOW
#endif	//_WIN32_
///////////////////////////////////////////////////////////////////////////////
int ZOS::checkuser(const char* sUser,const char* sPassword)
{
	if(sUser != NULL && sPassword != NULL)
	{
#if	defined(_WIN32_)||defined(_MINGW_)
		HANDLE	hToken	= NULL;

		if(LogonUser((CHAR*)sUser,NULL,(CHAR*)sPassword,LOGON32_LOGON_INTERACTIVE,LOGON32_PROVIDER_DEFAULT,&hToken))
		{
			::CloseHandle(hToken);
			return 0;
		}
#else
		struct	passwd*	puw		= ::getpwnam(sUser);
#ifdef	USE_SHADOW
		struct spwd*	spwd	= ::getspnam(sUser);;
		if(spwd != NULL)
		{
			puw->pw_passwd	= spwd->sp_pwdp;
		}
#endif	//USE_SHADOW

		if(puw != NULL)
		{
			if(puw->pw_passwd[0]=='\0')
			{
				return 0;
			}
#ifdef	USE_SHADOW
			if(strcmp(puw->pw_passwd,crypt(sPassword,puw->pw_passwd))==0)
#else
			if(strcmp(puw->pw_passwd,sPassword)==0)
#endif	//USE_SHADOW
			{
				return 0;
			}else{
#ifdef	USE_SHADOW
				//printf("ZOS::checkuser ERROR(SHADOW %s %s)\r\n",sUser,sPassword);
#else
				//printf("ZOS::checkuser ERROR(%s %s)\r\n",sUser,sPassword);
#endif	//USE_SHADOW
			}
		}
#endif	//_WIN32_
		return 1;
	}
	return -1;
}
///////////////////////////////////////////////////////////////////////////////
UINT64	ZOS::milliseconds()
{
#if	defined(_WIN32_)
    //use QueryPerformanceFrequency make sure second value not depends on current system time
    /*
    ZOSMutexLocker	locker(m_sLastMillisMutex);

    INT64	sCurrentMsecMilli	= (UINT) ::timeGetTime() + (m_sLastMsecMilli & m_sCompareWrap);
    if((sCurrentMsecMilli - m_sLastMsecMilli) < 0)
    {
        sCurrentMsecMilli	+= m_sWrapMsec;
    }
    m_sLastMsecMilli	= sCurrentMsecMilli;
    
 
    return ((sCurrentMsecMilli - m_sInitialMsec) + m_sMsecSince1970);
    */
    LARGE_INTEGER   tCurrentTick;

    if (QueryPerformanceCounter(&tCurrentTick))
    {
        if (m_nTimeFreq != 0)
        {
            return (tCurrentTick.QuadPart*1000)/m_nTimeFreq;
        }
    }

    return 0;

#else
    /*
	struct timeval	t;
	int	nError	= ::gettimeofday(&t,NULL);
	TMASSERT((nError == 0));

	INT64	sCurrentMsec;
	sCurrentMsec	= t.tv_sec;
	sCurrentMsec	*= 1000;
	sCurrentMsec	+= (t.tv_usec / 1000);

	return ((sCurrentMsec - m_sInitialMsec) + m_sMsecSince1970);
    */
    struct timespec	ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    INT64	sCurrentMsec;
    sCurrentMsec	= ts.tv_sec;
    sCurrentMsec	*= 1000;
    sCurrentMsec	+= (ts.tv_nsec / 1000000);

    return sCurrentMsec;
#endif	//(_WIN32_)
}
///////////////////////////////////////////////////////////////////////////////
INT64 ZOS::microseconds()
{
#ifdef _WIN32_
	LARGE_INTEGER tLastTick;
	QueryPerformanceCounter(&tLastTick);

	if (m_nTimeFreq != 0)
	{
		//INT64 temp = tLastTick.QuadPart*1000000/m_nTimeFreq;
		//MESSAGE_OUT(("[ZOS::microseconds] (%"F_NUM_64"d)\r\n", temp));
		//return temp;
		return tLastTick.QuadPart*1000000/m_nTimeFreq;
	}
	else
	{
		return 0;
	}
#else
    /*
	struct timeval t;
	::gettimeofday(&t,NULL);
	return t.tv_sec*1000000 + t.tv_usec;
    */
    struct timespec	ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec*1000000 + ts.tv_nsec / 1000;
#endif
}
///////////////////////////////////////////////////////////////////////////////
INT64 ZOS::getlastmicrosec()
{
	return m_nLastMicroSec;
}
void ZOS::setlastmicrosec(INT64 microsec)
{
	ZOSMutexLocker	locker(m_sLastMicroMutex);

	m_nLastMicroSec = microsec;
}
void ZOS::sleepmicrosec(int microsec)
{
#ifdef _WIN32_
	INT64 s;
	INT64 c;

	LARGE_INTEGER tLastTick;
	QueryPerformanceCounter(&tLastTick);
	s = c = (tLastTick.QuadPart*1000000/m_nTimeFreq);
	s += microsec;
	while(c < s)
	{
		__asm{nop}
		QueryPerformanceCounter(&tLastTick);
		c = (tLastTick.QuadPart*1000000/m_nTimeFreq);
	}

#endif

#ifdef _LINUX_
	struct timeval t;

	UINT64 cstart;
	UINT64 s;
	UINT64 c;
	
	UINT64 tmp;
	
	if (microsec < 0)
	{
		return;
	}

	if (::gettimeofday(&t,NULL) == 0)
	{
		tmp = t.tv_sec;
		tmp = tmp*1000000;
		tmp = tmp+t.tv_usec;
		cstart = s = c = tmp;
		
		s += microsec;
		
		while(c < s && cstart <= c)
		{
			asm("nop");
			if (::gettimeofday(&t,NULL) == 0)
			{
				cstart = c;

				tmp = t.tv_sec;
				tmp = tmp*1000000;
				tmp = tmp+t.tv_usec;
				
				c = tmp;
			}
			else
			{
				break;
			}
		}
	}
#endif
}
int ZOS::gettimeofdaycross(struct timeval* tp, int*)
{
#ifdef _WIN32_
    static LARGE_INTEGER tickFrequency, epochOffset;

    // For our first call, use "ftime()", so that we get a time with a proper epoch.
    // For subsequent calls, use "QueryPerformanceCount()", because it's more fine-grain.
    static bool isFirstCall = true;

    LARGE_INTEGER tickNow;
    QueryPerformanceCounter(&tickNow);

    if (isFirstCall)
    {
        struct timeb tb;
        ftime(&tb);
        tp->tv_sec = (long)tb.time;
        tp->tv_usec = 1000*tb.millitm;

        // Also get our counter frequency:
        QueryPerformanceFrequency(&tickFrequency);

        // And compute an offset to add to subsequent counter times, so we get a proper epoch:
        epochOffset.QuadPart
            = tb.time*tickFrequency.QuadPart + (tb.millitm*tickFrequency.QuadPart)/1000 - tickNow.QuadPart;

        isFirstCall = false; // for next time
    }
    else
    {
        // Adjust our counter time so that we get a proper epoch:
        tickNow.QuadPart += epochOffset.QuadPart;

        tp->tv_sec = (long) (tickNow.QuadPart / tickFrequency.QuadPart);
        tp->tv_usec = (long) (((tickNow.QuadPart % tickFrequency.QuadPart) * 1000000L) / tickFrequency.QuadPart);
    }
    return 0;
#else
    return ::gettimeofday(tp,NULL);
#endif//_WIN32_
}
///////////////////////////////////////////////////////////////////////////////
UINT64 ZOS::currenttime(time_t* ptimep)
{
	time_t	t	= 0;

	t	= ::time((time_t*)ptimep);

	return((UINT64)t);
}
///////////////////////////////////////////////////////////////////////////////
UINT64 ZOS::systemtime(time_t *ptimep)
{
	time_t	t	= 0;


	if(ptimep == NULL)
	{
		INT64	sCurrentMsec	= 0;
#if	defined(_WIN32_)
#if	_WIN32_WINNT > 0x0600 
		sCurrentMsec	= ::GetTickCount64();
#else
		sCurrentMsec	= ::GetTickCount();
#endif	//_WIN32_WINNT
#else
#ifdef	_POSIX_MONOTONIC_CLOCK
		struct timespec	tv;
		int	nError	= ::clock_gettime(CLOCK_MONOTONIC,&tv);
		TMASSERT((nError == 0));
        if (nError != 0)
        {
            //
        }
        

		sCurrentMsec	= tv.tv_sec;
		sCurrentMsec	*= 1000;
		sCurrentMsec	+= (tv.tv_nsec / 1000000);
#else
	struct timeval	tv;
	int	nError	= ::gettimeofday(&tv,NULL);
	TMASSERT((nError == 0));

	sCurrentMsec	= tv.tv_sec;
	sCurrentMsec	*= 1000;
	sCurrentMsec	+= (tv.tv_usec / 1000);
#endif	//_POSIX_MONOTONIC_CLOCK
#endif	//defined(_WIN32_)
		t		= (sCurrentMsec - m_sInitialTime);
	}else{
		t		= (*ptimep - m_sInitialTime);
	}

	return((UINT64)t);
}
///////////////////////////////////////////////////////////////////////////////
char* ZOS::datetime(const char* sformat,const time_t *ptimep,char* sdatetime,int* ndatetime)
{
	static	char	sdefaultdatetime[256];
	static	int		ndefaultdatetime= 256;
	static	char*	sdefaultformat	= (char*)DEFAULT_DATETIME_FORMAT;
	struct	tm		*p				= NULL;

	if(sformat == 0)
	{
		sformat	= sdefaultformat;
	}
	if(ptimep == 0)
	{
		time_t		timep;
		time(&timep);
		p	= localtime(&timep);
	}else{
		p	= localtime(ptimep);
	}
	if(sdatetime == 0)
	{
		sdatetime	= sdefaultdatetime;
		ndatetime	= &ndefaultdatetime;
	}
	if(p != 0 && sdatetime != 0 && ndatetime != 0 && *ndatetime > (int)(strlen(sformat) + 64))
	{
		sprintf(sdatetime,sformat,(p->tm_year+1900),(p->tm_mon+1),(p->tm_mday),(p->tm_hour),(p->tm_min),(p->tm_sec));
		*ndatetime	= strlen(sdatetime);
		return sdatetime;
	}

	LOG_ERROR(("ErrorCode:%s [ZOS::datetime] 1 parameter is not right!\r\n", GetErrorCodeString(20001)));
	return 0;
}

char* ZOS::datetime(const char* sformat,const char *sdatetimedata,const char *sdatetimeformat,char* sdatetime,int* ndatetime)
{
	static char		sdefaultdatetime[256];
	static int		ndefaultdatetime	= 256;

	if(sdatetimedata != 0 && sdatetimeformat != 0)
	{
		char*		sdefaultformat	= (char*)DEFAULT_DATETIME_FORMAT;
		int			i				= 0;
		struct tm	tm;

		if(sformat == 0)
		{
			sformat	= sdefaultformat;
		}
		if(sdatetime == 0)
		{
			sdatetime	= sdefaultdatetime;
			ndatetime	= &ndefaultdatetime;
		}
		if(sdatetime != 0 && ndatetime != 0 && *ndatetime > (int)(strlen(sformat) + 64))
		{
			if(sdatetimedata[0] >= '0' && sdatetimedata[0] <= '9')
			{
				sscanf(sdatetimedata,sdatetimeformat,&tm.tm_year,&tm.tm_mon,&tm.tm_mday,&tm.tm_hour,&tm.tm_min,&tm.tm_sec);
			}else{
				sscanf(sdatetimedata,sdatetimeformat,sdatetime,&tm.tm_mday,&tm.tm_year,&tm.tm_hour,&tm.tm_min,&tm.tm_sec);
				for(i = 0; i < 12; i ++)
				{
					if(strcmp(smonths[i],sdatetime) == 0)
					{
						tm.tm_mon	= (i + 1);
					}
				}
			}
			sprintf(sdatetime,sformat,(tm.tm_year),(tm.tm_mon),(tm.tm_mday),(tm.tm_hour),(tm.tm_min),(tm.tm_sec));
			*ndatetime	= strlen(sdatetime);
			return sdatetime;
		}
	}

	LOG_ERROR(("ErrorCode:%s [ZOS::datetime] 2 parameter is not right!\r\n", GetErrorCodeString(20001)));
	return 0;
}
char* ZOS::systemdatetime(const char* sformat,const time_t *ptimep,char* sdatetime,int* ndatetime)
{
	static	char		sdefaultdatetime[256];
	static	int			ndefaultdatetime	= 256;
			time_t		t					= 0;
	struct	tm			tm					= {0};

	if(sformat == 0)
	{
		sformat	= (char*)DEFAULT_SYSTEM_DATETIME_FORMAT;
	}
	if(ptimep == 0)
	{
		t			= (ZOS::systemtime()/1000);
	}else{
		t			= (*ptimep/1000);
	}
	if(sdatetime == 0)
	{
		sdatetime	= sdefaultdatetime;
		ndatetime	= &ndefaultdatetime;
	}
	if(sdatetime != 0 && ndatetime != 0 && *ndatetime > (int)(strlen(sformat) + 64))
	{
		tm.tm_sec	= (t%60);
		tm.tm_min	= ((t/60)%60);
		tm.tm_hour	= ((t/3600)%24);
		tm.tm_mday	= (int)(t/86400);
		sprintf(sdatetime,sformat,(tm.tm_mday),tm.tm_hour,(tm.tm_min),(tm.tm_sec));
		*ndatetime	= strlen(sdatetime);
		return sdatetime;
	}

	LOG_ERROR(("ErrorCode:%s [ZOS::systemdatetime] parameter is not right!\r\n", GetErrorCodeString(20001)));
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
char* ZOS::namefromfile(const char* sfile)
{
	if(sfile != 0)
	{
		char*	pname	= (char*)sfile;
		char*	ptemp	= (char*)sfile;

		for(;*ptemp != '\0';)
		{
			if(*ptemp == '\\' || *ptemp == '/')
			{
				pname	= ++ptemp;
			}else{
				ptemp	++;
			}
		}
		return pname;
	}

	LOG_ERROR(("ErrorCode:%s [ZOS::namefromfile] parameter is not right, sfile=NULL!\r\n", GetErrorCodeString(20002)));
	return 0;
}
char* ZOS::extendnamefromfile(const char* sfile)
{
	if(sfile != 0)
	{
		char*	pextendname		= NULL;
		char*	ptemp			= (char*)sfile;

		for(;*ptemp != '\0';)
		{
			if(*ptemp == '.')
			{
				pextendname	= ++ptemp;
			}else{
				ptemp	++;
			}
		}
		return pextendname;
	}

	LOG_ERROR(("ErrorCode:%s [ZOS::extendnamefromfile] parameter is not right, sfile=NULL!\r\n", GetErrorCodeString(20003)));
	return 0;
}
char* ZOS::pathfromfile(const char* sfile,char* spath,int* npath)
{
	if(sfile != 0 && spath != 0 && npath != 0 && *npath > (int)strlen(sfile))
	{
		char*	ppath		= 0;
		char*	ptemp		= 0;
		int		nlen		= 0;

		memset(spath,0,*npath);
		strncpy(spath,sfile,*npath);
		nlen	= strlen(spath);
		ptemp	= spath + (nlen - 1);

		for(;nlen >= 0;)
		{
			if(*ptemp == '\\' || *ptemp == '/')
			{
				ppath		= spath;
				ptemp[1]	= '\0';
				break;
			}
			ptemp	--;
			nlen	--;
		}
		*npath	= strlen(spath);
		return spath;
	}

	LOG_ERROR(("ErrorCode:%s [ZOS::pathfromfile] parameter is not right!\r\n", GetErrorCodeString(20004)));
	return 0;
}
char* ZOS::formatpath(const char* spath,char* sformatpath,int* nformatpath)
{
	if(spath != 0 && sformatpath != 0 && nformatpath != 0 && *nformatpath > (int)strlen(spath))
	{
		char*	ptemp	= 0;

		memset(sformatpath,0,*nformatpath);
		strncpy(sformatpath,spath,*nformatpath);
		ptemp	= sformatpath;
		while(*ptemp != '\0')
		{
			if(*ptemp == '\\') *ptemp = '/';
			ptemp	++;
		}
		*nformatpath	= strlen(sformatpath);
		return sformatpath;
	}

	MESSAGE_OUT(("ErrorCode:%s [ZOS::formatpath] parameter is not right!\r\n", GetErrorCodeString(20005)));
	return 0;
}
char* ZOS::formaturl(const char* surl,char* sformaturl,int* nformaturl)
{
	if(surl != 0 && sformaturl != 0 && nformaturl != 0 && *nformaturl > (int)strlen(surl))
	{
		char*	ptemp	= 0;

		memset(sformaturl,0,*nformaturl);
		strncpy(sformaturl,surl,*nformaturl);
		ptemp	= sformaturl;
		while(*ptemp != '\0')
		{
			if(*ptemp == '\\') *ptemp = '/';
			ptemp	++;
		}
		*nformaturl	= strlen(sformaturl);
		return sformaturl;
	}

	LOG_ERROR(("ErrorCode:%s [ZOS::formaturl] parameter is not right!\r\n", GetErrorCodeString(20006)));
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
char* ZOS::lower(char* s)
{
	char*	cp	= s;
	int		n	= ( 'a' - 'A');

	for (cp = s; *cp; ++cp)
	{
		if ('A' <= *cp && *cp <= 'Z')
		{
			*cp += n;
		}
	}
	return s;
}
char* ZOS::upper(char* s)
{
	char*	cp	= s;
	int		n	= ( 'a' - 'A');

	for (cp = s; *cp; ++cp)
	{
		if ('a' <= *cp && *cp <= 'z')
		{
			*cp -= n;
		}
	}
	return s;
}
///////////////////////////////////////////////////////////////////////////////
int ZOS::checkdir(const char* sdir)
{
#if	defined(_WIN32_)
	struct	stat64	thestat;
#elif	defined(_LINUX_)
#ifdef O_LARGEFILE
	struct	stat64	thestat;
#else
	struct	stat	thestat;
#endif	//
#elif defined(_FreeBSD_)
	struct	stat	thestat;
#else
	struct	stat	thestat;
#endif	//defined(_WIN32_)
	if(sdir != 0)
	{
#if	defined(_WIN32_)
		 if((::stat64(sdir, &thestat)) >= 0)
#elif	defined(_LINUX_)
#ifdef O_LARGEFILE
		 if((::stat64(sdir, &thestat)) >= 0)
#else
		 if((::stat(sdir, &thestat)) >= 0)
#endif	//
#elif defined(_FreeBSD_)
		 if((::stat(sdir, &thestat)) >= 0)
#else
		 if((::stat(sdir, &thestat)) >= 0)
#endif	//defined(_WIN32_)
		{
#if	defined(_WIN32_)||defined(_MINGW_)
			if((thestat.st_mode & _S_IFDIR)) 
#else
			if(S_ISDIR(thestat.st_mode))
#endif
			{
				//directory exists
				return 0;
			}else
#if	defined(_WIN32_)||defined(_MINGW_)
			if((thestat.st_mode & _S_IFREG)) 
#else
			if(S_ISREG(thestat.st_mode))
#endif
			{
				//file exists
				return 1;
			}
		}
	}
	return -1;
}
int ZOS::makedir(const char* sdir)
{
	if(sdir != 0)
	{
		struct stat	thestat;
		if (::stat(sdir, &thestat) == -1)
		{
			//this directory doesn't exist, so let's try to create it
#if	defined(_WIN32_)||defined(_MINGW_)
			if (::mkdir(sdir) == -1)
#else
			if (::mkdir(sdir,(S_IRWXU|S_IRWXG|S_IRWXO)) == -1)
#endif
				if(ZOS::setcurrentdir(sdir) != 0)
				{
					MESSAGE_OUT(("ErrorCode:%s [ZOS::makedir] system create path %s error!\r\n", GetErrorCodeString(20009), sdir));
					return ZOSThread::GetError();
				}
		}
#if	defined(_WIN32_)||defined(_MINGW_)
		else if(!(thestat.st_mode & _S_IFDIR))
		{
			MESSAGE_OUT(("ErrorCode:%s [ZOS::makedir] there is a file at this point in the path %s!\r\n", GetErrorCodeString(20008), sdir));
			return EEXIST; // there is a file at this point in the path!
		}
#else
		else if(!S_ISDIR(thestat.st_mode))
		{
			return EEXIST;//there is a file at this point in the path!
		}
#endif
		else
		{
			return ZOS::setcurrentdir(sdir);
		}
#if	defined(_WIN32_)||defined(_MINGW_)
#else
		chmod(sdir,(S_IRWXU|S_IRWXG|S_IRWXO));
#endif
		//directory exists
		return 0;
	}

	MESSAGE_OUT(("ErrorCode:%s [ZOS::makedir] parameter is not right!\r\n", GetErrorCodeString(20007)));
	return -1;
}
int ZOS::recursivemakedir(const char* sdir)
{
	char	spath[(MAX_FILE_PATH+4)];
	int		npath	= MAX_FILE_PATH;

	if(sdir != 0)
	{
		char*		ptemp						= 0;

		memset(spath,0,sizeof(spath));
		ZOS::formatpath(sdir,spath,&npath);
		
		ptemp	= spath;
		while(*ptemp == '/') ptemp	++;
		while(*ptemp != '\0')
		{
			if(*ptemp == '/')
			{
				*ptemp	= '\0';
				if(ZOS::makedir(spath) != 0)
				{
					*ptemp	= '/';
					break;
				}
				*ptemp	= '/';
			}
			ptemp	++;
		}
		return ZOS::makedir(spath);
	}

	MESSAGE_OUT(("ErrorCode:%s [ZOS::recursivemakedir] parameter is not right!\r\n", GetErrorCodeString(20010)));
	return -1;
}
int ZOS::setcurrentdir(const char* sdir)
{
	if(sdir != 0)
	{
#ifdef _WIN32_
        if (::chdir(sdir) == -1)
#else
        if (::chdir(sdir) == -1)
#endif
		{
			MESSAGE_OUT(("ErrorCode:%s [ZOS::setcurrentdir] system setcurrentdir fail, sdir=%s\r\n", GetErrorCodeString(20011), sdir));
			return ZOSThread::GetError();
		}
		return 0;
	}
	MESSAGE_OUT(("ErrorCode:%s [ZOS::setcurrentdir] parameter is not right!\r\n", GetErrorCodeString(20011)));
	return -1;
}
int ZOS::getlocalip(const char *sInterfaceName, char *sIP)
{
#ifdef _LINUX_
    int nReturn = -1;
    int sock_get_ip;

    struct sockaddr_in *sin;
    struct ifreq ifr_ip;

    sock_get_ip = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_get_ip != -1)
    {
        memset(&ifr_ip, 0, sizeof(ifr_ip));
        strncpy(ifr_ip.ifr_name, sInterfaceName, sizeof(ifr_ip.ifr_name)-1);

        if (ioctl(sock_get_ip, SIOCGIFADDR, &ifr_ip) >= 0)
        {
            sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
            strcpy(sIP, inet_ntoa(sin->sin_addr));
            nReturn = 0;
        }

        close(sock_get_ip);
    }

    return nReturn;

#endif
    return -1;
}
///////////////////////////////////////////////////////////////////////////////
int ZOS::outputcolor(OUTPUT_ATTRIBUTE attr,OUTPUT_COLOR fgcolor,OUTPUT_COLOR bgcolor)
{
#ifdef	_WIN32_
#ifndef	_MINGW_
	static	const	WORD	WIN32Attribute[]=
	{
		0,								//Reset All Attributes (return to normal mode)
		FOREGROUND_INTENSITY,			//Bright (Usually turns on BOLD)
		0,								//Dim
		COMMON_LVB_UNDERSCORE,			//Underline
		0,								//Blink
		0,								//Reverse
		0,								//Hidden
	};
	static	const	WORD	WIN32Color[]	= 
	{
		0,													//Black
		FOREGROUND_RED,										//Red
		FOREGROUND_GREEN,									//Green
		FOREGROUND_RED|FOREGROUND_GREEN,					//Yellow
		FOREGROUND_BLUE,									//Blue
		FOREGROUND_RED|FOREGROUND_BLUE,						//Magenta
		FOREGROUND_GREEN|FOREGROUND_BLUE,					//Cyan
		FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE,	//White

	};
	static	const	WORD	WIN32BKSHIFT	= 4;
					WORD	nAttr			= ((WIN32Attribute[attr]&0xFFFF)|(WIN32Color[fgcolor]&0xFFFF)|((WIN32Color[bgcolor]&0xFFFF)<<WIN32BKSHIFT));
					HANDLE	hstdout			= ::GetStdHandle(STD_OUTPUT_HANDLE); 

	if(hstdout != INVALID_HANDLE_VALUE) 
	{
		SetConsoleTextAttribute(hstdout,nAttr);
	}
#endif	//_MINGW_
#else
	char	command[32];
	sprintf(command,"%c[%d;%d;%dm",0x1B,attr,(fgcolor+30),(bgcolor+40));
	printf("%s",command);
#endif	//_WIN32_
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//protocol://username:password@host:port/path
//protocol:username:password@host:port/path
BOOL ZOS::DecodeURL(char* sURL,char** sProtocol,char** sHost,char** sPort,char** sUser,char** sPass,char** sPath)
{
	char*			pLocalUser		= NULL;
	char*			pLocalPass		= NULL;
	BOOL			bPath			= FALSE;

	if(sURL != NULL)
	{
		if(sProtocol != NULL)	*sProtocol	= NULL;
		if(sHost != NULL)		*sHost		= NULL;
		if(sPort != NULL)		*sPort		= NULL;
		if(sUser != NULL)		*sUser		= NULL;
		if(sPass != NULL)		*sPass		= NULL;
		if(sPath != NULL)		*sPath		= NULL;

		{
			char*	p	= (char*)sURL;
			int		m	= 0;
			int		n	= 0;
			if(sProtocol != NULL)
			{
				*sProtocol	= p;
			}
			while(*p != '\0')
			{
				//protocol://
				if(*p == ':')
				{
					//:
					if(m > 0)
					{
						if(pLocalPass == NULL)
						{
							pLocalPass	= (p + 1);
						}
					}
					m	++;
					if(!bPath)
					{
						*p	= '\0';
					}
				}else if(*p == '/')
				{
					if(*(p + 1) == '/')
					{
						if(pLocalUser == NULL)
						{
							pLocalUser	= (p + 2);
						}
						*p	= '\0';
						p	++;
						*p	= '\0';
					}else{
						bPath	= TRUE;
						if(n == 0)
						{
							if(sHost != NULL)
							{
								*sHost	= pLocalUser;
							}
							if(sPort != NULL)
							{
								*sPort	= pLocalPass;
							}
							if(sPath != NULL)
							{
								if(*sPath == NULL)
								{
									*sPath	= (p + 1);
								}
							}
							*p	= '\0';
						}
						n	++;
					}
				}else if(*p == '@')
				{
					if(pLocalUser != NULL)
					{
						*sUser		= pLocalUser;
						pLocalUser	= NULL;
					}
					if(sPass != NULL)
					{
						*sPass		= pLocalPass;
						pLocalPass	= NULL;
					}
					*p	= '\0';
					pLocalUser	= (p + 1);

				}else{
					if(m == 0)
					{
						if(*p > 'a' && *p < 'z')
						{
							*p	-= ('a' - 'A');
						}
					}
				}
				p	++;
			}
			if(sHost != NULL)
			{
				*sHost	= pLocalUser;
			}
			if(sPort != NULL)
			{
				*sPort	= pLocalPass;
			}
			return TRUE;
		}
	}
	LOG_ERROR(("ErrorCode:%s [ZOS::DecodeURL] parameter is not right, sURL=NULL\r\n", GetErrorCodeString(20012)));
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
//protocol://host:port/path
//protocol:host:port/path
BOOL ZOS::URLUserClear(const char* sURL,char* sLocalURL,int nLocalURL)
{
    if(sURL != NULL && sLocalURL != NULL && nLocalURL >= (int)strlen(sURL))
    {
        char*	p	= sLocalURL;
        char*	q	= 0;

        memset(sLocalURL,0,nLocalURL);
        sprintf(sLocalURL,"%s",sURL);

        while(*p != '\0')
        {
            //protocol://
            //protocol:
            if(*p == ':')
            {
                if(q == 0)
                {
                    p	++;
                    while((*p == '/')||(*p == '\\')) p ++;
                    q	= p;
                }
            }
            //protocol://user:pass@xxx/path
            if(*p == '@')
            {
                break;
            }
            //protocol://xxx/path
            if((*p == '/')||(*p == '\\'))
            {
                break;
            }
            p	++;
        }
        if(*p == '@')
        {
            strcpy(q,p+1);
        }
        return TRUE;
    }
    return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef	_DEBUG_
void DecodeURLTest()
{
	char*	sURL		= NULL;
	char*	sProtocol	= NULL;
	char*	sHost		= NULL;
	char*	sPort		= NULL;
	char*	sUser		= NULL;
	char*	sPass		= NULL;
	char*	sPath		= NULL;

	sURL	= (char*)"http://127.0.0.1";
	ZOS::DecodeURL(sURL,&sProtocol,&sHost,&sPort,&sUser,&sPass,&sPath);
	printf("TEST URL = %s\r\n",sURL);
	printf("TEST PROTOCOL = %s\r\n",sProtocol);
	printf("TEST HOST     = %s\r\n",sHost);
	printf("TEST PORT     = %s\r\n",sPort);
	printf("TEST USER     = %s\r\n",sUser);
	printf("TEST PASS     = %s\r\n",sPass);
	printf("TEST PATH     = %s\r\n",sPath);

	sURL	= (char*)"http://127.0.0.1:8080";
	ZOS::DecodeURL(sURL,&sProtocol,&sHost,&sPort,&sUser,&sPass,&sPath);
	printf("TEST URL = %s\r\n",sURL);
	printf("TEST PROTOCOL = %s\r\n",sProtocol);
	printf("TEST HOST     = %s\r\n",sHost);
	printf("TEST PORT     = %s\r\n",sPort);
	printf("TEST USER     = %s\r\n",sUser);
	printf("TEST PASS     = %s\r\n",sPass);
	printf("TEST PATH     = %s\r\n",sPath);

	sURL	= (char*)"http://admin:123@127.0.0.1";
	ZOS::DecodeURL(sURL,&sProtocol,&sHost,&sPort,&sUser,&sPass,&sPath);
	printf("TEST URL = %s\r\n",sURL);
	printf("TEST PROTOCOL = %s\r\n",sProtocol);
	printf("TEST HOST     = %s\r\n",sHost);
	printf("TEST PORT     = %s\r\n",sPort);
	printf("TEST USER     = %s\r\n",sUser);
	printf("TEST PASS     = %s\r\n",sPass);
	printf("TEST PATH     = %s\r\n",sPath);

	sURL	= (char*)"http://admin:123@127.0.0.1:8080";
	ZOS::DecodeURL(sURL,&sProtocol,&sHost,&sPort,&sUser,&sPass,&sPath);
	printf("TEST URL = %s\r\n",sURL);
	printf("TEST PROTOCOL = %s\r\n",sProtocol);
	printf("TEST HOST     = %s\r\n",sHost);
	printf("TEST PORT     = %s\r\n",sPort);
	printf("TEST USER     = %s\r\n",sUser);
	printf("TEST PASS     = %s\r\n",sPass);
	printf("TEST PATH     = %s\r\n",sPath);

	sURL	= (char*)"http://admin:123@127.0.0.1:8080/1/test.html";
	ZOS::DecodeURL(sURL,&sProtocol,&sHost,&sPort,&sUser,&sPass,&sPath);
	printf("TEST URL = %s\r\n",sURL);
	printf("TEST PROTOCOL = %s\r\n",sProtocol);
	printf("TEST HOST     = %s\r\n",sHost);
	printf("TEST PORT     = %s\r\n",sPort);
	printf("TEST USER     = %s\r\n",sUser);
	printf("TEST PASS     = %s\r\n",sPass);
	printf("TEST PATH     = %s\r\n",sPath);

	sURL	= (char*)"rtsp://127.0.0.1/test/1.mp4";
	ZOS::DecodeURL(sURL,&sProtocol,&sHost,&sPort,&sUser,&sPass,&sPath);
	printf("TEST URL = %s\r\n",sURL);
	printf("TEST PROTOCOL = %s\r\n",sProtocol);
	printf("TEST HOST     = %s\r\n",sHost);
	printf("TEST PORT     = %s\r\n",sPort);
	printf("TEST USER     = %s\r\n",sUser);
	printf("TEST PASS     = %s\r\n",sPass);
	printf("TEST PATH     = %s\r\n",sPath);

	sURL	= (char*)"sip:alice@10.1.2.3";
	ZOS::DecodeURL(sURL,&sProtocol,&sHost,&sPort,&sUser,&sPass,&sPath);
	printf("TEST URL = %s\r\n",sURL);
	printf("TEST PROTOCOL = %s\r\n",sProtocol);
	printf("TEST HOST     = %s\r\n",sHost);
	printf("TEST PORT     = %s\r\n",sPort);
	printf("TEST USER     = %s\r\n",sUser);
	printf("TEST PASS     = %s\r\n",sPass);
	printf("TEST PATH     = %s\r\n",sPath);
}
#endif	//_DEBUG_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
