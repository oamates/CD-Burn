#include "ZOSQueue.h"
///////////////////////////////////////////////////////////////////////////////
//#define	ZOSQUEUE_DEBUG		1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSQueueElement::ZOSQueueElement(void* pObject)
:m_pPrev(NULL)
,m_pNext(NULL)
,m_pQueue(NULL)
,m_pObject(pObject)
{
}
ZOSQueueElement::~ZOSQueueElement()
{
	TMASSERT((m_pQueue == NULL));
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSQueueElement* ZOSQueueElement::GetPrev()
{
	return(m_pPrev);
}
ZOSQueueElement* ZOSQueueElement::GetNext()
{
	return(m_pNext);
}
ZOSQueue* ZOSQueueElement::GetQueue()
{
	return(m_pQueue);
}
void* ZOSQueueElement::GetObject()
{
	return(m_pObject);
}
void* ZOSQueueElement::SetObject(void* pObject)
{
	m_pObject=pObject;
	return(m_pObject);
}
BOOL ZOSQueueElement::IsMember(ZOSQueue* pQueue)
{
	return(pQueue!=NULL?(pQueue==m_pQueue):(m_pQueue!= NULL));
}
BOOL ZOSQueueElement::Remove()
{
	if(m_pQueue!=NULL)
	{
		m_pQueue->Remove(this);
	}
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSQueue::ZOSQueue(CONST CHAR* sQueueName)
:ZObject(sQueueName)
,m_nLength(0)
{
	m_QueueElement.m_pPrev	= &m_QueueElement;
	m_QueueElement.m_pNext	= &m_QueueElement;
#ifdef	ZOSQUEUE_DEBUG
	MESSAGE_OUT(("ZOSQueue Init (%s)\r\n",GetObjectName()));
#endif	//ZOSQUEUE_DEBUG
}
ZOSQueue::~ZOSQueue()
{
	TMASSERT((m_nLength == 0));
#ifdef	ZOSQUEUE_DEBUG
	MESSAGE_OUT(("ZOSQueue Uninit (%s)\r\n",GetObjectName()));
#endif	//ZOSQUEUE_DEBUG
}
///////////////////////////////////////////////////////////////////////////////
ZOSQueueElement* ZOSQueue::Push(ZOSQueueElement* pQueueElement)
{
	TMASSERT((pQueueElement != NULL));
#ifdef	ZOSQUEUE_DEBUG
	MESSAGE_OUT(("ZOSQueue::Push (Queue %s Len %d)\r\n",GetObjectName(),m_nLength));
#endif	//ZOSQUEUE_DEBUG
	if(pQueueElement != NULL)
	{
		TMASSERT((pQueueElement->m_pQueue != this));
		if(pQueueElement->m_pQueue != this)
		{
			TMASSERT((pQueueElement->m_pQueue == NULL));
			pQueueElement->m_pPrev			= &m_QueueElement;
			pQueueElement->m_pNext			= m_QueueElement.m_pNext;
			pQueueElement->m_pQueue			= this;
			m_QueueElement.m_pNext->m_pPrev	= pQueueElement;
			m_QueueElement.m_pNext			= pQueueElement;
			m_nLength						++;
#ifdef	ZOSQUEUE_DEBUG
			MESSAGE_OUT(("ZOSQueue::Push (Queue %s Element 0x%08X Len %d)\r\n",GetObjectName(),pQueueElement,m_nLength));
#endif	//ZOSQUEUE_DEBUG
			return pQueueElement;
		}else{
			//DEBUG_OUT(("ZOSQueue::Push ERROR(Queue %s Element 0x%08X)\r\n",GetObjectName(),pQueueElement));
		}
	}else{
		DEBUG_OUT(("ZOSQueue::Push ERROR(Queue %s Element NULL)\r\n",GetObjectName()));
	}
	return NULL;
}
ZOSQueueElement* ZOSQueue::Pop()
{
	if(m_nLength > 0)
	{
#ifdef	ZOSQUEUE_DEBUG
		MESSAGE_OUT(("ZOSQueue::Pop (Queue %s Len %d)\r\n",GetObjectName(),m_nLength));
#endif	//ZOSQUEUE_DEBUG
		ZOSQueueElement* pQueueElement	= m_QueueElement.m_pPrev;
		TMASSERT((pQueueElement != &m_QueueElement));
		pQueueElement->m_pPrev->m_pNext		= &m_QueueElement;
		m_QueueElement.m_pPrev				= pQueueElement->m_pPrev;
		pQueueElement->m_pQueue				= NULL;
		m_nLength							--;
#ifdef	ZOSQUEUE_DEBUG
		MESSAGE_OUT(("ZOSQueue::Pop (Queue %s Element 0x%08X Len %d)\r\n",GetObjectName(),pQueueElement,m_nLength));
#endif	//ZOSQUEUE_DEBUG
		return pQueueElement;
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZOSQueue::Remove(ZOSQueueElement* pQueueElement)
{
	TMASSERT((pQueueElement != NULL));
	TMASSERT((pQueueElement != &m_QueueElement));
	TMASSERT((pQueueElement->m_pQueue == this));

#ifdef	ZOSQUEUE_DEBUG
		MESSAGE_OUT(("ZOSQueue::Remove (Queue %s Len %d)\r\n",GetObjectName(),m_nLength));
#endif	//ZOSQUEUE_DEBUG
	if((pQueueElement != NULL) && (pQueueElement != &m_QueueElement))
	{
		if(pQueueElement->m_pQueue == this)
		{
			pQueueElement->m_pPrev->m_pNext	= pQueueElement->m_pNext;
			pQueueElement->m_pNext->m_pPrev	= pQueueElement->m_pPrev;
			pQueueElement->m_pQueue			= NULL;
			m_nLength						--;
#ifdef	ZOSQUEUE_DEBUG
			MESSAGE_OUT(("ZOSQueue::Remove (Queue %s Element 0x%08X Len %d)\r\n",GetObjectName(),pQueueElement,m_nLength));
#endif	//ZOSQUEUE_DEBUG
			return TRUE;
		}else{
			//DEBUG_OUT(("ZOSQueue::Remove ERROR(Queue 0x%08X)\r\n",pQueueElement->m_pQueue));
		}
	}else{
		DEBUG_OUT(("ZOSQueue::Remove ERROR(Element 0x%08X)\r\n",pQueueElement));
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSQueueElement* ZOSQueue::GetHead()
{
	return(m_nLength>0?m_QueueElement.m_pPrev:NULL);
}
ZOSQueueElement* ZOSQueue::GetTail()
{
	return(m_nLength>0?m_QueueElement.m_pNext:NULL);
}
int ZOSQueue::GetLength()
{
	return(m_nLength);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSQueueElement* ZOSQueue::GetFirst(PELEMENT_FIND_HANDLE pHandle)
{
	if(pHandle != NULL)
	{
		pHandle->m_pCurrentElement	= ZOSQueue::GetHead();
		return pHandle->m_pCurrentElement;
	}
	return NULL;
}
ZOSQueueElement* ZOSQueue::GetNext(PELEMENT_FIND_HANDLE pHandle)
{
	if(pHandle != NULL)
	{
		if(pHandle->m_pCurrentElement == ZOSQueue::GetTail())
		{
			pHandle->m_pCurrentElement	= NULL;
		}else{
			if(pHandle->m_pCurrentElement != NULL)
			{
				pHandle->m_pCurrentElement	= pHandle->m_pCurrentElement->GetPrev();
			}
		}
		return pHandle->m_pCurrentElement;
	}
	return NULL;
}
ZOSQueueElement* ZOSQueue::GetCurrent(PELEMENT_FIND_HANDLE pHandle)
{
	if(pHandle != NULL)
	{
		return pHandle->m_pCurrentElement;
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSQueueBlocking::ZOSQueueBlocking()
:ZOSQueue("OSQueueBlocking")
,m_QueueCond()
{
}
ZOSQueueBlocking::~ZOSQueueBlocking()
{
}
///////////////////////////////////////////////////////////////////////////////
ZOSQueueElement* ZOSQueueBlocking::Push(ZOSQueueElement* pQueueElement)
{
#ifdef	ZOSQUEUE_DEBUG
	MESSAGE_OUT(("ZOSQueueBlocking::Push(Queue %s Element 0x%08X Len %d)\r\n",GetObjectName(),pQueueElement,GetLength()));
#endif	//ZOSQUEUE_DEBUG
	ZOSQueueElement*	pElement	= 	NULL;
	{
		pElement	= 	ZOSQueue::Push(pQueueElement);
	}
	m_QueueCond.Signal();
	return pElement;
}
ZOSQueueElement* ZOSQueueBlocking::PopBlocking(ZOSMutex* pMutex,int nMSec)
{
	if(GetLength() == 0)
	{
		m_QueueCond.Wait(pMutex,nMSec);
	}
	ZOSQueueElement*	pElement	= NULL;

	pElement	= 	ZOSQueue::Pop();
#ifdef	ZOSQUEUE_DEBUG
	MESSAGE_OUT(("ZOSQueueBlocking::PopBlocking(Queue %s Element 0x%08X Len %d)\r\n",GetObjectName(),pElement,GetLength()));
#endif	//ZOSQUEUE_DEBUG
	return pElement;
}
BOOL ZOSQueueBlocking::Remove(ZOSQueueElement* pQueueElement)
{
	BOOL			bReturn	= FALSE;

#ifdef	ZOSQUEUE_DEBUG
		MESSAGE_OUT(("ZOSQueueBlocking::Remove(Queue %s Element 0x%08X Len %d)\r\n",GetObjectName(),pQueueElement,GetLength()));
#endif	//ZOSQUEUE_DEBUG
	bReturn	= ZOSQueue::Remove(pQueueElement);
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSCond* ZOSQueueBlocking::GetQueueCond()
{
	return(&m_QueueCond);
}
void ZOSQueueBlocking::Signal()
{
	m_QueueCond.Signal();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
