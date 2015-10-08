#include "ZIdleTask.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//#define	ZIDLETASK_DEBUG		1
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_IDLETIME	90
///////////////////////////////////////////////////////////////////////////////
ZIdleTaskThread* ZIdleTask::m_pIdleTaskThread	= NULL;
///////////////////////////////////////////////////////////////////////////////
ZIdleTask::ZIdleTask(CONST CHAR* sTaskName)
:ZTask(sTaskName)
,m_IdleOSHeapElement()
{
	m_IdleOSHeapElement.SetObject(this);
}
ZIdleTask::~ZIdleTask()
{
	ZIdleTask::RemoveIdleTask();
	m_IdleOSHeapElement.SetObject(NULL);
}
///////////////////////////////////////////////////////////////////////////////
void ZIdleTask::Initialize()
{
	TMASSERT((m_pIdleTaskThread==NULL));
	if(m_pIdleTaskThread == NULL)
	{
		m_pIdleTaskThread	= NEW ZIdleTaskThread();
	}
}
void ZIdleTask::Uninitialize()
{
	SAFE_DELETE(m_pIdleTaskThread);
}
void ZIdleTask::StartIdleTaskThread()
{
	if(m_pIdleTaskThread != NULL)
	{
		m_pIdleTaskThread->Start();
	}
}
void ZIdleTask::StopIdleTaskThread()
{
	if(m_pIdleTaskThread != NULL)
	{
		m_pIdleTaskThread->Stop();
	}
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZIdleTask::AddIdleTask(int nMSec)
{
	BOOL	bReturn	= FALSE;
#ifdef	ZIDLETASK_DEBUG
	MESSAGE_OUT(("ZIdleTask::AddIdleTask\r\n"));
#endif	//ZIDLETASK_DEBUG
	TMASSERT((m_pIdleTaskThread != NULL));
	if(m_pIdleTaskThread != NULL)
	{
		bReturn	= m_pIdleTaskThread->AddIdleTask(this,nMSec);
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZIdleTask::AddIdleTask] ERROR m_pIdleTaskThread=NULL!\r\n", GetErrorCodeString(20023)));
	}

	return bReturn;
}
BOOL ZIdleTask::RemoveIdleTask()
{
	BOOL	bReturn	= FALSE;
#ifdef	ZIDLETASK_DEBUG
	MESSAGE_OUT(("ZIdleTask::RemoveIdleTask\r\n"));
#endif	//ZIDLETASK_DEBUG
	TMASSERT((m_pIdleTaskThread != NULL));
	if(m_pIdleTaskThread != NULL)
	{
		bReturn	= m_pIdleTaskThread->RemoveIdleTask(this);
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZIdleTask::RemoveIdleTask] ERROR m_pIdleTaskThread=NULL!\r\n", GetErrorCodeString(20024)));
	}

	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZIdleTaskThread::ZIdleTaskThread()
