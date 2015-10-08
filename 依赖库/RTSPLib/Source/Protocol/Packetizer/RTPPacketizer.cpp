#include "RTPPacketizer.h"

RTPPacketizer::RTPPacketizer()
: m_nPacketizerFlag(0)
, m_nPayloadType(0)
, m_nSSRC(0)
, m_nCurTimeStamp(0)
, m_nSeqNo(0)
, m_bPatchFrame(FALSE)
, m_nRTPPacketMaxSize(1440)
{

}

RTPPacketizer::~RTPPacketizer()
{

}

void RTPPacketizer::SetPacketizerFlag(int nFlag)
{
    m_nPacketizerFlag = nFlag;
}

int RTPPacketizer::GetPacketizerFlag()
{
    return m_nPacketizerFlag;
}

void RTPPacketizer::SetPayloadType(int nPayloadType)
{
    m_nPayloadType = nPayloadType;
}

int RTPPacketizer::GetPayloadType()
{
    return m_nPayloadType;
}

void RTPPacketizer::SetSSRC(UINT nSSRC)
{
    m_nSSRC = nSSRC;
}

UINT RTPPacketizer::GetSSRC()
{
    return m_nSSRC;
}

void RTPPacketizer::SetFrameTimestamp(int nTimeStamp)
{
    m_nCurTimeStamp = nTimeStamp;
}

UINT RTPPacketizer::GetFrameTimestamp()
{
    return m_nCurTimeStamp;
}

int RTPPacketizer::GetSeqNo()
{
    return m_nSeqNo;
}

void RTPPacketizer::IncreaseSeqNo()
{
    if (m_nSeqNo == 0xFFFF)
    {
        m_nSeqNo = 0;
    }
	else
	{
		m_nSeqNo ++;
	}
}

int RTPPacketizer::ReceiveFrameData(int nFlag, void *pData, int nData)
{
    return ProcessFrame((char*)pData, nData);
}

void RTPPacketizer::SetPatchFrame(BOOL bPatchFrame)
{
    //LOG_DEBUG(("[RTPPacketizer::SetPatchFrame] this %d patch frame %d\r\n", 
    //    this, bPatchFrame));
    m_bPatchFrame = bPatchFrame;
}

BOOL RTPPacketizer::IsPatchFrame()
{
    return m_bPatchFrame;
}

void RTPPacketizer::SetRTPPacketMaxSize(int nPacketSize)
{
    if (nPacketSize < 400)
    {
        nPacketSize = 400;
    }
    else if (nPacketSize >= 1460)
    {
        nPacketSize = 1440;
    }
    m_nRTPPacketMaxSize = nPacketSize;
}

int RTPPacketizer::GetRTPPacketMaxSize()
{
    return m_nRTPPacketMaxSize;
}

//////////////////////////////////////////////////////////////////////////
//BOOL RTPPacketizer::OnCommand(int nCommand)
//{
//    return TRUE;
//}
//
//BOOL RTPPacketizer::OnHeader(int nFlag,void* pHeader,int nHeader)
//{
//    return TRUE;
//}
//
//BOOL RTPPacketizer::OnData(int nFlag,void* pData,int nData)
//{
//    return TRUE;
//}

int RTPPacketizer::ProcessFrame(char *pData, int nData)
{
    SendData(1, pData, nData);

    return 0;
}

