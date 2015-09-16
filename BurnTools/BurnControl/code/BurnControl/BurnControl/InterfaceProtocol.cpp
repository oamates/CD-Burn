#include "stdafx.h"

#include "InterfaceProtocol.h"

#include "InterfaceServer.h"

#include <direct.h>

const static std::string g_strResponse=
"<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
"<burnControl>"
"<retcode>0</retcode>"
"<returnDescription>ok</returnDescription>"
"</burnControl>";

PROTOCOLCALLBACKFUN CInterfaceProtocol::m_fCallback=NULL;
void *(CInterfaceProtocol::m_pUsrParam)=NULL;

CInterfaceProtocol::CInterfaceProtocol(void)
{
}

CInterfaceProtocol::~CInterfaceProtocol(void)
{
}

std::string CInterfaceProtocol::GetTagValue(std::string strContent, std::string strTagName)
{
    std::string	strFind;
    size_t nBegin = 0;
    size_t nEnd = 0;
    std::string strReturn;

    strFind = "<";
    strFind += strTagName;
    strFind += ">";

    nBegin = strContent.find(strFind);
    if (nBegin != std::string::npos)
    {
        strFind = "</";
        strFind += strTagName;
        strFind += ">";

        nBegin += strTagName.length()+2;
        nEnd = strContent.find(strFind, nBegin);
        if (nEnd != std::string::npos && nEnd >= nBegin)
        {
            strReturn = strContent.substr(nBegin, nEnd-nBegin);
        }
    }

    return strReturn;
}

std::string CInterfaceProtocol::GetTagValueWithStartPos(std::string strContent, std::string strTagName, int &nRelativePos)
{
    std::string	strFind;
    size_t nBegin = 0;
    size_t nEnd = 0;
    std::string strReturn;

    strFind = "<";
    strFind += strTagName;
    strFind += ">";

    nBegin = strContent.find(strFind, nRelativePos);
    nRelativePos = -1;
    if (nBegin != std::string::npos)
    {
        strFind = "</";
        strFind += strTagName;
        strFind += ">";

        nBegin += strTagName.length()+2;
        nEnd = strContent.find(strFind, nBegin);
        if (nEnd != std::string::npos && nEnd >= nBegin)
        {
            strReturn = strContent.substr(nBegin, nEnd-nBegin);
            //nRelativePos = nBegin;
            nRelativePos = nEnd+strTagName.length()+3;
        }
    }

    return strReturn;
}

std::string CInterfaceProtocol::GetCommandType(const char *sProtocolContent)
{
    std::string	strContent;

    strContent = std::string(sProtocolContent);
    return GetTagValue(strContent, "commandType");
}

std::string CInterfaceProtocol::GetJobID(const char *sProtocolContent)
{
    std::string	strContent;

    strContent = std::string(sProtocolContent);
    return GetTagValue(strContent, "jobID");
}

std::string CInterfaceProtocol::InterfaceCommunicate(std::string strIP, int nPort,
                                                     const std::string &strContent,std::string strTag,
                                                     int nTimeOut)
{
    NCXCLIENTHANDLE     hNCXClient;
    std::string         strResponse;

    hNCXClient=ncxCreateNCXClient();

    if (hNCXClient != NULL &&
        strIP != "" &&
        nPort > 0 &&
        strContent != "" &&
        strTag != "")
    {
        LOG_DEBUG(("[CInterfaceProtocol::InterfaceCommunicate] Client %p,ip %s,port %d,content length %d,tag %s\r\n",
            hNCXClient,strIP.c_str(),nPort,strContent.length(),strTag.c_str()));

        ncxSetNCXClientOuterMostTagName(hNCXClient,const_cast<char *>(strTag.c_str()));

        if (ncxConnectServer(hNCXClient,strIP.c_str(),nPort,nTimeOut))
        {
            int nRet=ncxCommunicateWithServer(hNCXClient, strContent.c_str(),
                strContent.length()+1,nTimeOut);

            if (0 == nRet)
            {
                strResponse=ncxGetRespondProtocol(hNCXClient);
            }
            else
            {
                LOG_ERROR(("[CInterfaceProtocol::InterfaceCommunicate] ncxCommunicateWithServer failed,nRet=%d\r\n",nRet));
            }
        }
    }
    else
    {
        LOG_ERROR(("[CInterfaceProtocol::InterfaceCommunicate] Parameter error\r\n"));
    }

    if (hNCXClient != NULL)
    {
        ncxDestroyNCXClient(hNCXClient);
    }

    return strResponse;
}

void CInterfaceProtocol::SetCallBack(PROTOCOLCALLBACKFUN cbf,void *pUsrParm)
{
    m_fCallback=cbf;
    m_pUsrParam=pUsrParm;
}

void CInterfaceProtocol::OnRequest(NCXSERVERHANDLE hNCXServer, NCXServerCBParam *pCBParam)
{
    if (pCBParam != NULL &&
        hNCXServer != NULL)
    {
        LOG_INFO(("[CInterfaceProtocol::OnRequest] Receive msg : \r\n%s\r\n%s\r\n",
            pCBParam->szRemoteIP,pCBParam->sProtocolContent));

        if (ncxSendProtocolResponse(hNCXServer,*pCBParam,g_strResponse.c_str(),
            g_strResponse.length()+1) != 0)
        {
            LOG_ERROR(("[CInterfaceProtocol::OnRequest] ncxSendProtocolResponse failed,%s\r\n",
                pCBParam->szRemoteIP));
        }
        else
        {
            if (m_fCallback != NULL)
            {
                m_fCallback(hNCXServer,pCBParam,m_pUsrParam);
            }
        }
    }
}

void CInterfaceProtocol::AnalyzeProtocolSendRTBurnInfo(const std::string &strProtocolContent,CRTBurnInfo &RTBurnInfo)
{
    RTBurnInfo=CRTBurnInfo();

    RTBurnInfo.SetJobID(GetTagValue(strProtocolContent,"jobID"));

    RTBurnInfo.SetJobType(GetTagValue(strProtocolContent,"jobType"));

    RTBurnInfo.GetBurnServerInfo().SetIP(GetTagValue(strProtocolContent,"burnServerIP"));
    RTBurnInfo.GetBurnServerInfo().SetPort(GetTagValue(strProtocolContent,"burnServerPort"));

    std::string strPlayerInfoStruct=GetTagValue(strProtocolContent,"playerInfoStruct");
    RTBurnInfo.GetPlayerInfo().SetVersion(GetTagValue(strPlayerInfoStruct,"playerVersion"));
    RTBurnInfo.GetPlayerInfo().SetDownloadURL(GetTagValue(strPlayerInfoStruct,"playerDownloadURL"));
    RTBurnInfo.GetPlayerInfo().SetRemoteIP(GetTagValue(strPlayerInfoStruct,"remoteIP"));
    RTBurnInfo.GetPlayerInfo().SetRemotePort(GetTagValue(strPlayerInfoStruct,"remotePort"));

    RTBurnInfo.SetPlayListContent(GetTagValue(strProtocolContent,"playListInfo"));

    RTBurnInfo.SetNoteFileDownloadURL(GetTagValue(strProtocolContent,"noteFileDownloadURL"));
    RTBurnInfo.SetNoteFileRemoteIP(GetTagValue(strProtocolContent,"noteFileRemoteIP"));
    RTBurnInfo.SetNoteFileRemotePort(GetTagValue(strProtocolContent,"noteFileRemotePort"));

    int nRelativePos=0;

    //解析光驱信息
    std::string strCDROMVector=GetTagValue(strProtocolContent,"CDROMVector");
    std::string strCDROMStruct=GetTagValueWithStartPos(strCDROMVector,"CDROMStruct",nRelativePos);
    while (strCDROMStruct != "")
    {
        CCDROMDriverInfo CDROMInfo;

        CDROMInfo.SetID(GetTagValue(strCDROMStruct,"CDROMID"));
        CDROMInfo.SetDescription(GetTagValue(strCDROMStruct,"CDROMDescription"));

        std::string strDevSel=GetTagValue(strCDROMStruct,"isSelected");
        if (DEV_SELECTED_YES == strDevSel)
        {
        } 
        else
        {
            strDevSel=DEV_SELECTED_NO;
        }
        CDROMInfo.SetIsSelected(strDevSel);

        RTBurnInfo.GetBurnServerInfo().GetVecCDROMDriverInfo().push_back(CDROMInfo);

        strCDROMStruct=GetTagValueWithStartPos(strCDROMVector,"CDROMStruct",nRelativePos);
    }

    RTBurnInfo.GetCommonBurnParam().SetBurnType(GetTagValue(strProtocolContent,"burnType"));
    RTBurnInfo.GetCommonBurnParam().SetBurnPassword(GetTagValue(strProtocolContent,"burnPassword"));
    RTBurnInfo.GetCommonBurnParam().SetContentPassword(GetTagValue(strProtocolContent,"contentPassword"));

    RTBurnInfo.GetCommonBurnParam().SetCDName(GetTagValue(strProtocolContent,"CDName"));

    std::string strVideoLocationVector=GetTagValue(strProtocolContent,"videoLocationVector");
    nRelativePos=0;
    std::string strVideoLocationStruct=GetTagValueWithStartPos(
        strVideoLocationVector,"videoLocationStruct",nRelativePos);
    while (strVideoLocationStruct != "")
    {
        CLocationInfo videoLocationInfo;

        videoLocationInfo.SetID(GetTagValue(strVideoLocationStruct,"locationID"));
        videoLocationInfo.SetDescription(GetTagValue(strVideoLocationStruct,"locationDescription"));

        std::string strBurnOrNot=GetTagValue(strVideoLocationStruct,"locationBurnOrNot");
        videoLocationInfo.SetBurnOrNot(strBurnOrNot);

        //类似HM3的应用不会再发送开始实时刻录的协议，所以在这个协议里面必须填充流地址
        videoLocationInfo.SetURL(GetTagValue(strVideoLocationStruct,"locationURL"));

        RTBurnInfo.GetVecVideoLocationInfo().push_back(videoLocationInfo);

        strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,
            "videoLocationStruct",nRelativePos);
    }

    RTBurnInfo.SetNewFileName(GetTagValue(strProtocolContent,"newFileName"));
    RTBurnInfo.SetCDAlarmLimit(GetTagValue(strProtocolContent,"CDAlarmLimit"));
    RTBurnInfo.SetBurnBufferSize(GetTagValue(strProtocolContent,"burnBufferSize"));

    RTBurnInfo.SetUpServerIP(GetTagValue(strProtocolContent,"upServerIP"));
    RTBurnInfo.SetUpServerPort(GetTagValue(strProtocolContent,"upServerPort"));
}

