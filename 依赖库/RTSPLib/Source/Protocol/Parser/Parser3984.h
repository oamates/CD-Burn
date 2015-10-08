#ifndef _PARSER3984_H_
#define _PARSER3984_H_

#include "RTPParser.h"

class Parser3984
    : public RTPParser
{
protected:
    virtual BOOL SequenceStrictlyInOrder(int nMarkerPos);
    virtual int ProcessMarker(int nMarkerPos);
protected:
    bool CheckSliceIsComplete(int nMarkerPos);
public:
    Parser3984();
    virtual ~Parser3984();
protected:
    BOOL     m_bFirstFrame;
};

#endif //_PARSER3984_H_
