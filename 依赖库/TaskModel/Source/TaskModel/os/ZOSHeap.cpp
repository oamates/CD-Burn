#include "ZOSHeap.h"
//#include "ZOSMemory.h"
///////////////////////////////////////////////////////////////////////////////
//#define	ZOSHEAP_DEBUG			1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSHeap::ZOSHeap(CONST CHAR* sName,int nDefaultSize)
:ZObject(sName)
,m_ppElement(NULL)
,m_nIndex(0)
,m_nSize(0)
{
	if(nDefaultSize < DEFAULT_MINI_ELEMENT)
	{
		nDefaultSize	= DEFAULT_MINI_ELEMENT;
	}
	m_ppElement					= NEW ZOSHeapElement*[nDefaultSize];
	if(m_ppElement != NULL)
	{
		memset(m_ppElement,0,(sizeof(ZOSHeapElement*)*nDefaultSize));
	}
	m_nSize						= nDefaultSize;
	m_nIndex					= 1;
#ifdef	ZOSHEAP_DEBUG
	MESSAGE_OUT(("ZOSHeap Init (%s)\r\n",GetObjectName()));
#endif	//
}
ZOSHeap::~ZOSHeap()
{
	TMASSERT((m_nIndex == 1));
	SAFE_DELETE_ARRAY(m_ppElement);
#ifdef	ZOSHEAP_DEBUG
	MESSAGE_OUT(("ZOSHeap Uninit (%s)\r\n",GetObjectName()));
#endif	//
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ZOSHeap::HeapSize()
{
	return(m_nSize);
};
int ZOSHeap::HeapCount()
{
	return(m_nIndex-1);
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSHeapElement*	ZOSHeap::Insert(ZOSHeapElement* pElement)
{
	TMASSERT((pElement != NULL));
	if(pElement != NULL)
	{
		//TMASSERT((pElement->m_pHeap == NULL));
		if(pElement->m_pHeap == NULL)
		{
			if((m_ppElement == NULL) || (m_nIndex >= m_nSize))
			{
				m_nSize						*= 2;
				ZOSHeapElement**	ppElement	= NEW ZOSHeapElement*[m_nSize];
				if(ppElement != NULL)
				{
					memset(ppElement,0,(sizeof(ZOSHeapElement*)*m_nSize));
					if((m_ppElement != NULL) && (m_nIndex > 1))
					{
						memcpy(ppElement,m_ppElement,(sizeof(ZOSHeapElement*)*m_nIndex));
					}
				}
				SAFE_DELETE_ARRAY(m_ppElement);
				m_ppElement	= ppElement;
			}
			TMASSERT((m_ppElement != NULL));
			TMASSERT((m_nIndex < m_nSize));
			if(m_ppElement != NULL && m_nIndex < m_nSize)
			{
				m_ppElement[m_nIndex]	= pElement;
				int		nSwapIndex	= m_nIndex;
				while(nSwapIndex > 1)
				{
					int	nNextSwapIndex	= (nSwapIndex >> 1);
					if(m_ppElement[nSwapIndex]->m_nValue < m_ppElement[nNextSwapIndex]->m_nValue)
					{
						ZOSHeapElement*	pTemp		= m_ppElement[nSwapIndex];
						m_ppElement[nSwapIndex]		= m_ppElement[nNextSwapIndex];
						m_ppElement[nNextSwapIndex]	= pTemp;
						nSwapIndex					= nNextSwapIndex;
					}else{
						break;
					}
				}
				pElement->m_pHeap	= this;
				m_nIndex			++;
#ifdef	ZOSHEAP_DEBUG
			MESSAGE_OUT(("ZOSHeap (%s) Insert 0x%08X\r\n",GetObjectName(),(int)(pElement)));
#endif	//
			}else{
				DEBUG_OUT(("ZOSHeap::Insert ERROR(%d < %d)\r\n",m_nIndex,m_nSize));
			}
		}else{
			//DEBUG_OUT(("ZOSHeap::Insert ERROR(0x%08X)\r\n",pElement->m_pHeap));
		}
	}else{
		DEBUG_OUT(("ZOSHeap::Insert ERROR(NULL)\r\n"));
	}
	return NULL;
}
ZOSHeapElement*	ZOSHeap::Extract(int nIndex)
{
	TMASSERT((m_ppElement != NULL));
	TMASSERT((nIndex < m_nIndex));
	if(m_ppElement != NULL && nIndex < m_nIndex)
	{
		ZOSHeapElement*	pElement	= m_ppElement[nIndex];
		TMASSERT((pElement->m_pHeap == this));
		pElement->m_pHeap	= NULL;

		m_ppElement[nIndex]	= m_ppElement[(m_nIndex - 1)];
		m_nIndex			--;

		int	nParentIndex	= nIndex;
		while(nParentIndex < m_nIndex)
		{
			int	nGreatestIndex	= nParentIndex;
			int	nLeftIndex		= (nParentIndex * 2);
			int	nRightIndex		= (nParentIndex * 2 + 1);
			if((nLeftIndex < m_nIndex) && (m_ppElement[nLeftIndex]->m_nValue <  m_ppElement[nGreatestIndex]->m_nValue))
			{
				nGreatestIndex	= nLeftIndex;
			}
			if((nRightIndex < m_nIndex) && (m_ppElement[nRightIndex]->m_nValue <  m_ppElement[nGreatestIndex]->m_nValue))
			{
				nGreatestIndex	= nRightIndex;
			}
			if(nGreatestIndex == nParentIndex)
			{
				break;
			}
			ZOSHeapElement*	pTemp		= m_ppElement[nParentIndex];
			m_ppElement[nParentIndex]	= m_ppElement[nGreatestIndex];
			m_ppElement[nGreatestIndex]	= pTemp;

			nParentIndex	= nGreatestIndex;
		}
#ifdef	ZOSHEAP_DEBUG
	MESSAGE_OUT(("ZOSHeap (%s) Extract 0x%08X\r\n",GetObjectName(),(int)(pElement)));
#endif	//
		return pElement;
	}else{
		DEBUG_OUT(("ZOSHeap::Extract ERROR(%d < %d)\r\n",m_nIndex,m_nSize));
	}
	return NULL;
}
ZOSHeapElement* ZOSHeap::ExtractMin()
{
	return(ZOSHeap::Extract(1));
}
ZOSHeapElement*	ZOSHeap::Peek(int nIndex)
{
	if (nIndex >= m_nIndex)
	{
		return NULL;
	}
	else
	{
		return m_ppElement[nIndex];
	}
}
ZOSHeapElement* ZOSHeap::PeekMin()
{
	return(m_nIndex > 1 ? m_ppElement[1] : NULL);
};
ZOSHeapElement*	ZOSHeap::Remove(ZOSHeapElement* pElement)
{
	TMASSERT((m_ppElement != NULL));
	TMASSERT((pElement != NULL));

	if(m_ppElement != NULL && m_nIndex > 1)
	{
		if(pElement != NULL)
		{
			int	nIndex	= 1;
			for(nIndex = 1; nIndex < m_nIndex; nIndex ++)
			{
				if(pElement == m_ppElement[nIndex])
				{
					break;
				}
			}
			if(nIndex < m_nIndex)
			{
				return ZOSHeap::Extract(nIndex);
			}
		}
	}

	return NULL;
}
ZOSHeapElement*	ZOSHeap::RemoveAll()
{
	TMASSERT((m_ppElement != NULL));

	if(m_ppElement != NULL && m_nIndex > 1)
	{
		int	nIndex	= 1;
		for(nIndex = 1; nIndex < m_nIndex; nIndex ++)
		{
            if (m_ppElement[nIndex] != NULL)
            {
                m_ppElement[nIndex]->m_pHeap = NULL;
                m_ppElement[nIndex] = NULL;
            }
		}
	}

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSHeapElement::ZOSHeapElement(UINT64 nValue,void* pObject)
:m_nValue(nValue)
,m_pObject(pObject)
,m_pHeap(NULL)
{
}
ZOSHeapElement::~ZOSHeapElement()
{
	TMASSERT((m_pHeap == NULL));
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT64 ZOSHeapElement::GetValue()
{
	return(m_nValue);
}
UINT64 ZOSHeapElement::SetValue(UINT64 nValue)
{
	m_nValue=nValue;
	return(m_nValue);
}
void* ZOSHeapElement::GetObject()
{
	return(m_pObject);
}
void* ZOSHeapElement::SetObject(void* pObject)
{
	m_pObject=pObject;
	return(m_pObject);
}
ZOSHeap* ZOSHeapElement::GetHeap()
{
	return(m_pHeap);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZOSHeapElement::Remove()
{
	if(m_pHeap != NULL)
	{
		m_pHeap->Remove(this);
	}
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
