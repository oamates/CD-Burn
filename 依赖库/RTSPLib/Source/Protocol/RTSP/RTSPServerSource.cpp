#include "RTSPServerSource.h"
#include "Packetizer3984.h"
#include "Packetizer3016.h"
#include "Packetizer3640.h"
#include "PacketizerG711.h"
#include "MulticastPin.h"
#include "ZRTPPacket.h"
#include "SDPUtil.h"
#include "NalUtil.h"
#include "RemoteRTSPClient.h"

RTSPServerSource::RTSPServerSource()
: ZTask("RTSPServerSource")
, m_arrayRTPPacketizer(10)
, m_mutexRTPPacketizerArray("Mutex_RTPPacketizerArray")
, m_bIsMulticast(FALSE)
, m_pMulticastPin(NULL)
, m_sSDP(NULL)
, m_nLastCheckSdpChangeTime(0)
, m_nBackupSPS(0)
, m_nBackupPPS(0)
, m_nAACPayload(98)
, m_pAACPacketizer(NULL)
, m_pG711Packetizer(NULL)
, m_nH264Payload(96)
, m_pH264Packetizer(NULL)
, m_arrayRemoteClient(10)
, m_mutexRemoteClientArray("Mutex_RemoteClientArray")
, m_nMaxRemoteClient(4)
, m_StatusCallBackFunc(NULL)
, m_pscbfContext(NULL)
, m_bServerStart(FALSE)
, m_nEachTimeSendCount(10)
, m_bPatchAudioFrame(FALSE)
, m_nRTPPacketMaxSize(1440)
, m_nRTSPStreamMode(0)
{
    memset(m_sBackupSPS, 0, 8192);
    memset(m_sBackupPPS, 0, 8192);
    memset(&m_config, 0, sizeof(m_config));
    ZTask::Create();
}

RTSPServerSource::~RTSPServerSource()
{
    int i = 0;

    ZTask::Close();
    
    {
        ZOSMutexLocker locker(&m_mutexRTPPacketizerArray);
        for (i = 0; i < m_arrayRTPPacketizer.Size(); i ++)
        {
            if (m_arrayRTPPacketizer[i] != NULL)
            {
                DEL m_arrayRTPPacketizer[i];
            }
        }
        m_arrayRTPPacketizer.RemoveAll();
    }
    if (m_pMulticastPin != NULL)
    {
        DEL m_pMulticastPin;
        m_pMulticastPin = NULL;
    }
    SAFE_DELETE(m_sSDP);

    {
        ZOSMutexLocker locker(&m_mutexRemoteClientArray);
        for (i = 0; i < m_arrayRemoteClient.Size(); i ++)
        {
            if (m_arrayRemoteClient[i] != NULL)
            {
                m_arrayRemoteClient[i]->Close();
                DEL m_arrayRemoteClient[i];
            }
        }
        m_arrayRemoteClient.RemoveAll();
    }

    if (m_config.sSPS != NULL)
    {
        SAFE_DELETE(m_config.sSPS);
        m_config.nSPS = 0;
    }
    if (m_config.sPPS != NULL)
    {
        SAFE_DELETE(m_config.sPPS);
        m_config.nPPS = 0;
    }

    m_bServerStart = FALSE;

}

void RTSPServerSource::SetConfig(RTSP_SERVER_CONFIG config)
{
    m_config.nSendType = config.nSendType;
    strncpy(m_config.sMulticastIP, config.sMulticastIP, 16);
    m_config.sMulticastIP[15] = '\0';
    m_config.nMulticastPort = config.nMulticastPort;
    m_config.nAudioEncodeType = config.nAudioEncodeType;
    m_config.nSampleRate = config.nSampleRate;
    m_config.nChannelNo = config.nChannelNo;
    m_config.nProfileLevelID = config.nProfileLevelID;
    m_config.nMode = config.nMode;
    m_config.nPayloadFormatType = config.nPayloadFormatType;
    if (config.nSPS > 0)
    {
        m_config.sSPS = NEW char[config.nSPS];
        memcpy(m_config.sSPS, config.sSPS, config.nSPS);
    }
    m_config.nSPS = config.nSPS;
    if (config.nPPS > 0)
    {
        m_config.sPPS = NEW char[config.nPPS];
        memcpy(m_config.sPPS, config.sPPS, config.nPPS);
    }
    m_config.nPPS = config.nPPS;
    m_config.nPacketizationMode = config.nPacketizationMode;
    m_config.nSRPacketTimeInterval = config.nSRPacketTimeInterval;
    m_config.nRRWaitThreshold = config.nRRWaitThreshold;
    strncpy(m_config.sUserName, config.sUserName, 16);
    m_config.sUserName[15] = '\0';
    strncpy(m_config.sPassword, config.sPassword, 16);
    m_config.sPassword[15] = '\0';

    // check multicast
    if (
        (strlen(m_config.sMulticastIP) > 0)
        && (m_config.nMulticastPort > 0)
        )
    {
        m_bIsMulticast = TRUE;
        SAFE_DELETE(m_pMulticastPin);
        m_pMulticastPin = NEW MulticastPin;
        if (m_pMulticastPin != NULL)
        {
            m_pMulticastPin->SetIPPort(m_config.sMulticastIP, m_config.nMulticastPort);
        }
    }

    if (!CreatePacketizer())
    {
        LOG_ERROR(("[RTSPServerSource::SetConfig] CreatePacketizer error\r\n"));
    }

    if (
        (m_config.nSPS > 0)
        && (m_config.nPPS > 0)
        )
    {
        if (!CreateSDP())
        {
            LOG_ERROR(("[RTSPServerSource::SetConfig] CreateSDP error\r\n"));
        }
    }
}

