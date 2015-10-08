#ifndef _SDPUTIL_H_
#define _SDPUTIL_H_

/**
 *
 * @
 * @
 * @
 * return
 * 100 = h264
 * 200 = aac-generic
 * 201 = aac-latm
 * 300 = g711u
 * 301 = g711a
 */
int SDP_GetStreamType(char *sSDP, int nSDP, int nStreamNo);

int SDP_GetSpsPpsBased(char *sSDP, int nSDP, int nStreamNo, 
                  char *pSPS, int *nSPS, char *pPPS, int *nPPS);

int SDP_GetSpsPps(char *sSDP, int nSDP, int nStreamNo,
                  char *pSPS, int *nSPS, char *pPPS, int *nPPS);

int SDP_GetAudioProperty(char *sSDP, int nSDP, int nStreamNo,
                         int *nSampleRate, int *nChannelNo);

/**
 *
 * @nAudioObjectType
 * 1-AAC Main; 2-AAC LC; 3-AAC SSR; 4-AAC LTP
 * @
 * @
 */
int SDP_GetMpeg4GenericConfig(int nAudioObjectType, int nSampleRate, int nChannel);

const char *SDP_GetModeName(int nMode);

int SDP_GetStreamCount(char *sSDP, int nSDP);

bool SDP_IsMulticastSession(char *sSDP, int nSDP);

char* SDP_GetMulticastIP(char *sSDP, int nSDP, char *sMultiIP);

/**
 *
 * @
 * @
 * @
 * if sMedia=NULL, only return size of sMedia
 */
int SDP_GetMedia(char *sSDP, int nSDP, int nStreamNo, char *sMedia);

int SDP_GetMulticastPort(char *sSDP, int nSDP, int nStreamNo);

#endif //_SDPUTIL_H_
