#ifndef _RTSPSERVERSOURCE_H_
#define _RTSPSERVERSOURCE_H_

#include "ZOSArray.h"
#include "ZRTSPSession.h"
#include "ChnsysRTSP.h"
#include "RTPPacketizer.h"
#include "RTSPServerListener.h"

class MulticastPin;
class RemoteRTSPClient;

typedef ZOSArray<ZRTSPSession*>     RTSPSessionArray;
typedef ZOSArray<RTPPacketizer*>    RTPPacketizerArray;

class RTSPServerSource
    : public RTSPServerListener
    , public ZTask
{
//////////////////////////////////////////////////////////////////////////
public:
    void SetConfig(RTSP_SERVER_CONFIG config);
    int GetConfig(RTSP_SERVER_CONFIG *pConfig);
    int ServerStart();
    int ServerStop();
    int PushH264Data(UINT nTimestamp, char *sData, int nData);
    int PushAACData(UINT nTimestamp, char *sData, int nData);
    int PushG711Data(UINT nTimestamp, char *sData, int nData);
    void SetStatusCallBack(RTSP_SERVER_StatusCallBackFunc scbf, void *pContext);
    int GetClientCount();
    int GetClientProperty(int nIndex, RTSP_SERVER_CLIENTPROPERTY *pProperty);
    int DisconnectClient(RTSP_SERVER_CLIENTPROPERTY clientProperty);
	int DisconnectAllClient();
	int SetMaxClientCount(int nCount);
	int GetMaxClientCount();
    int ResetSDPByAudio(RTSP_SERVER_CONFIG config);
    void SetSDP(char *sSDP, int nSDP,char *sMultiAddr,int nMultiPort);
    int PushRTPData(int nStreamNo, char *sRTPData, int nRTPSize);
    int SetEachTimeSendCount(int nSendCount);
    void SetPatchAudioFrame(BOOL bPatch);
    int SetRTPPacketMaxSize(int nRTPPacketMaxSize);
    int GetRTPPacketMaxSize();
    int SetRTSPStreamMode(int nRTSPStreamMode);
    int GetRTSPStreamMode();
//////////////////////////////////////////////////////////////////////////
public:
    virtual	int Run(int nEvent = 0);
//////////////////////////////////////////////////////////////////////////
public:
    virtual BOOL OnNotifyOption(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl);
    virtual BOOL OnNotifyDescribe(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl, CHAR *sSdp, int *nSDP);
    virtual BOOL OnNotifySetup(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl, int nStreamNo, int *nSendType, ZDataPin **ppDataPin);
    virtual BOOL OnNotifyPlay(ZRTSPSession *pRTSPSession, int nStartSecond);
    virtual BOOL OnNotifyPause(ZRTSPSession *pRTSPSession, int *nCurSecond);
    virtual BOOL OnNotifyTearDown(ZRTSPSession *pRTSPSession);
    virtual BOOL OnNotifySessionTimeout(ZRTSPSession *pRTSPSession);
//////////////////////////////////////////////////////////////////////////
public:
    RemoteRTSPClient *GetRemoteRTSPClient(ZRTSPSession *pRTSPSession);
    int AddRemoteRTSPClient(RemoteRTSPClient *pRemoteRTSPClient);
protected:
    BOOL CreatePacketizer();
    BOOL HaveAACPacketizer();
    BOOL HaveG711Packetizer();
    BOOL HaveH264Packetizer();
    BOOL CreateSDP();
    int CheckClientSession();
public:
    RTSPServerSource();
    virtual ~RTSPServerSource();
private:
    RTPPacketizerArray      m_arrayRTPPacketizer;
    ZOSMutex                m_mutexRTPPacketizerArray;

    BOOL                    m_bIsMulticast;
    MulticastPin            *m_pMulticastPin;
    char                    *m_sSDP;

    UINT64                  m_nLastCheckSdpChangeTime;
    char                    m_sBackupSPS[8192];
    int                     m_nBackupSPS;
    char                    m_sBackupPPS[8192];
    int                     m_nBackupPPS;

    int                     m_nAACPayload;
    RTPPacketizer           *m_pAACPacketizer;
    RTPPacketizer           *m_pG711Packetizer;
    int                     m_nH264Payload;
    RTPPacketizer           *m_pH264Packetizer;

    ZOSArray<RemoteRTSPClient*>     m_arrayRemoteClient;
    ZOSMutex                        m_mutexRemoteClientArray;
	int                             m_nMaxRemoteClient;
//////////////////////////////////////////////////////////////////////////
private:
    RTSP_SERVER_CONFIG      m_config;
    RTSP_SERVER_StatusCallBackFunc      m_StatusCallBackFunc;
    void                                *m_pscbfContext;
    BOOL                    m_bServerStart;
    int                     m_nEachTimeSendCount;
    BOOL                    m_bPatchAudioFrame;
    int                     m_nRTPPacketMaxSize;
    // 0 - vid & aud
    // 1 - vid
    // 2 - aud
    int                     m_nRTSPStreamMode;
};

#endif //_RTSPSERVERSOURCE_H_
