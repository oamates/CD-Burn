//////////////////////////////////////////////////////////////////////////
#ifndef _AACUTIL_H_
#define _AACUTIL_H_

// 
// nObjType: 0 - AAC Main; 1 - AAC LC; 2 - AAC SSR; 3 - AAC LTP;
void FormatADTSHeader(int nObjType, int nFrameLength, int nSampleRate, 
					  int nChannels, unsigned char *sADTSHeader);

// get aac frame length from the adts header
// return the length of aac frame, include 7 byte adts header
int GetAACFrameLength(unsigned char *sADTSHeader);

#endif
//////////////////////////////////////////////////////////////////////////
