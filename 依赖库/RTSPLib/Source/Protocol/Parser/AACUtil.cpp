//////////////////////////////////////////////////////////////////////////
#include "AACUtil.h"
#include <stdlib.h>

/*
// more info in 14496-3
adts_fixed_header()
{
	Syncword					12
	ID							1
	Layer						2
	protection_absent			1
	Profile_ObjectType			2
	sampling_frequency_index	4
	private_bit					1
	channel_configuration		3
	original/copy				1
	home						1
	Emphasis					2
}

0	1	2	3	4	5	6	7
|--------------SyncWord------
--------------| ID |Layer |p-absent|
 type |  sample_inex  |   |cha
nnel  | o | h |  E    | ------
--------frame_Length----------
-----------|----buffer_fullness
----------------------|no-raw- |




*/

/*

0x0			96000
0x1			88200
0x2			64000
0x3			48000
0x4			44100
0x5			32000
0x6			24000
0x7			22050
0x8			16000
0x9			12000
0xa			11025
0xb			8000
0xc			7350


*/

void FormatADTSHeader(int nObjType, int nFrameLength, int nSampleRate, int nChannels, unsigned char *sADTSHeader)
{
	if (sADTSHeader == NULL)
	{
		return;
	}

	int nRateIdx = 0;
	if (nSampleRate == 96000)
	{
		nRateIdx = 0;
	}
	else if (nSampleRate == 88200)
	{
		nRateIdx = 1;
	}
	else if (nSampleRate == 64000)
	{
		nRateIdx = 2;
	}
	else if (nSampleRate == 48000)
	{
		nRateIdx = 3;
	}
	else if (nSampleRate == 44100)
	{
		nRateIdx = 4;
	}
	else if (nSampleRate == 32000)
	{
		nRateIdx = 5;
	}
	else if (nSampleRate == 24000)
	{
		nRateIdx = 6;
	}
	else if (nSampleRate == 22050)
	{
		nRateIdx = 7;
	}
	else if (nSampleRate == 16000)
	{
		nRateIdx = 8;
	}
	else if (nSampleRate == 12000)
	{
		nRateIdx = 9;
	}
	else if (nSampleRate == 11025)
	{
		nRateIdx = 10;
	}
	else if (nSampleRate == 8000)
	{
		nRateIdx = 11;
	}
	else if (nSampleRate == 7350)
	{
		nRateIdx = 12;
	}

	unsigned int nNumDataBlock = nFrameLength/1024;
	nFrameLength += 7;

	sADTSHeader[0] = 0xFF;
	sADTSHeader[1] = 0xF1;
	sADTSHeader[2] = (nObjType<<6);
	sADTSHeader[2] |= (nRateIdx<<2);
	sADTSHeader[2] |= (nChannels&0x4)>>2;
	sADTSHeader[3] = (nChannels&0x3)<<6;
	sADTSHeader[3] |= (nFrameLength&0x1800)>>11;
	sADTSHeader[4] = (nFrameLength&0x1FF8)>>3;
	sADTSHeader[5] = (nFrameLength&0x7)<<5;
	sADTSHeader[5] |= 0x1F;
	sADTSHeader[6] = 0xFC;
	sADTSHeader[6] |= (nNumDataBlock&0x03);
}

int GetAACFrameLength(unsigned char *sADTSHeader)
{
#if 0
	int nAACFrameLength = 0;
	nAACFrameLength = (sADTSHeader[3]<<24);
	nAACFrameLength |= (sADTSHeader[4]<<16);
	nAACFrameLength |= (sADTSHeader[5]<<8);
	nAACFrameLength |= (sADTSHeader[6]);

	nAACFrameLength = (nAACFrameLength&0x3ffe000)>>13;
#endif

	int nAACFrameLength = 0;
	nAACFrameLength = (sADTSHeader[3]&0x3)<<11;
	nAACFrameLength |= (sADTSHeader[4]&0xff)<<3;
	nAACFrameLength |= (sADTSHeader[5]&0xe0)>>5;
	return nAACFrameLength;
}

//////////////////////////////////////////////////////////////////////////
