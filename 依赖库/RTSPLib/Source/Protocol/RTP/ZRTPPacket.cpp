#include "ZRTPPacket.h"
#include "ZOS.h"
///////////////////////////////////////////////////////////////////////////////
#if	(defined(_WIN32_)||defined(_MINGW_))
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif	//!(defined(_WIN32_)||defined(_MINGW_))
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CONST DWORD ZRTPPacket::m_nSupportedVersion		= 2;
CONST DWORD ZRTPPacket::m_nMaxCount				= 15;
CONST INT	ZRTPPacket::m_nPacketHeaderSize		= 12;
DWORD ZRTPPacket::m_nGlobleSSRC                 = 1;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZRTPHeader::ZRTPHeader()
:m_pHeader(NULL)
{
}
ZRTPHeader::~ZRTPHeader()
{
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPHeader::Set(const char* sBuffer,int nBuffer)
{
	if(sBuffer != NULL && nBuffer >= ZRTPPacket::m_nPacketHeaderSize)
	{
		m_pHeader	= (RTPHEADER*)sBuffer;
	}
	return (m_pHeader != NULL);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZRTPPacket::ZRTPPacket()
:ZObject()
,m_pFreeNext(NULL)
,m_pUsedNext(NULL)
,m_pPacketBuffer(NULL)
,m_nPacketBuffer(0)
,m_nPacketLength(0)
,m_ePacketHeader()
{
	m_pPacketBuffer	= NEW char[DEFAULT_RTP_BUFFER_MAX];
	m_nPacketBuffer	= DEFAULT_RTP_BUFFER_MAX;
	m_nPacketLength	= 0;
	m_ePacketHeader.Set(m_pPacketBuffer,m_nPacketBuffer);
}
ZRTPPacket::~ZRTPPacket()
{
	SAFE_DELETE_ARRAY(m_pPacketBuffer);
	m_nPacketBuffer	= 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
char* ZRTPPacket::GetPacketPointer() CONST
{
	return (char*)m_pPacketBuffer;
}
int ZRTPPacket::GetPacketLength() CONST
{
	return m_nPacketLength;
}
int ZRTPPacket::GetPacketPayloadLength() CONST
{
    unsigned char *pPacketBuffer = (unsigned char *)m_pPacketBuffer;
    if (
        (pPacketBuffer != NULL)
        && (pPacketBuffer[0]&0x20)
        )
    {// rtp packet with padding
        return m_nPacketLength - pPacketBuffer[m_nPacketLength-1];
    }

    return m_nPacketLength;
}
int ZRTPPacket::SetPacketData(const char* sData,const int nData)
{
	if(sData != NULL && nData > 0 && nData <= DEFAULT_RTP_BUFFER_MAX)
	{
		if(m_pPacketBuffer != NULL)
		{
			memcpy(m_pPacketBuffer,sData,nData);
			m_nPacketLength	= nData;
			m_nTimeReceived = ZOS::milliseconds();
		}
	}
	return m_nPacketLength;
}
UINT64 ZRTPPacket::GetRecievedTime()
{
	return m_nTimeReceived;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTPPacket::IsValid()
{
	if(m_pPacketBuffer != NULL && m_nPacketLength >= m_nPacketHeaderSize)
	{
		if(ZRTPPacket::GetVersion() == ZRTPPacket::m_nSupportedVersion)
		{
			return TRUE;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTPPacket::GetVersion()
{
	if(m_pPacketBuffer != NULL && m_nPacketLength >= m_nPacketHeaderSize)
	{
		DWORD	nHeader	= GETUINT32(&m_pPacketBuffer[0]);
		DWORD	nValue	= ntohl(nHeader);
		nValue	= ((nValue&0xC0000000UL)>>30);
		TMASSERT((nValue==m_nSupportedVersion));
		return nValue;
	}
	return 0;
}
DWORD ZRTPPacket::GetPadding()
{
	if(m_pPacketBuffer != NULL && m_nPacketLength >= m_nPacketHeaderSize)
	{
		DWORD	nHeader	= GETUINT32(&m_pPacketBuffer[0]);
		DWORD	nValue	= ntohl(nHeader);
		return ((nValue&0x20000000UL));
	}
	return 0;
}
DWORD ZRTPPacket::GetExtension()
{
	if(m_pPacketBuffer != NULL && m_nPacketLength >= m_nPacketHeaderSize)
	{
		DWORD	nHeader	= GETUINT32(&m_pPacketBuffer[0]);
		DWORD	nValue	= ntohl(nHeader);
		return ((nValue&0x10000000UL));
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTPPacket::GetCount()
{
	if(m_pPacketBuffer != NULL && m_nPacketLength >= m_nPacketHeaderSize)
	{
		DWORD	nHeader	= GETUINT32(&m_pPacketBuffer[0]);
		DWORD	nValue	= ntohl(nHeader);
		return ((nValue&0x0F000000UL)>>24);
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTPPacket::GetMarker()
{
	if(m_pPacketBuffer != NULL && m_nPacketLength >= m_nPacketHeaderSize)
	{
		DWORD	nHeader	= GETUINT32(&m_pPacketBuffer[0]);
		DWORD	nValue	= ntohl(nHeader);
		return ((nValue&0x00800000UL));
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTPPacket::GetPayload()
{
	if(m_pPacketBuffer != NULL && m_nPacketLength >= m_nPacketHeaderSize)
	{
		DWORD	nHeader	= GETUINT32(&m_pPacketBuffer[0]);
		DWORD	nValue	= ntohl(nHeader);
		return ((nValue&0x007F0000UL)>>16);
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
WORD ZRTPPacket::GetSequence()
{
	if(m_pPacketBuffer != NULL && m_nPacketLength >= m_nPacketHeaderSize)
	{
		DWORD	nHeader	= GETUINT32(&m_pPacketBuffer[0]);
		DWORD	nValue	= ntohl(nHeader);
		return (WORD)((nValue&0x0000FFFFUL));
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTPPacket::GetTimeStamp()
{
	if(m_pPacketBuffer != NULL && m_nPacketLength >= m_nPacketHeaderSize)
	{
		DWORD	nTimeStamp	= GETUINT32(&m_pPacketBuffer[4]);
		DWORD	nValue		= ntohl(nTimeStamp);
		return ((nValue));
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTPPacket::GetPacketSSRC()
{
	if(m_pPacketBuffer != NULL && m_nPacketLength >= m_nPacketHeaderSize)
	{
		DWORD	nSSRC		= GETUINT32(&m_pPacketBuffer[8]);
		DWORD	nValue		= ntohl(nSSRC);
		return ((nValue));
	}
	return 0;
}
DWORD ZRTPPacket::GenerateSSRC()
{
    if (m_nGlobleSSRC == 0)
    {
        m_nGlobleSSRC ++;
    }
    return m_nGlobleSSRC++;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
