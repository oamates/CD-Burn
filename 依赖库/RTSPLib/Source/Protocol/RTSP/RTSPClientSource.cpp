#include "RTSPClientSource.h"
#include "SDPUtil.h"
#include "ZTCPSocket.h"
#include "Parser3984.h"
#include "Parser3640.h"
#include "Parser3016.h"
#include "ParserG711.h"
#include "NalUtil.h"

#define DEFAULT_CHECK_RTSPSTATE_TIME		(1000)



/*typedef struct _CBitstream_ 
{
	int	m_uNumOfBitsInBuffer;
	const unsigned char *m_chDecBuffer;
	unsigned char m_chDecData;
	int	m_chDecBufferSize;
}CBitstream;

static unsigned char exp_golomb_bits[256] = 
{
	8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0
};

void bitstream_init(CBitstream* bitsStream,const unsigned char *buffer, unsigned int bit_len)
{
	bitsStream->m_chDecBuffer = buffer;
	bitsStream->m_chDecBufferSize = bit_len;
	bitsStream->m_uNumOfBitsInBuffer = 0;
}

unsigned int bitstream_get(CBitstream* bitsStream,unsigned int numBits)
{
	unsigned int retData;
	static const unsigned int msk[33] = 
	{
		0x00000000, 0x00000001, 0x00000003, 0x00000007,
		0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
		0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
		0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
		0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
		0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
		0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
		0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
		0xffffffff
	};
	if (numBits <= 32) 
	{  
		if (numBits == 0) 
		{
			return 0;
		}

		if (bitsStream->m_uNumOfBitsInBuffer >= (int)numBits)
		{
			bitsStream->m_uNumOfBitsInBuffer -= numBits;
			retData = bitsStream->m_chDecData >> bitsStream->m_uNumOfBitsInBuffer;
			// wmay - this gets done below...retData &= msk[numBits];
		} 
		else 
		{
			unsigned int nbits;
			nbits = numBits - bitsStream->m_uNumOfBitsInBuffer;
			if (nbits == 32)
			{
				retData = 0;
			}else
			{
				retData = bitsStream->m_chDecData << nbits;
			}
			switch ((nbits - 1) / 8)
			{
			case 3:
				nbits -= 8;
				if (bitsStream->m_chDecBufferSize >= 8)
				{
					retData |= *bitsStream->m_chDecBuffer++ << nbits;
					bitsStream->m_chDecBufferSize -= 8;
				}
			case 2:
				nbits -= 8;
				if (bitsStream->m_chDecBufferSize >= 8)
				{
					retData |= *bitsStream->m_chDecBuffer++ << nbits;
					bitsStream->m_chDecBufferSize -= 8;
				}
			case 1:
				nbits -= 8;
				if (bitsStream->m_chDecBufferSize >= 8)
				{
					retData |= *bitsStream->m_chDecBuffer++ << nbits;
					bitsStream->m_chDecBufferSize -= 8;
				}
			case 0:
				break;
			}
			if (bitsStream->m_chDecBufferSize >= (int)nbits)
			{
				bitsStream->m_chDecData	= *bitsStream->m_chDecBuffer++;
				bitsStream->m_uNumOfBitsInBuffer = MIN(8,bitsStream->m_chDecBufferSize) - nbits;
				bitsStream->m_chDecBufferSize -= MIN(8,bitsStream->m_chDecBufferSize);
				retData |= (bitsStream->m_chDecData >> bitsStream->m_uNumOfBitsInBuffer) & msk[nbits];
			}
		}
	}
	return (retData & msk[numBits]);
}

int bitstream_bits_remain(CBitstream* bitsStream)
{
	return bitsStream->m_chDecBufferSize + bitsStream->m_uNumOfBitsInBuffer;
}

int bitstream_peek(CBitstream* bitsStream,int bits)
{
	int ret;
	int			uNumOfBitsInBuffer	= bitsStream->m_uNumOfBitsInBuffer;
	const unsigned char * chDecBuffer = bitsStream->m_chDecBuffer;
	unsigned char chDecData	= bitsStream->m_chDecData;
	int	chDecBufferSize	= bitsStream->m_chDecBufferSize;

	ret = bitstream_get(bitsStream,bits);
	bitsStream->m_uNumOfBitsInBuffer = uNumOfBitsInBuffer;
	bitsStream->m_chDecBuffer = chDecBuffer;
	bitsStream->m_chDecData	= chDecData;
	bitsStream->m_chDecBufferSize = chDecBufferSize;

	return ret;
}

unsigned int H264NalDecoder(char** sDecoder,int* nDecoder,char* sSource,int nSource)
{
	if(sDecoder != NULL && sSource != NULL && nSource > 0)
	{
		char*	sStart	= sSource;
		char*	sEnd	= (sSource + nSource);
		char*	sTarget	= *sDecoder;
		while(sStart < sEnd)
		{
			if(sStart < (sEnd - 3) && sStart[0] == 0 && sStart[1] == 0 && sStart[2] == 3)
			{
				*sTarget ++	= 0;
				*sTarget ++	= 0;
				sStart ++;
				sStart ++;
				sStart ++;
				continue;
			}
			*sTarget ++	= *sStart ++;
		}
		if(nDecoder != NULL)
		{
			*nDecoder	= (sTarget - *sDecoder);
		}
		return (sTarget - *sDecoder);
	}
	return 0;
}

void H264Skip(CBitstream *bs,int nBits)
{
	bitstream_get(bs,nBits);
}

unsigned int H264Ue (CBitstream *bs)
{
	unsigned int bits, read;
	int bitsLeft;
	unsigned char coded;
	int done = 0;

	bits = 0;

	while (done == 0) 
	{
		bitsLeft = bitstream_bits_remain(bs);
		if (bitsLeft < 8) 
		{
			read = bitstream_peek(bs,bitsLeft) << (8 - bitsLeft);
			done = 1;
		} 
		else 
		{
			read = bitstream_peek(bs,8);
			if (read == 0) 
			{
				bitstream_get(bs,8);
				bits += 8;
			} 
			else 
			{
				done = 1;
			}
		}
	}

	coded = exp_golomb_bits[read];
	bitstream_get(bs,coded);
	bits += coded;

	return bitstream_get(bs,bits + 1) - 1;
}

unsigned int H264Se (CBitstream *bs)
{
	unsigned int ret;
	ret = H264Ue(bs);
	if ((ret & 0x1) == 0) 
	{
		ret >>= 1;
		int temp = 0 - ret;
		return temp;
	} 
	return (ret + 1) >> 1;
}

static void ScalingList (int sizeOfScalingList, CBitstream *bs)
{
	int lastScale = 8, nextScale = 8;
	int j;

	for (j = 0; j < sizeOfScalingList; j++) 
	{
		if (nextScale != 0) 
		{
			int deltaScale = H264Se(bs);
			nextScale = (lastScale + deltaScale + 256) % 256;
		}
		if (nextScale == 0)
		{
			lastScale = lastScale;
		} 
		else 
		{
			lastScale = nextScale;
		}
	}
}

void H264ParseSps(char* sSPS,int nSPS,int* nProfile,int* nLevel,int* nWidth,int* nHeight)
{
	//char*	sSPSDecoder	= (char*)malloc(nSPS);
	//int		nHeader		= (sSPS[2] == 1 ? 4 : 5);
	//int		nSPSDecoder	= 0;
	int		nPicProfile	= 0;
	int		nPicLevel	= 0;
	int		nPicWidth	= 0;
	int		nPicHeight	= 0;
	int		nFrame_mbs_only_flag	= 0;

	//if(H264NalDecoder((char**)&sSPSDecoder,&nSPSDecoder,sSPS,(nSPS)))
	{
		CBitstream	bs;
		int			poctype	= 0;
		int			cycle	= 0;
		int		    idc     = 0;
		int         i       = 0;

		bitstream_init(&bs,(unsigned char*)(sSPS + 1),((nSPS - 1)*8));

		nPicProfile	= bitstream_get(&bs,8);
		bitstream_get(&bs,8);
		nPicLevel		= bitstream_get(&bs,8);
		H264Ue(&bs);
		if(	nPicProfile == 100 || nPicProfile == 110 || 
			nPicProfile == 122 || nPicProfile == 144
			)
		{
			idc	= H264Ue(&bs);
			if(idc == 3)
			{
				bitstream_get(&bs,1);
			}
			H264Ue(&bs);
			H264Ue(&bs);
			bitstream_get(&bs,1);
			if(bitstream_get(&bs,1))
			{
				for(i = 0; i < 8; i ++)
				{
					if(bitstream_get(&bs,1))
					{
						ScalingList(i < 6 ? 16 : 64, &bs);
					}
				}
			}
		}
		H264Ue(&bs);
		poctype	= H264Ue(&bs);
		if(poctype == 0)
		{
			H264Ue(&bs);
		}
		else if(poctype == 1)
		{
			H264Skip(&bs,1);
			H264Se(&bs);
			H264Se(&bs);
			cycle	= H264Ue(&bs);
			if(cycle > 256) cycle = 256;
			while(cycle -- > 0)
			{
				H264Se(&bs);
			}
		}
		H264Ue(&bs);
		H264Skip(&bs,1);
		nPicWidth	= 16 * ( H264Ue(&bs) + 1 );
		nPicHeight	= 16 * ( H264Ue(&bs) + 1 );
		nFrame_mbs_only_flag	= bitstream_get(&bs,1);
		nPicHeight	*= (2 - nFrame_mbs_only_flag);
		if(nPicHeight == 1088) nPicHeight = 1080;
		if(nProfile != NULL)
		{
			*nProfile	= nPicProfile;
		}
		if(nLevel != NULL)
		{
			*nLevel	= nPicLevel;
		}
		if(nWidth != NULL)
		{
			*nWidth		= nPicWidth;
		}
		if(nHeight != NULL)
		{
			*nHeight	= nPicHeight;
		}
	}
	//free(sSPSDecoder);
}*/