void CInterfaceProtocol::AnalyzeProtocolSendBurnInfo(const std::string &strProtocolContent,CNormalBurnInfo &normalBurnInfo)
{
    normalBurnInfo=CNormalBurnInfo();

    //指向默认的刻录服务器
    const CBurnServerInfo *pDefaultServer=NULL;

    int nRelativePos=0;

    //解析刻录服务器信息
    std::string strburnServerVector=GetTagValue(strProtocolContent,"burnServerVector");
    nRelativePos=0;
    std::string strBurnServerStruct=GetTagValueWithStartPos(strburnServerVector,"burnServerStruct",nRelativePos);
    while (strBurnServerStruct != "")
    {
        CBurnServerInfo burnServerInfo;

        burnServerInfo.SetIP(GetTagValue(strBurnServerStruct,"burnServerIP"));
        burnServerInfo.SetPort(GetTagValue(strBurnServerStruct,"burnServerPort"));
        burnServerInfo.SetBurnServerName(GetTagValue(strBurnServerStruct,"burnServerName"));
        burnServerInfo.SetDescription(GetTagValue(strBurnServerStruct,"burnServerDescription"));
        burnServerInfo.SetIsDefault(GetTagValue(strBurnServerStruct,"isDefaultBurnServer"));

        if (NULL == pDefaultServer)
        {
            pDefaultServer=&burnServerInfo;
        }

        if ("1" == burnServerInfo.GetIsDefault())
        {
            pDefaultServer=&burnServerInfo;
        }

        normalBurnInfo.GetVecBurnServerInfo().push_back(burnServerInfo);
        strBurnServerStruct=GetTagValueWithStartPos(strburnServerVector,"burnServerStruct",nRelativePos);
    }

    normalBurnInfo.SetBurnFlag(GetTagValue(strProtocolContent,"burnFlag"));
    normalBurnInfo.GetCommonBurnParam().SetBurnPassword(GetTagValue(strProtocolContent,"burnPassword"));
    normalBurnInfo.GetCommonBurnParam().SetContentPassword(GetTagValue(strProtocolContent,"contentPassword"));
    normalBurnInfo.GetCommonBurnParam().SetBurnType(GetTagValue(strProtocolContent,"burnType"));

    std::string strPlayerInfoStruct=GetTagValue(strProtocolContent,"playerInfoStruct");
    if (strPlayerInfoStruct != "")
    {
        normalBurnInfo.GetPlayerInfo().SetVersion(GetTagValue(strPlayerInfoStruct,"playerVersion"));
        normalBurnInfo.GetPlayerInfo().SetDownloadURL(GetTagValue(strPlayerInfoStruct,"playerDownloadURL"));
        normalBurnInfo.GetPlayerInfo().SetRemoteIP(GetTagValue(strPlayerInfoStruct,"remoteIP"));
        normalBurnInfo.GetPlayerInfo().SetRemotePort(GetTagValue(strPlayerInfoStruct,"remotePort"));
    }

    std::string strJobVector=GetTagValue(strProtocolContent,"jobVector");
    nRelativePos=0;
    std::string strJobStruct=GetTagValueWithStartPos(strJobVector,"jobStruct",nRelativePos);
    while (strJobStruct != "")
    {
        CNormalBurnJobInfoEx normalBurnJobInfo;

        normalBurnJobInfo.SetJobID(GetTagValue(strJobStruct,"jobID"));
        normalBurnJobInfo.SetJobName(GetTagValue(strJobStruct,"jobName"));
        normalBurnJobInfo.SetJobDescription(GetTagValue(strJobStruct,"jobDescription"));

        normalBurnJobInfo.SetJobSize(StringToInt(GetTagValue(strJobStruct,"jobSize")));

        normalBurnJobInfo.GetBurnServerInfo().SetIP(GetTagValue(strJobStruct,"burnServerIP"));
        normalBurnJobInfo.GetBurnServerInfo().SetPort(GetTagValue(strJobStruct,"burnServerPort"));

        normalBurnJobInfo.GetCommonBurnParam().SetBurnPassword(GetTagValue(strJobStruct,"burnPassword"));
        normalBurnJobInfo.GetCommonBurnParam().SetContentPassword(GetTagValue(strJobStruct,"contentPassword"));
        normalBurnJobInfo.GetCommonBurnParam().SetCDName(GetTagValue(strJobStruct,"CDName"));

        normalBurnJobInfo.SetNoteFileRemoteIP(GetTagValue(strJobStruct,"noteFileRemoteIP"));
        normalBurnJobInfo.SetNoteFileRemotePort(GetTagValue(strJobStruct,"noteFileRemotePort"));
        normalBurnJobInfo.SetNoteRelativePath(GetTagValue(strJobStruct,"noteFileDownloadURL"));
        normalBurnJobInfo.SetNoteFileName(GetTagValue(strJobStruct,"noteFileName"));

        normalBurnJobInfo.SetAutorunFileRemoteIP(GetTagValue(strJobStruct,"autorunFileRemoteIP"));
        normalBurnJobInfo.SetAutorunFileRemotePort(GetTagValue(strJobStruct,"autorunFileRemotePort"));
        normalBurnJobInfo.SetAutorunFilePath(GetTagValue(strJobStruct,"autorunFileDownloadURL"));

        normalBurnJobInfo.SetVideoFileRemoteIP(GetTagValue(strJobStruct,"videoFileRemoteIP"));
        normalBurnJobInfo.SetVideoFileRemotePort(GetTagValue(strJobStruct,"videoFileRemotePort"));
        normalBurnJobInfo.SetVideoFileRelativePath(GetTagValue(strJobStruct,"videoFileDownloadURL"));

        std::string strVideoLocationVector=GetTagValue(strJobStruct,"videoLocationVector");
        int nVideoLocationVectorRelativePos=0;
        std::string strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,"videoLocationStruct",
            nVideoLocationVectorRelativePos);
        while (strVideoLocationStruct != "")
        {
            CLocationInfo videoLocationInfo;
            videoLocationInfo.SetID(GetTagValue(strVideoLocationStruct,"locationID"));
            videoLocationInfo.SetDescription(GetTagValue(strVideoLocationStruct,"locationDescription"));
            std::string strBurnOrNot=GetTagValue(strVideoLocationStruct,"locationBurnOrNot");
            videoLocationInfo.SetBurnOrNot(strBurnOrNot);
            videoLocationInfo.SetStreamType(GetTagValue(strVideoLocationStruct,"streamType"));

            //分解一个画面的文件列表
            std::string strVideoFileVector=GetTagValue(strVideoLocationStruct,"videoFileVector");
            int nVideoFileVectorRelativePos=0;
            std::string strVideoFileStruct=GetTagValueWithStartPos(strVideoFileVector,"videoFileStruct",
                nVideoFileVectorRelativePos);
            while (strVideoFileStruct != "")
            {
                CSingleFileInfo videoFileInfo;
                videoFileInfo.SetFileName(GetTagValue(strVideoFileStruct,"videoFileName"));
                videoFileInfo.SetSize(GetTagValue(strVideoFileStruct,"videoFileSize"));
                videoFileInfo.SetNewFileName(GetTagValue(strVideoFileStruct,"newVideoFileName"));

                videoLocationInfo.GetVecFileInfo().push_back(videoFileInfo);
                strVideoFileStruct=GetTagValueWithStartPos(strVideoFileVector,"videoFileStruct",
                    nVideoFileVectorRelativePos);
            }

            normalBurnJobInfo.GetVecLocationInfo().push_back(videoLocationInfo);
            strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,"videoLocationStruct",
                nVideoLocationVectorRelativePos);
        }

        normalBurnJobInfo.SetPlayListContent(GetTagValue(strJobStruct,"playListInfo"));

        std::string strPrintWordContentVector;
        strPrintWordContentVector=GetTagValue(strJobStruct,"printWordVector");
        //分解打印信息
        int nPrintWordContentVectorRelativePos=0;
        std::string strPrintWordContentStruct=GetTagValueWithStartPos(strPrintWordContentVector,"printWordStruct",
            nPrintWordContentVectorRelativePos);
        while (strPrintWordContentStruct != "")
        {
            std::string strID;
            std::string strValue;
            strID=GetTagValue(strPrintWordContentStruct,"itemID");
            strValue=GetTagValue(strPrintWordContentStruct,"itemValue");

            if (strID != "" &&
                strValue != "")
            {
                normalBurnJobInfo.GetMapLabelFieldText()[strID]=strValue;
            }
            strPrintWordContentStruct=GetTagValueWithStartPos(strPrintWordContentVector,"printWordStruct",
                nPrintWordContentVectorRelativePos);
        }

        //应用默认设置
        if ( ("" == normalBurnJobInfo.GetBurnServerInfo().GetIP()) ||
            ("" == normalBurnJobInfo.GetBurnServerInfo().GetPort()) )
        {
            if (pDefaultServer != NULL)
            {
                normalBurnJobInfo.SetBurnServerInfo(*pDefaultServer);
            }
            else
            {
                if (!normalBurnInfo.GetVecBurnServerInfo().empty())
                {
                    normalBurnJobInfo.SetBurnServerInfo(normalBurnInfo.GetVecBurnServerInfo().at(0));
                }
                else
                {
                    //如果上层不给BurnServer地址，默认使用本地
                    CBurnServerInfo hostBurnServerInfo;
                    hostBurnServerInfo.SetIP(NetUtil::GetHostIP());
                    hostBurnServerInfo.SetPort(IntToString(BURNSERVER_PORT));

                    normalBurnJobInfo.SetBurnServerInfo(hostBurnServerInfo);

                    LOG_INFO(("[CInterfaceProtocol::AnalyzeProtocolSendBurnInfo] Add local BurnServer,%d\r\n",
                        __LINE__));
                }
            }
        }

        if (normalBurnJobInfo.GetPlayerInfo().GetRemoteIP() == "")
        {
            normalBurnJobInfo.SetPlayerInfo(normalBurnInfo.GetPlayerInfo());
        }

        if ("" == normalBurnJobInfo.GetCommonBurnParam().GetBurnPassword())
        {
            normalBurnJobInfo.GetCommonBurnParam().SetBurnPassword(
                normalBurnInfo.GetCommonBurnParam().GetBurnPassword());
        }
        if ("" == normalBurnJobInfo.GetCommonBurnParam().GetContentPassword())
        {
            normalBurnJobInfo.GetCommonBurnParam().SetContentPassword(
                normalBurnInfo.GetCommonBurnParam().GetContentPassword());
        }
        if ("" == normalBurnJobInfo.GetCommonBurnParam().GetBurnType())
        {
            normalBurnJobInfo.GetCommonBurnParam().SetBurnType(normalBurnInfo.GetCommonBurnParam().GetBurnType());
        }

        //计算任务总的大小
        normalBurnJobInfo.CalculateJobSize();

        normalBurnInfo.GetVecNormalBurnJobInfo().push_back(normalBurnJobInfo);

        strJobStruct=GetTagValueWithStartPos(strJobVector,"jobStruct",nRelativePos);
    }//while
}

