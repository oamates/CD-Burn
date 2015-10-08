#include "Parser3016.h"
#include "AACUtil.h"

Parser3016::Parser3016()
: RTPParser()
{
    m_pFrameBuffer = NEW char [AACFRAME_MAX_SIZE];
    //SetSpliceThreshold(3);
    SetPoolCount(200);

    m_nObjType = 1;
    m_nSampleRate = 48000;
    m_nChannel = 2;
    m_bWithAdtsHeader = FALSE;
}

Parser3016::~Parser3016()
{
    //
}

void Parser3016::SetAdtsHeaderParameter(int nObjectType, int nSampleRate, int nChannels, BOOL bWithAdtsHeader)
{
    m_nObjType = nObjectType;
    m_nSampleRate = nSampleRate;
    m_nChannel = nChannels;
    m_bWithAdtsHeader = bWithAdtsHeader;
}

int Parser3016::ProcessMarker(int nMarkerPos)
{
    char *pRTPPacket = NULL;
    int nRTPPacketPayloadLen = 0;
    unsigned char sAdtsHeader[7];
    long nNTPSecond;
    long nNTPUSecond;

    TMASSERT((nMarkerPos == 0));
    if (m_pFrameBuffer != NULL)
    {
        if (m_arrayRTPPacket[0] != NULL)
        {
            pRTPPacket = m_arrayRTPPacket[0]->GetPacketPointer();
            nRTPPacketPayloadLen = m_arrayRTPPacket[0]->GetPacketPayloadLength();

            memset(sAdtsHeader, 0, 7);
            if (((unsigned char)(*(pRTPPacket+12)))==0xFF)
            {
                if (m_bWithAdtsHeader)
                {
                    FormatADTSHeader(1, nRTPPacketPayloadLen-14, m_nSampleRate, m_nChannel, sAdtsHeader);
                    memcpy(m_pFrameBuffer, sAdtsHeader, 7);

                    memcpy(m_pFrameBuffer+7, pRTPPacket+14, nRTPPacketPayloadLen-14);
                    if (m_pParserListener != NULL)
                    {
                        GetFrameTimestamp(m_nSampleRate, m_arrayRTPPacket[0]->GetTimeStamp(), &nNTPSecond, &nNTPUSecond);
                        m_pParserListener->ParserOnData(m_nParserFlag, m_pFrameBuffer, nRTPPacketPayloadLen-14+7, nNTPSecond, nNTPUSecond);
                    }
                }
                else
                {
                    memcpy(m_pFrameBuffer, pRTPPacket+14, nRTPPacketPayloadLen-14);
                    if (m_pParserListener != NULL)
                    {
                        GetFrameTimestamp(m_nSampleRate, m_arrayRTPPacket[0]->GetTimeStamp(), &nNTPSecond, &nNTPUSecond);
                        m_pParserListener->ParserOnData(m_nParserFlag, m_pFrameBuffer, nRTPPacketPayloadLen-14, nNTPSecond, nNTPUSecond);
                    }
                }
            }
            else
            {
                if (m_bWithAdtsHeader)
                {
                    FormatADTSHeader(1, nRTPPacketPayloadLen-13, m_nSampleRate, m_nChannel, sAdtsHeader);
                    memcpy(m_pFrameBuffer, sAdtsHeader, 7);
                    //
                    memcpy(m_pFrameBuffer+7, pRTPPacket+13, nRTPPacketPayloadLen-13);
                    if (m_pParserListener != NULL)
                    {
                        GetFrameTimestamp(m_nSampleRate, m_arrayRTPPacket[0]->GetTimeStamp(), &nNTPSecond, &nNTPUSecond);
                        m_pParserListener->ParserOnData(m_nParserFlag, m_pFrameBuffer, nRTPPacketPayloadLen-13+7, nNTPSecond, nNTPUSecond);
                    }
                }
                else
                {
                    memcpy(m_pFrameBuffer, pRTPPacket+13, nRTPPacketPayloadLen-13);
                    if (m_pParserListener != NULL)
                    {
                        GetFrameTimestamp(m_nSampleRate, m_arrayRTPPacket[0]->GetTimeStamp(), &nNTPSecond, &nNTPUSecond);
                        m_pParserListener->ParserOnData(m_nParserFlag, m_pFrameBuffer, nRTPPacketPayloadLen-13, nNTPSecond, nNTPUSecond);
                    }
                }
            }
        }
    }


	if(m_tFpsTime == 0)
	{
		m_tFpsTime = ZOS::milliseconds();
	}

	m_nFrames++;
	INT diff = (INT)(ZOS::milliseconds() - m_tFpsTime);
	if(diff > 1000)
	{
		m_nFps = m_nFrames * 1000 / diff;
		m_nFrames = 0;
		m_tFpsTime = ZOS::milliseconds();
	}


    return 0;
}
