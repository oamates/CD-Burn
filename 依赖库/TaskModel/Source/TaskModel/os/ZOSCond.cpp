#include "ZOSCond.h"
#include "ZOSThread.h"
///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32_
#include	<time.h>
#endif	//
#ifndef _WIN32_
#include	<sys/time.h>
#endif
///////////////////////////////////////////////////////////////////////////////
//#define	ZOSCOND_DEBUG		1
///////////////////////////////////////////////////////////////////////////////
ZOSCond::ZOSCond()
{
#if	defined(_WIN32_)
	m_pCondition	= ::CreateEvent(NULL,FALSE,FALSE,NULL);
	TMASSERT((m_pCondition != 0));
#elif defined(__PTHREADS_MUTEXES__)
	pthread_condattr_t	cond_attr;
	pthread_condattr_init(&cond_attr);
	pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC);
	int	ret	= pthread_cond_init(&m_pCondition,&cond_attr);
	TMASSERT((ret == 0));
    if (ret != 0)
    {
        //todo
    }
    
#else
	m_pCondition	= mycondition_alloc();
	TMASSERT((m_pCondition != 0));
#endif
#ifdef	ZOSCOND_DEBUG
	MESSAGE_OUT(("ZOSCond Init (%s)\r\n",GetObjectName()));
#endif	//
}
ZOSCond::~ZOSCond()
{
#if	defined(_WIN32_)
	BOOL	bError	= ::CloseHandle(m_pCondition);
	TMASSERT((bError == TRUE));
#elif defined(__PTHREADS_MUTEXES__)
	pthread_cond_destroy(&m_pCondition);
#else
	TMASSERT((m_pCondition != 0));
	mycondition_free(m_pCondition);
#endif
#ifdef	ZOSCOND_DEBUG
	MESSAGE_OUT(("ZOSCond Uninit (%s)\r\n",GetObjectName()));
#endif	//
}
///////////////////////////////////////////////////////////////////////////////
#if	defined(__PTHREADS_MUTEXES__)
void ZOSCond::TimedWait(ZOSMutex* pMutex,UINT nTimeoutInMilSecs)
{
	struct timespec	ts;
	//struct timeval	tv;
	//struct timezone	tz;
	int				sec,usec;
    
	//These platforms do refcounting manually, and wait will release the mutex,
	// so we need to update the counts here

	pMutex->m_pHolderCount	--;
	pMutex->m_pHolder		= 0;


	if (nTimeoutInMilSecs == 0)
	{
		(void)pthread_cond_wait(&m_pCondition, &pMutex->m_pMutex);
	}else{
		//gettimeofday(&tv,&tz);
        clock_gettime(CLOCK_MONOTONIC, &ts);
		sec					= nTimeoutInMilSecs/1000;
		nTimeoutInMilSecs	= (nTimeoutInMilSecs - (sec * 1000));
		TMASSERT((nTimeoutInMilSecs < 1000));
		usec				= (nTimeoutInMilSecs * 1000);
		//TMASSERT((tv.tv_usec < 1000000));
		ts.tv_sec			= (ts.tv_sec + sec);
		ts.tv_nsec			= (ts.tv_nsec + usec * 1000);
		TMASSERT((ts.tv_nsec < 2000000000));
		if(ts.tv_nsec > 999999999)
		{
			ts.tv_sec	++;
			ts.tv_nsec	-= 1000000000;
		}
		(void)pthread_cond_timedwait(&m_pCondition,&pMutex->m_pMutex,&ts);
	}

	pMutex->m_pHolderCount	++;
	pMutex->m_pHolder		= ZOSThread::GetCurrentThreadID();    
}
#endif
///////////////////////////////////////////////////////////////////////////////