void CInterfaceProtocol::AnalyzeProtocolStartRTBurnInfo(const std::string &strProtocolContent,CRTBurnInfo &RTBurnInfo)
{
    RTBurnInfo.SetJobID(GetTagValue(strProtocolContent,"jobID"));

    RTBurnInfo.SetJobType(GetTagValue(strProtocolContent,"jobType"));

    std::string strPlayerInfoStruct=GetTagValue(strProtocolContent,"playerInfoStruct");
    if (strPlayerInfoStruct != "")
    {
        RTBurnInfo.GetPlayerInfo().SetVersion(GetTagValue(strPlayerInfoStruct,"playerVersion"));
        RTBurnInfo.GetPlayerInfo().SetDownloadURL(GetTagValue(strPlayerInfoStruct,"playerDownloadURL"));
        RTBurnInfo.GetPlayerInfo().SetRemoteIP(GetTagValue(strPlayerInfoStruct,"remoteIP"));
        RTBurnInfo.GetPlayerInfo().SetRemotePort(GetTagValue(strPlayerInfoStruct,"remotePort"));
    }

    RTBurnInfo.SetPlayListContent(GetTagValue(strProtocolContent,"playListInfo"));

    RTBurnInfo.SetNoteFileDownloadURL(GetTagValue(strProtocolContent,"noteFileDownloadURL"));
    RTBurnInfo.SetNoteFileRemoteIP(GetTagValue(strProtocolContent,"noteFileRemoteIP"));
    RTBurnInfo.SetNoteFileRemotePort(GetTagValue(strProtocolContent,"noteFileRemotePort"));

    RTBurnInfo.GetBurnServerInfo().SetIP(GetTagValue(strProtocolContent,"burnServerIP"));
    RTBurnInfo.GetBurnServerInfo().SetPort(GetTagValue(strProtocolContent,"burnServerPort"));

    std::string strCDROMVector=GetTagValue(strProtocolContent,"CDROMVector");
    int nRelativePos=0;
    std::string strCDROMStruct=GetTagValueWithStartPos(strCDROMVector,"CDROMStruct",nRelativePos);
    while (strCDROMStruct != "")
    {
        CCDROMDriverInfo CDROMInfo;

        CDROMInfo.SetID(GetTagValue(strCDROMStruct,"CDROMID"));
        CDROMInfo.SetDescription(GetTagValue(strCDROMStruct,"CDROMDescription"));

        CDROMInfo.SetIsSelected("1");

        RTBurnInfo.GetBurnServerInfo().GetVecCDROMDriverInfo().push_back(CDROMInfo);

        strCDROMStruct=GetTagValueWithStartPos(strCDROMVector,"CDROMStruct",nRelativePos);
    }

    //刻录方式，这个字段很重要，因为它决定了每个光驱该刻哪些内容
    RTBurnInfo.GetCommonBurnParam().SetBurnType(GetTagValue(strProtocolContent,"burnType"));

    RTBurnInfo.GetCommonBurnParam().SetBurnPassword(GetTagValue(strProtocolContent,"burnPassword"));
    RTBurnInfo.GetCommonBurnParam().SetContentPassword(GetTagValue(strProtocolContent,"contentPassword"));
    RTBurnInfo.GetCommonBurnParam().SetCDName(GetTagValue(strProtocolContent,"CDName"));

    std::string strVideoLocationVector=GetTagValue(strProtocolContent,"videoLocationVector");
    nRelativePos=0;
    std::string strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,"videoLocationStruct",
        nRelativePos);
    while (strVideoLocationStruct != "")
    {
        CLocationInfo videoLocationInfo;
        videoLocationInfo.SetID(GetTagValue(strVideoLocationStruct,"locationID"));
        videoLocationInfo.SetDescription(GetTagValue(strVideoLocationStruct,"locationDescription"));

        std::string strBurnOrNot=GetTagValue(strVideoLocationStruct,"locationBurnOrNot");
        videoLocationInfo.SetBurnOrNot(strBurnOrNot);

        //videoLocationInfo.SetBurnOrNot("1");
        videoLocationInfo.SetURL(GetTagValue(strVideoLocationStruct,"locationURL"));
        RTBurnInfo.GetVecVideoLocationInfo().push_back(videoLocationInfo);

        strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,"videoLocationStruct",nRelativePos);
    }

    RTBurnInfo.SetNewFileName(GetTagValue(strProtocolContent,"newFileName"));
    RTBurnInfo.SetCDAlarmLimit(GetTagValue(strProtocolContent,"CDAlarmLimit"));
    RTBurnInfo.SetBurnBufferSize(GetTagValue(strProtocolContent,"burnBufferSize"));

    RTBurnInfo.SetUpServerIP(GetTagValue(strProtocolContent,"upServerIP"));
    RTBurnInfo.SetUpServerPort(GetTagValue(strProtocolContent,"upServerPort"));
}