RTSPClientSource::RTSPClientSource()
: ZIdleTask("RTSPClientSource")
, m_arrayParser(8)
, m_pSocket(NULL)
, m_pRTSPSession(NULL)
, m_bSessionTimeout(FALSE)
, m_nAlreadyReconnectTimes(0)
, m_nSessionStartTime(0)
, m_nSessionStopTime(0)
, m_bIsPlaying(FALSE)
, m_bFirstPlaySuccess(FALSE)
, m_mutexPlay("MutexPlay")
, m_StatusCallBackFunc(NULL)
, m_pscbfContext(NULL)
, m_RtpCallBackFunc(NULL)
, m_prcbfContext(NULL)
, m_RawDataCallBackFunc(NULL)
, m_prdcbfContext(NULL)
{
    memset(m_Config.sUrl, 0, 1024);
    m_Config.nTransportType = 1;
    m_Config.bSynchronizationMode = TRUE;
    m_Config.nWaitSecond = 10;
    m_Config.bKeepReconnect = TRUE;
    m_Config.bReconnect = TRUE;
    m_Config.nReconnectTryTimes = 0x7FFFFFFF;
    m_Config.nReconnectInterval = 5;
    m_Config.nRRPacketTimeInterval = 3;
    m_Config.bAACRawDataOutputWithAdtsHeader = FALSE;
}

RTSPClientSource::~RTSPClientSource()
{
    DoStop();
}

void RTSPClientSource::SetStatusCallBack(RTSP_CLIENT_StatusCallBackFunc scbf, void* pContext)
{
    m_StatusCallBackFunc = scbf;
    m_pscbfContext = pContext;
}

void RTSPClientSource::SetRtpDataCallBack(RTSP_CLIENT_RtpCallBackFunc rtpcbf, void* pContext)
{
    m_RtpCallBackFunc = rtpcbf;
    m_prcbfContext = pContext;
}

void RTSPClientSource::SetRawDataCallBack(RTSP_CLIENT_RawDataCallBackFunc rdcbf, VOID* pContext)
{
    m_RawDataCallBackFunc = rdcbf;
    m_prdcbfContext = pContext;
}

