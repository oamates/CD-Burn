#include "ZDataPin.h"
///////////////////////////////////////////////////////////////////////////////
//#define	DATAPIN_DEBUG		1
///////////////////////////////////////////////////////////////////////////////
ZDataPin::~ZDataPin()
{

}
///////////////////////////////////////////////////////////////////////////////
ZDataIn::ZDataIn()
:m_DataInMutex("DataInMutex")
,m_pDataInPin(NULL)
{
}
ZDataIn::~ZDataIn()
{
	ZDataIn::SetDataPin();
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZDataIn::AttachePin(ZDataPin* pPin)
{
	return FALSE;
}
BOOL ZDataIn::DetachePin(ZDataPin* pPin)
{
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZDataIn::SetDataPin(ZDataOut* pDataInPin)
{
	BOOL	bReturn	= TRUE;

	if(m_pDataInPin != pDataInPin)
	{
		bReturn	= FALSE;
		if(m_pDataInPin != NULL)
		{
			m_pDataInPin->DetachePin(this);
		}
		if(pDataInPin != NULL)
		{
			if(pDataInPin->AttachePin(this))
			{
				m_pDataInPin	= pDataInPin;
				bReturn			= TRUE;
			}
		}
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZDataIn::OnCommand(int nCommand)
{
	if(nCommand == DEFAULT_COMMAND_CLOSE)
	{
		m_pDataInPin	= NULL;
	}
	return TRUE;
}
BOOL ZDataIn::OnHeader(int nFlag,void* pHeader,int nHeader)
{
	return FALSE;
}
BOOL ZDataIn::OnData(int nFlag,void* pData,int nData)
{
	return FALSE;
}
BOOL ZDataIn::OnUserData(int nFlag,void *pData)
{
    return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZDataOut::ZDataOut()
:m_DataOutMutex("DataOutMutex")
,m_DataOutArray(64)
{
}
ZDataOut::~ZDataOut()
{
	ZDataOut::SendCommand(DEFAULT_COMMAND_CLOSE);
	TMASSERT((m_DataOutArray.Count()==0));
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZDataOut::AttachePin(ZDataPin* pPin)
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_DataOutMutex);

	if(pPin != NULL)
	{
		int	i	= 0;
		for(i = 0; i < m_DataOutArray.Count(); i ++)
		{
			if(m_DataOutArray[i] == pPin)
			{
				break;
			}
		}
		if(i >= m_DataOutArray.Count())
		{
			m_DataOutArray.Add(pPin);
		}
		bReturn	= TRUE;
	}
	return bReturn;
}
BOOL ZDataOut::DetachePin(ZDataPin* pPin)
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_DataOutMutex);

	if(pPin != NULL)
	{
		int	i	= 0;
		for(i = 0; i < m_DataOutArray.Count(); i ++)
		{
			if(m_DataOutArray[i] == pPin)
			{
				m_DataOutArray.Remove(i);
				bReturn	= TRUE;
				break;
			}
		}
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZDataOut::SendCommand(int nCommand)
{
	int				i	= 0;
	ZOSMutexLocker	locker(&m_DataOutMutex);
	for(i = 0; i < m_DataOutArray.Count(); i ++)
	{
		if(m_DataOutArray[i] != NULL)
		{
			m_DataOutArray[i]->OnCommand(nCommand);
		}
	}
	if(nCommand == DEFAULT_COMMAND_CLOSE)
	{
		m_DataOutArray.RemoveAll();
	}
	return TRUE;
}
BOOL ZDataOut::SendHeader(int nFlag,void* pHeader,int nHeader)
{
	int				i	= 0;
	ZOSMutexLocker	locker(&m_DataOutMutex);
	for(i = 0; i < m_DataOutArray.Count(); i ++)
	{
		if(m_DataOutArray[i] != NULL)
		{
			m_DataOutArray[i]->OnHeader(nFlag,pHeader,nHeader);
		}
	}
	return TRUE;
}
BOOL ZDataOut::SendData(int nFlag,void* pData,int nData)
{
	int				i	= 0;
	ZOSMutexLocker	locker(&m_DataOutMutex);
	for(i = 0; i < m_DataOutArray.Count(); i ++)
	{
		if(m_DataOutArray[i] != NULL)
		{
			m_DataOutArray[i]->OnData(nFlag,pData,nData);
		}
	}
	return TRUE;
}
BOOL ZDataOut::SendUserData(int nFlag, void *pData)
{
    int				i	= 0;
    ZOSMutexLocker	locker(&m_DataOutMutex);
    for(i = 0; i < m_DataOutArray.Count(); i ++)
    {
        if(m_DataOutArray[i] != NULL)
        {
            m_DataOutArray[i]->OnUserData(nFlag,pData);
        }
    }
    return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZDataOut::OnCommand(int nCommand)
{
	return TRUE;
}
BOOL ZDataOut::OnHeader(int nFlag,void* pHeader,int nHeader)
{
	return FALSE;
}
BOOL ZDataOut::OnData(int nFlag,void* pData,int nData)
{
	return FALSE;
}
BOOL ZDataOut::OnUserData(int nFlag,void *pData)
{
    return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZDataInOut::ZDataInOut(ZDataOut* pDataPin)
:m_DataInOutMutex("DataInOutMutex")
,m_pDataInPin(NULL)
,m_DataOutArray(64)
{
	ZDataInOut::SetDataPin(pDataPin);
#ifdef	DATAPIN_DEBUG
	MESSAGE_OUT(("ZDataInOut New Pin(0x%08X)\r\n",this));
#endif	//DATAPIN_DEBUG
}
ZDataInOut::~ZDataInOut()
{
	ZDataInOut::SendCommand(DEFAULT_COMMAND_CLOSE);
	TMASSERT((m_DataOutArray.Count()==0));
	ZDataInOut::SetDataPin();
#ifdef	DATAPIN_DEBUG
	MESSAGE_OUT(("ZDataInOut Release Pin(0x%08X)\r\n",this));
#endif	//DATAPIN_DEBUG
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZDataInOut::AttachePin(ZDataPin* pPin)
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_DataInOutMutex);

	if(pPin != NULL)
	{
		int	i	= 0;
		for(i = 0; i < m_DataOutArray.Count(); i ++)
		{
			if(m_DataOutArray[i] == pPin)
			{
				break;
			}
		}
		if(i >= m_DataOutArray.Count())
		{
			m_DataOutArray.Add(pPin);
		}
		bReturn	= TRUE;
	}
#ifdef	DATAPIN_DEBUG
	MESSAGE_OUT(("ZDataInOut::AttachePin(0x%08X)\r\n",pPin));
#endif	//DATAPIN_DEBUG

	return bReturn;
}
BOOL ZDataInOut::DetachePin(ZDataPin* pPin)
{
	BOOL			bReturn	= FALSE;
	ZOSMutexLocker	locker(&m_DataInOutMutex);

	if(pPin != NULL)
	{
		int	i	= 0;
		for(i = 0; i < m_DataOutArray.Count(); i ++)
		{
			if(m_DataOutArray[i] == pPin)
			{
				m_DataOutArray.Remove(i);
				bReturn	= TRUE;
				break;
			}
		}
	}
#ifdef	DATAPIN_DEBUG
	MESSAGE_OUT(("ZDataInOut::DetachePin(0x%08X)\r\n",pPin));
#endif	//DATAPIN_DEBUG
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZDataInOut::SendCommand(int nCommand)
{
	int				i	= 0;
	ZOSMutexLocker	locker(&m_DataInOutMutex);
	for(i = 0; i < m_DataOutArray.Count(); i ++)
	{
		if(m_DataOutArray[i] != NULL)
		{
			m_DataOutArray[i]->OnCommand(nCommand);
		}
	}
	if(nCommand == DEFAULT_COMMAND_CLOSE)
	{
		m_DataOutArray.RemoveAll();
	}
#ifdef	DATAPIN_DEBUG
	MESSAGE_OUT(("ZDataInOut::SendCommand(%d)\r\n",nCommand));
#endif	//DATAPIN_DEBUG
	return TRUE;
}
BOOL ZDataInOut::SendHeader(int nFlag,void* pHeader,int nHeader)
{
	int				i	= 0;
	ZOSMutexLocker	locker(&m_DataInOutMutex);
	for(i = 0; i < m_DataOutArray.Count(); i ++)
	{
		if(m_DataOutArray[i] != NULL)
		{
			m_DataOutArray[i]->OnHeader(nFlag,pHeader,nHeader);
		}
	}
	return TRUE;
}
BOOL ZDataInOut::SendData(int nFlag,void* pData,int nData)
{
	int				i	= 0;
	ZOSMutexLocker	locker(&m_DataInOutMutex);
	for(i = 0; i < m_DataOutArray.Count(); i ++)
	{
		if(m_DataOutArray[i] != NULL)
		{
			m_DataOutArray[i]->OnData(nFlag,pData,nData);
		}
	}
	return TRUE;
}
BOOL ZDataInOut::SendUserData(int nFlag, void *pData)
{
    int				i	= 0;
    ZOSMutexLocker	locker(&m_DataInOutMutex);
    for(i = 0; i < m_DataOutArray.Count(); i ++)
    {
        if(m_DataOutArray[i] != NULL)
        {
            m_DataOutArray[i]->OnUserData(nFlag,pData);
        }
    }
    return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZDataInOut::SetDataPin(ZDataOut* pDataInPin)
{
	BOOL	bReturn	= TRUE;

	if(m_pDataInPin != pDataInPin)
	{
		bReturn	= FALSE;
		if(m_pDataInPin != NULL)
		{
			m_pDataInPin->DetachePin(this);
		}
		if(pDataInPin != NULL)
		{
			if(pDataInPin->AttachePin(this))
			{
				m_pDataInPin	= pDataInPin;
				bReturn			= TRUE;
			}
		}
	}
#ifdef	DATAPIN_DEBUG
	MESSAGE_OUT(("ZDataInOut::SetDataPin(0x%08X)\r\n",pDataInPin));
#endif	//DATAPIN_DEBUG
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZDataInOut::OnCommand(int nCommand)
{
	if(nCommand == DEFAULT_COMMAND_CLOSE)
	{
		m_pDataInPin	= NULL;
	}
#ifdef	DATAPIN_DEBUG
	MESSAGE_OUT(("ZDataInOut::OnCommand(%d)\r\n",nCommand));
#endif	//DATAPIN_DEBUG
	return ZDataInOut::SendCommand(nCommand);
}
BOOL ZDataInOut::OnHeader(int nFlag,void* pHeader,int nHeader)
{
	return ZDataInOut::SendHeader(nFlag,pHeader,nHeader);
}
BOOL ZDataInOut::OnData(int nFlag,void* pData,int nData)
{
	return ZDataInOut::SendData(nFlag,pData,nData);
}
BOOL ZDataInOut::OnUserData(int nFlag,void *pData)
{
    return ZDataInOut::SendUserData(nFlag, pData);
}
///////////////////////////////////////////////////////////////////////////////
int	ZDataInOut::GetDataOutArraySize()
{
	ZOSMutexLocker	locker(&m_DataInOutMutex);

	return m_DataOutArray.Size();
}
///////////////////////////////////////////////////////////////////////////////
