#ifndef _ZBITSTREAM_H_
#define _ZBITSTREAM_H_

#include "public.h"

class ZBitStream
{
    ///////////////////////////////////////////////////////////////////////////////
public:
    ZBitStream();
    ZBitStream(CONST BYTE* pBuffer,UINT nBuffer);
    virtual	~ZBitStream();
    ///////////////////////////////////////////////////////////////////////////////
public:
    void		Init(CONST BYTE* pBuffer,UINT nBuffer);
    ///////////////////////////////////////////////////////////////////////////////
public:
    BYTE*		StreamBuffer();
    UINT32		StreamLength();
    UINT32		StreamOffset();
    ///////////////////////////////////////////////////////////////////////////////
public:
    UINT32		GetBits(UINT nBits);
    UINT32		PutBits(UINT nBits,UINT32 nData);
    UINT32		PeekBits(UINT nBits);
    ///////////////////////////////////////////////////////////////////////////////
public:
    UINT32		GetBuffer(UINT nBits,BYTE* pBuffer = NULL);
    UINT32		PutBuffer(UINT nBits,BYTE* pBuffer);
    ///////////////////////////////////////////////////////////////////////////////
public:
    UINT32		GetDataBits();
    UINT32		GetStreamBits();
    UINT32		GetRemainBits();
    UINT32		ByteAlign(BOOL bWrite = TRUE);
    ///////////////////////////////////////////////////////////////////////////////
private:
    BYTE*		m_pStreamBuffer;
    UINT32		m_nStreamBuffer;
    BYTE*		m_pStreamBeginBuffer;
    BYTE*		m_pStreamTailBuffer;
    UINT32		m_nStreamBits;
    BYTE		m_cDataByte;
    UINT32		m_nDataBits;
    ///////////////////////////////////////////////////////////////////////////////
};

#endif //_ZBITSTREAM_H_
