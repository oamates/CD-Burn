#include "ZMD5.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define S11					7
#define S12					12
#define S13					17
#define S14					22
#define S21					5
#define S22					9
#define S23					14
#define S24					20
#define S31					4
#define S32					11
#define S33					16
#define S34					23
#define S41					6
#define S42					10
#define S43					15
#define S44					21
///////////////////////////////////////////////////////////////////////////////
#define F(x, y, z)			(((x)&(y))|((~x)&(z)))				//XY v not(X)Z
#define G(x, y, z)			(((x)&(z))|((y)&(~z)))				//XZ v Y not(Z)
#define H(x, y, z)			((x)^(y)^(z))						//X xor Y xor Z
#define I(x, y, z)			((y)^((x)|(~z)))					//Y xor (X v not(Z))
///////////////////////////////////////////////////////////////////////////////
/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n)	(((x) << (n)) | ((x) >> (32-(n))))
///////////////////////////////////////////////////////////////////////////////
/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) {					\
 (a) += (F((b), (c), (d)) + (x) + (INT32)(ac));	\
 (a) = ROTATE_LEFT ((a), (s));						\
 (a) += (b);										\
  }
#define GG(a, b, c, d, x, s, ac) {					\
 (a) += (G((b), (c), (d)) + (x) + (INT32)(ac));	\
 (a) = ROTATE_LEFT ((a), (s));						\
 (a) += (b);										\
  }
#define HH(a, b, c, d, x, s, ac) {					\
 (a) += (H((b), (c), (d)) + (x) + (INT32)(ac));	\
 (a) = ROTATE_LEFT ((a), (s));						\
 (a) += (b);										\
  }
#define II(a, b, c, d, x, s, ac) {					\
 (a) += (I((b), (c), (d)) + (x) + (INT32)(ac));	\
 (a) = ROTATE_LEFT ((a), (s));						\
 (a) += (b);										\
  }
