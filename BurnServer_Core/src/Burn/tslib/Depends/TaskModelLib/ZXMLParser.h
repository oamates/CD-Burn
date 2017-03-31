///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZXMLParser	Header File
	Create		20100603		ZHAOTT
	Modify		20120330		ZHAOTT		ZBase
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZXMLPARSER_H_
#define	_ZXMLPARSER_H_
///////////////////////////////////////////////////////////////////////////////
#include "public.h"
#include "ZOSArray.h"
///////////////////////////////////////////////////////////////////////////////
class	ZOSFile;
class	ZXMLBase;
class	ZXMLNode;
class	ZXMLElement;
class	ZXMLParser;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ZXMLBase
{
///////////////////////////////////////////////////////////////////////////////
protected:
	static	char*	SeekTag(char* p);
///////////////////////////////////////////////////////////////////////////////
	static	char*	SkipTag(char* p);
	static	char*	SkipData(char* p);
	static	char*	SkipDTD(char* p);
	static	char*	SkipComment(char* p);
	static	char*	SkipElement(char* p);
	static	char*	SkipAttribute(char* p);
	static	char*	Skip(char* p);
///////////////////////////////////////////////////////////////////////////////
	static	char*	ParseElement(char* p,ZXMLParser* pParser);
	static	char*	Parse(char* p,ZXMLNode* pNode,ZXMLParser* pParser);
///////////////////////////////////////////////////////////////////////////////
	static	char*	ReadName(char* p,char* sName,int nName);
	static	char*	ReadValue(char* p,char* sValue,int nValue);
	static	char*	ReadAttribute(char* p,char* sAttribute,int nAttribute);
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef	ZOSArray<ZXMLNode*>	ZXMLNodeArray;
///////////////////////////////////////////////////////////////////////////////
class ZXMLNode : public ZXMLBase
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZXMLNode(UINT nDepth);
	virtual	~ZXMLNode();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	int			GetChildCount();
	virtual	ZXMLNode*	GetChild(int i);
	virtual	ZXMLNode*	AddChild(ZXMLNode* pNode);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	char*		Format(char* p,int n);
	virtual	BOOL		Save(ZOSFile* pFile);
///////////////////////////////////////////////////////////////////////////////
protected:
	UINT			m_nDepth;
	ZXMLNodeArray	m_ChildNodes;
///////////////////////////////////////////////////////////////////////////////
public:
	friend	class	ZXMLParser;
///////////////////////////////////////////////////////////////////////////////
};
class ZXMLElement : public ZXMLNode
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZXMLElement(UINT nDepth);
	ZXMLElement(const char* sName,const char* sValue,UINT nDepth);
	virtual	~ZXMLElement();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	char*	GetName();
	virtual	char*	SetName(const char* sName);
	virtual	char*	GetValue();
	virtual	char*	SetValue(const char* sValue);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	char*		Parse(char* p,ZXMLParser* pParser);
	virtual	char*		Format(char* p,int n);
	virtual	BOOL		Save(ZOSFile* pFile);
///////////////////////////////////////////////////////////////////////////////
protected:
	char*			m_sName;
	char*			m_sValue;
///////////////////////////////////////////////////////////////////////////////
public:
	friend	class	ZXMLParser;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ZXMLParser
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZXMLParser();
	virtual ~ZXMLParser();
///////////////////////////////////////////////////////////////////////////////
public:
	BOOL	ReadFile(const char* sName);
	BOOL	WriteFile(const char* sName);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	char*	OnHeaderParse(char* p,ZXMLNode* pNode);
	virtual	char*	OnDataHeaderParse(char* p,ZXMLNode* pNode);
	virtual	char*	OnDTDHeaderParse(char* p,ZXMLNode* pNode);
	virtual	char*	OnElementParse(char* p,ZXMLNode* pNode);
	virtual	char*	OnUnknownParse(char* p,ZXMLNode* pNode);
///////////////////////////////////////////////////////////////////////////////
protected:
	BOOL	Parse(const char* p);
///////////////////////////////////////////////////////////////////////////////
protected:
	char*	ParseElement(const char* p,ZXMLNode* pNode);
///////////////////////////////////////////////////////////////////////////////
protected:
	ZXMLNode*		m_pRootNode;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZXMLPARSER_H_
///////////////////////////////////////////////////////////////////////////////
