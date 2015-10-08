///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project			ZMediaServer
	ZHeaderParser	Header File
	Create			20100728		ZHAOTT		Header Parser
	Modify			20110602		ZHAOTT		Header Parser
	Modify			20120502		ZHAOTT		ZProtocol
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZHEADERPARSER_H_
#define	_ZHEADERPARSER_H_
///////////////////////////////////////////////////////////////////////////////
#include "Common.h"
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_HEADER_EOL		"\r\n"
#define	DEFAULT_HEADER_COLON	":"
///////////////////////////////////////////////////////////////////////////////
class	ZHeaderParser;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ZHeaderParser
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZHeaderParser(CONST CHAR* sHeader,int nHeader);
	virtual ~ZHeaderParser();
///////////////////////////////////////////////////////////////////////////////
public:
	CHAR*	GetHeader();
	UINT	GetHeaderSize();
	UINT	GetLineCount();
///////////////////////////////////////////////////////////////////////////////
public:
	CHAR*	GetLine(int nLine);
	CHAR	GetChar(int nLine,int* nPos);
	CHAR*	GetWords(int nLine,int* nPos);
	CHAR*	GetDigit(int nLine,int* nPos);
	UINT	GetIntegerNumber(int nLine,int* nPos);
	DOUBLE	GetFloatNumber(int nLine,int* nPos);
	DOUBLE	GetTime(int nLine,int* nPos);
	CHAR*	GetBlocks(int nLine,int* nPos);
	CHAR*	GetString(int nLine,int* nPos,CHAR cStop);
	CHAR*	GetRemain(int nLine,int* nPos);
	VOID	SkipSpace(int nLine,int* nPos);
	VOID	SkipChar(int nLine,int* nPos,CHAR cSkip);
	UINT	IsLineBegin(int nLine,int nPos);
	UINT	IsLineEnd(int nLine,int nPos);
	UINT	Compare(int nLine,int nPos,CONST CHAR cCompare);
	UINT	Compare(int nLine,int nPos,CONST CHAR* sCompare);
	UINT	CompareNoCase(int nLine,int nPos,CONST CHAR cCompare);
	UINT	CompareNoCase(int nLine,int nPos,CONST CHAR* sCompare);
	UINT	Find(int nLine,int nPos,CONST CHAR sFind);
	UINT	Find(int nLine,int nPos,CONST CHAR* sFind);
	UINT	FindNoCase(int nLine,int nPos,CONST CHAR sFind);
	UINT	FindNoCase(int nLine,int nPos,CONST CHAR* sFind);
///////////////////////////////////////////////////////////////////////////////
protected:
	BOOL	ParseHeader(CONST CHAR* sHeader,int nHeader);
///////////////////////////////////////////////////////////////////////////////
protected:
	struct	HEADER_LINE
	{
		CHAR*		m_sLine;
		int			m_nLine;
	};
///////////////////////////////////////////////////////////////////////////////
protected:
	CHAR*			m_sHeader;
	int				m_nHeader;
	HEADER_LINE*	m_sLines;
	int				m_nLines;
	CHAR*			m_sData;
	int				m_nData;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZHEADERPARSER_H_
///////////////////////////////////////////////////////////////////////////////
