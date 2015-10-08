#ifndef _PARSER3016_H_
#define _PARSER3016_H_

#include "RTPParser.h"

class Parser3016
    : public RTPParser
{
public:
    void SetAdtsHeaderParameter(int nObjectType, int nSampleRate, int nChannels, BOOL bWithAdtsHeader);
protected:
    virtual int ProcessMarker(int nMarkerPos);
public:
    Parser3016();
    virtual ~Parser3016();
private:
    int         m_nObjType;
    int         m_nSampleRate;
    int         m_nChannel;
    BOOL        m_bWithAdtsHeader;
};


#endif //_PARSER3016_H_
