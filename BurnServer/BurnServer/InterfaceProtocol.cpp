#include "InterfaceProtocol.h"

#include "CommonUtil.h"
#include "FileAnywhereManager.h"
#include "InterfaceServer.h"

#ifdef WIN32
#else
#define NDEBUG
#endif
#include <assert.h>

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

std::string CInterfaceProtocol::InterfaceCommunicate(std::string strIP, int nPort,const std::string &strContent,std::string strTag)
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
        LOG_INFO(("[CInterfaceProtocol::InterfaceCommunicate] Client %p,ip %s,port %d,content length %d,tag %s\r\n",
            hNCXClient,strIP.c_str(),nPort,strContent.length(),strTag.c_str()));

        ncxSetNCXClientOuterMostTagName(hNCXClient,const_cast<char *>(strTag.c_str()));

        if (ncxConnectServer(hNCXClient,strIP.c_str(),nPort,7))
        {
            int nRet=ncxCommunicateWithServer(hNCXClient, strContent.c_str(),strContent.length()+1,7);

            if (0 == nRet)
            {
                strResponse=ncxGetRespondProtocol(hNCXClient);
            }
            else
            {
                LOG_ERROR(("[CInterfaceProtocol::InterfaceCommunicate] ncxCommunicateWithServer failed,nRet=%d,%s,%d,%s\r\n",
                    nRet,strIP.c_str(),nPort,strTag.c_str()));
            }
        }
        else
        {
            LOG_ERROR(("[CInterfaceProtocol::InterfaceCommunicate] ncxConnectServer error\r\n"));
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
        std::string strResponse;
        strResponse="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
        strResponse+="<burnControl>";
        strResponse+="<retcode>0</retcode>";
        strResponse+="<returnDescription>";
        strResponse+="ok";
        strResponse+="</returnDescription>";
        strResponse+="</burnControl>";

        std::string strCommandType=CInterfaceProtocol::GetCommandType(pCBParam->sProtocolContent);

        if ("transmissionFeedBack" == strCommandType)
        {
            //下载反馈信息协议
            CInterfaceProtocol::FeedBack(hNCXServer,pCBParam);
        }
        else if ("getCDROMInfo" == strCommandType ||
            "getBurnServerInfo" == strCommandType)
        {
            //获取光驱信息协议，需要返回有实际意义的结果
            if (m_fCallback != NULL)
            {
                m_fCallback(hNCXServer,pCBParam,m_pUsrParam);
            }
        }
        else if("keepAlive" == strCommandType)
        {
            //无意义的内容，可能包括保活发送的数据
            ncxSendProtocolResponse(hNCXServer,*pCBParam,strResponse.c_str(),
                strResponse.length()+1);
        }
        else
        {
            //不需要返回有实际意义的结果的协议，立即返回响应结果
            ncxSendProtocolResponse(hNCXServer,*pCBParam,strResponse.c_str(),
                strResponse.length()+1);
            
            if (m_fCallback != NULL)
            {
                m_fCallback(hNCXServer,pCBParam,m_pUsrParam);
            }
        }
    }
}

int CInterfaceProtocol::DoAddTransmissionTask(FileAnywhereTaskParameter parameter)
{
    std::string     strRequest;
    std::string     strResponse;

    strRequest = "<fileAnywhere>\r\n";

    strRequest += "<commandType>";
    strRequest += "addTransmissionTask";
    strRequest += "</commandType>\r\n";

    strRequest += "<tasks>\r\n";
    strRequest += "<task>\r\n";

    strRequest += "<taskID>";
    strRequest += parameter.TaskID();
    strRequest += "</taskID>\r\n";

    strRequest += "<remoteIP>";
    strRequest += parameter.RemoteIP();
    strRequest += "</remoteIP>\r\n";

    strRequest += "<remotePort>";
    strRequest += IntToString(parameter.RemotePort());
    strRequest += "</remotePort>\r\n";

    strRequest += "<isFolder>";
    strRequest += parameter.IsFolder() ? "true" : "false";
    strRequest += "</isFolder>\r\n";

    strRequest += "<direction>";
    strRequest += parameter.Direction();
    strRequest += "</direction>\r\n";

    strRequest += "<remoteFile>";
    strRequest += parameter.RemoteFile();
    strRequest += "</remoteFile>\r\n";

    strRequest += "<localFile>";
    strRequest += parameter.LocalFile();
    strRequest += "</localFile>\r\n";

    strRequest += "<fileResume>";
    strRequest += "true";
    strRequest += "</fileResume>\r\n";

    strRequest += "<verification>";
    strRequest += "true";
    strRequest += "</verification>\r\n";

    strRequest += "<transmissionSpeed>";
    strRequest += "100000000";
    strRequest += "</transmissionSpeed>\r\n";

    strRequest += "<tryTimes>";
    strRequest += "1";
    strRequest += "</tryTimes>\r\n";

    strRequest += "<tryDelaySecond>";
    strRequest += "60";
    strRequest += "</tryDelaySecond>\r\n";

    strRequest += "<feedBackServerIP>";
    strRequest += "";
    strRequest += "</feedBackServerIP>\r\n";

    strRequest += "<feedBackServerPort>";
    strRequest += IntToString(FILE_ANY_WHERE_PORT);
    strRequest += "</feedBackServerPort>\r\n";

    strRequest += "<progressFBInterval>";
    strRequest += "1";
    strRequest += "</progressFBInterval>\r\n";

    strRequest += "</task>\r\n";
    strRequest += "</tasks>\r\n";
    strRequest += "</fileAnywhere>\r\n";

    //todo 127.0.0.1, 18479 should get from config file
    strResponse = CInterfaceProtocol::InterfaceCommunicate("127.0.0.1", 18479, strRequest,DOWNLOAD_PROTOCOL_TAG);
    if (strResponse.length() > 0)
    {
        return 0;
    }

    return -1;
}

