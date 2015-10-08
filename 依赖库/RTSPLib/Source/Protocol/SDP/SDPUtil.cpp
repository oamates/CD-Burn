#include "SDPUtil.h"
#include <stdio.h>
#include <string.h>
#include "ZOS.h"

int SDP_GetStreamType(char *sSDP, int nSDP, int nStreamNo)
{
    char sMedia[4096];

    if (
        (sSDP != NULL)
        && (nSDP > 0)
        )
    {
        memset(sMedia, 0, 4096);
        if (SDP_GetMedia(sSDP, nSDP, nStreamNo, sMedia) > 0)
        {
            ZOS::lower(sMedia);
            if (strstr(sMedia, "h264") != NULL)
            {
                return 100;
            }
            else if (strstr(sMedia, "mpeg4-generic") != NULL)
            {
                return 200;
            }
            else if (strstr(sMedia, "mp4a-latm") != NULL)
            {
                return 201;
            }
            else if (strstr(sMedia, "pcmu") != NULL)
            {
                return 300;
            }
            else if (strstr(sMedia, "pcma") != NULL)
            {
                return 301;
            }

        }
    }

	char mediumName[16];
	unsigned short nClientPortNum = 0;
	unsigned int payloadFormat = 0;
	char const* protocolName = NULL;
	if ((sscanf(sMedia, "m=%s %hu rtp/avp %u",
		mediumName, &nClientPortNum, &payloadFormat) == 3 ||
		sscanf(sMedia, "m=%s %hu/%*u rtp/avp %u",
		mediumName, &nClientPortNum, &payloadFormat) == 3)
		&& payloadFormat <= 127) {
			protocolName = "RTP";
	} else if ((sscanf(sMedia, "m=%s %hu udp %u",
		mediumName, &nClientPortNum, &payloadFormat) == 3 ||
		sscanf(sMedia, "m=%s %hu udp %u",
		mediumName, &nClientPortNum, &payloadFormat) == 3 ||
		sscanf(sMedia, "m=%s %hu raw/raw/udp %u",
		mediumName, &nClientPortNum, &payloadFormat) == 3)
		&& payloadFormat <= 127) {
			// This is a RAW UDP source
			protocolName = "UDP";
	}

	if(payloadFormat == 0)
	{
		return 300;
	}
	else if(payloadFormat == 8)
	{
		return 301;
	}

    return -1;
}

int SDP_GetSpsPpsBased(char *sSDP, int nSDP, int nStreamNo, 
                  char *pSPS, int *nSPS, char *pPPS, int *nPPS)
{
    char sMedia[4096];
    char sSPSBase64[4096];
    char sPPSBase64[4096];
    int  nSPSBase64 = 0;
    int  nPPSBase64 = 0;
    char *pParameterSet = NULL;
    int nCurPos = 0;//relative to sMedia
    int nTempPos = 0;

    if (
        (sSDP != NULL)
        && (nSDP > 0)
        )
    {
        memset(sMedia, 0 ,4096);
        memset(sSPSBase64, 0, 4096);
        memset(sPPSBase64, 0, 4096);

        if (SDP_GetMedia(sSDP, nSDP, nStreamNo, sMedia) > 0)
        {
            pParameterSet = strstr(sMedia, "sprop-parameter-sets=");
            if (pParameterSet != NULL)
            {
                nCurPos = (pParameterSet-sMedia);
                nCurPos += strlen("sprop-parameter-sets=");

                // get sps
                nTempPos = 0;
                while (
                    (*(sMedia+nCurPos) != ',')
                    && (nCurPos < (int)strlen(sMedia))
                    )
                {
                    sSPSBase64[nTempPos] = sMedia[nCurPos];
                    nCurPos ++;
                    nTempPos ++;
                }
                if (*(sMedia+nCurPos) == ',')
                {
                    nSPSBase64 = nTempPos;
                    if (nSPS != NULL)
                    {
                        *nSPS = nSPSBase64;
                    }
                    nCurPos ++;

                    // get pps
                    nTempPos = 0;
                    while (
                        (*(sMedia+nCurPos) != ';')
                        && (*(sMedia+nCurPos) != '\r')
                        && (nCurPos < (int)strlen(sMedia))
                        )
                    {
                        sPPSBase64[nTempPos] = sMedia[nCurPos];
                        nCurPos ++;
                        nTempPos ++;
                    }
                    if (
                        (*(sMedia+nCurPos) == ';')
                        || (*(sMedia+nCurPos) == '\r')
                        )
                    {
                        nPPSBase64 = nTempPos;
                        if (nPPS != NULL)
                        {
                            *nPPS = nPPSBase64;
                        }
                        memcpy(pSPS, sSPSBase64, nSPSBase64);
                        pSPS[nSPSBase64] = '\0';
                        memcpy(pPPS, sPPSBase64, nPPSBase64);
                        pPPS[nPPSBase64] = '\0';

                        return 0;
                    }
                }
            }
        }
    }

    return -1;
}

