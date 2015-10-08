///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZOSPool		Header File
	Create		20120227		ZHAOTT
	Modify		20120306		ZHAOTT		ZBase
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "ZOSMemory.h"
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZOSPOOL_H_
#define	_ZOSPOOL_H_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template<class T>
class ZOSPool
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSPool(UINT nSize = 4)
	:m_ppData(NULL)
	,m_nnData(0)
	,m_nnCount(0)
	,m_pFreeBegin(NULL)
	,m_pFreeEnd(NULL)
	,m_nFree(0)
	,m_pUsedBegin(NULL)
	,m_pUsedEnd(NULL)
	,m_nUsed(0)
    ,m_nDivisor(8)
	{
		Init(nSize);
	}
	virtual	~ZOSPool()
	{
		UnInit();
	}
///////////////////////////////////////////////////////////////////////////////
public:
	UINT SetSize(UINT nSize)
	{
		if(Init(nSize))
		{
			return nSize;
		}
		return 0;
	}
	UINT GetSize()
	{
		return m_nnData;
	}
	UINT GetPoolSize()
	{
		return m_nnCount;
	}
	UINT GetFreeSize()
	{
		return m_nFree;
	}
	UINT GetUsedSize()
	{
		return m_nUsed;
	}
    VOID SetDivisor(UINT nDivisor)
    {
        m_nDivisor = nDivisor;
    }
    UINT GetDivisor()
    {
        return m_nDivisor;
    }
///////////////////////////////////////////////////////////////////////////////
public:
    // GetFree, SetUsed must be used in pair
	T*	GetFree()
	{
		T*	p	= NULL;
        int i   = 0;

        if (m_nDivisor == 0)
        {
            return NULL;
        }
		//if(m_nFree > (m_nInit + m_nUsed))
		//{
		//	//Gather();
		//}
		if(m_pFreeBegin != NULL)
		{
			p						= m_pFreeBegin;
			m_pFreeBegin			= m_pFreeBegin->m_pFreeNext;
			m_nFree					--;
			if(m_pFreeEnd == p)
			{
				TMASSERT((m_pFreeBegin == NULL));
				TMASSERT((m_nFree == 0));
				m_pFreeEnd				= NULL;
			}
		}
        else
        {
            if(m_nnCount+m_nnData/m_nDivisor <= m_nnData)
            {
                // alloc memory
                for(i = 0; i < (int)(m_nnData/m_nDivisor); i ++)
                {
                    p	= Alloca();
                    if(p != NULL)
                    {
                        if(m_pFreeBegin == NULL)
                        {
                            m_pFreeBegin	= 
                                m_pFreeEnd		= p;
                        }else{
                            m_pFreeEnd->m_pFreeNext	= p;
                            m_pFreeEnd				= p;
                        }
                        m_nFree					++;
                    }
                }

                // get from free link list
                if (m_pFreeBegin != NULL)
                {
                    p						= m_pFreeBegin;
                    m_pFreeBegin			= m_pFreeBegin->m_pFreeNext;
                    m_nFree					--;
                    if(m_pFreeEnd == p)
                    {
                        TMASSERT((m_pFreeBegin == NULL));
                        TMASSERT((m_nFree == 0));
                        m_pFreeEnd				= NULL;
                    }
                }
            }
		}
		if(p != NULL)
		{
			p->m_pFreeNext		= NULL;
			p->m_pUsedNext		= NULL;
		}else{
			TMASSERT((m_pFreeBegin == NULL));
			m_pFreeEnd	= NULL;
		}
		return p;
	}
	void	SetUsed(T* p)
	{
		if(p != NULL)
		{
			if(m_pUsedBegin == NULL)
			{
				m_pUsedBegin	= p;
			}else{
				TMASSERT((m_pUsedEnd!=NULL));
			}
			if(m_pUsedEnd != NULL)
			{
				m_pUsedEnd->m_pUsedNext		= p;
			}
			p->m_pFreeNext		= NULL;
			p->m_pUsedNext		= NULL;
			m_pUsedEnd			= p;
			m_nUsed				++;
		}
	}
    // GetUsed, SetFree must be used in pair
	T*	GetUsed()
	{
		T*	p	= NULL;

		if(m_pUsedBegin != NULL)
		{
			p				= m_pUsedBegin;
			m_pUsedBegin	= m_pUsedBegin->m_pUsedNext;
			m_nUsed			--;
			if(m_pUsedEnd == p)
			{
				TMASSERT((m_pUsedBegin == NULL));
				TMASSERT((m_nUsed == 0));
				m_pUsedEnd		= NULL;
			}
		}
		if(p != NULL)
		{
			p->m_pFreeNext		= NULL;
			p->m_pUsedNext		= NULL;
		}else{
			TMASSERT((m_pUsedBegin == NULL));
			m_pUsedEnd			= NULL;
		}
		return p;
	}
    T* GetUsedBegin()
    {
        return m_pUsedBegin;
    }
	void	SetFree(T* p)
	{
		if(p != NULL)
		{
			if(m_pFreeBegin == NULL)
			{
				m_pFreeBegin	= p;
			}
			if(m_pFreeEnd != NULL)
			{
				m_pFreeEnd->m_pFreeNext	= p;
			}
			p->m_pFreeNext		= NULL;
			p->m_pUsedNext		= NULL;
			m_pFreeEnd			= p;
			m_nFree				++;
		}
	}
