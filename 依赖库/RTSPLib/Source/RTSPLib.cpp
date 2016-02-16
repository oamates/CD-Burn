#include "RTSPLib.h"
#include "RTSPClientSource.h"
#include "RTSPServerSource.h"
#include "RTSPServerListenerManager.h"
#include "ZRTPSocketPool.h"
#include "ZTask.h"
#if defined(_LINUX_)
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#endif	//defined(_LINUX_)


CHNSYS_INT RTSP_RTSP_GetConifg(RTSP_RTSP_CONFIG *pConfig)
{
    if (pConfig != NULL)
    {
        pConfig->nRecvFreq = ZRTPSession::GetRecvFreq();
        pConfig->nRecvCount = ZRTPSession::GetRecvCount();
        pConfig->nSendFreq = ZRTPSession::GetSendFreq();
        pConfig->nSendCount = ZRTPSession::GetSendCount();
        pConfig->nUDPPairCount = ZRTPSocketPool::GetPairCount();
        pConfig->nPacketInterval = ZUDPSocketPair::GetPacketInterval();
        pConfig->nFilterPacketSize = ZUDPSocketPair::GetFilterPacketSize();
    }

    return -1;
}

VOID RTSP_RTSP_SetConfig(RTSP_RTSP_CONFIG config)
{
    /*ZRTPSession::SetRecvFreq(config.nRecvFreq);
    ZRTPSession::SetRecvCount(config.nRecvCount);
    ZRTPSession::SetSendFreq(config.nSendFreq);
    ZRTPSession::SetSendCount(config.nSendCount);
    ZRTPSocketPool::SetPairCount(config.nUDPPairCount);*/
    ZUDPSocketPair::SetPacketInterval(config.nPacketInterval);
    ZUDPSocketPair::SetFilterPacketSize(config.nFilterPacketSize);
}

ZLog *g_pLog = NULL;

VOID RTSP_RTSP_Init(CONST CHNSYS_CHAR *sLogPath)
{
#ifdef _LINUX_
    struct	sigaction	act;
    struct	rlimit		rl;

    act.sa_flags	= 0;
    act.sa_handler	= SIG_IGN;

    (void)::sigaction(SIGPIPE,	&act,	NULL);
    (void)::sigaction(SIGHUP,	&act,	NULL);
    (void)::sigaction(SIGINT,	&act,	NULL);
    (void)::sigaction(SIGTERM,	&act,	NULL);
    (void)::sigaction(SIGQUIT,	&act,	NULL);
    (void)::sigaction(SIGALRM,	&act,	NULL);

    rl.rlim_cur	= 10240;
    rl.rlim_max	= 10240;
    setrlimit(RLIMIT_NOFILE,&rl);

    rl.rlim_cur	= RLIM_INFINITY;
    rl.rlim_max	= RLIM_INFINITY;
    setrlimit(RLIMIT_CORE,&rl);
#endif//_LINUX_
    g_pLog = new ZLog;
    LOG_SET_LEVEL(ZLog::LOG_DEBUG);
    LOG_SET_PATH(sLogPath);
    LOG_SET_SETSINGLELOOP(TRUE);
    LOG_SET_LOOPTHRESHOLD(512*1024);
    LOG_SET_FIXNAME("rtsplib.log");

    ZOS::Initialize();
    ZOSThread::Initialize();
    ZSocket::Initialize();
    ZIdleTask::Initialize();
    ZTaskThreadPool::Initialize(80, 1);
    ZRTCPTask::Initialize();
    ZRTPSocketPool::Initialize();
    ZSocket::StartEventThread();
    ZIdleTask::StartIdleTaskThread();
    ZTaskThreadPool::StartTaskThread();
	if(strcmp(sLogPath,"/home/RTSPLib") == 0)
	{
		RTSPServerListenerManager::Initialize();
	}
}