void RTSPClientSource::GetStatistics(int nStreamNo, RTSP_CLIENT_STATISTICSDATA *pStatistics)
{

    if (pStatistics != NULL)
    {
        if (m_nSessionStopTime == 0)
        {
            pStatistics->nSessionDuration = (UINT)((ZOS::milliseconds()-m_nSessionStartTime)/1000);
        }
        else
        {
            pStatistics->nSessionDuration = (UINT)((m_nSessionStopTime-m_nSessionStartTime)/1000);
        }

        if (nStreamNo < m_arrayParser.Size())
        {
            pStatistics->nTotalPacketCount = m_arrayParser[nStreamNo]->GetTotalPacketCount();
            pStatistics->nTotalLostPacketCount = m_arrayParser[nStreamNo]->GetLostPacketCount();
        }
    }
}

char* RTSPClientSource::GetSDP()
{
    if (m_pRTSPSession != NULL)
    {
        return m_pRTSPSession->GetContent();
    }
    return NULL;
}

int RTSPClientSource::GetStreamCount()
{
    if (m_pRTSPSession != NULL)
    {
        return m_pRTSPSession->GetDataStreamCount();
    }
    else
    {
        return 0;
    }
}

int RTSPClientSource::GetStreamType(int nStreamNo)
{
    if (m_pRTSPSession != NULL)
    {
        return SDP_GetStreamType(m_pRTSPSession->GetContent(), 
            m_pRTSPSession->GetContentLength(), nStreamNo);
    }

    return -1;
}

int RTSPClientSource::GetVideoProperty(int nStreamNo, RTSP_RTSP_VIDEOPROPERTY *pVideoProperty)
{
    int nStreamType;

    if (
        (pVideoProperty != NULL)
        && (nStreamNo < m_arrayParser.Size())
        && (m_pRTSPSession != NULL)
        )
    {
        nStreamType = SDP_GetStreamType(m_pRTSPSession->GetContent(), 
            m_pRTSPSession->GetContentLength(), nStreamNo);
        if (nStreamType == 100)
        {
            pVideoProperty->nEncodeType = 1;
        }
        pVideoProperty->pSPS = NEW char[4096];
        pVideoProperty->nSPS = 4096;
        pVideoProperty->pPPS = NEW char[4096];
        pVideoProperty->nPPS = 4096;

		pVideoProperty->pSPS[0] = 0;
		pVideoProperty->pSPS[1] = 0;
		pVideoProperty->pSPS[2] = 0;
		pVideoProperty->pSPS[3] = 1;

		pVideoProperty->pPPS[0] = 0;
		pVideoProperty->pPPS[1] = 0;
		pVideoProperty->pPPS[2] = 0;
		pVideoProperty->pPPS[3] = 1;

        /*SDP_GetSpsPps(m_pRTSPSession->GetContent(), m_pRTSPSession->GetContentLength(), nStreamNo,
            pVideoProperty->pSPS, &pVideoProperty->nSPS, pVideoProperty->pPPS, &pVideoProperty->nPPS);
        pVideoProperty->nTimeBase = 90000;
        pVideoProperty->nWidth = 0;
        pVideoProperty->nHeight = 0;
		H264ParseSps(pVideoProperty->pSPS,pVideoProperty->nSPS,&pVideoProperty->nProfile,&pVideoProperty->nLevel,&pVideoProperty->nWidth,&pVideoProperty->nHeight);*/
		SDP_GetSpsPps(m_pRTSPSession->GetContent(), m_pRTSPSession->GetContentLength(), nStreamNo,
			pVideoProperty->pSPS + 4, &pVideoProperty->nSPS, pVideoProperty->pPPS + 4, &pVideoProperty->nPPS);
		pVideoProperty->nSPS += 4;
		pVideoProperty->nPPS += 4;
		pVideoProperty->nTimeBase = 90000;
		NALUTIL_ParseSPS(pVideoProperty->pSPS, pVideoProperty->nSPS, 
			&pVideoProperty->nProfile, &pVideoProperty->nLevel,
			&pVideoProperty->nWidth, &pVideoProperty->nHeight);

		pVideoProperty->nBitrate = m_arrayParser[nStreamNo]->GetBitRate();
        pVideoProperty->nFrameRate = m_arrayParser[nStreamNo]->GetFps();

        return 0;
    }

    return -1;
}

int RTSPClientSource::GetAudioProperty(int nStreamNo, RTSP_RTSP_AUDIOPROPERTY *pAudioProperty)
{
    int nStreamType;
	int nRet = FALSE;

    if (pAudioProperty != NULL && m_pRTSPSession != NULL)
    {
        pAudioProperty->nEncodeType = 5;
        nStreamType = SDP_GetStreamType(m_pRTSPSession->GetContent(), m_pRTSPSession->GetContentLength(),
            nStreamNo);
        if (nStreamType == 200)
        {
            pAudioProperty->nPayloadFormatType = 1;
        }
        else if (nStreamType == 201)
        {
            pAudioProperty->nPayloadFormatType = 2;
        }
		else if(nStreamType == 300)
		{
			pAudioProperty->nPayloadFormatType = 3;
		}
		else if(nStreamType == 301)
		{
			pAudioProperty->nPayloadFormatType = 4;
		}
        nRet = SDP_GetAudioProperty(m_pRTSPSession->GetContent(), m_pRTSPSession->GetContentLength(), 
            nStreamNo, &pAudioProperty->nSampleRate, &pAudioProperty->nChannelNo);
		if(nRet == -1)
		{
			if(nStreamType == 300 || nStreamType == 301)
			{
				pAudioProperty->nSampleRate = 8000;
				pAudioProperty->nChannelNo = 1;
			}
		}
        pAudioProperty->nTimeBase = pAudioProperty->nSampleRate;
        pAudioProperty->nBitrate = m_arrayParser[nStreamNo]->GetBitRate();
    }

    return 0;
}

char* RTSPClientSource::GetMediaFromSDP(int nStreamNo, char *sMedia, int *pMediaSize)
{
    if (m_pRTSPSession != NULL)
    {
        SDP_GetMedia(m_pRTSPSession->GetContent(), m_pRTSPSession->GetContentLength(), nStreamNo, sMedia);
        if (pMediaSize != NULL)
        {
            *pMediaSize = strlen(sMedia);
        }
        return sMedia;
    }

    return NULL;
}

