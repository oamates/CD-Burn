#include "Packetizer3984.h"
#include "ZUDPSocket.h"
#include "ZRTPPacket.h"
#include "NalUtil.h"

Packetizer3984::Packetizer3984()
{
    m_nLastTimestamp = 0;
}

Packetizer3984::~Packetizer3984()
{
    //
}

int Packetizer3984::ProcessSingle(char *sNalData, int nNalData)
{
    unsigned char sRTPPacket[DEFAULT_UDP_BUFFER_USED];
    int nCurSeqNo;


    nCurSeqNo = GetSeqNo();
    IncreaseSeqNo();
    memset(sRTPPacket, 0, DEFAULT_UDP_BUFFER_USED);
    sRTPPacket[0] = 0x80;
    sRTPPacket[1] = GetPayloadType()&0x7F;
	if((sNalData[4] == 0x06) || (sNalData[4] == 0x67) || (sNalData[4] == 0x68))
	{
		// marker = false
		sRTPPacket[1] = sRTPPacket[1]&0x7F;
		//printf("##############################################################\n");
	}
	else
	{
		//marker=true
		sRTPPacket[1] = sRTPPacket[1]|0x80;
	}
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

    //if (nCurSeqNo % 1000 == 0)
    //{
    //    LOG_DEBUG(("[Packetizer3984::ProcessSingle] %x microSecond = %"F_NUM_64"d nCurSeqNo = %d timestamp = %d \r\n", 
    //        this, ZOS::microseconds(), nCurSeqNo, GetFrameTimestamp()));
    //}

    memcpy(sRTPPacket+12, sNalData+4, nNalData-4);
    SendData(1, sRTPPacket, 12+nNalData-4);

    return 0;
}

int Packetizer3984::ProcessFU(char *sData, int nData)
{
    unsigned char sRTPPacket[DEFAULT_UDP_BUFFER_USED];
    int nCurSeqNo;
    int nLeftSize = nData;
    int nRTPPacketMaxSize = GetRTPPacketMaxSize();
    int nEachProcessSize = nRTPPacketMaxSize-2;
    bool bStartPakcet = true;
    bool bEndPacket = false;
    int nCurPos = 5;


    while (nLeftSize > 0)
    {
        nCurSeqNo = GetSeqNo();
        IncreaseSeqNo();
        memset(sRTPPacket, 0, DEFAULT_UDP_BUFFER_USED);
        sRTPPacket[0] = 0x80;
        sRTPPacket[1] = GetPayloadType()&0x7F;
        // marker = false
        sRTPPacket[1] = sRTPPacket[1]&0x7F;
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
        //3bit
        sRTPPacket[12] = (sData[4]&0xE0);
        //5bit
        sRTPPacket[12] = (sRTPPacket[12]|28);

        //if (nCurSeqNo % 1000 == 0)
        //{
        //    LOG_DEBUG(("[Packetizer3984::ProcessFU] %x microSecond = %"F_NUM_64"d nCurSeqNo = %d timestamp = %d \r\n", 
        //        this, ZOS::microseconds(), nCurSeqNo, GetFrameTimestamp()));
        //}

        if (nLeftSize <= nEachProcessSize)
        {
            bEndPacket = true;
        }

        if (bStartPakcet)
        {
            //3bit
            sRTPPacket[13] = 0x80;
            bStartPakcet = false;
            //5bit
            sRTPPacket[13] = (sRTPPacket[13]|(sData[4]&0x1F));
            memcpy(sRTPPacket+14, sData+nCurPos, nEachProcessSize);
            SendData(1, sRTPPacket, 14+nEachProcessSize);
            nCurPos += nEachProcessSize;
            nLeftSize -= nEachProcessSize;
        }
        else if (bEndPacket)
        {
            //3bit
            sRTPPacket[13] = 0x40;
            //5bit
            sRTPPacket[13] = (sRTPPacket[13]|(sData[4]&0x1F));
            //marker=true
            sRTPPacket[1] = sRTPPacket[1]|0x80;
            memcpy(sRTPPacket+14, sData+nCurPos, nLeftSize);
            SendData(1, sRTPPacket, 14+nLeftSize);
            nCurPos += nLeftSize;
            nLeftSize = 0;
        }
        else
        {
            //3bit
            sRTPPacket[13] = 0x00;
            //5bit
            sRTPPacket[13] = (sRTPPacket[13]|(sData[4]&0x1F));
            memcpy(sRTPPacket+14, sData+nCurPos, nEachProcessSize);
            SendData(1, sRTPPacket, 14+nEachProcessSize);
            nCurPos += nEachProcessSize;
            nLeftSize -= nEachProcessSize;
        }

    }

    return 0;
}

int Packetizer3984::ProcessFrame(char *pData, int nData)
{
    int nCurDataPos = 0;
    int nLeftDataSize = nData;
    int nRelativePos = 0;
    int nNalSize = nData;
    int nRTPPacketMaxSize = GetRTPPacketMaxSize();

    if (m_nLastTimestamp == 0)
    {
        m_nLastTimestamp = GetFrameTimestamp();
    }
    if (GetFrameTimestamp() > m_nLastTimestamp)
    {
        UINT nTimestampDiff = GetFrameTimestamp()-m_nLastTimestamp;
        if (nTimestampDiff > 10000)
        {
            //LOG_DEBUG(("[Packetizer3984::ProcessFrame] this %d h264 ts diff %d\r\n",
            //    this, nTimestampDiff));
        }
    }
    m_nLastTimestamp = GetFrameTimestamp();

    while (NALUTIL_GetNal(pData+nCurDataPos, nLeftDataSize, &nRelativePos, &nNalSize) != NULL)
    {
        if ((nNalSize-4) > nRTPPacketMaxSize)
        {
            ProcessFU(pData+nCurDataPos+nRelativePos, nNalSize);
        }
        else
        {
            ProcessSingle(pData+nCurDataPos+nRelativePos, nNalSize);
        }

        nLeftDataSize-=nNalSize;
        nCurDataPos+=nNalSize;
    }

    return 0;
}
