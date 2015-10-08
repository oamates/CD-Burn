#ifndef _NALUTIL_H_
#define _NALUTIL_H_

char *NALUTIL_GetSPS(char *sNalData, int nNalSize, int *nRelativePos, int *nSPSSize);
char *NALUTIL_GetPPS(char *sNalData, int nNalSize, int *nRelativePos, int *nPPSSize);
char *NALUTIL_GetNal(char *sData, int nData, int *nRelativePos, int *nNalSize);
void NALUTIL_ParseSPS(char *sSPS, int nSPS, int *nProfile, int *nLevel, int *nWidth, int *nHeight);

#endif //_NALUTIL_H_