void RTSPClientSource::GetConfig(RTSP_CLIENT_CONFIG *pConfig)
{
    if (pConfig != NULL)
    {
        strncpy(pConfig->sUrl, m_Config.sUrl, 1024);
        pConfig->sUrl[1023] = '\0';
        pConfig->nTransportType = m_Config.nTransportType;
        pConfig->bSynchronizationMode = m_Config.bSynchronizationMode;
        pConfig->nWaitSecond = m_Config.nWaitSecond;
        pConfig->bKeepReconnect = m_Config.bKeepReconnect;
        pConfig->bReconnect = m_Config.bReconnect;
        pConfig->nReconnectTryTimes = m_Config.nReconnectTryTimes;
        pConfig->nReconnectInterval = m_Config.nReconnectInterval;
        pConfig->nRRPacketTimeInterval = m_Config.nRRPacketTimeInterval;
        pConfig->bAACRawDataOutputWithAdtsHeader = m_Config.bAACRawDataOutputWithAdtsHeader;
    }
}

int RTSPClientSource::SetConfig(RTSP_CLIENT_CONFIG config)
{
    strncpy(m_Config.sUrl, config.sUrl, 1024);
    m_Config.sUrl[1023] = '\0';
    m_Config.nTransportType = config.nTransportType;
    m_Config.bSynchronizationMode = config.bSynchronizationMode;
    if (config.nWaitSecond <= 0)
    {
        config.nWaitSecond = 10;
    }
    m_Config.nWaitSecond = config.nWaitSecond;
    m_Config.bKeepReconnect = config.bKeepReconnect;
    m_Config.bReconnect = config.bReconnect;
    m_Config.nReconnectTryTimes = config.nReconnectTryTimes;
    m_Config.nReconnectInterval = config.nReconnectInterval;
    m_Config.nRRPacketTimeInterval = config.nRRPacketTimeInterval;
    m_Config.bAACRawDataOutputWithAdtsHeader = config.bAACRawDataOutputWithAdtsHeader;

    return 0;
}

int RTSPClientSource::RTSPPlay()
{
    if (!m_bIsPlaying)
    {
        return DoPlay();
    }
    else
    {
        LOG_DEBUG(("[RTSPClientSource::RTSPPlay] is already playing\r\n"));
        return -1;
    }
}

int RTSPClientSource::RTSPPause()
{
    return 0;
}

int RTSPClientSource::RTSPSeek()
{
    return 0;
}

int RTSPClientSource::RTSPStop()
{
    return DoStop();
}

//////////////////////////////////////////////////////////////////////////
BOOL RTSPClientSource::ParserOnData(int nFlag, void*pData, int nData, long nTimestampSecond, long nTimestampUSecond)
{
    RTSP_CLIENT_RAWDATACBDATA   rdcbd;
    RTSP_CLIENT_RTPCBDATA       rtpcbd;

    if (nFlag >= 100)
    {//rtp packet
        if (m_RtpCallBackFunc != NULL)
        {
            rtpcbd.hRTSPClient = this;
            rtpcbd.nStreamNo = nFlag%100;
            rtpcbd.sData = (char *)pData;
            rtpcbd.nData = nData;
            rtpcbd.pContext = m_prcbfContext;
            m_RtpCallBackFunc(rtpcbd);
        }
    }
    else
    {//raw data packet,h264 or aac
        if (
            (m_RawDataCallBackFunc != NULL)
            && (m_arrayParser[nFlag] != NULL)
            )
        {
            rdcbd.hRTSPClient = this;
            rdcbd.nStreamNo = nFlag;
            rdcbd.nTimestampSecond = nTimestampSecond;
            rdcbd.nTimestampUSecond = nTimestampUSecond;
            rdcbd.sData = (char *)pData;
            rdcbd.nData = nData;
            rdcbd.pContext = m_prdcbfContext;
            m_RawDataCallBackFunc(rdcbd);
        }
    }

    return TRUE;
}

int RTSPClientSource::Run(int nEvent)
{
    int		nTaskTime	= 0;
    UINT	nLocalEvent	= 0;

    nLocalEvent	= GetEvent(nEvent);

    ZTask::Run(nLocalEvent);

    if (nLocalEvent&TASK_UPDATE_EVENT)
    {
        nTaskTime = OnUpdate();
        //LOG_DEBUG(("[RTSPClientSource::Run] nTaskTime %d\r\n", nTaskTime));
    }
    if (nLocalEvent&TASK_IDLE_EVENT)
    {
        nTaskTime = OnIdle();
    }

    return nTaskTime;
}

BOOL RTSPClientSource::OnReceiveResponse_OPTION(int nState)
{
    RTSP_CLIENT_STATUSCBDATA    scbd;

    if (m_StatusCallBackFunc != NULL)
    {
        scbd.hRTSPClient = this;
        scbd.nStatusType = 1;
        scbd.nStatusValue = nState;
        scbd.pContext = m_pscbfContext;
        m_StatusCallBackFunc(scbd);
    }

    return TRUE;
}

BOOL RTSPClientSource::OnReceiveResponse_DESCRIBE(int nState, CHAR *sSDP, int nSDP)
{
    RTSP_CLIENT_STATUSCBDATA    scbd;

    DestroyParser();
    CreateParser(sSDP, nSDP);

    if (m_StatusCallBackFunc != NULL)
    {
        scbd.hRTSPClient = this;
        scbd.nStatusType = 2;
        scbd.nStatusValue = nState;
        scbd.pContext = m_pscbfContext;
        m_StatusCallBackFunc(scbd);
    }

    return TRUE;
}

BOOL RTSPClientSource::OnReceiveResponse_SETUP(int nState, int nStreamNo)
{
    RTSP_CLIENT_STATUSCBDATA    scbd;

    if (m_StatusCallBackFunc != NULL)
    {
        scbd.hRTSPClient = this;
        scbd.nStatusType = 3;
        scbd.nStatusValue = nState;
        scbd.pContext = m_pscbfContext;
        m_StatusCallBackFunc(scbd);
    }

    return TRUE;
}

