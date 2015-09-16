#include <stdafx.h>

#include "NetUtil.h"
#include <stdio.h>
#include <stdlib.h>
#if WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <IPHlpApi.h>
#pragma comment(lib,"iphlpapi")
#else
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif//WIN32

NetUtil::NetUtil()
{
    //
}

NetUtil::~NetUtil()
{
    //
}

#ifdef WIN32
int NetUtil::GetInterfaceCount()
{
    PIP_ADAPTER_INFO    pAdapterInfo = NULL;
    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    ULONG nBufferLength = sizeof(IP_ADAPTER_INFO);
    int nInterfaceCount = 0;

    if (pAdapterInfo != NULL)
    {
        if (GetAdaptersInfo(pAdapterInfo, &nBufferLength) == ERROR_BUFFER_OVERFLOW)
        {
            free(pAdapterInfo);
            pAdapterInfo = (IP_ADAPTER_INFO*)malloc(nBufferLength);
        }

        if (pAdapterInfo != NULL)
        {
            if (GetAdaptersInfo(pAdapterInfo, &nBufferLength) == NO_ERROR)
            {
                PIP_ADAPTER_INFO pTemp = pAdapterInfo;
                while (pTemp)
                {
                    nInterfaceCount ++;
                    pTemp = pTemp->Next;
                }
            }

            if (pAdapterInfo != NULL)
            {
                free(pAdapterInfo);
                pAdapterInfo = NULL;
            }
        }
    }

    return nInterfaceCount;
}

int NetUtil::GetInterfaceParameter(int nInterfaceNo, std::string &strIPAddr, std::string &strNetmask,
                                   std::string &strGateway, std::string &strMacAddr, std::string &strDescription)
{
    PIP_ADAPTER_INFO    pAdapterInfo = NULL;
    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    ULONG nBufferLength = sizeof(IP_ADAPTER_INFO);
    int nCurInterfaceNo = 0;
    int nReturn = -1;

    if (pAdapterInfo != NULL)
    {
        if (GetAdaptersInfo(pAdapterInfo, &nBufferLength) == ERROR_BUFFER_OVERFLOW)
        {
            free(pAdapterInfo);
            pAdapterInfo = (IP_ADAPTER_INFO*)malloc(nBufferLength);
        }

        if (pAdapterInfo != NULL)
        {
            if (GetAdaptersInfo(pAdapterInfo, &nBufferLength) == NO_ERROR)
            {
                PIP_ADAPTER_INFO pTemp = pAdapterInfo;
                while (pTemp)
                {
                    if (nCurInterfaceNo == nInterfaceNo)
                    {
                        strIPAddr = pTemp->IpAddressList.IpAddress.String;
                        strNetmask = pTemp->IpAddressList.IpMask.String;
                        strGateway = pTemp->GatewayList.IpAddress.String;
                        char szMac[128] = {0};
                        sprintf(szMac, "%.2x-%.2x-%.2x-%.2x-%.2x-%.2x", pTemp->Address[0], pTemp->Address[1], pTemp->Address[2], 
                            pTemp->Address[3], pTemp->Address[4], pTemp->Address[5]);
                        strMacAddr = szMac;
                        strDescription = pTemp->Description;
                        nReturn = 0;
                        break;
                    }
                    nCurInterfaceNo ++;
                    pTemp = pTemp->Next;
                }
            }

            if (pAdapterInfo != NULL)
            {
                free(pAdapterInfo);
                pAdapterInfo = NULL;
            }
        }
    }

    return nReturn;
}

#else
int NetUtil::GetInterfaceCount()
{
    int nCount = 0;

    FILE *hFile;
    char szCmd[128];
    char szLine[1024];

    sprintf(szCmd, "cat /proc/net/dev |grep eth");
    hFile = popen(szCmd, "r");
    if (hFile != NULL)
    {
        while (fgets(szLine, 1024, hFile) != NULL)
        {
            nCount ++;
        }
        pclose(hFile);
    }

    return nCount;
}

