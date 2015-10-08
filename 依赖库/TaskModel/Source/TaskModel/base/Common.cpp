#include "Common.h"
#include "ZOS.h"
#include "ZLog.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZObject::ZObject(const char* sObjectName)
{
	memset(m_sObjectLabel,0,sizeof(m_sObjectLabel));
	memset(m_sObjectName,0,sizeof(m_sObjectName));
	memcpy(m_sObjectLabel,DEFAULT_OBJECT_LABEL,DEFAULT_OBJECT_LABEL_LENGTH);
	if(sObjectName != NULL)
	{
		strncpy(m_sObjectName,sObjectName,MAX_KEY_LABEL);
	}
}
ZObject::~ZObject()
{
	memset(m_sObjectLabel,0,sizeof(m_sObjectLabel));
	memset(m_sObjectName,0,sizeof(m_sObjectName));
}
///////////////////////////////////////////////////////////////////////////////
char* ZObject::GetObjectName()
{
	return(m_sObjectName);
};
///////////////////////////////////////////////////////////////////////////////
void ZObject::Assert(const char *sAssert)
{
	if(sAssert != 0)
	{
        LOG_ERROR(("ErrorCode:%s [ZObject::Assert] %s\r\n", GetErrorCodeString(20000), sAssert));
	}
#if	defined(_WIN32_)
	//::DebugBreak();
#else
	//(*(UINT*)0) = 0;
#endif	//
}
int ZObject::IsValidObject(ZObject* pPoint)
{
	if(pPoint == NULL)
	{
		return 1;
	}
#if defined(_WIN32_)
	if(IsBadReadPtr(pPoint,sizeof(DWORD)))
	{
		TMASSERT((pPoint));
		return 1;
	}
#else
#endif //_WIN32_
	if(memcmp(((char*)pPoint+DEFAULT_OBJECT_VPTR_SIZE),DEFAULT_OBJECT_LABEL,DEFAULT_OBJECT_LABEL_LENGTH) != 0)
	{
		TMASSERT((pPoint));
		return 2;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
#define	ARCH_X86	1
/////////////////////////////////////////////////////////////////////
UINT16 Common_set16(char* s,UINT16 n)
{
	if(s != NULL)
	{
#ifdef	ARCH_X86	//x86 little endian
		s[ 0] = (char)((n		 ) & 0xFF);
		s[ 1] = (char)((n	>>	8) & 0xFF);
#else				//other big endian
		s[ 1] = (char)((n		 ) & 0xFF);
		s[ 0] = (char)((n	>>	8) & 0xFF);
#endif	//ARCH_X86
	}
	return n;
}
UINT32 Common_set24(char* s,UINT32 n)
{
	if(s != NULL)
	{
#ifdef	ARCH_X86	//x86 little endian
		s[ 0] = (char)((n		  ) & 0xFF);
		s[ 1] = (char)((n	>>	 8) & 0xFF);
		s[ 2] = (char)((n	>>	16) & 0xFF);
#else				//other big endian
		s[ 2] = (char)((n		  ) & 0xFF);
		s[ 1] = (char)((n	>>	 8) & 0xFF);
		s[ 0] = (char)((n	>>	16) & 0xFF);
#endif	//ARCH_X86
	}
	return n;
}
UINT32 Common_set32(char* s,UINT32 n)
{
	if(s != NULL)
	{
#ifdef	ARCH_X86	//x86 little endian
		s[ 0] = (char)((n		  ) & 0xFF);
		s[ 1] = (char)((n	>>	 8) & 0xFF);
		s[ 2] = (char)((n	>>	16) & 0xFF);
		s[ 3] = (char)((n	>>	24) & 0xFF);
#else				//other big endian
		s[ 3] = (char)((n		  ) & 0xFF);
		s[ 2] = (char)((n	>>	 8) & 0xFF);
		s[ 1] = (char)((n	>>	16) & 0xFF);
		s[ 0] = (char)((n	>>	24) & 0xFF);
#endif	//ARCH_X86
	}
	return n;
}
UINT64 Common_set64(char* s,UINT64 n)
{
	if(s != NULL)
	{
#ifdef	ARCH_X86	//x86 little endian
		s[ 0] = (char)((n		  ) & 0xFF);
		s[ 1] = (char)((n	>>	 8) & 0xFF);
		s[ 2] = (char)((n	>>	16) & 0xFF);
		s[ 3] = (char)((n	>>	24) & 0xFF);
		s[ 4] = (char)((n	>>	32) & 0xFF);
		s[ 5] = (char)((n	>>	40) & 0xFF);
		s[ 6] = (char)((n	>>	48) & 0xFF);
		s[ 7] = (char)((n	>>	56) & 0xFF);
#else				//other big endian
		s[ 7] = (char)((n		  ) & 0xFF);
		s[ 6] = (char)((n	>>	 8) & 0xFF);
		s[ 5] = (char)((n	>>	16) & 0xFF);
		s[ 4] = (char)((n	>>	24) & 0xFF);
		s[ 3] = (char)((n	>>	32) & 0xFF);
		s[ 2] = (char)((n	>>	40) & 0xFF);
		s[ 1] = (char)((n	>>	48) & 0xFF);
		s[ 0] = (char)((n	>>	56) & 0xFF);
#endif	//ARCH_X86
	}
	return n;
}
BYTE Common_get8(char* s)
{
	return	(s[0]&0xFF);
}
UINT16 Common_get16(char* s)
{
	UINT16 n = 0;

	if(s != NULL)
	{
#ifdef	ARCH_X86	//x86 little endian
		n = (UINT16)(((s[0]) & 0x00FF) | ((s[1] << 8) & 0xFF00));
#else				//other big endian
		n = (UINT16)(((s[1]) & 0x00FF) | ((s[0] << 8) & 0xFF00));
#endif	//ARCH_X86 
	}
	return n;
}
UINT32 Common_get24(char* s)
{
	UINT32 n = 0;

	if(s != NULL)
	{
#ifdef	ARCH_X86	//x86 little endian
		n = (UINT32)(  ((s[0]      ) & 0x000000FF) | ((s[1] <<  8) & 0x0000FF00) |
					   ((s[2] << 16) & 0x00FF0000));
#else				//other big endian
		n = (UINT32)(   ((s[2]      ) & 0x000000FF) | ((s[1] <<  8) & 0x0000FF00) |
					   ((s[0] << 16) & 0x00FF0000));
#endif	//ARCH_X86
	}
	return n;
}
UINT32 Common_get32(char* s)
{
	UINT32 n = 0;

	if(s != NULL)
	{
#ifdef	ARCH_X86	//x86 little endian
		n = (UINT32)(  ((s[0]      ) & 0x000000FF) | ((s[1] <<  8) & 0x0000FF00) |
					   ((s[2] << 16) & 0x00FF0000) | ((s[3] << 24) & 0xFF000000));
#else				//other big endian
		n = (UINT32)(   ((s[3]      ) & 0x000000FF) | ((s[2] <<  8) & 0x0000FF00) |
					   ((s[1] << 16) & 0x00FF0000) | ((s[0] << 24) & 0xFF000000));
#endif	//ARCH_X86
	}
	return n;
}
UINT64 Common_get64(char* s)
{
	UINT64 n = 0;

	if(s != NULL)
	{
#ifdef	ARCH_X86	//x86 little endian
		n = (UINT64)(  (((UINT64)s[0]      ) & 0x00000000000000FFLL) | (((UINT64)s[1] <<  8) & 0x000000000000FF00LL) |
					   (((UINT64)s[2] << 16) & 0x0000000000FF0000LL) | (((UINT64)s[3] << 24) & 0x00000000FF000000LL) |
					   (((UINT64)s[4] << 32) & 0x000000FF00000000LL) | (((UINT64)s[5] << 40) & 0x0000FF0000000000LL) |
					   (((UINT64)s[6] << 48) & 0x00FF000000000000LL) | (((UINT64)s[7] << 56) & 0xFF00000000000000LL));
#else				//other big endian
		n = (UINT64)(  (((UINT64)s[7]      ) & 0x00000000000000FFLL) | (((UINT64)s[6] <<  8) & 0x000000000000FF00LL) |
					   (((UINT64)s[5] << 16) & 0x0000000000FF0000LL) | (((UINT64)s[4] << 24) & 0x00000000FF000000LL) |
					   (((UINT64)s[3] << 32) & 0x000000FF00000000LL) | (((UINT64)s[2] << 40) & 0x0000FF0000000000LL) |
					   (((UINT64)s[1] << 48) & 0x00FF000000000000LL) | (((UINT64)s[0] << 56) & 0xFF00000000000000LL));
#endif	//ARCH_X86
	}
	return n;
}
UINT16 Common_swap16(UINT16 n)
{
	return(UINT16)(	((n&0xFF00)>>8)|
					((n&0x00FF)<<8));
}
UINT32 Common_swap24(UINT32 n)
{
	return(UINT32)(	((n&0x00FF0000)>>16)|((n&0x0000FF00))|
					((n&0x000000FF)<<16));
}
UINT32 Common_swap32(UINT32 n)
{
	return(UINT32)(	((n&0xFF000000)>>24)|((n&0x00FF0000)>>8)|
					((n&0x000000FF)<<24)|((n&0x0000FF00)<<8));
}
UINT64 Common_swap64(UINT64 n)
{
	return(UINT64)(	((n&0xFF00000000000000LL)>>56)|((n&0x00FF000000000000LL)>>40)|((n&0x0000FF0000000000LL)>>24)|((n&0x000000FF00000000LL)>>8)|
					((n&0x00000000000000FFLL)<<56)|((n&0x000000000000FF00LL)<<40)|((n&0x0000000000FF0000LL)<<24)|((n&0x00000000FF000000LL)<<8));
}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
static const char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/////////////////////////////////////////////////////////////////////
int Common_Base64Encode(const char* sSource,int nSource,char* sTarget,int nTarget)
{
	int		i	= 0;
	int		n	= 0;
	char*	p	= NULL;
	

	if(sSource != NULL && nSource > 0)
	{
		n	= ((nSource + 2) / 3 * 4) + 1;
		if(sTarget != NULL && nTarget >= n)
		{
			p = sTarget;

			for(i = 0; i < nSource - 2; i += 3)
			{
				*p++	= basis_64[((sSource[i] >> 2) & 0x3F)];
				*p++	= basis_64[((sSource[i] & 0x3) << 4) | ((int) (sSource[(i + 1)] & 0xF0) >> 4)];
				*p++	= basis_64[((sSource[(i + 1)] & 0xF) << 2) | ((int) (sSource[(i + 2)] & 0xC0) >> 6)];
				*p++	= basis_64[((sSource[(i + 2)] & 0x3F)    ) ];
			}
			if(i < nSource)
			{
				*p++	= basis_64[(sSource[i] >> 2) & 0x3F ];
				if(i == (nSource - 1))
				{
					*p++ = basis_64[((sSource[i] & 0x3) << 4) ];
					*p++ = '=';
				}else{
					*p++ = basis_64[((sSource[i] & 0x3) << 4) | ((int) (sSource[(i + 1)] & 0xF0) >> 4)];
					*p++ = basis_64[((sSource[(i + 1)] & 0xF) << 2) ];
				}
				*p++	= '=';
			}
			*p++ = '\0';
			return (p - sTarget);
		}
		return n;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////
static const unsigned char pr2six[256] =
{
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};
/////////////////////////////////////////////////////////////////////
int Common_Base64Decode(const char* sSource,int nSource,char* sTarget,int nTarget)
{
	int								l	= 0;
	int								n	= 0;
	register const unsigned char*	p	= NULL;
    register unsigned char*			q	= NULL;
	

	if(sSource != NULL && nSource > 0)
	{
		p	= (const unsigned char *)sSource;
		while(pr2six[*(p++)] <= 63);
		l	= (p - (const unsigned char *) sSource) - 1;
		n	= ((l + 3) / 4) * 3;

		if(sTarget != NULL && nTarget >= n)
		{
			p	= (const unsigned char *) sSource;
			q	= (unsigned char *) sTarget;

			while (l > 4)
			{
				*(q ++)	= (unsigned char) (pr2six[p[0]] << 2 | pr2six[p[1]] >> 4);
				*(q ++)	= (unsigned char) (pr2six[p[1]] << 4 | pr2six[p[2]] >> 2);
				*(q ++)	= (unsigned char) (pr2six[p[2]] << 6 | pr2six[p[3]]);
				p		+= 4;
				l		-= 4;
			}

			if (l > 1)
			{
				*(q++)	= (unsigned char) (pr2six[p[0]] << 2 | pr2six[p[1]] >> 4);
			}
			if (l > 2)
			{
				*(q++)	= (unsigned char) (pr2six[p[1]] << 4 | pr2six[p[2]] >> 2);
			}
			if (l > 3)
			{
				*(q++)	= (unsigned char) (pr2six[p[2]] << 6 | pr2six[p[3]]);
			}

			*(q ++)	= '\0';
			n		-= (4 - l) & 3;
			return n;
		}
		return n;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
