///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZOSVector	Header File
	Create		20110714		ZHAOTT
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#include "Common.h"
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZOSVECTOR_H_
#define	_ZOSVECTOR_H_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template<class T>
class ZOSVector
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSVector(UINT32 nSize = 0)
	:m_nCapacity(0)
	,m_nSize(0)
	,m_pData(NULL)
	{
		Reserve(nSize);
	}
	ZOSVector(CONST ZOSVector &v)
	:m_nCapacity(0)
	,m_nSize(0)
	,m_pData(NULL)
	{
		Reserve(v.Size());
		m_nSize	= v.Size();
		for(UINT32 i = 0; i < m_nSize; i ++)
		{
			NEW (&m_pData[i]) T(v[i]);
		}
	}
	~ZOSVector()
	{
		Clear();
	}
///////////////////////////////////////////////////////////////////////////////
public:
	UINT32 Capacity() CONST
	{
		return(m_nCapacity);
	}
	UINT32 Size() CONST
	{
		return(m_nSize);
	}
	T* Data() CONST
	{
		return(m_pData);
	}
///////////////////////////////////////////////////////////////////////////////
public:
	T &operator[](UINT32 i) CONST
	{
		if(i >= 0 && i < m_nSize)
		{
			return(m_pData[i]);
		}
		return T();
	}
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSVector& operator = (CONST ZOSVector &v)
	{
		Clear();
		Reserve(v.Size());
		m_nSize	= v.Size();
		for(UINT32 i = 0; i < m_nSize; i ++)
		{
			NEW (&m_pData[i]) T(v[i]);
		}
		return(*this);
	}
///////////////////////////////////////////////////////////////////////////////
public:
	BOOL Insert(UINT32 nPosition,CONST T& Item)
	{
		return Insert(nPosition,1,Item);
	}
	BOOL Insert(UINT32 nPosition,UINT32 nCount,CONST T& Item)
	{
		UINT32	i	= 0;

		if(nPosition >= 0)
		{
			if(nCount > 0)
			{
				if(nPosition > m_nSize)
				{
					nPosition = m_nSize;
				}
				Reserve((m_nSize + nCount));
				for(i = m_nSize; i > nPosition; i --)
				{
					NEW (&m_pData[i - 1 + nCount]) T(m_pData[(i - 1)]);
					m_pData[(i - 1)].~T();
				}
				for(i = nPosition; i < (nPosition + nCount); i ++)
				{
					NEW (&m_pData[i]) T(Item);
				}
				m_nSize	+= nCount; 
				return TRUE;
			}
		}
		return FALSE;
	}
	BOOL Erase(UINT32 nPosition,UINT32 nCount = 1)
	{
		UINT32	i		= 0;
		UINT32	nStart	= 0;
		UINT32	nEnd	= 0;

		if(nPosition >= 0 && nPosition < m_nSize)
		{
			if(nCount > 0)
			{
				nStart	= nPosition;
				nEnd	= MIN(m_nSize,(nPosition + nCount));
				nCount	= (nEnd - nStart);
				for(i = nStart; i < nEnd; i ++)
				{
					m_pData[(i)].~T();
				}
				for(i = nEnd; i < m_nSize; i ++)
				{
					NEW (&m_pData[i - nCount]) T(m_pData[(i)]); 
					m_pData[(i)].~T();
				}
				m_nSize	-= nCount;
			}
		}
		return FALSE;
	}
///////////////////////////////////////////////////////////////////////////////
public:
	BOOL Clear()
	{
		UINT32	i	= 0;

		for(i = 0; i < m_nSize; i ++)
		{
			m_pData[i].~T();
		}
		m_nSize			= 0;
		m_nCapacity		= 0;
		SAFE_DELETE_ARRAY(m_pData);
		return TRUE;
	}
	BOOL Resize(UINT32 nSize,CONST T &Item = T())
	{
		UINT32	i	= 0;

		if(nSize >= 0)
		{
			if(nSize > m_nSize)
			{
				Reserve(nSize);
				for(i = m_nSize; i < nSize; i ++)
				{
					NEW (&m_pData[i]) T(Item);
				}
			}else if(nSize < m_nSize)
			{
				for(i = nSize; i < m_nSize; i ++)
				{
					m_pData[i].~T();
				}
			}
			m_nSize		= nSize;
			return TRUE;
		}
		return FALSE;
	}
///////////////////////////////////////////////////////////////////////////////
protected:
	BOOL Reserve(UINT32 nCapacity)
	{
		if(nCapacity > m_nCapacity)
		{
			UINT32	nTheCapacity	= (m_nCapacity <= 0 ? 4 : m_nCapacity);
			while(nTheCapacity < nCapacity) nTheCapacity *= 2;
			return ReserveImplement(nTheCapacity);
		}
		return FALSE;
	}
///////////////////////////////////////////////////////////////////////////////
private:
	BOOL ReserveImplement(UINT32 nCapacity)
	{
		T*		pData	= (T*)NEW T[nCapacity];
		UINT	i		= 0;
		
		if(pData != NULL)
		{
			for(i = 0; i < m_nSize; i ++)
			{
				NEW (&pData[i]) T(m_pData[i]);
			}
			for(i = 0; i < m_nSize; i ++)
			{
				m_pData[i].~T();
			}
			SAFE_DELETE_ARRAY(m_pData);
			m_pData		= pData;
			m_nCapacity	= nCapacity;
			return TRUE;
		}
		return FALSE;
	}
///////////////////////////////////////////////////////////////////////////////
private:
	UINT32		m_nCapacity;
	UINT32		m_nSize;
	T*			m_pData;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZOSVECTOR_H_
///////////////////////////////////////////////////////////////////////////////
