#ifndef _RTSPSERVERLISTENER_H_
#define _RTSPSERVERLISTENER_H_

#include "Common.h"

class ZDataPin;
class ZRTSPSession;

class RTSPServerListener
{
public:
    virtual BOOL OnFirstCommand(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl, BOOL *bOutNumber);

    virtual BOOL OnNotifyOption(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl);
    // sSdp[out]
    // nSdp[out]
    virtual BOOL OnNotifyDescribe(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl, CHAR *sSdp, int *nSDP);
    
    // nStreanOn[in]
    // nSendType[out]
    // 0 - unicast
    // 1 - multicast
    // ppDataPin[out]
    virtual BOOL OnNotifySetup(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl, int nStreamNo, int *nSendType, ZDataPin **ppDataPin);
    
    // vod. listener should seek the file to nPlayStartSecond.
    //nStartSecond[in]
    virtual BOOL OnNotifyPlay(ZRTSPSession *pRTSPSession, int nStartSecond);

    //
    //nCurSecond[out]
    virtual BOOL OnNotifyPause(ZRTSPSession *pRTSPSession, int *nCurSecond);

    virtual BOOL OnNotifyTearDown(ZRTSPSession *pRTSPSession);

    virtual BOOL OnNotifySessionTimeout(ZRTSPSession *pRTSPSession);
public:
    RTSPServerListener();
    virtual ~RTSPServerListener();
};

#endif //_RTSPSERVERLISTENER_H_