std::string CInterfaceProtocol::GenerateProtocolSendRTBurnInfoToUpper(const CRTBurnInfo &RTBurnInfo)
{
    std::string strProtocolContent="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    strProtocolContent+="\r\n";

    strProtocolContent+="<burnControl>\r\n";

    strProtocolContent+="<commandType>sendRTBurnConfig</commandType>\r\n";

    strProtocolContent+="<jobID>";
    strProtocolContent+=RTBurnInfo.GetJobID();
    strProtocolContent+="</jobID>\r\n";

    strProtocolContent+="<jobFlag>";
    strProtocolContent+=RTBurnInfo.GetJobFlag();
    strProtocolContent+="</jobFlag>\r\n";

    //光驱列表信息
    strProtocolContent+="<CDROMVector>\r\n";
    const std::vector<CCDROMDriverInfo> &vecCDROM=RTBurnInfo.GetBurnServerInfo().GetVecCDROMDriverInfo();
    for (size_t i=0;i<vecCDROM.size();++i)
    {
        strProtocolContent+="<CDROMStruct>\r\n";

        strProtocolContent+="<CDROMID>";
        strProtocolContent+=vecCDROM[i].GetID();
        strProtocolContent+="</CDROMID>\r\n";

        strProtocolContent+="<CDROMDescription>";
        strProtocolContent+=vecCDROM[i].GetDescription();
        strProtocolContent+="</CDROMDescription>\r\n";

        strProtocolContent+="<isSelected>";
        strProtocolContent+=vecCDROM[i].GetIsSelected();
        strProtocolContent+="</isSelected>\r\n";

        strProtocolContent+="</CDROMStruct>\r\n";
    }
    strProtocolContent+="</CDROMVector>\r\n";

    strProtocolContent+="<burnType>";
    strProtocolContent+=RTBurnInfo.GetCommonBurnParam().GetBurnType();
    strProtocolContent+="</burnType>\r\n";

    strProtocolContent+="<burnPassword>";
    strProtocolContent+=RTBurnInfo.GetCommonBurnParam().GetBurnPassword();
    strProtocolContent+="</burnPassword>\r\n";

    strProtocolContent+="<contentPassword>";
    strProtocolContent+=RTBurnInfo.GetCommonBurnParam().GetContentPassword();
    strProtocolContent+="</contentPassword>\r\n";

    const std::vector<CLocationInfo> &vecVideoLocationInfo=RTBurnInfo.GetVecVideoLocationInfo();

    strProtocolContent+="<videoLocationVector>\r\n";

    for (size_t j=0;j<vecVideoLocationInfo.size();++j)
    {
        strProtocolContent+="<videoLocationStruct>\r\n";

        strProtocolContent+="<locationID>";
        strProtocolContent+=vecVideoLocationInfo[j].GetID();
        strProtocolContent+="</locationID>\r\n";

        strProtocolContent+="<locationDescription>";
        strProtocolContent+=vecVideoLocationInfo[j].GetDescription();
        strProtocolContent+="</locationDescription>\r\n";

        strProtocolContent+="<locationBurnOrNot>";
        strProtocolContent+=vecVideoLocationInfo.at(j).GetBurnOrNot();
        strProtocolContent+="</locationBurnOrNot>\r\n";

        strProtocolContent+="</videoLocationStruct>\r\n";
    }

    strProtocolContent+="</videoLocationVector>\r\n";

    strProtocolContent+="<newFileName>";
    strProtocolContent+=RTBurnInfo.GetNewFileName();
    strProtocolContent+="</newFileName>\r\n";

    strProtocolContent+="<CDAlarmLimit>";
    strProtocolContent+=RTBurnInfo.GetCDAlarmLimit();
    strProtocolContent+="</CDAlarmLimit>\r\n";

    strProtocolContent+="<burnBufferSize>";
    strProtocolContent+=RTBurnInfo.GetBurnBufferSize();
    strProtocolContent+="</burnBufferSize>\r\n";

    strProtocolContent+="</burnControl>";

    return strProtocolContent;
}

std::string CInterfaceProtocol::GenerateProtocolSendRTBurnStateToUpper(const CRTBurnInfo &RTBurnInfo)
{
    std::string strProtocolContent="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    strProtocolContent+="\r\n";

    strProtocolContent+="<burnControl>\r\n";

    strProtocolContent+="<commandType>sendRTBurnState</commandType>\r\n";

    strProtocolContent+="<jobID>";
    strProtocolContent+=RTBurnInfo.GetJobID();
    strProtocolContent+="</jobID>\r\n";
    strProtocolContent+="<stateFlag>";
    strProtocolContent+=RTBurnInfo.GetStateFlag();
    strProtocolContent+="</stateFlag>\r\n";
    strProtocolContent+="<stateDescription>";
    strProtocolContent+=RTBurnInfo.GetStateDescription();
    strProtocolContent+="</stateDescription>\r\n";

    strProtocolContent+="</burnControl>";

    return strProtocolContent;
}

std::string CInterfaceProtocol::GenerateProtocolSendRTBurnRequestToUpper(std::string strJobID,std::string strRequestType)
{
    std::string strProtocolContent="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    strProtocolContent+="\r\n";

    strProtocolContent+="<burnControl>\r\n";

    strProtocolContent+="<commandType>sendRTBurnRequest</commandType>\r\n";

    strProtocolContent+="<jobID>";
    strProtocolContent+=strJobID;
    strProtocolContent+="</jobID>\r\n";
    strProtocolContent+="<requestType>";
    strProtocolContent+=strRequestType;
    strProtocolContent+="</requestType>\r\n";

    strProtocolContent+="</burnControl>";

    return strProtocolContent;
}

std::string CInterfaceProtocol::GenerateProtocolStartRTBurnToBurnServer(const CRTBurnInfo &RTBurnInfo)
{
    std::string strProtocolContent="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    strProtocolContent+="\r\n";

    strProtocolContent+="<burnControl>\r\n";

    strProtocolContent+="<commandType>startRTBurn</commandType>\r\n";

    strProtocolContent+="<jobID>";
    strProtocolContent+=RTBurnInfo.GetJobID();
    strProtocolContent+="</jobID>\r\n";

    //播放器信息
    strProtocolContent+="<playerInfoStruct>\r\n";
    strProtocolContent+="<playerVersion>";
    strProtocolContent+=RTBurnInfo.GetPlayerInfo().GetVersion();
    strProtocolContent+="</playerVersion>\r\n";
    strProtocolContent+="<playerDownloadURL>";
    strProtocolContent+=RTBurnInfo.GetPlayerInfo().GetDownloadURL();
    strProtocolContent+="</playerDownloadURL>\r\n";
    strProtocolContent+="<remoteIP>";
    strProtocolContent+=RTBurnInfo.GetPlayerInfo().GetRemoteIP();
    strProtocolContent+="</remoteIP>\r\n";
    strProtocolContent+="<remotePort>";
    strProtocolContent+=RTBurnInfo.GetPlayerInfo().GetRemotePort();
    strProtocolContent+="</remotePort>\r\n";
    strProtocolContent+="</playerInfoStruct>\r\n";

    //播放列表文件信息
    strProtocolContent+="<playlistInfoStruct>\r\n";
    strProtocolContent+="<playlistDownloadURL>";
    strProtocolContent+=RTBurnInfo.GetPlaylistFileInfo().GetDownloadURL();
    strProtocolContent+="</playlistDownloadURL>\r\n";
    strProtocolContent+="<remoteIP>";
    strProtocolContent+=RTBurnInfo.GetPlaylistFileInfo().GetRemoteIP();
    strProtocolContent+="</remoteIP>\r\n";
    strProtocolContent+="<remotePort>";
    strProtocolContent+=RTBurnInfo.GetPlaylistFileInfo().GetRemotePort();
    strProtocolContent+="</remotePort>\r\n";
    strProtocolContent+="</playlistInfoStruct>\r\n";

    //笔录文件信息
    strProtocolContent+="<noteFileRemoteIP>";
    strProtocolContent+=RTBurnInfo.GetNoteFileRemoteIP();
    strProtocolContent+="</noteFileRemoteIP>\r\n";
    strProtocolContent+="<noteFileRemotePort>";
    strProtocolContent+=RTBurnInfo.GetNoteFileRemotePort();
    strProtocolContent+="</noteFileRemotePort>\r\n";
    strProtocolContent+="<noteFileDownloadURL>";
    strProtocolContent+=RTBurnInfo.GetNoteFileDownloadURL();
    strProtocolContent+="</noteFileDownloadURL>\r\n";

    //autorun.inf文件信息
    strProtocolContent+="<autorunFileRemoteIP>";
    strProtocolContent+=RTBurnInfo.GetAutorunFileRemoteIP();
    strProtocolContent+="</autorunFileRemoteIP>\r\n";
    strProtocolContent+="<autorunFileRemotePort>";
    strProtocolContent+=RTBurnInfo.GetAutorunFileRemotePort();
    strProtocolContent+="</autorunFileRemotePort>\r\n";
    strProtocolContent+="<autorunFileDownloadURL>";
    strProtocolContent+=RTBurnInfo.GetAutorunFilePath();
    strProtocolContent+="</autorunFileDownloadURL>\r\n";

    strProtocolContent+="<burnType>";
    strProtocolContent+=RTBurnInfo.GetCommonBurnParam().GetBurnType();
    strProtocolContent+="</burnType>\r\n";

    strProtocolContent+="<burnPassword>";
    strProtocolContent+=RTBurnInfo.GetCommonBurnParam().GetBurnPassword();
    strProtocolContent+="</burnPassword>\r\n";

    strProtocolContent+="<contentPassword>";
    strProtocolContent+=RTBurnInfo.GetCommonBurnParam().GetContentPassword();
    strProtocolContent+="</contentPassword>\r\n";

    strProtocolContent+="<CDName>";
    strProtocolContent+=RTBurnInfo.GetCommonBurnParam().GetCDName();
    strProtocolContent+="</CDName>\r\n";

    //光驱列表信息
    strProtocolContent+="<CDROMVector>\r\n";
    const std::vector<CCDROMDriverInfo> &vecCDROM=RTBurnInfo.GetBurnServerInfo().GetVecCDROMDriverInfo();
    for (size_t i=0;i<vecCDROM.size();++i)
    {
        if ("1" == vecCDROM[i].GetIsSelected())
        {
            strProtocolContent+="<CDROMStruct>\r\n";

            strProtocolContent+="<CDROMID>";
            strProtocolContent+=vecCDROM[i].GetID();
            strProtocolContent+="</CDROMID>\r\n";

            strProtocolContent+="<CDROMDescription>";
            strProtocolContent+=vecCDROM[i].GetDescription();
            strProtocolContent+="</CDROMDescription>\r\n";

            strProtocolContent+="<videoLocationVector>\r\n";

            //当前处理方式是将刻录内容保存在实时刻录类实例里面
            const std::vector<CLocationInfo> &vecVideoLocationInfo=RTBurnInfo.GetVecVideoLocationInfo();
            for (size_t j=0;j<vecVideoLocationInfo.size();++j)
            {
                if (BURN_YES == vecVideoLocationInfo.at(j).GetBurnOrNot())
                {
                    strProtocolContent+="<videoLocationStruct>\r\n";

                    strProtocolContent+="<locationID>";
                    strProtocolContent+=vecVideoLocationInfo[j].GetID();
                    strProtocolContent+="</locationID>\r\n";

                    strProtocolContent+="<locationDescription>";
                    strProtocolContent+=vecVideoLocationInfo[j].GetDescription();
                    strProtocolContent+="</locationDescription>\r\n";

                    strProtocolContent+="<locationURL>";
                    strProtocolContent+=vecVideoLocationInfo[j].GetURL();
                    strProtocolContent+="</locationURL>\r\n";

                    strProtocolContent+="</videoLocationStruct>\r\n";
                }
            }

            strProtocolContent+="</videoLocationVector>\r\n";

            strProtocolContent+="</CDROMStruct>\r\n";
        }
    }
    strProtocolContent+="</CDROMVector>\r\n";

    strProtocolContent+="<newFileName>";
    if (RTBurnInfo.GetNewFileName() == "")
    {
        strProtocolContent+="trial.ts";
    }
    else
    {
        strProtocolContent+=RTBurnInfo.GetNewFileName();
    }
    strProtocolContent+="</newFileName>\r\n";

    strProtocolContent+="<CDAlarmLimit>";
    strProtocolContent+=RTBurnInfo.GetCDAlarmLimit();
    strProtocolContent+="</CDAlarmLimit>\r\n";

    strProtocolContent+="<burnBufferSize>";
    strProtocolContent+=RTBurnInfo.GetBurnBufferSize();
    strProtocolContent+="</burnBufferSize>\r\n";

    strProtocolContent+="<upServerIP>";
    strProtocolContent+=NetUtil::GetHostIP();
    strProtocolContent+="</upServerIP>\r\n";
    strProtocolContent+="<upServerPort>";
    strProtocolContent+=IntToString(BURNCONTROL_SERVER_PORT);
    strProtocolContent+="</upServerPort>\r\n";

    strProtocolContent+="</burnControl>";

    return strProtocolContent;
}