///////////////////////////////////////////////////////////////////////////////
protected:
	BOOL Init(UINT nSize)
	{
		UINT	i	= 0;
		T*		p	= NULL;

		UnInit();

        // make size divide m_nDivisor equal 0.
        TMASSERT((m_nDivisor!=0));
        nSize = ((nSize+m_nDivisor-1)/m_nDivisor)*m_nDivisor;
		if(nSize > 0)
		{
			if(m_ppData == NULL && m_nnData == 0)
			{
				m_ppData		= (T**)NEW T*[nSize];
				if(m_ppData != NULL)
				{
					memset(m_ppData,0,(sizeof(T*)*nSize));
				}
				m_nnData	= nSize;
				m_nnCount	= 0;
                
				if(m_nnData/m_nDivisor > 0)
				{
					for(i = 0; i < m_nnData/m_nDivisor; i ++)
					{
						p	= Alloca();
						if(p != NULL)
						{
							if(m_pFreeBegin == NULL)
							{
								m_pFreeBegin	= 
								m_pFreeEnd		= p;
							}else{
								m_pFreeEnd->m_pFreeNext	= p;
								m_pFreeEnd				= p;
							}
							m_nFree					++;
						}
					}
				}
			}
		}
		return (m_ppData != NULL);
	}
	BOOL UnInit()
	{
		UINT	i	= 0;

		if(m_ppData != NULL && m_nnData > 0)
		{
			for(i = 0; i < m_nnData; i ++)
			{
				if(m_ppData[i] != NULL)
				{
					DEL m_ppData[i];
				}
			}
			SAFE_DELETE_ARRAY(m_ppData);
			m_nnData		= 0;
			m_nnCount	= 0;
		}
		m_pFreeBegin	= NULL;
		m_pFreeEnd		= NULL;
		m_pUsedBegin	= NULL;
		m_pUsedEnd		= NULL;

		return TRUE;
	}
	BOOL Gather()
	{
		T*	p	= NULL;
		T*	pa	= NULL;
		T*	pb	= NULL;

		if(m_pFreeBegin != NULL)
		{
			pb	= NULL;
			p	= m_pFreeBegin;
			if(m_pFreeBegin != NULL)
			{
				pa	= m_pFreeBegin->m_pFreeNext;
			}
			while(p != NULL)
			{
				if(Free(p) == NULL)
				{
					if(p == m_pFreeBegin)
					{
						m_pFreeBegin	= pa;
					}
					if(p == m_pFreeEnd)
					{
						m_pFreeEnd		= pb;
					}
					if(pb != NULL)
					{
						pb->m_pFreeNext	= pa;
					}
					m_nFree					--;
					break;
				}else{
					pb	= p;
				}
				p	= pa;
				if(pa != NULL)
				{
					pa	= pa->m_pFreeNext;
				}
			}
			TMASSERT((m_pFreeBegin != NULL));
		}
		return FALSE;
	}
	T* Alloca()
	{
		if(m_nnCount < m_nnData)
		{
			TMASSERT((m_ppData[m_nnCount] == NULL));
			if(m_ppData[m_nnCount] == NULL)
			{
				m_ppData[m_nnCount]		= (T*)NEW T;
				m_nnCount				++;
				return m_ppData[(m_nnCount-1)];
			}
		}
		return NULL;
	}
	T* Free(T* p)
	{
		if(m_nnCount > 0)
		{
			if(p == m_ppData[(m_nnCount-1)])
			{
				TMASSERT((m_ppData[m_nnCount-1] != NULL));
				if(m_ppData[(m_nnCount-1)] != NULL)
				{
					DEL m_ppData[(m_nnCount-1)];
					m_ppData[(m_nnCount-1)]		= NULL;
					m_nnCount					--;
					return NULL;
				}
			}
		}

		return p;
	}
///////////////////////////////////////////////////////////////////////////////
private:
	T**					m_ppData;
	UINT				m_nnData;//max size of pool
	UINT				m_nnCount;//current allocate size of pool
	T*					m_pFreeBegin;
	T*					m_pFreeEnd;
	UINT				m_nFree;
	T*					m_pUsedBegin;
	T*					m_pUsedEnd;
	UINT				m_nUsed;
    UINT                m_nDivisor;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZOSPOOL_H_
///////////////////////////////////////////////////////////////////////////////
