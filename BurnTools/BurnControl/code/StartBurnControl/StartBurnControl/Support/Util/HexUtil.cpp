#include "HexUtil.h"
#include <string.h>

HexUtil::HexUtil()
: m_sHexStream(NULL)
, m_nHexStreamSize(0)
, m_pByteStream(NULL)
, m_nByteStreamSize(0)
, m_nByteStreamCount(0)
{
    InitByteStreamMem();
    InitHexStreamMem();
}

HexUtil::~HexUtil()
{
    UnitHexStreamMem();
    UnitByteStreamMem();
}

bool HexUtil::HexStreamToByteStream(char *sHexStream)
{
    PrepareHexStreamMem(strlen(sHexStream));
    if (m_sHexStream != NULL)
    {
        memset(m_sHexStream, 0, m_nHexStreamSize);
        for (int i = 0, j = 0; i < (int)strlen(sHexStream); i ++)
        {
            if (sHexStream[i] != ' ')
            {
                m_sHexStream[j] = sHexStream[i];
                j ++;
            }
        }

        m_nByteStreamCount = 0;
        if (strlen(m_sHexStream)%2 == 0)
        {
            int nNeedCount = strlen(m_sHexStream)/2;
            PrepareByteStreamMem(nNeedCount);

            if (m_pByteStream != NULL)
            {
                for (int i = 0; i < nNeedCount; i ++)
                {
                    m_pByteStream[i] = HexCharToByte(m_sHexStream[i*2], m_sHexStream[i*2+1]);
                }
                m_nByteStreamCount = nNeedCount;

                return true;
            }
        }
    }

    return false;
}

unsigned char *HexUtil::GetByteStream()
{
    return m_pByteStream;
}

int HexUtil::GetByteStreamCount()
{
    return m_nByteStreamCount;
}

void HexUtil::InitByteStreamMem()
{
    m_pByteStream = new unsigned char[1024];
    if (m_pByteStream != NULL)
    {
        m_nByteStreamSize = 1024;
    }
}

void HexUtil::UnitByteStreamMem()
{
    if (m_pByteStream != NULL)
    {
        delete m_pByteStream;
        m_pByteStream = NULL;
    }
    m_nByteStreamSize = 0;
}

void HexUtil::PrepareByteStreamMem(int nNeedCount)
{
    if (nNeedCount > m_nByteStreamSize)
    {
        UnitByteStreamMem();
        m_pByteStream = new unsigned char(nNeedCount);
        if (m_pByteStream != NULL)
        {
            m_nByteStreamSize = nNeedCount;
        }
    }
}

void HexUtil::InitHexStreamMem()
{
    m_sHexStream = new char[1024];
    if (m_sHexStream != NULL)
    {
        memset(m_sHexStream, 0, 1024);
        m_nHexStreamSize = 1024;
    }
}

void HexUtil::UnitHexStreamMem()
{
    if (m_sHexStream != NULL)
    {
        delete m_sHexStream;
        m_sHexStream = NULL;
    }
    m_nHexStreamSize = 0;
}

void HexUtil::PrepareHexStreamMem(int nNeedCount)
{
    if (nNeedCount > m_nHexStreamSize)
    {
        UnitHexStreamMem();
        m_sHexStream = new char [nNeedCount];
        if (m_sHexStream != NULL)
        {
            memset(m_sHexStream, 0, nNeedCount);
            m_nHexStreamSize = nNeedCount;
        }
    }
}

unsigned char HexUtil::CharToByte(char cInput)
{
    if (cInput >= 'a' && cInput <= 'f')
    {
        cInput = cInput - 'a' + 10;
    }
    else if (cInput >= 'A' && cInput <= 'F')
    {
        cInput = cInput - 'A' + 10;
    }
    else if (cInput >= '0' && cInput <= '9')
    {
        cInput = cInput - '0';
    }
    else
    {
        //error
        cInput = 0;
    }

    return cInput;
}

unsigned char HexUtil::HexCharToByte(char cFirst, char cSecond)
{
    return (CharToByte(cFirst)<<4)|CharToByte(cSecond);
}

void HexUtil::ByteToHexChar(unsigned char cInput, char &cFirst, char &cSecond)
{
    cFirst = (cInput&0xF0)>>4;
    if (cFirst >= 0 && cFirst <= 9)
    {
        cFirst = cFirst + '0';
    }
    else if (cFirst >= 10 && cFirst <= 15)
    {
        cFirst = (cFirst - 10) + 'a';
    }

    cSecond = (cInput&0x0F);
    if (cSecond >= 0 && cSecond <= 9)
    {
        cSecond = cSecond + '0';
    }
    else if (cSecond >= 10 && cSecond <= 15)
    {
        cSecond = (cSecond - 10) + 'a';
    }
}

void HexUtil::ByteToHexCharUpper(unsigned char cInput, char &cFirst, char &cSecond)
{
    cFirst = (cInput&0xF0)>>4;
    if (cFirst >= 0 && cFirst <= 9)
    {
        cFirst = cFirst + '0';
    }
    else if (cFirst >= 10 && cFirst <= 15)
    {
        cFirst = (cFirst - 10) + 'A';
    }

    cSecond = (cInput&0x0F);
    if (cSecond >= 0 && cSecond <= 9)
    {
        cSecond = cSecond + '0';
    }
    else if (cSecond >= 10 && cSecond <= 15)
    {
        cSecond = (cSecond - 10) + 'A';
    }
}
