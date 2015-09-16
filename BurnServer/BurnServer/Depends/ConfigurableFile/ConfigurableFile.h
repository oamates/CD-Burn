#ifndef _CONFIGURABLE_FILE_H_
#define _CONFIGURABLE_FILE_H_

#include <string>
#include <map>

#include "tinyxml.h"

#define TIXML_USE_STL

class ConfigurableFile
{
public:
    ConfigurableFile();
    ~ConfigurableFile();

    bool LoadFile(const std::string &strFilePath);
    std::string GetValue(const std::string &strSectionName,const std::string &strKeyName);

private:
    TiXmlDocument m_tiXmlDoc;

    std::map<std::string,std::string>     m_mapKeyValue;
};

#endif //_CONFIGURABLE_FILE_H_

