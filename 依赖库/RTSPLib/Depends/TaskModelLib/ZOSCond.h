///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZOSCond		Header File
	Create		20100607		ZHAOTT
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#include "ZOS.h"
#include "ZOSMutex.h"
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZOSCOND_H_
#define	_ZOSCOND_H_
///////////////////////////////////////////////////////////////////////////////
class	ZOSCond;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ZOSCond : public ZObject
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSCond();
	~ZOSCond();
///////////////////////////////////////////////////////////////////////////////
public:
	inline void	Signal();
	inline void	Wait(ZOSMutex* pMutex,int nTimeoutInMilSecs = 0);
///////////////////////////////////////////////////////////////////////////////
private:
#if	defined(_WIN32_)
	HANDLE				m_pCondition;
	UINT				m_pWaitCount;
#elif defined(__PTHREADS_MUTEXES__)
	pthread_cond_t     m_pCondition;
	void TimedWait(ZOSMutex* pMutex,UINT nTimeoutInMilSecs);
#else
	mycondition_t		m_pCondition;
#endif
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
inline void	ZOSCond::Signal()
{
#if	defined(_WIN32_)
	BOOL bError	= ::SetEvent(m_pCondition);
	TMASSERT((bError == TRUE));
#elif defined(__PTHREADS_MUTEXES__)
	pthread_cond_signal(&m_pCondition);
#else
	TMASSERT((m_pCondition != 0));
	mycondition_signal(m_pCondition);
#endif
}
inline void	ZOSCond::Wait(ZOSMutex* pMutex,int nTimeoutInMilSecs)
{
#if	defined(_WIN32_)
	DWORD	theTimeout	= INFINITE;
	if(nTimeoutInMilSecs > 0)
	{
		theTimeout = nTimeoutInMilSecs;
	}
    if (pMutex != NULL)
    {
        TMASSERT(pMutex != NULL);
        pMutex->Unlock();
    }
	m_pWaitCount	++;
	DWORD	nError	= ::WaitForSingleObject(m_pCondition,theTimeout);
	m_pWaitCount	--;
	TMASSERT(((nError == WAIT_OBJECT_0) || (nError == WAIT_TIMEOUT)));
    if (pMutex != NULL)
    {
        pMutex->Lock();
    }
#elif defined(__PTHREADS_MUTEXES__)
	this->TimedWait(pMutex,nTimeoutInMilSecs);
#else
    TMASSERT((m_pCondition != 0));
    mycondition_wait(m_pCondition,pMutex->m_pMutex,nTimeoutInMilSecs);
#endif
}
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZOSCOND_H_
///////////////////////////////////////////////////////////////////////////////
