#include "ZRTCPPacket.h"
#include "ZOSMemory.h"
///////////////////////////////////////////////////////////////////////////////
#if	(defined(_WIN32_)||defined(_MINGW_))
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif	//!(defined(_WIN32_)||defined(_MINGW_))
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CONST DWORD ZRTCPPacket::m_nSupportedVersion	= 2;
CONST DWORD ZRTCPPacket::m_nMaxCount			= 15;
CONST DWORD ZRTCPPacket::m_nRTCPPacketSize		= 8;
CONST DWORD ZRTCPPacket::m_nRTCPPacketHeaderSize= 4;
///////////////////////////////////////////////////////////////////////////////
ZRTCPPacket::ZRTCPPacket(UINT nBuffer,CONST CHAR* sBuffer)
:ZObject()
,m_pFreeNext(NULL)
,m_pUsedNext(NULL)
,m_nPacketType((RTCP_PACKET_TYPE)0)
,m_pPacketBuffer(NULL)
,m_nPacketBuffer(0)
,m_nPacketLength(0)
{
	if(nBuffer > 0)
	{
		m_pPacketBuffer	= NEW CHAR[nBuffer];
		if(m_pPacketBuffer != NULL)
		{
			if(sBuffer != NULL)
			{
				memcpy(m_pPacketBuffer,sBuffer,nBuffer);
				m_nPacketLength		= nBuffer;
			}else{
				PRTCPHeader	pHeader	= (PRTCPHeader)m_pPacketBuffer;
				memset(m_pPacketBuffer,0,nBuffer);
				pHeader->m_nHeader	= htons(0x8000);
			}
			m_nPacketBuffer	= nBuffer;
			TMASSERT((m_nPacketBuffer>=m_nPacketLength));
		}
	}
}
ZRTCPPacket::ZRTCPPacket(CONST ZRTCPPacket& Packet)
:ZObject()
,m_nPacketType((RTCP_PACKET_TYPE)0)
,m_pPacketBuffer(NULL)
,m_nPacketBuffer(0)
,m_nPacketLength(0)
{
	if(Packet.GetBufferLength() > 0)
	{
		m_pPacketBuffer	= NEW CHAR[Packet.GetBufferLength()];
		if(m_pPacketBuffer != NULL)
		{
			if(Packet.GetBuffer() != NULL)
			{
				memcpy(m_pPacketBuffer,Packet.GetBuffer(),Packet.GetBufferLength());
				m_nPacketLength		= Packet.GetPacketLength();
			}else{
				PRTCPHeader	pHeader	= (PRTCPHeader)m_pPacketBuffer;
				memset(m_pPacketBuffer,0,Packet.GetBufferLength());
				pHeader->m_nHeader	= htons(0x8000);
			}
			m_nPacketBuffer	= Packet.GetBufferLength();
			TMASSERT((m_nPacketBuffer>=m_nPacketLength));
		}
	}
}
ZRTCPPacket::~ZRTCPPacket()
{
	m_nPacketType	= ((RTCP_PACKET_TYPE)0);
	SAFE_DELETE_ARRAY(m_pPacketBuffer);
	m_nPacketBuffer	= 0;
	m_nPacketLength	= 0;
}
///////////////////////////////////////////////////////////////////////////////
CHAR* ZRTCPPacket::GetBuffer() CONST
{
	return m_pPacketBuffer;
}
DWORD ZRTCPPacket::GetBufferLength() CONST
{
	return m_nPacketBuffer;
}
CHAR* ZRTCPPacket::GetPacket() CONST
{
	return m_pPacketBuffer;
}
DWORD ZRTCPPacket::GetPacketLength() CONST
{
	return m_nPacketLength;
}
VOID ZRTCPPacket::SetPacketLength(DWORD nLength)
{
    m_nPacketLength = nLength;
}
DWORD ZRTCPPacket::SetPacketData(CONST CHAR* sData,CONST DWORD nData)
{
	if(sData != NULL && nData > 0)
	{
		if(m_pPacketBuffer != NULL && nData <= m_nPacketBuffer)
		{
			memcpy(m_pPacketBuffer,sData,nData);
			m_nPacketLength	= nData;
		}
	}
	return m_nPacketLength;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTCPPacket::IsValid()
{
	if(m_pPacketBuffer != NULL && m_nPacketBuffer >= m_nRTCPPacketSize)
	{
		if(ZRTCPPacket::GetVersion() == ZRTCPPacket::m_nSupportedVersion)
		{
			return TRUE;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTCPPacket::GetVersion()
{
	if(m_pPacketBuffer != NULL && m_nPacketBuffer >= m_nRTCPPacketSize)
	{
		DWORD	nHeader	= GETUINT32(&m_pPacketBuffer[0]);
		DWORD	nValue	= ntohl(nHeader);
		return ((nValue&0xC0000000UL)>>30);
	}
	return 0;
}
DWORD ZRTCPPacket::GetPadding()
{
	if(m_pPacketBuffer != NULL && m_nPacketBuffer >= m_nRTCPPacketSize)
	{
		DWORD	nHeader	= GETUINT32(&m_pPacketBuffer[0]);
		DWORD	nValue	= ntohl(nHeader);
		return ((nValue&0x20000000UL));
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTCPPacket::GetCount()
{
	if(m_pPacketBuffer != NULL && m_nPacketBuffer >= m_nRTCPPacketSize)
	{
		DWORD	nHeader	= GETUINT32(&m_pPacketBuffer[0]);
		DWORD	nValue	= ntohl(nHeader);
		return ((nValue&0x0F000000UL)>>24);
	}
	return 0;
}
DWORD ZRTCPPacket::SetCount(DWORD nCount)
{
	if(m_pPacketBuffer != NULL && m_nPacketBuffer >= m_nRTCPPacketSize)
	{
		if(nCount <= m_nMaxCount)
		{
			DWORD	nHeader	= GETUINT32(&m_pPacketBuffer[0]);
			nHeader	= ntohl(nHeader);
			nHeader	&= (0xF0FFFFFF);
			nHeader	|= ((nCount&0x0F)<<24);
			nHeader	= ntohl(nHeader);
			SETUINT32(&m_pPacketBuffer[0],nHeader);
			return nCount;
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTCPPacket::GetPacketType()
{
	if(m_pPacketBuffer != NULL && m_nPacketBuffer >= m_nRTCPPacketSize)
	{
		DWORD	nHeader	= GETUINT32(&m_pPacketBuffer[0]);
		DWORD	nValue	= ntohl(nHeader);
		return ((nValue&0x00FF0000UL)>>16);
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTCPPacket::GetPacketSSRC()
{
	if(m_pPacketBuffer != NULL && m_nPacketBuffer >= m_nRTCPPacketSize)
	{
		DWORD	nLength	= GETUINT32(&m_pPacketBuffer[4]);
		nLength	= ntohl(nLength);
		return (nLength);
	}
	return 0;
}
DWORD ZRTCPPacket::SetPacketSSRC(DWORD nSSRC)
{
	if(m_pPacketBuffer != NULL && m_nPacketBuffer >= m_nRTCPPacketSize)
	{
		DWORD	nLength	= htonl(nSSRC);
		SETUINT32(&m_pPacketBuffer[4],nLength);
		return nLength;
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CONST DWORD ZRTCPSRPacket::m_nSRMessageSize	= 20;
CONST DWORD ZRTCPSRPacket::m_nSRHeaderSize	= 24;
///////////////////////////////////////////////////////////////////////////////
ZRTCPSRPacket::ZRTCPSRPacket(UINT nBuffer,CONST CHAR* sBuffer)
:ZRTCPPacket(nBuffer,sBuffer)
,m_nReportBlocks(0)
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength > 0)
		{
			m_nReportBlocks	= ((m_nPacketBuffer - m_nRTCPPacketSize - m_nSRMessageSize) / m_nSRHeaderSize);
		}else{
			PRTCPHeader	pHeader	= (PRTCPHeader)(m_pPacketBuffer);
			pHeader->m_nHeader	= htons(0x80C8);
			m_nPacketLength		= m_nRTCPPacketSize;
		}
	}
	m_nPacketType	= RTCP_PACKET_SR;
}
ZRTCPSRPacket::ZRTCPSRPacket(CONST ZRTCPSRPacket& Packet)
:ZRTCPPacket(Packet)
,m_nReportBlocks(0)
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength > 0)
		{
			m_nReportBlocks	= Packet.GetReportBlocks();
		}else{
			PRTCPHeader	pHeader	= (PRTCPHeader)(m_pPacketBuffer);
			pHeader->m_nHeader	= htons(0x80C8);
			m_nPacketLength		= m_nRTCPPacketSize;
		}
	}
	m_nPacketType	= RTCP_PACKET_SR;
}
ZRTCPSRPacket::~ZRTCPSRPacket()
{
	m_nReportBlocks	= 0;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTCPSRPacket::GetReportBlocks() CONST
{
	return m_nReportBlocks;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT64 ZRTCPSRPacket::GetNTPTimestamp()
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength > m_nSRHeaderSize)
		{
			return GETUINT64(&m_pPacketBuffer[8]);
		}
	}
	return 0;
}
UINT32 ZRTCPSRPacket::GetNTPTimestampMSW()
{
    if(m_pPacketBuffer != NULL)
    {
        if(m_nPacketLength > m_nSRHeaderSize)
        {
            return htonl(GETUINT32(&m_pPacketBuffer[8]));
        }
    }
    return 0;
}
UINT32  ZRTCPSRPacket::GetNTPTimestampLSW()
{
    if(m_pPacketBuffer != NULL)
    {
        if(m_nPacketLength > m_nSRHeaderSize)
        {
            return htonl(GETUINT32(&m_pPacketBuffer[8+4]));
        }
    }
    return 0;
}
UINT32 ZRTCPSRPacket::GetRTPTimestamp()
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength > m_nSRHeaderSize)
		{
			return htonl(GETUINT32(&m_pPacketBuffer[8+8]));
		}
	}
	return 0;
}
VOID ZRTCPSRPacket::SetNTPTimestamp(UINT64 nNTPTimestamp)
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength > (m_nRTCPPacketSize - m_nSRMessageSize))
		{
		}
	}
}

