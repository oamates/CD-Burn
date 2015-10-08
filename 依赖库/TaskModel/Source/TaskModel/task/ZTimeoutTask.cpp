#include "ZTimeoutTask.h"
///////////////////////////////////////////////////////////////////////////////
//#define	ZTIMEOUTTASK_DEBUG	1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZTimeoutTask::ZTimeoutTask(CONST CHAR* sTaskName)
:ZIdleTask(sTaskName)
,m_pTimeoutTask(NULL)
,m_nTimeoutMTime(0)
,m_nTimeoutMSec(0)
,m_TimeoutMutex("TimeoutMutex")
{
}
ZTimeoutTask::~ZTimeoutTask()
{
	Close();
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZTimeoutTask::SetTimeoutTask(ZTask* pTask)
{
	ZOSMutexLocker	locker(&m_TimeoutMutex);

	m_pTimeoutTask	= pTask;

#ifdef	ZTIMEOUTTASK_DEBUG
	MESSAGE_OUT(("ZTimeoutTask::SetTimeoutTask(0x%08X)\r\n",pTask));
#endif	//ZTIMEOUTTASK_DEBUG

	return TRUE;
}
BOOL ZTimeoutTask::SetTimeout(int nMSec)
{
	ZOSMutexLocker	locker(&m_TimeoutMutex);


	m_nTimeoutMSec	= nMSec;
	if(m_nTimeoutMSec > 0)
	{
		m_nTimeoutMTime	= (ZOS::milliseconds() + nMSec);
		ZIdleTask::AddIdleTask(nMSec);
	}else{
		m_nTimeoutMTime	= 0;
	}

#ifdef	ZTIMEOUTTASK_DEBUG
	MESSAGE_OUT(("ZTimeoutTask::SetTimeoutTask(%d)\r\n",m_nTimeoutMTime));
#endif	//ZTIMEOUTTASK_DEBUG

	return TRUE;
}
int ZTimeoutTask::GetTimeout()
{
    return (int)m_nTimeoutMSec;
}
BOOL ZTimeoutTask::RefreshTimeout()
{
	ZOSMutexLocker	locker(&m_TimeoutMutex);

	TMASSERT((m_nTimeoutMSec > 0));
	if(m_nTimeoutMSec > 0)
	{
		m_nTimeoutMTime	= (ZOS::milliseconds() + m_nTimeoutMSec);
		ZIdleTask::AddIdleTask((int)m_nTimeoutMSec);
	}

#ifdef	ZTIMEOUTTASK_DEBUG
	MESSAGE_OUT(("ZTimeoutTask::RefreshTimeout(%"F_NUM_64"d)\r\n",m_nTimeoutMTime));
#endif	//ZTIMEOUTTASK_DEBUG

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZTimeoutTask::OnTimeoutEvent()
{
#ifdef	ZTIMEOUTTASK_DEBUG
	MESSAGE_OUT(("ZTimeoutTask::OnTimeoutEvent(%"F_NUM_64"d)\r\n",m_nTimeoutMSec));
#endif	//ZTIMEOUTTASK_DEBUG
	return FALSE;
}
BOOL ZTimeoutTask::OnIdleEvent()
{
	ZOSMutexLocker	locker(&m_TimeoutMutex);

	if(m_nTimeoutMTime > 0)
	{
		UINT64	nTaskTime	= ZOS::milliseconds();
		if(m_nTimeoutMTime <= nTaskTime)
		{
			if(m_pTimeoutTask != NULL)
			{
				m_pTimeoutTask->AddEvent(TASK_TIMEOUT_EVENT);
			}
		}else{
			ZIdleTask::AddIdleTask((int)m_nTimeoutMSec);
		}
	}

#ifdef	ZTIMEOUTTASK_DEBUG
	MESSAGE_OUT(("ZTimeoutTask::OnIdleEvent(%"F_NUM_64"d)\r\n",m_nTimeoutMTime));
#endif	//ZTIMEOUTTASK_DEBUG

	return TRUE;
}

int ZTimeoutTask::Create()
{
	ZTask::Create();
	SetTimeoutTask(this);

	return 0;
}

int ZTimeoutTask::Close()
{
	if(m_pTimeoutTask != NULL)
	{
		m_pTimeoutTask->RemoveEvent();
	}
	SetTimeoutTask(NULL);
	SetTimeout(0);

	ZIdleTask::RemoveIdleTask();

	ZTask::Close();

	return 0;
}
///////////////////////////////////////////////////////////////////////////////
int ZTimeoutTask::Run(int nEvent)
{
	int	nIdleTime	= 0;

	nIdleTime	= ZIdleTask::Run(nEvent);

	if(nEvent&TASK_KILL_EVENT)
	{
	}else
	if(nEvent&TASK_TIMEOUT_EVENT)
	{
		ZTimeoutTask::OnTimeoutEvent();
	}else
	if(nEvent&TASK_IDLE_EVENT)
	{
		ZTimeoutTask::OnIdleEvent();
	}
	return nIdleTime;
}
///////////////////////////////////////////////////////////////////////////////
