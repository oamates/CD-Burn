#include	"ZOSFile.h"
#include	"ZOSThread.h"
///////////////////////////////////////////////////////////////////////////////
#include	<sys/stat.h>
#include	<fcntl.h>
///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32_
#include	<io.h>
#include    <shellapi.h>
#else
#include	<unistd.h>
#endif	//_WIN32_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZOSFile::ZOSFile()
:ZObject()
,m_hFile(INVALID_FILE_HANDLE)
,m_nFileFlag(0)
,m_bDir(FALSE)
,m_bEOF(FALSE)
,m_bReadOnlyFlag(FALSE)
,m_nSize(0)
,m_nPosition(0)
,m_tModData(0)
{
	memset(m_sPath,0,sizeof(m_sPath));
#ifdef USE_ZOSFILE_W
    memset(m_sPathW,0,sizeof(m_sPathW));
#endif//USE_ZOSFILE_W
}
ZOSFile::~ZOSFile()
{
	ZOSFile::Close();
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZOSFile::IsOpen()
{
	return(m_hFile!=INVALID_FILE_HANDLE);
};
BOOL ZOSFile::IsDir()
{
	return(m_bDir);
};
BOOL ZOSFile::IsReadOnly()
{
    return m_bReadOnlyFlag;
};
BOOL ZOSFile::IsReadable()
{
	return((m_hFile!=INVALID_FILE_HANDLE)&&(m_bEOF!=TRUE));
};
BOOL ZOSFile::IsWriteable()
{
	return((m_hFile!=INVALID_FILE_HANDLE)&&(!(m_nFileFlag&O_RDONLY)));
};
///////////////////////////////////////////////////////////////////////////////
int ZOSFile::Read(char* sData,int nData)
{
	int	nReturn	= 0;

	if(sData != NULL && nData > 0)
	{
		if(m_hFile != INVALID_FILE_HANDLE)
		{
			if(!m_bEOF)
			{
				nReturn = ::read(m_hFile,(char*)sData,nData);
				if(nReturn > 0)
				{
					m_nPosition	+= nReturn;
				}
				else if (nReturn < 0)
				{
					LOG_ERROR(("ErrorCode:%s [ZOSFile::Read] ERROR(File = %s Error = %s Position = %"F_NUM_64"d)!\r\n", GetErrorCodeString(20013),m_sPath,strerror(ZOSThread::GetError()),m_nPosition));
					m_bEOF		= TRUE;
				}
			}
		}
	}
	return nReturn;
}
int ZOSFile::Write(const char* sData,int nData)
{
	int	nReturn	= 0;

	if(sData != NULL && nData > 0)
	{
		if(m_hFile != INVALID_FILE_HANDLE)
		{
			nReturn	= ::write(m_hFile,(char*)sData,nData);
			if(nReturn > 0)
			{
				m_nSize		+= nReturn;
				m_nPosition	+= nReturn;
			}
			else if (nReturn < 0)
			{
				LOG_ERROR(("ErrorCode:%s [ZOSFile::Write] ERROR(File = %s Error = %s)!\r\n", GetErrorCodeString(20014),m_sPath,strerror(ZOSThread::GetError())));
			}
		}
	}
	return nReturn;
}
int ZOSFile::Seek(UINT64 nPos)
{
	int	nReturn	= -1;

	if(nPos >= 0)
	{
		if(nPos <= m_nSize)
		{
			if(m_hFile != INVALID_FILE_HANDLE)
			{
				m_nPosition	= (UINT64)::lseek64(m_hFile,nPos,SEEK_SET);
				m_bEOF		= FALSE;
				nReturn		= 0;
			}
		}
	}
	return nReturn;
}
///////////////////////////////////////////////////////////////////////////////
int ZOSFile::GetHandle()
{
	return(m_hFile);
}
char* ZOSFile::GetPath()
{
	return(m_sPath);
}
wchar_t* ZOSFile::GetPathW()
{
    return m_sPathW;
}
///////////////////////////////////////////////////////////////////////////////
UINT64 ZOSFile::GetPosition()
{
	return(m_nPosition);
}
UINT64 ZOSFile::SetPosition(UINT64 nPos)
{
	Seek(nPos);
	return(m_nPosition);
}
UINT64 ZOSFile::GetSize()
{
	return(m_nSize);
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZOSFile::FileExist(const char* sFileFullPath)
{
	if (sFileFullPath != NULL)
	{
		FILE *hFile = NULL;
		hFile = fopen(sFileFullPath, "r+b");
		if (hFile != NULL)
		{
			fclose(hFile);
			return TRUE;
		}
	}

	return FALSE;
}

UINT64 ZOSFile::FileSize(const char* sFileFullPath)
{
    ZOSFile     osFile;
    UINT64      nFileSize = 0;

    if (sFileFullPath != NULL)
    {
        if (osFile.Open(sFileFullPath))
        {
            nFileSize = osFile.GetSize();
            osFile.Close();
        }
    }

    return nFileSize;
}

BOOL ZOSFile::DelFile(const char* sFileFullPath)
{
    if (remove(sFileFullPath) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL ZOSFile::DelDir(const char* sDir)
{
#ifdef _WIN32_
    char szPath[1024];
    memset(szPath, 0, sizeof(szPath));
    sprintf(szPath, sDir);

    SHFILEOPSTRUCT fileOp={0};
    fileOp.fFlags = FOF_NOCONFIRMATION;
    fileOp.pFrom = szPath;
    fileOp.pTo = NULL;
    fileOp.wFunc = FO_DELETE;
    if (SHFileOperation(&fileOp) == 0)
    {
        return TRUE;
    }

    return FALSE;
#else

    char szPath[1024];
    sprintf(szPath, "rm -rf %s", sDir);
    if (system(szPath) == 0)
    {
        return TRUE;
    }

    return FALSE;
#endif//_WIN32_
}

#ifdef _WIN32_

BOOL ZOSFile::OpenW(const wchar_t* sName,BOOL bReadOnly,BOOL bAppend)
{
    struct  stat64  stat;

    ZOSFile::Close();
    m_hFile         = INVALID_FILE_HANDLE;
    m_nFileFlag		= O_BINARY;
    m_bDir			= FALSE;
    m_bEOF			= FALSE;
    m_nSize			= 0;
    m_nPosition		= 0;
    m_tModData		= 0;
    memset(m_sPathW,0,sizeof(m_sPathW));

    if(sName != NULL)
    {
        if(bReadOnly)
        {
            m_nFileFlag		= (O_RDONLY);
            m_bReadOnlyFlag = TRUE;
        }
        else
        {
            m_nFileFlag		= (O_RDWR);
            if(bAppend)
            {
                m_nFileFlag	|= O_APPEND;
            }
            m_bReadOnlyFlag = FALSE;
        }
        m_nFileFlag	|= O_BINARY;
        m_hFile	= ::_wopen(sName,m_nFileFlag);
        if(m_hFile != INVALID_FILE_HANDLE)
        {
            ::memset(&stat,sizeof(stat),0);
            if((::fstat64(m_hFile,&stat)) >= 0)
            {
                wcsncpy(m_sPathW,sName,MAX_FILE_PATH);
                m_nSize			= stat.st_size;
                m_nPosition		= 0;
                m_tModData		= stat.st_mtime;
                m_bDir			= S_ISDIR(stat.st_mode);
            }
            else
            {
                LOG_ERROR(("[ZOSFile::OpenW] ERROR(Error = %s)\r\n", strerror(ZOSThread::GetError())));
                ZOSFile::Close();
            }
        }
        else
        {
            //LOG_INFO(("[ZOSFile::OpenW] ERROR(Error = %s)\r\n", strerror(ZOSThread::GetError())));
        }
    }
    else
    {
        LOG_ERROR(("[ZOSFile::OpenW] parameter is not right sName=NULL!\r\n"));
    }

    return (m_hFile != INVALID_FILE_HANDLE);
}

BOOL ZOSFile::CreateW(const wchar_t* sName,BOOL bCreateNew)
{
    int				nMode		= 0;

    struct	stat64	stat;

    ZOSFile::Close();

    m_hFile			= INVALID_FILE_HANDLE;
    m_nFileFlag		= O_BINARY;
    m_bDir			= FALSE;
    m_bEOF			= FALSE;
    m_nSize			= 0;
    m_nPosition		= 0;
    m_tModData		= 0;
    memset(m_sPath, 0, sizeof(m_sPath));

    if(sName != NULL)
    {
        if(bCreateNew)
        {
            m_nFileFlag	= (O_RDWR | O_CREAT | O_TRUNC);
            nMode		= (S_IRWXU| S_IRWXG | S_IRWXO);
        }
        else
        {
            m_nFileFlag	= (O_RDWR | O_APPEND);
        }
        m_bReadOnlyFlag = FALSE;
        m_nFileFlag	|= O_BINARY;
        m_hFile	= ::_wopen(sName,m_nFileFlag,nMode);
        if(m_hFile != INVALID_FILE_HANDLE)
        {
            ::memset(&stat,sizeof(stat),0);
            if((::fstat64(m_hFile,&stat)) >= 0)
            {
                wcsncpy(m_sPathW,sName,MAX_FILE_PATH);
                m_nSize			= stat.st_size;
                m_nPosition		= 0;
                m_tModData		= stat.st_mtime;
                m_bDir			= S_ISDIR(stat.st_mode);
            }
            else
            {
                LOG_ERROR(("[ZOSFile::CreateW] ERROR(Error = %s)!\r\n", strerror(ZOSThread::GetError())));
                ZOSFile::Close();
            }
        }
        else
        {
            LOG_ERROR(("[ZOSFile::CreateW] ERROR(Error = %s)!\r\n", strerror(ZOSThread::GetError())));
        }
    }
    else
    {
        LOG_ERROR(("[ZOSFile::CreateW] parameter is not right sName=NULL!\r\n"));
    }
    return (m_hFile != INVALID_FILE_HANDLE);
}

BOOL ZOSFile::FileExistW(const wchar_t* sFileFullPath)
{
    if (sFileFullPath != NULL)
    {
        FILE *hFile = NULL;
        hFile = _wfopen(sFileFullPath, L"r+b");
        if (hFile != NULL)
        {
            fclose(hFile);
            return TRUE;
        }
    }

    return FALSE;
}

UINT64 ZOSFile::FileSizeW(const wchar_t* sFileFullPath)
{
    ZOSFile     osFile;
    UINT64      nFileSize = 0;

    if (sFileFullPath != NULL)
    {
        if (osFile.OpenW(sFileFullPath))
        {
            nFileSize = osFile.GetSize();
            osFile.Close();
        }
    }

    return nFileSize;
}

BOOL ZOSFile::DelFileW(const wchar_t* sFileFullPath)
{
    if (_wremove(sFileFullPath) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL ZOSFile::DelDirW(const wchar_t* sDir)
{
    wchar_t szPath[1024];
    memset(szPath, 0, sizeof(szPath));
    swprintf(szPath, sDir);

    SHFILEOPSTRUCTW fileOp={0};
    fileOp.fFlags = FOF_NOCONFIRMATION;
    fileOp.pFrom = szPath;
    fileOp.pTo = NULL;
    fileOp.wFunc = FO_DELETE;
    if (SHFileOperationW(&fileOp) == 0)
    {
        return TRUE;
    }

    return FALSE;
}
#endif

///////////////////////////////////////////////////////////////////////////////
BOOL ZOSFile::Open(const char* sName,BOOL bReadOnly,BOOL bAppend)
{
	int				nReturn		= 0;
#if	defined(_WIN32_)
	struct	stat64	stat;
#elif	defined(_LINUX_)
#ifdef O_LARGEFILE
	struct	stat64	stat;
#else
	struct	stat	stat;
#endif	//
#else
	struct	stat	stat;
#endif	//defined(_WIN32_)

	ZOSFile::Close();
	m_hFile			= INVALID_FILE_HANDLE;
	m_nFileFlag		= O_BINARY;
	m_bDir			= FALSE;
	m_bEOF			= FALSE;
	m_nSize			= 0;
	m_nPosition		= 0;
	m_tModData		= 0;
	memset(m_sPath,0,sizeof(m_sPath));

	if(sName != NULL)
	{
		if(bReadOnly)
		{
			m_nFileFlag		= (O_RDONLY);
            m_bReadOnlyFlag = TRUE;
		}else{
			m_nFileFlag		= (O_RDWR);
			if(bAppend)
			{
				m_nFileFlag	|= O_APPEND;
			}
            m_bReadOnlyFlag = FALSE;
		}
#if	defined(_WIN32_)
		m_nFileFlag	|= O_BINARY;
#elif	defined(_LINUX_)
#ifdef O_LARGEFILE
		m_nFileFlag	|= O_LARGEFILE;
#endif	//O_LARGEFILE
#else
#endif	//defined(_WIN32_)
		m_hFile	= ::open(sName,m_nFileFlag);
		if(m_hFile != INVALID_FILE_HANDLE)
		{
			::memset(&stat,sizeof(stat),0);
#if	defined(_WIN32_)
			 if((nReturn =::fstat64(m_hFile,&stat)) >= 0)
#elif	defined(_LINUX_)
#ifdef O_LARGEFILE
			 if((nReturn =::fstat64(m_hFile,&stat)) >= 0)
#else
			 if((nReturn =::fstat(m_hFile,&stat)) >= 0)
#endif	//
#else
			 if((nReturn =::fstat(m_hFile,&stat)) >= 0)
#endif	//defined(_WIN32_)
			 {
				strncpy(m_sPath,sName,MAX_FILE_PATH);
				m_nSize			= stat.st_size;
				m_nPosition		= 0;
				m_tModData		= stat.st_mtime;
				m_bDir			= S_ISDIR(stat.st_mode);
			 }
			 else
			 {
				 LOG_ERROR(("ErrorCode:%s [ZOSFile::Open] ERROR(Stat Name = %s;Error = %s)\r\n", GetErrorCodeString(20017),sName,strerror(ZOSThread::GetError())));
				 ZOSFile::Close();
			 }
		}
		else
		{
			LOG_INFO(("ErrorCode:%s [ZOSFile::Open] ERROR(Open Name = %s;Error = %s)\r\n", GetErrorCodeString(20016),sName,strerror(ZOSThread::GetError())));
		}
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZOSFile::Open] parameter is not right sName=NULL!\r\n", GetErrorCodeString(20015)));
	}

	return (m_hFile != INVALID_FILE_HANDLE);
}
BOOL ZOSFile::Create(const char* sName,BOOL bCreateNew)
{
	int				nReturn		= 0;
	int				nMode		= 0;
#if	defined(_WIN32_)
	struct	stat64	stat;
#elif	defined(_LINUX_)
#ifdef O_LARGEFILE
	struct	stat64	stat;
#else
	struct	stat	stat;
#endif	//
#else
	struct	stat	stat;
#endif	//defined(_WIN32_)

	ZOSFile::Close();

	m_hFile			= INVALID_FILE_HANDLE;
	m_nFileFlag		= O_BINARY;
	m_bDir			= FALSE;
	m_bEOF			= FALSE;
	m_nSize			= 0;
	m_nPosition		= 0;
	m_tModData		= 0;
	memset(m_sPath,0,sizeof(m_sPath));

	if(sName != NULL)
	{
		if(bCreateNew)
		{
			m_nFileFlag	= (O_RDWR | O_CREAT | O_TRUNC);
			nMode		= (S_IRWXU| S_IRWXG | S_IRWXO);
		}else{
			m_nFileFlag	= (O_RDWR | O_APPEND);
		}
        m_bReadOnlyFlag = FALSE;
#if	defined(_WIN32_)
		m_nFileFlag	|= O_BINARY;
#elif	defined(_LINUX_)
#ifdef O_LARGEFILE
		m_nFileFlag	|= O_LARGEFILE;
#endif	//O_LARGEFILE
#else
#endif	//defined(_WIN32_)
		m_hFile	= ::open(sName,m_nFileFlag,nMode);
		if(m_hFile != INVALID_FILE_HANDLE)
		{
			::memset(&stat,sizeof(stat),0);
#if	defined(_WIN32_)
			 if((nReturn =::fstat64(m_hFile,&stat)) >= 0)
#elif	defined(_LINUX_)
#ifdef O_LARGEFILE
			 if((nReturn =::fstat64(m_hFile,&stat)) >= 0)
#else
			 if((nReturn =::fstat(m_hFile,&stat)) >= 0)
#endif	//
#else
			 if((nReturn =::fstat(m_hFile,&stat)) >= 0)
#endif	//defined(_WIN32_)
			 {
				strncpy(m_sPath,sName,MAX_FILE_PATH);
				m_nSize			= stat.st_size;
				m_nPosition		= 0;
				m_tModData		= stat.st_mtime;
				m_bDir			= S_ISDIR(stat.st_mode);
			 }
			 else
			 {
				 LOG_ERROR(("ErrorCode:%s [ZOSFile::Create] ERROR(Stat Name = %s; Error = %s)!\r\n", GetErrorCodeString(20020),sName,strerror(ZOSThread::GetError())));
				 ZOSFile::Close();
			 }
		}
		else
		{
			LOG_ERROR(("ErrorCode:%s [ZOSFile::Create] ERROR(Open Name = %s; Error = %s)!\r\n", GetErrorCodeString(20019),sName,strerror(ZOSThread::GetError())));
		}
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZOSFile::Create] parameter is not right sName=NULL!\r\n", GetErrorCodeString(20018)));
	}
	return (m_hFile != INVALID_FILE_HANDLE);
}
BOOL ZOSFile::Close()
{
	if(m_hFile != INVALID_FILE_HANDLE)
	{
		close(m_hFile);
		m_hFile			= INVALID_FILE_HANDLE;
		m_nFileFlag		= 0;
		m_bDir			= FALSE;
		m_bEOF			= FALSE;
        m_bReadOnlyFlag = FALSE;
		m_nSize			= 0;
		m_nPosition		= 0;
		m_tModData		= 0;
		memset(m_sPath,0,sizeof(m_sPath));
		return TRUE;
	}
	//LOG_ERROR(("ErrorCode:%s [ZOSFile::Close] parameter is not right m_hFile=INVALID_FILE_HANDLE!\r\n", GetErrorCodeString(20021)));
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
