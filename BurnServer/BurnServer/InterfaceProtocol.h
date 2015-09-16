#ifndef _INTERFACE_PROTOCOL_H_
#define _INTERFACE_PROTOCOL_H_

#include "BurnInfo.h"

#include "NCX.h"

#include "FileAnywhereTaskParameter.h"

#define BURN_PROTOCOL_TAG        "burnControl"
#define DOWNLOAD_PROTOCOL_TAG    "fileAnywhere"

typedef void (* PROTOCOLCALLBACKFUN)(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam,void *pUsrParam);

class CInterfaceProtocol
{
public:
    CInterfaceProtocol(void);
    ~CInterfaceProtocol(void);

    static std::string GetTagValue(std::string strContent, std::string strTagName);
    static std::string GetTagValueWithStartPos(std::string strContent, std::string strTagName, int &nRelativePos);
    static std::string GetCommandType(const char *sProtocolContent);
    static std::string GetJobID(const char *sProtocolContent);
    static std::string InterfaceCommunicate(std::string strIP, int nPort,const std::string &strContent,std::string strTag);

    static void SetCallBack(PROTOCOLCALLBACKFUN cbf,void *pUsrParm);

    //BurnControl Server收到消息后进入此回调函数
    static void OnRequest(NCXSERVERHANDLE hNCXServer, NCXServerCBParam *pCBParam);

    static int DoAddTransmissionTask(FileAnywhereTaskParameter parameter);
    static int DoDeleteTransmissionTask(std::string strTaskID);

    //解析上层发送的实时刻录协议，此协议用于显示刻录UI
    static void AnalyzeProtocolSendRTBurnInfo(const std::string &strProtocolContent,CRTBurnInfo &RTBurnInfo);

    //解析上层发送的事后刻录协议，包括多任务、单任务、COS事后刻录
    static void AnalyzeProtocolSendBurnInfo(const std::string &strProtocolContent,CNormalBurnInfo &normalBurnInfo);

    //解析上层发送的开始实时刻录协议
    static void AnalyzeProtocolStartRTBurnInfo(const std::string &strProtocolContent,CRTBurnInfo &RTBurnInfo);

    //生成向上层发送的刻录设置信息协议
    static std::string GenerateProtocolSendRTBurnInfoToUpper(const CRTBurnInfo &RTBurnInfo);

    //生成向上层发送的实时刻录状态协议
    static std::string GenerateProtocolSendRTBurnStateToUpper(const CRTBurnInfo &RTBurnInfo);

    //生成向上层发送的实时刻录控制请求，如暂停实时刻录、恢复实时刻录等
    static std::string GenerateProtocolSendRTBurnRequestToUpper(std::string strJobID,std::string strRequestType);

    //生成向BurnServer发送的开始实时刻录协议
    static std::string GenerateProtocolStartRTBurnToBurnServer(const CRTBurnInfo &RTBurnInfo);

    //生成向BurnServer发送的开始事后刻录协议
    static std::string GenerateProtocolStartBurnToBurnServer(const CNormalBurnJobInfoEx &normalBurnJobInfoEx);

    //生成向下层发送的刻录控制命令
    static std::string GenerateProtocolSendBurnCtrolCMDToBurnServer(std::string strJobID,std::string strCMD);

    //解析上层发送给BurnServer的开始实时刻录协议
    static void AnalyzeProtocolStartRTBurnToBurnServer(const std::string &strProtocolContent,CRTBurnInfoEx &RTBurnInfoEx);

    //解析上层发送给BurnServer的开始事后刻录协议
    static void AnalyzeProtocolStartBurnToBurnServer(const std::string &strProtocolContent,CNormalBurnJobInfoEx &normalBurnJobInfoEx);

    //生成由BurnServer向上层发送的刻录状态协议
    static std::string GenerateProtocolSendBurnState(std::string strJobID,std::string strState,std::string strStateDescription,
        std::vector<CCDROMDriverInfo> &vecCDROMDriver);

    //解析BurnServer发送的刻录状态协议
    static void AnalyzeProtocolSendBurnState(const std::string &strProtocolContent,std::vector<CCDROMDriverInfo> &vecCDROMInfo);

    //生成由BurnServer发送的光驱信息协议
    static std::string GenerateProtocolCDROMDriverInfo(const std::vector<CCDROMDriverInfo> &vecCDROMDriver);

    //解析BurnServer发送的光驱信息协议
    static void AnalyzeProtocolCDROMDriverInfo(const std::string &strProtocolContent,std::vector<CCDROMDriverInfo> &vecCDROMInfo);
    //解析BurnServer发送的BurnServer信息协议
    static void AnalyzeProtocolBurnServerInfo(const std::string &strProtocolContent,std::vector<CBurnServerInfo> &vecBurnServerInfo);
private:
    static int FeedBack(NCXSERVERHANDLE hNCXServer, NCXServerCBParam *pCBParam);

    static PROTOCOLCALLBACKFUN m_fCallback;
    static void *m_pUsrParam;
};
#endif
