#ifndef _CHNSYSRTSP_H_
#define _CHNSYSRTSP_H_

#include "ChnsysTypes.h"

#ifdef _WIN32_

#ifdef RTSP_DLL
#define RTSP_IN_EXPORT _declspec(dllexport)
#elif defined RTSP_LIB
#define RTSP_IN_EXPORT 
#else
#define RTSP_IN_EXPORT _declspec(dllimport)
#endif//RTSP_DLL

#else //_WIN32_
#define RTSP_IN_EXPORT 
#endif//_WIN32_

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

typedef VOID *RTSP_CLIENT_HANDLE;
typedef VOID *RTSP_SERVER_HANDLE;

typedef struct _RTSP_RTSP_CONIFG_
{
    CHNSYS_INT      nRecvFreq;
    CHNSYS_INT      nRecvCount;
    CHNSYS_INT      nSendFreq;
    CHNSYS_INT      nSendCount;
    CHNSYS_INT      nUDPPairCount;
    CHNSYS_INT      nPacketInterval;
    CHNSYS_INT      nFilterPacketSize;
}RTSP_RTSP_CONFIG;

/**
 *
 * @
 * @
 * @
 */
RTSP_IN_EXPORT CHNSYS_INT RTSP_RTSP_GetConifg(RTSP_RTSP_CONFIG *pConfig);

/**
 *
 * @
 * @
 * @
 */
RTSP_IN_EXPORT VOID RTSP_RTSP_SetConfig(RTSP_RTSP_CONFIG config);

/**
 * called only once.
 */
RTSP_IN_EXPORT VOID RTSP_RTSP_Init(CONST CHNSYS_CHAR *sLogPath, int nThreadCount);

/**
 * should be called before the program exit.
 */
RTSP_IN_EXPORT VOID RTSP_RTSP_Uninit();

RTSP_IN_EXPORT VOID RTSP_SERVER_Init();

RTSP_IN_EXPORT VOID RTSP_SERVER_Uninit();

/**
 * a struct used in the status callback function
 *
 * @hRTSPClient
 * used to identify the rtsp client instance
 * @nStatusType
 * 1 OPTION response
 * 2 DESCRIBE
 * 3 SETUP
 * 4 PLAY
 * 101 disconnect with server
 * 102 start reconnect
 * 103 reconnect failed
 * 104 reconnect success
 * 105 try n times, give up reconnect
 * 201 lost packet
 * @nStatusValue
 * 0 OK
 * -1 ERROR
 * other value determined by nStatusType
 * when nStatusType = 201, nStatuaValue = current lost packet count
 * @pContext
 * value set by RTSP_CLIENT_SetStatusCallBack function
 */
typedef struct _RTSP_CLIENT_STATUSCBDATA_
{
    RTSP_CLIENT_HANDLE  hRTSPClient;
    CHNSYS_INT          nStatusType;
    CHNSYS_INT          nStatusValue;
    VOID                *pContext;
}RTSP_CLIENT_STATUSCBDATA;


/**
 * a struct used to describe statistics infomation of the rtsp client.
 *
 * @nTotalPacketCount
 * the total packet received during the rtsp session.
 * @nTotalLostPacketCount
 * the total packet lost during the rtsp session.
 * @nSessionDuration
 * the total second of the rtsp session
 */
typedef struct _RTSP_CLIENT_STATISTICSDATA_
{
    CHNSYS_UINT64         nTotalPacketCount;
    CHNSYS_UINT64         nTotalLostPacketCount;
    CHNSYS_UINT           nSessionDuration;
}RTSP_CLIENT_STATISTICSDATA;

/**
 * a struct used in the rtp data callback function
 *
 * @hRTSPClient
 * used to identify the rtsp client instance
 * @nStreamNo
 * the index of the stream. start from 0.
 * @sData
 * rtp data,  must be copied for latter use.
 * @nData
 * rtp data length
 * @pContext
 * value set by RTSP_CLIENT_SetRtpDataCallBack function
 */
