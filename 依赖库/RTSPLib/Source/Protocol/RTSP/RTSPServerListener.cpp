#include "RTSPServerListener.h"

RTSPServerListener::RTSPServerListener()
{
//    m_bFirstCommand = TRUE;
}

RTSPServerListener::~RTSPServerListener()
{

}

BOOL RTSPServerListener::OnFirstCommand(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl,BOOL *bOutNumber)
{
    return FALSE;
}

BOOL RTSPServerListener::OnNotifyOption(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl)
{
    //return 
    //if (m_bFirstCommand)
    //{
    //    //m_bFirstCommand = FALSE;
    //    OnFirstCommand(pRTSPSession, sRequestUrl);
    //    OnNotifyOption(pRTSPSession, sRequestUrl);
    //}

    return FALSE;
}

BOOL RTSPServerListener::OnNotifyDescribe(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl, CHAR *sSdp, int *nSDP)
{
    //if (m_bFirstCommand)
    //{
    //    m_bFirstCommand = FALSE;
    //    OnFirstCommand(pRTSPSession, sRequestUrl);
    //    OnNotifyDescribe(pRTSPSession, sRequestUrl,sSdp, nSDP);
    //}

    //*nSDP = 0;
    return FALSE;
}

BOOL RTSPServerListener::OnNotifySetup(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl, int nStreamNo, int *nSendType, ZDataPin **ppDataPin)
{
    return FALSE;
}

BOOL RTSPServerListener::OnNotifyPlay(ZRTSPSession *pRTSPSession, int nStartSecond)
{
    return FALSE;
}

BOOL RTSPServerListener::OnNotifyPause(ZRTSPSession *pRTSPSession, int *nCurSecond)
{
    return FALSE;
}

BOOL RTSPServerListener::OnNotifyTearDown(ZRTSPSession *pRTSPSession)
{
    //m_bFirstCommand = TRUE;

    return FALSE;
}

BOOL RTSPServerListener::OnNotifySessionTimeout(ZRTSPSession *pRTSPSession)
{
   // m_bFirstCommand = TRUE;

    return FALSE;
}
//
//BOOL RTSPServerListener::IsFirstCommand()
//{
//    return m_bFirstCommand;
//}
//
//void RTSPServerListener::SetFirstCommand(BOOL bFirstCommand)
//{
//    m_bFirstCommand = bFirstCommand;
//}
