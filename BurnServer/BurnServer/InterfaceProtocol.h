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

    //BurnControl Server�յ���Ϣ�����˻ص�����
    static void OnRequest(NCXSERVERHANDLE hNCXServer, NCXServerCBParam *pCBParam);

    static int DoAddTransmissionTask(FileAnywhereTaskParameter parameter);
    static int DoDeleteTransmissionTask(std::string strTaskID);

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
    static std::string GenerateProtocolStartBurnToBurnServer(const CNormalBurnJobInfoEx &normalBurnJobInfoEx);

    //�������²㷢�͵Ŀ�¼��������
    static std::string GenerateProtocolSendBurnCtrolCMDToBurnServer(std::string strJobID,std::string strCMD);

    //�����ϲ㷢�͸�BurnServer�Ŀ�ʼʵʱ��¼Э��
    static void AnalyzeProtocolStartRTBurnToBurnServer(const std::string &strProtocolContent,CRTBurnInfoEx &RTBurnInfoEx);

    //�����ϲ㷢�͸�BurnServer�Ŀ�ʼ�º��¼Э��
    static void AnalyzeProtocolStartBurnToBurnServer(const std::string &strProtocolContent,CNormalBurnJobInfoEx &normalBurnJobInfoEx);

    //������BurnServer���ϲ㷢�͵Ŀ�¼״̬Э��
    static std::string GenerateProtocolSendBurnState(std::string strJobID,std::string strState,std::string strStateDescription,
        std::vector<CCDROMDriverInfo> &vecCDROMDriver);

    //����BurnServer���͵Ŀ�¼״̬Э��
    static void AnalyzeProtocolSendBurnState(const std::string &strProtocolContent,std::vector<CCDROMDriverInfo> &vecCDROMInfo);

    //������BurnServer���͵Ĺ�����ϢЭ��
    static std::string GenerateProtocolCDROMDriverInfo(const std::vector<CCDROMDriverInfo> &vecCDROMDriver);

    //����BurnServer���͵Ĺ�����ϢЭ��
    static void AnalyzeProtocolCDROMDriverInfo(const std::string &strProtocolContent,std::vector<CCDROMDriverInfo> &vecCDROMInfo);
    //����BurnServer���͵�BurnServer��ϢЭ��
    static void AnalyzeProtocolBurnServerInfo(const std::string &strProtocolContent,std::vector<CBurnServerInfo> &vecBurnServerInfo);
private:
    static int FeedBack(NCXSERVERHANDLE hNCXServer, NCXServerCBParam *pCBParam);

    static PROTOCOLCALLBACKFUN m_fCallback;
    static void *m_pUsrParam;
};
#endif