int CInterfaceProtocol::DoDeleteTransmissionTask(std::string strTaskID)
{
    std::string     strRequest;
    std::string     strResponse;

    strRequest = "<fileAnywhere>";

    strRequest += "<commandType>";
    strRequest += "deleteTransmissionTask";
    strRequest += "</commandType>";

    strRequest += "<tasks>";

    strRequest += "<task>";
    strRequest += "<taskID>";
    strRequest += strTaskID;
    strRequest += "</taskID>";
    strRequest += "</task>";

    strRequest += "</tasks>";

    strRequest += "</fileAnywhere>";

    LOG_INFO(("[CInterfaceProtocol::DoDeleteTransmissionTask] Request : \r\n%s\r\n",strRequest.c_str()));

    //todo 127.0.0.1, 18479 should get from config file
    std::string strHostIP="127.0.0.1";
    int nFileAnywherePort=18479;
    strResponse = CInterfaceProtocol::InterfaceCommunicate(strHostIP.c_str(),nFileAnywherePort,strRequest,"fileAnywhere");
    if (strResponse.length() > 0)
    {
        return 0;
    }

    return -1;
}

void CInterfaceProtocol::AnalyzeProtocolSendRTBurnInfo(const std::string &strProtocolContent,CRTBurnInfo &RTBurnInfo)
{
    RTBurnInfo.SetJobID(GetTagValue(strProtocolContent,"jobID"));
    RTBurnInfo.SetJobType(GetTagValue(strProtocolContent,"jobType"));

    RTBurnInfo.GetBurnServerInfo().SetIP(GetTagValue(strProtocolContent,"burnServerIP"));
    RTBurnInfo.GetBurnServerInfo().SetPort(GetTagValue(strProtocolContent,"burnServerPort"));

    //解析光驱信息
    int nRelativePos=0;

    //解析光驱信息
    std::string strCDROMVector=GetTagValue(strProtocolContent,"CDROMVector");
    nRelativePos=0;
    std::string strCDROMStruct=GetTagValueWithStartPos(strCDROMVector,"CDROMStruct",nRelativePos);
    while (strCDROMStruct != "")
    {
        CCDROMDriverInfo CDROMInfo;
        CDROMInfo.SetID(GetTagValue(strCDROMStruct,"CDROMID"));
        CDROMInfo.SetDescription(GetTagValue(strCDROMStruct,"CDROMDescription"));
        CDROMInfo.SetIsSelected((std::string("1") == GetTagValue(strCDROMStruct,"isSelected")) ? "1" : "2");

        RTBurnInfo.GetBurnServerInfo().GetVecCDROMDriverInfo().push_back(CDROMInfo);
        strCDROMStruct=GetTagValueWithStartPos(strCDROMVector,"CDROMStruct",nRelativePos);
    }

    RTBurnInfo.GetCommonBurnParam().SetBurnType(GetTagValue(strProtocolContent,"burnType"));

    RTBurnInfo.GetCommonBurnParam().SetBurnPassword(GetTagValue(strProtocolContent,"burnPassword"));
    RTBurnInfo.GetCommonBurnParam().SetContentPassword(GetTagValue(strProtocolContent,"contentPassword"));

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

        RTBurnInfo.GetVecVideoLocationInfo().push_back(videoLocationInfo);
        strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,"videoLocationStruct",nRelativePos);
    }

    RTBurnInfo.SetUpServerIP(GetTagValue(strProtocolContent,"upServerIP"));
    RTBurnInfo.SetUpServerPort(GetTagValue(strProtocolContent,"upServerPort"));
}