VOID ZRTCPSRPacket::SetNTPTimestampMSW(UINT32 nNTPTimestampMSW)
{
    if(m_pPacketBuffer != NULL)
    {
        //LOG_DEBUG(("in %d\r\n", m_nPacketLength));
        if(m_nPacketLength >= 12)
        {
            DWORD	nNTPMSW	= htonl(nNTPTimestampMSW);
            SETUINT32(&m_pPacketBuffer[8],nNTPMSW);
        }
    }
}

VOID ZRTCPSRPacket::SetNTPTimestampLSW(UINT32 nNTPTimestampLSW)
{
    if(m_pPacketBuffer != NULL)
    {
        if(m_nPacketLength >= 16)
        {
            DWORD	nNTPLSW	= htonl(nNTPTimestampLSW);
            SETUINT32(&m_pPacketBuffer[12], nNTPLSW);
        }
    }
}

VOID ZRTCPSRPacket::SetRTPTimestamp(UINT32 nRTPTimestamp)
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength >= 20)
		{
            DWORD	nTemp	= htonl(nRTPTimestamp);
            SETUINT32(&m_pPacketBuffer[16], nTemp);
		}
	}
}
VOID ZRTCPSRPacket::SetPacketCount(UINT32 nPacketCount)
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength >= 24)
		{
		}
	}
}
VOID ZRTCPSRPacket::SetByteCount(UINT32 nByteCount)
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength >= 28)
		{
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTCPSRPacket::AddReportBlock(UINT nSSRC,UINT nFraction,UINT nLost,UINT nLastSequence,UINT nLastSR,UINT nDelayLastSR)
{
	if(m_pPacketBuffer != NULL && m_nPacketBuffer >= (m_nPacketLength + m_nSRMessageSize + m_nSRHeaderSize))
	{
		if(m_nReportBlocks < m_nMaxCount)
		{
			PRTCPHeader		pHeader		= (PRTCPHeader)(m_pPacketBuffer);
			PRTCPSRHeader	pSRHeader	= (PRTCPSRHeader)(m_pPacketBuffer + m_nPacketLength);
			UINT			nLength		= 0;
			memset(pSRHeader,0,sizeof(RTCPSRHeader));
			pSRHeader->m_nSSRC			= htonl(nSSRC);
			pSRHeader->m_nFraction		= nFraction;
			pSRHeader->m_nLastSequence	= htonl(nLastSequence);
			pSRHeader->m_nLastSR		= htonl(nLastSR);
			pSRHeader->m_nDelayLastSR	= htonl(nDelayLastSR);
			if(nLost > 0x7FFFFF)
			{
				pSRHeader->m_nLost		= (htonl(0x7FFFFFL)&0xFFFFFF);
			}else if(nLost < 0xFF800000)
			{
				pSRHeader->m_nLost		= htonl(0x800000);
			}else{
				pSRHeader->m_nLost		= htonl(nLost);
			}
			m_nReportBlocks	++;
			nLength						= ((m_nSRMessageSize + m_nSRHeaderSize * m_nReportBlocks + 3) / 4);
			ZRTCPPacket::SetCount(m_nReportBlocks);
			pHeader->m_nLength			= htons((WORD)nLength);
			m_nPacketLength				= (m_nRTCPPacketSize + (nLength * 4));
			return TRUE;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CONST DWORD ZRTCPRRPacket::m_nSRMessageSize	= 4;
CONST DWORD ZRTCPRRPacket::m_nRRHeaderSize	= 24;
///////////////////////////////////////////////////////////////////////////////
ZRTCPRRPacket::ZRTCPRRPacket(UINT nBuffer,CONST CHAR* sBuffer)
:ZRTCPPacket(nBuffer,sBuffer)
,m_nReportBlocks(0)
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength > 0)
		{
			m_nReportBlocks	= ((m_nPacketBuffer - m_nRTCPPacketSize) / m_nRRHeaderSize);
		}else{
			PRTCPHeader	pHeader	= (PRTCPHeader)(m_pPacketBuffer);
			pHeader->m_nHeader	= htons(0x80C9);
			m_nPacketLength		= m_nRTCPPacketSize;
		}
	}
	m_nPacketType	= RTCP_PACKET_RR;
}
ZRTCPRRPacket::ZRTCPRRPacket(CONST ZRTCPRRPacket& Packet)
:ZRTCPPacket(Packet)
,m_nReportBlocks(0)
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength > 0)
		{
			m_nReportBlocks	= Packet.GetReportBlocks();
		}else{
			PRTCPHeader	pHeader	= (PRTCPHeader)(m_pPacketBuffer);
			pHeader->m_nHeader	= htons(0x80C9);
			m_nPacketLength		= m_nRTCPPacketSize;
		}
	}
	m_nPacketType	= RTCP_PACKET_RR;
}
ZRTCPRRPacket::~ZRTCPRRPacket()
{
	m_nReportBlocks	= 0;
}
///////////////////////////////////////////////////////////////////////////////
DWORD ZRTCPRRPacket::GetReportBlocks() CONST
{
	return m_nReportBlocks;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTCPRRPacket::AddReportBlock(UINT nSSRC,UINT nFraction,UINT nLost,UINT nLastSequence,UINT nLastSR,UINT nDelayLastSR)
{
	if(m_pPacketBuffer != NULL && m_nPacketBuffer >= (m_nPacketLength + m_nRRHeaderSize))
	{
		if(m_nReportBlocks < m_nMaxCount)
		{
			PRTCPHeader		pHeader		= (PRTCPHeader)(m_pPacketBuffer);
			PRTCPRRHeader	pRRHeader	= (PRTCPRRHeader)(m_pPacketBuffer + m_nPacketLength);
			UINT			nLength		= 0;
			memset(pRRHeader,0,sizeof(RTCPRRHeader));
			pRRHeader->m_nSSRC			= htonl(nSSRC);
			pRRHeader->m_nFraction		= nFraction;
			pRRHeader->m_nLastSequence	= htonl(nLastSequence);
			pRRHeader->m_nLastSR		= htonl(nLastSR);
			pRRHeader->m_nDelayLastSR	= htonl(nDelayLastSR);
			if(nLost > 0x7FFFFF)
			{
				pRRHeader->m_nLost		= (htonl(0x7FFFFFL)&0xFFFFFF);
			}else if(nLost < 0xFF800000)
			{
				pRRHeader->m_nLost		= htonl(0x800000);
			}else{
				pRRHeader->m_nLost		= htonl(nLost);
			}
			m_nReportBlocks	++;
			nLength						= ((m_nSRMessageSize + m_nRRHeaderSize * m_nReportBlocks + 3) / 4);
			ZRTCPPacket::SetCount(m_nReportBlocks);
			pHeader->m_nLength			= htons((WORD)nLength);
			m_nPacketLength				= (m_nRTCPPacketHeaderSize + (nLength * 4));
			return TRUE;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZRTCPSDESPacket::ZRTCPSDESPacket(UINT nBuffer,CONST CHAR* sBuffer)
:ZRTCPPacket(nBuffer,sBuffer)
,m_nReportBlocks(0)
,m_nTotalItem(0)
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength > 0)
		{
			m_nReportBlocks	= ((m_nPacketBuffer - m_nRTCPPacketHeaderSize) / 4);
		}else{
			PRTCPHeader	pHeader	= (PRTCPHeader)(m_pPacketBuffer);
			pHeader->m_nHeader	= htons(0x80CA);
			m_nPacketLength		= m_nRTCPPacketHeaderSize;
		}
	}
	m_nPacketType	= RTCP_PACKET_BYE;
}
ZRTCPSDESPacket::ZRTCPSDESPacket(CONST ZRTCPRRPacket& Packet)
:ZRTCPPacket(Packet)
,m_nReportBlocks(0)
,m_nTotalItem(0)
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength > 0)
		{
			m_nReportBlocks	= Packet.GetReportBlocks();
		}else{
			PRTCPHeader	pHeader	= (PRTCPHeader)(m_pPacketBuffer);
			pHeader->m_nHeader	= htons(0x80CA);
			m_nPacketLength		= m_nRTCPPacketHeaderSize;
		}
	}
	m_nPacketType	= RTCP_PACKET_RR;
}
ZRTCPSDESPacket::~ZRTCPSDESPacket()
{
	m_nReportBlocks	= 0;
	m_nTotalItem	= 0;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTCPSDESPacket::AddReportBlock(UINT nSRC,CONST CHAR* sName)
{
	if(sName != NULL)
	{
		UINT	nName	= strlen(sName);
		if(m_pPacketBuffer != NULL && m_nPacketBuffer >= (m_nPacketLength + 7 + nName))
		{
			if(m_nReportBlocks < m_nMaxCount)
			{
				PRTCPHeader		pHeader		= (PRTCPHeader)(m_pPacketBuffer);
				UINT			nLength		= 0;
				UINT*			pSRC		= (UINT*)(m_pPacketBuffer + m_nPacketLength);
				CHAR*			pItem		= (CHAR*)(m_pPacketBuffer + m_nPacketLength + 4);
				*pSRC						= htonl(nSRC);
				pItem[0]					= 0x01;
				pItem[1]					= nName;
				strcpy(pItem+2,sName);
				pItem[nName+2]				= 0x00;
				pItem[nName+3]				= 0x00;
				pItem[nName+4]				= 0x00;
				m_nReportBlocks				++;
				m_nTotalItem				+= nName;
				nLength						= ((7 * m_nReportBlocks + m_nTotalItem + 3) / 4);
				ZRTCPPacket::SetCount(m_nReportBlocks);
				pHeader->m_nLength			= htons((WORD)nLength);
				m_nPacketLength				= (m_nRTCPPacketHeaderSize + (nLength * 4));
				return TRUE;
			}
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZRTCPBYEPacket::ZRTCPBYEPacket(UINT nBuffer,CONST CHAR* sBuffer)
:ZRTCPPacket(nBuffer,sBuffer)
,m_nReportBlocks(0)
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength > 0)
		{
			m_nReportBlocks	= ((m_nPacketBuffer - m_nRTCPPacketHeaderSize) / 4);
		}else{
			PRTCPHeader	pHeader	= (PRTCPHeader)(m_pPacketBuffer);
			pHeader->m_nHeader	= htons(0x80CB);
			m_nPacketLength		= m_nRTCPPacketHeaderSize;
		}
	}
	m_nPacketType	= RTCP_PACKET_BYE;
}
ZRTCPBYEPacket::ZRTCPBYEPacket(CONST ZRTCPRRPacket& Packet)
:ZRTCPPacket(Packet)
,m_nReportBlocks(0)
{
	if(m_pPacketBuffer != NULL)
	{
		if(m_nPacketLength > 0)
		{
			m_nReportBlocks	= Packet.GetReportBlocks();
		}else{
			PRTCPHeader	pHeader	= (PRTCPHeader)(m_pPacketBuffer);
			pHeader->m_nHeader	= htons(0x80CB);
			m_nPacketLength		= m_nRTCPPacketHeaderSize;
		}
	}
	m_nPacketType	= RTCP_PACKET_RR;
}
ZRTCPBYEPacket::~ZRTCPBYEPacket()
{
	m_nReportBlocks	= 0;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTCPBYEPacket::AddReportBlock(UINT nSRC)
{
	if(m_pPacketBuffer != NULL && m_nPacketBuffer >= (m_nPacketLength + 4))
	{
		if(m_nReportBlocks < m_nMaxCount)
		{
			PRTCPHeader		pHeader		= (PRTCPHeader)(m_pPacketBuffer);
			UINT*			pSRC		= (UINT*)(m_pPacketBuffer + m_nPacketLength);
			*pSRC						= htonl(nSRC);
			m_nReportBlocks				++;
			ZRTCPPacket::SetCount(m_nReportBlocks);
			pHeader->m_nLength			= htons((WORD)(m_nReportBlocks));
			m_nPacketLength				= (m_nRTCPPacketHeaderSize + 4 * m_nReportBlocks);
			return TRUE;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZRTCPBYEPacket::AddReason(CHAR* sReason)
{
	if(sReason != NULL)
	{
		UINT	nReason	= strlen(sReason);
		if(m_pPacketBuffer != NULL && m_nPacketBuffer > (m_nPacketLength + 1 + nReason))
		{
			PRTCPHeader		pHeader		= (PRTCPHeader)(m_pPacketBuffer);
			UINT			nLength		= 0;
			BYTE*			pLength		= (BYTE*)(m_pPacketBuffer + m_nPacketLength);
			CHAR*			pReason		= (CHAR*)(m_pPacketBuffer + m_nPacketLength + 1);
			*pLength					= (nReason&0xFF);
			strncpy(pReason,sReason,(nReason&0xFF));
			pReason[(nReason&0xFF)]		= 0;
			nLength						= (m_nReportBlocks + ((1 + nReason + 3) / 4));
			pHeader->m_nLength			= htons((WORD)nLength);
			m_nPacketLength				= (m_nRTCPPacketHeaderSize + (nLength * 4));
			return TRUE;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
