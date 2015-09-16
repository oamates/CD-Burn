#include "stdafx.h"
#include "ConfigurableFile.h"

ConfigurableFile::ConfigurableFile()
{

}

ConfigurableFile::~ConfigurableFile()
{

}

int ConfigurableFile::LoadFile(std::string strFilePath)
{
    int nResult;
    if (m_TiXmlDoc.LoadFile(strFilePath))
    {
        TiXmlHandle hRoot(&m_TiXmlDoc);
        TiXmlHandle hXMLHandle(0);
        TiXmlElement *pElement;        
        TiXmlElement *pChildElement;
        std::string sKeyName;
        std::string sSectionName;
        int nMark;
        int nSign;

        pElement = hRoot.FirstChildElement().Element();
        hXMLHandle = TiXmlHandle(pElement);
       
        for (pElement = hXMLHandle.FirstChildElement().Element(), nMark = 0; pElement != NULL;
              pElement = pElement->NextSiblingElement())
        {
			m_mapSection.insert(make_pair(pElement->ValueStr(),pElement));
            sSectionName = pElement->ValueStr() + ",";
            hXMLHandle = TiXmlHandle(pElement);
            nMark++;
            
            for (pChildElement = hXMLHandle.FirstChildElement().Element(), nSign = 0; pChildElement != NULL;
                 pChildElement = pChildElement->NextSiblingElement())
            {
                sKeyName = sSectionName + pChildElement->ValueStr();
//                 if (pChildElement->FirstChild() == NULL 
//                     || (pChildElement->FirstChild() != NULL && pChildElement->FirstChild()->ToText()))
//                 {
                    m_mapKeyValue.insert(make_pair(sKeyName,pChildElement)); 
					nSign++;
//                 }                
            }        
        }
        if (nMark != 0 && nSign != 0)
        {
            nResult = 0;
        }
        else
        {
            nResult = -2;
        }
        
    }
    else
    {
        nResult = -1;
    }
    
    return nResult;      
}

int ConfigurableFile::SaveFile(std::string strFilePath)
{
    int nReturn;

    //if strFilePath is "", use strFilePath from LoadFile.
    if (strFilePath == "")
    {
        strFilePath = m_TiXmlDoc.Value();
    }

    //save file
    if (m_TiXmlDoc.SaveFile(strFilePath))
    {
        nReturn = 0;
    }
    else
    {
        nReturn = -1;
    }

    return nReturn;
}

std::string ConfigurableFile::GetValue(std::string sSectionName, std::string sKeyName, bool *bExist)
{
    std::string sValue = "";
    std::map<std::string, TiXmlNode *>::iterator iter;

    iter = m_mapKeyValue.find(sSectionName + "," + sKeyName);
    if(iter != m_mapKeyValue.end())
    {
        if (iter->second->FirstChild() != NULL)
        {
            sValue = iter->second->FirstChild()->ValueStr();
        }        
        if (bExist != NULL)
        {
            *bExist = true;
        }        
    }
    else
    {
        if (bExist != NULL)
        {
            *bExist = false;
        }
    }

    return sValue;

}

bool ConfigurableFile::SetValue(std::string sSectionName, std::string sKeyName, std::string sKeyValue)
{
    bool bReturn = false;
    std::map<std::string, TiXmlNode *>::iterator iter;
    
    iter = m_mapKeyValue.find(sSectionName + "," + sKeyName);
    if(iter != m_mapKeyValue.end())
    {
        if (iter->second->FirstChild() != NULL)
        {
            iter->second->FirstChild()->SetValue(sKeyValue);
        }
        else
        {
            iter->second->InsertEndChild(TiXmlText(sKeyValue));
        }
        bReturn = true;
    }
        
    return bReturn;
}

