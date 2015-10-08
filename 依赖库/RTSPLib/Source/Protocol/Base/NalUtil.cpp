#include "NalUtil.h"
#include <stdio.h>
#include "ZBitStream.h"

char *NALUTIL_GetSPS(char *sNalData, int nNalSize, int *nRelativePos, int *nSPSSize)
{
    int nCurPos = 0;
    //int nNextNalPos = 0;
    bool bFindSPS = false;

    if (
        (nRelativePos != NULL)
        && (nSPSSize != NULL)
        )
    {
        while (nCurPos+4 < nNalSize)
        {
            if (
                (sNalData[nCurPos] == 0x00)
                && (sNalData[nCurPos+1] == 0x00)
                && (sNalData[nCurPos+2] == 0x00)
                && (sNalData[nCurPos+3] == 0x01)
                && ((sNalData[nCurPos+4]&0x1F) == 7)
                )
            {
                *nRelativePos = nCurPos;
                bFindSPS = true;
                break;
            }
            else
            {
                nCurPos ++;
            }
        }

        if (bFindSPS)
        {
            nCurPos ++;
            while (nCurPos+3 < nNalSize)
            {
                if (
                    (sNalData[nCurPos] == 0x00)
                    && (sNalData[nCurPos+1] == 0x00)
                    && (sNalData[nCurPos+2] == 0x00)
                    && (sNalData[nCurPos+3] == 0x01)
                    )
                {
                    break;
                }
                else
                {
                    nCurPos ++;
                }
            }
            if (nCurPos+3 >= nNalSize)
            {
                *nSPSSize = nNalSize-(*nRelativePos);
            }
            else
            {
                *nSPSSize = nCurPos-(*nRelativePos);
            }
            return sNalData+(*nRelativePos);
        }
    }

    return NULL;
}

char *NALUTIL_GetPPS(char *sNalData, int nNalSize, int *nRelativePos, int *nPPSSize)
{
    int nCurPos = 0;
    //int nNextNalPos = 0;
    bool bFindPPS = false;

    if (
        (nRelativePos != NULL)
        && (nPPSSize != NULL)
        )
    {
        while (nCurPos+4 < nNalSize)
        {
            if (
                (sNalData[nCurPos] == 0x00)
                && (sNalData[nCurPos+1] == 0x00)
                && (sNalData[nCurPos+2] == 0x00)
                && (sNalData[nCurPos+3] == 0x01)
                && ((sNalData[nCurPos+4]&0x1F) == 8)
                )
            {
                *nRelativePos = nCurPos;
                bFindPPS = true;
                break;
            }
            else
            {
                nCurPos ++;
            }
        }

        if (bFindPPS)
        {
            nCurPos ++;
            while (nCurPos+3 < nNalSize)
            {
                if (
                    (sNalData[nCurPos] == 0x00)
                    && (sNalData[nCurPos+1] == 0x00)
                    && (sNalData[nCurPos+2] == 0x00)
                    && (sNalData[nCurPos+3] == 0x01)
                    )
                {
                    break;
                }
                else
                {
                    nCurPos ++;
                }
            }
            if (nCurPos+3 >= nNalSize)
            {
                *nPPSSize = nNalSize-(*nRelativePos);
            }
            else
            {
                *nPPSSize = nCurPos-(*nRelativePos);
            }
            return sNalData+(*nRelativePos);
        }
    }

    return NULL;
}

char *NALUTIL_GetNal(char *sData, int nData, int *nRelativePos, int *nNalSize)
{
    int nCurPos = 0;
    //int nNextNalPos = 0;
    bool bFindNal = false;

    if (
        (nRelativePos != NULL)
        && (nNalSize != NULL)
        )
    {
        while (nCurPos+3 < nData)
        {
            if (
                (sData[nCurPos] == 0x00)
                && (sData[nCurPos+1] == 0x00)
                && (sData[nCurPos+2] == 0x00)
                && (sData[nCurPos+3] == 0x01)
                )
            {
                *nRelativePos = nCurPos;
                bFindNal = true;
                break;
            }
            else
            {
                nCurPos ++;
            }
        }

        if (bFindNal)
        {
            nCurPos ++;
            while (nCurPos+3 < nData)
            {
                if (
                    (sData[nCurPos] == 0x00)
                    && (sData[nCurPos+1] == 0x00)
                    && (sData[nCurPos+2] == 0x00)
                    && (sData[nCurPos+3] == 0x01)
                    )
                {
                    break;
                }
                else
                {
                    nCurPos ++;
                }
            }
            if (nCurPos+3 >= nData)
            {
                *nNalSize = nData - (*nRelativePos);
            }
            else
            {
                *nNalSize = nCurPos - (*nRelativePos);
            }
            return sData+(*nRelativePos);
        }
    }

    return NULL;
}



//////////////////////////////////////////////////////////////////////////
static unsigned char g_exp_golomb_bits[256] = {
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
	0, 
};

