#ifndef _RTSPCLIENTSOURCE_H_
#define _RTSPCLIENTSOURCE_H_

#include "RTSPClientListener.h"
#include "ZOSArray.h"
#include "ZDataPin.h"
#include "ZRTSPSession.h"
#include "RTPParser.h"
#include "ChnsysRTSP.h"
#include "ZIdleTask.h"
#include "ZTCPSocket.h"

class RTPParser;

typedef ZOSArray<RTPParser *>    ParserArray;

class RTSPClientSource
    : public RTSPClientListener
    , public RTPParserListener
    , public ZIdleTask
{
//////////////////////////////////////////////////////////////////////////
public:
    // interface function
    void SetStatusCallBack(RTSP_CLIENT_StatusCallBackFunc scbf, void* pContext);
    void SetRtpDataCallBack(RTSP_CLIENT_RtpCallBackFunc rtpcbf, void* pContext);
    void SetRawDataCallBack(RTSP_CLIENT_RawDataCallBackFunc rdcbf, VOID* pContext);
    void GetStatistics(int nStreamNo, RTSP_CLIENT_STATISTICSDATA *pStatistics);
    char* GetSDP();
    int GetStreamCount();
    int GetStreamType(int nStreamNo);
    int GetVideoProperty(int nStreamNo, RTSP_RTSP_VIDEOPROPERTY *pVideoProperty);
    int GetAudioProperty(int nStreamNo, RTSP_RTSP_AUDIOPROPERTY *pAudioProperty);
    char *GetMediaFromSDP(int nStreamNo, char *sMedia, int *pMediaSize);
    void GetConfig(RTSP_CLIENT_CONFIG *pConfig);
    int SetConfig(RTSP_CLIENT_CONFIG config);
    int RTSPPlay();
    int RTSPPause();
    int RTSPSeek();
    int RTSPStop();
//////////////////////////////////////////////////////////////////////////
protected:
    // override RTPParserListener function
    virtual BOOL ParserOnData(int nFlag, void*pData, int nData, long nTimestampSecond, long nTimestampUSecond);
protected:
    // override ZIdleTask function
    virtual	int Run(int nEvent = 0);
protected:
    // override RTSPClientListener function
    BOOL OnReceiveResponse_OPTION(int nState);
    BOOL OnReceiveResponse_DESCRIBE(int nState, CHAR *sSDP, int nSDP);
    BOOL OnReceiveResponse_SETUP(int nState, int nStreamNo);
    BOOL OnReceiveResponse_PLAY(int nState);

    BOOL DisconnectWithServer();
    BOOL BeforeReconnect();
    BOOL AfterReconnect(BOOL bReconnectOK);
    BOOL GiveupReconnect();

    BOOL DetectPacketLost(int nLostCount);
protected:
    // inner process functions
    virtual	int	OnUpdate();
    virtual	int	OnIdle();
    int DoPlay();
    int DoFirstPlay();
    int DoStop();
    BOOL DoReconnect();
    BOOL CreateParser(CHAR *sSDP, int nSDP);
    BOOL DestroyParser();
public:
    RTSPClientSource();
    virtual ~RTSPClientSource();
private:
    ParserArray         m_arrayParser;

    ZTCPSocket          *m_pSocket;
    ZRTSPSession        *m_pRTSPSession;

    BOOL			    m_bSessionTimeout;
    int                 m_nAlreadyReconnectTimes;

    UINT64              m_nSessionStartTime;
    UINT64              m_nSessionStopTime;

    BOOL                m_bIsPlaying;
    BOOL                m_bFirstPlaySuccess;

    //many place can use m_pRTSPSession and m_pSocket.
    //1 DoPlay
    //2 DoReconnect
    //3 DoFirstPlay
    //4 DoStop
    //can only be one place invoked or 
    ZOSMutex            m_mutexPlay;
//////////////////////////////////////////////////////////////////////////
private:
    RTSP_CLIENT_StatusCallBackFunc      m_StatusCallBackFunc;
    VOID                                *m_pscbfContext;
    RTSP_CLIENT_RtpCallBackFunc         m_RtpCallBackFunc;
    VOID                                *m_prcbfContext;
    RTSP_CLIENT_RawDataCallBackFunc     m_RawDataCallBackFunc;
    VOID                                *m_prdcbfContext;

    RTSP_CLIENT_CONFIG                  m_Config;
//////////////////////////////////////////////////////////////////////////
};

#endif //_RTSPCLIENTSOURCE_H_
