#ifndef _COMMON_UTIL_H_
#define _COMMON_UTIL_H_

#include <string>


#ifndef INT64
#ifdef WIN32
typedef __int64				INT64;
#else
typedef signed long long	INT64;
#endif
#endif//INT64

//#ifndef UINT64
//#ifdef WIN32
//typedef unsigned __int64    UINT64;
//#else
//typedef unsigned long long  UINT64;
//#endif
//
//#endif//

int StringToInt(std::string strValue);
INT64 StringToInt64(std::string strValue);
std::string IntToString(int nValue);
std::string Int64ToString(INT64 nValue);


bool IntToBool(int nValue);


std::string DirFromFile(std::string strFilePath);
wchar_t* DirFromFileW(const wchar_t* szFilePath, wchar_t* szDir);

#endif //_COMMON_UTIL_H_
