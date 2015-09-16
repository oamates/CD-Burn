#include "stdafx.h"
#include "CommonUtil.h"

int StringToInt(std::string strValue)
{
    return atoi(strValue.c_str());
}

INT64 StringToInt64(std::string strValue)
{
#ifdef WIN32
    return _atoi64(strValue.c_str());
#else
    return strtoull(strValue.c_str(), NULL, 10); 
#endif
}

std::string IntToString(int nValue)
{
    char szValue[1024];
    memset(szValue, 0, 1024);
    sprintf(szValue, "%d", nValue);
    return std::string(szValue);
}

std::string Int64ToString(INT64 nValue)
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

bool IntToBool(int nValue)
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

std::string DirFromFile(std::string strFilePath)
{
#ifdef WIN32
    char cFind = '\\';
#else
    char cFind = '/';
#endif
    return strFilePath.substr(0, strFilePath.rfind(cFind)+1);
}

wchar_t* DirFromFileW(const wchar_t* szFilePath, wchar_t* szDir)
{
    int nPosition;

    if (szFilePath != NULL && szDir != NULL)
    {
        wcscpy(szDir, szFilePath);
        nPosition = wcslen(szFilePath) - 1;
        while (
            (nPosition > 0)
            && (szDir[nPosition] != L'\\')
            && (szDir[nPosition] != L'/')
            )
        {
            szDir[nPosition] = L'\0';
            nPosition --;
        }
        if (nPosition > 0)
        {
            szDir[nPosition] = L'\0';
            return szDir;
        }
    }

    return NULL;
}
