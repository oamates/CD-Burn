#ifndef _PACKETIZER3640_H_
#define _PACKETIZER3640_H_

#include "RTPPacketizer.h"

class Packetizer3640
    : public RTPPacketizer
{
public:
    virtual int ProcessFrame(char *pData, int nData);
public:
    Packetizer3640();
    virtual ~Packetizer3640();
private:
    INT64       m_nWaitStartTime;
    UINT        m_nRelativeTimestamp;
    INT         m_nFrameCount;
    INT64       m_nLastPrintfTime;
};

#endif //_PACKETIZER3640_H_
