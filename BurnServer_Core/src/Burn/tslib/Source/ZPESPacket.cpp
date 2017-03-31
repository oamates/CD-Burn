#include "ZOSMemory.h"
#include "ZPESPacket.h"
#include "ZMPEGFormat.h"
///////////////////////////////////////////////////////////////////////////////
#include "ZOSFile.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZPESPacket::ZPESPacket(UINT nPacketPID,UINT nType)
:m_nPacketPID(nPacketPID)
,m_nPacketType(nType)
,m_nStreamID(0xFF)
,m_nPacketPCR(0)
,m_nPacketScrambled(0)
,m_nPacketCC(0)
,m_nPacketHeader(6)
,m_nPacketPayload(0)
,m_nSampleNumber(0)
,m_nSamplePTS(0)
,m_nSampleDTS(0)
,m_nSystemPCR(0)
,m_nBufferLength(0)
,m_nBufferPoint(0)
,m_nPacketNumber(0)
,m_bBufferComplete(FALSE)
{
	m_sBuffer	= NULL;
	m_nBuffer	= 0;
}
ZPESPacket::~ZPESPacket()
{
	ZPESPacket::Close();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPESPacket::Create()
{
	ZPESPacket::Close();

	if(ZMPEGFormat::MPEGHDVideo(m_nPacketType))
	{
		m_sBuffer	= NEW BYTE[DEFAULT_PES_VIDEO_BUFFER_MAX];
		m_nBuffer	= DEFAULT_PES_VIDEO_BUFFER_MAX;
	}else{
		m_sBuffer	= NEW BYTE[DEFAULT_PES_BUFFER_MAX];
		m_nBuffer	= DEFAULT_PES_BUFFER_MAX;
	}
	return TRUE;
}
BOOL ZPESPacket::Close()
{
	m_nStreamID			= 0xFF;
	m_nPacketPCR		= 0;
	m_nPacketScrambled	= 0;
	m_nPacketCC			= 0;
	m_nPacketHeader		= 6;
	m_nPacketPayload	= 0;
	m_nSampleNumber		= 0;
	m_nSamplePTS		= 0;
	m_nSampleDTS		= 0;
	m_nSystemPCR		= 0;
	m_nBufferLength		= 0;
	m_nBufferPoint		= 0;
	m_bBufferComplete	= FALSE;

	SAFE_DELETE(m_sBuffer);
	m_nBuffer			= 0;

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZPESPacket::GetPID()
{
	return m_nPacketPID;
}
UINT ZPESPacket::GetSID()
{
	return m_nStreamID;
}
UINT ZPESPacket::SetSID(UINT nSID)
{
	m_nStreamID	= nSID;
	return m_nStreamID;
}
BOOL ZPESPacket::SetPCR(BOOL bPCR)
{
	if(bPCR)
	{
		m_nPacketPCR	= 0x10;
	}else{
		m_nPacketPCR	= 0x00;
	}
	return ((BOOL)m_nPacketPCR);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT64 ZPESPacket::GetSampleNumber()
{
	return m_nSampleNumber;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZPESPacket::GetLength()
{
	if(m_nBufferLength > 0)
	{
		return m_nBufferLength;
	}
	return m_nPacketHeader;
}
BOOL ZPESPacket::GetComplete()
{
	if(m_nBufferLength > 0)
	{
		if(m_nBufferPoint > 0)
		{
			if(m_nBufferPoint >= m_nBufferLength)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD ZPESPacket::GetSample(CHAR* sSample,UINT nSample)
{
	return 0;
}
DWORD ZPESPacket::SetSample(CHAR* sSample,UINT nSample,UINT64 nSamplePTS,UINT64 nSampleDTS)
{
	if(sSample != NULL && nSample < m_nBuffer)
	{
		memcpy(&m_sBuffer[19],sSample,nSample);
		m_nBufferLength		= (19 + nSample);
		m_nBufferPoint		= 0;
		m_bBufferComplete	= FALSE;
		m_nSampleNumber		++;
		m_nSystemPCR		= nSamplePTS;
		m_nSamplePTS		= nSamplePTS;
		m_nSampleDTS		= nSampleDTS;

		return nSample;
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD ZPESPacket::GetBuffer(char* sdata,int ndata)
{
	UINT16	nLength	= 0;

	if(sdata != NULL && ndata > 0)
	{
		if((m_nBufferPoint + ndata) < m_nBufferLength)
		{
			nLength		= ndata;
		}else{
			nLength		= (m_nBufferLength - m_nBufferPoint);
		}
		memcpy(sdata,&m_sBuffer[m_nBufferPoint],nLength);
		m_nBufferPoint	+= nLength;
	}
	return nLength;
}
DWORD ZPESPacket::AppendBuffer(char* sdata,int ndata)
{
	UINT16	nLength	= 0;

	if(sdata != NULL && ndata > 0)
	{
		if(m_nBufferPoint + ndata < m_nBuffer)
		{
			memcpy(&m_sBuffer[m_nBufferPoint],sdata,ndata);
			m_nBufferPoint	+= ndata;
			if(m_nBufferPoint >= m_nPacketHeader)
			{
				if(m_nBufferLength == 0)
				{
					m_nBufferLength	= (((m_sBuffer[4]&0xFF) << 8) | (m_sBuffer[5]&0xFF));
					if(m_nBufferLength > 0)
					{
						m_nBufferLength	+= m_nPacketHeader;
					}else{
						m_nBufferLength	= m_nBufferPoint;
					}
				}
			}
			nLength	= ndata;
		}
	}
	return nLength;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPESPacket::Parse(char* sdata,int ndata)
{
	UINT			bUintStart			= 0;
	UINT			nUintPID			= 0;
	UINT			bUintScrambled		= 0;
	UINT			bUintAdapation		= 0;
	UINT			bUintPayload		= 0;
	UINT			nUintCC				= 0;
	UINT			bUintDiscontinuity	= 0;
	UINT			bUintPCR			= 0;
	CHAR*			pUintPayload		= NULL;
	UINT			nUintPayload		= 0;

	if(sdata != NULL && ndata > 0)
	{
		if((sdata[0]&0xFF) == 0x47)
		{
			bUintStart		= (sdata[1]&0x40);
			nUintPID		= (((sdata[1]&0x1F)<<8)|(sdata[2]&0xFF));
			bUintScrambled	= (sdata[3]&0x80);
			bUintAdapation	= (sdata[3]&0x20);
			bUintPayload	= (sdata[3]&0x10);
			nUintCC			= (sdata[3]&0x0F);

			TMASSERT((nUintPID==m_nPacketPID));

			if(bUintPayload)
			{
				if(bUintAdapation)
				{
					bUintDiscontinuity	= (sdata[5]&0x80);
					bUintPCR			= (sdata[5]&0x10);
					ZPESPacket::ParsePCR(sdata,ndata);
					pUintPayload	= (sdata + 5 + (sdata[4]&0xFF));
				}else{
					pUintPayload	= (sdata + 4);
				}

				nUintPayload	= ndata + (sdata - pUintPayload);
				if(bUintStart)
				{
					ZPESPacket::DecodePacket();
				}
				ZPESPacket::AppendBuffer(pUintPayload,nUintPayload);
				m_bBufferComplete	= ZPESPacket::GetComplete();
				if(m_bBufferComplete)
				{
					ZPESPacket::DecodePacket();
				}
			}
		}
		return TRUE;
	}

	return FALSE;
}
BOOL ZPESPacket::Write(char* sdata,int ndata)
{
	UINT			bUintStart			= 0;
	UINT			nUintPID			= 0;
	UINT			bUintScrambled		= 0;
	UINT			bUintAdapation		= 0;
	UINT			bUintPayload		= 0;
	UINT			nUintCC				= 0;
	UINT			bUintDiscontinuity	= 0;
	UINT			bUintPCR			= 0;
	UINT			nUintExtension		= 0;
	CHAR*			pUintPayload		= NULL;
	UINT			nUintPayload		= 0;
	UINT			nUintBuffer			= 0;

	if(!m_bBufferComplete)
	{
		ZPESPacket::EncodePacket();
	}

	if(sdata != NULL && ndata > 0 && m_nBufferPoint < m_nBufferLength)
	{
		m_nPacketPayload= MIN((m_nBufferLength-m_nBufferPoint),(UINT32)(m_nPacketPCR?(184-8):184));
		bUintStart		= ((m_nBufferPoint==0)?0x40:0x00);
		nUintPID		= m_nPacketPID;
		bUintPCR		= m_nPacketPCR;
		nUintExtension	= (m_nPacketPCR?(184-8-m_nPacketPayload):(184-m_nPacketPayload));
		bUintScrambled	= m_nPacketScrambled;
		bUintAdapation	= ((bUintPCR||nUintExtension)?0x20:0x00);
		bUintPayload	= (m_nPacketPayload?0x10:0x00);
		nUintCC			= (m_nPacketCC);
		bUintDiscontinuity	= 0x00;
		//
		sdata[0]		= 0x47;
		sdata[1]		= ((bUintStart&0x40)|((nUintPID>>8)&0x1F));
		sdata[2]		= (nUintPID&0xFF);
		sdata[3]		= ((bUintScrambled&0x80)|(bUintAdapation&0x20)|(bUintPayload&0x10)|(nUintCC&0x0F));

		if(bUintAdapation)
		{
			memset(&sdata[4],0xFF,(ndata-4));
			if(bUintPCR)
			{
				sdata[4]		= (8+nUintExtension-1);
				sdata[5]		= ((bUintDiscontinuity&0xFF)|0x10);
				ZPESPacket::WritePCR(sdata,ndata);
				pUintPayload	= &sdata[13+nUintExtension-1];
				nUintPayload	= (ndata-(13+nUintExtension-1));
			}else{
				sdata[4]		= (nUintExtension-1);
				if(nUintExtension>1)
				{
					sdata[5]		= 0x00;
				}
				pUintPayload	= &sdata[5+nUintExtension-1];
				nUintPayload	= (ndata-(5+nUintExtension-1));
			}
		}else{
			pUintPayload	= &sdata[4];
			nUintPayload	= (ndata-4);
		}
		TMASSERT((m_nPacketPayload==nUintPayload));
		nUintBuffer	= ZPESPacket::GetBuffer(pUintPayload,nUintPayload);
		m_nPacketCC	++;
		if(m_nPacketCC > 0x0F)
		{
			m_nPacketCC	= 0;
		}
		m_nPacketNumber	++;
		if((m_nPacketNumber+1)%80==0)
		{
			m_nPacketPCR	= 0x10;
		}
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPESPacket::ParsePCR(char* sdata,int ndata)
{
	UINT64	nPCR	= 0;

	if(sdata != NULL && ndata >= 188)
	{
		if((sdata[3]&0x20)&&((sdata[4]&0xFF)>=7)&&(sdata[5]&0x10))
		{
			nPCR	= (
						(UINT64)((sdata[ 6]&0xFF)<<25)|
						(UINT64)((sdata[ 7]&0xFF)<<17)|
						(UINT64)((sdata[ 8]&0xFF)<< 9)|
						(UINT64)((sdata[ 9]&0xFF)<< 1)|
						(UINT64)((sdata[10]&0xFF)>> 7)
					  );
			if(m_nSystemPCR==0)
			{
				m_nSystemPCR	= nPCR;
				m_nPacketPCR	= (sdata[5]&0x10);
			}
		}
	}
	return FALSE;
}
BOOL ZPESPacket::WritePCR(char* sdata,int ndata)
{
	UINT64	nPCR	= 0;

	if(sdata != NULL && ndata >= 188)
	{
		if((sdata[3]&0x20)&&((sdata[4]&0xFF)>=7)&&(sdata[5]&0x10))
		{
			nPCR			= m_nSystemPCR;
			sdata[ 6]		= ((nPCR>>25)&0xFF);
			sdata[ 7]		= ((nPCR>>17)&0xFF);
			sdata[ 8]		= ((nPCR>> 9)&0xFF);
			sdata[ 9]		= ((nPCR>> 1)&0xFF);
			sdata[10]		= ((nPCR<< 7)&0x80);
			sdata[10]		|= 0x7E;
			sdata[11]		= 0x00;
			m_nPacketPCR	= 0x00;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPESPacket::ParsePESHeader()
{
	UINT8	nStreamID		= 0;
	UINT16	nLength			= 0;
	UINT16	nSkip			= 0;
	UINT64	nPTS			= 0;
	UINT64	nDTS			= 0;

	if(m_sBuffer != NULL && m_nBufferLength > 0)
	{
		//00 00 01
		if((m_sBuffer[0]&0xFF)==0x00&&(m_sBuffer[1]&0xFF)==0x00&&(m_sBuffer[2]&0xFF)==0x01)
		{
			//Stream ID
			nStreamID		= (m_sBuffer[3]&0xFF);
			m_nStreamID		= nStreamID;
			//Stream Length
			nLength			= (((m_sBuffer[4]&0xFF)<<8)|(m_sBuffer[5]&0xFF));
			//
			switch(nStreamID)
			{
			case 0xBC:	//1011 1100
			case 0xBE:	//1011 1110
			case 0xBF:	//1011 1111
			case 0xF0:	//1111 0000	ECM
			case 0xF1:	//1111 0001	EMM
			case 0xF2:	//1111 0010
			case 0xF8:	//1111 1000
			case 0xFF:	//1111 1111
				{
					nSkip	= 6;
				}
				break;
			default:
				{
					if((m_sBuffer[6]&0xC0)==0x80)	//MPEG2
					{
						nSkip	= ((m_sBuffer[8]&0xFF)+9);
						if((m_sBuffer[7]&0x80))
						{
							nPTS	= (	((INT64)(m_sBuffer[ 9]&0x0E)<<29)|
										((INT64)(m_sBuffer[10]&0xFF)<<22)|
										((INT64)(m_sBuffer[11]&0xFE)<<14)|
										((INT64)(m_sBuffer[12]&0xFF)<< 7)|
										((INT64)(m_sBuffer[13]&0xFE)>> 1));
							m_nSamplePTS	= nPTS;
						}
						if((m_sBuffer[7]&0x40))
						{
							nDTS	= (	((INT64)(m_sBuffer[14]&0x0E)<<29)|
										((INT64)(m_sBuffer[15]&0xFF)<<22)|
										((INT64)(m_sBuffer[16]&0xFE)<<14)|
										((INT64)(m_sBuffer[17]&0xFF)<< 7)|
										((INT64)(m_sBuffer[18]&0xFF)>> 1));
							m_nSampleDTS	= nDTS;
						}
					}else{						//MPEG1
						nSkip	= 6;
						while((nSkip<23)&&((m_sBuffer[nSkip]&0xFF)==0xFF))
						{
							nSkip	++;
						}
						if(nSkip<23)
						{
							if(((m_sBuffer[nSkip]&0xC0)==0x40))
							{
								nSkip	+= 2;
							}
							if((m_sBuffer[nSkip]&0x20))
							{
								nPTS	= (	((INT64)(m_sBuffer[nSkip+0]&0x0E)<<29)|
											((INT64)(m_sBuffer[nSkip+1]&0xFF)<<22)|
											((INT64)(m_sBuffer[nSkip+2]&0xFE)<<14)|
											((INT64)(m_sBuffer[nSkip+3]&0xFF)<< 7)|
											((INT64)(m_sBuffer[nSkip+4]&0xFF)>> 1));
								nSkip	+= 5;
								if((m_sBuffer[nSkip]&0x10))
								{
									nDTS	= (	((INT64)(m_sBuffer[nSkip+0]&0x0E)<<29)|
												((INT64)(m_sBuffer[nSkip+1]&0xFF)<<22)|
												((INT64)(m_sBuffer[nSkip+2]&0xFE)<<14)|
												((INT64)(m_sBuffer[nSkip+3]&0xFF)<< 7)|
												((INT64)(m_sBuffer[nSkip+4]&0xFF)>> 1));
									nSkip	+= 5;
								}
							}else{
								nSkip	+= 1;
							}
						}
					}
				}
				break;
			}
			//
		}
		return TRUE;
	}
	return FALSE;
}
BOOL ZPESPacket::WritePESHeader()
{
	UINT8	nStreamID		= 0;
	UINT16	nLength			= 0;
	UINT16	nSkip			= 0;
	UINT64	nPTS			= 0;
	UINT64	nDTS			= 0;

	if(m_sBuffer != NULL && m_nBufferLength > 0)
	{
		nStreamID		= m_nStreamID;
		if((m_nBufferLength - m_nPacketHeader) > DEFAULT_PES_LENGTH_MAX)
		{
			nLength		= 0;
		}else{
			nLength		= (m_nBufferLength - m_nPacketHeader);
		}
		nPTS			= m_nSamplePTS;
		nDTS			= m_nSampleDTS;
		//00 00 01
		m_sBuffer[0]	= 0x00;
		m_sBuffer[1]	= 0x00;
		m_sBuffer[2]	= 0x01;
		//Stream ID
		m_sBuffer[3]	= (nStreamID&0xFF);

		switch(nStreamID)
		{
			case 0xBC:	//1011 1100
			case 0xBE:	//1011 1110
			case 0xBF:	//1011 1111
			case 0xF0:	//1111 0000	ECM
			case 0xF1:	//1111 0001	EMM
			case 0xF2:	//1111 0010
			case 0xF8:	//1111 1000
			case 0xFF:	//1111 1111
				{
					m_sBuffer[4]	= ((nLength>>8)&0xFF);
					m_sBuffer[5]	= ((nLength)&0xFF);
					nSkip			= 6;
				}
				break;
			default:
				{
					m_sBuffer[4]	= ((nLength>>8)&0xFF);
					m_sBuffer[5]	= ((nLength)&0xFF);
					m_sBuffer[6]	= (0x80);			//MPEG2
					m_sBuffer[7]	= ((0x80)|(0x40));	//pts & dts
					m_sBuffer[8]	= 0x0A;				//Header Size
					nSkip			= 9;
					{
						if((m_sBuffer[7]&0x80))
						{
							m_sBuffer[ 9]	= (((m_sBuffer[7]&0x40)?0x30:0x20)|((nPTS>>29)&0x0E)|0x01);
							m_sBuffer[10]	= ((nPTS>>22)&0xFF);
							m_sBuffer[11]	= (((nPTS>>14)&0xFE)|0x01);
							m_sBuffer[12]	= ((nPTS>>7)&0xFF);
							m_sBuffer[13]	= (((nPTS<<1)&0xFE)|0x01);
							nSkip			+= 5;
						}
						if((m_sBuffer[7]&0x40))
						{
							m_sBuffer[14]	= ((0x10)|((nPTS>>29)&0xFF)|0x01);
							m_sBuffer[15]	= ((nPTS>>22)&0xFF);
							m_sBuffer[16]	= (((nPTS>>14)&0xFE)|0x01);
							m_sBuffer[17]	= ((nPTS>>7)&0xFF);
							m_sBuffer[18]	= (((nPTS<<1)&0xFE)|0x01);
							nSkip			+= 5;
						}
					}
				}
				break;
		}
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPESPacket::DecodePacket()
{
	if(ZPESPacket::ParsePESHeader())
	{
		m_nBufferPoint		= 0;
		m_bBufferComplete	= FALSE;
		return TRUE;
	}
	return FALSE;
}
BOOL ZPESPacket::EncodePacket()
{
	if(ZPESPacket::WritePESHeader())
	{
		//m_nBufferLength		= 0;
		m_nBufferPoint		= 0;
		m_bBufferComplete	= TRUE;
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZPESPacket* ZPESPacket::CreatePacket(UINT nPacketPID,UINT nType,UINT nExternID)
{
	ZPESPacket*	pPESPacket	= NEW ZPESPacket(nPacketPID,nType);

	if(pPESPacket != NULL)
	{
		if(pPESPacket->Create())
		{
			pPESPacket->SetSID(nExternID);
		}else{
			SAFE_DELETE(pPESPacket);
		}
	}
	return pPESPacket;
}
ZPESPacket* ZPESPacket::ClosePacket(ZPESPacket* pPacket)
{
	SAFE_DELETE(pPacket);
	return pPacket;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
