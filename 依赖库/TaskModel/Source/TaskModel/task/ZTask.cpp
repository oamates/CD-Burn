#include "ZTask.h"
#include "ZOS.h"
///////////////////////////////////////////////////////////////////////////////
//#define	ZTASK_DEBUG			1
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_TASKTIME		1
///////////////////////////////////////////////////////////////////////////////
unsigned int	ZTask::m_nTaskThreadIndex	= UINT32_MAX;
///////////////////////////////////////////////////////////////////////////////
ZTask::ZTask(CONST CHAR* sTaskName)
:ZObject(sTaskName)
,m_bTaskRun(FALSE)
,m_nTaskEventFlag(TASK_ALIVE_STATUS)
,m_nTaskThreadBegin(0)
,m_nTaskThreadBound(1)
,m_TaskMutex("TaskMutex")
,m_EventMutex("EventMutex")
,m_OSHeapElement()
,m_OSQueueElement()
,m_DefaultTaskThread(NULL)
,m_CurrentTaskThread(NULL)
,m_UsedTaskThread(NULL)
{
#ifdef	ZTASK_DEBUG
	MESSAGE_OUT(("TASK New Task(0x%08X %s)\r\n",this,sTaskName));
#endif	//ZTASK_DEBUG
	SetTaskThreadBegin(ZTaskThreadPool::GetNoBlockTaskThreadCounter());
	SetTaskThreadBound(ZTaskThreadPool::GetTaskThreadCounter());
}
ZTask::~ZTask()
{
	ZTask::Close();

	TMASSERT((m_OSHeapElement.GetHeap()==NULL));
	TMASSERT((m_OSQueueElement.GetQueue()==NULL));
	m_UsedTaskThread	= NULL;
#ifdef	ZTASK_DEBUG
	MESSAGE_OUT(("TASK Release Task(0x%08X)\r\n",this));
#endif	//ZTASK_DEBUG
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZTask::AddEvent(TASK_EVENT nEvent)
{
	BOOL				bReturn		= FALSE;
	UINT				nNewEvent	= (nEvent|TASK_ALIVE_STATUS);
	int					nIndex		= 0;
	//int					nCount		= 0;
	ZOSMutexLocker		theLocker(&m_EventMutex);

#ifdef	ZTASK_DEBUG
	MESSAGE_OUT(("ZTask::AddEvent (%d %s)\r\n",nEvent,GetObjectName()));
#endif	//ZTASK_DEBUG
	if(m_bTaskRun)
	{
		ZOS::systemor(&m_nTaskEventFlag,nNewEvent);

		TMASSERT((m_CurrentTaskThread==NULL));

		if(m_UsedTaskThread != NULL)
		{
			m_UsedTaskThread->AddTask(this);
			//MESSAGE_OUT(("ZTask::AddEvent  AddTask 1(%d %s)\r\n",nEvent,GetObjectName()));
			bReturn	= TRUE;
		}else{
			if(m_CurrentTaskThread == NULL)
			{
				m_CurrentTaskThread	= m_DefaultTaskThread;
			}
			if(m_CurrentTaskThread != NULL)
			{
				m_UsedTaskThread	= m_CurrentTaskThread;
			}else{
				TMASSERT((m_OSHeapElement.GetHeap()==NULL));
				TMASSERT((m_OSQueueElement.GetQueue()==NULL));
                ZOSMutexLocker  locker(&ZTaskThreadPool::m_mutexGetTaskThread);
#if 0
                /* fanrl comment. this logic can not ensure each thread have the same task count.
				do{
					nIndex					= ZOS::systemadd(&m_nTaskThreadIndex,1);
					nCount					= (m_nTaskThreadBound - m_nTaskThreadBegin);
					nIndex					%= nCount;
					nIndex					+= m_nTaskThreadBegin;
					m_UsedTaskThread		= ZTaskThreadPool::GetTaskThread(nIndex);
				}while(m_UsedTaskThread==NULL);
                */
#else
                UINT nMinTaskLoad = 0x7FFFFFFF;
                nIndex = m_nTaskThreadBegin;
                ZTaskThread *pTaskThread = NULL;
                for (UINT i = m_nTaskThreadBegin; i < m_nTaskThreadBound; i ++)
                {
                    pTaskThread = ZTaskThreadPool::GetTaskThread(i);
                    if (pTaskThread != NULL && pTaskThread->GetTaskThreadTaskLoad() < nMinTaskLoad)
                    {
                        nMinTaskLoad = pTaskThread->GetTaskThreadTaskLoad();
                        nIndex = i;
                    }
                }
                //LOG_DEBUG(("[ZTask::AddEvent] %s nIndex %d\r\n", GetObjectName(), nIndex));
                m_UsedTaskThread = ZTaskThreadPool::GetTaskThread(nIndex);
                if (m_UsedTaskThread != NULL)
                {
                    m_UsedTaskThread->AddTaskToArray(this);
                }
#endif
			}
			if(m_UsedTaskThread != NULL)
			{
				m_UsedTaskThread->AddTask(this);
				//MESSAGE_OUT(("ZTask::AddEvent  AddTask 2(%d %s)\r\n",nEvent,GetObjectName()));
				bReturn	= TRUE;
			}
		}
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZTask::AddEvent] (%d %s TASK STOP)!\r\n", GetErrorCodeString(20028),nEvent,GetObjectName()));
	}
	return bReturn;
}
BOOL ZTask::RemoveEvent()
{
	ZOSMutexLocker		theLocker(&m_EventMutex);
#ifdef	ZTASK_DEBUG
	MESSAGE_OUT(("ZTask::RemoveEvent(%s)\r\n",GetObjectName()));
#endif	//ZTASK_DEBUG
	if(m_UsedTaskThread != NULL)
	{
		m_UsedTaskThread->RemoveTask(this);
	}else{
		TMASSERT((m_OSHeapElement.GetHeap()==NULL));
		TMASSERT((m_OSQueueElement.GetQueue()==NULL));
		m_OSQueueElement.Remove();
		m_OSHeapElement.Remove();
	}
	return TRUE;
}
UINT ZTask::GetEvent(int nEvent)
{
	ZOSMutexLocker		theLocker(&m_EventMutex);
	UINT				nLocalEvent	= (m_nTaskEventFlag & TASK_EVENT_MASK);

	ZOS::systemsub(&m_nTaskEventFlag,nLocalEvent);

	return (nEvent | nLocalEvent);
}
BOOL ZTask::TaskRun()
{
	return (m_bTaskRun==TRUE);
}
///////////////////////////////////////////////////////////////////////////////
int ZTask::Create()
{
	m_OSQueueElement.SetObject(this);
	m_OSHeapElement.SetObject(this);

	m_bTaskRun	= TRUE;

	return 0;
}
int ZTask::Close()
{
	m_bTaskRun	= FALSE;

    {
        ZOSMutexLocker	theTaskLocker(&m_TaskMutex);

        ZOS::systeminit(&(m_nTaskEventFlag),ZTask::TASK_ALIVE_STATUS,0);

        m_OSQueueElement.SetObject(NULL);
        m_OSHeapElement.SetObject(NULL);

        ZTask::RemoveEvent();
    }

    {
        ZOSMutexLocker  locker(&ZTaskThreadPool::m_mutexGetTaskThread);
        if (m_UsedTaskThread != NULL)
        {
            m_UsedTaskThread->RemoveTaskFormArray(this);
        }
        m_UsedTaskThread	= NULL;
    }

	return 0;
}
int ZTask::Run(int nEvent)
{
	if(nEvent&TASK_TIMEOUT_EVENT)
	{
		ZOS::systeminit(&(m_nTaskEventFlag),ZTask::TASK_ALIVE_STATUS,0);
	}
	if(nEvent&TASK_KILL_EVENT)
	{
		ZOS::systeminit(&(m_nTaskEventFlag),ZTask::TASK_ALIVE_STATUS,0);
	}

	return 0;
}
int ZTask::Lock()
{
	m_TaskMutex.Lock();
	return 0;
}
int ZTask::TryLock()
{
	return m_TaskMutex.TryLock();
}
int ZTask::Unlock()
{
	m_TaskMutex.Unlock();
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZTaskThread::ZTaskThread(CONST CHAR* sThreadName)
:ZOSThread(sThreadName)
,m_TaskThreadTime(0)
,m_TaskThreadHeap("TaskThreadHeap")
,m_TaskThreadMutex("TaskThreadMutex")
,m_TaskThreadQueue()
,m_nTaskThreadLoad(0)
,m_arrayTask(10)
{
#ifdef	ZTASK_DEBUG
	MESSAGE_OUT(("TASK New Task Thread(0x%08X)\r\n",this));
#endif	//ZTASK_DEBUG
}
ZTaskThread::~ZTaskThread()
{
	TMASSERT((m_TaskThreadHeap.HeapCount() == 0));
	TMASSERT((m_TaskThreadQueue.GetLength() == 0));
#ifdef	ZTASK_DEBUG
	MESSAGE_OUT(("TASK Release Task Thread(0x%08X)\r\n",this));
#endif	//ZTASK_DEBUG
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZTaskThread::Start()
{
	return ZOSThread::Start();
}
BOOL ZTaskThread::Stop(BOOL bWait)
{
	m_TaskThreadQueue.Signal();

	ZOSThread::Stop(bWait);

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZTaskThread::AddTask(ZTask* pTask)
{
	BOOL				bReturn	= FALSE;
	ZOSMutexLocker		theLocker(&m_TaskThreadMutex);

	TMASSERT((pTask != NULL));
	if(pTask != NULL)
	{
#ifdef	ZTASK_DEBUG
		MESSAGE_OUT(("ZTaskThread::AddTask(Owner 0x%08X QueueElement 0x%08X)\r\n",this,(&(pTask->m_OSQueueElement))));
#endif	//ZTASK_DEBUG
		if(pTask->m_OSHeapElement.GetHeap() != NULL)
		{
			TMASSERT((pTask->m_OSHeapElement.GetHeap() == &m_TaskThreadHeap));
			if((pTask->m_OSHeapElement.GetHeap() == &m_TaskThreadHeap))
			{
				m_TaskThreadHeap.Remove(&(pTask->m_OSHeapElement));
				m_nTaskThreadLoad	--;
			}
		}
		if((pTask->m_OSQueueElement.GetQueue() != &m_TaskThreadQueue))
		{
			TMASSERT((pTask->m_OSQueueElement.GetQueue() == NULL));
			if(m_TaskThreadQueue.Push(&(pTask->m_OSQueueElement)) != NULL)
			{
				m_nTaskThreadLoad	++;
			}
		}
		ZTaskThreadPool::AddTaskEvent();
#ifdef	ZTASK_DEBUG
		MESSAGE_OUT(("ZTaskThread::AddTask OK(Owner 0x%08X QueueElement 0x%08X)\r\n",this,(&(pTask->m_OSQueueElement))));
#endif	//ZTASK_DEBUG
		bReturn	= TRUE;
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZTaskThread::AddTask] ERROR(pTask = NULL)!\r\n", GetErrorCodeString(20029)));
	}
	return bReturn;
}
BOOL ZTaskThread::RemoveTask(ZTask* pTask)
{
	BOOL				bReturn	= FALSE;
	ZOSMutexLocker		theLocker(&m_TaskThreadMutex);

	TMASSERT((pTask != NULL));
	if(pTask != NULL)
	{
		if(pTask->m_OSQueueElement.GetQueue() == &m_TaskThreadQueue)
		{
#ifdef	ZTASK_DEBUG
		MESSAGE_OUT(("ZTaskThread::RemoveTask(Owner 0x%08X QueueElement 0x%08X)\r\n",this,(&(pTask->m_OSQueueElement))));
#endif	//ZTASK_DEBUG
			if(m_TaskThreadQueue.Remove(&(pTask->m_OSQueueElement)))
			{
				m_nTaskThreadLoad	--;
			}
		}else{
			TMASSERT((pTask->m_OSQueueElement.GetQueue()==NULL));
		}
		if(pTask->m_OSHeapElement.GetHeap() == &m_TaskThreadHeap)
		{
#ifdef	ZTASK_DEBUG
		MESSAGE_OUT(("ZTaskThread::RemoveTask(Owner 0x%08X HeapElement 0x%08X)\r\n",this,(&(pTask->m_OSHeapElement))));
#endif	//ZTASK_DEBUG
			if(m_TaskThreadHeap.Remove(&(pTask->m_OSHeapElement)) != NULL)
			{
				m_nTaskThreadLoad	--;
			}
		}else{
			TMASSERT((pTask->m_OSHeapElement.GetHeap()==NULL));
		}
		ZTaskThreadPool::SubTaskEvent();
#ifdef	ZTASK_DEBUG
		MESSAGE_OUT(("ZTaskThread::RemoveTask OK(Owner 0x%08X Task 0x%08X Type %s)\r\n",this,pTask,pTask->GetObjectName()));
#endif	//ZTASK_DEBUG
		bReturn	= TRUE;
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZTaskThread::RemoveTask] ERROR(pTask = NULL)!\r\n", GetErrorCodeString(20030)));
		bReturn = FALSE;
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
ZTask* ZTaskThread::WaitForTask()
{
	ZTask*				pTask			= NULL;
	ZOSQueueElement*	pQueueElement	= NULL;
	ZOSHeapElement*		pHeapElement	= NULL;
	int					nMSec			= DEFAULT_NET_MAX_MTIMEOUT;

	while(!IsStop())
	{
        //LOG_DEBUG(("[ZTaskThread::WaitForTask] %s\r\n", GetObjectName()));
		m_TaskThreadTime	= ZOS::milliseconds();
		{
			ZOSMutexLocker		theLocker(&m_TaskThreadMutex);
			if(m_TaskThreadQueue.GetLength() == 0)
			{
				pHeapElement	= m_TaskThreadHeap.PeekMin();
				if(pHeapElement != NULL && pHeapElement->GetValue() <= m_TaskThreadTime)
				{
					pHeapElement		= (ZOSHeapElement*)m_TaskThreadHeap.ExtractMin();
					if(pHeapElement != NULL)
					{
						pTask				= (ZTask*)pHeapElement->GetObject();
					}
					TMASSERT((pTask != NULL));
					m_nTaskThreadLoad	--;
					if(pTask != NULL)
					{
						if(!pTask->TryLock())
						{
							pHeapElement->SetValue((m_TaskThreadTime+3));
							m_TaskThreadHeap.Insert(pHeapElement);
							pTask	= NULL;
						}
#ifdef	ZTASK_DEBUG
						MESSAGE_OUT(("ZTaskThread::WaitForTask Heap(Owner 0x%08X Element 0x%08X Task 0x%08X)\r\n",this,pHeapElement,pTask));
#endif	//ZTASK_DEBUG
					}
					break;
				}
				if(pHeapElement != NULL)
				{
					nMSec	= (int)(pHeapElement->GetValue() - m_TaskThreadTime);
					TMASSERT((nMSec>0));
				}
				if(nMSec < DEFAULT_NET_MIN_MTIMEOUT)
				{
					nMSec	= DEFAULT_NET_MIN_MTIMEOUT;
				}
				if(nMSec > DEFAULT_NET_MAX_MTIMEOUT)
				{
					nMSec	= DEFAULT_NET_MAX_MTIMEOUT;
				}
			}

			pQueueElement	= m_TaskThreadQueue.PopBlocking(&m_TaskThreadMutex,nMSec);
			if(pQueueElement != NULL)
			{
				pTask				= (ZTask*)pQueueElement->GetObject();
				m_nTaskThreadLoad	--;
				if(pTask != NULL)
				{
					if(m_TaskThreadHeap.Remove(&(pTask->m_OSHeapElement)) != NULL)
					{
						m_nTaskThreadLoad	--;
					}
					if(!pTask->TryLock())
					{
						m_TaskThreadQueue.Push(&(pTask->m_OSQueueElement));
						pTask	= NULL;
					}
#ifdef	ZTASK_DEBUG
					MESSAGE_OUT(("ZTaskThread::WaitForTask Queue(Owner 0x%08X QueueElement 0x%08X Task 0x%08X)\r\n",this,(&(pTask->m_OSQueueElement)),pTask));
#endif	//ZTASK_DEBUG
				}
				break;
			}
		}
	}
	return pTask;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZTaskThread::OnThreadStart()
{
	return TRUE;
}
BOOL ZTaskThread::OnThreadEntry()
{
	int		nTime		= 0;
	UINT64	nTaskTime	= 0;

    //LOG_DEBUG(("[ZTaskThread::OnThreadEntry] in entry %s\r\n", GetObjectName()));
	ZTask*	pTask	= ZTaskThread::WaitForTask();

	if(pTask != NULL)
	{
		if(ZObject::IsValidObject(pTask) == 0)
		{
			if((pTask->m_nTaskEventFlag&ZTask::TASK_ALIVE_STATUS)==ZTask::TASK_ALIVE_STATUS)
			{
				nTime		= pTask->Run();
				nTaskTime	= ZOS::milliseconds();
				//1,update task,add update event and insert into task thread heap
				if(nTime > 0)
				{
					ZOSMutexLocker		theLocker(&m_TaskThreadMutex);
					nTaskTime			+= nTime;
					pTask->m_OSHeapElement.SetValue(nTaskTime);
					m_TaskThreadHeap.Insert(&(pTask->m_OSHeapElement));
					m_nTaskThreadLoad	++;
					ZOS::systemor(&(pTask->m_nTaskEventFlag),(ZTask::TASK_UPDATE_EVENT|ZTask::TASK_ALIVE_STATUS));
				//2,other task
				}else{
					ZOSMutexLocker		theLocker(&m_TaskThreadMutex);
					//2.1 event,insert into task thread heap
					if(((pTask->m_nTaskEventFlag&ZTask::TASK_ALIVE_STATUS)&&(pTask->m_nTaskEventFlag&ZTask::TASK_EVENT_MASK)))
					{
						if(pTask->m_OSQueueElement.GetQueue() == NULL)
						{
							TMASSERT((pTask->m_OSHeapElement.GetHeap()==NULL));
							nTaskTime			+= DEFAULT_TASKTIME;
							pTask->m_OSHeapElement.SetValue(nTaskTime);
							m_TaskThreadHeap.Insert(&(pTask->m_OSHeapElement));
							m_nTaskThreadLoad	++;
						}else{
							TMASSERT((pTask->m_OSQueueElement.GetQueue() == &m_TaskThreadQueue));
						}
					//2.2 no event,remove from task thread pool
					}else{
#if 0 /*comment.task will never remove from thread pool, until task is delete*/
						ZOS::systeminit(&(pTask->m_nTaskEventFlag),ZTask::TASK_ALIVE_STATUS,0);
						if(pTask->m_OSQueueElement.GetQueue() == NULL)
						{
							m_TaskThreadHeap.Remove(&(pTask->m_OSHeapElement));
							TMASSERT((pTask->m_OSHeapElement.GetHeap()==NULL));
							pTask->m_CurrentTaskThread	= NULL;
							pTask->m_UsedTaskThread		= NULL;
						}else{
							TMASSERT((pTask->m_OSQueueElement.GetQueue() == &m_TaskThreadQueue));
						}
						ZTaskThreadPool::SubTaskEvent();
#endif
					} 
				}
			}
			pTask->Unlock();
		}
		else
		{
			LOG_ERROR(("ErrorCode:%s [ZTaskThread::OnThreadEntry] ERROR(Task 0x%08X Dead)!\r\n", GetErrorCodeString(20031), pTask));
		}
	}
	return TRUE;
}
BOOL ZTaskThread::OnThreadStop()
{
	return TRUE;
}
UINT ZTaskThread::GetTaskThreadTaskLoad()
{
    ZOSMutexLocker		theLocker(&m_TaskThreadMutex);

    return m_arrayTask.Count();
}

BOOL ZTaskThread::AddTaskToArray(ZTask *pTask)
{
    ZOSMutexLocker      theLocker(&m_TaskThreadMutex);
    BOOL                bFind = FALSE;

    //LOG_DEBUG(("[ZTaskThread::AddTaskToArray] in\r\n"));
    for (int i = 0; i < m_arrayTask.Count(); i ++)
    {
        if (m_arrayTask[i] != NULL && m_arrayTask[i] == pTask)
        {
            bFind = TRUE;
            break;
        }
    }

    if (!bFind)
    {
        m_arrayTask.Add(pTask);
        LOG_DEBUG(("[ZTaskThread::AddTaskToArray] count %d, %s\r\n", 
            m_arrayTask.Count(),
            GetObjectName()));
    }

    return TRUE;
}

BOOL ZTaskThread::RemoveTaskFormArray(ZTask *pTask)
{
    ZOSMutexLocker      theLocker(&m_TaskThreadMutex);
    BOOL                bFind = FALSE;

    //LOG_DEBUG(("[ZTaskThread::RemoveTaskFormArray] in\r\n"));
    for (int i = 0; i < m_arrayTask.Count(); i ++)
    {
        if (m_arrayTask[i] != NULL && m_arrayTask[i] == pTask)
        {
            m_arrayTask.Remove(i);
            bFind = TRUE;
            //LOG_DEBUG(("[ZTaskThread::RemoveTaskFormArray] nCount %d %s\r\n", 
            //    m_arrayTask.Count(),
            //    GetObjectName()));
            break;
        }
    }

    return bFind;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT			ZTaskThreadPool::m_nTaskThreadCounter			= (DEFAULT_INIT_THREAD);
UINT			ZTaskThreadPool::m_nNoBlockTaskThreadCounter	= 6;
UINT			ZTaskThreadPool::m_nTaskEventCounter			= 0;
UINT			ZTaskThreadPool::m_nTaskThreadLoad				= 0;
ZTaskThread**	ZTaskThreadPool::m_ppTaskThread					= NULL;
ZOSMutex        ZTaskThreadPool::m_mutexGetTaskThread("MutexGetTaskThread");
///////////////////////////////////////////////////////////////////////////////
void ZTaskThreadPool::Initialize(UINT nPoolThreadCount, UINT nNoBlockTaskThreadCount)
{
	m_nTaskThreadCounter = nPoolThreadCount;
	m_nNoBlockTaskThreadCounter = nNoBlockTaskThreadCount;

	CHAR	sTask[256];
	TMASSERT((m_nTaskThreadCounter > 0));
	TMASSERT((m_ppTaskThread == NULL));
	if(m_ppTaskThread == NULL)
	{
		UINT	nIndex	= 0;
		m_ppTaskThread	= NEW ZTaskThread*[m_nTaskThreadCounter];
		for(nIndex = 0; nIndex < m_nTaskThreadCounter; nIndex ++)
		{
			sprintf(sTask,"TaskThread%08d",nIndex);
			m_ppTaskThread[nIndex]	= NEW ZTaskThread(sTask);
            //LOG_DEBUG(("[ZTaskThreadPool::Initialize] sTask = %s nTotal = %d\r\n", 
            //    sTask, m_nTaskThreadCounter));
		}
	}
}
void ZTaskThreadPool::Uninitialize()
{
	if(m_ppTaskThread != NULL)
	{
		UINT	nIndex	= 0;
		for(nIndex = 0; nIndex < m_nTaskThreadCounter; nIndex ++)
		{
			SAFE_DELETE(m_ppTaskThread[nIndex]);
		}
		SAFE_DELETE_ARRAY(m_ppTaskThread);
	}
}
void ZTaskThreadPool::StartTaskThread()
{
	if(m_ppTaskThread != NULL)
	{
		UINT	nIndex	= 0;
		for(nIndex = 0; nIndex < m_nTaskThreadCounter; nIndex ++)
		{
			if(m_ppTaskThread[nIndex] != NULL)
			{
				m_ppTaskThread[nIndex]->Start();
			}
		}
	}
}
void ZTaskThreadPool::StopTaskThread()
{
	if(m_ppTaskThread != NULL)
	{
		UINT	nIndex	= 0;

		for(nIndex = 0; nIndex < m_nTaskThreadCounter; nIndex ++)
		{
			if(m_ppTaskThread[nIndex] != NULL)
			{
				m_ppTaskThread[nIndex]->Stop(FALSE);
			}
		}
		for(nIndex = 0; nIndex < m_nTaskThreadCounter; nIndex ++)
		{
			if(m_ppTaskThread[nIndex] != NULL)
			{
				m_ppTaskThread[nIndex]->m_TaskThreadQueue.GetQueueCond()->Signal();
			}
		}
#ifdef MAXIMUM_WAIT_OBJECTS
		UINT	nMax	= 0;
		UINT	nCount	= 0;
		nMax = (MAXIMUM_WAIT_OBJECTS - 1);
		for(nIndex = 0; nIndex < m_nTaskThreadCounter; nIndex += nMax)
		{
			nCount = MIN(m_nTaskThreadCounter-nIndex,nMax);
			ZOSThread::WaitForAllThread(nCount,(ZOSThread**)&m_ppTaskThread[nIndex]);
		}
#else
		ZOSThread::WaitForAllThread(m_nTaskThreadCounter,(ZOSThread**)m_ppTaskThread);
#endif //MAXIMUM_WAIT_OBJECTS

	}
}
///////////////////////////////////////////////////////////////////////////////
int ZTaskThreadPool::GetTaskThreadCounter()
{
	return m_nTaskThreadCounter;
}
int ZTaskThreadPool::SetTaskThreadCounter(int nCount)
{
	TMASSERT((nCount >= DEFAULT_INIT_THREAD));
	TMASSERT((m_ppTaskThread == NULL));
	if(nCount < DEFAULT_INIT_THREAD)
	{
		nCount	= DEFAULT_INIT_THREAD;
	}
	m_nTaskThreadCounter		= nCount;
	m_nNoBlockTaskThreadCounter	= 6;
	return m_nTaskThreadCounter;
}
int ZTaskThreadPool::GetNoBlockTaskThreadCounter()
{
	return m_nNoBlockTaskThreadCounter;
}
///////////////////////////////////////////////////////////////////////////////
int ZTaskThreadPool::AddTaskEvent()
{
	ZOS::systemadd(&m_nTaskEventCounter,1);
	m_nTaskThreadLoad	= (m_nTaskEventCounter/m_nTaskThreadCounter);
	return m_nTaskEventCounter;
}
int ZTaskThreadPool::SubTaskEvent()
{
	ZOS::systemsub(&m_nTaskEventCounter,1);
	m_nTaskThreadLoad	= (m_nTaskEventCounter/m_nTaskThreadCounter);
	return m_nTaskEventCounter;
}
///////////////////////////////////////////////////////////////////////////////
ZTaskThread* ZTaskThreadPool::GetTaskThread(UINT nIndex)
{
	TMASSERT((m_nTaskThreadCounter > 0));
	if(nIndex >= 0 && nIndex < m_nTaskThreadCounter)
	{
		TMASSERT((m_ppTaskThread != NULL));
		if(m_ppTaskThread != NULL)
		{
			//if(m_ppTaskThread[nIndex]->m_nTaskThreadLoad < (m_nTaskThreadLoad + m_nTaskThreadCounter))
			{
				return m_ppTaskThread[nIndex];
			}
		}
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
