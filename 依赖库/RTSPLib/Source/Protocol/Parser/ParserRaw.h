#ifndef _PARSERRAW_H_
#define _PARSERRAW_H_

#include "RTPParser.h"

class ParserRaw
    : public RTPParser
{
protected:
    virtual int ProcessPacket();
public:
    ParserRaw();
    virtual ~ParserRaw();
};

#endif //_PARSERRAW_H_