BOOL RTSPClientSource::OnReceiveResponse_PLAY(int nState)
{
    RTSP_CLIENT_STATUSCBDATA    scbd;
    int i = 0;
    int nStreamNo = 0;

    if (nState == 0)
    {
        for (i = 0; i < m_arrayParser.Count(); i ++)
        {
            if (m_arrayParser[i] != NULL && m_pRTSPSession != NULL)
            {
                 nStreamNo = m_arrayParser[i]->GetParserFlag();
                 m_arrayParser[i]->SetUpperPin((ZDataOut*)m_pRTSPSession->GetDataPin(nStreamNo));
            }
        }
    }

    if (m_StatusCallBackFunc != NULL)
    {
        scbd.hRTSPClient = this;
        scbd.nStatusType = 4;
        scbd.nStatusValue = nState;
        scbd.pContext = m_pscbfContext;
        m_StatusCallBackFunc(scbd);
    }

    return TRUE;
}

BOOL RTSPClientSource::DisconnectWithServer()
{
    RTSP_CLIENT_STATUSCBDATA    scbd;

    if (m_StatusCallBackFunc != NULL)
    {
        scbd.hRTSPClient = this;
        scbd.nStatusType = 101;
        scbd.nStatusValue = 0;
        scbd.pContext = m_pscbfContext;
        m_StatusCallBackFunc(scbd);
    }

    return TRUE;
}

BOOL RTSPClientSource::BeforeReconnect()
{
    RTSP_CLIENT_STATUSCBDATA    scbd;

    if (m_StatusCallBackFunc != NULL)
    {
        scbd.hRTSPClient = this;
        scbd.nStatusType = 102;
        scbd.nStatusValue = 0;
        scbd.pContext = m_pscbfContext;
        m_StatusCallBackFunc(scbd);
    }

    return TRUE;
}

BOOL RTSPClientSource::AfterReconnect(BOOL bReconnectOK)
{
    RTSP_CLIENT_STATUSCBDATA    scbd;

    if (m_StatusCallBackFunc != NULL)
    {
        scbd.hRTSPClient = this;
        if (bReconnectOK)
        {
            scbd.nStatusType = 104;
        }
        else
        {
            scbd.nStatusType = 103;
        }
        scbd.nStatusValue = 0;
        scbd.pContext = m_pscbfContext;
        m_StatusCallBackFunc(scbd);
    }

    return TRUE;
}

BOOL RTSPClientSource::GiveupReconnect()
{
    RTSP_CLIENT_STATUSCBDATA    scbd;

    if (m_StatusCallBackFunc != NULL)
    {
        scbd.hRTSPClient = this;
        scbd.nStatusType = 105;
        scbd.nStatusValue = 0;
        scbd.pContext = m_pscbfContext;
        m_StatusCallBackFunc(scbd);
    }

    return TRUE;
}

BOOL RTSPClientSource::DetectPacketLost(int nLostCount)
{
    RTSP_CLIENT_STATUSCBDATA    scbd;

    if (m_StatusCallBackFunc != NULL)
    {
        scbd.hRTSPClient = this;
        scbd.nStatusType = 201;
        scbd.nStatusValue = nLostCount;
        scbd.pContext = m_pscbfContext;
        m_StatusCallBackFunc(scbd);
    }

    return TRUE;
}

int	RTSPClientSource::OnUpdate()
{
    //LOG_DEBUG(("[RTSPClientSource::OnUpdate] %d\r\n", m_Config.bKeepReconnect));

    if (m_Config.bKeepReconnect)
    {
        if (!m_bFirstPlaySuccess)
        {
            DoFirstPlay();
            return DEFAULT_CHECK_RTSPSTATE_TIME;
        }
    }

    m_bSessionTimeout = FALSE;
    if (m_pRTSPSession != NULL)
    {
        if (m_pRTSPSession->GetSessionState() == SESSION_STATE_TIMEOUT)
        {
            m_bSessionTimeout = TRUE;
            if(m_pSocket != NULL)
            {
                m_pSocket->SetTask(NULL);
            }
            if(m_pRTSPSession != NULL)
            {
                m_pRTSPSession->Close();
            }
            if(m_pSocket != NULL)
            {
                m_pSocket->Close();
            }
        }
    }
    else
    {
        m_bSessionTimeout = TRUE;
    }

    if (m_bSessionTimeout)
    {
        if (m_Config.bReconnect)
        {
            ZIdleTask::AddIdleTask(DEFAULT_CHECK_RTSPSTATE_TIME);
        }
        return 0;
    }
    else
    {
        return DEFAULT_CHECK_RTSPSTATE_TIME;
    }
}

