#ifndef _STRING_UTIL_H_
#define _STRING_UTIL_H_

#include <string>
#include <vector>

//todo add all use example

#ifndef UINT64
#ifdef WIN32
typedef unsigned __int64    UINT64;
#else
typedef unsigned long long	UINT64;
#endif
#endif//UINT64

#ifndef INT64
#ifdef WIN32
typedef __int64				INT64;
#else
typedef signed long long	INT64;
#endif
#endif//INT64

class StringUtil
{
public:
    static int StringToInt(std::string strValue);
    static std::string IntToString(int nValue);
    static INT64 StringToInt64(std::string strValue);
    static std::string Int64ToString(INT64 nValue);
    static bool IntToBool(int nValue);
    static bool StringToBool(std::string strValue);
    static std::string BoolToString(bool bValue);
    /**
     *
     * strElements = "abc|cde|efghijk|aaa"
     * strSpliter = "|"
     * vectElements = {"abc", "cde", "efghijk", "aaa"}
     * 
     * 
     */
    static int GetSpliterElement(std::string strElements, std::string strSpliter, std::vector<std::string> &vectElements);

    static std::string CombineElement(std::vector<std::string> vectSourceElement, std::string strSpliter);

    /**
     *
     * @
     * @
     * @
     */
    static std::string GetTagValue(std::string strContent, std::string strTagName);

    static std::string GetTagValueWithStartPos(std::string strContent, std::string strTagName, int &nRelativePos);

    static std::string MakeUpper(std::string strText);

    static std::string MakeLower(std::string strText);

private:
    StringUtil();
    ~StringUtil();
};




#endif//_STRING_UTIL_H_
