#include "ZOSMutex.h"
#include "ZOSThread.h"
///////////////////////////////////////////////////////////////////////////////
//#define	ZOSMUTEX_DEBUG	1
///////////////////////////////////////////////////////////////////////////////
#if defined(__PTHREADS_MUTEXES__)
static pthread_mutexattr_t sLocalMutexAttr;
static pthread_mutexattr_t *sMutexAttr = &sLocalMutexAttr;
static pthread_once_t sMutexAttrInit = PTHREAD_ONCE_INIT;
static void MutexAttrInit();
void MutexAttrInit()
{
	::memset(sMutexAttr,0,sizeof(pthread_mutexattr_t));
	pthread_mutexattr_init(sMutexAttr);
#ifdef _DEBUG_
	pthread_mutexattr_settype(sMutexAttr,PTHREAD_MUTEX_ERRORCHECK);
	pthread_mutexattr_settype(sMutexAttr,PTHREAD_MUTEX_RECURSIVE);
#endif //_DEBUG_
}
#endif //(__PTHREADS_MUTEXES__
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSMutex::ZOSMutex(const char* sName)
:ZObject(sName)
{
#if	defined(_WIN32_)
	::InitializeCriticalSection(&m_pMutex);;
	m_pHolder			= 0;
	m_pHolderCount		= 0;
#elif	defined(__PTHREADS_MUTEXES__)
	(void)pthread_once(&sMutexAttrInit,MutexAttrInit);
	(void)pthread_mutex_init(&m_pMutex,sMutexAttr);
	m_pHolder			= 0;
	m_pHolderCount		= 0;
#else
	m_pMutex	= mymutex_alloc();
#endif	//(__PTHREADS_MUTEXES__)
#ifdef	ZOSMUTEX_DEBUG
	MESSAGE_OUT(("ZOSMutex Init(%s)\n",GetObjectName()));
#endif	//ZOSMUTEX_DEBUG
}
ZOSMutex::~ZOSMutex()
{
	TMASSERT((m_pHolder==0));
	TMASSERT((m_pHolderCount==0));
#if	defined(_WIN32_)
	::DeleteCriticalSection(&m_pMutex);
	m_pHolder			= 0;;
	m_pHolderCount		= 0;
#elif	defined(__PTHREADS_MUTEXES__)
	pthread_mutex_destroy(&m_pMutex);
	m_pHolder			= 0;
	m_pHolderCount		= 0;
#else
	if(m_pMutex != 0)
	{
		mymutex_free(m_pMutex);
		m_pMutex		= 0;
	}
#endif	//(__PTHREADS_MUTEXES__)
#ifdef	ZOSMUTEX_DEBUG
	MESSAGE_OUT(("ZOSMutex Uninit(%s)\n",GetObjectName()));
#endif	//ZOSMUTEX_DEBUG
}
///////////////////////////////////////////////////////////////////////////////
#if	defined(__PTHREADS_MUTEXES__)||defined(_WIN32_)
void ZOSMutex::RecursiveLock()
{
#ifdef	ZOSMUTEX_DEBUG
	MESSAGE_OUT(("ZOSMutex::RecursiveLock(%s)\r\n",GetObjectName()));
#endif	//ZOSMUTEX_DEBUG
	if(ZOSThread::GetCurrentThreadID() == m_pHolder)
	{
		m_pHolderCount	++;
		return;
	}
#if	defined(_WIN32_)
	::EnterCriticalSection(&m_pMutex);
#elif	defined(__PTHREADS_MUTEXES__)
	pthread_mutex_lock(&m_pMutex);
#endif	//(__PTHREADS_MUTEXES__)
    TMASSERT((m_pHolder == 0));
    m_pHolder			= ZOSThread::GetCurrentThreadID();
    m_pHolderCount	++;
    TMASSERT((m_pHolderCount == 1));
#ifdef	ZOSMUTEX_DEBUG
	MESSAGE_OUT(("ZOSMutex::RecursiveLock(%s) OK\r\n",GetObjectName()));
#endif	//ZOSMUTEX_DEBUG
}
void ZOSMutex::RecursiveUnlock()
{
#ifdef	ZOSMUTEX_DEBUG
	MESSAGE_OUT(("ZOSMutex::RecursiveUnlock(%s)\r\n",GetObjectName()));
#endif	//ZOSMUTEX_DEBUG
	if(ZOSThread::GetCurrentThreadID() == m_pHolder)
	{
		TMASSERT((m_pHolderCount > 0));
		m_pHolderCount--;
		if(m_pHolderCount == 0)
		{
			m_pHolder = 0;
#if	defined(_WIN32_)
			::LeaveCriticalSection(&m_pMutex);
#elif	defined(__PTHREADS_MUTEXES__)
			pthread_mutex_unlock(&m_pMutex);
#endif	//(__PTHREADS_MUTEXES__)
		}
	}
#ifdef	ZOSMUTEX_DEBUG
	MESSAGE_OUT(("ZOSMutex::RecursiveUnlock(%s) OK\r\n",GetObjectName()));
#endif	//ZOSMUTEX_DEBUG
}
BOOL ZOSMutex::RecursiveTryLock()
{
	if(ZOSThread::GetCurrentThreadID() == m_pHolder)
	{
		m_pHolderCount	++;
		return TRUE;
	}
#if	defined(_WIN32_)
	if(!::TryEnterCriticalSection(&m_pMutex))
	{
		return FALSE;
	}
#elif	defined(__PTHREADS_MUTEXES__)
	int	nError	= pthread_mutex_trylock(&m_pMutex);
	if(nError != 0)
	{
		return FALSE;
	}
#endif	//(__PTHREADS_MUTEXES__)
    TMASSERT((m_pHolder == 0));
    m_pHolder			= ZOSThread::GetCurrentThreadID();
    m_pHolderCount	++;
    TMASSERT((m_pHolderCount == 1));

	return TRUE;
}
#endif	//defined(__PTHREADS_MUTEXES__)||defined(_WIN32_)
///////////////////////////////////////////////////////////////////////////////