std::string CInterfaceProtocol::GenerateProtocolStartBurnToBurnServer(const CNormalBurnJobInfoEx &normalJob)
{
    std::string strProtocolContent="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    strProtocolContent+="\r\n";

    strProtocolContent+="<burnControl>\r\n";

    strProtocolContent+="<commandType>startBurn</commandType>\r\n";

    strProtocolContent+="<jobID>";
    strProtocolContent+=normalJob.GetJobID();
    strProtocolContent+="</jobID>\r\n";

    strProtocolContent+="<burnServerType>";
    strProtocolContent+=normalJob.GetBurnServerInfo().GetBurnServerType();
    strProtocolContent+="</burnServerType>\r\n";

    //播放器信息
    strProtocolContent+="<playerInfoStruct>\r\n";

    strProtocolContent+="<playerVersion>";
    strProtocolContent+=normalJob.GetPlayerInfo().GetVersion();
    strProtocolContent+="</playerVersion>\r\n";

    strProtocolContent+="<playerDownloadURL>";
    strProtocolContent+=normalJob.GetPlayerInfo().GetDownloadURL();
    strProtocolContent+="</playerDownloadURL>\r\n";

    strProtocolContent+="<remoteIP>";
    strProtocolContent+=normalJob.GetPlayerInfo().GetRemoteIP();
    strProtocolContent+="</remoteIP>\r\n";

    strProtocolContent+="<remotePort>";
    strProtocolContent+=normalJob.GetPlayerInfo().GetRemotePort();
    strProtocolContent+="</remotePort>\r\n";

    strProtocolContent+="</playerInfoStruct>\r\n";

    //播放列表文件信息
    strProtocolContent+="<playlistInfoStruct>\r\n";
    strProtocolContent+="<playlistDownloadURL>";
    strProtocolContent+=normalJob.GetPlaylistFileInfo().GetDownloadURL();
    strProtocolContent+="</playlistDownloadURL>\r\n";
    strProtocolContent+="<remoteIP>";
    strProtocolContent+=normalJob.GetPlaylistFileInfo().GetRemoteIP();
    strProtocolContent+="</remoteIP>\r\n";
    strProtocolContent+="<remotePort>";
    strProtocolContent+=normalJob.GetPlaylistFileInfo().GetRemotePort();
    strProtocolContent+="</remotePort>\r\n";
    strProtocolContent+="</playlistInfoStruct>\r\n";

    //笔录文件信息
    if (normalJob.GetBurnNoteFile())
    {
        strProtocolContent+="<noteFileRemoteIP>";
        strProtocolContent+=normalJob.GetNoteFileRemoteIP();
        strProtocolContent+="</noteFileRemoteIP>\r\n";
        strProtocolContent+="<noteFileRemotePort>";
        strProtocolContent+=normalJob.GetNoteFileRemotePort();
        strProtocolContent+="</noteFileRemotePort>\r\n";
        strProtocolContent+="<noteFileDownloadURL>";
        strProtocolContent+=normalJob.GetNoteRelativePath();
        strProtocolContent+="</noteFileDownloadURL>\r\n";
        strProtocolContent+="<noteFileName>";
        strProtocolContent+=normalJob.GetNoteFileName();
        strProtocolContent+="</noteFileName>\r\n";
    }
    else
    {
        strProtocolContent+="<noteFileRemoteIP>";
        strProtocolContent+="";
        strProtocolContent+="</noteFileRemoteIP>\r\n";
        strProtocolContent+="<noteFileRemotePort>";
        strProtocolContent+="";
        strProtocolContent+="</noteFileRemotePort>\r\n";
        strProtocolContent+="<noteFileDownloadURL>";
        strProtocolContent+="";
        strProtocolContent+="</noteFileDownloadURL>\r\n";
        strProtocolContent+="<noteFileName>";
        strProtocolContent+="";
        strProtocolContent+="</noteFileName>\r\n";
    }

    //autorun.inf文件信息
    strProtocolContent+="<autorunFileRemoteIP>";
    strProtocolContent+=normalJob.GetAutorunFileRemoteIP();
    strProtocolContent+="</autorunFileRemoteIP>\r\n";
    strProtocolContent+="<autorunFileRemotePort>";
    strProtocolContent+=normalJob.GetAutorunFileRemotePort();
    strProtocolContent+="</autorunFileRemotePort>\r\n";
    strProtocolContent+="<autorunFileDownloadURL>";
    strProtocolContent+=normalJob.GetAutorunFilePath();
    strProtocolContent+="</autorunFileDownloadURL>\r\n";

    //刻录方式很重要，BurnServer依据这个字段实现不同的刻录逻辑
    strProtocolContent+="<burnType>";
    strProtocolContent+=normalJob.GetCommonBurnParam().GetBurnType();
    strProtocolContent+="</burnType>\r\n";

    strProtocolContent+="<burnPassword>";
    strProtocolContent+=normalJob.GetCommonBurnParam().GetBurnPassword();
    strProtocolContent+="</burnPassword>\r\n";

    strProtocolContent+="<contentPassword>";
    strProtocolContent+=normalJob.GetCommonBurnParam().GetContentPassword();
    strProtocolContent+="</contentPassword>\r\n";

    strProtocolContent+="<CDName>";
    strProtocolContent+=normalJob.GetCommonBurnParam().GetCDName();
    strProtocolContent+="</CDName>\r\n";

    strProtocolContent+="<videoFileRemoteIP>";
    strProtocolContent+=normalJob.GetVideoFileRemoteIP();
    strProtocolContent+="</videoFileRemoteIP>\r\n";

    strProtocolContent+="<videoFileRemotePort>";
    strProtocolContent+=normalJob.GetVideoFileRemotePort();
    strProtocolContent+="</videoFileRemotePort>\r\n";

    strProtocolContent+="<videoFileDownloadURL>";
    strProtocolContent+=normalJob.GetVideoFileRelativePath();
    strProtocolContent+="</videoFileDownloadURL>\r\n";

    std::string strBurnServerType=normalJob.GetBurnServerInfo().GetBurnServerType();

    bool bFlag=( strBurnServerType != "1" );//判断当前任务选择的是不是非普通刻录服务器

    //光驱列表信息
    strProtocolContent+="<CDROMVector>\r\n";
    std::vector<CCDROMDriverInfo> vecCDROMDriver=normalJob.GetBurnServerInfo().GetVecCDROMDriverInfo();

    if (bFlag && vecCDROMDriver.empty())
    {
        //对于仅配置了派美雅或者爱普生的机器，无法用普通光驱承载刻录内容（虽然这些普通光驱不会被使用）
        //因此构造虚拟的光驱用来承载刻录内容
        CCDROMDriverInfo devInfo;
        devInfo.SetID("9999");
        vecCDROMDriver.push_back(devInfo);
    }

    for (size_t i=0;i<vecCDROMDriver.size();++i)
    {
        bool bAdd=false;

        if (bFlag)
        {
            bAdd=true;
        }
        else
        {
            bAdd=( "1" == vecCDROMDriver[i].GetIsSelected() );
        }

        //如果光驱被选中
        if (bAdd)
        {
            strProtocolContent+="<CDROMStruct>\r\n";

            strProtocolContent+="<CDROMID>";
            strProtocolContent+=vecCDROMDriver[i].GetID();
            strProtocolContent+="</CDROMID>\r\n";

            strProtocolContent+="<CDROMDescription>";
            strProtocolContent+=vecCDROMDriver[i].GetDescription();
            strProtocolContent+="</CDROMDescription>\r\n";

            strProtocolContent+="<videoLocationVector>\r\n";
            const std::vector<CLocationInfo> &vecVideoLocationInfo=normalJob.GetVecLocationInfo();
            for (size_t j=0;j<vecVideoLocationInfo.size();++j)
            {
                if (BURN_NO == vecVideoLocationInfo.at(j).GetBurnOrNot())
                {
                    continue;
                }

                strProtocolContent+="<videoLocationStruct>\r\n";

                strProtocolContent+="<locationID>";
                strProtocolContent+=vecVideoLocationInfo[j].GetID();
                strProtocolContent+="</locationID>\r\n";
                strProtocolContent+="<locationDescription>";
                strProtocolContent+=vecVideoLocationInfo[j].GetDescription();
                strProtocolContent+="</locationDescription>\r\n";
                strProtocolContent+="<streamType>";
                strProtocolContent+=vecVideoLocationInfo[j].GetStreamType();
                strProtocolContent+="</streamType>\r\n";

                strProtocolContent+="<videoFileVector>\r\n";
                const std::vector<CSingleFileInfo> &vecVideoFileInfo=vecVideoLocationInfo.at(j).GetVecFileInfo();
                for (size_t i=0;i<vecVideoFileInfo.size();++i)
                {
                    strProtocolContent+="<videoFileStruct>\r\n";

                    strProtocolContent+="<videoFileName>";
                    strProtocolContent+=vecVideoFileInfo[i].GetFileName();
                    strProtocolContent+="</videoFileName>\r\n";
                    strProtocolContent+="<newVideoFileName>";
                    strProtocolContent+=vecVideoFileInfo[i].GetNewFileName();
                    strProtocolContent+="</newVideoFileName>\r\n";

                    strProtocolContent+="</videoFileStruct>\r\n";
                }
                strProtocolContent+="</videoFileVector>\r\n";

                strProtocolContent+="</videoLocationStruct>\r\n";
            }
            strProtocolContent+="</videoLocationVector>\r\n";

            strProtocolContent+="</CDROMStruct>\r\n";
        }
    }
    strProtocolContent+="</CDROMVector>\r\n";

    strProtocolContent+="<printWordVector>\r\n";
    std::map<std::string,std::string>::const_iterator iter;
    for (iter=normalJob.GetMapLabelFieldText().begin();
        iter!=normalJob.GetMapLabelFieldText().end();++iter)
    {
        strProtocolContent+="<printWordStruct>\r\n";

        strProtocolContent+="<itemID>";
        strProtocolContent+=iter->first;
        strProtocolContent+="</itemID>\r\n";
        strProtocolContent+="<itemValue>";
        strProtocolContent+=iter->second;
        strProtocolContent+="</itemValue>\r\n";

        strProtocolContent+="</printWordStruct>\r\n";
    }
    strProtocolContent+="</printWordVector>\r\n";

    strProtocolContent+="<upServerIP>";
    strProtocolContent+=NetUtil::GetHostIP();
    strProtocolContent+="</upServerIP>\r\n";
    strProtocolContent+="<upServerPort>";
    strProtocolContent+=IntToString(BURNCONTROL_SERVER_PORT);
    strProtocolContent+="</upServerPort>\r\n";

    strProtocolContent+="</burnControl>";

    return strProtocolContent;
}

