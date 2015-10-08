#ifndef _PACKETIZER3016_H_
#define _PACKETIZER3016_H_

#include "RTPPacketizer.h"

class Packetizer3016
    : public RTPPacketizer
{
public:
    virtual int ProcessFrame(char *pData, int nData);
public:
    Packetizer3016();
    virtual ~Packetizer3016();
};

#endif //_PACKETIZER3016_H_
