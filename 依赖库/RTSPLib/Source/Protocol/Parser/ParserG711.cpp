#include "ParserG711.h"

ParserG711::ParserG711()
: RTPParser()
{
    m_pFrameBuffer = NEW char [G711FRAME_MAX_SIZE];
    //SetSpliceThreshold(10);
    SetPoolCount(50);
}

ParserG711::~ParserG711()
{

}

int ParserG711::ProcessMarker(int nMarkerPos)
{
    char *pRTPPacket = NULL;
    int nRTPPacketPayloadLen = 0;
    long nNTPSecond;
    long nNTPUSecond;
    int nCurrentFrameBitCount = 0;

    TMASSERT((nMarkerPos == 0));
    if (m_pFrameBuffer != NULL)
    {
        if (m_arrayRTPPacket[0] != NULL)
        {
            nCurrentFrameBitCount = (nRTPPacketPayloadLen-12)*8;

            pRTPPacket = m_arrayRTPPacket[0]->GetPacketPointer();
            nRTPPacketPayloadLen = m_arrayRTPPacket[0]->GetPacketPayloadLength();
            memcpy(m_pFrameBuffer, pRTPPacket+12, nRTPPacketPayloadLen-12);
            if (m_pParserListener != NULL)
            {
                GetFrameTimestamp(8000, m_arrayRTPPacket[0]->GetTimeStamp(), &nNTPSecond, &nNTPUSecond);
                m_pParserListener->ParserOnData(m_nParserFlag, m_pFrameBuffer, nRTPPacketPayloadLen-12, nNTPSecond, nNTPUSecond);
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

