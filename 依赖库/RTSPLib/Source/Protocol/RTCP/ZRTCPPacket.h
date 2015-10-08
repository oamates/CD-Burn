///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZRTCPPacket	Header File
	Create		20110105		ZHAOTT		RTCP
	Modify		20110719		ZHAOTT		RTCP
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZRTCPPACKET_H_
#define	_ZRTCPPACKET_H_
///////////////////////////////////////////////////////////////////////////////
#include "Common.h"
///////////////////////////////////////////////////////////////////////////////
class	ZRTCPPacket;
class	ZRTCPSRPacket;
class	ZRTCPRRPacket;
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_RTCP_BUFFER_USED	1460
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_RTCP_BUFFER_MAX		14600
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//RTCP Packet
///////////////////////////////////////////////////////////////////////////////
//|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|V=2|P|   RC    |     PT        |             length            |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///////////////////////////////////////////////////////////////////////////////
class ZRTCPPacket : public ZObject
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZRTCPPacket(UINT nBuffer = DEFAULT_RTCP_BUFFER_USED,CONST CHAR* sBuffer = NULL);
	ZRTCPPacket(CONST ZRTCPPacket& Packet);
	virtual ~ZRTCPPacket();
///////////////////////////////////////////////////////////////////////////////
public:
	CHAR*			GetBuffer() CONST;
	DWORD			GetBufferLength() CONST;
	CHAR*			GetPacket() CONST;
	DWORD			GetPacketLength() CONST;
    VOID            SetPacketLength(DWORD nLength);
	DWORD			SetPacketData(CONST CHAR* sData,CONST DWORD nData);
	///////////////////////////////////////////////////////////////////////////////
	public:
	BOOL			IsValid();
	DWORD			GetVersion();
	DWORD			GetPadding();
	DWORD			GetCount();
	DWORD			SetCount(DWORD nCount);
	DWORD			GetPacketType();
	DWORD			GetPacketSSRC();
	DWORD			SetPacketSSRC(DWORD nSSRC);
///////////////////////////////////////////////////////////////////////////////
public:
	ZRTCPPacket*		m_pFreeNext;
	ZRTCPPacket*		m_pUsedNext;
///////////////////////////////////////////////////////////////////////////////
public:
typedef	struct	_RTCPHeader_
{
	UINT16		m_nHeader;
	UINT16		m_nLength;
	UINT32		m_nSSRC;
}RTCPHeader,*PRTCPHeader;
typedef	enum	_RTCP_PACKET_TYPE_
{
	RTCP_PACKET_SR		= 200,
	RTCP_PACKET_RR		= 201,
	RTCP_PACKET_SDES	= 202,
	RTCP_PACKET_BYE		= 203,
	RTCP_PACKET_APP		= 204,
}RTCP_PACKET_TYPE;
///////////////////////////////////////////////////////////////////////////////
protected:
	RTCP_PACKET_TYPE	m_nPacketType;
	char*				m_pPacketBuffer;
	DWORD				m_nPacketBuffer;
	DWORD				m_nPacketLength;
///////////////////////////////////////////////////////////////////////////////
public:
	STATIC CONST DWORD		m_nSupportedVersion;
	STATIC CONST DWORD		m_nMaxCount;
	STATIC CONST DWORD		m_nRTCPPacketSize;
	STATIC CONST DWORD		m_nRTCPPacketHeaderSize;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//RTCP Packet
///////////////////////////////////////////////////////////////////////////////
//|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|V=2|P|   RC    |   PT=SR=200   |             length            |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                     SSRC of sender                            |
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//|              NTP timestamp, most significant word             |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|             NTP timestamp, least significant word             |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                         RTP timestamp                         |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                     sender's packet count                     |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                      sender's octet count                     |
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//|                 SSRC_1 (SSRC of first source)                 |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//| fraction lost |       cumulative number of packets lost       |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|           extended highest sequence number received           |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                      interarrival jitter                      |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                         last SR (LSR)                         |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                   delay since last SR (DLSR)                  |
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
///////////////////////////////////////////////////////////////////////////////
class ZRTCPSRPacket : public ZRTCPPacket
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZRTCPSRPacket(UINT nBuffer = DEFAULT_RTCP_BUFFER_USED,CONST CHAR* sBuffer = NULL);
	ZRTCPSRPacket(CONST ZRTCPSRPacket& Packet);
	virtual ~ZRTCPSRPacket();
///////////////////////////////////////////////////////////////////////////////
public:
	DWORD			GetReportBlocks() CONST;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	UINT64	GetNTPTimestamp();
    virtual UINT32  GetNTPTimestampMSW();
    virtual UINT32  GetNTPTimestampLSW();
	virtual	UINT32	GetRTPTimestamp();
	virtual	VOID	SetNTPTimestamp(UINT64 nNTPTimestamp);
    virtual VOID    SetNTPTimestampMSW(UINT32 nNTPTimestampMSW);
    virtual VOID    SetNTPTimestampLSW(UINT32 nNTPTimestampLSW);
	virtual	VOID	SetRTPTimestamp(UINT32 nRTPTimestamp);
	virtual	VOID	SetPacketCount(UINT32 nPacketCount);
	virtual	VOID	SetByteCount(UINT32 nByteCount);