int SDP_GetSpsPps(char *sSDP, int nSDP, int nStreamNo,
                  char *pSPS, int *nSPS, char *pPPS, int *nPPS)
{
    char sSPSBase64[4096];
    int  nSPSBase64 = 0;
    char sPPSBase64[4096];
    int  nPPSBase64 = 0;
    char sSPSRaw[4096];
    int  nSPSRaw = 4096;
    char sPPSRaw[4096];
    int  nPPSRaw = 4096;

    if (
        (sSDP != NULL)
        && (pSPS != NULL)
        && (nSPS != NULL)
        && (pPPS != NULL)
        && (nPPS != NULL)
        )
    {
        if (SDP_GetSpsPpsBased(sSDP, nSDP, nStreamNo, sSPSBase64, &nSPSBase64, 
            sPPSBase64, &nPPSBase64) == 0)
        {
            nSPSRaw = Common_Base64Decode(sSPSBase64, nSPSBase64, sSPSRaw, nSPSRaw);
            nPPSRaw = Common_Base64Decode(sPPSBase64, nPPSBase64, sPPSRaw, nPPSRaw);
            *nSPS = nSPSRaw;
            memcpy(pSPS, sSPSRaw, nSPSRaw);
            *nPPS = nPPSRaw;
            memcpy(pPPS, sPPSRaw, nPPSRaw);
            return 0;
        }
    }

    return -1;
}

int SDP_GetAudioProperty(char *sSDP, int nSDP, int nStreamNo,
                         int *nSampleRate, int *nChannelNo)
{
    char sMedia[4096];
    char *pCurPos = NULL;

    if (
        (sSDP != NULL)
        && (nSDP > 0)
        )
    {
        memset(sMedia, 0, 4096);
        if (SDP_GetMedia(sSDP, nSDP, nStreamNo, sMedia) > 0)
        {
            ZOS::lower(sMedia);
            pCurPos = strstr(sMedia, "mpeg4-generic");
            if (pCurPos == NULL)
            {
                pCurPos = strstr(sMedia, "mp4a-latm");
				if(pCurPos == NULL)
				{
					pCurPos = strstr(sMedia, "pcma");
					if(pCurPos == NULL)
					{
						pCurPos = strstr(sMedia, "pcmu");
						if(pCurPos == NULL)
						{
							return -1;
						}
						else
						{
							pCurPos += strlen("pcmu");
						}
					}
					else
					{
						pCurPos += strlen("pcma");
					}
				}
				else
				{
					pCurPos += strlen("mp4a-latm");
				}
            }
            else
            {
                pCurPos += strlen("mpeg4-generic");
            }
            //skip '/'
            if (*pCurPos == '/')
            {
                pCurPos ++;
                *nSampleRate = 0;
                while (
                    (*pCurPos>='0')
                    &&(*pCurPos<='9')
                    &&(pCurPos<sMedia+strlen(sMedia))
                    )
                {
                    *nSampleRate = *nSampleRate*10;
                    *nSampleRate += (*pCurPos-'0');
                    pCurPos ++;
                }
                //skip '/'
                if (*pCurPos == '/')
                {
                    pCurPos ++;
                    *nChannelNo = (*pCurPos-'0');
                    return 0;
                }
            }
        }
    }

    return -1;
}