UINT NALUTIL_ParseSPS_UE(ZBitStream *bs)
{
	UINT bits, read;
	int bits_left;
	unsigned char coded;
	bool done = false;
	bits = 0;
	// we want to read 8 bits at a time - if we don't have 8 bits, 
	// read what's left, and shift.  The g_exp_golomb_bits calc remains the
	// same.
	while (done == false)
	{
		bits_left = bs->GetRemainBits();
		if (bits_left < 8)
		{
			read = bs->PeekBits(bits_left) << (8 - bits_left);
			done = true;
		}
		else
		{
			read = bs->PeekBits(8);
			if (read == 0)
			{
				bs->GetBits(8);
				bits += 8;
			}
			else
			{
				done = true;
			}
		}
	}

	coded = g_exp_golomb_bits[read];
	bs->GetBits(coded);
	bits += coded;

	return bs->GetBits(bits + 1) - 1;
}

UINT NALUTIL_ParseSPS_SE(ZBitStream *bs)
{
	UINT ret;
	ret = NALUTIL_ParseSPS_UE(bs);
	if ((ret & 0x1) == 0)
	{
		ret >>= 1;
		INT temp = 0 - ret;
		return temp;
	} 
	return (ret + 1) >> 1;
}

void NALUTIL_ParseSPS_Skip(ZBitStream *bs,int nBits)
{
	bs->GetBits(nBits);
}

void NALUTIL_ParseSPS_ScalingList(int sizeOfScalingList, ZBitStream *bs)
{
	int lastScale = 8, nextScale = 8;
	int j;

	for (j = 0; j < sizeOfScalingList; j++)
	{
		if (nextScale != 0)
		{
			nextScale = (lastScale + NALUTIL_ParseSPS_SE(bs) + 256) % 256;
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

void NALUTIL_ParseSPS(char *sSPS, int nSPS, int *nProfile, int *nLevel, int *nWidth, int *nHeight)
{
	int     nPicProfile = 0;
	int     nPicLevel   = 0;
	int     nPicWidth   = 0;
	int     nPicHeight  = 0;
	int     nFrameMbsOnlyFlag = 0;
	int     nPocType    = 0;
	int     nCycle      = 0;
	int     nHeader     = 1;

	if (sSPS != NULL)
	{
		if (
			(sSPS[0] == 0x00)
			&& (sSPS[1] == 0x00)
			&& (sSPS[2] == 0x00)
			&& (sSPS[3] == 0x01)
			)
		{
			nHeader = 5;
		}
		ZBitStream  bs((const BYTE*)(sSPS+nHeader), (nSPS-nHeader)*8);
		nPicProfile = bs.GetBits(8);
		bs.GetBits(8);
		nPicLevel = bs.GetBits(8);
		NALUTIL_ParseSPS_UE(&bs);
		if (
			(nPicProfile == 100)
			|| (nPicProfile == 110)
			|| (nPicProfile == 122)
			|| (nPicProfile == 144)
			)
		{
			if (NALUTIL_ParseSPS_UE(&bs) == 3)
			{
				bs.GetBits(1);
			}
			NALUTIL_ParseSPS_UE(&bs);
			NALUTIL_ParseSPS_UE(&bs);
			bs.GetBits(1);
			if (bs.GetBits(1))
			{
				for(int i = 0; i < 8; i ++)
				{
					if(bs.GetBits(1))
					{
						NALUTIL_ParseSPS_ScalingList(i < 6 ? 16 : 64, &bs);
					}
				}
			}

		}
		NALUTIL_ParseSPS_UE(&bs);
		nPocType = NALUTIL_ParseSPS_UE(&bs);
		if (nPocType == 0)
		{
			NALUTIL_ParseSPS_UE(&bs);
		}
		else if (nPocType == 1)
		{
			NALUTIL_ParseSPS_Skip(&bs, 1);
			NALUTIL_ParseSPS_SE(&bs);
			NALUTIL_ParseSPS_SE(&bs);
			nCycle = NALUTIL_ParseSPS_UE(&bs);
			if (nCycle > 256)
			{
				nCycle = 256;
			}
			while (nCycle -- > 0)
			{
				NALUTIL_ParseSPS_SE(&bs);
			}
		}
		NALUTIL_ParseSPS_UE(&bs);
		NALUTIL_ParseSPS_Skip(&bs, 1);
		nPicWidth = 16*(NALUTIL_ParseSPS_UE(&bs)+1);
		nPicHeight = 16*(NALUTIL_ParseSPS_UE(&bs)+1);
		nFrameMbsOnlyFlag = bs.GetBits(1);
		nPicHeight *= (2-nFrameMbsOnlyFlag);
		if (nPicHeight == 1088)
		{
			nPicHeight = 1080;
		}
		if (nProfile != NULL)
		{
			*nProfile = nPicProfile;
		}
		if (nLevel != NULL)
		{
			*nLevel = nPicLevel;
		}
		if (nWidth != NULL)
		{
			*nWidth = nPicWidth;
		}
		if (nHeight != NULL)
		{
			*nHeight = nPicHeight;
		}
	}
}

