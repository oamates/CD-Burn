#ifndef _RTPPARSER_H_
#define _RTPPARSER_H_

#include "Common.h"
#include "ZOSPool.h"
#include "ZDataPin.h"
#include "ZTask.h"
#include "ZRTPPacket.h"
#include "ZOSArray.h"

#define AACFRAME_MAX_SIZE   (2048)
#define H264NAL_MAX_SIZE    (1920*540)
#define G711FRAME_MAX_SIZE  (1024*10)

class RTPParserListener
{
public:
    virtual BOOL ParserOnData(int nFlag, void*pData, int nData, 
        long nTimestampSecond, long nTimestampUSecond);
};

class RTPParser
    : public ZDataInOut
    , public ZTask
{
public:
    void SetPoolCount(int nCount);
    void SetSpliceThreshold(int nCount);
    int GetSpliceThreshold();
    void SetParserFlag(int nFlag);
    int GetParserFlag();
    BOOL SetUpperPin(ZDataOut *pDataUpper);
    UINT64 GetTotalPacketCount();
    UINT64 GetLostPacketCount();
    void SetParserListener(RTPParserListener *pListener);
    BOOL Start();
    BOOL Stop();

	UINT GetBitRate();
	UINT GetFps();
	VOID SetStreamType(BOOL bType);
public:
    // override ZDataInOut function
    virtual	BOOL OnCommand(int nCommand);
    virtual	BOOL OnHeader(int nFlag,void* pHeader,int nHeader);
    virtual	BOOL OnData(int nFlag,void* pData,int nData);
    virtual BOOL OnUserData(int nFlag,void* pData);
public:
    // override ZTask function
    virtual	int Run(int nEvent = 0);
protected:
    ZRTPPacket* ConstructPacket(char *pData, int nData);
    BOOL AddPacketToArray(ZRTPPacket *pRTPPacket);
    BOOL ReceivePacket(char *pData, int nData);

    int ProcessPacket();
    BOOL ProcessSplice();
    int ProcessMissingPacket();

    BOOL ReceiveTimeout(int nMarkerPos, int nEndPos);
    //in order and strictly plus one
    virtual BOOL SequenceStrictlyInOrder(int nMarkerPos);
    virtual int ProcessRawRTP(int nMarkerPos);
    virtual int ProcessMarker(int nMarkerPos);

    int GetFrameTimestamp(int nFrequence, UINT nCurTimestamp, long *nNTPSecond, long *nNTPUSecond);
public:
    RTPParser();
    virtual ~RTPParser();
protected:
    ZOSMutex                m_mutexRTPPacketPool;
    ZOSPool<ZRTPPacket>     m_poolRTPPacket;
    ZOSMutex                m_mutexRTPPacketArray;
    ZOSArray<ZRTPPacket *>  m_arrayRTPPacket;

    int                     m_nLastPacketSeq;

    // less than this value will not do splice process.
    // will be usefull in audio rtp packet when marker bit is true,
    // but seq no is not in order.
    int                     m_nSpliceThreshold;
	WORD                    m_nNextExpectedSeqNo;
	BOOL                    m_bHaveSeenFirstPacket;
	int                     m_nParserFlag;

    UINT64                  m_nTotalPacketCount;
    UINT64                  m_nLostPacketCount;

    CHAR                    *m_pFrameBuffer;
    UINT                    m_nCurFrameTimeStamp;

    UINT                    m_nSyncTimestamp;
    long                    m_nSyncNTPSecond;
    long                    m_nSyncNTPUSecond;

    RTPParserListener       *m_pParserListener;

	UINT64                  m_tBitRateTime;
	UINT                    m_nBitRate;
	UINT                    m_nStreamBytes;

	UINT64                  m_tFpsTime;
	UINT                    m_nFps;
	UINT                    m_nFrames;

	BOOL                    m_bIsVideo;
};

#endif //_RTPPARSER_H_
