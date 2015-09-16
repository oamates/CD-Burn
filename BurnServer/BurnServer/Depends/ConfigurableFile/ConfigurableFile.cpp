#include "ConfigurableFile.h"

ConfigurableFile::ConfigurableFile()
{

}

ConfigurableFile::~ConfigurableFile()
{
}

bool ConfigurableFile::LoadFile(const std::string &strFilePath)
{
    bool bRet=false;

    if (m_tiXmlDoc.LoadFile(strFilePath.c_str(),TIXML_ENCODING_UTF8))
    {
        bRet=true;

        TiXmlNode *pRoot=m_tiXmlDoc.RootElement();

        TiXmlNode *pSection;
        for (pSection=pRoot->FirstChild();pSection!=NULL;pSection=pSection->NextSibling())
        {
            const char *section=pSection->Value();

            TiXmlNode *pItem;
            for (pItem=pSection->FirstChild();pItem!=NULL;pItem=pItem->NextSibling())
            {
                const char *key=pItem->Value();

                if (key != NULL)
                {
                    std::string strKey=std::string(section)+std::string(":")+std::string(key);

                    m_mapKeyValue[strKey]=pItem->ToElement()->GetText();
                }
            }
        }
    }

    return bRet;
}


std::string ConfigurableFile::GetValue(const std::string &strSectionName,const std::string &strKeyName)
{
    std::string str;

    std::string strKey=strSectionName+std::string(":")+strKeyName;

    std::map<std::string,std::string>::iterator iter;
    iter=m_mapKeyValue.find(strKey);
    if (iter != m_mapKeyValue.end())
    {
        str=iter->second;
    }

    return str;
}
