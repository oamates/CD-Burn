#include "Packetizer3016.h"
#include "ZUDPSocket.h"

Packetizer3016::Packetizer3016()
{
    //
}

Packetizer3016::~Packetizer3016()
{
    //
}

int Packetizer3016::ProcessFrame(char *pData, int nData)
{
    unsigned char sRTPPacket[DEFAULT_UDP_BUFFER_USED];
    int nCurSeqNo;


    nCurSeqNo = GetSeqNo();
    IncreaseSeqNo();
    memset(sRTPPacket, 0, DEFAULT_UDP_BUFFER_USED);
    sRTPPacket[0] = 0x80;
    sRTPPacket[1] = GetPayloadType()&0x7F;
    //marker=true
    sRTPPacket[1] = sRTPPacket[1]|0x80;
    sRTPPacket[2] = (nCurSeqNo&0xFF00)>>8;
    sRTPPacket[3] = nCurSeqNo&0xFF;
    UINT nRtpTimeStamp = GetFrameTimestamp();
    sRTPPacket[4] = (nRtpTimeStamp&0xFF000000)>>24;
    sRTPPacket[5] = (nRtpTimeStamp&0xFF0000)>>16;
    sRTPPacket[6] = (nRtpTimeStamp&0xFF00)>>8;
    sRTPPacket[7] = (nRtpTimeStamp&0xFF);
    sRTPPacket[8] = (GetSSRC()&0xFF000000)>>24;
    sRTPPacket[9] = (GetSSRC()&0xFF0000)>>16;
    sRTPPacket[10] = (GetSSRC()&0xFF00)>>8;
    sRTPPacket[11] = (GetSSRC()&0xFF);
    if (nData > 0xFF)
    {
        sRTPPacket[12] = 0xFF;
        sRTPPacket[13] = nData-0xFF;
        memcpy(sRTPPacket+14, pData, nData);
        SendData(1, sRTPPacket, 14+nData);
    }
    else
    {
        sRTPPacket[12] = nData;
        memcpy(sRTPPacket+13, pData, nData);
        SendData(1, sRTPPacket, 13+nData);
    }

    return 0;
}
