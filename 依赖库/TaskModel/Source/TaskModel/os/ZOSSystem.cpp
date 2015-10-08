#include "ZOSSystem.h"
#if	defined(_WIN32_)
#include <windows.h >
#elif	defined(_LINUX_)
#endif	//(_WIN32_)
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if	defined(_WIN32_)
CONST ZOSSystem::SYSTEM_DATA	ZOSSystem::m_sWindowsSystemData[]	=
{
	{0x00060001,1,"Windows 7"				,32,0},
	{0x00060001,2,"Windows Server 2008 R2"	,32,0},
	{0x00060000,1,"Windows Vista"			,32,0},
	{0x00060000,2,"Windows Server 2008"		,32,0},
	{0x00050002,1,"Windows XP"				,32,0},
	{0x00050002,2,"Windows Server 2003"		,32,0},
	{0x00050001,1,"Windows XP"				,32,0},
	{0x00050000,1,"Windows 2000"			,32,0},
	{0x00050000,2,"Windows Server 2000"		,32,0},
};
#elif	defined(_LINUX_)
#endif	//(_WIN32_)
///////////////////////////////////////////////////////////////////////////////
ZOSSystem::SYSTEM_DATA* ZOSSystem::GetOSSystemData()
{
	STATIC SYSTEM_DATA	sysd	= {0};
#if	defined(_WIN32_)
    DWORD				ver		= 0;
    DWORD				type	= 0;
    DWORD				bit		= 0;
	OSVERSIONINFOEX		osvi	= {0};
	BOOL				b64		= FALSE;
	int					i		= 0;

	osvi.dwOSVersionInfoSize	= sizeof(osvi);
	if(::GetVersionEx((LPOSVERSIONINFO)&osvi))
	{
		ver		= (((osvi.dwMajorVersion&0xFFFF)<<16)|(osvi.dwMinorVersion&0xFFFF));
		type	= (osvi.wProductType == 1 ? 1 : 2);
	}
	bit	= 32;
	if(::IsWow64Process(GetCurrentProcess(),&b64))
	{
		if(b64)
		{
			bit	= 64;
		}
	}
	for(i = 0; i < COUNT_OF(ZOSSystem::m_sWindowsSystemData); i ++)
	{
		if(	ZOSSystem::m_sWindowsSystemData[i].m_nSystemVersion == ver &&
			ZOSSystem::m_sWindowsSystemData[i].m_nSystemType == type
			)
		{
			memcpy(&sysd,&ZOSSystem::m_sWindowsSystemData[i],sizeof(SYSTEM_DATA));
			sysd.m_nSystemBit	= bit;
		}
	}
#elif	defined(_LINUX_)

#endif	//(_WIN32_)
	return &sysd;
}
CHAR* ZOSSystem::GetOSSystemName()
{
	STATIC CHAR		sName[256];
	SYSTEM_DATA*	pData		= ZOSSystem::GetOSSystemData();
	
	if(pData != NULL)
	{
		if(pData->m_nSystemVersion != 0)
		{
			sprintf(sName,"%s(%d)",pData->m_sSystemName,(int)pData->m_nSystemBit);
		}else{
			sprintf(sName,"UNKNOWN");
		}
	}
	
	return sName;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