int RTSPServerSource::GetConfig(RTSP_SERVER_CONFIG *pConfig)
{
    if (pConfig != NULL)
    {
        pConfig->nSendType = m_config.nSendType;
        strncpy(pConfig->sMulticastIP, m_config.sMulticastIP, 16);
        pConfig->sMulticastIP[15] = '\0';
        pConfig->nMulticastPort = m_config.nMulticastPort;
        pConfig->nAudioEncodeType = m_config.nAudioEncodeType;
        pConfig->nSampleRate = m_config.nSampleRate;
        pConfig->nChannelNo = m_config.nChannelNo;
        pConfig->nProfileLevelID = m_config.nProfileLevelID;
        pConfig->nMode = m_config.nMode;
        pConfig->nPayloadFormatType = m_config.nPayloadFormatType;
        if (m_config.nSPS > 0)
        {
            memcpy(pConfig->sSPS, m_config.sSPS, m_config.nSPS);
        }
        pConfig->nSPS = m_config.nSPS;
        if (m_config.nPPS > 0)
        {
            memcpy(pConfig->sPPS, m_config.sPPS, m_config.nPPS);
        }
        pConfig->nPPS = m_config.nPPS;
        pConfig->nPacketizationMode = m_config.nPacketizationMode;
        pConfig->nSRPacketTimeInterval = m_config.nSRPacketTimeInterval;
        pConfig->nRRWaitThreshold = m_config.nRRWaitThreshold;
        strncpy(pConfig->sUserName, m_config.sUserName, 16);
        pConfig->sUserName[15] = '\0';
        strncpy(pConfig->sPassword, m_config.sPassword, 16);
        pConfig->sPassword[15] = '\0';

        return 0;
    }

    return -1;
}

int RTSPServerSource::ServerStart()
{
    m_bServerStart = TRUE;
    if (m_pMulticastPin != NULL)
    {
        m_pMulticastPin->Start();
    }
    ZTask::AddEvent(TASK_START_EVENT);

    return 0;
}

int RTSPServerSource::ServerStop()
{
    m_bServerStart = FALSE;
    if (m_pMulticastPin != NULL)
    {
        m_pMulticastPin->Stop();
    }

    return 0;
}

int RTSPServerSource::PushH264Data(UINT nTimestamp, char *sData, int nData)
{
    int nRelative = 0;
    int nLen = 0;

    if (m_bServerStart)
    {
        UINT64 nCurrentTime = ZOS::milliseconds();

        //get sps from h264 stream
        if (
            (m_config.sSPS == NULL)
            && (m_config.nSPS <= 0)
            )
        {
            if (NALUTIL_GetSPS(sData, nData, &nRelative, &nLen) != NULL)
            {
                m_config.sSPS = NEW char[nLen];
                memcpy(m_config.sSPS, sData+nRelative, nLen);
                m_config.nSPS = nLen;
            }
        }

        //get pps from h264 stream
        if (
            (m_config.sPPS == NULL)
            && (m_config.nPPS <= 0)
            )
        {
            if (NALUTIL_GetPPS(sData, nData, &nRelative, &nLen) != NULL)
            {
                m_config.sPPS = NEW char[nLen];
                memcpy(m_config.sPPS, sData+nRelative, nLen);
                m_config.nPPS = nLen;
            }
        }

        // generate SDP
        if (
            (m_config.nSPS > 0) 
            && (m_config.nPPS > 0)
            && (m_sSDP == NULL)
            )
        {
            CreateSDP();
            m_nLastCheckSdpChangeTime = nCurrentTime;
        }

        //each 3000 milli second, check reset sdp
        if (
            (m_sSDP != NULL) 
            && (nCurrentTime - m_nLastCheckSdpChangeTime > 3000)
            )
        {//m_sSDP != NULL to ensure sdp have already generated
            nRelative = 0;
            nLen = 0;
            if (NALUTIL_GetSPS(sData, nData, &nRelative, &nLen) != NULL)
            {
                //LOG_DEBUG(("[RTSPServerSource::PushH264Data] get sps\r\n"));
                if (nLen <= 8192 && nLen > 0)
                {
                    memcpy(m_sBackupSPS, sData+nRelative, nLen);
                    m_nBackupSPS = nLen;
                }
                else
                {
                    LOG_ERROR(("[RTSPServerSource::PushH264Data] sps too large more than 8192 or less zero\r\n"));
                }
            }

            nRelative = 0;
            nLen = 0;
            if (NALUTIL_GetPPS(sData, nData, &nRelative, &nLen) != NULL)
            {
                //LOG_DEBUG(("[RTSPServerSource::PushH264Data] get pps\r\n"));
                if (nLen <= 8192 && nLen > 0)
                {
                    memcpy(m_sBackupPPS, sData+nRelative, nLen);
                    m_nBackupPPS = nLen;
                }
                else
                {
                    LOG_ERROR(("[RTSPServerSource::PushH264Data] pps too large more than 8192 or less zero\r\n"));
                }
            }

            if (m_nBackupSPS > 0 && m_nBackupPPS > 0)
            {
                bool bDiff = false;
                if (
                    (m_nBackupSPS != m_config.nSPS)
                    || (memcmp(m_sBackupSPS, m_config.sSPS, m_config.nSPS) != 0)
                    )
                {
                    bDiff = true;
                }
                if (
                    (m_nBackupPPS != m_config.nPPS)
                    || (memcmp(m_sBackupPPS, m_config.sPPS, m_config.nPPS) != 0)
                    )
                {
                    bDiff = true;
                }

                if (bDiff)
                {
                    //reset sdp string, packetizer no change.
                    memcpy(m_config.sSPS, m_sBackupSPS, m_nBackupSPS);
                    m_config.nSPS = m_nBackupSPS;
                    memcpy(m_config.sPPS, m_sBackupPPS, m_nBackupPPS);
                    m_config.nPPS = m_nBackupPPS;

                    LOG_DEBUG(("[RTSPServerSource::PushH264Data] reset sdp\r\n"));
                    CreateSDP();
                }

                m_nLastCheckSdpChangeTime = nCurrentTime;
                m_nBackupSPS = 0;
                m_nBackupPPS = 0;
            }
        }

        //0 - vid & aud
        //1 - vid
        if (m_nRTSPStreamMode == 0 || m_nRTSPStreamMode == 1)
        {
            if (m_pH264Packetizer != NULL)
            {
                m_pH264Packetizer->SetFrameTimestamp(nTimestamp);
                m_pH264Packetizer->ReceiveFrameData(101, sData, nData);
            }
        }
    }

    return 0;
}

int RTSPServerSource::PushAACData(UINT nTimestamp, char *sData, int nData)
{
    if (m_bServerStart)
    {
        //0 - vid & aud
        //2 - aud
        if (m_nRTSPStreamMode == 0 || m_nRTSPStreamMode == 2)
        {
            if (m_pAACPacketizer != NULL)
            {
                m_pAACPacketizer->SetFrameTimestamp(nTimestamp);
                m_pAACPacketizer->ReceiveFrameData(102, sData, nData);
            }
        }
    }

    return 0;
}

