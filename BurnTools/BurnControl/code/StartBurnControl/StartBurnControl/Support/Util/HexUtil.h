#ifndef _HEX_UTIL_H_
#define _HEX_UTIL_H_



class HexUtil
{
public:
    /**
     *
     * 
     * @sHexStream
     *  "af1991FA"
     *  or "af 19 91 FA"
     * @
     */
    bool HexStreamToByteStream(char *sHexStream);
    unsigned char *GetByteStream();
    int GetByteStreamCount();
protected:
    void InitByteStreamMem();
    void UnitByteStreamMem();
    void PrepareByteStreamMem(int nNeedCount);

    void InitHexStreamMem();
    void UnitHexStreamMem();
    void PrepareHexStreamMem(int nNeedCount);
public:
    static unsigned char CharToByte(char cInput);
    static unsigned char HexCharToByte(char cFirst, char cSecond);
    static void ByteToHexChar(unsigned char cInput, char &cFirst, char &cSecond);
    static void ByteToHexCharUpper(unsigned char cInput, char &cFirst, char &cSecond);
public:
    HexUtil();
    ~HexUtil();
private:
    char            *m_sHexStream;
    int             m_nHexStreamSize;

    unsigned char   *m_pByteStream;
    int             m_nByteStreamSize;
    int             m_nByteStreamCount;
};


#endif//_HEX_UTIL_H_
