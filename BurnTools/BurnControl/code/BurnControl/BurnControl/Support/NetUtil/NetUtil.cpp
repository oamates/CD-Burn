#include <stdafx.h>

#include "NetUtil.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <IPHlpApi.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib,"iphlpapi")

NetUtil::NetUtil()
{
    //
}

NetUtil::~NetUtil()
{
    //
}

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
                        sprintf(szMac, "%x-%x-%x-%x-%x-%x", pTemp->Address[0], pTemp->Address[1], pTemp->Address[2], 
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

std::string NetUtil::GetHostIP()
{
    std::string strHostIP;

    std::string strWirelessIP;

    int nInterfaceCount = NetUtil::GetInterfaceCount();
    for (int i = 0; i < nInterfaceCount; i ++)
    {
        std::string strIP;
        std::string strNetmask;
        std::string strGateway;
        std::string strMacAddr;
        std::string strDescription;

        if( 0 == NetUtil::GetInterfaceParameter(i, strIP, strNetmask, strGateway, strMacAddr, strDescription) )
        {
            //LOG_INFO(("[NetUtil::GetHostIP] ip decp :¡¡%s,%s\r\n",strIP.c_str(),strDescription.c_str()));

			if (strDescription.find("Wireless") != std::string::npos)
			{
				strWirelessIP=strIP;
			}

            if (strDescription.find("VMware") == std::string::npos &&
                strDescription.find("Wireless") == std::string::npos &&
                strDescription.find("ÐéÄâÊÊÅäÆ÷") == std::string::npos )
            {
                //LOG_INFO(("[NetUtil::GetHostIP] ip decp :¡¡%s\r\n",strDescription.c_str()));

                strHostIP=strIP;

                break;
            }
        }
    }

    if ("" == strHostIP || "0.0.0.0" == strHostIP)
    {
        strHostIP=strWirelessIP;
    }

    LOG_INFO(("[NetUtil::GetHostIP] Return host ip : %s\r\n",strHostIP.c_str()));

    return strHostIP;
}