VOID RTSP_RTSP_Uninit()
{
    RTSPServerListenerManager::Uninitialize();
    ZSocket::StopEventThread();
    ZIdleTask::StopIdleTaskThread();
    ZTaskThreadPool::StopTaskThread();
    ZRTPSocketPool::Uninitialize();
    ZRTCPTask::Uninitialize();
    ZTaskThreadPool::Uninitialize();
    ZIdleTask::Uninitialize();
    ZSocket::Uninitialize();
    ZOSThread::Uninitialize();
    ZOS::Uninitialize();
    if (g_pLog != NULL)
    {
        SAFE_DELETE(g_pLog);
    }
}

RTSP_CLIENT_HANDLE RTSP_CLIENT_CreateInstance()
{
    RTSPClientSource *pRTSPClientSource = NEW RTSPClientSource;
    if (pRTSPClientSource != NULL)
    {
        return (RTSP_CLIENT_HANDLE)pRTSPClientSource;
    }

    return NULL;
}

VOID RTSP_CLIENT_DestroyInstance(RTSP_CLIENT_HANDLE hRTSPClient)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        SAFE_DELETE(pRTSPClientSource);
    }
}

VOID RTSP_CLIENT_SetStatusCallBack(RTSP_CLIENT_HANDLE hRTSPClient, RTSP_CLIENT_StatusCallBackFunc scbf, VOID* pContext)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        pRTSPClientSource->SetStatusCallBack(scbf, pContext);
    }
}

VOID RTSP_CLIENT_SetRtpDataCallBack(RTSP_CLIENT_HANDLE hRTSPClient, RTSP_CLIENT_RtpCallBackFunc rtpcbf, VOID* pContext)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        pRTSPClientSource->SetRtpDataCallBack(rtpcbf, pContext);
    }
}

VOID RTSP_CLIENT_SetRawDataCallBack(RTSP_CLIENT_HANDLE hRTSPClient, RTSP_CLIENT_RawDataCallBackFunc rdcbf, VOID* pContext)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        pRTSPClientSource->SetRawDataCallBack(rdcbf, pContext);
    }
}

CHNSYS_INT RTSP_CLIENT_GetStatistics(RTSP_CLIENT_HANDLE hRTSPClient, CHNSYS_INT nStreamNo, RTSP_CLIENT_STATISTICSDATA *pStatistics)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        if (pStatistics != NULL)
        {
            pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
            pRTSPClientSource->GetStatistics(nStreamNo, pStatistics);

            return 0;
        }
    }

    return -1;
}

CHNSYS_CHAR *RTSP_CLIENT_GetSDP(RTSP_CLIENT_HANDLE hRTSPClient)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        return pRTSPClientSource->GetSDP();
    }

    return NULL;
}

CHNSYS_INT RTSP_CLIENT_GetStreamCount(RTSP_CLIENT_HANDLE hRTSPClient)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        return pRTSPClientSource->GetStreamCount();
    }

    return -1;
}

CHNSYS_INT RTSP_CLIENT_GetStreamType(RTSP_CLIENT_HANDLE hRTSPClient, CHNSYS_INT nStreamNo)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        return pRTSPClientSource->GetStreamType(nStreamNo);
    }

    return -1;
}

CHNSYS_INT RTSP_CLIENT_GetVideoProperty(RTSP_CLIENT_HANDLE hRTSPClient, CHNSYS_INT nStreamNo, RTSP_RTSP_VIDEOPROPERTY *pVideoProperty)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        return pRTSPClientSource->GetVideoProperty(nStreamNo, pVideoProperty);
    }

    return -1;
}

VOID RTSP_RTSP_FreeVideoProperty(RTSP_RTSP_VIDEOPROPERTY *pVideoProperty)
{
    if (pVideoProperty != NULL)
    {
        if (pVideoProperty->pSPS != NULL)
        {
            SAFE_DELETE_ARRAY(pVideoProperty->pSPS);
            pVideoProperty->nSPS = 0;
        }
        if (pVideoProperty->pPPS != NULL)
        {
            SAFE_DELETE_ARRAY(pVideoProperty->pPPS);
            pVideoProperty->nPPS = 0;
        }
    }
}

