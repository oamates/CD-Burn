#include "Parser3640.h"
#include "AACUtil.h"

Parser3640::Parser3640()
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

Parser3640::~Parser3640()
{
    //
}

void Parser3640::SetAdtsHeaderParameter(int nObjectType, int nSampleRate, int nChannels, BOOL bWithAdtsHeader)
{
    m_nObjType = nObjectType;
    m_nSampleRate = nSampleRate;
    m_nChannel = nChannels;
    m_bWithAdtsHeader = bWithAdtsHeader;
}

//FILE *g_aacFile = NULL;

int Parser3640::ProcessMarker(int nMarkerPos)
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
            if (m_bWithAdtsHeader)
            {
//                if (g_aacFile == NULL)
//                {
//                    g_aacFile = fopen("/home/by.aac", "w+b");
//                }
                memset(sAdtsHeader, 0, 7);
                FormatADTSHeader(1, nRTPPacketPayloadLen-12-4, m_nSampleRate, m_nChannel, sAdtsHeader);
                memcpy(m_pFrameBuffer, sAdtsHeader, 7);
                //LOG_DEBUG(("[Parser3640::ProcessMarker] %.2x %.2x %.2x %.2x %.2x %.2x %.2x\r\n",
                //    sAdtsHeader[0], sAdtsHeader[1], sAdtsHeader[2], sAdtsHeader[3],
                //    sAdtsHeader[4], sAdtsHeader[5], sAdtsHeader[6]));
                //
                memcpy(m_pFrameBuffer+7, pRTPPacket+12+4, nRTPPacketPayloadLen-12-4);

                if (m_pParserListener != NULL)
                {
                    GetFrameTimestamp(m_nSampleRate, m_arrayRTPPacket[0]->GetTimeStamp(), &nNTPSecond, &nNTPUSecond);
                    m_pParserListener->ParserOnData(m_nParserFlag, m_pFrameBuffer, nRTPPacketPayloadLen-12-4+7, nNTPSecond, nNTPUSecond);
//                    fwrite(m_pFrameBuffer, nRTPPacketPayloadLen-12-4+7, 1, g_aacFile);
//                    fflush(g_aacFile);
                }
            }
            else
            {
                memcpy(m_pFrameBuffer, pRTPPacket+12+4, nRTPPacketPayloadLen-12-4);

                if (m_pParserListener != NULL)
                {
                    GetFrameTimestamp(m_nSampleRate, m_arrayRTPPacket[0]->GetTimeStamp(), &nNTPSecond, &nNTPUSecond);
                    m_pParserListener->ParserOnData(m_nParserFlag, m_pFrameBuffer, nRTPPacketPayloadLen-12-4, nNTPSecond, nNTPUSecond);
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
		m_nFps = m_nFrames * 1000/ diff;
		m_nFrames = 0;
		m_tFpsTime = ZOS::milliseconds();
	}

    return 0;
}
