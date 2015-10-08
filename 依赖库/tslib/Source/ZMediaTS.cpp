///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "ZMPEGFormat.h"
#include "TSFile.h"
#include "ZMediaTS.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZMEDIATS* CreateMediaInstance()
{
	return CreateTSMediaInstance(0);
}
void CloseMediaInstance(ZMEDIATS* pInstance)
{
	CloseTSMediaInstance((ZMedia*)pInstance);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int MediaOpen(ZMEDIATS* pInstance,const char* sURL)
{
	if(pInstance != NULL)
	{
		if(((ZMPEGFormat*)pInstance)->Open(sURL))
		{
			return 0;
		}
	}
	return -1;
}
int MediaCreate(ZMEDIATS* pInstance,const char* sURL)
{
	if(pInstance != NULL)
	{
		if(((ZMPEGFormat*)pInstance)->Create(sURL))
		{
			return 0;
		}
	}
	return -1;
}
int MediaClose(ZMEDIATS* pInstance)
{
	if(pInstance != NULL)
	{
		if(((ZMPEGFormat*)pInstance)->Close())
		{
			return 0;
		}
	}
	return -1;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
unsigned int MediaAddProgram(ZMEDIATS* pInstance)
{
	if(pInstance != NULL)
	{
		return ((ZMPEGFormat*)pInstance)->AddProgram();
	}
	return 0;
}
unsigned int MediaDelProgram(ZMEDIATS* pInstance,unsigned int nPID)
{
	if(pInstance != NULL)
	{
		return ((ZMPEGFormat*)pInstance)->DelProgram(nPID);
	}
	return 0;
}
unsigned int MediaAddStream(ZMEDIATS* pInstance,unsigned int nPID,void* pHeader)
{
	if(pInstance != NULL)
	{
		return ((ZMPEGFormat*)pInstance)->AddStream(nPID,pHeader);
	}
	return 0;
}
unsigned int MediaDelStream(ZMEDIATS* pInstance,unsigned int nPID,unsigned int nSID)
{
	if(pInstance != NULL)
	{
		return ((ZMPEGFormat*)pInstance)->DelStream(nPID,nSID);
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
unsigned int MediaReadSample(ZMEDIATS* pInstance,unsigned int nSID,void* pSample)
{
	if(pInstance != NULL)
	{
		return ((ZMPEGFormat*)pInstance)->ReadSample(nSID,pSample);
	}
	return 0;
}
unsigned int MediaWriteSample(ZMEDIATS* pInstance,unsigned int nSID,void* pSample)
{
	if(pInstance != NULL)
	{
		return ((ZMPEGFormat*)pInstance)->WriteSample(nSID,pSample);
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
VOID SetMediaFileSystem(FILEOPEN fopen,FILEREAD fread,FILEWRITE fwrite,FILECLOSE fclose)
{
	TSFile::SetMediaFileSystem(fopen,fread,fwrite,fclose);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
