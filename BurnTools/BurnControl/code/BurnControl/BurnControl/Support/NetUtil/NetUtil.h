#ifndef _NET_UTIL_H_
#define _NET_UTIL_H_

#include <string>

/**
 * use example
 int nInterfaceCount = NetUtil::GetInterfaceCount();
 for (int i = 0; i < nInterfaceCount; i ++)
 {
     std::string strIP;
     std::string strNetmask;
     std::string strGateway;
     std::string strMacAddr;
     std::string strDescription;
     NetUtil::GetInterfaceParameter(i, strIP, strNetmask, strGateway, strMacAddr, strDescription);
 }
 *
 */

class NetUtil
{
public:
    static int GetInterfaceCount();
    static int GetInterfaceParameter(int nInterfaceNo, std::string &strIPAddr, std::string &strNetmask,
        std::string &strGateway, std::string &strMacAddr, std::string &strDescription);

    static std::string GetHostIP();
private:
    NetUtil();
    ~NetUtil();
};

#endif//_NET_UTIL_H_