void CInterfaceProtocol::AnalyzeProtocolSendBurnInfo(const std::string &strProtocolContent,CNormalBurnInfo &normalBurnInfo)
{
    normalBurnInfo=CNormalBurnInfo();

    //指向默认的刻录服务器
    CBurnServerInfo *pDefaultServer=NULL;

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
                videoFileInfo.SetFileName(GetTagValue(strVideoFileStruct,"newVideoFileName"));

                videoLocationInfo.GetVecFileInfo().push_back(videoFileInfo);
                strVideoFileStruct=GetTagValueWithStartPos(strVideoFileVector,"videoFileStruct",
                    nVideoFileVectorRelativePos);
            }

            normalBurnJobInfo.GetVecLocationInfo().push_back(videoLocationInfo);
            strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,"videoLocationStruct",
                nVideoLocationVectorRelativePos);
        }

        normalBurnJobInfo.SetPlayListContent(GetTagValue(strJobStruct,"playListInfo"));
        normalBurnJobInfo.SetViewCount(GetTagValue(strJobStruct,"viewCount"));

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
        if ("" == normalBurnJobInfo.GetBurnServerInfo().GetIP() ||
            "0" == normalBurnJobInfo.GetBurnServerInfo().GetPort())
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
                    /*assert(false);*/
                }
            }
        }

        if ("" == normalBurnJobInfo.GetCommonBurnParam().GetBurnPassword())
        {
            normalBurnJobInfo.GetCommonBurnParam().SetBurnPassword(normalBurnInfo.GetCommonBurnParam().GetBurnPassword());
        }
        if ("" == normalBurnJobInfo.GetCommonBurnParam().GetContentPassword())
        {
            normalBurnJobInfo.GetCommonBurnParam().SetContentPassword(normalBurnInfo.GetCommonBurnParam().GetContentPassword());
        }
        if ("" == normalBurnJobInfo.GetCommonBurnParam().GetBurnType())
        {
            normalBurnJobInfo.GetCommonBurnParam().SetBurnType(normalBurnInfo.GetCommonBurnParam().GetBurnType());
        }

        normalBurnInfo.GetVecNormalBurnJobInfo().push_back(normalBurnJobInfo);
        strJobStruct=GetTagValueWithStartPos(strJobVector,"jobStruct",nRelativePos);
    }//while
}

void CInterfaceProtocol::AnalyzeProtocolStartRTBurnInfo(const std::string &strProtocolContent,CRTBurnInfo &RTBurnInfo)
{
    RTBurnInfo.SetJobID(GetTagValue(strProtocolContent,"jobID"));

    std::string strPlayerInfoStruct=GetTagValue(strProtocolContent,"playerInfoStruct");
    if (strPlayerInfoStruct != "")
    {
        RTBurnInfo.GetPlayerInfo().SetVersion(GetTagValue(strPlayerInfoStruct,"playerVersion"));
        RTBurnInfo.GetPlayerInfo().SetDownloadURL(GetTagValue(strPlayerInfoStruct,"playerDownloadURL"));
        RTBurnInfo.GetPlayerInfo().SetRemoteIP(GetTagValue(strPlayerInfoStruct,"remoteIP"));
        RTBurnInfo.GetPlayerInfo().SetRemotePort(GetTagValue(strPlayerInfoStruct,"remotePort"));
    }

    RTBurnInfo.SetPlayListContent(GetTagValue(strProtocolContent,"playListInfo"));
    RTBurnInfo.SetViewCount(GetTagValue(strProtocolContent,"viewCount"));

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
        videoLocationInfo.SetURL(GetTagValue(strVideoLocationStruct,"locationURL"));
        RTBurnInfo.GetVecVideoLocationInfo().push_back(videoLocationInfo);

        strVideoLocationStruct=GetTagValueWithStartPos(strVideoLocationVector,"videoLocationStruct",nRelativePos);
    }

    RTBurnInfo.SetNewFileName(GetTagValue(strProtocolContent,"newFileName"));


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
    strProtocolContent+=RTBurnInfo.GetJobType();
    strProtocolContent+="</jobFlag>\r\n";

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

            strProtocolContent+="<isSelected>";
            strProtocolContent+=vecCDROM[i].GetIsSelected();
            strProtocolContent+="</isSelected>\r\n";

            strProtocolContent+="</CDROMStruct>\r\n";
        }
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
    for (size_t j=0;j<vecVideoLocationInfo.size();++j)
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
        strProtocolContent+="<videoLocationStruct>\r\n";
    }

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

    //刻录方式，这个字段很重要，因为它决定了每个光驱该刻哪些内容
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

            const std::vector<CLocationInfo> &vecVideoLocationInfo=
                vecCDROM[i].GetVecLocationInfo();
            for (size_t j=0;j<vecVideoLocationInfo.size();++j)
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
                strProtocolContent+="<videoLocationStruct>\r\n";
            }

            strProtocolContent+="</CDROMStruct>\r\n";
        }
    }
    strProtocolContent+="</CDROMVector>\r\n";

    strProtocolContent+="<newFileName>";
    strProtocolContent+=RTBurnInfo.GetNewFileName();
    strProtocolContent+="</newFileName>\r\n";

    strProtocolContent+="<upServerIP>";
    strProtocolContent+=RTBurnInfo.GetUpServerIP();
    strProtocolContent+="</upServerIP>\r\n";
    strProtocolContent+="<upServerPort>";
    strProtocolContent+=RTBurnInfo.GetUpServerPort();
    strProtocolContent+="</upServerPort>\r\n";

    strProtocolContent+="</burnControl>";

    return strProtocolContent;
}

