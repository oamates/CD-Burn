#ifndef _PARSER3640_H_
#define _PARSER3640_H_

#include "RTPParser.h"

class Parser3640
    : public RTPParser
{
public:
    void SetAdtsHeaderParameter(int nObjectType, int nSampleRate, int nChannels, BOOL bWithAdtsHeader);
protected:
    virtual int ProcessMarker(int nMarkerPos);
public:
    Parser3640();
    virtual ~Parser3640();
private:
    int         m_nObjType;
    int         m_nSampleRate;
    int         m_nChannel;
    BOOL        m_bWithAdtsHeader;
};

#endif //_PARSER3640_H_
