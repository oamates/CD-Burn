#include "ZOSMemory.h"
///////////////////////////////////////////////////////////////////////////////
//#define	ZOSMEMORY__DEBUG		1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void* ZOSMemory::New(size_t nSize)
{
    void*	m	= malloc(nSize);
    if(m == NULL)
	{
		DEBUG_OUT(("ZOSMemory::New ERROR(Size = %d) ERRORNO %d\r\n",nSize, errno));
	}
	//LOG_DEBUG(("ZOSMemory::New %d %d\r\n", m, nSize));
#ifdef	ZOSMEMORY__DEBUG
	MESSAGE_OUT(("ZOSMemory::New %d\r\n", nSize));
#endif	//ZOSMEMORY__DEBUG
	return m;
}
void ZOSMemory::Delete(void* pMemory)
{
	if(pMemory != NULL)
	{
		free(pMemory);
	}
	//LOG_DEBUG(("ZOSMemory::Delete %d\r\n", pMemory));
#ifdef	ZOSMEMORY__DEBUG
	MESSAGE_OUT(("ZOSMemory::Delete \r\n"));
#endif	//ZOSMEMORY__DEBUG
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG_
#if	defined(_WIN32_)
#ifdef	_MINGW_
void ZOSMemory::CheckBegin()
{
}
void ZOSMemory::CheckEnd()
{
}
#else
#include <crtdbg.h>
_CrtMemState g_oldstate,g_newstate,g_diffstate;
void ZOSMemory::CheckBegin()
{
	_CrtMemCheckpoint(&g_oldstate);
}
void ZOSMemory::CheckEnd()
{
	_CrtMemCheckpoint(&g_newstate);
	if(_CrtMemDifference(&g_diffstate,&g_oldstate,&g_newstate))
	{
		_CrtMemDumpAllObjectsSince(&g_diffstate);
	}
}
#endif	//_MINGW_
#endif	//_WIN32_
#endif //_DEBUG_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