typedef struct _RTSP_CLIENT_RTPCBDATA_
{
    RTSP_CLIENT_HANDLE  hRTSPClient;
    CHNSYS_INT          nStreamNo;
    CHNSYS_CHAR         *sData;
    CHNSYS_INT          nData;
    VOID                *pContext;
}RTSP_CLIENT_RTPCBDATA;

/**
 * a struct used in the raw data callback function
 * 
 * @nRTSPClient
 * used to identify the rtsp client instance
 * @nStreamNo
 * the index of the stream. start from 0.
 * @nTimestamp
 * the sData's timestamp 
 * @sData
 * raw data, must be copied for latter use.
 * @nData
 * raw data length.
 * @pContext
 * value set by RTSP_CLIENT_RawDataCallBackFunc function
 */
typedef struct _RTSP_CLIENT_RAWDATACBDATA_
{
    RTSP_CLIENT_HANDLE  hRTSPClient;
    CHNSYS_INT          nStreamNo;
    CHNSYS_LONG         nTimestampSecond;/*seconds part*/
    CHNSYS_LONG         nTimestampUSecond;/*useconds part*/
    CHNSYS_CHAR         *sData;
    CHNSYS_INT          nData;
    VOID                *pContext;
}RTSP_CLIENT_RAWDATACBDATA;

/**
 * a struct used to describe video property.
 *
 * @nEncodeType
 * 1 h264
 * @pSPS
 * when the nEncodeType is h264, pSPS point to the sequence parameter set
 * @nSPS
 * the length of pSPS
 * @pPPS
 * when the nEncodeType is h264, pPPS point to the picture parameter set
 * @nPPS
 * the length of pPPS
 * @nTimeBase
 * the time base of the video stream
 * @nWidth
 * @nHeight
 * @nBitrate
 * the average bitrate of the video stream
 * @nFrameRate
 */
typedef struct _RTSP_RTSP_VIDEOPROPERTY_
{
    CHNSYS_INT      nEncodeType;
    CHNSYS_CHAR     *pSPS;
    CHNSYS_INT      nSPS;
    CHNSYS_CHAR     *pPPS;
    CHNSYS_INT      nPPS;
    CHNSYS_INT      nTimeBase;
    CHNSYS_INT      nWidth;
    CHNSYS_INT      nHeight;
    CHNSYS_INT      nBitrate;
    CHNSYS_INT      nFrameRate;
    CHNSYS_INT      nProfile;
    CHNSYS_INT      nLevel;
}RTSP_RTSP_VIDEOPROPERTY;

/**
 * a struct used to describe audio property.
 *
 * @nEncodeType
 * 1 AAC
 * 2 G711U
 * 3 G711A
 * @nPayloadFormatType
 * 1 mpeg4-generic
 * 2 mp4a-latm
 * @nTimeBase
 * the time base of the audio stream
 * @nSampleRate
 * sample rate 
 * @nChannelNo
 * channel number
 * @nBitrate
 * the average bitrate of the audio stream
 */
typedef struct _RTSP_RTSP_AUDIOPROPERTY_
{
    CHNSYS_INT      nEncodeType;
    CHNSYS_INT      nPayloadFormatType;
    CHNSYS_INT      nTimeBase;
    CHNSYS_INT      nSampleRate;
    CHNSYS_INT      nChannelNo;
    CHNSYS_INT      nBitrate;
}RTSP_RTSP_AUDIOPROPERTY;

/**
 * a struct used to describe client config infomation.
 * 
 * @sUrl
 * the rtsp format url string. should be '\0' end.
 * @nTransportType
 * 1 RTP/AVP
 * 2 RTP/AVP/TCP
 * @bSynchronizationMode
 * if TRUE the RTSP_CLIENT_Play function will return after the rtsp session communicate ends
 * or the nWaitSecond is timeout.
 * if FALSE the RTSP_CLIENT_Play function will return immediately.the user should check the 
 * RTSP_CLIENT_StatusCallBackFunc to check whether the rtsp communication is success or fail.
 * @nWaitSecond
 * if bSynchronizationMode is TRUE, it's used for timeout.
 * @bReconnect
 * if TRUE. the rtsp client will try to reconnect the rtsp server after the rtsp session is broken
 * by abnormal network. bReconnect is valid on condition that preview rtsp session is successful.
 * if FASLE. the rtsp client will not try to reconnect the rtsp server.
 * @nReconnectTryTimes
 * reconncet try times
 * @nReconnectInterval
 * interval second for reconnect
 * @nRRPacketTimeInterval
 * interval second for send RTCP Receive Report Packet
 */
