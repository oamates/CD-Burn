#ifndef _PACKETIZER3984_H_
#define _PACKETIZER3984_H_

#include "RTPPacketizer.h"

class Packetizer3984
    : public RTPPacketizer
{
public:
    virtual int ProcessFrame(char *pData, int nData);
protected:
    int ProcessSingle(char *sNalData, int nNalData);
    int ProcessFU(char *sData, int nData);
public:
    Packetizer3984();
    virtual ~Packetizer3984();
private:
    UINT        m_nLastTimestamp;
};

#endif //_PACKETIZER3984_H_
