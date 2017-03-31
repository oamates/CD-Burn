///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZOSTable	Header File
	Create		20110310		ZHAOTT
	Modify		20120306		ZHAOTT		ZBase
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "ZOSMutex.h"
#include "ZOSHashTable.h"
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZOSTABLE_H_
#define	_ZOSTABLE_H_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template<class DT,class DK>
class ZOSTableValue
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSTableValue(DT dt,DK dk)
	:m_DataKey(dk)
	,m_DataValue(dt)
	,m_pNextHashEntry(NULL)
	{
	}
	virtual ~ZOSTableValue()
	{
	}
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSTableValue*		GetNextHashEntry()
	{
		return m_pNextHashEntry;
	}
///////////////////////////////////////////////////////////////////////////////
public:
	DK					m_DataKey;
	DT					m_DataValue;
	ZOSTableValue*		m_pNextHashEntry;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template<class DT,class DK>
class ZOSTableKey
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSTableKey(ZOSTableValue<DT,DK>* dt)
	{
		m_DataKey		= dt->m_DataKey;
		m_nHashValue	= (UINT)dt->m_DataKey;
	}
	virtual ~ZOSTableKey()
	{
	}
///////////////////////////////////////////////////////////////////////////////
public:
	DK	GetKey()
	{
		return m_DataKey;
	}
///////////////////////////////////////////////////////////////////////////////
public:
	UINT	GetHashValue()
	{
		return m_nHashValue;
	}
///////////////////////////////////////////////////////////////////////////////
public:
	BOOL operator ==(const ZOSTableKey &key)
	{
		return (m_DataKey==key.m_DataKey);
	}
///////////////////////////////////////////////////////////////////////////////
private:
	DK				m_DataKey;
	UINT			m_nHashValue;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template<class DT,class DK>
class ZOSTable
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSTable(int nSize = DEFAULT_HASH_PRIME_1)
	:m_TableHashTable(nSize)
	,m_TableMutex("TableMutex")
	{
	}
	virtual ~ZOSTable()
	{

	}
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL Add(DK dk,DT dt)
	{
		ZOSMutexLocker	locker(&m_TableMutex);
		ZTableValue		value(dt,dk);
		ZTableKey		key(&value);
		if(m_TableHashTable.Map(&key) == NULL)
		{
			ZTableValue*	pValue	= NEW ZTableValue(dt,dk);
			if(pValue != NULL)
			{
				m_TableHashTable.Add(pValue);
				return TRUE;
			}
		}
		return FALSE;
	}
	virtual	BOOL Remove(DK dk,DT dt)
	{
		ZOSMutexLocker	locker(&m_TableMutex);
		ZTableValue		value(dt,dk);
		ZTableKey		key(&value);
		ZTableValue*	pValue	= m_TableHashTable.Map(&key);
		if(pValue != NULL)
		{
			{
				if(m_TableHashTable.Remove(pValue))
				{
					SAFE_DELETE(pValue);
					return TRUE;
				}
			}
		}
		return FALSE;
	}
///////////////////////////////////////////////////////////////////////////////
public:
	BOOL Get(DK dk,DT& dt)
	{
		ZOSMutexLocker	locker(&m_TableMutex);
		ZTableValue		value(dt,dk);
		ZTableKey		key(&value);
		ZTableValue*	pValue	= m_TableHashTable.Map(&key);
		if(pValue != NULL)
		{
			{
				dt	= pValue->m_DataValue;
				return TRUE;
			}
		}
		return FALSE;
	}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
protected: 
typedef	ZOSTableValue<DT,DK>					ZTableValue;
typedef	ZOSTableKey<DT,DK>						ZTableKey;
typedef ZOSHashTable<ZTableValue,ZTableKey>		ZOSTableHashTable;
///////////////////////////////////////////////////////////////////////////////
public:
	typedef	struct	_ELEMENT_FIND_HANDLE_
	{
		typename ZOSTableHashTable::ELEMENT_FIND_HANDLE hHadle;		
	}ELEMENT_FIND_HANDLE,*PELEMENT_FIND_HANDLE;
///////////////////////////////////////////////////////////////////////////////
public:
	DT	GetFirst(PELEMENT_FIND_HANDLE pHandle)
	{
		ZTableValue*	v	= NULL;
		if(pHandle != NULL)
		{
			v	= m_TableHashTable.GetFirst(&(pHandle->hHadle));
			if(v != NULL)
			{
				return v->m_DataValue;
			}
		}
		return NULL;
	}
	DT	GetNext(PELEMENT_FIND_HANDLE pHandle)
	{
		ZTableValue*	v	= NULL;
		if(pHandle != NULL)
		{
			v	= m_TableHashTable.GetNext(&(pHandle->hHadle));
			if(v != NULL)
			{
				return v->m_DataValue;
			}
		}
		return NULL;
	}
	DT	GetCurrent(PELEMENT_FIND_HANDLE pHandle)
	{
		ZTableValue*	v	= NULL;
		if(pHandle != NULL)
		{
			v	= pHandle->hHadle.m_pCurrentElement;
			if(v != NULL)
			{
				return v->m_DataValue;
			}
		}
		return NULL;
	}
///////////////////////////////////////////////////////////////////////////////
public:
	UINT	GetTableSize(){return(m_TableHashTable.GetTableSize());};
	UINT	GetTableEntriesCount(){return(m_TableHashTable.GetTableEntriesCount());};
	DT*		GetTableEntry(UINT i){return(m_TableHashTable.GetTableEntry(i).m_DataValue);};
///////////////////////////////////////////////////////////////////////////////
protected:
	ZOSTableHashTable	m_TableHashTable;
	ZOSMutex			m_TableMutex;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZOSTABLE_H_
///////////////////////////////////////////////////////////////////////////////
