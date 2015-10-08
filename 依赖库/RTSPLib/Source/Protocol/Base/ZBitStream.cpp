#include "ZBitStream.h"
#include "Common.h"

///////////////////////////////////////////////////////////////////////////////
ZBitStream::ZBitStream()
:m_pStreamBuffer(NULL)
,m_nStreamBuffer(0)
,m_pStreamBeginBuffer(NULL)
,m_pStreamTailBuffer(NULL)
,m_nStreamBits(0)
,m_cDataByte(0)
,m_nDataBits(0)
{
}
ZBitStream::ZBitStream(CONST BYTE* pBuffer,UINT nBuffer)
:m_pStreamBuffer(NULL)
,m_nStreamBuffer(0)
,m_pStreamBeginBuffer(NULL)
,m_pStreamTailBuffer(NULL)
,m_nStreamBits(0)
,m_cDataByte(0)
,m_nDataBits(0)
{
    Init(pBuffer,nBuffer);
}
ZBitStream::~ZBitStream()
{
}
///////////////////////////////////////////////////////////////////////////////
void ZBitStream::Init(CONST BYTE* pBuffer,UINT nBuffer)
{
    m_pStreamBuffer	= (BYTE*)pBuffer;
    m_nStreamBuffer	= (nBuffer<<3);
    m_pStreamBeginBuffer	= (BYTE*)pBuffer;
    m_pStreamTailBuffer		= (m_pStreamBeginBuffer + nBuffer);
    m_nStreamBits	= 0;
    m_cDataByte		= 0;
    m_nDataBits		= 0;
}
BYTE* ZBitStream::StreamBuffer()
{
    return(m_pStreamBuffer);
}
UINT32 ZBitStream::StreamLength()
{
    return(m_nStreamBuffer<<3);
}
UINT32 ZBitStream::StreamOffset()
{
    return(m_nStreamBits<<3);
}
///////////////////////////////////////////////////////////////////////////////
UINT32 ZBitStream::GetBits(UINT nBits)
{
    STATIC	CONST	UINT32	MASK[33]	= 
    {
        0x00000000, 0x00000001, 0x00000003, 0x00000007,
        0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F,
        0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF,
        0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF,
        0x0000FFFF, 0x0001FFFF, 0x0003FFFF, 0x0007FFFF,
        0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF,
        0x00FFFFFF, 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF,
        0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF,
        0xFFFFFFFF
    };
    UINT32	nReturn	= 0;

    TMASSERT((nBits <= 32));
    if(nBits <= 32)
    {
        if(nBits > 0)
        {
            if(m_nDataBits >= nBits)
            {
                m_nDataBits		-= nBits;
                m_nStreamBits	+= nBits;
                nReturn			= (m_cDataByte >> m_nDataBits);
            }else{
                UINT	nAskBits	= (nBits - m_nDataBits);
                if(nAskBits == 32)
                {
                    nReturn			= 0;
                }else{
                    nReturn			= (m_cDataByte << nAskBits);
                }
                switch(((nAskBits - 1) / 8))
                {
                case 3:
                    nAskBits		-= 8;
                    if(m_nStreamBuffer >= 8)
                    {
                        nReturn			|= ((*(m_pStreamBuffer ++)) << nAskBits);
                        m_nStreamBuffer	-= 8;
                    }
                case 2:
                    nAskBits		-= 8;
                    if(m_nStreamBuffer >= 8)
                    {
                        nReturn			|= ((*(m_pStreamBuffer ++)) << nAskBits);
                        m_nStreamBuffer	-= 8;
                    }
                case 1:
                    nAskBits		-= 8;
                    if(m_nStreamBuffer >= 8)
                    {
                        nReturn			|= ((*(m_pStreamBuffer ++)) << nAskBits);
                        m_nStreamBuffer	-= 8;
                    }
                case 0:
                    break;
                }
                if(m_nStreamBuffer >= nAskBits)
                {
                    m_cDataByte		= (*(m_pStreamBuffer ++));
                    m_nDataBits		= (MIN(8,m_nStreamBuffer) - nAskBits);
                    m_nStreamBuffer	-= MIN(8,m_nStreamBuffer);
                    nReturn			|= ((m_cDataByte >> m_nDataBits) & MASK[nAskBits]);
                }
                m_nStreamBits	+= nBits;
            }
        }
        TMASSERT((m_pStreamBuffer<=m_pStreamTailBuffer));
        TMASSERT((m_nStreamBuffer>=0));
        return (nReturn&MASK[nBits]);
    }
    return 0;
}
UINT32 ZBitStream::PutBits(UINT nBits,UINT32 nData)
{
    STATIC	CONST	UINT32	MASK[33]	= 
    {
        0x00000000, 0x00000001, 0x00000003, 0x00000007,
        0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F,
        0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF,
        0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF,
        0x0000FFFF, 0x0001FFFF, 0x0003FFFF, 0x0007FFFF,
        0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF,
        0x00FFFFFF, 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF,
        0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF,
        0xFFFFFFFF
    };
    UINT32	nReturn	= 0;

    TMASSERT((nBits <= 32));
    if(nBits <= 32)
    {
        if(nBits > 0)
        {
            if(m_nStreamBits == 0)
            {
                m_nDataBits		= 8;
            }
            UINT	nAskBits	= nBits;
            UINT	nAskData	= 0;
            UINT	nUsedBits	= 0;

            if(m_nDataBits > nBits)
            {
                nUsedBits		= nAskBits;
            }else{
                nUsedBits		= m_nDataBits;
            }

            m_nDataBits		-= nUsedBits;
            nAskBits		-= nUsedBits;
            nAskData		= (nData >> nAskBits);
            m_cDataByte		|= ((nAskData & MASK[nUsedBits]) << m_nDataBits);
            if(m_nDataBits == 0)
            {
                switch(((nAskBits) / 8))
                {
                case 4:
                    {
                        if(m_nStreamBuffer >= 8)
                        {
                            *m_pStreamBuffer	= m_cDataByte;
                            m_pStreamBuffer		++;
                            m_nDataBits			= 8;
                            m_nStreamBuffer		-= 8;
                        }
                        nUsedBits			= 8;
                        m_nDataBits			-= nUsedBits;
                        nAskBits			-= nUsedBits;
                        nAskData			= (nData >> nAskBits);
                        m_cDataByte			= (nAskData & MASK[nUsedBits]);
                    }
                case 3:
                    {
                        if(m_nStreamBuffer >= 8)
                        {
                            *m_pStreamBuffer	= m_cDataByte;
                            m_pStreamBuffer		++;
                            m_nDataBits			= 8;
                            m_nStreamBuffer		-= 8;
                        }
                        nUsedBits			= 8;
                        m_nDataBits			-= nUsedBits;
                        nAskBits			-= nUsedBits;
                        nAskData			= (nData >> nAskBits);
                        m_cDataByte			= (nAskData & MASK[nUsedBits]);
                    }
                case 2:
                    {
                        if(m_nStreamBuffer >= 8)
                        {
                            *m_pStreamBuffer	= m_cDataByte;
                            m_pStreamBuffer		++;
                            m_nDataBits			= 8;
                            m_nStreamBuffer		-= 8;
                        }
                        nUsedBits			= 8;
                        m_nDataBits			-= nUsedBits;
                        nAskBits			-= nUsedBits;
                        nAskData			= (nData >> nAskBits);
                        m_cDataByte			= (nAskData & MASK[nUsedBits]);
                    }
                case 1:
                    {
                        if(m_nStreamBuffer >= 8)
                        {
                            *m_pStreamBuffer	= m_cDataByte;
                            m_pStreamBuffer		++;
                            m_nDataBits			= 8;
                            m_nStreamBuffer		-= 8;
                        }
                        nUsedBits			= 8;
                        m_nDataBits			-= nUsedBits;
                        nAskBits			-= nUsedBits;
                        nAskData			= (nData >> nAskBits);
                        m_cDataByte			= (nAskData & MASK[nUsedBits]);
                    }
                case 0:
                    {
                        if(m_nStreamBuffer >= 8)
                        {
                            *m_pStreamBuffer	= m_cDataByte;
                            m_pStreamBuffer		++;
                            m_nDataBits			= 8;
                            m_nStreamBuffer		-= 8;
                        }
                        nUsedBits			= nAskBits;
                        m_nDataBits			-= nUsedBits;
                        nAskBits			-= nUsedBits;
                        nAskData			= (nData >> nAskBits);
                        m_cDataByte			= ((nAskData & MASK[nUsedBits]) << m_nDataBits);
                    }
                }
            }
            *m_pStreamBuffer	= m_cDataByte;
            m_nStreamBits		+= nBits;
        }
        return nReturn;
    }

    return 0;
}
UINT32 ZBitStream::PeekBits(UINT nBits)
{
    UINT32	nReturn			= 0;
    BYTE*	pStreamBuffer	= m_pStreamBuffer;
    UINT	nStreamBuffer	= m_nStreamBuffer;
    BYTE	cDataByte		= m_cDataByte;
    UINT	nDataBits		= m_nDataBits;
    UINT	nStreamBits		= m_nStreamBits;

    nReturn	= GetBits(nBits);

    m_pStreamBuffer		= pStreamBuffer;
    m_nStreamBuffer		= nStreamBuffer;
    m_cDataByte			= cDataByte;
    m_nDataBits			= nDataBits;
    m_nStreamBits		= nStreamBits;

    return nReturn;
}
///////////////////////////////////////////////////////////////////////////////
UINT32 ZBitStream::GetBuffer(UINT nBits,BYTE* pBuffer)
{
    UINT	nTheByte	= (nBits >> 3);
    UINT	nTheBits	= (nBits & 0x07);
    UINT	nTheIndex	= 0;

    for(nTheIndex = 0; nTheIndex < nTheByte; nTheIndex ++)
    {
        if(pBuffer != NULL)
        {
            pBuffer[nTheIndex]	= ZBitStream::GetBits(8);
        }else{
            ZBitStream::GetBits(8);
        }
    }
    if(nTheBits > 0)
    {
        if(pBuffer != NULL)
        {
            pBuffer[nTheIndex]	= (ZBitStream::GetBits(nTheBits) << (8 - nTheBits));
        }else{
            ZBitStream::GetBits(nTheBits);
        }
    }

    return (nTheByte * 8 + nTheBits);
}
UINT32 ZBitStream::PutBuffer(UINT nBits,BYTE* pBuffer)
{
    UINT	nTheByte	= (nBits >> 3);
    UINT	nTheBits	= (nBits & 0x07);
    UINT	nTheIndex	= 0;

    for(nTheIndex = 0; nTheIndex < nTheByte; nTheIndex ++)
    {
        if(pBuffer != NULL)
        {
            ZBitStream::PutBits(8,pBuffer[nTheIndex]);
        }else{
            ZBitStream::PutBits(8,0);
        }
    }
    if(nTheBits > 0)
    {
        if(pBuffer != NULL)
        {
            ZBitStream::PutBits(nTheBits,(pBuffer[nTheIndex] >> (8 - nTheBits)));
        }else{
            ZBitStream::PutBits(nTheBits,0);
        }
    }

    return (nTheByte * 8 + nTheBits);
}
///////////////////////////////////////////////////////////////////////////////
UINT32 ZBitStream::GetDataBits()
{
    return (m_nDataBits);
}
UINT32 ZBitStream::GetStreamBits()
{
    return (m_nStreamBits);
}
UINT32 ZBitStream::GetRemainBits()
{
    return (m_nDataBits+m_nStreamBuffer);
}
UINT32 ZBitStream::ByteAlign(BOOL bWrite)
{
    UINT32	nLeftBits	= m_nDataBits;

    if(nLeftBits != 0)
    {
        if(bWrite)
        {
            ZBitStream::PutBits(nLeftBits,0x00);
        }else{
            ZBitStream::GetBits(nLeftBits);
        }
    }
    return nLeftBits;
}