std::string GetGateway(char *szInterfaceName)
{
    std::string strGateway;
    FILE *hFile;
    char szCmd[128];
    char szLine[1024];
    char szTemp0[64];
    char szTemp1[64];
    char szTemp2[64];
    char szTemp3[64];
    char szTemp4[64];

    sprintf(szCmd, "ip route |grep default");

    // popen reslut like:
    // default via 10.14.1.254 dev eth0
    hFile = popen(szCmd, "r");
    if (hFile != NULL)
    {
        while (fgets(szLine, 1024, hFile) != NULL)
        {
            if (strstr(szLine, szInterfaceName) != NULL)
            {
                sscanf(szLine, "%s %s %s %s %s", 
                    szTemp0, szTemp1, szTemp2, szTemp3, szTemp4);
                strGateway = szTemp2;
                break;
            }
        }
        pclose(hFile);
    }

    return strGateway;
}

int NetUtil::GetInterfaceParameter(int nInterfaceNo, std::string &strIPAddr, std::string &strNetmask,
                                 std::string &strGateway, std::string &strMacAddr, std::string &strDescription)
{
    // get interface name
    char szInterfaceName[128];
    char szMaxAddr[128];

    int nReturn = -1;
    int nSockTemp;
    struct ifreq ifrAddr;

    sprintf(szInterfaceName, "eth%d", nInterfaceNo);

    nSockTemp = socket(AF_INET, SOCK_STREAM, 0);
    if (nSockTemp != -1)
    {
        do 
        {
            // get ip
            memset(&ifrAddr, 0, sizeof(ifrAddr));
            strncpy(ifrAddr.ifr_name, szInterfaceName, sizeof(ifrAddr.ifr_name)-1);
            if (ioctl(nSockTemp, SIOCGIFADDR, &ifrAddr) >= 0)
            {
                strIPAddr = inet_ntoa(((struct sockaddr_in *)&ifrAddr.ifr_addr)->sin_addr);
            }
            else
            {
                nReturn = -2;
                break;
            }

            // get netmask
            memset(&ifrAddr, 0, sizeof(ifrAddr));
            strncpy(ifrAddr.ifr_name, szInterfaceName, sizeof(ifrAddr.ifr_name)-1);
            if (ioctl(nSockTemp, SIOCGIFNETMASK, &ifrAddr) >= 0)
            {
                strNetmask = inet_ntoa(((struct sockaddr_in *)&ifrAddr.ifr_netmask)->sin_addr);
            }
            else
            {
                nReturn = -3;
                break;
            }

            // get mac addr
            memset(&ifrAddr, 0, sizeof(ifrAddr));
            strncpy(ifrAddr.ifr_name, szInterfaceName, sizeof(ifrAddr.ifr_name)-1);
            if (ioctl(nSockTemp, SIOCGIFHWADDR, &ifrAddr) >= 0)
            {
                sprintf(szMaxAddr,"%.2x-%.2x-%.2x-%.2x-%.2x-%.2x",
                    (unsigned char)ifrAddr.ifr_hwaddr.sa_data[0],
                    (unsigned char)ifrAddr.ifr_hwaddr.sa_data[1],
                    (unsigned char)ifrAddr.ifr_hwaddr.sa_data[2],
                    (unsigned char)ifrAddr.ifr_hwaddr.sa_data[3],
                    (unsigned char)ifrAddr.ifr_hwaddr.sa_data[4],
                    (unsigned char)ifrAddr.ifr_hwaddr.sa_data[5]);

                strMacAddr = szMaxAddr;
            }
            else
            {
                nReturn = -4;
                break;
            }

            // get gateway
            strGateway = GetGateway(szInterfaceName);

            strDescription = szInterfaceName;

            nReturn = 0;
        } while (false);
    }


    return nReturn;
}

#endif//WIN32
