#include "ZXMLParser.h"
#include "ZOSMemory.h"
#include "ZOSFile.h"
#include <ctype.h>
///////////////////////////////////////////////////////////////////////////////
//#define	DEBUG_XML_PARSE		1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL IsSpace(unsigned char c)
{
	return ( isspace( (unsigned char) c ) || c == '\n' || c == '\r' ); 
}
unsigned char* SkipSpace(unsigned char* p)
{
	if(p != NULL)
	{
		while( *p && IsSpace(*p))
		{
			++p;
		}
	}
	return p;
}
BOOL IsAlpha(unsigned char c)
{
	if(c < 127)
	{
		return isalpha(c);
	}
	return FALSE;
}
BOOL IsAlphaNumber(unsigned char c)
{
	if(c < 127)
	{
		return isalnum(c);
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
char* ZXMLBase::SeekTag(char* p)
{
	do{
		while(p && *p && *p != '<')
		{
			++p;
		}
	}while(FALSE);

	return p;
}
char* ZXMLBase::ReadName(char* p,char* sName,int nName)
{
	int		i		= 0;

	memset(sName,0,nName);

	if(p && *p && (IsAlpha(*p) || *p == '-'))
	{
		while(p && *p && (IsAlphaNumber(*p) || *p == '-' || *p == '_' || *p == '.' || *p == ':'))
		{
			if(i < (nName-1))
			{
				sName[i]	= *p;
				i++;
			}
			++p;
		}
	}
	return p;
}
char* ZXMLBase::ReadValue(char* p,char* sValue,int nValue)
{
	int		i		= 0;

	memset(sValue,0,nValue);


	while(p && *p)
	{
		if(*p == '<')
		{
			break;
		}
		if(i < (nValue-1))
		{
			sValue[i]	= *p;
			i++;
		}
		++p;
	}
	return p;
}
char* ZXMLBase::ReadAttribute(char* p,char* sAttribute,int nAttribute)
{
	int		i			= 0;

	memset(sAttribute,0,nAttribute);

	do{
		if( !p || !*p )
		{
			break;
		}
		while ( p && *p && !IsSpace(*p) && *p != '/' && (*p != '>'))
		{
			if(i < (nAttribute-1))
			{
				sAttribute[i]	= *p;
				i++;
			}
			++p;
		}
	}while(FALSE);

	return p;
}
char* ZXMLBase::SkipTag(char* p)
{
	p	= (char*)SkipSpace((unsigned char*)p);
	do{
		if( !p || !*p || *p != '<' )
		{
			break;
		}
		while(p && *p && *p != '>')
		{
			++p;
		}
		if(p && *p)
		{
			++p;
		}
	}while(FALSE);

	return p;
}
char* ZXMLBase::SkipData(char* p)
{
	p	= (char*)SkipSpace((unsigned char*)p);
	do{
		if( !p || !*p || (strncasecmp(p,"<![CDATA[",8) != 0) )
		{
			break;
		}
		while(p && *p && (strncasecmp(p,"]]>",3) != 0))
		{
			++p;
		}
		if(p && *p)
		{
			p	+= 3;
		}
	}while(FALSE);

	return p;
}
char* ZXMLBase::SkipDTD(char* p)
{
	p	= (char*)SkipSpace((unsigned char*)p);
	do{
		if( !p || !*p || (strncasecmp(p,"<!",2) != 0) )
		{
			break;
		}
		while(p && *p && *p != '>' )
		{
			++p;
			if(*p == '<')
			{
				p	= ZXMLBase::Skip(p);
			}
		}
		if(p && *p)
		{
			++p;
		}
	}while(FALSE);

	return p;
}
char* ZXMLBase::SkipComment(char* p)
{
	p	= (char*)SkipSpace((unsigned char*)p);
	do{
		if( !p || !*p || *p != '<' )
		{
			break;
		}
		if(strncasecmp(p,"<!--",4) != 0)
		{
			break;
		}
		while(p && *p && (strncasecmp(p,"-->",3) != 0))
		{
			++p;
		}
		if(p && *p)
		{
			p+=3;
		}
	}while(FALSE);

	return p;
}
char* ZXMLBase::SkipElement(char* p)
{
	p	= (char*)SkipSpace((unsigned char*)p);
	do{
		if( !p || !*p || *p != '<' )
		{
			break;
		}
		while ( p && *p && (*p != '>' && *p != '/'))
		{
			++p;
		}
		while ( p && *p )
		{
			p	= (char*)SkipSpace((unsigned char*)p);
			//"/>"
			if(*p == '/')
			{
				++p;
				//"/>"
				if(*p == '>')
				{
					break;
				}
			//"</xxx>"
			}else if( *p == '<')
			{
				//"</xxx>"
				if(*(p + 1) == '/')
				{
					while( p && *p && *p != '>')
					{
						++p;
					}
					break;
				//"<xxx>"
				}else{
					p	= ZXMLBase::Skip((char*)p);
				}
			//">xxx<"
			}else if(*p == '>')
			{
				p	= ZXMLBase::SeekTag((char*)p);
			//"xxx>"
			}else{
				p	= ZXMLBase::SkipAttribute((char*)p);
			}
		}
		if(p && *p)
		{
			++p;
		}
	}while(FALSE);

	return p;
}
char* ZXMLBase::SkipAttribute(char* p)
{
	do{
		if( !p || !*p )
		{
			break;
		}
		while ( p && *p && !IsSpace(*p) && *p != '/' && (*p != '>'))
		{
			++p;
		}
	}while(FALSE);
	return p;
}
///////////////////////////////////////////////////////////////////////////////
char* ZXMLBase::Skip(char* p)
{
	p	= (char*)SkipSpace((unsigned char*)p);
	do{
		if ( !p || !*p || *p != '<' )
		{
			break;
		}
		if(*(p + 1) == '/')
		{
			break;
		}
		//xml header
		if(strncasecmp(p,"<?xml",5)==0)
		{
			p	= ZXMLBase::SkipTag((char*)p);
		//xml comment
		}else if(strncasecmp(p,"<!--",4)==0)
		{
			p	= ZXMLBase::SkipComment((char*)p);
		//data header
		}else if(strncasecmp(p,"<![CDATA[",8)==0)
		{
			p	= ZXMLBase::SkipData((char*)p);
		//dtd header
		}else if(strncasecmp(p,"<!",2)==0)
		{
			p	= ZXMLBase::SkipDTD((char*)p);
		//node
		}else if(IsAlpha(*(p+1)) || *(p+1) == '-')
		{
			p	= ZXMLBase::SkipElement((char*)p);
		//unknown
		}else{
			p	= ZXMLBase::SkipTag((char*)p);
		}
	}while(FALSE);
	return (char*)p;
}
///////////////////////////////////////////////////////////////////////////////
char* ZXMLBase::ParseElement(char* p,ZXMLParser* pParser)
{
	char	s[1024];

	do{
		p	= (char*)SkipSpace((unsigned char*)p);
		if ( !p || !*p || *p != '<' )
		{
			break;
		}
		p	= (char*)SkipSpace((unsigned char*)(p+1));
		p	= ZXMLBase::ReadName(p,s,sizeof(s));
#ifdef	DEBUG_XML_PARSE
		if(s)
		{
			printf("<%s>\r\n",s);
		}
#endif	//DEBUG_XML_PARSE
		while ( p && *p )
		{
			p	= (char*)SkipSpace((unsigned char*)p);
			//"/>"
			if(*p == '/')
			{
				++p;
				//"/>"
				if(*p == '>')
				{
					break;
				}
			//"</xxx>"
			}else if( *p == '<')
			{
				//"</xxx>"
				if(*(p + 1) == '/')
				{
					++p;
					++p;
					p	= ZXMLBase::ReadName(p,s,sizeof(s));
#ifdef	DEBUG_XML_PARSE
		if(s)
		{
			printf("<%s>\r\n",s);
		}
#endif	//DEBUG_XML_PARSE
					break;
				//"<xxx>"
				}else{
					p	= ZXMLNode::Parse((char*)p,NULL,pParser);
				}
			//">xxx<"
			}else if(*p == '>')
			{
				++p;
				p	= (char*)SkipSpace((unsigned char*)(p));
				p	= ZXMLBase::ReadValue(p,s,sizeof(s));
#ifdef	DEBUG_XML_PARSE
		if(s)
		{
			printf("<%s>\r\n",s);
		}
#endif	//DEBUG_XML_PARSE
			//"xxx>"
			}else{
				p	= ZXMLBase::SkipAttribute((char*)p);
			}
		}
		if(*p == '>')
		{
			++p;
		}
	}while(FALSE);

	return p;
}
char* ZXMLBase::Parse(char* p,ZXMLNode* pNode,ZXMLParser* pParser)
{
	if(pParser != NULL)
	{
		p	= (char*)SkipSpace((unsigned char*)p);
		do{
			if ( !p || !*p || *p != '<' )
			{
				break;
			}
			if(*(p + 1) == '/')
			{
				break;
			}
			//xml header
			if(strncasecmp(p,"<?xml",5)==0)
			{
				p	= pParser->OnHeaderParse((char*)p,pNode);
			//xml comment
			}else if(strncasecmp(p,"<!--",4)==0)
			{
				p	= ZXMLBase::SkipComment((char*)p);
			//data header
			}else if(strncasecmp(p,"<![CDATA[",8)==0)
			{
				p	= pParser->OnDataHeaderParse((char*)p,pNode);
			//dtd header
			}else if(strncasecmp(p,"<!",2)==0)
			{
				p	= pParser->OnDTDHeaderParse((char*)p,pNode);
			//node
			}else if(IsAlpha(*(p+1)) || *(p+1) == '-')
			{
				p	= pParser->OnElementParse((char*)p,pNode);
			//unknown
			}else{
				p	= pParser->OnUnknownParse((char*)p,pNode);
			}
		}while(FALSE);
	}else{
		p	= ZXMLBase::Skip((char*)p);
	}
	return (char*)p;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZXMLNode::ZXMLNode(UINT nDepth)
:m_nDepth(nDepth)
,m_ChildNodes(8)
{
}
ZXMLNode::~ZXMLNode()
{
	if(m_ChildNodes.Count() > 0)
	{
		int	i	= 0;
		for(i = 0; i < m_ChildNodes.Count(); i ++)
		{
			if(m_ChildNodes[i] != NULL)
			{
				delete m_ChildNodes[i];
			}
		}
		m_ChildNodes.RemoveAll();
	}
}
///////////////////////////////////////////////////////////////////////////////
int ZXMLNode::GetChildCount()
{
	return m_ChildNodes.Count();
}
ZXMLNode* ZXMLNode::GetChild(int i)
{
	return m_ChildNodes[i];
}
ZXMLNode* ZXMLNode::AddChild(ZXMLNode* pNode)
{
	m_ChildNodes.Add(pNode);
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
char* ZXMLNode::Format(char* p,int n)
{
	if(m_ChildNodes.Count() > 0)
	{
		int	i	= 0;
		for(i = 0; i < m_ChildNodes.Count(); i ++)
		{
			m_ChildNodes[i]->Format(p,n);
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZXMLNode::Save(ZOSFile* pFile)
{
	if(m_ChildNodes.Count() > 0)
	{
		int	i	= 0;
		for(i = 0; i < m_ChildNodes.Count(); i ++)
		{
			m_ChildNodes[i]->Save(pFile);
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZXMLElement::ZXMLElement(UINT nDepth)
:ZXMLNode(nDepth)
,m_sName(NULL)
,m_sValue(NULL)
{
}
ZXMLElement::ZXMLElement(const char* sName,const char* sValue,UINT nDepth)
:ZXMLNode(nDepth)
,m_sName(NULL)
,m_sValue(NULL)
{
	ZXMLElement::SetName(sName);
	ZXMLElement::SetValue(sValue);
}
ZXMLElement::~ZXMLElement()
{
	SAFE_DELETE_ARRAY(m_sName);

	SAFE_DELETE_ARRAY(m_sValue);
}
///////////////////////////////////////////////////////////////////////////////
char* ZXMLElement::GetName()
{
	return m_sName;
}
char* ZXMLElement::SetName(const char* sName)
{
	TMASSERT((m_sName==NULL));

	SAFE_DELETE_ARRAY(m_sName);

	if(sName != NULL)
	{
		if(m_sName == NULL)
		{
			m_sName	= NEW char[(strlen(sName)+4)];
		}
		if(m_sName != NULL)
		{
			strcpy(m_sName,sName);
		}
	}
	return m_sName;
}
char* ZXMLElement::GetValue()
{
	return m_sValue;
}
char* ZXMLElement::SetValue(const char* sValue)
{
	SAFE_DELETE_ARRAY(m_sValue);

	if(sValue != NULL)
	{
		if(m_sValue == NULL)
		{
			m_sValue	= NEW char[(strlen(sValue)+4)];
		}
		if(m_sValue != NULL)
		{
			strcpy(m_sValue,sValue);
		}
	}
	return m_sValue;
}
///////////////////////////////////////////////////////////////////////////////
char* ZXMLElement::Parse(char* p,ZXMLParser* pParser)
{
	char	s[1024];

	do{
		p	= (char*)SkipSpace((unsigned char*)p);
		if ( !p || !*p || *p != '<' )
		{
			break;
		}
		p	= (char*)SkipSpace((unsigned char*)(p+1));
		p	= ZXMLBase::ReadName(p,s,sizeof(s));
		ZXMLElement::SetName(s);
		while ( p && *p )
		{
			p	= (char*)SkipSpace((unsigned char*)p);
			//"/>"
			if(*p == '/')
			{
				++p;
				//"/>"
				if(*p == '>')
				{
					break;
				}
			//"</xxx>"
			}else if( *p == '<')
			{
				//"</xxx>"
				if(*(p + 1) == '/')
				{
					++p;
					++p;
					p	= ZXMLBase::ReadName(p,s,sizeof(s));
					break;
				//"<xxx>"
				}else{
					p	= ZXMLNode::Parse((char*)p,this,pParser);
				}
			//">xxx<"
			}else if(*p == '>')
			{
				++p;
				p	= (char*)SkipSpace((unsigned char*)(p));
				p	= ZXMLBase::ReadValue(p,s,sizeof(s));
				ZXMLElement::SetValue(s);
			//"xxx>"
			}else{
				p	= ZXMLBase::SkipAttribute((char*)p);
			}
		}
		if(*p == '>')
		{
			++p;
		}
	}while(FALSE);

	return p;
}
char* ZXMLElement::Format(char* sXMLData,int nXMLData)
{
	if(m_ChildNodes.Count() > 0)
	{
		int	i	= 0;
		for(i = 0; i < (int)m_nDepth; i ++)
		{
			printf(" ");
		}
		printf("<%s>\r\n",m_sName);

		for(i = 0; i < (int)m_ChildNodes.Count(); i ++)
		{
			m_ChildNodes[i]->Format(sXMLData,nXMLData);
		}
		TMASSERT((m_sValue==NULL));
		for(i = 0; i < (int)m_nDepth; i ++)
		{
			printf(" ");
		}
		printf("</%s>\r\n",m_sName);
	}else{
		int	i	= 0;
		for(i = 0; i < (int)m_nDepth; i ++)
		{
			printf(" ");
		}
		if(m_sValue != NULL)
		{
			printf("<%s>%s</%s>\r\n",m_sName,m_sValue,m_sName);
		}else{
			printf("<%s/>\r\n",m_sName);
		}
	}
	return 0;
}
BOOL ZXMLElement::Save(ZOSFile* pFile)
{
	char	sSave[2048];
	int		i	= 0;

	if(pFile != NULL)
	{
		if(m_ChildNodes.Count() > 0)
		{
			memset(sSave,0,sizeof(sSave));
			for(i = 0; i < (int)m_nDepth; i ++)
			{
				strcat(sSave," ");
			}
			pFile->Write(sSave,strlen(sSave));

			sprintf(sSave,"<%s>\r\n",m_sName);
			pFile->Write(sSave,strlen(sSave));

			for(i = 0; i < (int)m_ChildNodes.Count(); i ++)
			{
				m_ChildNodes[i]->Save(pFile);
			}

			//TMASSERT((m_sValue==NULL));
			memset(sSave,0,sizeof(sSave));
			for(i = 0; i < (int)m_nDepth; i ++)
			{
				strcat(sSave," ");
			}
			pFile->Write(sSave,strlen(sSave));

			sprintf(sSave,"</%s>\r\n",m_sName);
			pFile->Write(sSave,strlen(sSave));

		}else{
			memset(sSave,0,sizeof(sSave));
			for(i = 0; i < (int)m_nDepth; i ++)
			{
				strcat(sSave," ");
			}
			pFile->Write(sSave,strlen(sSave));

			if(m_sValue != NULL)
			{
				sprintf(sSave,"<%s>%s</%s>\r\n",m_sName,m_sValue,m_sName);
			}else{
				sprintf(sSave,"<%s/>\r\n",m_sName);
			}
			pFile->Write(sSave,strlen(sSave));

		}
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZXMLParser::ZXMLParser()
:m_pRootNode(NULL)
{
}
ZXMLParser::~ZXMLParser()
{
	SAFE_DELETE(m_pRootNode);
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZXMLParser::ReadFile(const char* sName)
{
	BOOL	bReturn	= FALSE;

	SAFE_DELETE(m_pRootNode);

	if(sName != NULL)
	{
		ZOSFile	File;
		if(File.Open(sName,0))
		{
			if(File.GetSize() > 0 && !File.IsDir())
			{
				int		nData	= (int)File.GetSize();
				char*	sData	= NEW char[(nData+4)];
				if(sData != NULL)
				{
					memset(sData,0,(nData+4));
					if(File.Read(sData,nData) == nData)
					{
						bReturn	= ZXMLParser::Parse(sData);
					}
					SAFE_DELETE_ARRAY(sData);
				}
			}
			File.Close();
		}
	}
	return bReturn;
}
BOOL ZXMLParser::WriteFile(const char* sName)
{
	BOOL	bReturn	= FALSE;

	if(sName != NULL)
	{
		if(m_pRootNode != NULL)
		{
			ZOSFile	File;
			if(File.Create(sName))
			{
				m_pRootNode->Save(&File);
			}else{
				DEBUG_OUT(("ZXMLParser::WriteFile ERROR(Name = %s)\r\n",sName));
			}
		}
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZXMLParser::Parse(const char* sXMLData)
{
	BOOL	bReturn	= FALSE;
	char*	p		= (char*)sXMLData;
	char*	q		= (char*)sXMLData;
	
	while( p && *p )
	{
		p	= ZXMLBase::Parse(p,NULL,this);
		if(p == q) break;
		q = p;
	}
	if(p > sXMLData)
	{
		bReturn	= (p && !*p);
	}

	return bReturn;
}
char* ZXMLParser::ParseElement(const char* p,ZXMLNode* pNode)
{
	int				nDepth		= (pNode != NULL ? (pNode->m_nDepth + 1) : 0);
	ZXMLElement*	pElement	= NEW ZXMLElement(nDepth);
	if(pElement != NULL)
	{
		p	= pElement->Parse((char*)p,this);
		if(pNode != NULL)
		{
			pNode->AddChild(pElement);
		}else{
			m_pRootNode	= pElement;
		}
	}
	return (char*)p;
}
///////////////////////////////////////////////////////////////////////////////
char* ZXMLParser::OnHeaderParse(char* p,ZXMLNode* pNode)
{
	return ZXMLBase::SkipTag((char*)p);
}
char* ZXMLParser::OnDataHeaderParse(char* p,ZXMLNode* pNode)
{
	return ZXMLBase::SkipData((char*)p);
}
char* ZXMLParser::OnDTDHeaderParse(char* p,ZXMLNode* pNode)
{
	return ZXMLBase::SkipDTD((char*)p);
}
char* ZXMLParser::OnElementParse(char* p,ZXMLNode* pNode)
{
	return ZXMLBase::ParseElement(p,this);
}
char* ZXMLParser::OnUnknownParse(char* p,ZXMLNode* pNode)
{
	return ZXMLBase::SkipTag((char*)p);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