int RTSPClientSource::OnIdle()
{
    //LOG_DEBUG(("[RTSPClientSource::OnIdle] m_Config.nReconnectInterval %d %d\r\n", 
    //    m_Config.nReconnectInterval,
    //    m_Config.nReconnectTryTimes));
    if (m_bSessionTimeout)
    {
        if (m_nAlreadyReconnectTimes < m_Config.nReconnectTryTimes)
        {
            if (RTSPClientSource::DoReconnect())
            {
                m_nAlreadyReconnectTimes = 0;
                return DEFAULT_CHECK_RTSPSTATE_TIME;
            }
            else
            {
                m_nAlreadyReconnectTimes ++;
                ZIdleTask::AddIdleTask(DEFAULT_CHECK_RTSPSTATE_TIME*m_Config.nReconnectInterval);
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return DEFAULT_CHECK_RTSPSTATE_TIME;
    }
}

int RTSPClientSource::DoPlay()
{
    BOOL				bReturn		= FALSE;
    char*				sProtocol	= NULL;
    char*				sHost		= NULL;
    char*				sPort		= NULL;
    char*				sUser		= NULL;
    char*				sPass		= NULL;
    char*				sPath		= NULL;
    int					nAddr		= 0;
    int					nPort		= 554;
    char                sLocalUrl[1024];
	char                sUserStr[32];
	char                sPassStr[32];
    RTSP_PROTOCOL_TRANSPORT eProtocolTransport = RTSP_PROTOCOL_TRANSPORT_UDP;
    UINT64              nStartTimeStamp = 0;

    ZOSMutexLocker      locker(&m_mutexPlay);

    do{

        ZIdleTask::Create();

        // url process
        strncpy(sLocalUrl,m_Config.sUrl,1024);
        sLocalUrl[1023] = '\0';
        if(!ZOS::DecodeURL(sLocalUrl,&sProtocol,&sHost,&sPort,&sUser,&sPass,&sPath))
        {
            break;
        }
        if(
            (sProtocol == NULL)
            || (sHost == NULL)
            || (strcasecmp(sProtocol,"RTSP")!=0)
            )
        {
            break;
        }

        // tcp socket process
        nAddr = ZSocket::ConvertAddr(sHost);
        if(sPort != NULL)
        {
            nPort = atoi(sPort);
        }
		if(sUser != NULL)
		{
			strcpy(sUserStr,sUser);
			//sUserStr[63] = '\0';
			sUser = sUserStr;
		}
		if(sPass != NULL)
		{
			strcpy(sPassStr,sPass);
			//sPassStr[63] = '\0';
			sPass = sPassStr;
		}

        m_pSocket	= NEW ZTCPSocket();
        if(m_pSocket == NULL)
        {
            break;
        }
        if(!m_pSocket->Create())
        {
            break;
        }
        m_pSocket->SetRecvBufferSize(DEFAULT_TCP_RECV_BUFFER_SIZE);
        m_pSocket->SetNonBlocking(TRUE);
        if(!m_pSocket->Connect(nAddr,nPort))
        {
        }
        if (m_Config.bSynchronizationMode)
        {
            if(!m_pSocket->ConnectAble(m_Config.nWaitSecond*1000*1000))
            {
                break;
            }
        }
        else
        {
            if(!m_pSocket->ConnectAble(10*1000*1000))
            {
                break;
            }
        }
        //m_pSocket->SetRecvBufferSize(DEFAULT_TCP_RECV_BUFFER_SIZE);
        m_pSocket->SetTimeOut(DEFAULT_TCP_SEND_TIMEOUT,DEFAULT_TCP_RECV_TIMEOUT*10);

        // rtspsession process
        if (m_Config.nTransportType == 1)
        {
            eProtocolTransport = RTSP_PROTOCOL_TRANSPORT_UDP;
        }
        else if (m_Config.nTransportType == 2)
        {
            eProtocolTransport = RTSP_PROTOCOL_TRANSPORT_TCP;
        }
        ZOS::URLUserClear(m_Config.sUrl, sLocalUrl, 1024);
        m_pRTSPSession	= (ZRTSPSession *)ZRTSPSession::CreateSession(SESSION_TYPE_CLIENT,m_pSocket,
            (char*)sLocalUrl,eProtocolTransport,sUser,sPass);
        if(m_pRTSPSession == NULL)
        {
            break;
        }
        m_pRTSPSession->SetRTSPClientListener(this);
        m_pRTSPSession->SetSessionTimeoutMilliSecond(5*1000);

        // wait for rtsp session success.
        if (m_Config.bSynchronizationMode)
        {
            nStartTimeStamp = ZOS::milliseconds();
            while((CHNSYS_INT)(ZOS::milliseconds()-nStartTimeStamp)<(m_Config.nWaitSecond*1000))
            {
                if (m_pRTSPSession == NULL)
                {
                    LOG_DEBUG(("[RTSPClientSource::DoPlay] m_pRTSPSession == NULL\r\n"));
                    bReturn = FALSE;
                    break;
                }
                if(m_pRTSPSession->GetSessionState() == SESSION_STATE_PLAY)
                {
                    bReturn	= TRUE;
                    break;
                }
                if(m_pRTSPSession->GetSessionState() == SESSION_STATE_ERROR)
                {
                    bReturn	= FALSE;
                    break;
                }
                if(m_pRTSPSession->GetSessionState() == SESSION_STATE_TIMEOUT)
                {
                    bReturn	= FALSE;
                    break;
                }
                ZOSThread::Sleep(30);
            }
        }
        else
        {
            bReturn = TRUE;
        }
    }while(FALSE);

    if (!bReturn)
    {
        if(m_pSocket != NULL)
        {
            m_pSocket->SetTask(NULL);
        }

        if (m_pRTSPSession != NULL)
        {
            m_pRTSPSession->Close();
            SAFE_DELETE(m_pRTSPSession);
        }

        if (m_pSocket != NULL)
        {
            m_pSocket->Close();
            SAFE_DELETE(m_pSocket);
        }

        m_bIsPlaying = FALSE;
        m_bFirstPlaySuccess = FALSE;
        //LOG_DEBUG(("[RTSPClientSource::DoPlay] set m_bFirstPlaySuccess FALSE\r\n"));

        //LOG_DEBUG(("m_Config.bKeepReconnect %d \r\n", m_Config.bKeepReconnect));
        if (
            m_Config.bKeepReconnect 
            && (strcmp(m_Config.sUrl, "")!=0)
            )
        {
            //LOG_DEBUG(("[RTSPClientSource::DoPlay] do keep reconnect\r\n"));
            //do this, so if first time is not connected, always keep try
            ZIdleTask::AddEvent(ZTask::TASK_UPDATE_EVENT);
        }

        return -1;
    }
    else
    {
        m_bIsPlaying = TRUE;
        m_bFirstPlaySuccess = TRUE;
        ZIdleTask::AddEvent(ZTask::TASK_UPDATE_EVENT);
        return 0;
    }
}

int RTSPClientSource::DoFirstPlay()
{
    BOOL				bReturn		= FALSE;
    char*				sProtocol	= NULL;
    char*				sHost		= NULL;
    char*				sPort		= NULL;
    char*				sUser		= NULL;
    char*				sPass		= NULL;
    char*				sPath		= NULL;
    int					nAddr		= 0;
    int					nPort		= 554;
    char                sLocalUrl[1024];
    char                sUserStr[32];
    char                sPassStr[32];
    RTSP_PROTOCOL_TRANSPORT eProtocolTransport = RTSP_PROTOCOL_TRANSPORT_UDP;
    UINT64              nStartTimeStamp = 0;

    ZOSMutexLocker  locker(&m_mutexPlay);
    //LOG_DEBUG(("[RTSPClientSource::DoFirstPlay] url %s\r\n", m_Config.sUrl));
    do{

        // url process
        strncpy(sLocalUrl,m_Config.sUrl,1024);
        sLocalUrl[1023] = '\0';
        if(!ZOS::DecodeURL(sLocalUrl,&sProtocol,&sHost,&sPort,&sUser,&sPass,&sPath))
        {
            break;
        }
        if(
            (sProtocol == NULL)
            || (sHost == NULL)
            || (strcasecmp(sProtocol,"RTSP")!=0)
            )
        {
            //LOG_DEBUG(("[RTSPClientSource::DoFirstPlay] sHost == NULL\r\n"));
            break;
        }

        //LOG_DEBUG(("[RTSPClientSource::DoFirstPlay] before convert\r\n"));
        // tcp socket process
        nAddr = ZSocket::ConvertAddr(sHost);
        if(sPort != NULL)
        {
            nPort = atoi(sPort);
        }
        if(sUser != NULL)
        {
            strcpy(sUserStr,sUser);
            //sUserStr[63] = '\0';
            sUser = sUserStr;
        }
        if(sPass != NULL)
        {
            strcpy(sPassStr,sPass);
            //sPassStr[63] = '\0';
            sPass = sPassStr;
        }

        m_pSocket	= NEW ZTCPSocket();
        if(m_pSocket == NULL)
        {
            break;
        }
        if(!m_pSocket->Create())
        {
            break;
        }

        //LOG_DEBUG(("[RTSPClientSource::DoFirstPlay] before connect\r\n"));
        m_pSocket->SetRecvBufferSize(DEFAULT_TCP_RECV_BUFFER_SIZE);
        m_pSocket->SetNonBlocking(TRUE);
        if(!m_pSocket->Connect(nAddr,nPort))
        {
        }
        if (m_Config.bSynchronizationMode)
        {
            if(!m_pSocket->ConnectAble(m_Config.nWaitSecond*1000*1000))
            {
                break;
            }
        }
        else
        {
            if(!m_pSocket->ConnectAble(10*1000*1000))
            {
                break;
            }
        }
        //m_pSocket->SetRecvBufferSize(DEFAULT_TCP_RECV_BUFFER_SIZE);
        m_pSocket->SetTimeOut(DEFAULT_TCP_SEND_TIMEOUT,DEFAULT_TCP_RECV_TIMEOUT*10);

        // rtspsession process
        if (m_Config.nTransportType == 1)
        {
            eProtocolTransport = RTSP_PROTOCOL_TRANSPORT_UDP;
        }
        else if (m_Config.nTransportType == 2)
        {
            eProtocolTransport = RTSP_PROTOCOL_TRANSPORT_TCP;
        }
        ZOS::URLUserClear(m_Config.sUrl, sLocalUrl, 1024);
        m_pRTSPSession	= (ZRTSPSession *)ZRTSPSession::CreateSession(SESSION_TYPE_CLIENT,m_pSocket,
            (char*)sLocalUrl,eProtocolTransport,sUser,sPass);
        if(m_pRTSPSession == NULL)
        {
            break;
        }
        m_pRTSPSession->SetRTSPClientListener(this);
        m_pRTSPSession->SetSessionTimeoutMilliSecond(5*1000);

        // wait for rtsp session success.
        if (m_Config.bSynchronizationMode)
        {
            nStartTimeStamp = ZOS::milliseconds();
            while((CHNSYS_INT)(ZOS::milliseconds()-nStartTimeStamp)<(m_Config.nWaitSecond*1000))
            {
                if (m_pRTSPSession == NULL)
                {
                    bReturn = FALSE;
                    LOG_DEBUG(("[RTSPClientSource::DoFirstPlay] m_pRTSPSession == NULL\r\n"));
                    break;
                }

                if(m_pRTSPSession->GetSessionState() == SESSION_STATE_PLAY)
                {
                    bReturn	= TRUE;
                    break;
                }
                if(m_pRTSPSession->GetSessionState() == SESSION_STATE_ERROR)
                {
                    bReturn	= FALSE;
                    break;
                }
                if(m_pRTSPSession->GetSessionState() == SESSION_STATE_TIMEOUT)
                {
                    bReturn	= FALSE;
                    break;
                }
                ZOSThread::Sleep(30);
            }
        }
        else
        {
            bReturn = TRUE;
        }
    }while(FALSE);

    if (!bReturn)
    {
        if(m_pSocket != NULL)
        {
            m_pSocket->SetTask(NULL);
        }

        //LOG_DEBUG(("[DoFirstPlay] before close rtsp session\r\n"));
        if (m_pRTSPSession != NULL)
        {
            m_pRTSPSession->Close();
            SAFE_DELETE(m_pRTSPSession);
        }
        //LOG_DEBUG(("[DoFirstPlay] after close rtsp session\r\n"));

        if (m_pSocket != NULL)
        {
            m_pSocket->Close();
            SAFE_DELETE(m_pSocket);
        }

        m_bIsPlaying = FALSE;
        m_bFirstPlaySuccess = FALSE;

        return -1;
    }
    else
    {
        m_bIsPlaying = TRUE;
        m_bFirstPlaySuccess = TRUE;
        ZIdleTask::AddEvent(ZTask::TASK_UPDATE_EVENT);
        return 0;
    }
}

int RTSPClientSource::DoStop()
{
    ZOSMutexLocker  locker(&m_mutexPlay);
    

    //if (m_bIsPlaying)
    {
        ZIdleTask::Close();
        if (m_pSocket != NULL)
        {
            m_pSocket->SetTask(NULL);
        }
        if (m_pRTSPSession != NULL)
        {
            m_pRTSPSession->Close();
        }
        SAFE_DELETE(m_pRTSPSession);

        if (m_pSocket != NULL)
        {
            m_pSocket->Close();
        }
        SAFE_DELETE(m_pSocket);

        DestroyParser();

        m_bSessionTimeout = FALSE;

        m_bIsPlaying = FALSE;
        m_bFirstPlaySuccess = FALSE;
    }

    return 0;
}

BOOL RTSPClientSource::DoReconnect()
{
    BOOL				bReturn		= FALSE;
    char*				sProtocol	= NULL;
    char*				sHost		= NULL;
    char*				sPort		= NULL;
    char*				sUser		= NULL;
    char*				sPass		= NULL;
    char*				sPath		= NULL;
    int					nAddr		= 0;
    int					nPort		= 554;
    char				sLocalSource[1024];
    char                sUserStr[32];
    char                sPassStr[32];
    RTSP_PROTOCOL_TRANSPORT eProtocolTransport = RTSP_PROTOCOL_TRANSPORT_UDP;

    ZOSMutexLocker  locker(&m_mutexPlay);

    //LOG_DEBUG(("[RTSPClientSource::DoReconnect] start reconnect %s\r\n", m_Config.sUrl));
    do 
    {
        strncpy(sLocalSource,m_Config.sUrl,1024);
        if(!ZOS::DecodeURL(sLocalSource,&sProtocol,&sHost,&sPort,&sUser,&sPass,&sPath))
        {
            break;
        }
        if(
            (sProtocol == NULL)
            || (sHost == NULL)
            || (strcasecmp(sProtocol,"RTSP")!=0)
            )
        {
            break;
        }
        nAddr	= ZSocket::ConvertAddr(sHost);
        if(sPort != NULL)
        {
            nPort = atoi(sPort);
        }
        if(sUser != NULL)
        {
            strcpy(sUserStr,sUser);
            //sUserStr[63] = '\0';
            sUser = sUserStr;
        }
        if(sPass != NULL)
        {
            strcpy(sPassStr,sPass);
            //sPassStr[63] = '\0';
            sPass = sPassStr;
        }

        if(
            (m_pSocket == NULL)
            || (m_pRTSPSession == NULL)
            )
        {
            break;
        }
        if (m_pSocket->IsConnected())
        {
            bReturn = TRUE;
            break;
        }
        if(!m_pSocket->Create())
        {
            break;
        }
        m_pSocket->SetRecvBufferSize(DEFAULT_TCP_RECV_BUFFER_SIZE);
        m_pSocket->SetNonBlocking(TRUE);
        if(!m_pSocket->Connect(nAddr,nPort))
        {
        }
        if(!m_pSocket->ConnectAble(m_Config.nWaitSecond*1000*1000))
        {
            m_pSocket->Close();
            break;
        }
        //m_pSocket->SetRecvBufferSize(DEFAULT_TCP_RECV_BUFFER_SIZE);
        m_pSocket->SetTimeOut(DEFAULT_TCP_SEND_TIMEOUT,DEFAULT_TCP_RECV_TIMEOUT*10);

        // rtspsession process
        if (m_Config.nTransportType == 2)
        {
            eProtocolTransport = RTSP_PROTOCOL_TRANSPORT_TCP;
        }
        //LOG_DEBUG(("[RTSPClientSource::DoReconnect] sUrl %s %s %s\r\n", 
        //    m_Config.sUrl, sUser, sPass));
        ZOS::URLUserClear(m_Config.sUrl, sLocalSource, 1024);
        m_pSocket->SetTask(m_pRTSPSession);
        m_pRTSPSession->SetSessionStream(m_pSocket);
        m_pRTSPSession->SetSessionURI(sLocalSource);
        m_pRTSPSession->SetProtocolTransportType(eProtocolTransport);
        m_pRTSPSession->SetSessionUserPassword(sUser,sPass);
        if(!m_pRTSPSession->Create())
        {
            m_pSocket->Close();
            break;
        }
        bReturn = TRUE;

    } while (FALSE);

    if (bReturn)
    {
        m_bSessionTimeout = FALSE;
    }
    else
    {
        m_bSessionTimeout = TRUE;
    }

    return bReturn;
}

BOOL RTSPClientSource::CreateParser(CHAR *sSDP, int nSDP)
{
    int i = 0;
    int nStreamCount = 0;
    int nStreamType;
    RTPParser *pParser = NULL;
    int nSampleRate = 0;
    int nChannel = 0;

    if (
        (sSDP != NULL)
        && (strlen(sSDP) > 0)
        )
    {
        nStreamCount = SDP_GetStreamCount(sSDP, nSDP);
        for (i = 0; i < nStreamCount; i ++)
        {
            pParser = NULL;
            nStreamType = SDP_GetStreamType(sSDP, nSDP, i);
            if (nStreamType == 100)
            {//h264
                pParser = NEW Parser3984;
				pParser->SetStreamType(TRUE);
            }
            else if (nStreamType == 200)
            {//aac-generic
                SDP_GetAudioProperty(sSDP, nSDP, i, &nSampleRate, &nChannel);
                pParser = NEW Parser3640;
				pParser->SetStreamType(FALSE);
                ((Parser3640*)pParser)->SetAdtsHeaderParameter(1, nSampleRate, nChannel, m_Config.bAACRawDataOutputWithAdtsHeader);
            }
            else if (nStreamType == 201)
            {//aac-latm
                SDP_GetAudioProperty(sSDP, nSDP, i, &nSampleRate, &nChannel);
                pParser =  NEW Parser3016;
				pParser->SetStreamType(FALSE);
                ((Parser3016*)pParser)->SetAdtsHeaderParameter(1, nSampleRate, nChannel, m_Config.bAACRawDataOutputWithAdtsHeader);
            }
            else if (nStreamType == 300)
            {//g711u
                pParser = NEW ParserG711;
				pParser->SetStreamType(FALSE);
            }
            else if (nStreamType == 301)
            {//g711a
                pParser = NEW ParserG711;
				pParser->SetStreamType(FALSE);
            }
            if (pParser != NULL)
            {
                pParser->SetParserListener(this);
                pParser->SetParserFlag(i);
                pParser->Start();
                m_arrayParser.Add(pParser);
            }
        }
        if (m_arrayParser.Count() > 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL RTSPClientSource::DestroyParser()
{
    int i = 0;
    RTPParser *pParser = NULL;

    for (i = 0; i < m_arrayParser.Count(); i ++)
    {
        if (m_arrayParser[i] != NULL)
        {
            pParser = m_arrayParser[i];
            pParser->Stop();
            SAFE_DELETE(pParser);
        }
    }
    m_arrayParser.RemoveAll();

    return TRUE;
}