CHNSYS_INT RTSP_CLIENT_GetAudioProperty(RTSP_CLIENT_HANDLE hRTSPClient, CHNSYS_INT nStreamNo, RTSP_RTSP_AUDIOPROPERTY *pAudioProperty)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        return pRTSPClientSource->GetAudioProperty(nStreamNo, pAudioProperty);
    }

    return -1;
}

CONST CHNSYS_CHAR *RTSP_CLIENT_GetMediaFromSDP(RTSP_CLIENT_HANDLE hRTSPClient, CHNSYS_INT nStreamNo,
                                               CHNSYS_CHAR *sMedia, CHNSYS_INT *pMediaSize)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        return pRTSPClientSource->GetMediaFromSDP(nStreamNo, sMedia, pMediaSize);
    }

    return NULL;
}

VOID RTSP_CLIENT_GetConfig(RTSP_CLIENT_HANDLE hRTSPClient, RTSP_CLIENT_CONFIG *pConfig)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        pRTSPClientSource->GetConfig(pConfig);
    }
}

CHNSYS_INT RTSP_CLIENT_SetConfig(RTSP_CLIENT_HANDLE hRTSPClient, RTSP_CLIENT_CONFIG config)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        return pRTSPClientSource->SetConfig(config);
    }

    return -1;
}

CHNSYS_INT RTSP_CLIENT_Play(RTSP_CLIENT_HANDLE hRTSPClient)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        return pRTSPClientSource->RTSPPlay();
    }

    return -1;
}

CHNSYS_INT RTSP_CLIENT_Pause(RTSP_CLIENT_HANDLE hRTSPClient)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        return pRTSPClientSource->RTSPPause();
    }

    return -1;
}

CHNSYS_INT RTSP_CLIENT_Seek(RTSP_CLIENT_HANDLE hRTSPClient)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        return pRTSPClientSource->RTSPSeek();
    }

    return -1;
}

CHNSYS_INT RTSP_CLIENT_Stop(RTSP_CLIENT_HANDLE hRTSPClient)
{
    RTSPClientSource *pRTSPClientSource = NULL;

    if (hRTSPClient != NULL)
    {
        pRTSPClientSource = (RTSPClientSource *)hRTSPClient;
        return pRTSPClientSource->RTSPStop();
    }

    return -1;
}

//////////////////////////////////////////////////////////////////////////

RTSP_SERVER_HANDLE RTSP_SERVER_CreateInstance()
{
    RTSPServerSource *pRTSPServerSource = NEW RTSPServerSource;
    if (pRTSPServerSource != NULL)
    {
        return (RTSP_SERVER_HANDLE)pRTSPServerSource;
    }

    return NULL;
}

VOID RTSP_SERVER_DestroyInstance(RTSP_SERVER_HANDLE hRTSPServer)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        SAFE_DELETE(pRTSPServerSource);
    }
}

VOID RTSP_SERVER_SetConfig(RTSP_SERVER_HANDLE hRTSPServer, RTSP_SERVER_CONFIG config)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        pRTSPServerSource->SetConfig(config);
    }
}