:ZOSThread("IdleTaskThread")
,m_IdleTaskHeap("IdleTaskHeap")
,m_IdleTaskMutes("IdleTaskMutes")
,m_IdleTaskCond()
,m_IdleTaskTime(0)
{
}
ZIdleTaskThread::~ZIdleTaskThread()
{
	TMASSERT((m_IdleTaskHeap.HeapCount()==0));
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZIdleTaskThread::Start()
{
	return ZOSThread::Start();
}
BOOL ZIdleTaskThread::Stop(BOOL bWait)
{
	BOOL	bReturn	= FALSE;
	if(!m_bStop)
	{
		m_bStop	= TRUE;
#if	defined(_WIN32_)
		if(m_pThreadID != NULL)
		{
			if(bWait)
			{
				while(::WaitForSingleObject(m_pThreadID,DEFAULT_IDLETIME) == WAIT_TIMEOUT)
				{
					m_IdleTaskCond.Signal();
				}
			}
#elif	defined(_MINGW_)
		if(m_pThreadID.p != 0)
		{
			void*	retVal;
			m_IdleTaskCond.Signal();
			pthread_join((pthread_t)m_pThreadID,&retVal);
#else
#if defined(__PTHREADS__)
		if(m_pThreadID != 0)
		{
			void*	retVal;
			m_IdleTaskCond.Signal();
			pthread_join((pthread_t)m_pThreadID,&retVal);
#else
		if(m_pThreadID != 0)
		{
			m_IdleTaskCond.Signal();
			cthread_join((cthread_t)m_pThreadID);
#endif
#endif
			bReturn	= TRUE;
		}
	}
	m_IdleTaskHeap.RemoveAll();

	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZIdleTaskThread::AddIdleTask(ZIdleTask* pTask,int nMSec)
{
	UINT64			nTaskTime	= ZOS::milliseconds();
	BOOL			bReturn		= TRUE;
	ZOSMutexLocker	locker(&m_IdleTaskMutes);
#ifdef	ZIDLETASK_DEBUG
	MESSAGE_OUT(("ZIdleTaskThread::AddIdleTask(pTask = 0x%08X)\r\n",pTask));
#endif	//ZIDLETASK_DEBUG
	TMASSERT((pTask != NULL));
	TMASSERT((nMSec > 0));
	if(pTask != NULL && nMSec > 0)
	{
		if(pTask->m_IdleOSHeapElement.GetHeap() == NULL)
		{
			nTaskTime			+= nMSec;
			pTask->m_IdleOSHeapElement.SetValue(nTaskTime);

			m_IdleTaskHeap.Insert(&(pTask->m_IdleOSHeapElement));

#ifdef	ZIDLETASK_DEBUG
			MESSAGE_OUT(("ZIdleTaskThread::AddIdleTask(pTask->m_IdleOSHeapElement.GetHeap() = 0x%08X m_IdleTaskHeap = 0x%08X)\r\n",pTask->m_IdleOSHeapElement.GetHeap(),&m_IdleTaskHeap));
#endif	//ZIDLETASK_DEBUG
			bReturn = TRUE;
		}
		else
		{
			TMASSERT((pTask->m_IdleOSHeapElement.GetHeap() == &m_IdleTaskHeap));
			m_IdleTaskHeap.Remove(&(pTask->m_IdleOSHeapElement));
			nTaskTime			+= nMSec;
			pTask->m_IdleOSHeapElement.SetValue(nTaskTime);
			m_IdleTaskHeap.Insert(&(pTask->m_IdleOSHeapElement));
#ifdef	ZIDLETASK_DEBUG
			LOG_WARNING(("ErrorCode:%s [ZIdleTaskThread::AddIdleTask] ERROR(pTask->m_IdleOSHeapElement.GetHeap() = 0x%08X m_IdleTaskHeap = 0x%08X)!\r\n", 
				GetErrorCodeString(20026),pTask->m_IdleOSHeapElement.GetHeap(),&m_IdleTaskHeap));
#endif	//ZIDLETASK_DEBUG
			bReturn = FALSE;
		}
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZIdleTaskThread::AddIdleTask] ERROR(pTask = 0x%08X; nMSec = %d)!\r\n", GetErrorCodeString(20025),pTask,nMSec));
		bReturn = FALSE;
	}
	m_IdleTaskCond.Signal();
#ifdef	ZIDLETASK_DEBUG
	MESSAGE_OUT(("ZIdleTaskThread::AddIdleTask(pTask = 0x%08X,Count %d) OK\r\n",pTask,m_IdleTaskHeap.HeapCount()));
#endif	//ZIDLETASK_DEBUG
	return bReturn;
}
BOOL ZIdleTaskThread::RemoveIdleTask(ZIdleTask* pTask)
{
#ifdef	ZIDLETASK_DEBUG
	MESSAGE_OUT(("ZIdleTaskThread::RemoveIdleTask(pTask = 0x%08X)\r\n",pTask));
#endif	//ZIDLETASK_DEBUG
	ZOSMutexLocker	locker(&m_IdleTaskMutes);
	TMASSERT((pTask != NULL));
	if(pTask != NULL)
	{
		m_IdleTaskHeap.Remove(&(pTask->m_IdleOSHeapElement));
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZIdleTaskThread::RemoveIdleTask] ERROR pTask = 0x%08X!\r\n", GetErrorCodeString(20027),pTask));
	}
#ifdef	ZIDLETASK_DEBUG
	MESSAGE_OUT(("ZIdleTaskThread::RemoveIdleTask(pTask = 0x%08X) OK\r\n",pTask));
#endif	//ZIDLETASK_DEBUG

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZIdleTaskThread::OnThreadStart()
{
    	return TRUE;
}
BOOL ZIdleTaskThread::OnThreadEntry()
{
	if(!m_bStop)
	{
		ZOSMutexLocker	locker(&m_IdleTaskMutes);
		int				nHeap	= m_IdleTaskHeap.HeapCount();
		int				nWait	= 0;

		if(nHeap == 0)
		{
#ifdef	ZIDLETASK_DEBUG
			MESSAGE_OUT(("ZIdleTaskThread::OnThreadEntry(Wait)\r\n"));
#endif	//ZIDLETASK_DEBUG
			m_IdleTaskCond.Wait(&m_IdleTaskMutes);
#ifdef	ZIDLETASK_DEBUG
			MESSAGE_OUT(("ZIdleTaskThread::OnThreadEntry OK(Wait)\r\n"));
#endif	//ZIDLETASK_DEBUG
		}
		{
			ZIdleTask*		pIdleTask		= NULL;
			//MESSAGE_OUT(("[ZIdleTaskThread::OnThreadEntry] m_IdleTaskHeap %d\r\n", nHeap));
			ZOSHeapElement*	pHeapElement	= (ZOSHeapElement*)m_IdleTaskHeap.PeekMin();
							m_IdleTaskTime	= ZOS::milliseconds();
			//MESSAGE_OUT(("[ZIdleTaskThread::OnThreadEntry] GetValue (%"F_NUM_64"d) CurTime (%"F_NUM_64"d)\r\n",
			//	pHeapElement->GetValue(), m_IdleTaskTime));
			if(pHeapElement != NULL && pHeapElement->GetValue() <= m_IdleTaskTime)
			{
				pHeapElement	= (ZOSHeapElement*)m_IdleTaskHeap.ExtractMin();
				pIdleTask		= (ZIdleTask*)pHeapElement->GetObject();
				TMASSERT((pIdleTask != NULL));
				if(pIdleTask != NULL)
				{
					pIdleTask->AddEvent(ZTask::TASK_IDLE_EVENT);
				}
//#ifdef	ZIDLETASK_DEBUG
				//MESSAGE_OUT(("ZIdleTaskThread::OnThreadEntry(AddEvent pTask = 0x%08X) %s\r\n",pIdleTask, pIdleTask->GetObjectName()));
//#endif	//ZIDLETASK_DEBUG
				nHeap	--;
			}
		}
		nHeap	= m_IdleTaskHeap.HeapCount();
		if(nHeap > 0)
		{
			UINT64	nTime	= m_IdleTaskHeap.PeekMin()->GetValue();
			if(nTime > m_IdleTaskTime)
			{
				nTime			-= m_IdleTaskTime;
				nWait			= (int)nTime;
				TMASSERT((nWait>0));
				if(nWait < DEFAULT_NET_MIN_MTIMEOUT)
				{
					nWait	= DEFAULT_NET_MIN_MTIMEOUT;
				}
				if(nWait > (DEFAULT_NET_MAX_MTIMEOUT ))
				{
					nWait	= (DEFAULT_NET_MAX_MTIMEOUT);
				}
#ifdef	ZIDLETASK_DEBUG
				MESSAGE_OUT(("ZIdleTaskThread::OnThreadEntry(Wait %d)\r\n",nWait));
#endif	//ZIDLETASK_DEBUG
				m_IdleTaskCond.Wait(&m_IdleTaskMutes,nWait);
#ifdef	ZIDLETASK_DEBUG
				MESSAGE_OUT(("ZIdleTaskThread::OnThreadEntry OK(Wait %d)\r\n",nWait));
#endif	//ZIDLETASK_DEBUG
			}
		}
	}
	return TRUE;
}
BOOL ZIdleTaskThread::OnThreadStop()
{
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
