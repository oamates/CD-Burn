#include "TSFile.h"
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_TS_64BITS			1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TSFile::TSFile()
:m_MediaFileSystem(g_FileSystem)
,m_bIsCreate(FALSE)
,m_bIs64Bits(DEFAULT_TS_64BITS)
,m_MediaFile()
,m_nTime()
,m_nDuration()
,m_nTimeScale()
{
}
///////////////////////////////////////////////////////////////////////////////
TSFile::~TSFile()
{
	TSFile::Close();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL TSFile::Open(CONST CHAR* sFileName)
 {
	 BOOL	bReturn		= FALSE;

	TSFile::Close();

	if(m_MediaFileSystem.m_bInit)
	{
		if(m_MediaFileSystem.m_fOpen != NULL)
		{
			m_MediaFileSystem.m_hFile
					= (m_MediaFileSystem.m_fOpen)(sFileName,"a");
			bReturn	= TRUE;
		}
	}else{
		bReturn	= m_MediaFile.Open(sFileName);
	}

	return bReturn;
}
BOOL TSFile::Create(CONST CHAR* sFileName)
{
	 BOOL	bReturn		= FALSE;

	TSFile::Close();

	if(m_MediaFileSystem.m_bInit)
	{
		if(m_MediaFileSystem.m_fOpen != NULL)
		{
			m_MediaFileSystem.m_hFile
					= (m_MediaFileSystem.m_fOpen)(sFileName,"w");
			bReturn	= TRUE;
		}
	}else{
		bReturn	= m_MediaFile.Create(sFileName);
	}
	m_bIsCreate	= bReturn;

	return m_bIsCreate;
}
BOOL TSFile::Close()
{
	BOOL	bReturn		= FALSE;

	if(m_MediaFileSystem.m_bInit)
	{
		if(m_MediaFileSystem.m_fClose != NULL)
		{
			(m_MediaFileSystem.m_fClose)(m_MediaFileSystem.m_hFile);
			m_MediaFileSystem.m_hFile
					= (ZFILE*)INVALID_FILE_HANDLE;
			bReturn	= TRUE;
		}
	}else{
		bReturn	= m_MediaFile.Close();
	}
	m_bIsCreate	= FALSE;


	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT64 TSFile::Read(VOID* sData,UINT64 nData)
{
	if(m_MediaFileSystem.m_bInit)
	{
		if(m_MediaFileSystem.m_fRead != NULL)
		{
			return (m_MediaFileSystem.m_fRead)(sData,1,(size_t)nData,m_MediaFileSystem.m_hFile);
		}
	}else{
		return m_MediaFile.Read((CHAR*)sData,(int)nData);
	}
	return 0;
}
UINT64 TSFile::Write(VOID* sData,UINT64 nData)
{
	if(m_MediaFileSystem.m_bInit)
	{
		if(m_MediaFileSystem.m_fWrite != NULL)
		{
			return (m_MediaFileSystem.m_fWrite)(sData,1,(size_t)nData,m_MediaFileSystem.m_hFile);
		}
	}else{
		return m_MediaFile.Write((CHAR*)sData,(int)nData);
	}
	return 0;
}
UINT64 TSFile::Seek(UINT64 nSeek)
{
	if(m_MediaFileSystem.m_bInit)
	{
		DEBUG_OUT(("TSFile::Seek No Implement\r\n"));
	}else{
		return m_MediaFile.Seek(nSeek);
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT64 TSFile::GetPosition()
{
	if(m_MediaFileSystem.m_bInit)
	{
		DEBUG_OUT(("TSFile::GetPosition No Implement\r\n"));
	}else{
		return m_MediaFile.GetPosition();
	}
	return 0;
}
UINT64 TSFile::GetSize()
{
	if(m_MediaFileSystem.m_bInit)
	{
		DEBUG_OUT(("TSFile::GetSize No Implement\r\n"));
	}else{
		return m_MediaFile.GetSize();
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL TSFile::IsCreate()
{
	return m_bIsCreate;
}
BOOL TSFile::Is64Bits()
{
	return m_bIs64Bits;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT8 TSFile::ReadUINT8()
{
	BYTE	nValue	= 0;
	if(TSFile::Read((char*)&nValue,1) == 1)
	{
		return nValue;
	}
	return 0;
}
UINT16 TSFile::ReadUINT16()
{
	BYTE	nValue[2];
	if(TSFile::Read((char*)nValue,2) == 2)
	{
		return (((nValue[0]&0xFF)<<8)|(nValue[1]&0xFF));
	}
	return 0;
}
UINT32 TSFile::ReadUINT24()
{
	BYTE	nValue[3];
	if(TSFile::Read((char*)nValue,3) == 3)
	{
		return (((nValue[1]&0xFF)<<16)|((nValue[2]&0xFF)<<8)|(nValue[3]&0xFF));
	}
	return 0;
}
UINT32 TSFile::ReadUINT32()
{
	BYTE	nValue[4];
	if(TSFile::Read((char*)nValue,4) == 4)
	{
		return (((nValue[0]&0xFF)<<24)|((nValue[1]&0xFF)<<16)|((nValue[2]&0xFF)<<8)|(nValue[3]&0xFF));
	}
	return 0;
}
UINT64 TSFile::ReadUINT64()
{
	BYTE	nValue[8];
	if(TSFile::Read((char*)nValue,8) == 8)
	{
		UINT64	nValue64	= 0;
		UINT64	nTempValue64= 0;

		nTempValue64		= (nValue[0]&0xFF);
		nValue64			|= (nTempValue64<<56);
		nTempValue64		= (nValue[1]&0xFF);
		nValue64			|= (nTempValue64<<48);
		nTempValue64		= (nValue[2]&0xFF);
		nValue64			|= (nTempValue64<<40);
		nTempValue64		= (nValue[3]&0xFF);
		nValue64			|= (nTempValue64<<32);
		nTempValue64		= (nValue[4]&0xFF);
		nValue64			|= (nTempValue64<<24);
		nTempValue64		= (nValue[5]&0xFF);
		nValue64			|= (nTempValue64<<16);
		nTempValue64		= (nValue[6]&0xFF);
		nValue64			|= (nTempValue64<<8);
		nTempValue64		= (nValue[7]&0xFF);
		nValue64			|= (nTempValue64);

		return nValue64;
	}
	return 0;
}
UINT64 TSFile::ReadBytes(BYTE* pData,UINT64 nData)
{
	if(pData != NULL && nData > 0)
	{
		return TSFile::Read((char*)pData,nData);
	}
	return 0;
}
UINT8 TSFile::WriteUINT8(UINT8 nValue)
{
	BYTE	sValue[1];

	sValue[0]	= ((nValue    )&0xFF);
	if(TSFile::Write((char*)sValue,1) == 1)
	{
		return nValue;
	}
	return 0;
}
UINT16 TSFile::WriteUINT16(UINT16 nValue)
{
	BYTE	sValue[2];

	sValue[0]	= ((nValue>> 8)&0xFF);
	sValue[1]	= ((nValue    )&0xFF);
	if(TSFile::Write((char*)sValue,2) == 2)
	{
		return nValue;
	}
	return 0;
}
UINT32 TSFile::WriteUINT24(UINT32 nValue)
{
	BYTE	sValue[3];

	sValue[0]	= ((nValue>>16)&0xFF);
	sValue[1]	= ((nValue>> 8)&0xFF);
	sValue[2]	= ((nValue    )&0xFF);
	if(TSFile::Write((char*)sValue,3) == 3)
	{
		return nValue;
	}
	return 0;
}
UINT32 TSFile::WriteUINT32(UINT32 nValue)
{
	BYTE	sValue[4];

	sValue[0]	= ((nValue>>24)&0xFF);
	sValue[1]	= ((nValue>>16)&0xFF);
	sValue[2]	= ((nValue>> 8)&0xFF);
	sValue[3]	= ((nValue    )&0xFF);
	if(TSFile::Write((char*)sValue,4) == 4)
	{
		return nValue;
	}
	return 0;
}
UINT64 TSFile::WriteUINT64(UINT64 nValue)
{
	BYTE	sValue[8];

	sValue[0]	= ((nValue>>56)&0xFF);
	sValue[1]	= ((nValue>>48)&0xFF);
	sValue[2]	= ((nValue>>40)&0xFF);
	sValue[3]	= ((nValue>>32)&0xFF);
	sValue[4]	= ((nValue>>24)&0xFF);
	sValue[5]	= ((nValue>>16)&0xFF);
	sValue[6]	= ((nValue>> 8)&0xFF);
	sValue[7]	= ((nValue    )&0xFF);
	if(TSFile::Write((char*)sValue,8) == 8)
	{
		return nValue;
	}
	return 0;
}
UINT64 TSFile::WriteBytes(BYTE* pData,UINT64 nData)
{
	if(pData != NULL && nData > 0)
	{
		return TSFile::Write((char*)pData,nData);
	}
	return 0;
}
UINT64 TSFile::SkipBytes(UINT64 nData)
{
	UINT64	nPos	= 0;
	if(nData > 0)
	{
		nPos	= TSFile::GetPosition();
		nPos	+= nData;
		if(TSFile::Seek(nPos) == nPos)
		{
			return nData;
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TSFile::FILE_SYSTEM		TSFile::g_FileSystem	= {0,0,0,0,0,0};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
VOID TSFile::SetMediaFileSystem(FILEOPEN fopen,FILEREAD fread,FILEWRITE fwrite,FILECLOSE fclose)
{
	if(fopen != NULL || fread != NULL || fwrite != NULL || fclose != NULL)
	{
		TSFile::g_FileSystem.m_bInit	= TRUE;
		TSFile::g_FileSystem.m_fOpen	= fopen;
		TSFile::g_FileSystem.m_fRead	= fread;
		TSFile::g_FileSystem.m_fWrite	= fwrite;
		TSFile::g_FileSystem.m_fClose	= fclose;
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