typedef struct _RTSP_CLIENT_CONFIG_
{
    CHNSYS_CHAR     sUrl[1024];
    CHNSYS_INT      nTransportType;
    CHNSYS_BOOL     bSynchronizationMode;
    CHNSYS_INT      nWaitSecond;
    CHNSYS_BOOL     bReconnect;
    CHNSYS_INT      nReconnectTryTimes;
    CHNSYS_INT      nReconnectInterval;
    CHNSYS_INT      nRRPacketTimeInterval;
    CHNSYS_BOOL     bAACRawDataOutputWithAdtsHeader;
}RTSP_CLIENT_CONFIG;

RTSP_IN_EXPORT RTSP_CLIENT_HANDLE RTSP_CLIENT_CreateInstance();
RTSP_IN_EXPORT VOID RTSP_CLIENT_DestroyInstance(RTSP_CLIENT_HANDLE hRTSPClient);

typedef int (*RTSP_CLIENT_StatusCallBackFunc)(RTSP_CLIENT_STATUSCBDATA scbd);
typedef int (*RTSP_CLIENT_RtpCallBackFunc)(RTSP_CLIENT_RTPCBDATA rcbd);
typedef int (*RTSP_CLIENT_RawDataCallBackFunc)(RTSP_CLIENT_RAWDATACBDATA rdcbd);

RTSP_IN_EXPORT VOID RTSP_CLIENT_SetStatusCallBack(RTSP_CLIENT_HANDLE hRTSPClient, RTSP_CLIENT_StatusCallBackFunc scbf, VOID* pContext);
RTSP_IN_EXPORT VOID RTSP_CLIENT_SetRtpDataCallBack(RTSP_CLIENT_HANDLE hRTSPClient, RTSP_CLIENT_RtpCallBackFunc rtpcbf, VOID* pContext);
RTSP_IN_EXPORT VOID RTSP_CLIENT_SetRawDataCallBack(RTSP_CLIENT_HANDLE hRTSPClient, RTSP_CLIENT_RawDataCallBackFunc rdcbf, VOID* pContext);

RTSP_IN_EXPORT CHNSYS_INT RTSP_CLIENT_GetStatistics(RTSP_CLIENT_HANDLE hRTSPClient, CHNSYS_INT nStreamNo, RTSP_CLIENT_STATISTICSDATA *pStatistics);

/**
 *
 * @
 * @
 * @
 */
RTSP_IN_EXPORT CHNSYS_CHAR *RTSP_CLIENT_GetSDP(RTSP_CLIENT_HANDLE hRTSPClient);

RTSP_IN_EXPORT CHNSYS_INT RTSP_CLIENT_GetStreamCount(RTSP_CLIENT_HANDLE hRTSPClient);
RTSP_IN_EXPORT CHNSYS_INT RTSP_CLIENT_GetStreamType(RTSP_CLIENT_HANDLE hRTSPClient, CHNSYS_INT nStreamNo);

RTSP_IN_EXPORT CHNSYS_INT RTSP_CLIENT_GetVideoProperty(RTSP_CLIENT_HANDLE hRTSPClient, CHNSYS_INT nStreamNo, RTSP_RTSP_VIDEOPROPERTY *pVideoProperty);
RTSP_IN_EXPORT VOID RTSP_RTSP_FreeVideoProperty(RTSP_RTSP_VIDEOPROPERTY *pVideoProperty);

RTSP_IN_EXPORT CHNSYS_INT RTSP_CLIENT_GetAudioProperty(RTSP_CLIENT_HANDLE hRTSPClient, CHNSYS_INT nStreamNo, RTSP_RTSP_AUDIOPROPERTY *pAudioProperty);

/**
 * return m= value
 * @
 * @
 * @
 */