CHNSYS_INT RTSP_SERVER_GetConfig(RTSP_SERVER_HANDLE hRTSPServer, RTSP_SERVER_CONFIG *pConfig)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->GetConfig(pConfig);
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_Start(RTSP_SERVER_HANDLE hRTSPServer)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->ServerStart();
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_Stop(RTSP_SERVER_HANDLE hRTSPServer)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->ServerStop();
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_SetEachTimeSendCount(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_INT nSendCount)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->SetEachTimeSendCount(nSendCount);
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_SetPatchAudioFrame(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_BOOL bPatch)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        pRTSPServerSource->SetPatchAudioFrame(bPatch);
        return 0;
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_SetRTPPacketMaxSize(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_INT nMaxSize)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        pRTSPServerSource->SetRTPPacketMaxSize(nMaxSize);
        return 0;
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_GetRTPPacketMaxSize(RTSP_SERVER_HANDLE hRTSPServer)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->GetRTPPacketMaxSize();
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_SetStreamMode(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_INT nStreamMode)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        pRTSPServerSource->SetRTSPStreamMode(nStreamMode);
        return 0;
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_GetStreamMode(RTSP_SERVER_HANDLE hRTSPServer)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->GetRTSPStreamMode();
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_PushH264Data(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_UINT nTimestamp, CHNSYS_CHAR *sData, CHNSYS_INT nData)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->PushH264Data(nTimestamp, sData, nData);
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_PushAACData(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_UINT nTimestamp, CHNSYS_CHAR *sData, CHNSYS_INT nData)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->PushAACData(nTimestamp, sData, nData);
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_PushG711Data(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_UINT nTimestamp, CHNSYS_CHAR *sData, CHNSYS_INT nData)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->PushG711Data(nTimestamp, sData, nData);
    }

    return -1;
}

VOID RTSP_SERVER_SetStatusCallBack(RTSP_SERVER_HANDLE hRTSPServer, RTSP_SERVER_StatusCallBackFunc scbf, VOID *pContext)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        pRTSPServerSource->SetStatusCallBack(scbf, pContext);
    }
}

VOID RTSP_SERVER_SetRequestCallBack(RTSP_SERVER_RequestCallBackFunc rcbf, VOID *pContext)
{
    RTSPServerListenerManager::GetInstance()->SetRequestCallBack(rcbf, pContext);
}

CHNSYS_INT RTSP_SERVER_GetClientCount(RTSP_SERVER_HANDLE hRTSPServer)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->GetClientCount();
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_GetClientProperty(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_INT nIndex, 
                                         RTSP_SERVER_CLIENTPROPERTY *pProperty)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->GetClientProperty(nIndex, pProperty);
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_DisconnectClient(RTSP_SERVER_HANDLE hRTSPServer, RTSP_SERVER_CLIENTPROPERTY clientProperty)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->DisconnectClient(clientProperty);
    }

    return -1;
}

CHNSYS_INT RTSP_SERVER_DisconnectAllClient(RTSP_SERVER_HANDLE hRTSPServer)
{
	RTSPServerSource *pRTSPServerSource = NULL;

	if (hRTSPServer != NULL)
	{
		pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
		return pRTSPServerSource->DisconnectAllClient();
	}

	return -1;
}

CHNSYS_INT RTSP_SERVER_SetMaxClientCount(RTSP_SERVER_HANDLE hRTSPServer,int nCount)
{
	RTSPServerSource *pRTSPServerSource = NULL;

	if (hRTSPServer != NULL)
	{
		pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
		return pRTSPServerSource->SetMaxClientCount(nCount);
	}

	return -1;
}

CHNSYS_INT RTSP_SERVER_ResetSDPByAudio(RTSP_SERVER_HANDLE hRTSPServer, RTSP_SERVER_CONFIG config)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->ResetSDPByAudio(config);
    }

    return -1;
}

VOID RTSP_SERVER_SetSDP(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_CHAR *sSDP, CHNSYS_INT nSDP,CHNSYS_CHAR *sMultiAddr,CHNSYS_INT nMultiPort)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        pRTSPServerSource->SetSDP(sSDP, nSDP,sMultiAddr,nMultiPort);
    }
}

CHNSYS_INT RTSP_SERVER_PushRTPData(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_INT nStreamNo, CHNSYS_CHAR *sRTPData, CHNSYS_INT nRTPSize)
{
    RTSPServerSource *pRTSPServerSource = NULL;

    if (hRTSPServer != NULL)
    {
        pRTSPServerSource = (RTSPServerSource *)hRTSPServer;
        return pRTSPServerSource->PushRTPData(nStreamNo, sRTPData, nRTPSize);
    }

    return -1;
}
