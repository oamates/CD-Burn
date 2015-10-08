#include "ZDemuxer.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZDemuxerKey::ZDemuxerKey(ZDemuxerTask* pTask)
{
	TMASSERT((pTask != NULL));
	if(pTask != NULL)
	{
		strncpy(m_sHashKey,pTask->m_sHashKey,MAX_KEY_LABEL);
		m_nHashValue	= pTask->m_nHashValue;
	}
}
ZDemuxerKey::~ZDemuxerKey()
{

}
///////////////////////////////////////////////////////////////////////////////
int ZDemuxerKey::GetHashValue()
{
	return(m_nHashValue);
}
char* ZDemuxerKey::GetHashKey()
{
	return(m_sHashKey);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZDemuxerTask::ZDemuxerTask(CONST CHAR* sTaskName)
:ZTask(sTaskName)
,m_nHashValue(0)
,m_pNextHashEntry(NULL)
{
	memset(m_sHashKey,0,sizeof(m_sHashKey));
}
ZDemuxerTask::~ZDemuxerTask()
{
}
///////////////////////////////////////////////////////////////////////////////
int ZDemuxerTask::GetHashValue()
{
	return(m_nHashValue);
}
char* ZDemuxerTask::GetHashKey()
{
	return(m_sHashKey);
}
ZDemuxerTask* ZDemuxerTask::GetNextHashEntry()
{
	return(m_pNextHashEntry);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZDemuxer::ZDemuxer()
:ZObject()
,m_DemuxerHashTable(DEFAULT_HASH_PRIME_4)
,m_DemuxerMutex("DemuxerMutex")
{
}
ZDemuxer::~ZDemuxer()
{
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZDemuxer::AddTask(ZDemuxerTask* pTask)
{
	ZOSMutexLocker	locker(&m_DemuxerMutex);
	if(pTask != NULL)
	{
		ZDemuxerKey Key(pTask);
		if(m_DemuxerHashTable.Map(&Key) == NULL)
		{
			m_DemuxerHashTable.Add(pTask);
			return TRUE;
		}
	}
	return FALSE;
}
BOOL ZDemuxer::RemoveTask(ZDemuxerTask* pTask)
{
	ZOSMutexLocker	locker(&m_DemuxerMutex);
	if(pTask != NULL)
	{
		ZDemuxerKey		Key(pTask);
		ZDemuxerTask*	pLocalTask	= m_DemuxerHashTable.Map(&Key);
		if(pLocalTask != NULL && pLocalTask == pTask)
		{
			m_DemuxerHashTable.Remove(pTask);
			return TRUE;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
ZDemuxerTask* ZDemuxer::GetTask(ZDemuxerKey* pKey)
{
	ZOSMutexLocker	locker(&m_DemuxerMutex);
	return m_DemuxerHashTable.Map(pKey);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