RTSP_IN_EXPORT CONST CHNSYS_CHAR *RTSP_CLIENT_GetMediaFromSDP(RTSP_CLIENT_HANDLE hRTSPClient, CHNSYS_INT nStreamNo, 
                                               CHNSYS_CHAR *sMedia, CHNSYS_INT *pMediaSize);

RTSP_IN_EXPORT VOID RTSP_CLIENT_GetConfig(RTSP_CLIENT_HANDLE hRTSPClient, RTSP_CLIENT_CONFIG *pConfig);
RTSP_IN_EXPORT CHNSYS_INT RTSP_CLIENT_SetConfig(RTSP_CLIENT_HANDLE hRTSPClient, RTSP_CLIENT_CONFIG config);

//
RTSP_IN_EXPORT CHNSYS_INT RTSP_CLIENT_Play(RTSP_CLIENT_HANDLE hRTSPClient);
RTSP_IN_EXPORT CHNSYS_INT RTSP_CLIENT_Pause(RTSP_CLIENT_HANDLE hRTSPClient);
RTSP_IN_EXPORT CHNSYS_INT RTSP_CLIENT_Seek(RTSP_CLIENT_HANDLE hRTSPClient);
RTSP_IN_EXPORT CHNSYS_INT RTSP_CLIENT_Stop(RTSP_CLIENT_HANDLE hRTSPClient);


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// specific use:
// two stream; video is h264; audio is aac; use video and audio raw data
//////////////////////////////////////////////////////////////////////////
//VOID RTSP_SERVER_Init();
//VOID RTSP_SERVER_Uninit();

RTSP_IN_EXPORT RTSP_SERVER_HANDLE RTSP_SERVER_CreateInstance();
RTSP_IN_EXPORT VOID RTSP_SERVER_DestroyInstance(RTSP_SERVER_HANDLE hRTSPServer);

/**
 *
 * @
 * @nSendType
 * 1 unicast
 * 2 multicast
 * @nAudioEncodeType
 * 1 AAC
 * 2 G711U
 * 3 G711A
 * @nProfileLevelID
 * 14496-1 audioProfileLevelIndication
 * @nMode
 * 1 generic
 * 2 CELP-cbr
 * 3 CELP-vbr
 * 4 AAC-lbr
 * 5 AAC-hbr
 * @nPayloadFormatType
 * 1 mpeg4-generic
 * 2 mp4a-latm
 */
typedef struct _RTSP_SERVER_CONFIG_
{
    CHNSYS_INT      nSendType;
    CHNSYS_CHAR     sMulticastIP[16];
    CHNSYS_INT      nMulticastPort;
    CHNSYS_INT      nAudioEncodeType;
    CHNSYS_INT      nSampleRate;
    CHNSYS_INT      nChannelNo;
    CHNSYS_INT      nProfileLevelID;
    CHNSYS_INT      nMode;
    CHNSYS_INT      nPayloadFormatType;
    CHNSYS_CHAR     *sSPS;
    CHNSYS_INT      nSPS;
    CHNSYS_CHAR     *sPPS;
    CHNSYS_INT      nPPS;
    CHNSYS_INT      nPacketizationMode;
    CHNSYS_INT      nSRPacketTimeInterval;
    CHNSYS_INT      nRRWaitThreshold;
    CHNSYS_CHAR     sUserName[16];
    CHNSYS_CHAR     sPassword[16];
}RTSP_SERVER_CONFIG;

RTSP_IN_EXPORT VOID RTSP_SERVER_SetConfig(RTSP_SERVER_HANDLE hRTSPServer, RTSP_SERVER_CONFIG config);
RTSP_IN_EXPORT CHNSYS_INT RTSP_SERVER_GetConfig(RTSP_SERVER_HANDLE hRTSPServer, RTSP_SERVER_CONFIG *pConfig);

RTSP_IN_EXPORT CHNSYS_INT RTSP_SERVER_Start(RTSP_SERVER_HANDLE hRTSPServer);
RTSP_IN_EXPORT CHNSYS_INT RTSP_SERVER_Stop(RTSP_SERVER_HANDLE hRTSPServer);

//参数的输入输出属性描述清楚
//