///////////////////////////////////////////////////////////////////////////////
public:
typedef	struct	_RTCPSRHeader_
{
	UINT32		m_nSSRC;
	UINT32		m_nFraction:8;
	UINT32		m_nLost:24;
	UINT32		m_nLastSequence;
	UINT32		m_nJitter;
	UINT32		m_nLastSR;
	UINT32		m_nDelayLastSR;
}RTCPSRHeader,*PRTCPSRHeader;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	AddReportBlock(UINT nSSRC,UINT nFraction,UINT nLost,UINT nLastSequence,UINT nLastSR,UINT nDelayLastSR);
///////////////////////////////////////////////////////////////////////////////
protected:
	DWORD			m_nReportBlocks;
///////////////////////////////////////////////////////////////////////////////
public:
	STATIC CONST DWORD		m_nSRHeaderSize;
	STATIC CONST DWORD		m_nSRMessageSize;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//RTCP Packet
///////////////////////////////////////////////////////////////////////////////
//|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|V=2|P|   RC    |   PT=RR=201   |             length            |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                     SSRC of packet sender                     |
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//|                 SSRC_1 (SSRC of first source)                 |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//| fraction lost |       cumulative number of packets lost       |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|           extended highest sequence number received           |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                      interarrival jitter                      |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                         last SR (LSR)                         |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                   delay since last SR (DLSR)                  |
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
///////////////////////////////////////////////////////////////////////////////
class ZRTCPRRPacket : public ZRTCPPacket
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZRTCPRRPacket(UINT nBuffer = DEFAULT_RTCP_BUFFER_USED,CONST CHAR* sBuffer = NULL);
	ZRTCPRRPacket(CONST ZRTCPRRPacket& Packet);
	virtual ~ZRTCPRRPacket();
///////////////////////////////////////////////////////////////////////////////
public:
	DWORD			GetReportBlocks() CONST;
///////////////////////////////////////////////////////////////////////////////
public:
typedef	struct	_RTCPRRHeader_
{
	UINT32		m_nSSRC;
	UINT32		m_nFraction:8;
	UINT32		m_nLost:24;
	UINT32		m_nLastSequence;
	UINT32		m_nJitter;
	UINT32		m_nLastSR;
	UINT32		m_nDelayLastSR;
}RTCPRRHeader,*PRTCPRRHeader;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	AddReportBlock(UINT nSSRC,UINT nFraction,UINT nLost,UINT nLastSequence,UINT nLastSR,UINT nDelayLastSR);
///////////////////////////////////////////////////////////////////////////////
protected:
	DWORD			m_nReportBlocks;
///////////////////////////////////////////////////////////////////////////////
public:
	STATIC CONST DWORD		m_nSRMessageSize;
	STATIC CONST DWORD		m_nRRHeaderSize;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//RTCP Packet
///////////////////////////////////////////////////////////////////////////////
//|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|V=2|P|   SC    |  PT=SDES=202  |             length            |
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//|                          SSRC/CSRC_1                          |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                           SDES items                          |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                              ...                              |
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ZRTCPSDESPacket : public ZRTCPPacket
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZRTCPSDESPacket(UINT nBuffer = DEFAULT_RTCP_BUFFER_USED,CONST CHAR* sBuffer = NULL);
	ZRTCPSDESPacket(CONST ZRTCPRRPacket& Packet);
	virtual ~ZRTCPSDESPacket();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	AddReportBlock(UINT nSRC,CONST CHAR* sName);
///////////////////////////////////////////////////////////////////////////////
protected:
	DWORD			m_nReportBlocks;
	DWORD			m_nTotalItem;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
//RTCP Packet
///////////////////////////////////////////////////////////////////////////////
//|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|V=2|P|   SC    |  PT=BYE=203   |             length            |
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//|                          SSRC/CSRC                            |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                              ...                              |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|     length    |               reason for leaving              |
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
///////////////////////////////////////////////////////////////////////////////
class ZRTCPBYEPacket : public ZRTCPPacket
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZRTCPBYEPacket(UINT nBuffer = DEFAULT_RTCP_BUFFER_USED,CONST CHAR* sBuffer = NULL);
	ZRTCPBYEPacket(CONST ZRTCPRRPacket& Packet);
	virtual ~ZRTCPBYEPacket();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	AddReportBlock(UINT nSRC);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	AddReason(CHAR* sReason);
///////////////////////////////////////////////////////////////////////////////
protected:
	DWORD			m_nReportBlocks;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//RTCP Packet
///////////////////////////////////////////////////////////////////////////////
//|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|V=2|P| subtype |   PT=APP=204  |             length            |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                           SSRC/CSRC                           |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                          name (ASCII)                         |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                   application-dependent data                  |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZRTCPPACKET_H_
///////////////////////////////////////////////////////////////////////////////
