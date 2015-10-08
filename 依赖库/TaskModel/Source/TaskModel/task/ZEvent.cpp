#include "ZEvent.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//#define	ZEVENT_DEBUG	1
///////////////////////////////////////////////////////////////////////////////
#define	MIN_EVENT_UID	1
#define	MAX_EVENT_UID	8000
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT	ZEvent::m_gEventUID	= 0;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZEvent::ZEvent(ZTask* pTask,ZEventThread* pEventThread)
:ZObject()
,m_nState(0)
,m_hHandle(INVALID_FILE_HANDLE)
,m_pEventTask(pTask)
,m_EventObject()
,m_EventMutex("EventMutex")
,m_nEventUID(0)
,m_pEventThread(pEventThread)
{
}
ZEvent::~ZEvent()
{
#ifdef	ZEVENT_DEBUG
	MESSAGE_OUT(("ZEvent::~ZEvent(0x%08X)\r\n",this));
#endif	//ZEVENT_DEBUG

	ZEvent::Close();

#ifdef	ZEVENT_DEBUG
	MESSAGE_OUT(("ZEvent::~ZEvent(0x%08X)OK\r\n",this));
#endif	//ZEVENT_DEBUG
}
///////////////////////////////////////////////////////////////////////////////
UINT ZEvent::GetStatus()
{
	return m_nState;
}
int ZEvent::GetHandle()
{
	return m_hHandle;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZEvent::SetTask(ZTask* pTask)
{
	ZOSMutexLocker	locker(&m_EventMutex);

	if(m_pEventTask != NULL)
	{
		m_pEventTask->RemoveEvent();
	}
	m_pEventTask	= pTask;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZEvent::Create(int hHandle)
{
	if(m_pEventThread != NULL)
	{
		m_pEventThread->RemoveEvent(this,0);
	}
	m_hHandle	= hHandle;
	return TRUE;
}
BOOL ZEvent::Close()
{
	if(m_pEventThread != NULL)
	{
		m_pEventThread->RemoveEvent(this,0);
	}
	m_hHandle	= INVALID_FILE_HANDLE;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZEvent::RequestEvent(UINT nEvent)
{
	if(m_pEventThread != NULL)
	{
		if(m_nEventUID == 0)
		{
			if(!ZOS::systeminit(&m_gEventUID,MAX_EVENT_UID,MIN_EVENT_UID))
			{
				m_nEventUID	= ZOS::systemadd(&m_gEventUID);
			}else{
				m_nEventUID	= MIN_EVENT_UID;
			}
		}
		m_pEventThread->AddEvent(this,nEvent);
		return TRUE;
	}
	return FALSE;
}
BOOL ZEvent::RemoveEvent(UINT nEvent)
{
	if(m_pEventThread != NULL)
	{
		m_pEventThread->RemoveEvent(this,nEvent);
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZEvent::ProcessEvent(UINT nEvent)
{
	ZOSMutexLocker	locker(&m_EventMutex);

	if(ZObject::IsValidObject(m_pEventTask) == 0)
	{
		return m_pEventTask->AddEvent((ZTask::TASK_EVENT)nEvent);
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZEvent::ProcessEvent] parameter is not right m_pEventTask is invalid!\r\n", GetErrorCodeString(20022)));
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZEventThread::ZEventThread()
:ZOSThread("EventThread")
,m_EventThreadMutex("EventMutex")
,m_EventThreadTable("EventTable")
{
}
ZEventThread::~ZEventThread()
{
	TMASSERT((m_EventThreadTable.Count()==0));
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZEventThread::OnThreadStart()
{
	return TRUE;
}
BOOL ZEventThread::OnThreadEntry()
{
	WaitForEvent();

	return TRUE;
}
BOOL ZEventThread::OnThreadStop()
{
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZEventThread::AddEvent(ZEvent* pEvent,UINT nEvent)
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_EventThreadMutex);
#ifdef	ZEVENT_DEBUG
	MESSAGE_OUT(("ZEventThread::AddEvent(0x%08X)\r\n",pEvent));
#endif	//ZEVENT_DEBUG
	TMASSERT((pEvent != NULL));
	if(pEvent != NULL)
	{
		pEvent->m_EventObject.Set(pEvent->m_hHandle,pEvent);
		bReturn	= m_EventThreadTable.Register(&(pEvent->m_EventObject));
	}
	return bReturn;
}
BOOL ZEventThread::RemoveEvent(ZEvent* pEvent,UINT nEvent)
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_EventThreadMutex);

	TMASSERT((pEvent != NULL));
	
	if(pEvent != NULL)
	{
		bReturn	= m_EventThreadTable.Unregister(&(pEvent->m_EventObject));
		pEvent->m_EventObject.Set((UINT32)0,NULL);
	}
#ifdef	ZEVENT_DEBUG
	MESSAGE_OUT(("ZEventThread::RemoveEvent(0x%08X)\r\n",pEvent));
#endif	//ZEVENT_DEBUG
	return bReturn;
}
BOOL ZEventThread::WaitForEvent()
{
	ZOSThread::Sleep(9);
	return FALSE;
}
BOOL ZEventThread::OnEvent(int hHandle,UINT nEvent)
{
	BOOL			bReturn			= FALSE;
	ZOSMutexLocker	locker(&m_EventThreadMutex);
	ZEvent*			pLocalEvent		= NULL;
	ZOSObjectKey	Key(hHandle);
	ZOSObject*		pObject			= m_EventThreadTable.GetObject(&Key);
	if(pObject != NULL)
	{
		m_EventThreadTable.Unregister(pObject);
		pLocalEvent	= (ZEvent *)pObject->GetObject();
		if(pLocalEvent != NULL)
		{
            locker.Unlock();
			bReturn	= pLocalEvent->ProcessEvent(nEvent);
#ifdef	ZEVENT_DEBUG
			MESSAGE_OUT(("ZEventThread::GetEvent(0x%08X)\r\n",nEvent));
#endif	//ZEVENT_DEBUG
		}
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
