///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZMath		Header File
	Create		20110721		ZHAOTT
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#include "Common.h"
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZMATH_H_
#define	_ZMATH_H_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include <math.h>
///////////////////////////////////////////////////////////////////////////////
class ZMath
{
///////////////////////////////////////////////////////////////////////////////
public:
STATIC INLINE INT ZFloatToInteger(FLOAT f)
{
	INT	i;
#if	defined(_WIN32_)
	__asm
	{
		fld   f
		fistp i
	}
#elif defined(_LINUX_)
		__asm__ __volatile__ 
		(
			"flds %1 \n\t"
			"fistpl %0 \n\t"
			: "=m" (i)
			: "m" (f)
		);
#else
	i	= (INT)f;
#endif	//
	return i;
};
STATIC INLINE FLOAT ZLog(FLOAT f)
{
	return FLOAT(log(f));
}
STATIC INLINE FLOAT ZPow(FLOAT fX,FLOAT fY)
{
	return FLOAT(pow(fX,fY));
}
STATIC int ZUINTCompare(CONST VOID* a,CONST VOID* b)
{
	return(int(*((UINT*)a)-*((UINT*)b)));
}
STATIC INLINE VOID ZUINTSort(UINT* nBase,UINT nSize)
{
	qsort(nBase,nSize,sizeof(UINT),ZUINTCompare);
}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//Hash
/////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//HASH
///////////////////////////////////////////////////////////////////////////////
typedef	enum	_MATH_HASH_TYPE_
{
	MATH_HASH_SUM	= 1,		//sum		//fast
	MATH_HASH_BERNSTEIN,		//Bernstein	//fast
	MATH_HASH_FNV,				//FNV		//
}MATH_HASH_TYPE;
///////////////////////////////////////////////////////////////////////////////
//Bernstein 
#define	BERNSTEIN_CONSTANTS		33
#define	BERNSTEIN_CONSTANTS_1	33		//BERNSTEIN_CONSTANTS
#define	BERNSTEIN_CONSTANTS_2	1089	//BERNSTEIN_CONSTANTS
#define	BERNSTEIN_CONSTANTS_3	35937	//BERNSTEIN_CONSTANTS
#define	BERNSTEIN_CONSTANTS_4	1185921	//BERNSTEIN_CONSTANTS
//FNV
#define	FNV_32_PRIME	0x1000193
#define	FNV_32_PRIME_1	0x0000000001000193ULL	//FNV_32_PRIME
#define	FNV_32_PRIME_2	0x0001000326027A69ULL	//FNV_32_PRIME
#define	FNV_32_PRIME_3	0x04B9076F3EE6B34BULL	//FNV_32_PRIME
#define	FNV_32_PRIME_4	0xDE859AD3502C3F11ULL	//FNV_32_PRIME
/////////////////////////////////////////////////////////////////////
STATIC INLINE UINT32 Hash(CONST CHAR* sHash,MATH_HASH_TYPE nType = MATH_HASH_BERNSTEIN)
{
	if(nType == MATH_HASH_SUM)
	{
		if(sHash != NULL)
		{
			UINT	nLen	= strlen((CHAR*)sHash);
			if(nLen > 0)
			{
				UINT	nStep1	= nLen>>2;
				UINT	nStep2	= nLen>>1;
				UINT	nStep3	= nLen-nStep1;

				//+ Fast
				return(nLen*(((UINT)sHash[(0)])+
							((UINT)sHash[(nStep1)])+
							((UINT)sHash[(nStep2)])+
							((UINT)sHash[(nStep3)])+
							((UINT)sHash[(nLen-1)])));
			}
			return 1;//""
		}
		return 0;//NULL
	}
	if(nType == MATH_HASH_BERNSTEIN)
	{
		if(sHash != NULL)
		{
			UINT	nLen	= strlen((CHAR*)sHash);
			if(nLen > 0)
			{
				UINT	nStep1	= nLen>>2;
				UINT	nStep2	= nLen>>1;
				UINT	nStep3	= nLen-nStep1;

				//+ Fast
				return(nLen*((((UINT)sHash[(0)]))+
							(((UINT)sHash[(nStep1)])*BERNSTEIN_CONSTANTS_1)+
							(((UINT)sHash[(nStep2)])*BERNSTEIN_CONSTANTS_2)+
							(((UINT)sHash[(nStep3)])*BERNSTEIN_CONSTANTS_3)+
							(((UINT)sHash[(nLen-1)])*BERNSTEIN_CONSTANTS_4)));
			}
			return 1;//""
		}
		return 0;//NULL
	}
	if(nType == MATH_HASH_FNV)
	{
		if(sHash != NULL)
		{
			UINT	nLen	= strlen((CHAR*)sHash);
			if(nLen > 0)
			{
				UINT	nStep1	= nLen>>2;
				UINT	nStep2	= nLen>>1;
				UINT	nStep3	= nLen-nStep1;

				//+ Fast
				return(nLen*((UINT)
							((((UINT64)sHash[(0)]))+
							(((UINT64)sHash[(nStep1)])*FNV_32_PRIME_1)+
							(((UINT64)sHash[(nStep2)])*FNV_32_PRIME_2)+
							(((UINT64)sHash[(nStep3)])*FNV_32_PRIME_3)+
							(((UINT64)sHash[(nLen-1)])*FNV_32_PRIME_4))));
			}
			return 1;//""
		}
		return 0;//NULL
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZMATH_H_
///////////////////////////////////////////////////////////////////////////////