// sData = a NAL or multi NALs division by 00 00 00 01 
RTSP_IN_EXPORT CHNSYS_INT RTSP_SERVER_PushH264Data(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_UINT nTimestamp, CHNSYS_CHAR *sData, CHNSYS_INT nData);

// sData = aac frame without adts header
RTSP_IN_EXPORT CHNSYS_INT RTSP_SERVER_PushAACData(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_UINT nTimestamp, CHNSYS_CHAR *sData, CHNSYS_INT nData);

RTSP_IN_EXPORT CHNSYS_INT RTSP_SERVER_PushG711Data(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_UINT nTimestamp, CHNSYS_CHAR *sData, CHNSYS_INT nData);

typedef struct _RTSP_SERVER_CLIENTPROPERTY_
{
    CHNSYS_CHAR     sClientIP[16];
    CHNSYS_UINT     nRTSPPort;
    CHNSYS_UINT     nSessionDuration;
    CHNSYS_UINT64   nTotalPacketCount;
    CHNSYS_UINT64   nTotalLostPacketCount;
}RTSP_SERVER_CLIENTPROPERTY;

/**
 *
 * @nStatusType
 * 1 OPTION
 * 2 DESCRIBE
 * 3 SETUP
 * 4 PLAY
 * 5 TEARDOWN
 * 1000 client connect
 * 1001 client disconnect
 * @
 * @
 */
typedef struct _RTSP_SERVER_STATUSCBDATA_
{
    RTSP_SERVER_HANDLE          hRTSPServer;
    CHNSYS_INT                  nStatusType;
    CHNSYS_INT                  nStatusValue;
    RTSP_SERVER_CLIENTPROPERTY  clientProperty;
    VOID                        *pContext;
}RTSP_SERVER_STATUSCBDATA;

typedef int (*RTSP_SERVER_StatusCallBackFunc)(RTSP_SERVER_STATUSCBDATA scbd);

RTSP_IN_EXPORT VOID RTSP_SERVER_SetStatusCallBack(RTSP_SERVER_HANDLE hRTSPServer, RTSP_SERVER_StatusCallBackFunc scbf, VOID *pContext);

typedef struct _RTSP_SERVER_REQUESTCBDATA_
{
    CHNSYS_CHAR                 sRequestURL[1024];
    RTSP_SERVER_HANDLE          *phRTSPServer;
}RTSP_SERVER_REQUESTCBDATA;

typedef int (*RTSP_SERVER_RequestCallBackFunc)(RTSP_SERVER_REQUESTCBDATA rcbd);

RTSP_IN_EXPORT VOID RTSP_SERVER_SetRequestCallBack(RTSP_SERVER_RequestCallBackFunc rcbf, VOID *pContext);

RTSP_IN_EXPORT CHNSYS_INT RTSP_SERVER_GetClientCount(RTSP_SERVER_HANDLE hRTSPServer);
RTSP_IN_EXPORT CHNSYS_INT RTSP_SERVER_GetClientProperty(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_INT nIndex, 
                                         RTSP_SERVER_CLIENTPROPERTY *pProperty);
RTSP_IN_EXPORT CHNSYS_INT RTSP_SERVER_DisconnectClient(RTSP_SERVER_HANDLE hRTSPServer, RTSP_SERVER_CLIENTPROPERTY clientProperty);

// for advance use;
RTSP_IN_EXPORT VOID RTSP_SERVER_SetSDP(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_CHAR *sSDP, CHNSYS_INT nSDP);
RTSP_IN_EXPORT VOID RTSP_SERVER_SetSDPMulticast(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_CHAR *sSDP, CHNSYS_INT nSDP, CHNSYS_CHAR *sMultiAddr, CHNSYS_INT nMultiPort);
RTSP_IN_EXPORT CHNSYS_INT RTSP_SERVER_PushRTPData(RTSP_SERVER_HANDLE hRTSPServer, CHNSYS_INT nStreamNo, CHNSYS_CHAR *sRTPData, CHNSYS_INT nRTPSize);

RTSP_IN_EXPORT int	rtsptsmutexinit();
//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //_CHNSYSRTSP_H_