std::string CInterfaceProtocol::GenerateProtocolSendBurnCtrolCMDToBurnServer(std::string strJobID,std::string strCMD)
{
    std::string strProtocolContent="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    strProtocolContent+="\r\n";

    strProtocolContent+="<burnControl>\r\n";

    strProtocolContent+="<commandType>sendBurnCtrlCMD</commandType>\r\n";

    strProtocolContent+="<jobID>";
    strProtocolContent+=strJobID;
    strProtocolContent+="</jobID>\r\n";
    strProtocolContent+="<ctrlCMD>";
    strProtocolContent+=strCMD;
    strProtocolContent+="</ctrlCMD>\r\n";
    strProtocolContent+="<upServerIP>";
    strProtocolContent+=NetUtil::GetHostIP();
    strProtocolContent+="</upServerIP>\r\n";
    strProtocolContent+="<upServerPort>";
    strProtocolContent+=IntToString(BURNCONTROL_SERVER_PORT);
    strProtocolContent+="</upServerPort>\r\n";

    strProtocolContent+="</burnControl>";

    return strProtocolContent;
}

void CInterfaceProtocol::AnalyzeProtocolStartRTBurnToBurnServer(const std::string &strProtocolContent,CRTBurnInfo &RTBurnInfoEx)
{
    RTBurnInfoEx.SetJobID(GetTagValue(strProtocolContent,"jobID"));

    std::string strPlayerInfoStruct=GetTagValue(strProtocolContent,"playerInfoStruct");
    if (strPlayerInfoStruct != "")
    {
        RTBurnInfoEx.GetPlayerInfo().SetVersion(GetTagValue(strPlayerInfoStruct,"playerVersion"));
        RTBurnInfoEx.GetPlayerInfo().SetDownloadURL(GetTagValue(strPlayerInfoStruct,"playerDownloadURL"));
        RTBurnInfoEx.GetPlayerInfo().SetRemoteIP(GetTagValue(strPlayerInfoStruct,"remoteIP"));
        RTBurnInfoEx.GetPlayerInfo().SetRemotePort(GetTagValue(strPlayerInfoStruct,"remotePort"));
    }

    std::string strPlaylistInfoStruct=GetTagValue(strProtocolContent,"playlistInfoStruct");
    if (strPlaylistInfoStruct != "")
    {
        RTBurnInfoEx.GetPlaylistFileInfo().SetDownloadURL(GetTagValue(strPlayerInfoStruct,"playlistDownloadURL"));
        RTBurnInfoEx.GetPlaylistFileInfo().SetRemoteIP(GetTagValue(strPlayerInfoStruct,"remoteIP"));
        RTBurnInfoEx.GetPlaylistFileInfo().SetRemotePort(GetTagValue(strPlayerInfoStruct,"remotePort"));
    }

    RTBurnInfoEx.SetNoteFileDownloadURL(GetTagValue(strProtocolContent,"noteFileDownloadURL"));
    RTBurnInfoEx.SetNoteFileRemoteIP(GetTagValue(strProtocolContent,"noteFileRemoteIP"));
    RTBurnInfoEx.SetNoteFileRemotePort(GetTagValue(strProtocolContent,"noteFileRemotePort"));

    RTBurnInfoEx.SetAutorunFileRemoteIP(GetTagValue(strProtocolContent,"autorunFileRemoteIP"));
    RTBurnInfoEx.SetAutorunFileRemotePort(GetTagValue(strProtocolContent,"autorunFileRemotePort"));
    RTBurnInfoEx.SetAutorunFilePath(GetTagValue(strProtocolContent,"autorunFileDownloadURL"));

    RTBurnInfoEx.GetCommonBurnParam().SetBurnType(GetTagValue(strProtocolContent,"burnType"));
    RTBurnInfoEx.GetCommonBurnParam().SetBurnPassword(GetTagValue(strProtocolContent,"burnPassword"));
    RTBurnInfoEx.GetCommonBurnParam().SetContentPassword(GetTagValue(strProtocolContent,"contentPassword"));
    RTBurnInfoEx.GetCommonBurnParam().SetCDName(GetTagValue(strProtocolContent,"CDName"));

    std::string strCDROMVector=GetTagValue(strProtocolContent,"CDROMVector");
    int nRelativePos=0;
    std::string strCDROMStruct=GetTagValueWithStartPos(strCDROMVector,"CDROMStruct",nRelativePos);
    while (strCDROMStruct != "")
    {
        CCDROMDriverInfo CDROMInfo;
        CDROMInfo.SetID(GetTagValue(strCDROMStruct,"CDROMID"));
        CDROMInfo.SetDescription(GetTagValue(strCDROMStruct,"CDROMDescription"));

        std::string strVideoLocationVector=GetTagValue(strCDROMStruct,"videoLocationVector");
        int nVideoLocationRelativePos=0;
        std::string strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,"videoLocationStruct",
            nVideoLocationRelativePos);
        while (strVideoLocationStruct != "")
        {
            CLocationInfo videoLocationInfo;
            videoLocationInfo.SetID(GetTagValue(strVideoLocationStruct,"locationID"));
            videoLocationInfo.SetDescription(GetTagValue(strVideoLocationStruct,"locationDescription"));
            videoLocationInfo.SetURL(GetTagValue(strVideoLocationStruct,"locationURL"));

            CDROMInfo.GetVecLocationInfo().push_back(videoLocationInfo);
            strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,
                "videoLocationStruct",nVideoLocationRelativePos);
        }

        RTBurnInfoEx.GetBurnServerInfo().GetVecCDROMDriverInfo().push_back(CDROMInfo);
        strCDROMStruct=GetTagValueWithStartPos(strCDROMVector,"CDROMStruct",nRelativePos);
    }

    RTBurnInfoEx.SetNewFileName(GetTagValue(strProtocolContent,"newFileName"));

    RTBurnInfoEx.SetUpServerIP(GetTagValue(strProtocolContent,"upServerIP"));
    RTBurnInfoEx.SetUpServerPort(GetTagValue(strProtocolContent,"upServerPort"));

    RTBurnInfoEx.SetHardDiskBackup(GetTagValue(strProtocolContent,"hardDiskBackup"));
    RTBurnInfoEx.SetSpeed(GetTagValue(strProtocolContent,"speed"));
    RTBurnInfoEx.SetCDAlarmLimit(GetTagValue(strProtocolContent,"CDAlarmLimit"));
    RTBurnInfoEx.SetCDAutoFormat(GetTagValue(strProtocolContent,"CDAutoFormat"));
    RTBurnInfoEx.SetBurnBufferSize(GetTagValue(strProtocolContent,"burnBufferSize"));
    RTBurnInfoEx.SetHardDiskBakAlarmLimit(GetTagValue(strProtocolContent,"hardDiskBakAlarmLimit"));
    RTBurnInfoEx.SetStreamType(GetTagValue(strProtocolContent,"streamType"));
    RTBurnInfoEx.SetMd5(GetTagValue(strProtocolContent,"md5"));
}

