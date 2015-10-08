#ifndef _PACKETIZERG711_H_
#define _PACKETIZERG711_H_

#include "RTPPacketizer.h"

class PacketizerG711
    : public RTPPacketizer
{
public:
    virtual int ProcessFrame(char *pData, int nData);
public:
    PacketizerG711();
    virtual ~PacketizerG711();
};

#endif //_PACKETIZERG711_H_