int RTSPServerSource::PushG711Data(UINT nTimestamp, char *sData, int nData)
{
    if (m_bServerStart)
    {
        //0 - vid & aud
        //2 - aud
        if (m_nRTSPStreamMode == 0 || m_nRTSPStreamMode == 2)
        {
            if (m_pG711Packetizer != NULL)
            {
                m_pG711Packetizer->SetFrameTimestamp(nTimestamp);
                m_pG711Packetizer->ReceiveFrameData(103, sData, nData);
            }
        }
    }

    return 0;
}

void RTSPServerSource::SetStatusCallBack(RTSP_SERVER_StatusCallBackFunc scbf, void *pContext)
{
    m_StatusCallBackFunc = scbf;
    m_pscbfContext = pContext;
}

int RTSPServerSource::GetClientCount()
{
    ZOSMutexLocker  locker(&m_mutexRemoteClientArray);

    return m_arrayRemoteClient.Count();
}

int RTSPServerSource::GetClientProperty(int nIndex, RTSP_SERVER_CLIENTPROPERTY *pProperty)
{
    ZRTSPSession *pRTSPSession = NULL;
    ZOSMutexLocker  locker(&m_mutexRemoteClientArray);

    if (
        (nIndex < m_arrayRemoteClient.Count())
        && (pProperty != NULL)
        )
    {
        if (m_arrayRemoteClient[nIndex] != NULL)
        {
            pRTSPSession = m_arrayRemoteClient[nIndex]->GetRTSPSession();
            if (pRTSPSession != NULL)
            {
                strncpy(pProperty->sClientIP, ZSocket::ConvertAddr(pRTSPSession->GetSessionAddr()), MAX_IP_LENGTH);
                pProperty->sClientIP[MAX_IP_LENGTH-1] = '\0';
                pProperty->nRTSPPort = pRTSPSession->GetSessionPort();
                pProperty->nSessionDuration = pRTSPSession->GetSessionDuration();
                pProperty->nTotalPacketCount = pRTSPSession->GetSessionTotalPacketCount();
                pProperty->nTotalLostPacketCount = pRTSPSession->GetSessionLostPacketCount();
                return 0;
            }
        }
    }

    return -1;
}

int RTSPServerSource::DisconnectClient(RTSP_SERVER_CLIENTPROPERTY clientProperty)
{
    int i = 0;
    ZOSMutexLocker      locker(&m_mutexRemoteClientArray);

    for (i = 0; i < m_arrayRemoteClient.Size(); i ++)
    {
        if (m_arrayRemoteClient[i] != NULL)
        {
            if (
                (strcmp(m_arrayRemoteClient[i]->GetRemoteIP(),clientProperty.sClientIP) == 0)
                && (m_arrayRemoteClient[i]->GetRemotePort()==clientProperty.nRTSPPort)
                )
            {
                m_arrayRemoteClient[i]->Close();
                DEL m_arrayRemoteClient[i];
                m_arrayRemoteClient.Remove(i);
                break;
            }
        }
    }


    return 0;
}

int RTSPServerSource::DisconnectAllClient()
{
	int i = 0;
	ZOSMutexLocker locker(&m_mutexRemoteClientArray);
	for (i = 0; i < m_arrayRemoteClient.Size(); i ++)
	{
		if (m_arrayRemoteClient[i] != NULL)
		{
			m_arrayRemoteClient[i]->Close();
			DEL m_arrayRemoteClient[i];
		}
	}
	m_arrayRemoteClient.RemoveAll();

	return 0;
}

int RTSPServerSource::SetMaxClientCount(int nCount)
{
	m_nMaxRemoteClient = nCount;

	return 0;
}

int RTSPServerSource::GetMaxClientCount()
{
	return m_nMaxRemoteClient;
}

int RTSPServerSource::ResetSDPByAudio(RTSP_SERVER_CONFIG config)
{
    m_config.nAudioEncodeType = config.nAudioEncodeType;
    m_config.nSampleRate = config.nSampleRate;
    m_config.nChannelNo = config.nChannelNo;
    m_config.nProfileLevelID = config.nProfileLevelID;
    m_config.nMode = config.nMode;
    m_config.nPayloadFormatType = config.nPayloadFormatType;

    CreateSDP();

    return 0;
}

#if 0
void RTSPServerSource::SetSDP(char *sSDP, int nSDP,char *sMultiAddr,int nMultiPort)
{
    int nStreamCount = 0;
    int i = 0;
    CHAR sMultiIP[MAX_IP_LENGTH];

    SAFE_DELETE(m_sSDP);
    m_sSDP = NEW char[nSDP+4];
    strncpy(m_sSDP, sSDP, nSDP+4);
    m_sSDP[nSDP] = '\0';
    memset(sMultiIP, 0, MAX_IP_LENGTH);

    // create rtppacketizer
    nStreamCount = SDP_GetStreamCount(sSDP, nSDP);
    for (i = 0; i < nStreamCount; i ++)
    {
        RTPPacketizer *pPacketizer = NEW RTPPacketizer;
        if (pPacketizer != NULL)
        {
            ZOSMutexLocker locker(&m_mutexRTPPacketizerArray);
            m_arrayRTPPacketizer.Add(pPacketizer);
        }
    }

    //multicast session need add multicast pin
    if (SDP_IsMulticastSession(sSDP, nSDP))
    {
        m_bIsMulticast = TRUE;
        SAFE_DELETE(m_pMulticastPin);
        m_pMulticastPin = NEW MulticastPin;
        if (m_pMulticastPin != NULL)
        {
            m_pMulticastPin->SetIPPort(SDP_GetMulticastIP(sSDP, nSDP, sMultiIP), 0);
            ZOSMutexLocker locker(&m_mutexRTPPacketizerArray);
            for (i = 0; i < m_arrayRTPPacketizer.Count(); i ++)
            {
                if (m_arrayRTPPacketizer[i] != NULL)
                {
                    m_pMulticastPin->AddStream(m_arrayRTPPacketizer[i], i, 
                        SDP_GetMulticastPort(sSDP, nSDP, i));
                }
            }
        }
    }
}
#endif