void CInterfaceProtocol::AnalyzeProtocolStarBurnToBurnServer(const std::string &strProtocolContent,CNormalBurnJobInfoEx &normalBurnJobInfoEx)
{
    normalBurnJobInfoEx.SetJobID(GetTagValue(strProtocolContent,"jobID"));

    normalBurnJobInfoEx.GetBurnServerInfo().SetBurnServerType(GetTagValue(strProtocolContent,"burnServerType"));

    std::string strPlayerInfoStruct=GetTagValue(strProtocolContent,"playerInfoStruct");
    if (strPlayerInfoStruct != "")
    {
        normalBurnJobInfoEx.GetPlayerInfo().SetVersion(GetTagValue(strPlayerInfoStruct,"playerVersion"));
        normalBurnJobInfoEx.GetPlayerInfo().SetDownloadURL(GetTagValue(strPlayerInfoStruct,"playerDownloadURL"));
        normalBurnJobInfoEx.GetPlayerInfo().SetRemoteIP(GetTagValue(strPlayerInfoStruct,"remoteIP"));
        normalBurnJobInfoEx.GetPlayerInfo().SetRemotePort(GetTagValue(strPlayerInfoStruct,"remotePort"));
    }

    std::string strPlaylistInfoStruct=GetTagValue(strProtocolContent,"playlistInfoStruct");
    if (strPlaylistInfoStruct != "")
    {
        normalBurnJobInfoEx.GetPlaylistFileInfo().SetDownloadURL(GetTagValue(strPlaylistInfoStruct,"playlistDownloadURL"));//playlistDownloadURL
        normalBurnJobInfoEx.GetPlaylistFileInfo().SetRemoteIP(GetTagValue(strPlaylistInfoStruct,"remoteIP"));
        normalBurnJobInfoEx.GetPlaylistFileInfo().SetRemotePort(GetTagValue(strPlaylistInfoStruct,"remotePort"));
    }

    normalBurnJobInfoEx.SetNoteFileRemoteIP(GetTagValue(strProtocolContent,"noteFileRemoteIP"));
    normalBurnJobInfoEx.SetNoteFileRemotePort(GetTagValue(strProtocolContent,"noteFileRemotePort"));
    normalBurnJobInfoEx.SetNoteRelativePath(GetTagValue(strProtocolContent,"noteFileDownloadURL"));
    normalBurnJobInfoEx.SetNoteFileName(GetTagValue(strProtocolContent,"noteFileName"));

    normalBurnJobInfoEx.SetAutorunFileRemoteIP(GetTagValue(strProtocolContent,"autorunFileRemoteIP"));
    normalBurnJobInfoEx.SetAutorunFileRemotePort(GetTagValue(strProtocolContent,"autorunFileRemotePort"));
    normalBurnJobInfoEx.SetAutorunFilePath(GetTagValue(strProtocolContent,"autorunFileDownloadURL"));

    normalBurnJobInfoEx.GetCommonBurnParam().SetBurnType(GetTagValue(strProtocolContent,"burnType"));
    normalBurnJobInfoEx.GetCommonBurnParam().SetBurnPassword(GetTagValue(strProtocolContent,"burnPassword"));
    normalBurnJobInfoEx.GetCommonBurnParam().SetContentPassword(GetTagValue(strProtocolContent,"contentPassword"));
    normalBurnJobInfoEx.GetCommonBurnParam().SetCDName(GetTagValue(strProtocolContent,"CDName"));

    normalBurnJobInfoEx.SetVideoFileRemoteIP(GetTagValue(strProtocolContent,"videoFileRemoteIP"));
    normalBurnJobInfoEx.SetVideoFileRemotePort(GetTagValue(strProtocolContent,"videoFileRemotePort"));
    normalBurnJobInfoEx.SetVideoFileRelativePath(GetTagValue(strProtocolContent,"videoFileDownloadURL"));

    std::string strCDROMVector=GetTagValue(strProtocolContent,"CDROMVector");
    int nRelativePos=0;
    std::string strCDROMStruct=GetTagValueWithStartPos(strCDROMVector,"CDROMStruct",nRelativePos);
    while (strCDROMStruct != "")
    {
        CCDROMDriverInfo CDROMInfo;
        CDROMInfo.SetID(GetTagValue(strCDROMStruct,"CDROMID"));
        CDROMInfo.SetDescription(GetTagValue(strCDROMStruct,"CDROMDescription"));

        std::string strVideoLocationVector=GetTagValue(strProtocolContent,"videoLocationVector");
        int nVideoLocationRelativePos=0;
        std::string strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,"videoLocationStruct",
            nVideoLocationRelativePos);
        while (strVideoLocationStruct != "")
        {
            CLocationInfo videoLocationInfo;
            videoLocationInfo.SetID(GetTagValue(strVideoLocationStruct,"locationID"));
            videoLocationInfo.SetDescription(GetTagValue(strVideoLocationStruct,"locationDescription"));

            std::string strVideoFileVector=GetTagValue(strVideoLocationStruct,"videoFileVector");
            int nVideoFileVectorRelativePos=0;
            std::string strVideoFileStruct=GetTagValueWithStartPos(strVideoFileVector,"videoFileStruct",
                nVideoFileVectorRelativePos);
            while (strVideoFileStruct != "")
            {
                CSingleFileInfo videoFileInfo;
                videoFileInfo.SetFileName(GetTagValue(strVideoFileStruct,"videoFileName"));
                videoFileInfo.SetNewFileName(GetTagValue(strVideoFileStruct,"newVideoFileName"));

                videoLocationInfo.GetVecFileInfo().push_back(videoFileInfo);
                strVideoFileStruct=GetTagValueWithStartPos(strVideoFileVector,"videoFileStruct",
                    nVideoFileVectorRelativePos);
            }

            CDROMInfo.GetVecLocationInfo().push_back(videoLocationInfo);
            strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,"videoLocationStruct",
                nVideoLocationRelativePos);
        }

        normalBurnJobInfoEx.GetBurnServerInfo().GetVecCDROMDriverInfo().push_back(CDROMInfo);
        strCDROMStruct=GetTagValueWithStartPos(strCDROMVector,"CDROMStruct",nRelativePos);
    }

    normalBurnJobInfoEx.SetUpServerIP(GetTagValue(strProtocolContent,"upServerIP"));
    normalBurnJobInfoEx.SetUpServerPort(GetTagValue(strProtocolContent,"upServerPort"));
}

std::string CInterfaceProtocol::GenerateProtocolSendBurnState(std::string strJobID,
                                                              std::string strState,std::string strStateDescription,
                                                              std::vector<CCDROMDriverInfo> &vecCDROMDriver)
{
    std::string strProtocolContent="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    strProtocolContent+="\r\n";

    strProtocolContent+="<burnControl>\r\n";

    strProtocolContent+="<commandType>sendBurnState</commandType>\r\n";

    strProtocolContent+="<jobID>";
    strProtocolContent+=strJobID;
    strProtocolContent+="</jobID>\r\n";

    strProtocolContent+="<stateFlag>";
    strProtocolContent+=strState;
    strProtocolContent+="</stateFlag>\r\n";

    strProtocolContent+="<stateDescription>";
    strProtocolContent+=strStateDescription;
    strProtocolContent+="</stateDescription>\r\n";

    strProtocolContent+="</burnControl>";

    return strProtocolContent;
}

void CInterfaceProtocol::AnalyzeProtocolSendBurnState(const std::string &strProtocolContent,std::vector<CCDROMDriverInfo> &vecCDROMInfo)
{
    vecCDROMInfo.clear();

    std::string strCDROMVector=GetTagValue(strProtocolContent,"CDROMVector");
    int nRelativePos=0;
    std::string strCDROMStruct=GetTagValueWithStartPos(strCDROMVector,"CDROMStruct",nRelativePos);
    while (strCDROMStruct != "")
    {
        CCDROMDriverInfo CDROMDriverInfo;
        CDROMDriverInfo.SetID(GetTagValue(strCDROMStruct,"CDROMID"));
        CDROMDriverInfo.SetStateFlag(GetTagValue(strCDROMStruct,"stateFlag"));
        CDROMDriverInfo.SetStateDescription(GetTagValue(strCDROMStruct,"stateDescription"));

        std::string strVideoLocationVector=GetTagValue(strProtocolContent,"videoLocationVector");
        int nVideoLocationRelativePos=0;
        std::string strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,"videoLocationStruct",
            nVideoLocationRelativePos);
        while (strVideoLocationStruct != "")
        {
            CLocationInfo videoLocationInfo;
            videoLocationInfo.SetID(GetTagValue(strVideoLocationStruct,"locationID"));
            videoLocationInfo.SetDescription(GetTagValue(strVideoLocationStruct,"locationDescription"));

            std::string strVideoFileVector=GetTagValue(strVideoLocationStruct,"videoFileVector");
            int nVideoFileVectorRelativePos=0;
            std::string strVideoFileStruct=GetTagValueWithStartPos(strVideoFileVector,"videoFileStruct",
                nVideoFileVectorRelativePos);
            while (strVideoFileStruct != "")
            {
                CSingleFileInfo videoFileInfo;
                videoFileInfo.SetFileName(GetTagValue(strVideoFileStruct,"fileName"));
                videoFileInfo.SetBurnState(GetTagValue(strVideoFileStruct,"burnState"));
                videoFileInfo.SetSize(GetTagValue(strVideoFileStruct,"size"));
                videoFileInfo.SetPercent(GetTagValue(strVideoFileStruct,"percent"));

                videoLocationInfo.GetVecFileInfo().push_back(videoFileInfo);
                strVideoFileStruct=GetTagValueWithStartPos(strVideoFileVector,"videoFileStruct",
                    nVideoFileVectorRelativePos);
            }

            CDROMDriverInfo.GetVecLocationInfo().push_back(videoLocationInfo);
            strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,"videoLocationStruct",
                nVideoLocationRelativePos);
        }

        vecCDROMInfo.push_back(CDROMDriverInfo);
        strCDROMStruct=GetTagValueWithStartPos(strCDROMVector,"CDROMStruct",nRelativePos);
    }
}

