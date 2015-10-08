#include "ZSessionStream.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZSessionStream::ZSessionStream()
:ZObject()
,m_SessionStreamMutex("SessionStreamMutex")
,m_pSessionStream(NULL)
,m_sBuffer(NULL)
,m_nBuffer(0)
,m_nSize(0)
{
	m_sBuffer	= NEW char[DEFAULT_SESSION_BUFFER_SIZE];
	memset(m_sBuffer, 0 ,DEFAULT_SESSION_BUFFER_SIZE);
	m_nBuffer	= 0;
	if(m_sBuffer != NULL)
	{
		m_nSize		= DEFAULT_SESSION_BUFFER_SIZE;
	}
}
ZSessionStream::~ZSessionStream()
{
	m_pSessionStream	= NULL;
	SAFE_DELETE_ARRAY(m_sBuffer);
}
///////////////////////////////////////////////////////////////////////////////
void ZSessionStream::AttachStream(ZBaseStream* pStream)
{
	ZOSMutexLocker	locker(&m_SessionStreamMutex);

	m_pSessionStream	= pStream;
};
void ZSessionStream::DetachStream(ZBaseStream* pStream)
{
	ZOSMutexLocker	locker(&m_SessionStreamMutex);

	if(pStream==NULL)
	{
		m_pSessionStream	= NULL;
	}else if(m_pSessionStream==pStream)
	{
		m_pSessionStream	= NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////
ZBaseStream* ZSessionStream::GetStream()
{
	return(m_pSessionStream);
};
///////////////////////////////////////////////////////////////////////////////
char* ZSessionStream::GetBuffer()
{
	return(m_sBuffer);
};
int ZSessionStream::GetBufferLength()
{
	return(m_nBuffer);
};
int ZSessionStream::GetBufferSize()
{
	return(m_nSize);
};
int ZSessionStream::MoveBuffer(int nLen)
{
	if(nLen < m_nBuffer)
	{
		memmove(m_sBuffer,&m_sBuffer[nLen],(m_nBuffer-nLen));
		m_nBuffer	-= nLen;
		memset(&m_sBuffer[m_nBuffer],0,(m_nSize-m_nBuffer));
	}else{
		m_nBuffer	= 0;
		memset(m_sBuffer,0,m_nSize);
	}
	return m_nBuffer;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZSessionStream::RequestEvent()
{
	if(m_pSessionStream != NULL)
	{
		return m_pSessionStream->RequestEvent(ZEvent::EVENT_READ);
	}
	return FALSE;
}
BOOL ZSessionStream::RemoveEvent()
{
	if(m_pSessionStream != NULL)
	{
		return m_pSessionStream->RemoveEvent(ZEvent::EVENT_READ);
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
int ZSessionStream::ReadStream()
{
	int				nError	= 0;
	int				nRead	= 0;
	ZOSMutexLocker	locker(&m_SessionStreamMutex);

	if(m_pSessionStream != NULL)
	{
		if(m_nBuffer == m_nSize)
		{
			int		nSize	= MAX(m_nSize*2,m_nBuffer*2);
			char*	sBuffer	= NEW char[nSize];
			memset(sBuffer, 0, nSize);
			if(sBuffer != NULL)
			{
				memcpy(sBuffer,m_sBuffer,m_nBuffer);
				SAFE_DELETE_ARRAY(m_sBuffer);
				m_sBuffer	= sBuffer;
				m_nSize		= nSize;
			}
		}
		if(m_nBuffer < m_nSize)
		{
			if(m_pSessionStream->StreamReadable())
			{
                nRead	= m_pSessionStream->StreamRead(&m_sBuffer[m_nBuffer],(m_nSize-m_nBuffer));
				if(nRead > 0)
				{
					m_nBuffer	+= nRead;
					nError	= SESSION_STREAM_STATE_READ;
				}
				else if(nRead == ZSOCKET_CLOSED)
				{
					nError	= SESSION_STREAM_STATE_EOF;
					//LOG_ERROR(("ErrorCode:%s [ZSessionStream::ReadStream] ERROR(stream is closed, read size = 0)!\r\n", GetErrorCodeString(24000)));
				}
				else if(nRead == ZSOCKET_TIMEOUT)
				{
					nError	= SESSION_STREAM_STATE_READ;
					LOG_ERROR(("ErrorCode:%s [ZSessionStream::ReadStream] ERROR(stream is timeout, read size = -1)!\r\n", GetErrorCodeString(24001)));
				}
				else
				{
					nError	= SESSION_STREAM_STATE_ERROR;
					LOG_ERROR(("ErrorCode:%s [ZSessionStream::ReadStream] ERROR(stream read error, read size < -1)!\r\n", GetErrorCodeString(24002)));
				}
			}
			else if (m_pSessionStream->StreamError())
			{
				nError = SESSION_STREAM_STATE_ERROR;
				LOG_ERROR(("ErrorCode:%s [ZSessionStream::ReadStream] stream error!\r\n", GetErrorCodeString(24004)));
			}
			else
			{
				nError	= SESSION_STREAM_STATE_READ;
				//LOG_DEBUG(("ErrorCode:%s [ZSessionStream::ReadStream] ERROR(stream read error, can not be read)!\r\n", GetErrorCodeString(24003)));
				//LOG_DEBUG(("[ZSessionStream::ReadStream] (stream read error, can not be read)!\r\n"));
			}
		}
		else
		{
			nError	= SESSION_STREAM_STATE_ERROR;
			LOG_ERROR(("ErrorCode:%s [ZSessionStream::ReadStream] ERROR(read size %d less than require)!\r\n", GetErrorCodeString(24004), m_nBuffer));
		}
	}
	else
	{
		nError	= SESSION_STREAM_STATE_INVALID;
		LOG_ERROR(("ErrorCode:%s [ZSessionStream::ReadStream] ERROR(m_pSessionStream = NULL)!\r\n", GetErrorCodeString(24005)));
	}
	return nError;
}
int ZSessionStream::SendStream(const char* sData,int nData)
{
	int				nError	= 0;
	int				nSend	= 0;
	ZOSMutexLocker	locker(&m_SessionStreamMutex);

	if(m_pSessionStream != NULL)
	{
		if(sData != NULL && nData > 0)
		{
			if(m_pSessionStream->StreamWriteable())
			{
				nSend	= m_pSessionStream->StreamWrite(sData,nData);
			}else{
				//LOG_DEBUG(("[ZSessionStream::SendRequest] StreamWriteable ERROR %d\r\n", ZOSThread::GetError()));
			}
		}
		if(nSend == nData)
		{
			nError	= SESSION_STREAM_STATE_COMPLETE;
		}else{
			nError	= SESSION_STREAM_STATE_ERROR;
			//LOG_DEBUG(("ZSessionStream::SendStream ERROR(SESSION_STREAM_STATE_ERROR) nSend %d != nData %d\r\n", nSend, nData));
		}
	}else{
		nError	= SESSION_STREAM_STATE_INVALID;
		LOG_DEBUG(("ZSessionStream::SendRequest ERROR(SESSION_STREAM_STATE_INVALID)\r\n"));
	}
	return nError;
}

int ZSessionStream::SendStreamNoWritableCheck(const char* sData, int nData)
{
    int				nError	= 0;
    int				nSend	= 0;
    ZOSMutexLocker	locker(&m_SessionStreamMutex);

    if(m_pSessionStream != NULL)
    {
        if(sData != NULL && nData > 0)
        {
            nSend	= m_pSessionStream->StreamWrite(sData,nData);
        }
        if(nSend == nData)
        {
            nError	= SESSION_STREAM_STATE_COMPLETE;
        }else{
            nError	= SESSION_STREAM_STATE_ERROR;
            //LOG_DEBUG(("ZSessionStream::SendRequest ERROR(SESSION_STREAM_STATE_ERROR) nSend %d != nData %d\r\n", nSend, nData));
        }
    }else{
        nError	= SESSION_STREAM_STATE_INVALID;
        LOG_DEBUG(("ZSessionStream::SendRequest ERROR(SESSION_STREAM_STATE_INVALID)\r\n"));
    }
    return nError;
}

int ZSessionStream::CleanStream()
{
	int				nError	= SESSION_STREAM_STATE_INVALID;
	int				nRead	= 0;
	ZOSMutexLocker	locker(&m_SessionStreamMutex);

	memset(m_sBuffer,0,m_nSize);
	m_nBuffer	= 0;
	if(m_pSessionStream != NULL)
	{
		for(;;)
		{
			nRead	= m_pSessionStream->StreamRead(m_sBuffer,m_nSize);
			if(nRead > 0)
			{
				nError	= SESSION_STREAM_STATE_READ;
			}else
			{
				nError	= SESSION_STREAM_STATE_COMPLETE;
				break;
			}
		}
	}
	else
	{
		nError	= SESSION_STREAM_STATE_INVALID;
		LOG_ERROR(("ErrorCode:%s [ZSessionStream::CleanStream] ERROR(m_pSessionStream = NULL)!\r\n", GetErrorCodeString(24006)));
	}
	memset(m_sBuffer,0,m_nSize);
	m_nBuffer	= 0;
	return nError;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
