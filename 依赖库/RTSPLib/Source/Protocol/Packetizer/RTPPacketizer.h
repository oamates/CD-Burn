#ifndef _RTPPACKETIZER_H_
#define _RTPPACKETIZER_H_

#include "Common.h"
#include "ZDataPin.h"

class RTPPacketizer
    : public ZDataInOut
{
public:
    void SetPacketizerFlag(int nFlag);
    int GetPacketizerFlag();
    void SetPayloadType(int nPayloadType);
    int GetPayloadType();
    void SetSSRC(UINT nSSRC);
    UINT GetSSRC();
    void SetFrameTimestamp(int nTimeStamp);
    UINT GetFrameTimestamp();
    int GetSeqNo();
    void IncreaseSeqNo();
    int ReceiveFrameData(int nFlag, void *pData, int nData);
    void SetPatchFrame(BOOL bPatchFrame);
    BOOL IsPatchFrame();
    void SetRTPPacketMaxSize(int nPacketSize);
    int GetRTPPacketMaxSize();
//public:
//    virtual	BOOL OnCommand(int nCommand);
//    virtual	BOOL OnHeader(int nFlag,void* pHeader,int nHeader);
//    virtual	BOOL OnData(int nFlag,void* pData,int nData);
public:
    virtual int ProcessFrame(char *pData, int nData);
public:
    RTPPacketizer();
    virtual ~RTPPacketizer();
private:
    // 101-h264
    // 102-aac
    // 103-g711
    int         m_nPacketizerFlag;
    int         m_nPayloadType;
    UINT        m_nSSRC;
    UINT        m_nCurTimeStamp;
    int         m_nSeqNo;

    BOOL        m_bPatchFrame;
    int         m_nRTPPacketMaxSize;
};

#endif //_RTPPACKETIZER_H_