std::string CInterfaceProtocol::GenerateProtocolCDROMDriverInfo(const std::vector<CNormalBurnJobInfoEx> &vecNormalBurnJobInfoEx,
                                                                const std::vector<CRTBurnInfo> &vecRTBurnInfo,
                                                                const std::vector<CCDROMDriverInfo> &vecCDROMDriver)
{
    std::string strProtocolContent;

    strProtocolContent+="<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n";
    strProtocolContent+="<burnControl>\r\n";
    strProtocolContent+="<CDROMVector>\r\n";

    std::vector<CCDROMDriverInfo>::const_iterator iter;
    for (iter=vecCDROMDriver.begin();iter!=vecCDROMDriver.end();++iter)
    {
        strProtocolContent+="<CDROMStruct>\r\n";

        strProtocolContent+="<CDROMID>";
        strProtocolContent+=iter->GetID();
        strProtocolContent+="</CDROMID>\r\n";

        strProtocolContent+="<CDROMDescription>";
        strProtocolContent+=iter->GetDescription();
        strProtocolContent+="</CDROMDescription>\r\n";

        strProtocolContent+="<stateFlag>";
        strProtocolContent+=iter->GetStateFlag();
        strProtocolContent+="</stateFlag>\r\n";

        strProtocolContent+="<stateDescription>";
        strProtocolContent+=iter->GetStateDescription();
        strProtocolContent+="</stateDescription>\r\n";

        strProtocolContent+="<hasCD>";
        strProtocolContent+=iter->GetHasCD();
        strProtocolContent+="</hasCD>\r\n";

        strProtocolContent+="<CDLeftCapacity>";
        strProtocolContent+=iter->GetCDLeftCapacity();
        strProtocolContent+="</CDLeftCapacity>\r\n";

        strProtocolContent+="<CDTotalCapacity>";
        strProtocolContent+=iter->GetCDTotalCapacity();
        strProtocolContent+="</CDTotalCapacity>\r\n";

        strProtocolContent+="<jobID>";
        strProtocolContent+=iter->GetJobID();
        strProtocolContent+="</jobID>\r\n";

        std::string strBurnType;

        //大费周章的返回刻录方式
        std::vector<CNormalBurnJobInfoEx>::const_iterator iterNormalBurnJobInfoEx;
        for (iterNormalBurnJobInfoEx=vecNormalBurnJobInfoEx.begin();
            iterNormalBurnJobInfoEx!=vecNormalBurnJobInfoEx.end();++iterNormalBurnJobInfoEx)
        {
            const std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo=
                iterNormalBurnJobInfoEx->GetBurnServerInfo().GetVecCDROMDriverInfo();
            size_t nCount=vecCDROMDriverInfo.size();
            for (size_t i=0;i<nCount;++i)
            {
                if (vecCDROMDriverInfo[i].GetID() == iter->GetID())
                {
                    strBurnType=iterNormalBurnJobInfoEx->GetCommonBurnParam().GetBurnType();
                    break;
                }
            }
        }

        if ("" == strBurnType)
        {
            std::vector<CRTBurnInfo>::const_iterator iterRTBurnInfo;
            for (iterRTBurnInfo=vecRTBurnInfo.begin();iterRTBurnInfo!=vecRTBurnInfo.end();
                ++iterRTBurnInfo)
            {
                const std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo=
                    iterRTBurnInfo->GetBurnServerInfo().GetVecCDROMDriverInfo();
                size_t nCount=vecCDROMDriverInfo.size();
                for (size_t i=0;i<nCount;++i)
                {
                    if (vecCDROMDriverInfo[i].GetID() == iter->GetID())
                    {
                        strBurnType=iterRTBurnInfo->GetCommonBurnParam().GetBurnType();
                        break;
                    }
                }
            }
        }

        strProtocolContent+="<burnType>";
        strProtocolContent+=strBurnType;
        strProtocolContent+="</burnType>\r\n";

        strProtocolContent+="</CDROMStruct>\r\n";
    }

    strProtocolContent+="</CDROMVector>\r\n";
    strProtocolContent+="</burnControl>\r\n";

    return strProtocolContent;
}

void CInterfaceProtocol::AnalyzeProtocolCDROMDriverInfo(const std::string &strProtocolContent,std::vector<CCDROMDriverInfo> &vecCDROMInfo)
{
    vecCDROMInfo.clear();

    std::string strCDROMDriverVec=CInterfaceProtocol::GetTagValue(strProtocolContent,
        "CDROMVector");
    int nRelativePosCDROMDriver=0;
    std::string strCDROMDriverStruct=CInterfaceProtocol::GetTagValueWithStartPos(
        strCDROMDriverVec,"CDROMStruct",nRelativePosCDROMDriver);
    while (strCDROMDriverStruct != "")
    {
        CCDROMDriverInfo CDROMDriverInfo;
        CDROMDriverInfo.SetID(GetTagValue(strCDROMDriverStruct,"CDROMID"));
        CDROMDriverInfo.SetStateFlag(GetTagValue(strCDROMDriverStruct,"stateFlag"));
        CDROMDriverInfo.SetStateDescription(GetTagValue(strCDROMDriverStruct,"stateDescription"));
        CDROMDriverInfo.SetJobID(GetTagValue(strCDROMDriverStruct,"jobID"));
        CDROMDriverInfo.SetIsSelected(DEV_SELECTED_NO);

        vecCDROMInfo.push_back(CDROMDriverInfo);

        strCDROMDriverStruct=CInterfaceProtocol::GetTagValueWithStartPos(
            strCDROMDriverVec,"CDROMStruct",nRelativePosCDROMDriver);
    }
}

void CInterfaceProtocol::AnalyzeProtocolBurnServerInfo(const std::string &strProtocolContent,std::vector<CBurnServerInfoEx> &vecBurnServerInfoEx)
{
    vecBurnServerInfoEx.clear();

    std::string strBurnServerVec=CInterfaceProtocol::GetTagValue(strProtocolContent,"burnServerVector");
    int nRelativePosBurnServer=0;
    std::string strBurnServerStruct=CInterfaceProtocol::GetTagValueWithStartPos(
        strBurnServerVec,"burnServerStruct",nRelativePosBurnServer);
    while (strBurnServerStruct != "")
    {
        CBurnServerInfo burnServerInfo;

        std::string strBurnServerType=CInterfaceProtocol::GetTagValue(strBurnServerStruct,"burnServerType");
        burnServerInfo.SetBurnServerType( (strBurnServerType!="") ? strBurnServerType : BURNSERVER_TYPE_NORMAL );
        burnServerInfo.SetBurnServerTypeDescription(CInterfaceProtocol::GetTagValue(strBurnServerStruct,"burnServerTypeDescription"));
        burnServerInfo.SetDescription(CInterfaceProtocol::GetTagValue(strBurnServerStruct,"burnServerDescription"));
        burnServerInfo.SetIsAvailable("1");

        vecBurnServerInfoEx.push_back(CBurnServerInfoEx(burnServerInfo));

        strBurnServerStruct=CInterfaceProtocol::GetTagValueWithStartPos(strBurnServerVec,"burnServerStruct",nRelativePosBurnServer);
    }
}

std::string CInterfaceProtocol::GenerateProtocolRTBurnBackupResponse(std::string strJobID,std::string strResponse)
{
    std::string strProtocolContent="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    strProtocolContent+="\r\n";

    strProtocolContent+="<burnControl>\r\n";

    strProtocolContent+="<commandType>sendRTBurnBackupResponse</commandType>\r\n";

    strProtocolContent+="<jobID>";
    strProtocolContent+=strJobID;
    strProtocolContent+="</jobID>\r\n";

    strProtocolContent+="<response>";
    strProtocolContent+=strResponse;
    strProtocolContent+="</response>\r\n";

    strProtocolContent+="</burnControl>";

    return strProtocolContent;
}

std::string CInterfaceProtocol::GenerateProtocolRTBurnBackupAfterRestartResponse(std::string strJobID,std::string strResponse)
{
    std::string strProtocolContent="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    strProtocolContent+="\r\n";

    strProtocolContent+="<burnControl>\r\n";

    strProtocolContent+="<commandType>sendRTBurnBackupAfterRestartResponse</commandType>\r\n";

    strProtocolContent+="<jobID>";
    strProtocolContent+=strJobID;
    strProtocolContent+="</jobID>\r\n";

    strProtocolContent+="<response>";
    strProtocolContent+=strResponse;
    strProtocolContent+="</response>\r\n";

    strProtocolContent+="</burnControl>";

    return strProtocolContent;
}

