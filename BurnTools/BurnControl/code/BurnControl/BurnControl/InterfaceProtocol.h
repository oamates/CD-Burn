#pragma once

#include "BurnInfo.h"

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
    static std::string InterfaceCommunicate(std::string strIP, int nPort,const std::string &strContent,
        std::string strTag,int nTimeOut);

    static void SetCallBack(PROTOCOLCALLBACKFUN cbf,void *pUsrParm);

    //BurnControl Server�յ���Ϣ�����˻ص�����
    static void OnRequest(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam);

    //�����ϲ㷢�͵�ʵʱ��¼Э�飬��Э��������ʾ��¼UI
    static void AnalyzeProtocolSendRTBurnInfo(const std::string &strProtocolContent,CRTBurnInfo &RTBurnInfo);

    //�����ϲ㷢�͵��º��¼Э�飬���������񡢵�����COS�º��¼
    static void AnalyzeProtocolSendBurnInfo(const std::string &strProtocolContent,CNormalBurnInfo &normalBurnInfo);


    //�����ϲ㷢�͵Ŀ�ʼʵʱ��¼Э��
    static void AnalyzeProtocolStartRTBurnInfo(const std::string &strProtocolContent,CRTBurnInfo &RTBurnInfo);

    //�������ϲ㷢�͵Ŀ�¼������ϢЭ��
    static std::string GenerateProtocolSendRTBurnInfoToUpper(const CRTBurnInfo &RTBurnInfo);

    //�������ϲ㷢�͵�ʵʱ��¼״̬Э��
    static std::string GenerateProtocolSendRTBurnStateToUpper(const CRTBurnInfo &RTBurnInfo);

    //�������ϲ㷢�͵�ʵʱ��¼������������ͣʵʱ��¼���ָ�ʵʱ��¼��
    static std::string GenerateProtocolSendRTBurnRequestToUpper(std::string strJobID,std::string strRequestType);

    //������BurnServer���͵Ŀ�ʼʵʱ��¼Э��
    static std::string GenerateProtocolStartRTBurnToBurnServer(const CRTBurnInfo &RTBurnInfo);

    //������BurnServer���͵Ŀ�ʼ�º��¼Э��
    static std::string GenerateProtocolStartBurnToBurnServer(const CNormalBurnJobInfoEx &normalJob);

    //�������²㷢�͵Ŀ�¼��������
    static std::string GenerateProtocolSendBurnCtrolCMDToBurnServer(std::string strJobID,std::string strCMD);

    //�����ϲ㷢�͸�BurnServer�Ŀ�ʼʵʱ��¼Э��
    static void AnalyzeProtocolStartRTBurnToBurnServer(const std::string &strProtocolContent,CRTBurnInfo &RTBurnInfoEx);

    //�����ϲ㷢�͸�BurnServer�Ŀ�ʼ�º��¼Э��
    static void AnalyzeProtocolStarBurnToBurnServer(const std::string &strProtocolContent,CNormalBurnJobInfoEx &normalBurnJobInfoEx);

    //������BurnServer���ϲ㷢�͵Ŀ�¼״̬Э��
    static std::string GenerateProtocolSendBurnState(std::string strJobID,
        std::string strState,std::string strStateDescription,
        std::vector<CCDROMDriverInfo> &vecCDROMDriver);

    //����BurnServer���͵Ŀ�¼״̬Э��
    static void AnalyzeProtocolSendBurnState(const std::string &strProtocolContent,std::vector<CCDROMDriverInfo> &vecCDROMInfo);

    //������BurnServer���͵Ĺ�����ϢЭ��
    static std::string GenerateProtocolCDROMDriverInfo(const std::vector<CNormalBurnJobInfoEx> &vecNormalBurnJobInfoEx,
        const std::vector<CRTBurnInfo> &vecRTBurnInfo,const std::vector<CCDROMDriverInfo> &vecCDROMDriver);

    //����BurnServer���͵Ĺ�����ϢЭ��
    static void AnalyzeProtocolCDROMDriverInfo(const std::string &strProtocolContent,std::vector<CCDROMDriverInfo> &vecCDROMInfo);

    //����BurnServer���͵�BurnServer��ϢЭ��
    static void AnalyzeProtocolBurnServerInfo(const std::string &strProtocolContent,std::vector<CBurnServerInfoEx> &vecBurnServerInfoEx);

    //�����Ƿ񲹿̵�Э��
    static std::string GenerateProtocolRTBurnBackupResponse(std::string strJobID,std::string strResponse);

    static std::string GenerateProtocolRTBurnBackupAfterRestartResponse(std::string strJobID,std::string strResponse);
private:
    static PROTOCOLCALLBACKFUN m_fCallback;
    static void *m_pUsrParam;
};
