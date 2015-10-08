#include	"ZIOBase.h"
#include	<fcntl.h>
///////////////////////////////////////////////////////////////////////////////
ZIOBase::ZIOBase()
:ZOSFile()
,m_sStreamBuffer(NULL)
,m_nStreamBuffer(0)
{
}
ZIOBase::~ZIOBase()
{
	ZIOBase::StreamClose();
}
///////////////////////////////////////////////////////////////////////////////
int	ZIOBase::StreamCreate()
{
	ZIOBase::StreamClose();

	SAFE_DELETE_ARRAY(m_sStreamBuffer);
	m_sStreamBuffer	= NEW char[DEFAULT_IOBASE_BUFFER_SIZE];
	memset(m_sStreamBuffer,0,DEFAULT_IOBASE_BUFFER_SIZE);
	m_nStreamBuffer	= 0;

	return 0;
}
int	ZIOBase::StreamClose()
{
	if(m_sStreamBuffer != NULL && m_nStreamBuffer > 0)
	{
		ZOSFile::Write(m_sStreamBuffer,m_nStreamBuffer);
	}
	SAFE_DELETE_ARRAY(m_sStreamBuffer);
	m_nStreamBuffer	= 0;

	return 0;
}
///////////////////////////////////////////////////////////////////////////////
int	ZIOBase::StreamRead(char* sdata,int ndata)
{
	return ZOSFile::Read(sdata,ndata);
}
int	ZIOBase::StreamWrite(const char* sdata,int ndata)
{
	int	nReturn	= 0;

	if(m_sStreamBuffer != NULL)
	{
		if((m_nStreamBuffer + ndata) > DEFAULT_IOBASE_BUFFER_SIZE)
		{
			if(m_nStreamBuffer > 0)
			{
				ZOSFile::Write(m_sStreamBuffer,m_nStreamBuffer);
			}
			memset(m_sStreamBuffer,0,sizeof(m_sStreamBuffer));
			m_nStreamBuffer	= 0;
			if(ndata > (DEFAULT_IOBASE_BUFFER_SIZE/2))
			{
				nReturn			= ZOSFile::Write(sdata,ndata);
			}else{
				memcpy((m_sStreamBuffer+m_nStreamBuffer),sdata,ndata);
				m_nStreamBuffer	+= ndata;
				nReturn			= ndata;
			}
		}else{
			memcpy((m_sStreamBuffer+m_nStreamBuffer),sdata,ndata);
			m_nStreamBuffer	+= ndata;
			nReturn			= ndata;
		}
	}else{
		nReturn			= ZOSFile::Write(sdata,ndata);
	}
	return nReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZIOBase::StreamSeek(UINT64 nPos)
{
	return (ZOSFile::Seek(nPos)==0);
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZIOBase::StreamReadable()
{
	return ZIOBase::IsReadable();
}
BOOL ZIOBase::StreamWriteable()
{
	return ZIOBase::IsWriteable();
}
BOOL ZIOBase::StreamError()
{
	return FALSE;
}
BOOL ZIOBase::RequestEvent(UINT nEvent)
{
	return FALSE;
}
BOOL ZIOBase::RemoveEvent(UINT nEvent)
{
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
UINT64 ZIOBase::GetPosition()
{
	return ZOSFile::GetPosition();
}
UINT64 ZIOBase::SetPosition(UINT64 nPos)
{
	ZOSFile::Seek(nPos);
	return ZOSFile::GetPosition();
}
UINT64 ZIOBase::GetSize()
{
	return ZOSFile::GetSize();
}
///////////////////////////////////////////////////////////////////////////////