std::string CInterfaceProtocol::GenerateProtocolStartBurnToBurnServer(const CNormalBurnJobInfoEx &normalBurnJobInfoEx)
{
    std::string strProtocolContent="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    strProtocolContent+="\r\n";

    strProtocolContent+="<burnControl>\r\n";

    strProtocolContent+="<commandType>startBurn</commandType>\r\n";

    strProtocolContent+="<jobID>";
    strProtocolContent+=normalBurnJobInfoEx.GetJobID();
    strProtocolContent+="</jobID>\r\n";

    //播放器信息
    strProtocolContent+="<playerInfoStruct>\r\n";
    strProtocolContent+="<playerVersion>";
    strProtocolContent+=normalBurnJobInfoEx.GetPlayerInfo().GetVersion();
    strProtocolContent+="</playerVersion>\r\n";
    strProtocolContent+="<playerDownloadURL>";
    strProtocolContent+=normalBurnJobInfoEx.GetPlayerInfo().GetDownloadURL();
    strProtocolContent+="</playerDownloadURL>\r\n";
    strProtocolContent+="<remoteIP>";
    strProtocolContent+=normalBurnJobInfoEx.GetPlayerInfo().GetRemoteIP();
    strProtocolContent+="</remoteIP>\r\n";
    strProtocolContent+="<remotePort>";
    strProtocolContent+=normalBurnJobInfoEx.GetPlayerInfo().GetRemotePort();
    strProtocolContent+="</remotePort>\r\n";
    strProtocolContent+="</playerInfoStruct>\r\n";

    //播放列表文件信息
    strProtocolContent+="<playlistInfoStruct>\r\n";
    strProtocolContent+="<playlistDownloadURL>";
    strProtocolContent+=normalBurnJobInfoEx.GetPlaylistFileInfo().GetDownloadURL();
    strProtocolContent+="</playlistDownloadURL>\r\n";
    strProtocolContent+="<remoteIP>";
    strProtocolContent+=normalBurnJobInfoEx.GetPlaylistFileInfo().GetRemoteIP();
    strProtocolContent+="</remoteIP>\r\n";
    strProtocolContent+="<remotePort>";
    strProtocolContent+=normalBurnJobInfoEx.GetPlaylistFileInfo().GetRemotePort();
    strProtocolContent+="</remotePort>\r\n";
    strProtocolContent+="</playlistInfoStruct>\r\n";

    //笔录文件信息
    strProtocolContent+="<noteFileRemoteIP>";
    strProtocolContent+=normalBurnJobInfoEx.GetNoteFileRemoteIP();
    strProtocolContent+="</noteFileRemoteIP>\r\n";
    strProtocolContent+="<noteFileRemotePort>";
    strProtocolContent+=normalBurnJobInfoEx.GetNoteFileRemotePort();
    strProtocolContent+="</noteFileRemotePort>\r\n";
    strProtocolContent+="<noteFileDownloadURL>";
    strProtocolContent+=normalBurnJobInfoEx.GetNoteRelativePath();
    strProtocolContent+="</noteFileDownloadURL>\r\n";
    strProtocolContent+="<noteFileName>";
    strProtocolContent+=normalBurnJobInfoEx.GetNoteFileName();
    strProtocolContent+="</noteFileName>\r\n";

    //autorun.inf文件信息
    strProtocolContent+="<autorunFileRemoteIP>";
    strProtocolContent+=normalBurnJobInfoEx.GetAutorunFileRemoteIP();
    strProtocolContent+="</autorunFileRemoteIP>\r\n";
    strProtocolContent+="<autorunFileRemotePort>";
    strProtocolContent+=normalBurnJobInfoEx.GetAutorunFileRemotePort();
    strProtocolContent+="</autorunFileRemotePort>\r\n";
    strProtocolContent+="<autorunFileDownloadURL>";
    strProtocolContent+=normalBurnJobInfoEx.GetAutorunFilePath();
    strProtocolContent+="</autorunFileDownloadURL>\r\n";

    strProtocolContent+="<burnType>";
    strProtocolContent+=normalBurnJobInfoEx.GetCommonBurnParam().GetBurnType();
    strProtocolContent+="</burnType>\r\n";

    strProtocolContent+="<burnPassword>";
    strProtocolContent+=normalBurnJobInfoEx.GetCommonBurnParam().GetBurnPassword();
    strProtocolContent+="</burnPassword>\r\n";

    strProtocolContent+="<contentPassword>";
    strProtocolContent+=normalBurnJobInfoEx.GetCommonBurnParam().GetContentPassword();
    strProtocolContent+="</contentPassword>\r\n";

    strProtocolContent+="<CDName>";
    strProtocolContent+=normalBurnJobInfoEx.GetCommonBurnParam().GetCDName();
    strProtocolContent+="</CDName>\r\n";

    //光驱列表信息
    strProtocolContent+="<CDROMVector>\r\n";
    const std::vector<CCDROMDriverInfo> &vecCDROMDriver=normalBurnJobInfoEx.GetBurnServerInfo().GetVecCDROMDriverInfo();
    for (size_t i=0;i<vecCDROMDriver.size();++i)
    {
        if ("1" == vecCDROMDriver[i].GetIsSelected())
        {
            strProtocolContent+="<CDROMStruct>\r\n";

            strProtocolContent+="<CDROMID>";
            strProtocolContent+=vecCDROMDriver[i].GetID();
            strProtocolContent+="</CDROMID>\r\n";

            strProtocolContent+="<CDROMDescription>";
            strProtocolContent+=vecCDROMDriver[i].GetDescription();
            strProtocolContent+="</CDROMDescription>\r\n";

            const std::vector<CLocationInfo> &vecVideoLocationInfo=
                vecCDROMDriver[i].GetVecLocationInfo();
            for (size_t j=0;j<vecVideoLocationInfo.size();++j)
            {
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
                const std::vector<CSingleFileInfo> &vecVideoFileInfo=
                    vecCDROMDriver[i].GetVecLocationInfo().at(j).GetVecFileInfo();
                for (size_t i=0;i<vecVideoFileInfo.size();++i)
                {
                    strProtocolContent+="<videoFileStruct>\r\n";
                    strProtocolContent+="<videoFileName>";
                    strProtocolContent+=vecVideoFileInfo[i].GetFileName();
                    strProtocolContent+="</videoFileName>\r\n";
                    strProtocolContent+="<newVideoFileName>";
                    strProtocolContent+=vecVideoFileInfo[i].GetNewFileName();
                    strProtocolContent+="</newVideoFileName>\r\n";
                    strProtocolContent+="<videoFileStruct>\r\n";
                }
                strProtocolContent+="</videoFileVector>\r\n";

                strProtocolContent+="<videoLocationStruct>\r\n";
            }

            strProtocolContent+="</CDROMStruct>\r\n";
        }
    }
    strProtocolContent+="</CDROMVector>\r\n";

    strProtocolContent+="<printWordVector>";
    std::map<std::string,std::string>::const_iterator iter;
    for (iter=normalBurnJobInfoEx.GetMapLabelFieldText().begin();
        iter!=normalBurnJobInfoEx.GetMapLabelFieldText().end();++iter)
    {
        strProtocolContent+="</printWordStruct>\r\n";

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
    strProtocolContent+=normalBurnJobInfoEx.GetUpServerIP();
    strProtocolContent+="</upServerIP>\r\n";
    strProtocolContent+="<upServerPort>";
    strProtocolContent+=normalBurnJobInfoEx.GetUpServerPort();
    strProtocolContent+="</upServerPort>\r\n";

    strProtocolContent+="</burnControl>";

    return strProtocolContent;
}

std::string CInterfaceProtocol::GenerateProtocolSendBurnCtrolCMDToBurnServer(std::string strJobID,std::string strCMD)
{
    std::string strProtocolContent="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    strProtocolContent+="\r\n";

    strProtocolContent+="<burnControl>\r\n";

    strProtocolContent+="<commandType>sendBurnCtrolCMD</commandType>\r\n";

    strProtocolContent+="<jobID>";
    strProtocolContent+=strJobID;
    strProtocolContent+="</jobID>\r\n";
    strProtocolContent+="<ctrlCMD>";
    strProtocolContent+=strCMD;
    strProtocolContent+="</ctrlCMD>\r\n";

    strProtocolContent+="</burnControl>";

    return strProtocolContent;
}

void CInterfaceProtocol::AnalyzeProtocolStartRTBurnToBurnServer(const std::string &strProtocolContent,CRTBurnInfoEx &RTBurnInfoEx)
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
        RTBurnInfoEx.GetPlaylistFileInfo().SetDownloadURL(GetTagValue(strPlaylistInfoStruct,"playlistDownloadURL"));
        RTBurnInfoEx.GetPlaylistFileInfo().SetRemoteIP(GetTagValue(strPlaylistInfoStruct,"remoteIP"));
        RTBurnInfoEx.GetPlaylistFileInfo().SetRemotePort(GetTagValue(strPlaylistInfoStruct,"remotePort"));
    }

    RTBurnInfoEx.SetNoteFileDownloadURL(GetTagValue(strProtocolContent,"noteFileDownloadURL"));
    RTBurnInfoEx.SetNoteFileRemoteIP(GetTagValue(strProtocolContent,"noteFileRemoteIP"));
    RTBurnInfoEx.SetNoteFileRemotePort(GetTagValue(strProtocolContent,"noteFileRemotePort"));

    RTBurnInfoEx.SetAutorunFileRemoteIP(GetTagValue(strProtocolContent,"autorunFileRemoteIP"));
    RTBurnInfoEx.SetAutorunFileRemotePort(GetTagValue(strProtocolContent,"autorunFileRemotePort"));
    RTBurnInfoEx.SetAutorunFilePath(GetTagValue(strProtocolContent,"autorunFileDownloadURL"));

    //刻录方式，这个字段很重要，因为它决定了每个光驱该刻哪些内容
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

