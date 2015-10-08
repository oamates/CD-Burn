#include "ZOSThread.h"
///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32_
#include	<time.h>
#include	<process.h>
#endif	//_WIN32_
#ifndef _WIN32_
#include	<sys/time.h>
#endif	//_WIN32_
///////////////////////////////////////////////////////////////////////////////
//#define		ZOSTHREAD_DEBUG		1
///////////////////////////////////////////////////////////////////////////////
#if	defined(_WIN32_)
DWORD	ZOSThread::m_sThreadStorageIndex	= 0;
#else
#if	defined(__PTHREADS__)
pthread_key_t	ZOSThread::m_gMainKey		= 0;
#if	defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
pthread_attr_t	ZOSThread::m_sThreadAttr;
#endif	//(_POSIX_THREAD_PRIORITY_SCHEDULING)
#endif	//(__PTHREADS__)
#endif	//(_WIN32_)
void*	ZOSThread::m_sMainThreadData	= NULL;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSThread::ZOSThread(CONST CHAR* sThreadName)
:ZObject(sThreadName)
,m_ThreadMutex("ThreadMutex")
{
	m_bStop		= TRUE;
    m_nID       = 0;
#if	defined(_WIN32_)
	m_pThreadID	= NULL;
#elif	defined(_MINGW_)
	memset(&m_pThreadID,0,sizeof(pthread_t));
#else
#if	defined(__PTHREADS__)
	m_pThreadID	= 0;
#else
	m_pThreadID	= 0;
#endif	//(__PTHREADS__)
#endif	//(_WIN32_)
#ifdef	ZOSTHREAD_DEBUG
	MESSAGE_OUT(("ZOSThread Init(%s) \r\n",GetObjectName()));
#endif	//ZOSTHREAD_DEBUG
}
ZOSThread::~ZOSThread()
{
	ZOSThread::Stop(FALSE);
#ifdef	ZOSTHREAD_DEBUG
	MESSAGE_OUT(("ZOSThread Uninit(%s) \r\n",GetObjectName()));
#endif	//ZOSTHREAD_DEBUG
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZOSThread::Start()
{
#if	defined(_WIN32_)
	UINT	nID	= 0;
    m_bStop = FALSE;
	m_pThreadID	= (HANDLE)_beginthreadex(NULL,0,_Entry,(void*)this,0,&nID);
    m_nID = nID;
	TMASSERT((m_pThreadID != 0));
	return TRUE;
#else
#if defined(__PTHREADS__)
    pthread_attr_t threadAttr;
    if (pthread_attr_init(&threadAttr) != 0)
    {
        return FALSE;
    }
    //todo fanrl, write a function
    if (pthread_attr_setstacksize(&threadAttr, 4*1024*1024) != 0)
    {
        return FALSE;
    }
    m_bStop = FALSE;
	int	nError	= pthread_create((pthread_t*)&m_pThreadID,&threadAttr,_Entry,(void*)this);
	TMASSERT((nError == 0));
    if (nError != 0)
    {
        printf("thread create failed %d\r\n", nError);
    }

    pthread_attr_destroy(&threadAttr);
    
	return TRUE;
#else
    m_bStop = FALSE;
	m_pThreadID	= (UINT)cthread_fork((cthread_fn_t)_Entry,(any_t)this);
	return TRUE;
#endif
#endif
	return FALSE;
}
BOOL ZOSThread::Stop(BOOL bWait)
{
	if(!m_bStop)
	{
		m_bStop	= TRUE;
        m_nID = 0;
#if	defined(_WIN32_)
		if(m_pThreadID != NULL)
		{
			if(bWait)
			{
				DWORD	nError	= ::WaitForSingleObject(m_pThreadID,INFINITE);
				TMASSERT((nError == WAIT_OBJECT_0));
				::CloseHandle(m_pThreadID);
				m_pThreadID	= NULL;
#elif	defined(_MINGW_)
		if(m_pThreadID.p != 0)
		{
			if(bWait)
			{
				DWORD	nError	= ::WaitForSingleObject(m_pThreadID.p,INFINITE);
				TMASSERT((nError == WAIT_OBJECT_0));
				::CloseHandle(m_pThreadID.p);
				m_pThreadID.p	= NULL;
#else
#if defined(__PTHREADS__)
		if(m_pThreadID != 0)
		{
			if(bWait)
			{
				void*	retVal	= NULL;
				pthread_join((pthread_t)m_pThreadID,&retVal);
				if(retVal != NULL)
				{
					free(retVal);
				}
				m_pThreadID	= 0;
#else
		if(m_pThreadID != 0)
		{
			if(bWait)
			{
				cthread_join((cthread_t)m_pThreadID);
				m_pThreadID	= 0;
#endif	//(__PTHREADS__)
#endif	//(_WIN32_)
			}
			return TRUE;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZOSThread::OnThreadStart()
{
	return TRUE;
};
BOOL ZOSThread::OnThreadEntry()
{
	return TRUE;
};
BOOL ZOSThread::OnThreadStop()
{
	return TRUE;
};
///////////////////////////////////////////////////////////////////////////////
void ZOSThread::Initialize()
{
#if	defined(_WIN32_)
	if(m_sThreadStorageIndex == 0)
	{
		m_sThreadStorageIndex	= ::TlsAlloc();
		TMASSERT((m_sThreadStorageIndex >= 0));
	}
#else
#if	defined(__PTHREADS__)
	pthread_key_create(&ZOSThread::m_gMainKey,NULL);
#if	defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	pthread_attr_init(&m_sThreadAttr);
	pthread_attr_setscope(&m_sThreadAttr,PTHREAD_SCOPE_SYSTEM);
#endif	//(_POSIX_THREAD_PRIORITY_SCHEDULING)
#endif	//(__PTHREADS__)
#endif	//(_WIN32_)
}
void ZOSThread::Uninitialize()
{
#if	defined(_WIN32_)
	if(m_sThreadStorageIndex != 0)
	{
		BOOL	bError	= ::TlsFree(m_sThreadStorageIndex);
		m_sThreadStorageIndex	= 0;
		TMASSERT((bError));
	}
#else
#if	defined(__PTHREADS__)
	pthread_key_delete(ZOSThread::m_gMainKey);
#if	defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	pthread_attr_destroy(&m_sThreadAttr);
#endif	//(_POSIX_THREAD_PRIORITY_SCHEDULING)
#endif	//(__PTHREADS__)
#endif	//(_WIN32_)
}
///////////////////////////////////////////////////////////////////////////////
#if	defined(_WIN32_)
UINT WINAPI ZOSThread::_Entry(LPVOID pThreadParam)
#else
void* ZOSThread::_Entry(void* pThreadParam)
#endif	//(_WIN32_)
{
	int			nreturn	= -1;

	ZOSThread*	pThread	= (ZOSThread*)pThreadParam;
#if	defined(_WIN32_)
    BOOL	bError	= ::TlsSetValue(m_sThreadStorageIndex,pThreadParam);
    TMASSERT((bError == TRUE));
#else
#if	defined(__PTHREADS__)
	pThread->m_pThreadID = (pthread_t)pthread_self();
	pthread_setspecific(ZOSThread::m_gMainKey,pThreadParam);
#else
	pThread->m_pThreadID = (UINT)cthread_self();
	cthread_set_data(cthread_self(),(any_t)pThreadParam);
#endif	//(__PTHREADS__)
#endif	//(_WIN32_)

	if(pThread != NULL)
	{
		if(pThread->OnThreadStart())
		{
			while(!pThread->m_bStop)
			{
				pThread->OnThreadEntry();
			}
			pThread->OnThreadStop();
		}
		nreturn	= 0;
	}

#if	defined(_WIN32_)
	return nreturn;
#else
#if	defined(__PTHREADS__)
#endif	//(__PTHREADS__)
	return NULL;
#endif	//(_WIN32_)
}
ZOSThread* ZOSThread::GetCurrent()
{
#if	defined(_WIN32_)
	return (ZOSThread *)::TlsGetValue(m_sThreadStorageIndex);
#else
#if	defined(__PTHREADS__)
	return (ZOSThread *)pthread_getspecific(ZOSThread::m_gMainKey);
#else
	return (ZOSThread*)cthread_data(cthread_self());
#endif	//(__PTHREADS__)
#endif	//(_WIN32_)
}
///////////////////////////////////////////////////////////////////////////////
void ZOSThread::Sleep(int nMSec)
{
#if	defined(_WIN32_)
	if(nMSec > 0)
	{
		::Sleep(nMSec);
	}
#elif defined(_LINUX_)||defined(_MINGW_)
	if(nMSec > 0)
	{
		INT64	nStartMsec	= ZOS::milliseconds();
		INT64	nLeftMsec	= nMSec;
		INT64	nLeftUsec	= (nMSec * 1000);
		INT64	nSleepMsec	= 0;

		do{

			nLeftMsec	= (nMSec - nSleepMsec);
			if(nLeftMsec < 1)
			{
				break;
			}
			nLeftUsec	= (nLeftMsec * 1000);
			::usleep(nLeftUsec);
			nSleepMsec	= (ZOS::milliseconds() - nStartMsec);
			if(nSleepMsec < 0)
			{
				break;
			}
		}while(nSleepMsec < nMSec);
	}
#else
	if(nMSec > 0)
	{
		INT64	nLeftUsec	= (nMSec * 1000);
		::usleep(nLeftUsec);
	}
#endif
}
///////////////////////////////////////////////////////////////////////////////
int ZOSThread::GetError()
{
	int	nError	= 0;
#if	defined(_WIN32_)
	nError	= ::GetLastError();
	//MESSAGE_OUT(("ZOSThread::GetError(GetLastError=%d)\r\n",nError));
	switch(nError)
	{
		case ERROR_FILE_NOT_FOUND:
			nError	= ENOENT;
			break;
		case ERROR_PATH_NOT_FOUND:
			nError	= ENOENT;
			break;
		case WSAEINTR:
			nError	= EINTR;
			break;
		case WSAENETRESET:
			nError	= EPIPE;
			break;
		case WSAENOTCONN:
			nError	= ENOTCONN;
			break;
		case WSAEWOULDBLOCK:
			nError	= EAGAIN;
			break;
		case WSAECONNRESET:
			nError	= EPIPE;
			break;
		case WSAEADDRINUSE:
			nError	= EADDRINUSE;
			break;
		case WSAEMFILE:
			nError	= EMFILE;
			break;
		case WSAEINPROGRESS:
			nError	= EINPROGRESS;
			break;
		case WSAEADDRNOTAVAIL:
			nError	= EADDRNOTAVAIL;
			break;
		case WSAECONNABORTED:
			nError	= EPIPE;
			break;
		case 0:
			nError	= 0;
			break;
		default:
			nError	= ENOTCONN;
			break;
	}
#else
#if defined(__PTHREADS__)
	nError	= errno;
#else
	nError	= cthread_errno();
#endif
#endif
	return nError;;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ZOSThread::WaitForAllThread(DWORD nCount,ZOSThread** pThreads)
{
	if(nCount > 0 && pThreads != NULL)
	{
		DWORD	i	= 0;

#if	defined(_WIN32_)
		DWORD	nError	= 0;
		DWORD	nHandle	= 0;
		HANDLE*	pHandle	= NEW HANDLE[nCount];
		if(pHandle != NULL)
		{
			for(i = 0; i < nCount; i ++)
			{
				if(pThreads[i] != NULL && pThreads[i]->m_pThreadID != NULL)
				{
					pHandle[nHandle]	= pThreads[i]->m_pThreadID;
					nHandle				++;
				}
			}
			if(nHandle > 0)
			{
				nError	= ::WaitForMultipleObjects(nHandle,pHandle,TRUE,INFINITE);
				TMASSERT((nError == WAIT_OBJECT_0));
			}
		}
		for(i = 0; i < nCount; i ++)
		{
			if(pThreads[i] != NULL && pThreads[i]->m_pThreadID != NULL)
			{
				::CloseHandle(pThreads[i]->m_pThreadID);
				pThreads[i]->m_pThreadID	= NULL;
			}
		}
		SAFE_DELETE_ARRAY(pHandle);
#elif	defined(_MINGW_)
		DWORD	nError	= 0;
		DWORD	nHandle	= 0;
		HANDLE*	pHandle	= NEW HANDLE[nCount];
		if(pHandle != NULL)
		{
			for(i = 0; i < nCount; i ++)
			{
				if(pThreads[i] != NULL && pThreads[i]->m_pThreadID.p != NULL)
				{
					pHandle[nHandle]	= pThreads[i]->m_pThreadID.p;
					nHandle				++;
				}
			}
			if(nHandle > 0)
			{
				nError	= ::WaitForMultipleObjects(nHandle,pHandle,TRUE,INFINITE);
				TMASSERT((nError == WAIT_OBJECT_0));
			}
			for(i = 0; i < nCount; i ++)
			{
				if(pThreads[i] != NULL && pThreads[i]->m_pThreadID.p != NULL)
				{
					::CloseHandle(pThreads[i]->m_pThreadID.p);
					pThreads[i]->m_pThreadID.p	= NULL;
				}
			}
		}
		SAFE_DELETE_ARRAY(pHandle);
#else
#if defined(__PTHREADS__)
		for(i = 0; i < nCount; i ++)
		{
			if(pThreads[i] != NULL)
			{
				if(pThreads[i]->m_pThreadID != 0)
				{
					void*	retVal	= NULL;
					pthread_join((pthread_t)pThreads[i]->m_pThreadID,&retVal);
					if(retVal != 0)
					{
						free(retVal);
					}
					pThreads[i]->m_pThreadID	= 0;
				}
			}
		}
#else
		for(i = 0; i < nCount; i ++)
		{
			if(pThreads[i] != NULL)
			{
				if(pThreads[i]->m_pThreadID != 0)
				{
					cthread_join((cthread_t)pThreads[i]->m_pThreadID);
					pThreads[i]->m_pThreadID	= 0;
				}
			}
		}
#endif
#endif
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
