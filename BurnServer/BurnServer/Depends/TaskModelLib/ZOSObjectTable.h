///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZOSObjectTable		Header File
	Create				20100628		ZHAOTT
	Modify				20120306		ZHAOTT		ZBase
	Modify				20120702		ZHAOTT		64bits
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "ZOSHashTable.h"
#include "ZOSMutex.h"
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZOSOBJECTTABLE_H_
#define	_ZOSOBJECTTABLE_H_
///////////////////////////////////////////////////////////////////////////////
class	ZOSObject;
class	ZOSObjectKey;
class	ZOSObjectTable;
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_HASH_SIZE	DEFAULT_HASH_PRIME_3//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class	ZOSObject
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSObject();
	virtual ~ZOSObject();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	Set(CONST CHAR* sHashKey,CONST VOID* pObject);
	virtual	BOOL	Set(CONST UINT32 snHashKey,CONST VOID* pObject);
///////////////////////////////////////////////////////////////////////////////
public:
	VOID*		GetObject();
	UINT		GetReferenceCount();
	UINT32		GetHashValue();
	CHAR*		GetHashKey();
	ZOSObject*	GetNextHashEntry();
///////////////////////////////////////////////////////////////////////////////
public:
	STATIC	UINT32	HashValue(CONST CHAR* sHashKey);
///////////////////////////////////////////////////////////////////////////////
private:
	VOID*			m_pObject;
	UINT			m_nReference;
	UINT32			m_nHashValue;
	CHAR			m_sHashKey[MAX_KEY_LABEL+4];
	ZOSObject*		m_pNextHashEntry;
///////////////////////////////////////////////////////////////////////////////
public:
	friend	class	ZOSObjectKey;
	friend	class	ZOSObjectTable;
	friend	class	ZOSHashTable<ZOSObject,ZOSObjectKey>;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
class	ZOSObjectKey
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSObjectKey(CONST CHAR* sHashKey);
	ZOSObjectKey(CONST UINT32 nHashKey);
	ZOSObjectKey(ZOSObject* o);
	virtual ~ZOSObjectKey();
///////////////////////////////////////////////////////////////////////////////
public:
	UINT32		GetHashValue();
	CHAR*		GetHashKey();
///////////////////////////////////////////////////////////////////////////////
public:
	BOOL	operator	==(const ZOSObjectKey &key);
///////////////////////////////////////////////////////////////////////////////
private:
	UINT32			m_nHashValue;
	CHAR			m_sHashKey[MAX_KEY_LABEL+4];
///////////////////////////////////////////////////////////////////////////////
public:
	friend	class	ZOSObjectTable;
	friend	class	ZOSHashTable<ZOSObject,ZOSObjectKey>;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
typedef ZOSHashTable<ZOSObject,ZOSObjectKey>	ZOSObjectHashTable;
///////////////////////////////////////////////////////////////////////////////
class	ZOSObjectTable : public ZObject
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSObjectTable(CONST CHAR* sTableName,INT nSize = DEFAULT_HASH_SIZE);
	virtual ~ZOSObjectTable();
///////////////////////////////////////////////////////////////////////////////
public:
	BOOL		Register(ZOSObject* pObject);
	BOOL		Unregister(ZOSObject* pObject);
	UINT		Count();
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSObject*	GetObject(ZOSObjectKey* pKey);
///////////////////////////////////////////////////////////////////////////////
public:
	typedef	struct	_OBJECT_FIND_HANDLE_
	{
		UINT				m_nCurrentIndex;
		ZOSObject*			m_pCurrentOSObject;
	}OBJECT_FIND_HANDLE,*POBJECT_FIND_HANDLE;
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSObject*	GetFirst(POBJECT_FIND_HANDLE pHandle);
	ZOSObject*	GetNext(POBJECT_FIND_HANDLE pHandle);
	ZOSObject*	GetCurrent(POBJECT_FIND_HANDLE pHandle);
///////////////////////////////////////////////////////////////////////////////
public:
	ZOSObjectHashTable*	GetHashTable();
///////////////////////////////////////////////////////////////////////////////
private:
	ZOSObjectHashTable	m_HashTable;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZOSOBJECTTABLE_H_
///////////////////////////////////////////////////////////////////////////////