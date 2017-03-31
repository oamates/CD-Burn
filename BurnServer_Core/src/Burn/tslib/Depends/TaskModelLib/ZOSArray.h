///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZOSArray			Header File
	Create				20100628		ZHAOTT
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "ZOSMemory.h"
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZOSARRAY_H_
#define	_ZOSARRAY_H_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template<class T>
class	ZOSArray
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSArray(int nSize)
	{
        //make sure element of Array is class pointer
        //T   t       = NULL;
		m_Array		= NULL;
		m_nSize		= 0;
		m_nElement	= 0;
		TMASSERT((nSize>0));
		if(nSize>0)
		{
			m_Array		= NEW (T[nSize]);
			TMASSERT((m_Array != NULL));
			memset(m_Array,0,(sizeof(T)*nSize));
			m_nSize		= nSize;
			m_nElement	= 0;
		}
	}
	virtual	~ZOSArray()
	{
		TMASSERT((m_nElement==0));
		SAFE_DELETE_ARRAY(m_Array);
	}
///////////////////////////////////////////////////////////////////////////////
public:
	int Add(T t)
	{
		return Insert(m_nElement,t);
	}
	int Insert(int n,T t)
	{
		TMASSERT((m_nElement<=m_nSize));
		TMASSERT((n<=m_nElement));
		if(m_Array==NULL||m_nElement>=m_nSize)
		{
			m_nSize		= 2 * MAX(m_nSize,1);
			T*	Array	= NEW (T[m_nSize]);
			if(Array!=NULL&&m_Array!=NULL&&m_nElement>0)
			{
				memcpy(Array,m_Array,(sizeof(T)*m_nElement));
			}
			SAFE_DELETE_ARRAY(m_Array);
			m_Array	= Array;
		}
		TMASSERT((m_Array!=NULL));
		TMASSERT((n<=m_nElement));
		if(m_Array!=NULL)
		{
			if(n<m_nElement)
			{
				memmove(&m_Array[n+1],&m_Array[n],((m_nElement-n)*sizeof(T)));
			}
			m_Array[n]	= t;
			m_nElement	++;
		}
		return n;
	}
	int	Set(int n,T t)
	{
		TMASSERT((m_nElement<=m_nSize));
		TMASSERT((n<=m_nElement));
		if(m_Array!=NULL)
		{
			if(n<m_nElement)
			{
				m_Array[n]	= t;
			}
		}
		return n;
	}
	int Remove(int n)
	{
		TMASSERT((m_nElement<=m_nSize));
		TMASSERT((n<m_nElement));
		if(m_Array!=NULL)
		{
			if(n<m_nElement)
			{
				memmove(&m_Array[n],&m_Array[n+1],((m_nElement-n-1)*sizeof(T)));
                m_nElement	--;
                return n;
			}
		}

        return -1;
	}
    // include n
    void RemoveBefore(int n)
    {
        TMASSERT((m_nElement<=m_nSize));
        TMASSERT((n<=m_nElement));
        if(m_Array!=NULL)
        {
            if(n<m_nElement)
            {
                memmove(&m_Array[0],&m_Array[n+1],((m_nElement-n-1)*sizeof(T)));
                m_nElement = m_nElement - n - 1;
            }
        }
    }
	int	RemoveAll()
	{
		TMASSERT((m_nElement<=m_nSize));
		memset(m_Array,0,(sizeof(T)*m_nSize));
		m_nElement	= 0;
		return 0;
	}
///////////////////////////////////////////////////////////////////////////////
public:
	T operator[](int n) CONST
	{
		TMASSERT((m_nElement<=m_nSize));
		TMASSERT((n<m_nElement));
		if(m_Array!=NULL)
		{
			if(n<m_nElement)
			{
				return m_Array[n];
			}
		}
		return ((T)NULL);
	}
///////////////////////////////////////////////////////////////////////////////
public:
	int	Count() CONST
	{
		return m_nElement;
	}
	int Size() CONST
	{
		return m_nElement;
	}
///////////////////////////////////////////////////////////////////////////////
private:
	T*				m_Array; 
	int				m_nSize;
	int				m_nElement;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZOSARRAY_H_
///////////////////////////////////////////////////////////////////////////////