void CInterfaceProtocol::AnalyzeProtocolStartBurnToBurnServer(const std::string &strProtocolContent,CNormalBurnJobInfoEx &normalBurnJobInfoEx)
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
            videoLocationInfo.SetStreamType(GetTagValue(strVideoLocationStruct,"streamType"));

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

    std::string strPrintWordVector=GetTagValue(strProtocolContent,"printWordVector");

    nRelativePos=0;
    std::string strPrintWordStruct=GetTagValueWithStartPos(strPrintWordVector,"printWordStruct",nRelativePos);
    while (strPrintWordStruct != "")
    {
        std::string strItemID=GetTagValue(strPrintWordStruct,"itemID");
        std::string strItemValue=GetTagValue(strPrintWordStruct,"itemValue");

        normalBurnJobInfoEx.GetMapLabelFieldText().insert(std::make_pair(strItemID,strItemValue));

        strPrintWordStruct=GetTagValueWithStartPos(strPrintWordVector,"printWordStruct",nRelativePos);
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

    strProtocolContent+="<jobState>";
    strProtocolContent+=strState;
    strProtocolContent+="</jobState>\r\n";

    strProtocolContent+="<jobStateDescription>";
    strProtocolContent+=strStateDescription;
    strProtocolContent+="</jobStateDescription>\r\n";

    if (!vecCDROMDriver.empty())
    {
        //光驱列表信息，在事后刻录的普通类型刻录中，每个光驱对象包含了详细的刻录信息
        strProtocolContent+="<CDROMVector>\r\n";
        for (size_t i=0;i<vecCDROMDriver.size();++i)
        {
            strProtocolContent+="<CDROMStruct>\r\n";

            strProtocolContent+="<CDROMID>";
            strProtocolContent+=vecCDROMDriver[i].GetID();
            strProtocolContent+="</CDROMID>\r\n";

            strProtocolContent+="<CDROMDescription>";
            strProtocolContent+=vecCDROMDriver[i].GetDescription();
            strProtocolContent+="</CDROMDescription>\r\n";

            strProtocolContent+="<stateFlag>";
            strProtocolContent+=vecCDROMDriver[i].GetStateFlag();
            strProtocolContent+="</stateFlag>\r\n";

            strProtocolContent+="<stateDescription>";
            strProtocolContent+=vecCDROMDriver[i].GetStateDescription();
            strProtocolContent+="</stateDescription>\r\n";

            strProtocolContent+="<videoLocationVector>\r\n";
            const std::vector<CLocationInfo> &vecLocationInfo=vecCDROMDriver[i].GetVecLocationInfo();
            for (size_t j=0;j<vecLocationInfo.size();++j)
            {
                strProtocolContent+="<videoLocationStruct>\r\n";

                strProtocolContent+="<locationID>";
                strProtocolContent+=vecLocationInfo[j].GetID();
                strProtocolContent+="</locationID>\r\n";
                strProtocolContent+="<locationDescription>";
                strProtocolContent+=vecLocationInfo[j].GetDescription();
                strProtocolContent+="</locationDescription>\r\n";

                strProtocolContent+="<videoFileVector>\r\n";
                const std::vector<CSingleFileInfo> &vecVideoFileInfo=
                    vecCDROMDriver[i].GetVecLocationInfo().at(j).GetVecFileInfo();
                for (size_t i=0;i<vecVideoFileInfo.size();++i)
                {
                    strProtocolContent+="<videoFileStruct>\r\n";

                    strProtocolContent+="<fileName>";
                    strProtocolContent+=vecVideoFileInfo[i].GetFileName();
                    strProtocolContent+="</fileName>\r\n";

                    strProtocolContent+="<burnState>";
                    strProtocolContent+=vecVideoFileInfo[i].GetBurnState();
                    strProtocolContent+="</burnState>\r\n";

                    strProtocolContent+="<size>";
                    strProtocolContent+=vecVideoFileInfo[i].GetSize();
                    strProtocolContent+="</size>\r\n";

                    strProtocolContent+="<percent>";
                    strProtocolContent+=vecVideoFileInfo[i].GetDownloadPercent();
                    strProtocolContent+="</percent>\r\n";

                    strProtocolContent+="</videoFileStruct>\r\n";
                }
                strProtocolContent+="</videoFileVector>\r\n";

                strProtocolContent+="</videoLocationStruct>\r\n";
            }
            strProtocolContent+="</videoLocationVector>\r\n";

            strProtocolContent+="</CDROMStruct>\r\n";
        }
        strProtocolContent+="</CDROMVector>\r\n";
    }

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
                videoFileInfo.SetDownloadPercent(GetTagValue(strVideoFileStruct,"percent"));

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

int CInterfaceProtocol::FeedBack(NCXSERVERHANDLE hNCXServer, NCXServerCBParam *pCBParam)
{
    int             nReturn = -1;
    std::string     strContent;
    std::string     strResponse;
    std::string		strTask;
    FileAnywhereObserverParameter   parameter;
    int				nEventType;
    int             nReason;
    bool            bDoNotify = false;

    //LOG_INFO(("[InterfaceProtocolSpecific::FeedBack] receive protocol feedBack\r\n"));
    if (pCBParam != NULL && pCBParam->sProtocolContent != NULL)
    {
        //LOG_INFO(("[InterfaceProtocolSpecific::FeedBack] remoteIP %s\r\n", pCBParam->szRemoteIP));
        strContent = std::string(pCBParam->sProtocolContent);

        strTask = GetTagValue(strContent, "task");
        parameter.TaskID(GetTagValue(strTask, "taskID"));
        parameter.FileTotalSize(StringToInt64(GetTagValue(strTask,"fileTotalSize")));
        nEventType = StringToInt(GetTagValue(strTask, "eventType"));
        if (nEventType == 1)
        {
            parameter.EventType(1);
            parameter.Percentage(StringToInt(GetTagValue(strTask, "percentage")));
            bDoNotify = true;
        }
        else if (nEventType == 2)
        {
            nReason = StringToInt(GetTagValue(strTask, "reason"));
            if (nReason == 1)
            {
                parameter.EventType(2);
                parameter.Percentage(100);
                bDoNotify = true;
            }
            else if (nReason == 2)
            {
                parameter.EventType(5);
                bDoNotify = true;
            }
            else
            {
                parameter.EventType(3);
                bDoNotify = true;
            }
        }
        else if (nEventType == 3)
        {
            //retry
        }
        else
        {
            LOG_ERROR(("[InterfaceProtocolSpecific::FeedBack] eventType is not defined\r\n"));
        }

        strResponse += "<fileAnywhere>";
        strResponse += "</fileAnywhere>";

        if (ncxSendProtocolResponse(hNCXServer, *pCBParam, strResponse.c_str(), strResponse.length()+1) != 0)
        {
            LOG_ERROR(("[InterfaceProtocolSpecific::FeedBack] ncxSendProtocolResponse failed. remote ip %s\r\n", pCBParam->szRemoteIP));
        }
        if (bDoNotify)
        {
            FileAnywhereManager::GetInstance()->OnFileAnywhereFeedBack(parameter);
        }
    }

    return nReturn;
}

#define DEV_FLAG_BUSY   "1"
#define DEV_FLAG_IDLE   "0"
std::string CInterfaceProtocol::GenerateProtocolCDROMDriverInfo(const std::vector<CCDROMDriverInfo> &vecCDROMDriver)
{
    std::string strProtocolContent;

    strProtocolContent+="<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n";
    strProtocolContent+="<burnControl>\r\n";
    strProtocolContent+="<CDROMVector>\r\n";

    std::string strJobID;
    std::string strStateFlag="-1";

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

        strJobID=iter->GetJobID();

        strProtocolContent+="<jobID>";
        strProtocolContent+=strJobID;
        strProtocolContent+="</jobID>\r\n";


        strProtocolContent+="<hasCD>";
        strProtocolContent+=iter->GetHasCD();
        strProtocolContent+="</hasCD>\r\n";

        strProtocolContent+="<CDLeftCapacity>";
        strProtocolContent+=iter->GetCDLeftCapacity();
        strProtocolContent+="</CDLeftCapacity>\r\n";

        strProtocolContent+="<CDTotalCapacity>";
        strProtocolContent+=iter->GetCDTotalCapacity();
        strProtocolContent+="</CDTotalCapacity>\r\n";

        strProtocolContent+="<stateFlag>";
        strProtocolContent+=( (strJobID != "") ? DEV_FLAG_BUSY : DEV_FLAG_IDLE);
        strProtocolContent+="</stateFlag>\r\n";

        strProtocolContent+="</CDROMStruct>\r\n";
    }

    strProtocolContent+="</CDROMVector>\r\n";

    strProtocolContent+="</burnControl>\r\n";

    return strProtocolContent;

#if 0
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

        

        strProtocolContent+="<jobID>";
        strProtocolContent+=iter->GetJobID();
        strProtocolContent+="</jobID>\r\n";

        //刻录方式
        std::string strBurnType;
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
            std::vector<CRTBurnInfoEx>::const_iterator iterRTBurnInfo;
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

    LOG_INFO(("[CInterfaceProtocol::GenerateProtocolCDROMDriverInfo] Protocol content : \r\n%s\r\n",
        strProtocolContent.c_str()));

    return strProtocolContent;
#endif
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
        CDROMDriverInfo.SetStateFlag(GetTagValue(strCDROMDriverStruct,"stateFlag"));
        CDROMDriverInfo.SetStateDescription(
            GetTagValue(strCDROMDriverStruct,"stateDescription"));

        vecCDROMInfo.push_back(CDROMDriverInfo);

        strCDROMDriverStruct=CInterfaceProtocol::GetTagValueWithStartPos(
            strCDROMDriverVec,"CDROMStruct",nRelativePosCDROMDriver);
    }
}

