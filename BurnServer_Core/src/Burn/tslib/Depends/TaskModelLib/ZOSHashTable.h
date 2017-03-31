///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZOSHashTable		Header File
	Create				20100628		ZHAOTT
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "ZOSMemory.h"
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZOSHASHTABLE_H_
#define	_ZOSHASHTABLE_H_
///////////////////////////////////////////////////////////////////////////////
//#define	ZOSHASHTABLE_DEBUG		1
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_HASH_PRIME_1	89
#define	DEFAULT_HASH_PRIME_2	577
#define	DEFAULT_HASH_PRIME_3	1193
#define	DEFAULT_HASH_PRIME_4	5779
#define	DEFAULT_HASH_PRIME_5	10993
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template<class T,class K>
class	ZOSHashTable
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSHashTable(UINT nSize)
	{
		m_HashTable	= NULL;
		m_nSize		= 0;
		m_nMask		= 0;
		m_nEntries	= 0;
		TMASSERT((nSize > 0));
		if(nSize > 0)
		{
			m_HashTable	= NEW (T*[nSize]);
			TMASSERT((m_HashTable != NULL));
			memset(m_HashTable,0,(sizeof(T*)*nSize));
			m_nSize		= nSize;
			m_nMask		= (m_nSize - 1);
			if((m_nMask & m_nSize) != 0)
			{
				m_nMask	= 0;
			}
			m_nEntries	= 0;
		}
#ifdef	ZOSHASHTABLE_DEBUG
		MESSAGE_OUT(("ZOSHashTable Init (Size %d)\r\n",m_nSize));
#endif	//ZOSHASHTABLE_DEBUG
	}
	virtual	~ZOSHashTable()
	{
		TMASSERT((m_nEntries==0));
		SAFE_DELETE_ARRAY(m_HashTable);
#ifdef	ZOSHASHTABLE_DEBUG
		MESSAGE_OUT(("ZOSHashTable Uninit (Size %d)\r\n",m_nSize));
#endif	//ZOSHASHTABLE_DEBUG
	}
///////////////////////////////////////////////////////////////////////////////
public:
	T*	Add(T* e)
	{
		T*	t	= NULL;

		TMASSERT((e != NULL));
		if(e != NULL)
		{
			TMASSERT((e->m_pNextHashEntry == NULL));
			if(e->m_pNextHashEntry == NULL)
			{
				K	k(e);
				UINT nIndex			= GetIndex(k.GetHashValue());
				TMASSERT((nIndex<m_nSize));
				e->m_pNextHashEntry	= m_HashTable[nIndex];
				m_HashTable[nIndex]	= e;
				t					= e;
				m_nEntries			++;
#ifdef	ZOSHASHTABLE_DEBUG
				MESSAGE_OUT(("ZOSHashTable Add (Index %d Key %s)\r\n",nIndex,k.GetHashKey()));
#endif	//ZOSHASHTABLE_DEBUG
			}
		}

		return t;
	}
	T*	Remove(T* e)
	{
		T*	t	= NULL;

		TMASSERT((e != NULL));
		if(e != NULL)
		{
			K	k(e);
			T*	l		= NULL;
			UINT	nIndex	= GetIndex(k.GetHashValue());
			TMASSERT((nIndex<m_nSize));
			t			= m_HashTable[nIndex];

			while(t && t != e)
			{
				l	= t;
				t	= t->m_pNextHashEntry;
			}

			if(t)
			{
				if(l)
				{
					l->m_pNextHashEntry	= t->m_pNextHashEntry;
				}else{
					m_HashTable[nIndex]	= t->m_pNextHashEntry;
				}
				if(t == m_HashTable[nIndex])
				{
					m_HashTable[nIndex]	= NULL;
				}
				t->m_pNextHashEntry	= NULL;
				m_nEntries			--;
			}
#ifdef	ZOSHASHTABLE_DEBUG
				MESSAGE_OUT(("ZOSHashTable Remove (Index %d Key %s)\r\n",nIndex,k.GetHashKey()));
#endif	//ZOSHASHTABLE_DEBUG
		}
		return t;
	}
	T*	Map(K* k)
	{
		T*	t	= NULL;

		TMASSERT((k != NULL));
		if(k != NULL)
		{
			UINT	nIndex	= GetIndex(k->GetHashValue());
			TMASSERT((nIndex<m_nSize));
			t			= m_HashTable[nIndex];
			while(t)
			{
				K	tk(t);
				if(tk == *k)
				{
					break;
				}
				t	= t->m_pNextHashEntry;
			}
		}

		return t;
	}
///////////////////////////////////////////////////////////////////////////////
public:
	UINT	GetTableSize(){return(m_nSize);}
	UINT	GetTableEntriesCount(){return(m_nEntries);}
	T*		GetTableEntry(int i){return(m_HashTable[i]);}
///////////////////////////////////////////////////////////////////////////////
private:
	UINT	GetIndex(int nHashKey)
	{
		if(m_nMask)
		{
			return (nHashKey & m_nMask);
		}else{
			return (nHashKey % m_nSize);
		}
	}
///////////////////////////////////////////////////////////////////////////////
private:
	T**				m_HashTable; 
	UINT			m_nSize;
	unsigned int	m_nMask;
	int				m_nEntries;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZOSHASHTABLE_H_
///////////////////////////////////////////////////////////////////////////////