///////////////////////////////////////////////////////////////////////////////
STATIC	UCHAR	PADDING[64]	=
{
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZMD5::ZMD5()
{
	ZMD5::MD5Init();
}
ZMD5::~ZMD5()
{
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CHAR* ZMD5::MD5String(CHAR* sString)
{
	STATIC	CHAR	sMD5Code[16]	= {0};

	ZMD5::MD5Init();
	ZMD5::MD5Update((CHAR*)sString,(UINT)strlen(sString));
	ZMD5::MD5Final(sMD5Code);

	return sMD5Code;
}
CHAR* ZMD5::MD5Memory(BYTE* pData,UINT nData)
{
	STATIC	CHAR	sMD5Code[16]	= {0};

	ZMD5::MD5Init();
	ZMD5::MD5Update((CHAR*)pData,(UINT)nData);
	ZMD5::MD5Final(sMD5Code);

	return sMD5Code;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
VOID ZMD5::MD5Init()
{
    m_nMD5Buffer[0]	= 0x67452301;
    m_nMD5Buffer[1]	= 0xefcdab89;
    m_nMD5Buffer[2]	= 0x98badcfe;
    m_nMD5Buffer[3]	= 0x10325476;

    m_nMD5Bits[0]	= 0;
    m_nMD5Bits[1]	= 0;

	ZMD5::MD5Memset(m_sMD5In,0,sizeof(CHAR)*64);
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZMD5::MD5Update(CHAR* sBuffer,UINT nBufferLen)
{
	UINT32 i, index, partLen;

	if(sBuffer != NULL)
	{
		index = (INT32)((m_nMD5Bits[0] >> 3) & 0x3F);

		if ((m_nMD5Bits[0] += (((INT32)nBufferLen) << 3))  < (((INT32)nBufferLen) << 3))
		{
			m_nMD5Bits[1] ++;
		}
		m_nMD5Bits[1] += (((INT32)nBufferLen) >> 29);

		partLen = 64 - index;

		if (nBufferLen >= partLen)
		{
			ZMD5::MD5Memcopy((CHAR*)&m_sMD5In[index], (CHAR*)sBuffer, partLen);
			ZMD5::MD5Transform(m_nMD5Buffer,m_sMD5In);
			for (i = partLen; i + 63 < nBufferLen; i += 64)
			{
				MD5Transform(m_nMD5Buffer,&sBuffer[i]);
			}

			index = 0;
		}else
		{
			i = 0;
		}

		ZMD5::MD5Memcopy((CHAR*)&m_sMD5In[index], (CHAR*)&sBuffer[i],nBufferLen-i);

		return TRUE;
	}
	return FALSE;
}
BOOL ZMD5::MD5Final(CHAR sMD5Code[16])
{
  CHAR bits[8];
  INT32 index, padLen;

  ZMD5::Encode (bits,8,m_nMD5Bits,2);

  index = (INT32)((m_nMD5Bits[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  ZMD5::MD5Update ((CHAR*)PADDING, padLen);

  ZMD5::MD5Update (bits,8);
  ZMD5::Encode (sMD5Code,16,m_nMD5Buffer,4);

  return TRUE;
}

VOID ZMD5::MD5Transform(INT32 nMD5Buffer[4],CHAR sMD5In[64])
{
    register INT32 a, b, c, d;
	INT32 x[16];

    a = nMD5Buffer[0];
    b = nMD5Buffer[1];
    c = nMD5Buffer[2];
    d = nMD5Buffer[3];
	ZMD5::Decode(x,16,sMD5In,64);
	
    FF(a, b, c, d, x[ 0], S11, 0xd76aa478);	/* 1 */  
    FF(d, a, b, c, x[ 1], S12, 0xe8c7b756);	/* 2 */  
    FF(c, d, a, b, x[ 2], S13, 0x242070db);	/* 3 */  
    FF(b, c, d, a, x[ 3], S14, 0xc1bdceee);	/* 4 */  
    FF(a, b, c, d, x[ 4], S11, 0xf57c0faf);	/* 5 */  
    FF(d, a, b, c, x[ 5], S12, 0x4787c62a);	/* 6 */  
    FF(c, d, a, b, x[ 6], S13, 0xa8304613);	/* 7 */  
    FF(b, c, d, a, x[ 7], S14, 0xfd469501);	/* 8 */  
    FF(a, b, c, d, x[ 8], S11, 0x698098d8);	/* 9 */  
    FF(d, a, b, c, x[ 9], S12, 0x8b44f7af);	/* 10 */ 
    FF(c, d, a, b, x[10], S13, 0xffff5bb1);	/* 11 */ 
    FF(b, c, d, a, x[11], S14, 0x895cd7be);	/* 12 */ 
    FF(a, b, c, d, x[12], S11, 0x6b901122);	/* 13 */ 
    FF(d, a, b, c, x[13], S12, 0xfd987193);	/* 14 */ 
    FF(c, d, a, b, x[14], S13, 0xa679438e);	/* 15 */ 
    FF(b, c, d, a, x[15], S14, 0x49b40821);	/* 16 */ 
                                                                  
    GG(a, b, c, d, x[ 1], S21, 0xf61e2562);	/* 17 */ 
    GG(d, a, b, c, x[ 6], S22, 0xc040b340);	/* 18 */ 
    GG(c, d, a, b, x[11], S23, 0x265e5a51);	/* 19 */ 
    GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa);	/* 20 */ 
    GG(a, b, c, d, x[ 5], S21, 0xd62f105d);	/* 21 */ 
    GG(d, a, b, c, x[10], S22, 0x2441453);	/* 22 */ 
    GG(c, d, a, b, x[15], S23, 0xd8a1e681);	/* 23 */ 
    GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8);	/* 24 */ 
    GG(a, b, c, d, x[ 9], S21, 0x21e1cde6);	/* 25 */ 
    GG(d, a, b, c, x[14], S22, 0xc33707d6);	/* 26 */ 
    GG(c, d, a, b, x[ 3], S23, 0xf4d50d87);	/* 27 */ 
    GG(b, c, d, a, x[ 8], S24, 0x455a14ed);	/* 28 */ 
    GG(a, b, c, d, x[13], S21, 0xa9e3e905);	/* 29 */ 
    GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8);	/* 30 */ 
    GG(c, d, a, b, x[ 7], S23, 0x676f02d9);	/* 31 */ 
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);	/* 32 */ 
                                                                  
    HH(a, b, c, d, x[ 5], S31, 0xfffa3942);	/* 33 */ 
    HH(d, a, b, c, x[ 8], S32, 0x8771f681);	/* 34 */ 
    HH(c, d, a, b, x[11], S33, 0x6d9d6122);	/* 35 */ 
    HH(b, c, d, a, x[14], S34, 0xfde5380c);	/* 36 */ 
    HH(a, b, c, d, x[ 1], S31, 0xa4beea44);	/* 37 */ 
    HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9);	/* 38 */ 
    HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60);	/* 39 */ 
    HH(b, c, d, a, x[10], S34, 0xbebfbc70);	/* 40 */ 
    HH(a, b, c, d, x[13], S31, 0x289b7ec6);	/* 41 */ 
    HH(d, a, b, c, x[ 0], S32, 0xeaa127fa);	/* 42 */ 
    HH(c, d, a, b, x[ 3], S33, 0xd4ef3085);	/* 43 */ 
    HH(b, c, d, a, x[ 6], S34, 0x4881d05);	/* 44 */ 
    HH(a, b, c, d, x[ 9], S31, 0xd9d4d039);	/* 45 */ 
    HH(d, a, b, c, x[12], S32, 0xe6db99e5);	/* 46 */ 
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8);	/* 47 */ 
    HH(b, c, d, a, x[ 2], S34, 0xc4ac5665);	/* 48 */ 
                                                                  
    II(a, b, c, d, x[ 0], S41, 0xf4292244);	/* 49 */ 
    II(d, a, b, c, x[ 7], S42, 0x432aff97);	/* 50 */ 
    II(c, d, a, b, x[14], S43, 0xab9423a7);	/* 51 */ 
    II(b, c, d, a, x[ 5], S44, 0xfc93a039);	/* 52 */ 
    II(a, b, c, d, x[12], S41, 0x655b59c3);	/* 53 */ 
    II(d, a, b, c, x[ 3], S42, 0x8f0ccc92);	/* 54 */ 
    II(c, d, a, b, x[10], S43, 0xffeff47d);	/* 55 */ 
    II(b, c, d, a, x[ 1], S44, 0x85845dd1);	/* 56 */ 
    II(a, b, c, d, x[ 8], S41, 0x6fa87e4f);	/* 57 */ 
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0);	/* 58 */ 
    II(c, d, a, b, x[ 6], S43, 0xa3014314);	/* 59 */ 
    II(b, c, d, a, x[13], S44, 0x4e0811a1);	/* 60 */ 
    II(a, b, c, d, x[ 4], S41, 0xf7537e82);	/* 61 */ 
    II(d, a, b, c, x[11], S42, 0xbd3af235);	/* 62 */ 
    II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb);	/* 63 */ 
    II(b, c, d, a, x[ 9], S44, 0xeb86d391);	/* 64 */ 

    nMD5Buffer[0] += a;
    nMD5Buffer[1] += b;
    nMD5Buffer[2] += c;
    nMD5Buffer[3] += d;

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZMD5::Encode (CHAR* output,INT32 outputlen,INT32* input,INT32 inputlen)
{
  INT32 i, j;

  if(output != NULL && input != NULL && outputlen == inputlen * 4)
  {
	for (i = 0, j = 0; i < inputlen; i++)
	{
		output[j++]	= (CHAR)(input[i] & 0xff);
		output[j++]	= (CHAR)((input[i] >> 8) & 0xff);
		output[j++]	= (CHAR)((input[i] >> 16) & 0xff);
		output[j++]	= (CHAR)((input[i] >> 24) & 0xff);
	}
	return TRUE;
  }
  return FALSE;
}

BOOL ZMD5::Decode (INT32* output,INT32 outputlen,CHAR* input,INT32 inputlen)
{
  INT32 i, j;

  if(output != NULL && input != NULL && inputlen == outputlen * 4)
  {
	for (i = 0, j = 0; j < inputlen; i++, j += 4)
	{
		output[i] = ((INT32)input[j]) | (((INT32)input[j+1]) << 8) | (((INT32)input[j+2]) << 16) | (((INT32)input[j+3]) << 24);
	}
	return TRUE;
  }
  return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
VOID ZMD5::MD5Memset(CHAR* output,INT32 nVal,INT32 len)
{
	INT32 i = 0;
	if(output != NULL && len > 0)
	{
		for (i = 0; i < len; i++)
		{
				output[i] = nVal;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
VOID ZMD5::MD5Memcopy (CHAR* output,CHAR* input,INT32 len)
{
	INT32 i = 0;
	if(output != NULL && input != NULL && len > 0)
	{
		for (i = 0; i < len; i++)
		{
				output[i] = input[i];
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