int SDP_GetMpeg4GenericConfig(int nAudioObjectType, int nSampleRate, int nChannel)
{
    int nConfig;
    int nSampleRateIndex = 0;

    if (nSampleRate == 96000)
    {
        nSampleRateIndex = 0;
    }
    else if (nSampleRate == 88200)
    {
        nSampleRateIndex = 1;
    }
    else if (nSampleRate == 64000)
    {
        nSampleRateIndex = 2;
    }
    else if (nSampleRate == 48000)
    {
        nSampleRateIndex = 3;
    }
    else if (nSampleRate == 44100)
    {
        nSampleRateIndex = 4;
    }
    else if (nSampleRate == 32000)
    {
        nSampleRateIndex = 5;
    }
    else if (nSampleRate == 24000)
    {
        nSampleRateIndex = 6;
    }
    else if (nSampleRate == 22050)
    {
        nSampleRateIndex = 7;
    }
    else if (nSampleRate == 16000)
    {
        nSampleRateIndex = 8;
    }
    else if (nSampleRate == 12000)
    {
        nSampleRateIndex = 9;
    }
    else if (nSampleRate == 11025)
    {
        nSampleRateIndex = 10;
    }
    else if (nSampleRate == 8000)
    {
        nSampleRateIndex = 11;
    }
    else if (nSampleRate == 7350)
    {
        nSampleRateIndex = 12;
    }

    nConfig = 0;
    nConfig |= nAudioObjectType<<11;
    nConfig |= nSampleRateIndex<<7;
    nConfig |= nChannel<<3;

    return nConfig;
}

const char *SDP_GetModeName(int nMode)
{
    if (nMode == 1)
    {
        return "generic";
    }
    else if (nMode == 2)
    {
        return "CELP-cbr";
    }
    else if (nMode == 3)
    {
        return "CELP-vbr";
    }
    else if (nMode == 4)
    {
        return "AAC-lbr";
    }
    else if (nMode == 5)
    {
        return "AAC-hbr";
    }

    return "AAC-hbr";
}

int SDP_GetStreamCount(char *sSDP, int nSDP)
{
    int nStreamCount = 0;
    int nCurPos = 0;

    if (sSDP != NULL)
    {
        while (nCurPos < nSDP)
        {
            if (
                (*(sSDP+nCurPos)=='m')
                && (nCurPos+1<nSDP)
                && (*(sSDP+nCurPos+1)=='=')
                && (*(sSDP+nCurPos-1)=='\n')
                && (*(sSDP+nCurPos-2)=='\r')
                )
            {
                nStreamCount ++;
            }
            nCurPos ++;
        }
    }

    return nStreamCount;
}

/**
 *
 * @
 * @
 * @
 * c=IN IP4 224.0.0.0/255
 * c=IN IP4 239.255.255.255/255
 * find first byte of ip is between 224 and 239
 */
bool SDP_IsMulticastSession(char *sSDP, int nSDP)
{
    bool bIsMulticastSession = false;
    char *pCurPos = NULL;
    int nFirstByte = 0;

    pCurPos = strstr(sSDP, "c=IN IP4 ");
    while (pCurPos != NULL)
    {
        pCurPos = pCurPos+strlen("c=IN IP4 ");
        while (
            (*pCurPos>='0')
            &&(*pCurPos<='9')
            )
        {
            nFirstByte = nFirstByte*10;
            nFirstByte += ((*pCurPos)-'0');
            pCurPos++;
        }
        if (
            (nFirstByte>=224)
            && (nFirstByte<=239)
            )
        {
            bIsMulticastSession = true;
            break;
        }
        pCurPos = strstr(pCurPos, "c=IN IP4 ");
    }

    return bIsMulticastSession;
}

