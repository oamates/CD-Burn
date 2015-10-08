#include "ZHeaderParser.h"
#include "ZOSMemory.h"
#include <ctype.h>
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_WORDS_SIZE	1024
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef	DEFAULT_CHARACTER_PARSEER
BOOL IsPrintCharacter(CHAR c)
{
	if(c >= 0x20 && c <= 0x7E)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL IsWordCharacter(CHAR c)
{
	//'-' and '_'
	if(c == '-' || c == '_')
	{
		return TRUE;
	}
	//'A'-'Z'
	if(c >= 'A' && c <= 'Z')
	{
		return TRUE;
	}
	//'a'-'z'
	if(c >= 'a' && c <= 'z')
	{
		return TRUE;
	}
	return FALSE;
}
BOOL IsDigitCharacter(CHAR c)
{
	if(c >= '0' && c <= '9')
	{
		return TRUE;
	}
	return FALSE;
}
#else
STATIC	CONST	BYTE	PRINT_CHARACTER_MASK[]	=
{
//	0 1 2 3 4 5 6 7 8 9 a b c d e f
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//0
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//1
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,//2
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,//3
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,//4
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,//5
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,//6
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,//7
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//8
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//9
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//a
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//b
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//c
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//d
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//e
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//f
};
STATIC	CONST	BYTE	WORD_CHARACTER_MASK[]	=
{
//	0 1 2 3 4 5 6 7 8 9 a b c d e f
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//0
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//1
	0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,//2
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//3
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,//4
	1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,//5
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,//6
	1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,//7
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//8
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//9
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//a
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//b
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//c
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//d
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//e
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//f
};
STATIC	CONST	BYTE	DIGIT_CHARACTER_MASK[]	=
{
//	0 1 2 3 4 5 6 7 8 9 a b c d e f
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//0
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//1
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//2
	1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,//3
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//4
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//5
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//6
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//7
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//8
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//9
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//a
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//b
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//c
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//d
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//e
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//f
};
BOOL IsPrintCharacter(CHAR c)
{
	return PRINT_CHARACTER_MASK[(BYTE)c];
}
BOOL IsWordCharacter(CHAR c)
{
	return WORD_CHARACTER_MASK[(BYTE)c];
}
BOOL IsDigitCharacter(CHAR c)
{
	return DIGIT_CHARACTER_MASK[(BYTE)c];
}
#endif	//DEFAULT_CHARACTER_PARSEER
BOOL IsSpaceCharacter(CHAR c)
{
	if((c == ' ') || (c == '\t') || (c == '\r')|| (c == '\n'))
	{
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZHeaderParser::ZHeaderParser(CONST CHAR* sHeader,int nHeader)
:m_sHeader(NULL)
,m_nHeader(0)
,m_sLines(NULL)
,m_nLines(0)
,m_sData(0)
,m_nData(0)
{
	UINT	nLines	= 0;

	if(sHeader != NULL && nHeader > 0)
	{
		m_sHeader	= NEW CHAR[(nHeader+4)];
		nLines		= (nHeader/2);
		m_sLines		= NEW (HEADER_LINE[nLines]);
		m_sData		= NEW CHAR[DEFAULT_WORDS_SIZE+4];
		if(m_sHeader != NULL && m_sLines != NULL && m_sData != NULL)
		{
			memset(m_sHeader,0,(nHeader+4));
			memset(m_sLines,0,(sizeof(HEADER_LINE)*nLines));
			memset(m_sData,0,(DEFAULT_WORDS_SIZE+4));
			ZHeaderParser::ParseHeader(sHeader,nHeader);
		}
	}
}
ZHeaderParser::~ZHeaderParser()
{
	SAFE_DELETE_ARRAY(m_sHeader);

	SAFE_DELETE_ARRAY(m_sLines);

	SAFE_DELETE_ARRAY(m_sData);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CHAR* ZHeaderParser::GetHeader()
{
	return(m_sHeader);
}
UINT ZHeaderParser::GetHeaderSize()
{
	return(m_nHeader);
}
UINT ZHeaderParser::GetLineCount()
{
	return(m_nLines);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CHAR* ZHeaderParser::GetLine(int nLine)
{
	CHAR*	sLines	= NULL;	

	if(nLine >= 0 && nLine < m_nLines)
	{
		sLines	= m_sLines[nLine].m_sLine;
	}
	return sLines;
}
CHAR ZHeaderParser::GetChar(int nLine,int* nPos)
{
	CHAR	c	= 0;

	if(nLine >= 0 && nLine < m_nLines)
	{
		if(nPos != NULL)
		{
			if(*nPos >= 0 && *nPos < m_sLines[nLine].m_nLine)
			{
				c		= m_sLines[nLine].m_sLine[*nPos];
				*nPos	+= 1;
			}
		}
	}
	return c;
}
CHAR* ZHeaderParser::GetWords(int nLine,int* nPos)
{
	BOOL	bWords	= FALSE;

	memset(m_sData,0,(DEFAULT_WORDS_SIZE+4));
	m_nData	= 0;

	if(nLine >= 0 && nLine < m_nLines)
	{
		if(nPos != NULL)
		{
			if(*nPos >= 0 && *nPos < m_sLines[nLine].m_nLine)
			{
				CHAR*	p	= m_sLines[nLine].m_sLine;
				int		i	= *nPos;
				for(i = *nPos; i < m_sLines[nLine].m_nLine; i ++)
				{
					if(IsWordCharacter(p[i]))
					{
						bWords				= TRUE;
						m_sData[m_nData]	= p[i];
						m_nData				++;
					}else{
						*nPos				= i;
						if(bWords)
						{
							break;
						}
					}
				}
			}
		}
	}
	return m_sData;
}
CHAR* ZHeaderParser::GetDigit(int nLine,int* nPos)
{
	BOOL	bDigits	= FALSE;

	memset(m_sData,0,(DEFAULT_WORDS_SIZE+4));
	m_nData	= 0;

	if(nLine >= 0 && nLine < m_nLines)
	{
		if(nPos != NULL)
		{
			if(*nPos >= 0 && *nPos < m_sLines[nLine].m_nLine)
			{
				CHAR*	p	= m_sLines[nLine].m_sLine;
				int		i	= *nPos;
				for(i = *nPos; i < m_sLines[nLine].m_nLine; i ++)
				{
					if(IsDigitCharacter(p[i]))
					{
						bDigits				= TRUE;
						m_sData[m_nData]	= p[i];
						m_nData				++;
					}else{
						*nPos				= i;
						if(bDigits)
						{
							break;
						}
					}
				}
			}
		}
	}
	return m_sData;
}
UINT ZHeaderParser::GetIntegerNumber(int nLine,int* nPos)
{
	UINT	nNumber	= 0;
	BOOL	bDigits	= FALSE;

	if(nLine >= 0 && nLine < m_nLines)
	{
		if(nPos != NULL)
		{
			if(*nPos >= 0 && *nPos < m_sLines[nLine].m_nLine)
			{
				CHAR*	p	= m_sLines[nLine].m_sLine;
				int		i	= *nPos;
				for(i = *nPos; i < m_sLines[nLine].m_nLine; i ++)
				{
					if(IsDigitCharacter(p[i]))
					{
						bDigits		= TRUE;
						nNumber		= (nNumber* 10 + (p[i] - '0'));
					}else{
						if(bDigits)
						{
							*nPos	= i;
							break;
						}
					}
				}
			}
		}
	}
	return nNumber;
}
DOUBLE ZHeaderParser::GetFloatNumber(int nLine,int* nPos)
{
	DOUBLE	dNumber	= 0.0;
	BOOL	bDigits	= FALSE;
	BOOL	bDot	= FALSE;
	DOUBLE	dMult	= 0.1;

	if(nLine >= 0 && nLine < m_nLines)
	{
		if(nPos != NULL)
		{
			if(*nPos >= 0 && *nPos < m_sLines[nLine].m_nLine)
			{
				CHAR*	p	= m_sLines[nLine].m_sLine;
				int		i	= *nPos;
				for(i = *nPos; i < m_sLines[nLine].m_nLine; i ++)
				{
					if(IsDigitCharacter(p[i]))
					{
						bDigits		= TRUE;
						if(bDot)
						{
							dNumber		+= (dMult * (p[i] - '0'));
							dMult		*= (DOUBLE)0.1;
						}else{
							dNumber		= (dNumber* 10 + (p[i] - '0'));
						}
					}if(p[i] == '.')
					{
						bDot	= TRUE;
					}else{
						if(bDigits)
						{
							*nPos	= i;
							break;
						}
					}
				}
			}
		}
	}

	return dNumber;
}
DOUBLE ZHeaderParser::GetTime(int nLine,int* nPos)
{
	DOUBLE	dTime		= 0;
	DOUBLE	fValue[5]	= {0,0,0,0,0};
	DOUBLE	fDiv[5]		= {1,1,1,1,1};
	UINT	nIndex		= 0;
	UINT	nType		= 0;
	BOOL	bDigits		= FALSE;

	if(nLine >= 0 && nLine < m_nLines)
	{
		if(nPos != NULL)
		{
			if(*nPos >= 0 && *nPos < m_sLines[nLine].m_nLine)
			{
				CHAR*	p	= m_sLines[nLine].m_sLine;
				int		i	= *nPos;
				for(i = *nPos; i < m_sLines[nLine].m_nLine; i ++)
				{
					if(IsDigitCharacter(p[i]))
					{
						fValue[nIndex]	= ((fValue[nIndex] * 10) + (p[i] - '0'));
						fDiv[nIndex]	*= 10;
						bDigits			= TRUE;
					}else if(p[i] == ':')
					{
						if(nIndex > 0 && nType < 1)
						{
							*nPos	= i;
							break;
						}else{
							nIndex	++;
							nType	++;
						}
					}else if(p[i] == '.')
					{
						nIndex	++;
					}else{
						if(bDigits)
						{
							*nPos	= i;
							break;
						}
					}
				}
			}
		}
	}
	if(nType)
	{
		dTime	= (fValue[0] * 3600) + (fValue[1] * 60) + fValue[2] + (fValue[3] / fDiv[3]);
	}else{
		dTime	= fValue[0] + (fValue[1] / fDiv[1]);
	}
	return dTime;
}
CHAR* ZHeaderParser::GetBlocks(int nLine,int* nPos)
{
	BOOL	bBlock	= FALSE;

	memset(m_sData,0,(DEFAULT_WORDS_SIZE+4));
	m_nData	= 0;

	if(nLine >= 0 && nLine < m_nLines)
	{
		if(nPos != NULL)
		{
			if(*nPos >= 0 && *nPos < m_sLines[nLine].m_nLine)
			{
				CHAR*	p	= m_sLines[nLine].m_sLine;
				int		i	= *nPos;
				for(i = *nPos; i < m_sLines[nLine].m_nLine; i ++)
				{
					if(IsSpaceCharacter(p[i]))
					{
						*nPos				= i;
						if(bBlock)
						{
							break;
						}
					}else{
						bBlock				= TRUE;
						m_sData[m_nData]	= p[i];
						m_nData				++;
					}
				}
			}
		}
	}
	return m_sData;
}
CHAR* ZHeaderParser::GetString(int nLine,int* nPos,CHAR cStop)
{
	BOOL	bString	= FALSE;

	memset(m_sData,0,(DEFAULT_WORDS_SIZE+4));
	m_nData	= 0;

	if(nLine >= 0 && nLine < m_nLines)
	{
		if(nPos != NULL)
		{
			if(*nPos >= 0 && *nPos < m_sLines[nLine].m_nLine)
			{
				CHAR*	p	= m_sLines[nLine].m_sLine;
				int		i	= *nPos;
				for(i = *nPos; i < m_sLines[nLine].m_nLine; i ++)
				{
					if(IsSpaceCharacter(p[i]))
					{
						*nPos				= i;
						if(bString)
						{
							break;
						}
					}else if(p[i] == cStop)
					{
						*nPos				= i;
						break;
					}else{
						bString				= TRUE;
						m_sData[m_nData]	= p[i];
						m_nData				++;
					}
				}
			}
		}
	}
	return m_sData;
}
CHAR* ZHeaderParser::GetRemain(int nLine,int* nPos)
{
	memset(m_sData,0,(DEFAULT_WORDS_SIZE+4));
	m_nData	= 0;

	if(nLine >= 0 && nLine < m_nLines)
	{
		if(nPos != NULL)
		{
			if(*nPos >= 0 && *nPos < m_sLines[nLine].m_nLine)
			{
				strcpy(m_sData,&m_sLines[nLine].m_sLine[*nPos]);
				m_nData				= m_sLines[nLine].m_nLine-*nPos;
				*nPos				=  m_sLines[nLine].m_nLine;
			}
		}
	}
	return m_sData;
}
///////////////////////////////////////////////////////////////////////////////
void ZHeaderParser::SkipSpace(int nLine,int* nPos)
{
	if(nLine >= 0 && nLine < m_nLines)
	{
		if(nPos != NULL)
		{
			if(*nPos >= 0 && *nPos < m_sLines[nLine].m_nLine)
			{
				CHAR*	p	= m_sLines[nLine].m_sLine;
				int		i	= *nPos;
				for(i = *nPos; i < m_sLines[nLine].m_nLine; i ++)
				{
					if(IsSpaceCharacter(p[i]))
					{
						*nPos	= i;
					}else{
						*nPos	= i;
						break;
					}
				}
			}
		}
	}
}
void ZHeaderParser::SkipChar(int nLine,int* nPos,CHAR cSkip)
{
	if(nLine >= 0 && nLine < m_nLines)
	{
		if(nPos != NULL)
		{
			if(*nPos >= 0 && *nPos < m_sLines[nLine].m_nLine)
			{
				CHAR*	p	= m_sLines[nLine].m_sLine;
				int		i	= *nPos;
				for(i = *nPos; i < m_sLines[nLine].m_nLine; i ++)
				{
					if(p[i] == cSkip)
					{
						*nPos	= i;
					}else{
						*nPos	= i;
						break;
					}
				}
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
UINT ZHeaderParser::IsLineBegin(int nLine,int nPos)
{
	if(nLine >= 0 && nLine < m_nLines)
	{
		return nPos;
	}
	return UINT32_MAX;
}
UINT ZHeaderParser::IsLineEnd(int nLine,int nPos)
{
	if(nLine >= 0 && nLine < m_nLines)
	{
		{
			if(nPos >= 0 && nPos < m_sLines[nLine].m_nLine)
			{
				int		i	= nPos;
				if(	((i + 1) == (m_sLines[nLine].m_nLine))		&&
					(m_sLines[nLine].m_sLine[(i)] == '\n')
					)
				{
					return 0;
				}
				if(	((i + 2) == (m_sLines[nLine].m_nLine))		&&
					(m_sLines[nLine].m_sLine[(i    )] == '\r')	&&
					(m_sLines[nLine].m_sLine[(i + 1)] == '\n')
					)
				{
					return 0;
				}
			}
		}
	}
	return UINT32_MAX;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZHeaderParser::Compare(int nLine,int nPos,CONST CHAR cCompare)
{
	if(nLine >= 0 && nLine < m_nLines)
	{
		{
			if(nPos >= 0 && nPos < m_sLines[nLine].m_nLine)
			{
				CHAR	c	= m_sLines[nLine].m_sLine[nPos];
				if(c == cCompare)
				{
					return 0;
				}
			}
		}
	}
	return UINT32_MAX;
}
UINT ZHeaderParser::Compare(int nLine,int nPos,CONST CHAR* sCompare)
{
	if(nLine >= 0 && nLine < m_nLines)
	{
		if(sCompare != NULL)
		{
			if(nPos >= 0 && nPos < m_sLines[nLine].m_nLine)
			{
				CONST	CHAR*	p	= &(m_sLines[nLine].m_sLine[nPos]);
				CONST	CHAR*	q	= sCompare;
				while(*q != '\0')
				{
					if(*p != *q)
					{
						return UINT32_MAX;
					}
					p	++;
					q	++;
				}
				return 0;
			}
		}
	}
	return UINT32_MAX;
}
UINT ZHeaderParser::CompareNoCase(int nLine,int nPos,CONST CHAR cCompare)
{
	if(nLine >= 0 && nLine < m_nLines)
	{
		{
			if(nPos >= 0 && nPos < m_sLines[nLine].m_nLine)
			{
				CHAR	c	= m_sLines[nLine].m_sLine[nPos];
				if(::tolower(c) == ::tolower(cCompare))
				{
					return 0;
				}
			}
		}
	}
	return UINT32_MAX;
}
UINT ZHeaderParser::CompareNoCase(int nLine,int nPos,CONST CHAR* sCompare)
{
	if(nLine >= 0 && nLine < m_nLines)
	{
		if(sCompare != NULL)
		{
			if(nPos >= 0 && nPos < m_sLines[nLine].m_nLine)
			{
				CONST	CHAR*	p	= &(m_sLines[nLine].m_sLine[nPos]);
				CONST	CHAR*	q	= sCompare;
				while(*q != '\0')
				{
					if(::tolower(*p) != ::tolower(*q))
					{
						return UINT32_MAX;
					}
					p	++;
					q	++;
				}
				return 0;
			}
		}
	}
	return UINT32_MAX;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZHeaderParser::Find(int nLine,int nPos,CONST CHAR sFind)
{
	UINT	nIndex	= 0;

	if(nLine >= 0 && nLine < m_nLines)
	{
		{
			for(nIndex = nPos; nIndex >= 0 && nIndex < (UINT)m_sLines[nLine].m_nLine; nIndex ++)
			{
				CHAR	c	= m_sLines[nLine].m_sLine[nIndex];
				if(c == sFind)
				{
					return nIndex;
				}
			}
		}
	}
	return UINT32_MAX;
}
UINT ZHeaderParser::Find(int nLine,int nPos,CONST CHAR* sFind)
{
	UINT	nIndex	= 0;

	if(nLine >= 0 && nLine < m_nLines)
	{
		if(sFind != NULL)
		{
			for(nIndex = nPos; nIndex >= 0 && nIndex < (UINT)m_sLines[nLine].m_nLine; nIndex ++)
			{
				CONST	CHAR*	p	= &(m_sLines[nLine].m_sLine[nIndex]);
				CONST	CHAR*	q	= sFind;
				while(*q != '\0')
				{
					if(*p != *q)
					{
						break;
					}
					p	++;
					q	++;
				}
				if(*q == '\0')
				{
					return nIndex;
				}
			}
		}
	}
	return UINT32_MAX;
}
UINT ZHeaderParser::FindNoCase(int nLine,int nPos,CONST CHAR sFind)
{
	UINT	nIndex	= 0;

	if(nLine >= 0 && nLine < m_nLines)
	{
		{
			for(nIndex = nPos; nIndex >= 0 && nIndex < (UINT)m_sLines[nLine].m_nLine; nIndex ++)
			{
				CHAR	c	= m_sLines[nLine].m_sLine[nIndex];
				if(::tolower(c) == ::tolower(sFind))
				{
					return nIndex;
				}
			}
		}
	}
	return UINT32_MAX;
}
UINT ZHeaderParser::FindNoCase(int nLine,int nPos,CONST CHAR* sFind)
{
	UINT	nIndex	= 0;

	if(nLine >= 0 && nLine < m_nLines)
	{
		if(sFind != NULL)
		{
			for(nIndex = nPos; nIndex >= 0 && nIndex < (UINT)m_sLines[nLine].m_nLine; nIndex ++)
			{
				CONST	CHAR*	p	= &(m_sLines[nLine].m_sLine[nIndex]);
				CONST	CHAR*	q	= sFind;
				while(*q != '\0')
				{
					if(::tolower(*p) != ::tolower(*q))
					{
						break;
					}
					p	++;
					q	++;
				}
				if(*q == '\0')
				{
					return nIndex;
				}
			}
		}
	}
	return UINT32_MAX;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZHeaderParser::ParseHeader(CONST CHAR* sHeader,int nHeader)
{
	CHAR*	p	= NULL;
	CHAR*	q	= NULL;
	CHAR*	l	= NULL;
	int		n	= 0;

	if(sHeader != NULL && nHeader > 0)
	{
		if(m_sHeader != NULL)
		{
			p		= (CHAR*)sHeader;
			q		= (CHAR*)sHeader;
			l		= (CHAR*)m_sHeader;
			n		= 0;
			while(*p !=  '\0')
			{
				if(n > nHeader)
				{
					break;
				}
				//\r;\r\r;\r\n;\r\n\r\n;\r\n\0
				if(*p == '\r')
				{
					m_sHeader[n]				= *p;
					p							++;
					n							++;
					//\r\r
					if(*p == '\r')
					{
						m_sHeader[n]				= *p;
						p							++;
						n							++;
						m_sLines[m_nLines].m_sLine	= l;
						m_sLines[m_nLines].m_nLine	= (p-q);
						m_nLines					++;
						q							= p;
						l							= &m_sHeader[n];
						m_nHeader					= n;
						break;
					//\r\n
					}else if(*p == '\n')
					{
						m_sHeader[n]				= *p;
						p							++;
						n							++;
						m_sLines[m_nLines].m_sLine	= l;
						m_sLines[m_nLines].m_nLine	= (p-q);
						m_nLines					++;
						q							= p;
						l							= &m_sHeader[n];
						//\r\n\r\n
						if(n >= 4)
						{
							if(*(p-4) == '\r' && *(p-3) == '\n' && *(p-2) == '\r' && *(p-1) == '\n')
							{
								m_nHeader					= n;
								break;
							}
						}
						//\r\n\0
						if(*p == '\0')
						{
							m_nHeader					= n;
							break;
						}
					}
				//\n;\n\n;\n\0
				}else if(*p == '\n')
				{
					m_sHeader[n]				= *p;
					p							++;
					n							++;
					m_sLines[m_nLines].m_sLine	= l;
					m_sLines[m_nLines].m_nLine	= (p-q);
					m_nLines					++;
					q							= p;
					l							= &m_sHeader[n];
					//\n\n
					if(n >= 2)
					{
						if(*(p-2) == '\n' && *(p-1) == '\n')
						{
							m_nHeader					= n;
							break;
						}
					}
					//\n\0
					if(*p == '\0')
					{
						m_nHeader					= n;
						break;
					}
				}else{
					m_sHeader[n]				= *p;
					p							++;
					n							++;
				}
			}
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
