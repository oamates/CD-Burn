#ifndef _PARSERG711_H_
#define _PARSERG711_H_

#include "RTPParser.h"

class ParserG711
    : public RTPParser
{
protected:
    virtual int ProcessMarker(int nMarkerPos);
public:
    ParserG711();
    virtual ~ParserG711();
};

#endif //_PARSERG711_H_
