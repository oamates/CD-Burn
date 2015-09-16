#include "StringUtil.h"

StringUtil::StringUtil()
{
    //
}

StringUtil::~StringUtil()
{
    //
}

int StringUtil::StringToInt(std::string strValue)
{
    return atoi(strValue.c_str());
}

std::string StringUtil::IntToString(int nValue)
{
    char szValue[1024];
    memset(szValue, 0, 1024);
    sprintf(szValue, "%d", nValue);
    return std::string(szValue);
}

INT64 StringUtil::StringToInt64(std::string strValue)
{
#ifdef WIN32
    return _atoi64(strValue.c_str());
#else
    return strtoull(strValue.c_str(), NULL, 10); 
#endif
}

std::string StringUtil::Int64ToString(INT64 nValue)
{
    char szValue[1024];
    memset(szValue, 0, 1024);
#ifdef WIN32
    sprintf(szValue, "%I64d", nValue);
#else
    sprintf(szValue, "%lld", nValue);
#endif
    return std::string(szValue);
}

bool StringUtil::IntToBool(int nValue)
{
    if (nValue == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool StringUtil::StringToBool(std::string strValue)
{
    if (strValue == "true")
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::string StringUtil::BoolToString(bool bValue)
{
    if (bValue)
    {
        return "true";
    }
    else
    {
        return "false";
    }
}

int StringUtil::GetSpliterElement(std::string strElements, std::string strSpliter, std::vector<std::string> &vectElements)
{
    size_t nBegin = 0;
    size_t nEnd = 0;

    if (strElements.length() > 0)
    {
        // ensure head strSpliter;
        if (strElements.find(strSpliter) != 0)
        {
            strElements = strSpliter + strElements;
        }

        // ensure end strSpliter;
        if (strElements.rfind(strSpliter) != strElements.length()-strSpliter.length())
        {
            strElements = strElements + strSpliter;
        }

        do 
        {
            nBegin = strElements.find(strSpliter, nBegin);
            if (nBegin != std::string::npos)
            {
                nBegin = nBegin+strSpliter.length();
                nEnd = strElements.find(strSpliter, nBegin);
                if (nEnd != std::string::npos)
                {
                    vectElements.push_back(strElements.substr(nBegin, nEnd-nBegin));
                    nBegin = nEnd;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        } while (true);
    }

    return 0;
}

std::string StringUtil::CombineElement(std::vector<std::string> vectSourceElement, std::string strSpliter)
{
    std::string strCombined;

    for (size_t i = 0; i < vectSourceElement.size(); i ++)
    {
        strCombined += vectSourceElement.at(i);
        if (i < vectSourceElement.size() - 1)
        {
            strCombined += strSpliter;
        }
    }

    return strCombined;
}

std::string StringUtil::GetTagValue(std::string strContent, std::string strTagName)
{
    std::string	strFind;
    size_t nBegin = 0;
    size_t nEnd = 0;
    std::string strReturn;

    strFind = "<";
    strFind += strTagName;
    strFind += ">";

    nBegin = strContent.find(strFind);
    if (nBegin != std::string::npos)
    {
        strFind = "</";
        strFind += strTagName;
        strFind += ">";

        nBegin += strTagName.length()+2;
        nEnd = strContent.find(strFind, nBegin);
        if (nEnd != std::string::npos && nEnd >= nBegin)
        {
            strReturn = strContent.substr(nBegin, nEnd-nBegin);
        }
    }

    return strReturn;
}


std::string StringUtil::GetTagValueWithStartPos(std::string strContent, std::string strTagName, int &nRelativePos)
{
    std::string	strFind;
    size_t nBegin = 0;
    size_t nEnd = 0;
    std::string strReturn;

    strFind = "<";
    strFind += strTagName;
    strFind += ">";

    nBegin = strContent.find(strFind, nRelativePos);
    nRelativePos = -1;
    if (nBegin != std::string::npos)
    {
        strFind = "</";
        strFind += strTagName;
        strFind += ">";

        nBegin += strTagName.length()+2;
        nEnd = strContent.find(strFind, nBegin);
        if (nEnd != std::string::npos && nEnd >= nBegin)
        {
            strReturn = strContent.substr(nBegin, nEnd-nBegin);
            nRelativePos = nBegin;
        }
    }

    return strReturn;
}

std::string StringUtil::MakeUpper(std::string strText)
{
    for(size_t i = 0; i < strText.size(); i ++)
    {
        if((strText[i] >= 'a') && (strText[i] <= 'z'))
        {
            strText[i] -= 32;
        }
    }

    return strText;
}

std::string StringUtil::MakeLower(std::string strText)
{
    for(size_t i = 0; i < strText.size(); i ++)
    {
        if((strText[i] >= 'A') && (strText[i] <= 'Z'))
        {
            strText[i] += 32;
        }
    }

    return strText;
}

