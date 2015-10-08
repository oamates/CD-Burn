#include "Packetizer3640.h"
#include "ZUDPSocket.h"

Packetizer3640::Packetizer3640()
: m_nWaitStartTime(0)
, m_nRelativeTimestamp(0)
, m_nFrameCount(0)
, m_nLastPrintfTime(0)
{
    //
}

Packetizer3640::~Packetizer3640()
{
    //
}

int Packetizer3640::ProcessFrame(char *pData, int nData)
{
    unsigned char sRTPPacket[DEFAULT_UDP_BUFFER_USED];
    int nCurSeqNo;

    if (m_nWaitStartTime == 0)
    {
        m_nWaitStartTime = ZOS::milliseconds();
    }

    if (ZOS::milliseconds() - m_nWaitStartTime > 60000)
    {
        if (m_nRelativeTimestamp == 0)
        {
            m_nRelativeTimestamp = GetFrameTimestamp();
            m_nFrameCount = 0;
            m_nLastPrintfTime = ZOS::milliseconds();
            LOG_DEBUG(("[Packetizer3640::ProcessFrame] %d first time\r\n", this));
        }

        if (GetFrameTimestamp() < m_nRelativeTimestamp)
        {//process reverse
            m_nRelativeTimestamp = GetFrameTimestamp();
            m_nFrameCount = 0;
            m_nLastPrintfTime = ZOS::milliseconds();
            LOG_DEBUG(("[Packetizer3640::ProcessFrame] %d reverse\r\n", this));
        }
        else
        {
            if (GetFrameTimestamp() - m_nRelativeTimestamp > m_nFrameCount*1024+10240)
            {
                m_nRelativeTimestamp = GetFrameTimestamp();
                m_nFrameCount = 0;
                m_nLastPrintfTime = ZOS::milliseconds();
                LOG_DEBUG(("[Packetizer3640::ProcessFrame] %d adjust\r\n", this));
            }
        }

        m_nFrameCount ++;

        INT64 nCurrentTime = ZOS::milliseconds();
        if (nCurrentTime - m_nLastPrintfTime > 5000)
        {//each 5 sec print time
            LOG_DEBUG(("[Packetizer3640::ProcessFrame] %d m_nFrameCount %d %d %d %d %d\r\n",
                this,
                m_nFrameCount,
                m_nRelativeTimestamp,
                GetFrameTimestamp()-m_nRelativeTimestamp,
                (m_nFrameCount-1)*1024,
                GetFrameTimestamp()-m_nRelativeTimestamp-(m_nFrameCount-1)*1024
                ));
            m_nLastPrintfTime = nCurrentTime;
        }
    }

    nCurSeqNo = GetSeqNo();
    IncreaseSeqNo();
    memset(sRTPPacket, 0, DEFAULT_UDP_BUFFER_USED);
    sRTPPacket[0] = 0x80;
    sRTPPacket[1] = GetPayloadType()&0x7F;
    //marker=true
    sRTPPacket[1] = sRTPPacket[1]|0x80;
    sRTPPacket[2] = (nCurSeqNo&0xFF00)>>8;
    sRTPPacket[3] = nCurSeqNo&0xFF;
    sRTPPacket[4] = (GetFrameTimestamp()&0xFF000000)>>24;
    sRTPPacket[5] = (GetFrameTimestamp()&0xFF0000)>>16;
    sRTPPacket[6] = (GetFrameTimestamp()&0xFF00)>>8;
    sRTPPacket[7] = (GetFrameTimestamp()&0xFF);
    sRTPPacket[8] = (GetSSRC()&0xFF000000)>>24;
    sRTPPacket[9] = (GetSSRC()&0xFF0000)>>16;
    sRTPPacket[10] = (GetSSRC()&0xFF00)>>8;
    sRTPPacket[11] = (GetSSRC()&0xFF);
    sRTPPacket[12] = 0x00;
    sRTPPacket[13] = 0x10;
    sRTPPacket[14] = nData >> 5;
    sRTPPacket[15] = (nData&0x1F)<<3;

    memcpy(sRTPPacket+16, pData, nData);
    SendData(1, sRTPPacket, 16+nData);

    return 0;
}