TiXmlNode* ConfigurableFile::GetKeyNode(std::string sSectionName, std::string sKeyName)
{
    TiXmlNode* keyNode;
    keyNode = NULL;
    std::map<std::string, TiXmlNode *>::iterator iterKey;
    std::map<std::string, TiXmlNode *>::iterator iterSection;

    std::string str = sSectionName + "," + sKeyName;
    iterKey = m_mapKeyValue.find(str);
    iterSection = m_mapSection.find(sSectionName);
    if (sKeyName != "")
    {
        if (iterKey != m_mapKeyValue.end())
        {
            keyNode = iterKey->second;
        }
    }
    else
    {
        if (iterSection != m_mapSection.end())
        {
            keyNode = iterSection->second;
        }
    }    
    return keyNode;
}

bool ConfigurableFile::SetKeyNode(const TiXmlNode& newNode,std::string sSectionName, std::string sKeyName)
{
    bool bReturn = false;
    TiXmlNode* keyNode;
    keyNode = NULL;

    std::map<std::string, TiXmlNode *>::iterator iterKey;
    std::map<std::string, TiXmlNode *>::iterator iterSection;

    iterKey = m_mapKeyValue.find(sSectionName + "," + sKeyName);
    iterSection = m_mapSection.find(sSectionName);

    if(iterKey != m_mapKeyValue.end() && iterSection != m_mapSection.end())
    {
        keyNode = iterSection->second->ReplaceChild(iterKey->second,newNode);
        if (keyNode != NULL)
        {
            m_mapKeyValue.erase(iterKey);
            m_mapKeyValue.insert(make_pair(sSectionName + "," +sKeyName,keyNode));
            bReturn = true;
        } 
    }

    return bReturn;
}
int ConfigurableFile::AddKeyNode(std::string sAddSectionName, const TiXmlNode& AddNode, std::string sAfterThisKey/* = ""*/, bool bInsertHead/* = false*/)
{
    int nReturn = 0;
    TiXmlElement AddSectionName(sAddSectionName);
    AddSectionName.InsertEndChild(AddNode);
    std::map<std::string, TiXmlNode *>::iterator iterKey;
    std::map<std::string, TiXmlNode *>::iterator iterSection;
    iterSection = m_mapSection.find(sAddSectionName);

    if (sAfterThisKey == "")
    {//means add to the first or last of the section named @sAddSectionName
        if(iterSection != m_mapSection.end())
        {//means the section named @sAddSectionName exist
            if (!bInsertHead)
            {//means add to the last
                iterSection->second->InsertEndChild(AddNode);
                m_mapKeyValue.insert(make_pair(sAddSectionName + "," + AddNode.ValueStr(),iterSection->second->LastChild()));
            } 
            else
            {//means add to the first
                iterSection->second->InsertBeforeChild(iterSection->second->FirstChild(),AddNode);
                m_mapKeyValue.insert(make_pair(sAddSectionName + "," + AddNode.ValueStr(),iterSection->second->FirstChild()));
            }
        }
        else
        {//means add a new section named @sAddSectionName
            m_TiXmlDoc.RootElement()->InsertEndChild(AddSectionName);
            m_mapSection.insert(make_pair(sAddSectionName,m_TiXmlDoc.RootElement()->LastChild()));
            m_mapKeyValue.insert(make_pair(sAddSectionName + "," + AddNode.ValueStr(),m_TiXmlDoc.RootElement()->LastChild()->FirstChild()));
        }		
    } 
    else
    {//means add to the key named @sAfterThisKey
        iterKey = m_mapKeyValue.find(sAddSectionName + "," + sAfterThisKey);

        if (iterKey != m_mapKeyValue.end())
        {//means the section and key exist
            iterKey->second->Parent()->InsertAfterChild(iterKey->second,AddNode);
            m_mapKeyValue.insert(make_pair(sAddSectionName + "," +AddNode.ValueStr(),iterKey->second->NextSiblingElement()));
        } 
        else
        {//means the section or the key does not exist
            if (iterSection != m_mapSection.end())
            {//means the section exist but key not
                nReturn = -1;			
            }
            else
            {//means the section does not exist
                nReturn = -2;		
            }						
        }		
    }

    return nReturn;
}

