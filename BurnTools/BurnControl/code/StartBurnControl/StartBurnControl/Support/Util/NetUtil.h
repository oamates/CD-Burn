#ifndef _NET_UTIL_H_
#define _NET_UTIL_H_

#include <string>

/**
 * use example
 int nInterfaceCount = NetUtil::GetInterfaceCount();
 for (int i = 0; i < nInterfaceCount; i ++)
 {
     if (NetUtil::GetInterfaceParameter(i, strIP, strNetmask, strGateway,
     strMacAddr, strDescription) == 0)
     {
        printf("----------\r\n");
        printf("IP: %s\n", strIP.c_str());
        printf("Netmask: %s\r\n", strNetmask.c_str());
        printf("Gateway: %s\r\n", strGateway.c_str());
        printf("MaxAddr: %s\r\n", strMacAddr.c_str());
        printf("Description: %s\r\n", strDescription.c_str());
        printf("----------\r\n");
     }
 }
 *
 */

class NetUtil
{
public:
    static int GetInterfaceCount();
    /**
     */
    static int GetInterfaceParameter(int nInterfaceNo, std::string &strIPAddr, std::string &strNetmask,
        std::string &strGateway, std::string &strMacAddr, std::string &strDescription);
private:
    NetUtil();
    ~NetUtil();
};

#endif//_NET_UTIL_H_