static int GetLine(char *sSrcData, int nSrcTotalSize, int nStartPos, char *sValue)
{
	if (
		nStartPos >= nSrcTotalSize 
		|| sSrcData == NULL
		|| sValue == NULL
		)
	{
		return -1;
	}

	int nPos = nStartPos;
	while (nPos < nSrcTotalSize)
	{
		if (sSrcData[nPos] == '\n')
		{
			memset(sValue, 0, (nPos-nStartPos+2));
			memcpy(sValue, sSrcData+nStartPos, nPos-nStartPos+1);
			return nPos;
		}
		else
		{
			nPos ++;
		}
	}

	return -1;
}

static void ModifyMultiAddr(char *pSrc, char *pAddr)
{
	// pSrc == "c=IN IP4 0.0.0.0"
	char	szTmp1[64];
	char	szTmp2[64];
	char	szTmp3[64];

	sscanf(pSrc, "%s %s %s", szTmp1, szTmp2, szTmp3);
	sprintf(pSrc, "%s %s %s/255\r\n", szTmp1, szTmp2, pAddr);
}

static void ModifyMultiPort(char *pSrc, int nPort)
{
	// pSrc == "m=audio 0 RTP/AVP 98" 
	char	szTmp1[64];
	char	szTmp2[64];
	char	szTmp3[64];
	char	szTmp4[64];

	sscanf(pSrc, "%s %s %s %s", szTmp1, szTmp2, szTmp3, szTmp4);
	sprintf(pSrc, "%s %d %s %s\r\n", szTmp1, nPort, szTmp3, szTmp4);
}

void RTSPServerSource::SetSDP(char *sSDP, int nSDP,char *sMultiAddr,int nMultiPort)
{
	
	int nStreamCount = 0;
	int i = 0;

	SAFE_DELETE(m_sSDP);

	nStreamCount = SDP_GetStreamCount(sSDP, nSDP);
	for (i = 0; i < nStreamCount; i ++)
	{
		RTPPacketizer *pPacketizer = NEW RTPPacketizer;
		if (pPacketizer != NULL)
		{
			ZOSMutexLocker locker(&m_mutexRTPPacketizerArray);
			m_arrayRTPPacketizer.Add(pPacketizer);
		}
	}

	if(sMultiAddr == NULL)
	{
		m_sSDP = NEW char[nSDP+4];
		strncpy(m_sSDP, sSDP, nSDP+4);
		m_sSDP[nSDP] = '\0';
	}
	else
	{
		int		nStartPos = 0;
		char	szLine[512];

		m_sSDP = NEW char[nSDP+16];
		memset(m_sSDP,0,nSDP+16);
		i = 0;

		nStartPos = GetLine(sSDP, nSDP, 0, szLine);
		while (nStartPos > 0 && nStartPos < nSDP)
		{
			if (strstr(szLine, "c=IN IP4"))
			{
				ModifyMultiAddr(szLine, sMultiAddr);
			}
			else if (strstr(szLine, "m=audio "))
			{
				ModifyMultiPort(szLine, nMultiPort + i);
				i += 2;
			}
			else if (strstr(szLine, "m=video "))
			{
				ModifyMultiPort(szLine, nMultiPort + i);
				i += 2;
			}

			strcat(m_sSDP, szLine);
			nStartPos = GetLine(sSDP, nSDP, nStartPos+1, szLine);
		}

		m_bIsMulticast = TRUE;
		SAFE_DELETE(m_pMulticastPin);
		m_pMulticastPin = NEW MulticastPin;
		if (m_pMulticastPin != NULL)
		{
			m_pMulticastPin->SetIPPort(sMultiAddr,nMultiPort);
			ZOSMutexLocker locker(&m_mutexRTPPacketizerArray);
			for (i = 0; i < m_arrayRTPPacketizer.Count(); i ++)
			{
				if (m_arrayRTPPacketizer[i] != NULL)
				{
					m_pMulticastPin->AddStream(m_arrayRTPPacketizer[i], i,1200);
				}
			}
		}
	}

}

int RTSPServerSource::PushRTPData(int nStreamNo, char *sRTPData, int nRTPSize)
{
    RTPPacketizer *pRtpPacketizer = NULL;

    {
        ZOSMutexLocker locker(&m_mutexRTPPacketizerArray);
        if (nStreamNo < m_arrayRTPPacketizer.Count())
        {
            pRtpPacketizer = m_arrayRTPPacketizer[nStreamNo];
        }
    }

    if (pRtpPacketizer != NULL)
    {
        pRtpPacketizer->ProcessFrame(sRTPData, nRTPSize);
    }

    return 0;
}

int RTSPServerSource::SetEachTimeSendCount(int nSendCount)
{
    int i = 0;

    m_nEachTimeSendCount = nSendCount;
    //
    ZOSMutexLocker  locker(&m_mutexRemoteClientArray);
    for (i = 0; i < m_arrayRemoteClient.Size(); i ++)
    {
        if (m_arrayRemoteClient[i] != NULL && m_arrayRemoteClient[i]->GetRTSPSession() != NULL)
        {
            m_arrayRemoteClient[i]->GetRTSPSession()->SetEachTimeSendCount(nSendCount);
        }
    }


    return 0;
}

void RTSPServerSource::SetPatchAudioFrame(BOOL bPatch)
{
    int i = 0;
    m_bPatchAudioFrame = bPatch;

    ZOSMutexLocker  locker(&m_mutexRTPPacketizerArray);

    for (i = 0; i < m_arrayRTPPacketizer.Count(); i ++)
    {
        //todo, do only audio. now include video.
        if (m_arrayRTPPacketizer[i] != NULL)
        {
            m_arrayRTPPacketizer[i]->SetPatchFrame(m_bPatchAudioFrame);
        }
    }
}

int RTSPServerSource::SetRTPPacketMaxSize(int nRTPPacketMaxSize)
{
    int i = 0;
    m_nRTPPacketMaxSize = nRTPPacketMaxSize;

    ZOSMutexLocker  locker(&m_mutexRTPPacketizerArray);

    for (i = 0; i < m_arrayRTPPacketizer.Count(); i ++)
    {
        if (m_arrayRTPPacketizer[i] != NULL)
        {
            m_arrayRTPPacketizer[i]->SetRTPPacketMaxSize(m_nRTPPacketMaxSize);
        }
    }

    return 0;
}