int ConfigurableFile::AddKey(std::string sAddSectionName, std::string sAddKeyName, std::string sAddKeyValue, std::string sAfterThisKey /* = "" */, bool bInsertHead /* = false */)
{
    int nReturn = 0;
	TiXmlText AddKeyValue(sAddKeyValue);
	TiXmlElement AddKeyName(sAddKeyName);
	AddKeyName.InsertEndChild(AddKeyValue);
	TiXmlElement AddSectionName(sAddSectionName);
	AddSectionName.InsertEndChild(AddKeyName);

	
	std::map<std::string, TiXmlNode *>::iterator iterKey;
	std::map<std::string, TiXmlNode *>::iterator iterSection;
	iterSection = m_mapSection.find(sAddSectionName);

	if (sAfterThisKey == "")
	{//means add to the first or last of the section named @sAddSectionName
		if(iterSection != m_mapSection.end())
		{//means the section named @sAddSectionName exist
			if (!bInsertHead)
			{//means add to the last
				iterSection->second->InsertEndChild(AddKeyName);
				m_mapKeyValue.insert(make_pair(sAddSectionName + "," +sAddKeyName,iterSection->second->LastChild()));
			} 
			else
			{//means add to the first
				iterSection->second->InsertBeforeChild(iterSection->second->FirstChild(),AddKeyName);
				m_mapKeyValue.insert(make_pair(sAddSectionName + "," +sAddKeyName,iterSection->second->FirstChild()));
			}
		}
		else
		{//means add a new section named @sAddSectionName
			m_TiXmlDoc.RootElement()->InsertEndChild(AddSectionName);
			m_mapSection.insert(make_pair(sAddSectionName,m_TiXmlDoc.RootElement()->LastChild()));
			m_mapKeyValue.insert(make_pair(sAddSectionName + "," +sAddKeyName,m_TiXmlDoc.RootElement()->LastChild()->FirstChild()));
		}		
	} 
	else
	{//means add to the key named @sAfterThisKey
		iterKey = m_mapKeyValue.find(sAddSectionName + "," + sAfterThisKey);

		if (iterKey != m_mapKeyValue.end())
		{//means the section and key exist
			iterKey->second->Parent()->InsertAfterChild(iterKey->second,AddKeyName);
			m_mapKeyValue.insert(make_pair(sAddSectionName + "," +sAddKeyName,iterKey->second->NextSiblingElement()));
		} 
		else
		{//means the section or the key does not exist
			if (iterSection != m_mapSection.end())
			{//means the section exist but key not
				nReturn = -1;			
			}
			else
			{//means the section does not exist
				nReturn = -2;		
			}						
		}		
	}

    return nReturn;
}

bool ConfigurableFile::DeleteKey(std::string sSectionName, std::string sKeyName, std::string sKeyValue /* = "" */)
{
    bool bReturn = false;
    std::map<std::string, TiXmlNode *>::iterator iter;

    iter = m_mapKeyValue.find(sSectionName + "," + sKeyName);
    if(iter != m_mapKeyValue.end() && (sKeyValue == "" || sKeyValue ==(iter->second)->FirstChild()->ValueStr()))
    {
        iter->second->Parent()->RemoveChild(iter->second);
		m_mapKeyValue.erase(iter);
        bReturn = true;
    }

    return bReturn;
}

bool ConfigurableFile::DeleteSection(std::string sSectionName)
{
	bool bReturn = false;
	std::map<std::string, TiXmlNode *>::iterator iterSection;
	std::map<std::string, TiXmlNode *>::iterator iterKey;
	TiXmlElement *pElement;

	iterSection = m_mapSection.find(sSectionName);
	if(iterSection != m_mapSection.end())
	{
		for (pElement = iterSection->second->FirstChildElement(); pElement != NULL; pElement = pElement->NextSiblingElement())
		{
			iterKey = m_mapKeyValue.find(sSectionName +"," + pElement->ValueStr());
			if (iterKey != m_mapKeyValue.end())
			{
				m_mapKeyValue.erase(iterKey);
			}
		}
		iterSection->second->Parent()->RemoveChild(iterSection->second);
		m_mapSection.erase(iterSection);

		bReturn = true;
	}

	return bReturn;
}
