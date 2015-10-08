#include "ZOSObjectTable.h"
#include "ZMath.h"
///////////////////////////////////////////////////////////////////////////////
//#define	ZOSOBJECTTABLE_DEBUG	1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSObject::ZOSObject()
:m_pObject(NULL)
,m_nReference(0)
,m_nHashValue(0)
,m_pNextHashEntry(NULL)
{
	memset(m_sHashKey,0,sizeof(m_sHashKey));
}
ZOSObject::~ZOSObject()
{
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZOSObject::Set(CONST CHAR* sHashKey,CONST VOID* pObject)
{
	if(sHashKey != NULL && pObject != NULL)
	{
		strncpy(m_sHashKey,sHashKey,MAX_KEY_LABEL);
		m_pObject		= (VOID*)pObject;
		m_nHashValue	= ZOSObject::HashValue(m_sHashKey);

		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZOSObject::Set(CONST UINT32 nHashKey,CONST VOID* pObject)
{
	if(pObject != NULL)
	{
		sprintf(m_sHashKey,"%XL",nHashKey);
		m_pObject		= (VOID*)pObject;
		m_nHashValue	= nHashKey;

		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
VOID* ZOSObject::GetObject()
{
	return(m_pObject);
}
UINT ZOSObject::GetReferenceCount()
{
	return(m_nReference);
}
UINT32 ZOSObject::GetHashValue()
{
	return(m_nHashValue);
}
CHAR* ZOSObject::GetHashKey()
{
	return(m_sHashKey);
}
ZOSObject* ZOSObject::GetNextHashEntry()
{
	return(m_pNextHashEntry);
}
///////////////////////////////////////////////////////////////////////////////
UINT32 ZOSObject::HashValue(CONST CHAR* sHashKey)
{
	TMASSERT((sHashKey != NULL));
	TMASSERT((strlen(sHashKey)>0));

	return ZMath::Hash(sHashKey);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSObjectKey::ZOSObjectKey(CONST CHAR* sHashKey)
{
	TMASSERT((sHashKey != NULL));
	if(sHashKey != NULL)
	{
		strncpy(m_sHashKey,sHashKey,MAX_KEY_LABEL);
		m_nHashValue	= ZOSObject::HashValue(m_sHashKey);
	}
}
ZOSObjectKey::ZOSObjectKey(CONST UINT32 nHashKey)
{
	sprintf(m_sHashKey,"%XL",nHashKey);
	m_nHashValue	= nHashKey;
}
ZOSObjectKey::ZOSObjectKey(ZOSObject* o)
{
	TMASSERT((o != NULL));
	if(o != NULL)
	{
		strncpy(m_sHashKey,o->m_sHashKey,MAX_KEY_LABEL);
		m_nHashValue	= o->m_nHashValue;
	}
}
ZOSObjectKey::~ZOSObjectKey()
{
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT32 ZOSObjectKey::GetHashValue()
{
	return(m_nHashValue);
}
CHAR* ZOSObjectKey::GetHashKey()
{
	return(m_sHashKey);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZOSObjectKey::operator ==(const ZOSObjectKey &key)
{
	return (strcmp(m_sHashKey,key.m_sHashKey) == 0);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSObjectTable::ZOSObjectTable(CONST CHAR* sTableName,INT nSize)
:ZObject(sTableName)
,m_HashTable(nSize)
{
#ifdef	ZOSOBJECTTABLE_DEBUG
	MESSAGE_OUT(("ZOSObjectTable Init (%s)\n",GetObjectName()));
#endif	//ZOSOBJECTTABLE_DEBUG
}
ZOSObjectTable::~ZOSObjectTable()
{
#ifdef	ZOSOBJECTTABLE_DEBUG
	MESSAGE_OUT(("ZOSObjectTable Uninit (%s)\n",GetObjectName()));
#endif	//ZOSOBJECTTABLE_DEBUG
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZOSObjectTable::Register(ZOSObject* pObject)
{
	BOOL			bReturn	= FALSE;

	TMASSERT((pObject != NULL));
	if(pObject != NULL)
	{
		TMASSERT((pObject->m_nReference == 0));
		TMASSERT((strlen(pObject->m_sHashKey) > 0));
		if(strlen(pObject->m_sHashKey) > 0)
		{
			ZOSObjectKey	Key(pObject);
			ZOSObject*		FirstObject	= m_HashTable.Map(&Key);
			if(FirstObject == NULL)
			{
				if(m_HashTable.Add(pObject))
				{
#ifdef	ZOSOBJECTTABLE_DEBUG
					MESSAGE_OUT(("ZOSObjectTable::Register(Object 0x%08X)\r\n",pObject));
#endif	//ZOSOBJECTTABLE_DEBUG
					bReturn	= TRUE;
				}else{
					DEBUG_OUT(("ZOSObjectTable::Register ERROR(m_HashTable.Add)\n"));
				}
			}
		}else{
			DEBUG_OUT(("ZOSObjectTable::Register ERROR(Key %s)\n",pObject->m_sHashKey));
		}
	}else{
		DEBUG_OUT(("ZOSObjectTable::Register ERROR(NULL)\n"));
	}

	return bReturn;
}
BOOL ZOSObjectTable::Unregister(ZOSObject* pObject)
{
	BOOL			bReturn	= FALSE;

	TMASSERT((pObject != NULL));
	if(pObject != NULL)
	{
		if(m_HashTable.Remove(pObject) != NULL)
		{
#ifdef	ZOSOBJECTTABLE_DEBUG
			MESSAGE_OUT(("ZOSObjectTable::Unregister(Object 0x%08X)\r\n",pObject));
#endif	//ZOSOBJECTTABLE_DEBUG
			bReturn	= TRUE;
		}else{
			//DEBUG_OUT(("ZOSObjectTable::Unregister ERROR(m_HashTable.Remove)\n"));
		}
	}else{
		DEBUG_OUT(("ZOSObjectTable::Unregister ERROR(NULL)\n"));
	}

	return bReturn;
}
UINT ZOSObjectTable::Count()
{
	return m_HashTable.GetTableEntriesCount();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSObject* ZOSObjectTable::GetObject(ZOSObjectKey* pKey)
{
	ZOSObject*		pObject	= NULL;
	if(pKey != NULL)
	{
		pObject	= m_HashTable.Map(pKey);
		while(pObject != NULL)
		{
			if(strcmp(pObject->GetHashKey(),pKey->GetHashKey()) == 0)
			{
				break;
			}
			pObject	= pObject->GetNextHashEntry();
		}
	}
	return pObject;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSObject* ZOSObjectTable::GetFirst(POBJECT_FIND_HANDLE pHandle)
{
	if(pHandle != NULL)
	{
		memset(pHandle,0,sizeof(OBJECT_FIND_HANDLE));
		for(pHandle->m_nCurrentIndex = 0; pHandle->m_nCurrentIndex < m_HashTable.GetTableSize(); pHandle->m_nCurrentIndex ++)
		{
			pHandle->m_pCurrentOSObject	= m_HashTable.GetTableEntry(pHandle->m_nCurrentIndex);
			if(pHandle->m_pCurrentOSObject != NULL)
			{
				break;
			}
		}
		return pHandle->m_pCurrentOSObject;
	}
	return NULL;
}
ZOSObject* ZOSObjectTable::GetNext(POBJECT_FIND_HANDLE pHandle)
{
	if(pHandle != NULL)
	{
		pHandle->m_pCurrentOSObject	= pHandle->m_pCurrentOSObject->m_pNextHashEntry;
		if(pHandle->m_pCurrentOSObject == NULL)
		{
			for(pHandle->m_nCurrentIndex = (pHandle->m_nCurrentIndex + 1); pHandle->m_nCurrentIndex < m_HashTable.GetTableSize(); pHandle->m_nCurrentIndex ++)
			{
				pHandle->m_pCurrentOSObject	= m_HashTable.GetTableEntry(pHandle->m_nCurrentIndex);
				if(pHandle->m_pCurrentOSObject != NULL)
				{
					break;
				}
			}
		}
		return pHandle->m_pCurrentOSObject;
	}
	return NULL;;
}
ZOSObject* ZOSObjectTable::GetCurrent(POBJECT_FIND_HANDLE pHandle)
{
	if(pHandle != NULL)
	{
		return pHandle->m_pCurrentOSObject;
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSObjectHashTable* ZOSObjectTable::GetHashTable()
{
	return(&m_HashTable);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
