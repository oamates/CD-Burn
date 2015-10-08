#include "PacketizerG711.h"
#include "ZUDPSocket.h"

PacketizerG711::PacketizerG711()
{
    //
}

PacketizerG711::~PacketizerG711()
{
    //
}

int PacketizerG711::ProcessFrame(char *pData, int nData)
{
    unsigned char sRTPPacket[DEFAULT_UDP_BUFFER_USED];
    int nCurSeqNo;


    nCurSeqNo = GetSeqNo();
    IncreaseSeqNo();
    memset(sRTPPacket, 0, DEFAULT_UDP_BUFFER_USED);
    sRTPPacket[0] = 0x80;
    sRTPPacket[1] = GetPayloadType()&0x7F;
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
    memcpy(sRTPPacket+12, pData, nData);
    SendData(1, sRTPPacket, 12+nData);

    return 0;
}