char * SDP_GetMulticastIP(char *sSDP, int nSDP, char *sMultiIP)
{
    char *pCurPos   = NULL;
    char sIP[16];
    int nIPPos      = 0;
    int nFirstByte  = 0;
    int nSecondByte = 0;
    int nThirdByte  = 0;
    int nFourthByte = 0;

    memset(sIP, 0, 16);

    pCurPos = strstr(sSDP, "c=IN IP4 ");
    while (pCurPos != NULL)
    {
        pCurPos = pCurPos+strlen("c=IN IP4 ");
        while (
            ((*pCurPos>='0')&&(*pCurPos<='9'))
            ||(*pCurPos=='.')
            )
        {
            sIP[nIPPos] = *pCurPos;
            pCurPos++;
            nIPPos++;
        }
        sIP[nIPPos]='\0';

        sscanf(sIP, "%d.%d.%d.%d", &nFirstByte, &nSecondByte, &nThirdByte, &nFourthByte);
        if (
            (nFirstByte>=224)
            && (nFirstByte<=239)
            )
        {
            strncpy(sMultiIP, sIP, 16);
            sMultiIP[15] = '\0';
            return sMultiIP;
        }
        pCurPos = strstr(pCurPos, "c=IN IP4 ");
    }

    return NULL;
}

int SDP_GetMedia(char *sSDP, int nSDP, int nStreamNo, char *sMedia)
{
    int nCurStreamNo = 0;
    int nCurPos = 0;
    int nMediaStartPos = 0;
    int nMediaEndPos = 0;

    if (sSDP != NULL)
    {
        // get nMediaStartPos
        while (nCurPos < nSDP)
        {
            if (
                (*(sSDP+nCurPos)=='m')
                && (nCurPos+1<nSDP)
                && (*(sSDP+nCurPos+1)=='=')
                && (*(sSDP+nCurPos-1)=='\n')
                && (*(sSDP+nCurPos-2)=='\r')
                )
            {
                if (nStreamNo == nCurStreamNo)
                {
                    nMediaStartPos = nCurPos;
                    break;
                }
                else
                {
                    nCurStreamNo ++;
                }
            }
            nCurPos ++;
        }

        // get nMediaEndPos
        if (nMediaStartPos > 0)
        {
            nCurPos ++;
            while (nCurPos < nSDP)
            {
                if (
                    (*(sSDP+nCurPos)=='m')
                    && (nCurPos+1<nSDP)
                    && (*(sSDP+nCurPos+1)=='=')
                    && (*(sSDP+nCurPos-1)=='\n')
                    && (*(sSDP+nCurPos-2)=='\r')
                    )
                {
                    nMediaEndPos = nCurPos;
                    break;
                }
                nCurPos ++;
            }
            if (nMediaEndPos == 0)
            {
                nMediaEndPos = strlen(sSDP);
            }

            memcpy(sMedia, sSDP+nMediaStartPos, nMediaEndPos-nMediaStartPos);
            sMedia[nMediaEndPos-nMediaStartPos] = '\0';
            return nMediaEndPos-nMediaStartPos;
        }
    }

    return 0;
}

int SDP_GetMulticastPort(char *sSDP, int nSDP, int nStreamNo)
{
    char sMedia[4096];
    char *pCurPos;
    int nPort = 0;

    memset(sMedia, 0, 4096);
    if (SDP_GetMedia(sSDP, nSDP, nStreamNo, sMedia) > 0)
    {
        // skip first space
        pCurPos = sMedia;
        while (*pCurPos != ' ')
        {
            pCurPos ++;
        }

        // get port
        pCurPos ++;
        while (
            (*pCurPos>='0')
            && (*pCurPos<='9')
            )
        {
            nPort = nPort*10;
            nPort += (*pCurPos-'0');
            pCurPos ++;
        }
    }

    return nPort;
}