int RTSPServerSource::GetRTPPacketMaxSize()
{
    return m_nRTPPacketMaxSize;
}

int RTSPServerSource::SetRTSPStreamMode(int nRTSPStreamMode)
{
    m_nRTSPStreamMode = nRTSPStreamMode;
    LOG_DEBUG(("[RTSPServerSource::SetRTSPStreamMode] rtsp stream mode %d\r\n", nRTSPStreamMode));

    CreateSDP();
    return 0;
}

int RTSPServerSource::GetRTSPStreamMode()
{
    return m_nRTSPStreamMode;
}

int RTSPServerSource::Run(int nEvent)
{
    int		nTaskTime	= 0;
    UINT	nLocalEvent	= 0;

    nLocalEvent	= GetEvent(nEvent);

    ZTask::Run(nLocalEvent);

    if (nLocalEvent&TASK_UPDATE_EVENT)
    {
        nTaskTime = CheckClientSession();
    }

    return nTaskTime;
}

//////////////////////////////////////////////////////////////////////////
BOOL RTSPServerSource::OnNotifyOption(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl)
{
    RTSP_SERVER_STATUSCBDATA    scbd;
    RemoteRTSPClient            *pClient = NULL;

    pClient = GetRemoteRTSPClient(pRTSPSession);
    if (pClient != NULL)
    {
        //LOG_DEBUG(("[RTSPServerSource::OnNotifyOption] %s %d %s\r\n", 
        //    pClient->GetRemoteIP(), pClient->GetRemotePort(), sRequestUrl));
        if (m_StatusCallBackFunc != NULL)
        {
            scbd.hRTSPServer = this;
            scbd.nStatusType = 1;
            scbd.nStatusValue = 0;
            strncpy(scbd.clientProperty.sClientIP, pClient->GetRemoteIP(), MAX_IP_LENGTH);
            scbd.clientProperty.sClientIP[MAX_IP_LENGTH-1] = '\0';
            scbd.clientProperty.nRTSPPort = pClient->GetRemotePort();
            scbd.clientProperty.nSessionDuration = 0;
            scbd.clientProperty.nTotalPacketCount = 0;
            scbd.clientProperty.nTotalLostPacketCount = 0;
            scbd.pContext = m_pscbfContext;
            m_StatusCallBackFunc(scbd);
        }
        return TRUE;
    }

    return FALSE;
}