void CInterfaceProtocol::AnalyzeProtocolBurnServerInfo(const std::string &strProtocolContent,std::vector<CBurnServerInfo> &vecBurnServerInfo)
{
    vecBurnServerInfo.clear();

    std::string strBurnServerVec=CInterfaceProtocol::GetTagValue(strProtocolContent,
        "burnServerVector");
    int nRelativePosBurnServer=0;
    std::string strBurnServerStruct=CInterfaceProtocol::GetTagValueWithStartPos(
        strBurnServerVec,"burnServerStruct",nRelativePosBurnServer);
    while (strBurnServerStruct != "")
    {
        CBurnServerInfo burnServerInfo;
        std::string strBurnServerType=CInterfaceProtocol::GetTagValue(strBurnServerStruct,"burnServerType");
        burnServerInfo.SetBurnServerType( (strBurnServerType!="") ? strBurnServerType : "1" );
        burnServerInfo.SetBurnServerTypeDescription(
            CInterfaceProtocol::GetTagValue(strBurnServerStruct,"burnServerTypeDescription"));

        vecBurnServerInfo.push_back(burnServerInfo);

        strBurnServerStruct=CInterfaceProtocol::GetTagValueWithStartPos(
            strBurnServerVec,"burnServerStruct",nRelativePosBurnServer);
    }
}