BOOL RTSPServerSource::OnNotifyDescribe(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl, CHAR *sSdp, int *nSDP)
{
    RTSP_SERVER_STATUSCBDATA    scbd;
    RemoteRTSPClient            *pClient = NULL;

    if (m_sSDP != NULL)
    {
        strcpy(sSdp, m_sSDP);
        *nSDP = strlen(sSdp);
    }

    pClient = GetRemoteRTSPClient(pRTSPSession);
    if (pClient != NULL)
    {
        //LOG_DEBUG(("[RTSPServerSource::OnNotifyDescribe] %s %d\r\n", 
        //    pClient->GetRemoteIP(), pClient->GetRemotePort()));
        if (m_StatusCallBackFunc != NULL)
        {
            scbd.hRTSPServer = this;
            scbd.nStatusType = 2;
            if (*nSDP > 0)
            {
                scbd.nStatusValue = 0;
            }
            else
            {
                scbd.nStatusValue = -1;
            }
            strncpy(scbd.clientProperty.sClientIP, pClient->GetRemoteIP(), MAX_IP_LENGTH);
            scbd.clientProperty.sClientIP[MAX_IP_LENGTH-1] = '\0';
            scbd.clientProperty.nRTSPPort = pClient->GetRemotePort();
            scbd.clientProperty.nSessionDuration = 0;
            scbd.clientProperty.nTotalPacketCount = 0;
            scbd.clientProperty.nTotalLostPacketCount = 0;
            scbd.pContext = m_pscbfContext;
            m_StatusCallBackFunc(scbd);
        }
        if (*nSDP > 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL RTSPServerSource::OnNotifySetup(ZRTSPSession *pRTSPSession, CHAR *sRequestUrl, int nStreamNo, int *nSendType, ZDataPin **ppDataPin)
{
    RTSP_SERVER_STATUSCBDATA    scbd;
    RemoteRTSPClient            *pClient = NULL;
    int                         nStatusValue = -1;
    RTPPacketizer               *pRtpPacketizer = NULL;

    {
        //todo:best is from nStreamNo and sdp content get packetizer
        ZOSMutexLocker      locker(&m_mutexRTPPacketizerArray);
        if (nStreamNo < m_arrayRTPPacketizer.Count())
        {
            if (m_nRTSPStreamMode == 0 || m_nRTSPStreamMode == 1)
            {
                pRtpPacketizer = m_arrayRTPPacketizer[nStreamNo];
            }
            else if (m_nRTSPStreamMode == 2)
            {
                if (m_arrayRTPPacketizer.Count() == 2)
                {
                    pRtpPacketizer = m_arrayRTPPacketizer[1];
                }
            }
            else
            {
                pRtpPacketizer = m_arrayRTPPacketizer[nStreamNo];
            }
        }
    }

    if (pRtpPacketizer != NULL)
    {
        if (m_bIsMulticast)
        {
            *nSendType = 1;
        }
        else
        {
            *nSendType = 0;
        }

        *ppDataPin = pRtpPacketizer;
        nStatusValue = 0;

        pClient = GetRemoteRTSPClient(pRTSPSession);
        if (pClient != NULL)
        {
            //LOG_DEBUG(("[RTSPServerSource::OnNotifySetup] %s %d\r\n", 
            //    pClient->GetRemoteIP(), pClient->GetRemotePort()));
            if (m_StatusCallBackFunc != NULL)
            {
                scbd.hRTSPServer = this;
                scbd.nStatusType = 3;
                scbd.nStatusValue = nStatusValue;
                strncpy(scbd.clientProperty.sClientIP, pClient->GetRemoteIP(), MAX_IP_LENGTH);
                scbd.clientProperty.sClientIP[MAX_IP_LENGTH-1] = '\0';
                scbd.clientProperty.nRTSPPort = pClient->GetRemotePort();
                scbd.clientProperty.nSessionDuration = 0;
                scbd.clientProperty.nTotalPacketCount = 0;
                scbd.clientProperty.nTotalLostPacketCount = 0;
                scbd.pContext = m_pscbfContext;
                m_StatusCallBackFunc(scbd);
            }

            return TRUE;
        }
    }
    return FALSE;
}

BOOL RTSPServerSource::OnNotifyPlay(ZRTSPSession *pRTSPSession, int nStartSecond)
{
    RTSP_SERVER_STATUSCBDATA    scbd;
    RemoteRTSPClient            *pClient = NULL;

    pClient = GetRemoteRTSPClient(pRTSPSession);
    if (pClient != NULL)
    {
        LOG_DEBUG(("[RTSPServerSource::OnNotifyPlay] %s %d\r\n", 
            pClient->GetRemoteIP(), pClient->GetRemotePort()));
        if (m_StatusCallBackFunc != NULL)
        {
            scbd.hRTSPServer = this;
            scbd.nStatusType = 4;
            scbd.nStatusValue = 0;
            strncpy(scbd.clientProperty.sClientIP, pClient->GetRemoteIP(), MAX_IP_LENGTH);
            scbd.clientProperty.sClientIP[MAX_IP_LENGTH-1] = '\0';
            scbd.clientProperty.nRTSPPort = pClient->GetRemotePort();
            scbd.clientProperty.nSessionDuration = 0;
            scbd.clientProperty.nTotalPacketCount = 0;
            scbd.clientProperty.nTotalLostPacketCount = 0;
            scbd.pContext = m_pscbfContext;
            m_StatusCallBackFunc(scbd);
        }

        return TRUE;
    }

    return FALSE;
}

BOOL RTSPServerSource::OnNotifyPause(ZRTSPSession *pRTSPSession, int *nCurSecond)
{
    return TRUE;
}

BOOL RTSPServerSource::OnNotifyTearDown(ZRTSPSession *pRTSPSession)
{
    RTSP_SERVER_STATUSCBDATA    scbd;
    RemoteRTSPClient            *pClient = NULL;
    BOOL                        bReturn = FALSE;

    pClient = GetRemoteRTSPClient(pRTSPSession);
    if (pClient != NULL)
    {
        //LOG_DEBUG(("[RTSPServerSource::OnNotifyTearDown] %s %d\r\n", 
        //    pClient->GetRemoteIP(), pClient->GetRemotePort()));
        if (m_StatusCallBackFunc != NULL)
        {
            scbd.hRTSPServer = this;
            scbd.nStatusType = 5;
            scbd.nStatusValue = 0;
            strncpy(scbd.clientProperty.sClientIP, pClient->GetRemoteIP(), MAX_IP_LENGTH);
            scbd.clientProperty.sClientIP[MAX_IP_LENGTH-1] = '\0';
            scbd.clientProperty.nRTSPPort = pClient->GetRemotePort();
            scbd.clientProperty.nSessionDuration = 0;
            scbd.clientProperty.nTotalPacketCount = 0;
            scbd.clientProperty.nTotalLostPacketCount = 0;
            scbd.pContext = m_pscbfContext;
            m_StatusCallBackFunc(scbd);
        }
        bReturn = TRUE;
    }

    ZTask::AddEvent(ZTask::TASK_UPDATE_EVENT);

    return bReturn;
}

BOOL RTSPServerSource::OnNotifySessionTimeout(ZRTSPSession *pRTSPSession)
{
    RemoteRTSPClient            *pClient = NULL;

    pClient = GetRemoteRTSPClient(pRTSPSession);
    if (pClient != NULL)
    {
        LOG_DEBUG(("[RTSPServerSource::OnNotifySessionTimeout] %s %d\r\n", 
            pClient->GetRemoteIP(), pClient->GetRemotePort()));
    }

    ZTask::AddEvent(ZTask::TASK_UPDATE_EVENT);

    return TRUE;
}

RemoteRTSPClient *RTSPServerSource::GetRemoteRTSPClient(ZRTSPSession *pRTSPSession)
{
    int i = 0;
    ZOSMutexLocker  locker(&m_mutexRemoteClientArray);

    for (i = 0; i < m_arrayRemoteClient.Size(); i ++)
    {
        if (m_arrayRemoteClient[i] != 0)
        {
            if (m_arrayRemoteClient[i]->IsMe(pRTSPSession))
            {
                return m_arrayRemoteClient[i];
            }
        }
    }

    return NULL;
}

int RTSPServerSource::AddRemoteRTSPClient(RemoteRTSPClient *pRemoteRTSPClient)
{
    int i = 0;
    bool bFind = false;
    ZOSMutexLocker  locker(&m_mutexRemoteClientArray);

    if (pRemoteRTSPClient != NULL && pRemoteRTSPClient->GetRTSPSession() != NULL)
    {
        pRemoteRTSPClient->GetRTSPSession()->SetEachTimeSendCount(m_nEachTimeSendCount);
    }
    LOG_DEBUG(("[RTSPServerSource::AddRemoteRTSPClient] %d each time send count %d\r\n", 
        m_arrayRemoteClient.Size(),
        m_nEachTimeSendCount));
    for (i = 0; i < m_arrayRemoteClient.Size(); i ++)
    {
        if (m_arrayRemoteClient[i] != 0)
        {
            if (m_arrayRemoteClient[i] == pRemoteRTSPClient)
            {
                bFind = true;
                break;
            }
        }
    }

    if (!bFind)
    {
        m_arrayRemoteClient.Add(pRemoteRTSPClient);
        return 0;
    }
    else
    {
        return 1;
    }
}

BOOL RTSPServerSource::CreatePacketizer()
{
    int i = 0;
    BOOL bCreateH264 = TRUE;
    BOOL bCreateAAC = TRUE;
    BOOL bCreateG711 = TRUE;

    ZOSMutexLocker  locker(&m_mutexRTPPacketizerArray);

    for (i = 0; i < m_arrayRTPPacketizer.Count(); i ++)
    {
        if (m_arrayRTPPacketizer[i]->GetPacketizerFlag() == 101)
        {//have h264
            bCreateH264 = FALSE;
        }
        else if (m_arrayRTPPacketizer[i]->GetPacketizerFlag() == 102)
        {//have AAC
            bCreateAAC = FALSE;
        }
        else if (m_arrayRTPPacketizer[i]->GetPacketizerFlag() == 103)
        {//have G711
            bCreateG711 = FALSE;
        }
    }

	if (bCreateH264)
    {
        RTPPacketizer *pPacketizer = NEW Packetizer3984;
        if (pPacketizer != NULL)
        {
            m_pH264Packetizer = pPacketizer;
            m_pH264Packetizer->SetPacketizerFlag(101);
            m_pH264Packetizer->SetPayloadType(m_nH264Payload);
            m_pH264Packetizer->SetSSRC(ZRTPPacket::GenerateSSRC());
            m_arrayRTPPacketizer.Add(pPacketizer);

            if (
                (m_pMulticastPin != NULL)
                && (pPacketizer != NULL)
                )
            {
                m_pMulticastPin->AddStream(pPacketizer, 0,2000);
            }
        }
    }

    // aac
    if (m_config.nAudioEncodeType == 1 && bCreateAAC)
    {
        RTPPacketizer *pPacketizer = NULL;
        if (m_config.nPayloadFormatType == 1)
        {//mpeg4-generic
            pPacketizer = NEW Packetizer3640;
            if (pPacketizer != NULL)
            {
                m_pAACPacketizer = pPacketizer;
                m_pAACPacketizer->SetPacketizerFlag(102);
                m_pAACPacketizer->SetPayloadType(m_nAACPayload);
                m_pAACPacketizer->SetSSRC(ZRTPPacket::GenerateSSRC());
                m_pAACPacketizer->SetPatchFrame(m_bPatchAudioFrame);
                m_arrayRTPPacketizer.Add(pPacketizer);
            }
        }
        else if (m_config.nPayloadFormatType == 2)
        {//mp4a-latm
            pPacketizer = NEW Packetizer3016;
            if (pPacketizer != NULL)
            {
                m_pAACPacketizer = pPacketizer;
                m_pAACPacketizer->SetPacketizerFlag(102);
                m_pAACPacketizer->SetPayloadType(m_nAACPayload);
                m_pAACPacketizer->SetSSRC(ZRTPPacket::GenerateSSRC());
                m_pAACPacketizer->SetPatchFrame(m_bPatchAudioFrame);
                m_arrayRTPPacketizer.Add(pPacketizer);
            }
        }
        if (
            (m_pMulticastPin != NULL)
            && (pPacketizer != NULL)
            )
        {
            m_pMulticastPin->AddStream(pPacketizer, 1,400);
        }
    }

    // g711
    if (
        (m_config.nAudioEncodeType == 2||m_config.nAudioEncodeType==3)
        && bCreateG711
        )
    {
        RTPPacketizer *pPacketizer = NULL;
        pPacketizer = NEW PacketizerG711;
        if (pPacketizer != NULL)
        {
            m_pG711Packetizer = pPacketizer;
            m_pG711Packetizer->SetPacketizerFlag(103);
            if (m_config.nAudioEncodeType==2)
            {
                m_pG711Packetizer->SetPayloadType(0);
            }
            else if (m_config.nAudioEncodeType==3)
            {
                m_pG711Packetizer->SetPayloadType(8);
            }
            else
            {
                m_pG711Packetizer->SetPayloadType(0);
            }
            m_pG711Packetizer->SetSSRC(ZRTPPacket::GenerateSSRC());
            m_arrayRTPPacketizer.Add(pPacketizer);
        }
        if (
            (m_pMulticastPin != NULL)
            && (pPacketizer != NULL)
            )
        {
            m_pMulticastPin->AddStream(pPacketizer, 1,400);
        }

    }

    if (m_arrayRTPPacketizer.Count() >= 1)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL RTSPServerSource::HaveAACPacketizer()
{
    int i = 0;
    ZOSMutexLocker  locker(&m_mutexRTPPacketizerArray);

    for (i = 0; i < m_arrayRTPPacketizer.Count(); i ++)
    {
        if (
            (m_arrayRTPPacketizer[i] != NULL)
            && (m_arrayRTPPacketizer[i]->GetPacketizerFlag() == 102)
            )
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL RTSPServerSource::HaveG711Packetizer()
{
    int i = 0;
    ZOSMutexLocker  locker(&m_mutexRTPPacketizerArray);

    for (i = 0; i < m_arrayRTPPacketizer.Count(); i ++)
    {
        if (
            (m_arrayRTPPacketizer[i] != NULL)
            && (m_arrayRTPPacketizer[i]->GetPacketizerFlag() == 103)
            )
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL RTSPServerSource::HaveH264Packetizer()
{
    int i = 0;
    ZOSMutexLocker  locker(&m_mutexRTPPacketizerArray);

    for (i = 0; i < m_arrayRTPPacketizer.Count(); i ++)
    {
        if (
            (m_arrayRTPPacketizer[i] != NULL)
            && (m_arrayRTPPacketizer[i]->GetPacketizerFlag() == 101)
            )
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL RTSPServerSource::CreateSDP()
{
    char szLine[1024];
    char szIP[MAX_IP_LENGTH];
    CHAR *pBasedSPS = NULL;
    CHAR *pBasedPPS = NULL;
    int nStreamNo = 0;
    int nG711Payload = 0;

    SAFE_DELETE(m_sSDP);
    m_sSDP = NEW char[4096];
    memset(m_sSDP, 0, 4096);
    memset(szLine, 0, 1024);
    memset(szIP, 0, MAX_IP_LENGTH);

	//m_config.nChannelNo = 2;
    //LOG_DEBUG(("[RTSPServerSource::CreateSDP] m_nRTSPStreamMode %d\r\n",
    //    m_nRTSPStreamMode));

    //
    if (ZOS::getlocalip("eth0", szIP) != 0)
    {
        if (ZOS::getlocalip("eth1", szIP) != 0)
        {
            sprintf(szIP, "0.0.0.0");
        }
    }

    strcat(m_sSDP, "v=0\r\n");
    sprintf(szLine, "o= - %d %d IN IP4 %s\r\n", (unsigned int)(ZOS::milliseconds()/0xffffffff), 
        (unsigned int)(ZOS::milliseconds()%0xffffffff), szIP);
    strcat(m_sSDP, szLine);
    if (!m_bIsMulticast)
    {
        strcat(m_sSDP, "c=IN IP4 0.0.0.0\r\n");
    }
    else
    {
        sprintf(szLine, "c=IN IP4 %s/255\r\n", m_config.sMulticastIP);
        strcat(m_sSDP, szLine);
    }
    strcat(m_sSDP, "a=tool: ZMediaServer\r\n");
    strcat(m_sSDP, "a=range:npt=0-\r\n");

	//m=video
    if (
        (m_nRTSPStreamMode == 0||m_nRTSPStreamMode == 1)
        && HaveH264Packetizer()
        )
    {
        if (
            (m_config.nSPS > 0) 
            && (m_config.sSPS != NULL)
            && (m_config.nPPS > 0)
            && (m_config.sPPS != NULL)
            )
        {
            if (!m_bIsMulticast)
            {
                sprintf(szLine, "m=video 0 RTP/AVP %d\r\n", m_nH264Payload);
            }
            else
            {
                sprintf(szLine, "m=video %d RTP/AVP %d\r\n", m_config.nMulticastPort, m_nH264Payload);
            }
            strcat(m_sSDP, szLine);
            sprintf(szLine, "a=rtpmap:%d H264/90000\r\n", m_nH264Payload);
            strcat(m_sSDP, szLine);
            pBasedSPS = new CHAR[m_config.nSPS*3];
            pBasedPPS	= new CHAR[m_config.nPPS*3];
            memset(pBasedSPS, 0, m_config.nSPS*3);
            memset(pBasedPPS, 0, m_config.nPPS*3);
            Common_Base64Encode(m_config.sSPS + 4, m_config.nSPS - 4, pBasedSPS, (m_config.nSPS -4)*3);
            Common_Base64Encode(m_config.sPPS + 4, m_config.nPPS - 4, pBasedPPS, (m_config.nPPS - 4)*3);
            sprintf(szLine, "a=fmtp:%d packetization-mode=1; sprop-parameter-sets=%s,%s;\r\n", m_nH264Payload, pBasedSPS, pBasedPPS);
            strcat(m_sSDP, szLine);
            sprintf(szLine, "a=control:trackID=%d\r\n", nStreamNo++);
            strcat(m_sSDP, szLine);
            
        }
    }
	
    //m=audio
    if (
        (m_nRTSPStreamMode == 0 || m_nRTSPStreamMode == 2)
        && HaveAACPacketizer()
        )
    {
        if (!m_bIsMulticast)
        {
            sprintf(szLine, "m=audio 0 RTP/AVP %d\r\n", m_nAACPayload);
        }
        else
        {
            sprintf(szLine, "m=audio %d RTP/AVP %d\r\n", m_config.nMulticastPort + 2, m_nAACPayload);
        }
        strcat(m_sSDP, szLine);
        if (m_config.nPayloadFormatType == 1)
        {//mpeg4-generic
            sprintf(szLine, "a=rtpmap:%d mpeg4-generic/%d/%d\r\n", 
                m_nAACPayload, m_config.nSampleRate, m_config.nChannelNo);
            strcat(m_sSDP, szLine);
            sprintf(szLine, "a=fmtp:%d streamtype=5; profile-level-id=%d; mode=%s; config=%.4X; SizeLength=13;IndexLength=3; IndexDeltaLength=3; Profile=1;\r\n", 
                m_nAACPayload, m_config.nProfileLevelID, SDP_GetModeName(m_config.nMode), SDP_GetMpeg4GenericConfig(2, m_config.nSampleRate, m_config.nChannelNo));
            strcat(m_sSDP, szLine);
        }
        else if (m_config.nPayloadFormatType == 2)
        {//mp4a-latm
            sprintf(szLine, "a=rtpmap:%d mp4a-latm/%d/%d\r\n", 
                m_nAACPayload, m_config.nSampleRate, m_config.nChannelNo);
            strcat(m_sSDP, szLine);
            sprintf(szLine, "a=fmtp:%d streamtype=5; profile-level-id=%d; cpresent=0; config=4000%.4X3FC0;\r\n",
                m_nAACPayload, m_config.nProfileLevelID, SDP_GetMpeg4GenericConfig(2, m_config.nSampleRate, m_config.nChannelNo)<<1);
            strcat(m_sSDP, szLine);
        }
        sprintf(szLine, "a=control:trackID=%d\r\n", nStreamNo++);
        strcat(m_sSDP, szLine);
		strcat(m_sSDP, "\r\n");
    }

    //m=audio
    if (
        (m_nRTSPStreamMode == 0 || m_nRTSPStreamMode == 2)
        && HaveG711Packetizer()
        )
    {
        if (m_config.nAudioEncodeType == 2)
        {
            nG711Payload = 0;
        }
        else if (m_config.nAudioEncodeType == 3)
        {
            nG711Payload = 8;
        }
        if (!m_bIsMulticast)
        {
            sprintf(szLine, "m=audio 0 RTP/AVP %d\r\n", nG711Payload);
        }
        else
        {
            sprintf(szLine, "m=audio %d RTP/AVP %d\r\n", m_config.nMulticastPort, nG711Payload);
        }
        strcat(m_sSDP, szLine);
		if(nG711Payload == 2)
		{
			sprintf(szLine, "a=rtmap:%d PCMU/8000\r\n", nG711Payload);
		}
		else
		{
			sprintf(szLine, "a=rtmap:%d PCMA/8000\r\n", nG711Payload);
		}
		strcat(m_sSDP, szLine);
        sprintf(szLine, "a=control:trackID=%d\r\n", nStreamNo++);
        strcat(m_sSDP, szLine);
		strcat(m_sSDP, "\r\n");
    }
    

    return TRUE;
}

int RTSPServerSource::CheckClientSession()
{
    int i = 0;
    bool bRemove = FALSE;
    ZOSMutexLocker  locker(&m_mutexRemoteClientArray);
	
    for (i = 0; i < m_arrayRemoteClient.Size(); i ++)
    {
        if (m_arrayRemoteClient[i] != NULL)
        {
            if (!m_arrayRemoteClient[i]->IsValid())
            {
                LOG_DEBUG(("[RTSPServerSource::CheckClientSession] delete remote %s %d\r\n", 
                    m_arrayRemoteClient[i]->GetRemoteIP(), m_arrayRemoteClient[i]->GetRemotePort()));
                RemoteRTSPClient *pClient = m_arrayRemoteClient[i];
                m_arrayRemoteClient.Remove(i);
                locker.Unlock();
                pClient->Close();
                DEL pClient;
                bRemove = TRUE;
                break;
            }
            }
        }

    if (bRemove)
    {
        return 10;
    }
    else
    {
        return 100;
    }
}
