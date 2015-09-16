#include "stdafx.h"

#include "BurnInfo.h"

//////////////////////////////////////////////////////////////////////////
CSingleFileInfo::CSingleFileInfo():
m_strFileName(""),
m_strNewFileName(""),
m_strBurnState(FILE_BURN_STATE_WAIT),
m_strSize("0"),
m_strPercent("0")
{
}

CSingleFileInfo::~CSingleFileInfo()
{
}

std::string CSingleFileInfo::GetFileName() const 
{ 
    return m_strFileName; 
}

void CSingleFileInfo::SetFileName(std::string val) 
{ 
    m_strFileName = val;
}

std::string CSingleFileInfo::GetNewFileName() const 
{ 
    return m_strNewFileName;
}

void CSingleFileInfo::SetNewFileName(std::string val)
{ 
    m_strNewFileName = val; 
}

std::string CSingleFileInfo::GetBurnState() const 
{
    return m_strBurnState; 
}

void CSingleFileInfo::SetBurnState(std::string val)
{ 
    m_strBurnState = val;
}

std::string CSingleFileInfo::GetSize() const 
{ 
    return m_strSize;
}

void CSingleFileInfo::SetSize(std::string val)
{ 
   
    m_strSize = val; 
}
std::string CSingleFileInfo::GetPercent() const 
{
    return m_strPercent;
}

void CSingleFileInfo::SetPercent(std::string val) 
{
    m_strPercent = val;
}

//////////////////////////////////////////////////////////////////////////
CLocationInfo::CLocationInfo(void):
m_strID(""),
m_strDescription(""),
m_strBurnOrNot(BURN_NO),
m_strStreamType("chnsys"),
m_strURL("")
{
}

CLocationInfo::~CLocationInfo(void)
{
}

CLocationInfo::CLocationInfo(const CLocationInfo &location):
m_strID(location.GetID()),
m_strDescription(location.GetDescription()),
m_strBurnOrNot(location.GetBurnOrNot()),
m_strStreamType(location.GetStreamType()),
m_strURL(location.GetURL()),
m_vecFileInfo(location.GetVecFileInfo())
{
}

CLocationInfo & CLocationInfo::operator=(const CLocationInfo &videoLocationInfo)
{
    m_strID=videoLocationInfo.GetID();
    m_strDescription=videoLocationInfo.GetDescription();
    m_strBurnOrNot=videoLocationInfo.GetBurnOrNot();
    m_strStreamType=videoLocationInfo.GetStreamType();
    m_strURL=videoLocationInfo.GetURL();
    m_vecFileInfo=videoLocationInfo.GetVecFileInfo();

    return *this;
}

std::string CLocationInfo::GetID() const
{
    return m_strID;
}

void CLocationInfo::SetID(std::string val) 
{ 
    m_strID = val;
}

std::string CLocationInfo::GetDescription() const 
{
    return m_strDescription;
}

void CLocationInfo::SetDescription(std::string val) 
{
    m_strDescription = val;
}

std::string CLocationInfo::GetBurnOrNot() const
{ 
    return m_strBurnOrNot;
}

void CLocationInfo::SetBurnOrNot(std::string val) 
{
    if ((val != BURN_YES) &&
        (val != BURN_NO))
    {
        val=BURN_NO;
    }

    m_strBurnOrNot = val;
}

std::string CLocationInfo::GetStreamType() const 
{
    return m_strStreamType;
}

void CLocationInfo::SetStreamType(std::string val)
{ 
    m_strStreamType = val;
}

std::string CLocationInfo::GetURL() const 
{ 
    return m_strURL;
}

void CLocationInfo::SetURL(std::string val)
{ 
    m_strURL = val;
}

std::vector<CSingleFileInfo> & CLocationInfo::GetVecFileInfo() 
{ 
    return m_vecFileInfo;
}

const std::vector<CSingleFileInfo> & CLocationInfo::GetVecFileInfo() const 
{ 
    return m_vecFileInfo;
}

void CLocationInfo::SetVecFileInfo(std::vector<CSingleFileInfo> val) 
{ 
    m_vecFileInfo = val; 
}

//////////////////////////////////////////////////////////////////////////
CCDROMDriverInfo::CCDROMDriverInfo(void):
m_strID(""),
m_strDescription(""),
m_strIsSelected(DEV_SELECTED_NO),
m_strStateFlag(DEV_STATE_UNAVAILABLE),
m_strStateDescription(""),
m_strJobID(""),
m_strHasCD(DEV_HAS_DISC_NO),
m_strCDLeftCapacity("0"),
m_strCDTotalCapacity("0")
{
}

CCDROMDriverInfo::~CCDROMDriverInfo(void)
{
}

CCDROMDriverInfo::CCDROMDriverInfo(const CCDROMDriverInfo &CDROMDriverInfo):
m_strID(CDROMDriverInfo.GetID()),
m_strDescription(CDROMDriverInfo.GetDescription()),
m_strIsSelected(CDROMDriverInfo.GetIsSelected()),
m_vecVideoLocationInfo(CDROMDriverInfo.GetVecLocationInfo()),
m_strStateFlag(CDROMDriverInfo.GetStateFlag()),
m_strStateDescription(CDROMDriverInfo.GetStateDescription()),
m_strJobID(CDROMDriverInfo.GetJobID()),
m_strHasCD(CDROMDriverInfo.GetHasCD()),
m_strCDLeftCapacity(CDROMDriverInfo.GetCDLeftCapacity()),
m_strCDTotalCapacity(CDROMDriverInfo.GetCDTotalCapacity())
{

}

CCDROMDriverInfo & CCDROMDriverInfo::operator=(const CCDROMDriverInfo &CDROMDriverInfo)
{
    m_strID=CDROMDriverInfo.GetID();
    m_strDescription=CDROMDriverInfo.GetDescription();
    m_strIsSelected=CDROMDriverInfo.GetIsSelected();
    m_vecVideoLocationInfo=CDROMDriverInfo.GetVecLocationInfo();
    m_strStateFlag=CDROMDriverInfo.GetStateFlag();
    m_strStateDescription=CDROMDriverInfo.GetStateDescription();
    m_strJobID=CDROMDriverInfo.GetJobID();
    m_strHasCD=CDROMDriverInfo.GetHasCD();
    m_strCDLeftCapacity=CDROMDriverInfo.GetCDLeftCapacity();
    m_strCDTotalCapacity=CDROMDriverInfo.GetCDTotalCapacity();

    return *this;
}

std::string CCDROMDriverInfo::GetID() const 
{
    return m_strID;
}

void CCDROMDriverInfo::SetID(std::string val)
{ 
    m_strID = val;
}

std::string CCDROMDriverInfo::GetDescription() const 
{
    return m_strDescription;
}

void CCDROMDriverInfo::SetDescription(std::string val) 
{
    m_strDescription = val;
}

std::string CCDROMDriverInfo::GetIsSelected() const
{
    return m_strIsSelected;
}

void CCDROMDriverInfo::SetIsSelected(std::string val)
{ 
    if (DEV_SELECTED_YES == val)
    {
        m_strIsSelected = val;
    }
    else
    {
        m_strIsSelected = DEV_SELECTED_NO;
    }
}

std::vector<CLocationInfo>  & CCDROMDriverInfo::GetVecLocationInfo()
{ 
    return m_vecVideoLocationInfo;
}

const std::vector<CLocationInfo> & CCDROMDriverInfo::GetVecLocationInfo() const 
{ 
    return m_vecVideoLocationInfo; 
}

void CCDROMDriverInfo::SetVecLocationInfo(std::vector<CLocationInfo> val)
{ 
    m_vecVideoLocationInfo = val;
}

std::string CCDROMDriverInfo::GetStateFlag() const 
{ 
    return m_strStateFlag;
}

void CCDROMDriverInfo::SetStateFlag(std::string val)
{ 
    m_strStateFlag = val;
}

std::string CCDROMDriverInfo::GetStateDescription() const 
{ 
    return m_strStateDescription;
}

void CCDROMDriverInfo::SetStateDescription(std::string val) 
{ 
    m_strStateDescription = val;
}

std::string CCDROMDriverInfo::GetJobID() const 
{
    return m_strJobID;
}

void CCDROMDriverInfo::SetJobID(std::string val) 
{ 
    m_strJobID = val;
}

std::string CCDROMDriverInfo::GetHasCD() const 
{
    return m_strHasCD;
}

void CCDROMDriverInfo::SetHasCD(std::string val)
{
    m_strHasCD = val; 
}

std::string CCDROMDriverInfo::GetCDLeftCapacity() const 
{
    return m_strCDLeftCapacity; 
}

void CCDROMDriverInfo::SetCDLeftCapacity(std::string val) 
{ 
    m_strCDLeftCapacity = val;
}

std::string CCDROMDriverInfo::GetCDTotalCapacity() const
{ 
    return m_strCDTotalCapacity;
}

void CCDROMDriverInfo::SetCDTotalCapacity(std::string val) 
{ 
    m_strCDTotalCapacity = val;
}

//////////////////////////////////////////////////////////////////////////
CPlaylistFileInfo::CPlaylistFileInfo():
m_strDownloadURL(""),
m_strRemoteIP(""),
m_strRemotePort("")
{
}

CPlaylistFileInfo::~CPlaylistFileInfo()
{
}

std::string CPlaylistFileInfo::GetDownloadURL() const 
{ 
    return m_strDownloadURL; 
}

void CPlaylistFileInfo::SetDownloadURL(std::string val) 
{ 
    m_strDownloadURL = val;
}

std::string CPlaylistFileInfo::GetRemoteIP() const 
{
    return m_strRemoteIP;
}

void CPlaylistFileInfo::SetRemoteIP(std::string val) 
{ 
    m_strRemoteIP = val; 
}

std::string CPlaylistFileInfo::GetRemotePort() const 
{ 
    return m_strRemotePort;
}

void CPlaylistFileInfo::SetRemotePort(std::string val) 
{ 
    m_strRemotePort = val;
}

//////////////////////////////////////////////////////////////////////////
CPlayerInfo::CPlayerInfo(void):
m_strVersion(""),
m_strDownloadURL(""),
m_strRemoteIP(""),
m_strRemotePort("")
{
}

CPlayerInfo::~CPlayerInfo(void)
{
}

std::string CPlayerInfo::GetVersion() const
{ 
    return m_strVersion; 
}

void CPlayerInfo::SetVersion(std::string val)
{ 
    m_strVersion = val;
}

std::string CPlayerInfo::GetDownloadURL() const 
{ 
    return m_strDownloadURL; 
}

void CPlayerInfo::SetDownloadURL(std::string val) 
{ 
    m_strDownloadURL = val;
}

std::string CPlayerInfo::GetRemoteIP() const 
{
    return m_strRemoteIP;
}

void CPlayerInfo::SetRemoteIP(std::string val) 
{ 
    m_strRemoteIP = val; 
}

std::string CPlayerInfo::GetRemotePort() const 
{ 
    return m_strRemotePort;
}

void CPlayerInfo::SetRemotePort(std::string val) 
{ 
    m_strRemotePort = val;
}

//////////////////////////////////////////////////////////////////////////
CPrintPictureInfo::CPrintPictureInfo():
m_strDownloadURL(""),
m_strRemoteIP(""),
m_strRemotePort("")
{
}

CPrintPictureInfo::~CPrintPictureInfo()
{
}

std::string CPrintPictureInfo::GetDownloadURL() const 
{ 
    return m_strDownloadURL; 
}

void CPrintPictureInfo::SetDownloadURL(std::string val) 
{ 
    m_strDownloadURL = val;
}

std::string CPrintPictureInfo::GetRemoteIP() const 
{
    return m_strRemoteIP;
}

void CPrintPictureInfo::SetRemoteIP(std::string val) 
{ 
    m_strRemoteIP = val; 
}

std::string CPrintPictureInfo::GetRemotePort() const 
{ 
    return m_strRemotePort;
}

void CPrintPictureInfo::SetRemotePort(std::string val) 
{ 
    m_strRemotePort = val;
}

//////////////////////////////////////////////////////////////////////////
CBurnServerInfo::CBurnServerInfo():
m_strIP(""),
m_strPort(""),
m_strBurnServerName(""),
m_strDescription(""),
m_strBurnServerType(BURNSERVER_TYPE_NORMAL),//默认为普通刻录机
m_strBurnServerTypeDescription(""),
m_strIsDefault(""),
m_strIsAvailable(BURNSERVER_AVAILABLE_NO)//默认非联通
{
}

CBurnServerInfo::~CBurnServerInfo()
{
}

CBurnServerInfo::CBurnServerInfo(const CBurnServerInfo &burnServerInfo):
m_strIP(burnServerInfo.GetIP()),
m_strPort(burnServerInfo.GetPort()),
m_strBurnServerName(burnServerInfo.GetBurnServerName()),
m_strDescription(burnServerInfo.GetDescription()),
m_vecCDROMInfo(burnServerInfo.GetVecCDROMDriverInfo()),
m_strBurnServerType(burnServerInfo.GetBurnServerType()),
m_strBurnServerTypeDescription(burnServerInfo.GetBurnServerTypeDescription()),
m_strIsDefault(burnServerInfo.GetIsDefault()),
m_strIsAvailable(burnServerInfo.GetIsAvailable())
{

}

CBurnServerInfo & CBurnServerInfo::operator=(const CBurnServerInfo &burnServerInfo)
{
    m_strIP=burnServerInfo.GetIP();
    m_strPort=burnServerInfo.GetPort();
    m_strBurnServerName=burnServerInfo.GetBurnServerName();
    m_strDescription=burnServerInfo.GetDescription();
    m_vecCDROMInfo=burnServerInfo.GetVecCDROMDriverInfo();
    m_strBurnServerType=burnServerInfo.GetBurnServerType();
    m_strBurnServerTypeDescription=burnServerInfo.GetBurnServerTypeDescription();
    m_strIsDefault=burnServerInfo.GetIsDefault();
    m_strIsAvailable=burnServerInfo.GetIsAvailable();

    return *this;
}

std::string CBurnServerInfo::GetIP() const
{ 
    return m_strIP;
}

void CBurnServerInfo::SetIP(std::string val) 
{ 
    m_strIP = val;
}

std::string CBurnServerInfo::GetPort() const 
{ 
    return m_strPort;
}

void CBurnServerInfo::SetPort(std::string val)
{ 
    m_strPort = val;
}

std::string CBurnServerInfo::GetBurnServerName() const
{
    return m_strBurnServerName;
}

void CBurnServerInfo::SetBurnServerName(std::string val) 
{ 
    m_strBurnServerName = val;
}

std::string CBurnServerInfo::GetDescription() const 
{
    return m_strDescription;
}

void CBurnServerInfo::SetDescription(std::string val) 
{ 
    m_strDescription = val; 
}

const std::vector<CCDROMDriverInfo> & CBurnServerInfo::GetVecCDROMDriverInfo() const 
{ 
    return m_vecCDROMInfo;
}

std::vector<CCDROMDriverInfo> & CBurnServerInfo:: GetVecCDROMDriverInfo()
{ 
    return m_vecCDROMInfo;
}

void CBurnServerInfo::SetVecCDROMInfo(const std::vector<CCDROMDriverInfo> &val) 
{ 
    m_vecCDROMInfo = val;
}

std::string CBurnServerInfo::GetBurnServerType() const 
{ 
    return m_strBurnServerType;
}

void CBurnServerInfo::SetBurnServerType(std::string val) 
{
    if ((val != BURNSERVER_TYPE_NORMAL) &&
        (val != BURNSERVER_TYPE_EPSON) &&
        (val != BURNSERVER_TYPE_PRIMERA))
    {
        val=BURNSERVER_TYPE_NORMAL;
    }

    m_strBurnServerType = val;
}

std::string CBurnServerInfo::GetBurnServerTypeDescription() const 
{
    return m_strBurnServerTypeDescription;
}

void CBurnServerInfo::SetBurnServerTypeDescription(std::string val) 
{
    m_strBurnServerTypeDescription = val; 
}

std::string CBurnServerInfo::GetIsDefault() const 
{ 
    return m_strIsDefault;
}

void CBurnServerInfo::SetIsDefault(std::string val)
{ 
    m_strIsDefault = val; 
}

std::string CBurnServerInfo::GetIsAvailable() const
{ 
    return m_strIsAvailable; 
}

void CBurnServerInfo::SetIsAvailable(std::string val) 
{
    m_strIsAvailable = val; 
}

//////////////////////////////////////////////////////////////////////////
CCommonBurnParam::CCommonBurnParam():
m_strBurnType(BURN_TYPE_SINGLE),
m_strBurnPassword(""),
m_strContentPassword(""),
m_strCDName("")
{
}

CCommonBurnParam::~CCommonBurnParam()
{
}

std::string CCommonBurnParam::GetBurnType() const 
{ 
    return m_strBurnType;
}

void CCommonBurnParam::SetBurnType(std::string val) 
{
    //默认设置为单盘刻录
    if (("" == val) ||
        ( (val != "1") && (val != "2") && (val != "3") ))
    {
        val="1";
    }
    m_strBurnType = val;
}

std::string CCommonBurnParam::GetBurnPassword() const 
{ 
    return m_strBurnPassword;
}

void CCommonBurnParam::SetBurnPassword(std::string val) 
{ 
    m_strBurnPassword = val;
}

std::string CCommonBurnParam::GetContentPassword() const
{
    return m_strContentPassword;
}

void CCommonBurnParam::SetContentPassword(std::string val) 
{
    m_strContentPassword = val;
}

std::string CCommonBurnParam::GetCDName() const 
{
    return m_strCDName;
}

void CCommonBurnParam::SetCDName(std::string val)
{
    m_strCDName = val; 
}

//////////////////////////////////////////////////////////////////////////
CRTBurnInfo::CRTBurnInfo(void):
m_strJobID(""),
m_strJobType(""),
m_strUpServerIP(""),
m_strUpServerPort(""),
m_strNewFileName(""),
m_strJobFlag("JOB_FLAG_SAVE_AND_BURN"),
m_strStateFlag(""),
m_strStateDescription(""),
m_strPlayListContent(""),
m_strNoteFileRemoteIP(""),
m_strNoteFileRemotePort(""),
m_strNoteFilePath(""),
m_strAutorunFileRemoteIP(""),
m_strAutorunFileRemotePort(""),
m_strAutorunFilePath(""),
m_strHardDiskBackup(""),
m_strSpeed(""),
m_strCDAlarmLimit(""),
m_strCDAutoFormat(""),
m_strBurnBufferSize(""),
m_strHardDiskBakAlarmLimit(""),
m_strStreamType(""),
m_strMd5("")
{
}

CRTBurnInfo::~CRTBurnInfo(void)
{
}

CRTBurnInfo::CRTBurnInfo(const CRTBurnInfo &RTBurnInfo):
m_strJobID(RTBurnInfo.GetJobID()),
m_strJobType(RTBurnInfo.GetJobType()),
m_BurnServerInfo(RTBurnInfo.GetBurnServerInfo()),
m_CommonBurnParam(RTBurnInfo.GetCommonBurnParam()),
m_vecLocationInfo(RTBurnInfo.GetVecVideoLocationInfo()),
m_strUpServerIP(RTBurnInfo.GetUpServerIP()),
m_strUpServerPort(RTBurnInfo.GetUpServerPort()),
m_strNewFileName(RTBurnInfo.GetNewFileName()),
m_strJobFlag(RTBurnInfo.GetJobFlag()),
m_strStateFlag(RTBurnInfo.GetStateFlag()),
m_strStateDescription(RTBurnInfo.GetStateDescription()),
m_PlayerInfo(RTBurnInfo.GetPlayerInfo()),
m_strPlayListContent(RTBurnInfo.GetPlayListContent()),
m_strNoteFileRemoteIP(RTBurnInfo.GetNoteFileRemoteIP()),
m_strNoteFileRemotePort(RTBurnInfo.GetNoteFileRemotePort()),
m_strNoteFilePath(RTBurnInfo.GetNoteFileDownloadURL()),
m_strAutorunFileRemoteIP(RTBurnInfo.GetAutorunFileRemoteIP()),
m_strAutorunFileRemotePort(RTBurnInfo.GetAutorunFileRemotePort()),
m_strAutorunFilePath(RTBurnInfo.GetAutorunFilePath()),
m_PlaylistFileInfo(RTBurnInfo.GetPlaylistFileInfo()),
m_strHardDiskBackup(RTBurnInfo.GetHardDiskBackup()),
m_strSpeed(RTBurnInfo.GetSpeed()),
m_strCDAlarmLimit(RTBurnInfo.GetCDAlarmLimit()),
m_strCDAutoFormat(RTBurnInfo.GetCDAutoFormat()),
m_strBurnBufferSize(RTBurnInfo.GetBurnBufferSize()),
m_strHardDiskBakAlarmLimit(RTBurnInfo.GetHardDiskBakAlarmLimit()),
m_strStreamType(RTBurnInfo.GetStreamType()),
m_strMd5(RTBurnInfo.GetMd5())
{

}

CRTBurnInfo & CRTBurnInfo::operator=(const CRTBurnInfo &RTBurnInfo)
{
    m_strJobID=RTBurnInfo.GetJobID();
    m_strJobType=RTBurnInfo.GetJobType();
    m_BurnServerInfo=RTBurnInfo.GetBurnServerInfo();
    m_CommonBurnParam=RTBurnInfo.GetCommonBurnParam();
    m_vecLocationInfo=RTBurnInfo.GetVecVideoLocationInfo();
    m_strUpServerIP=RTBurnInfo.GetUpServerIP();
    m_strUpServerPort=RTBurnInfo.GetUpServerPort();
    m_strNewFileName=RTBurnInfo.GetNewFileName();
    m_strJobFlag=RTBurnInfo.GetJobFlag();
    m_strStateFlag=RTBurnInfo.GetStateFlag();
    m_strStateDescription=RTBurnInfo.GetStateDescription();
    m_PlayerInfo=RTBurnInfo.GetPlayerInfo();
    m_strPlayListContent=RTBurnInfo.GetPlayListContent();
    m_strNoteFileRemoteIP=RTBurnInfo.GetNoteFileRemoteIP();
    m_strNoteFileRemotePort=RTBurnInfo.GetNoteFileRemotePort();
    m_strNoteFilePath=RTBurnInfo.GetNoteFileDownloadURL();
    m_strAutorunFileRemoteIP=RTBurnInfo.GetAutorunFileRemoteIP();
    m_strAutorunFileRemotePort=RTBurnInfo.GetAutorunFileRemotePort();
    m_strAutorunFilePath=RTBurnInfo.GetAutorunFilePath();
    m_PlaylistFileInfo=RTBurnInfo.GetPlaylistFileInfo();
    m_strHardDiskBackup=RTBurnInfo.GetHardDiskBackup();
    m_strSpeed=RTBurnInfo.GetSpeed();
    m_strCDAlarmLimit=RTBurnInfo.GetCDAlarmLimit();
    m_strCDAutoFormat=RTBurnInfo.GetCDAutoFormat();
    m_strBurnBufferSize=RTBurnInfo.GetBurnBufferSize();
    m_strHardDiskBakAlarmLimit=RTBurnInfo.GetHardDiskBakAlarmLimit();
    m_strStreamType=RTBurnInfo.GetStreamType();
    m_strMd5=RTBurnInfo.GetMd5();
    return *this;
}

std::string CRTBurnInfo::GetJobID() const 
{
    return m_strJobID;
}

void CRTBurnInfo::SetJobID(std::string val)
{ 
    m_strJobID = val;
}

std::string CRTBurnInfo::GetJobType() const 
{ 
    return m_strJobType; 
}

void CRTBurnInfo::SetJobType(std::string val)
{ 
    m_strJobType = val;
}

const CBurnServerInfo & CRTBurnInfo::GetBurnServerInfo() const 
{
    return m_BurnServerInfo; 
}

CBurnServerInfo  & CRTBurnInfo::GetBurnServerInfo()
{
    return m_BurnServerInfo;
}

void CRTBurnInfo::SetBurnServerInfo(const CBurnServerInfo &val)
{ 
    m_BurnServerInfo = val;
}

const CCommonBurnParam & CRTBurnInfo::GetCommonBurnParam() const
{
    return m_CommonBurnParam; 
}

CCommonBurnParam & CRTBurnInfo::GetCommonBurnParam()
{
    return m_CommonBurnParam; 
}

void CRTBurnInfo::SetCommonBurnParam(const CCommonBurnParam &val)
{ 
    m_CommonBurnParam = val;
}

const std::vector<CLocationInfo> & CRTBurnInfo::GetVecVideoLocationInfo() const
{ 
    return m_vecLocationInfo;
}

std::vector<CLocationInfo> & CRTBurnInfo::GetVecVideoLocationInfo()
{
    return m_vecLocationInfo;
}

void CRTBurnInfo::SetVecVideoLocationInfo(const std::vector<CLocationInfo> &val)
{ 
    m_vecLocationInfo = val;
}

std::string CRTBurnInfo::GetUpServerIP() const
{
    return m_strUpServerIP;
}

void CRTBurnInfo::SetUpServerIP(std::string val) 
{ 
    m_strUpServerIP = val;
}

std::string CRTBurnInfo::GetUpServerPort() const
{ 
    return m_strUpServerPort;
}

void CRTBurnInfo::SetUpServerPort(std::string val) 
{ 
    m_strUpServerPort = val;
}

std::string CRTBurnInfo::GetNewFileName() const 
{ 
    return m_strNewFileName;
}

void CRTBurnInfo::SetNewFileName(std::string val) 
{ 
    m_strNewFileName = val;
}

std::string CRTBurnInfo::GetJobFlag() const 
{ 
    return m_strJobFlag;
}

void CRTBurnInfo::SetJobFlag(std::string val) 
{ 
    m_strJobFlag = val; 
}

std::string CRTBurnInfo::GetStateFlag() const 
{ 
    return m_strStateFlag;
}

void CRTBurnInfo::SetStateFlag(std::string val)
{ 
    m_strStateFlag = val;
}

std::string CRTBurnInfo::GetStateDescription() const
{
    return m_strStateDescription;
}

void CRTBurnInfo::SetStateDescription(std::string val) 
{ 
    m_strStateDescription = val;
}

const CPlayerInfo & CRTBurnInfo::GetPlayerInfo() const
{ 
    return m_PlayerInfo;
}

CPlayerInfo & CRTBurnInfo::GetPlayerInfo()
{
    return m_PlayerInfo;
}

void CRTBurnInfo::SetPlayerInfo(CPlayerInfo val) 
{ 
    m_PlayerInfo = val;
}

std::string CRTBurnInfo::GetPlayListContent() const 
{ 
    return m_strPlayListContent;
}

void CRTBurnInfo::SetPlayListContent(std::string val)
{
    m_strPlayListContent = val;
}

std::string CRTBurnInfo::GetNoteFileRemoteIP() const 
{ 
    return m_strNoteFileRemoteIP;
}

void CRTBurnInfo::SetNoteFileRemoteIP(std::string val) 
{ 
    m_strNoteFileRemoteIP = val; 
}

std::string CRTBurnInfo::GetNoteFileRemotePort() const 
{ 
    return m_strNoteFileRemotePort;
}

void CRTBurnInfo::SetNoteFileRemotePort(std::string val) 
{ 
    m_strNoteFileRemotePort = val;
}

std::string CRTBurnInfo::GetNoteFileDownloadURL() const 
{ 
    return m_strNoteFilePath;
}

void CRTBurnInfo::SetNoteFileDownloadURL(std::string val) 
{
    m_strNoteFilePath = val; 
}

const CPlaylistFileInfo & CRTBurnInfo::GetPlaylistFileInfo() const
{ 
    return m_PlaylistFileInfo;
}
CPlaylistFileInfo & CRTBurnInfo::GetPlaylistFileInfo()
{
    return m_PlaylistFileInfo;
}

void CRTBurnInfo::SetPlaylistFileInfo(const CPlaylistFileInfo &val)
{ 
    m_PlaylistFileInfo = val;
}

std::string CRTBurnInfo::GetAutorunFileRemoteIP() const 
{ 
    return m_strAutorunFileRemoteIP; 
}

void CRTBurnInfo::SetAutorunFileRemoteIP(std::string val) 
{ 
    m_strAutorunFileRemoteIP = val;
}

std::string CRTBurnInfo::GetAutorunFileRemotePort() const
{ 
    return m_strAutorunFileRemotePort;
}

void CRTBurnInfo::SetAutorunFileRemotePort(std::string val) 
{ 
    m_strAutorunFileRemotePort = val;
}

std::string CRTBurnInfo::GetAutorunFilePath() const 
{
    return m_strAutorunFilePath; 
}

void CRTBurnInfo::SetAutorunFilePath(std::string val) 
{ 
    m_strAutorunFilePath = val;
}

std::string CRTBurnInfo::GetHardDiskBackup() const 
{ 
    return m_strHardDiskBackup;
}

void CRTBurnInfo::SetHardDiskBackup(std::string val) 
{ 
    m_strHardDiskBackup = val;
}

std::string CRTBurnInfo::GetSpeed() const 
{ 
    return m_strSpeed;
}

void CRTBurnInfo::SetSpeed(std::string val) 
{ 
    m_strSpeed = val;
}

std::string CRTBurnInfo::GetCDAlarmLimit() const 
{ 
    return m_strCDAlarmLimit;
}

void CRTBurnInfo::SetCDAlarmLimit(std::string val)
{
    m_strCDAlarmLimit = val;
}

std::string CRTBurnInfo::GetCDAutoFormat() const 
{
    return m_strCDAutoFormat; 
}

void CRTBurnInfo::SetCDAutoFormat(std::string val) 
{ 
    m_strCDAutoFormat = val;
}

std::string CRTBurnInfo::GetBurnBufferSize() const 
{
    return m_strBurnBufferSize;
}

void CRTBurnInfo::SetBurnBufferSize(std::string val) 
{ 
    m_strBurnBufferSize = val;
}

std::string CRTBurnInfo::GetHardDiskBakAlarmLimit() const 
{ 
    return m_strHardDiskBakAlarmLimit;
}

void CRTBurnInfo::SetHardDiskBakAlarmLimit(std::string val) 
{ 
    m_strHardDiskBakAlarmLimit = val;
}

std::string CRTBurnInfo::GetStreamType() const 
{ 
    return m_strStreamType;
}

void CRTBurnInfo::SetStreamType(std::string val) 
{ 
    m_strStreamType = val;
}

std::string CRTBurnInfo::GetMd5() const 
{
    return m_strMd5;
}

void CRTBurnInfo::SetMd5(std::string val)
{ 
    m_strMd5 = val; 
}

//////////////////////////////////////////////////////////////////////////
CNormalBurnJobInfo::CNormalBurnJobInfo():
m_strJobID(""),
m_strJobName(""),
m_strJobDescription(""),
m_dJobSize(-1),
m_strNoteFileRemoteIP(""),
m_strNoteFileRemotePort(""),
m_strNoteRelativePath(""),
m_strNoteFileName(""),
m_bBurnNoteFile(true),
m_strAutorunFileRemoteIP(""),
m_strAutorunFileRemotePort(""),
m_strAutorunFilePath(""),
m_strVideoFileRemoteIP(""),
m_strVideoFileRemotePort(""),
m_strVideoFileRelativePath(""),
m_strPlayListContent(""),
m_strUpServerIP(""),
m_strUpServerPort("")
{
}

CNormalBurnJobInfo::~CNormalBurnJobInfo()
{
}

CNormalBurnJobInfo::CNormalBurnJobInfo(const CNormalBurnJobInfo &normalBurnJobInfo):
m_strJobID(normalBurnJobInfo.GetJobID()),
m_strJobName(normalBurnJobInfo.GetJobName()),
m_strJobDescription(normalBurnJobInfo.GetJobDescription()),
m_dJobSize(normalBurnJobInfo.GetJobSize()),
m_BurnServerInfo(normalBurnJobInfo.GetBurnServerInfo()),
m_CommonBurnParam(normalBurnJobInfo.GetCommonBurnParam()),
m_PlayerInfo(normalBurnJobInfo.GetPlayerInfo()),
m_strNoteFileRemoteIP(normalBurnJobInfo.GetNoteFileRemoteIP()),
m_strNoteFileRemotePort(normalBurnJobInfo.GetNoteFileRemotePort()),
m_strNoteRelativePath(normalBurnJobInfo.GetNoteRelativePath()),
m_strNoteFileName(normalBurnJobInfo.GetNoteFileName()),
m_bBurnNoteFile(normalBurnJobInfo.GetBurnNoteFile()),
m_strAutorunFileRemoteIP(normalBurnJobInfo.GetAutorunFileRemoteIP()),
m_strAutorunFileRemotePort(normalBurnJobInfo.GetAutorunFileRemotePort()),
m_strAutorunFilePath(normalBurnJobInfo.GetAutorunFilePath()),
m_strVideoFileRemoteIP(normalBurnJobInfo.GetVideoFileRemoteIP()),
m_strVideoFileRemotePort(normalBurnJobInfo.GetVideoFileRemotePort()),
m_strVideoFileRelativePath(normalBurnJobInfo.GetVideoFileRelativePath()),
m_vecVideoLocationInfo(normalBurnJobInfo.GetVecLocationInfo()),
m_strPlayListContent(normalBurnJobInfo.GetPlayListContent()),
m_mapLabelFieldText(normalBurnJobInfo.GetMapLabelFieldText()),
m_PlaylistFileInfo(normalBurnJobInfo.GetPlaylistFileInfo()),
m_strUpServerIP(normalBurnJobInfo.GetUpServerIP()),
m_strUpServerPort(normalBurnJobInfo.GetUpServerPort())
{
}

CNormalBurnJobInfo & CNormalBurnJobInfo::operator=(const CNormalBurnJobInfo &normalBurnJobInfo)
{
    if (&normalBurnJobInfo != this)
    {
        m_strJobID=normalBurnJobInfo.GetJobID();
        m_strJobName=normalBurnJobInfo.GetJobName();
        m_strJobDescription=normalBurnJobInfo.GetJobDescription();
        m_dJobSize=normalBurnJobInfo.GetJobSize();
        m_BurnServerInfo=normalBurnJobInfo.GetBurnServerInfo();
        m_CommonBurnParam=normalBurnJobInfo.GetCommonBurnParam();
        m_PlayerInfo=normalBurnJobInfo.GetPlayerInfo();
        m_strNoteFileRemoteIP=normalBurnJobInfo.GetNoteFileRemoteIP();
        m_strNoteFileRemotePort=normalBurnJobInfo.GetNoteFileRemotePort();
        m_strNoteRelativePath=normalBurnJobInfo.GetNoteRelativePath();
        m_strNoteFileName=normalBurnJobInfo.GetNoteFileName();
        m_bBurnNoteFile=normalBurnJobInfo.GetBurnNoteFile();
        m_strAutorunFileRemoteIP=normalBurnJobInfo.GetAutorunFileRemoteIP();
        m_strAutorunFileRemotePort=normalBurnJobInfo.GetAutorunFileRemotePort();
        m_strAutorunFilePath=normalBurnJobInfo.GetAutorunFilePath();
        m_strVideoFileRemoteIP=normalBurnJobInfo.GetVideoFileRemoteIP();
        m_strVideoFileRemotePort=normalBurnJobInfo.GetVideoFileRemotePort();
        m_strVideoFileRelativePath=normalBurnJobInfo.GetVideoFileRelativePath();
        m_vecVideoLocationInfo=normalBurnJobInfo.GetVecLocationInfo();
        m_strPlayListContent=normalBurnJobInfo.GetPlayListContent();
        m_mapLabelFieldText=normalBurnJobInfo.GetMapLabelFieldText();
        m_PlaylistFileInfo=normalBurnJobInfo.GetPlaylistFileInfo();
        m_strUpServerIP=normalBurnJobInfo.GetUpServerIP();
        m_strUpServerPort=normalBurnJobInfo.GetUpServerPort();
    }

    return *this;
}

std::string CNormalBurnJobInfo::GetJobID() const 
{
    return m_strJobID;
}

void CNormalBurnJobInfo::SetJobID(std::string val) 
{
    m_strJobID = val;
}

std::string CNormalBurnJobInfo::GetJobName() const 
{ 
    return m_strJobName;
}

void CNormalBurnJobInfo::SetJobName(std::string val) 
{
    m_strJobName = val; 
}

std::string CNormalBurnJobInfo::GetJobDescription() const 
{ 
    return m_strJobDescription;
}

void CNormalBurnJobInfo::SetJobDescription(std::string val) 
{ 
    m_strJobDescription = val;
}

void CNormalBurnJobInfo::CalculateJobSize()
{
    m_dJobSize=0.0;

    double dFileSize=0.0;
    char szTemp[32]={0};

    std::vector<CLocationInfo>::const_iterator iterLocation;
    for (iterLocation=m_vecVideoLocationInfo.begin();iterLocation!=m_vecVideoLocationInfo.end();
        ++iterLocation)
    {
        std::vector<CSingleFileInfo>::const_iterator iterFile;
        for (iterFile=iterLocation->GetVecFileInfo().begin();iterFile!=iterLocation->GetVecFileInfo().end();
            ++iterFile)
        {
            dFileSize=0.0;

            std::string strFileSize=iterFile->GetSize();
            if (strFileSize != "")
            {
                dFileSize=atof(strFileSize.c_str());
            }

            m_dJobSize+=dFileSize;
        }
    }
}

double CNormalBurnJobInfo::GetJobSize() const
{
    return m_dJobSize; 
}

void CNormalBurnJobInfo::SetJobSize(double val)
{ 
    m_dJobSize = val;
}

const CBurnServerInfo & CNormalBurnJobInfo::GetBurnServerInfo() const 
{
    return m_BurnServerInfo;
}

CBurnServerInfo & CNormalBurnJobInfo::GetBurnServerInfo()
{
    return m_BurnServerInfo;
}

void CNormalBurnJobInfo::SetBurnServerInfo(CBurnServerInfo val) 
{ 
    m_BurnServerInfo = val;
}

const CCommonBurnParam & CNormalBurnJobInfo::GetCommonBurnParam() const 
{
    return m_CommonBurnParam;
}

CCommonBurnParam & CNormalBurnJobInfo::GetCommonBurnParam()
{
    return m_CommonBurnParam;;
}

void CNormalBurnJobInfo::SetCommonBurnParam(CCommonBurnParam val)
{ 
    m_CommonBurnParam = val;
}

const CPlayerInfo & CNormalBurnJobInfo::GetPlayerInfo() const 
{
    return m_PlayerInfo;
}

CPlayerInfo & CNormalBurnJobInfo::GetPlayerInfo()
{
    return m_PlayerInfo;
}

void CNormalBurnJobInfo::SetPlayerInfo(CPlayerInfo val) 
{ 
    m_PlayerInfo = val;
}

std::string CNormalBurnJobInfo::GetNoteFileRemoteIP() const 
{
    return m_strNoteFileRemoteIP;
}

void CNormalBurnJobInfo::SetNoteFileRemoteIP(std::string val) 
{
    m_strNoteFileRemoteIP = val;
}

std::string CNormalBurnJobInfo::GetNoteFileRemotePort() const 
{
    return m_strNoteFileRemotePort;
}

void CNormalBurnJobInfo::SetNoteFileRemotePort(std::string val) 
{ 
    m_strNoteFileRemotePort = val;
}

std::string CNormalBurnJobInfo::GetNoteRelativePath() const 
{ 
    return m_strNoteRelativePath;
}

void CNormalBurnJobInfo::SetNoteRelativePath(std::string val) 
{
    m_strNoteRelativePath = val; 
}

std::string CNormalBurnJobInfo::GetNoteFileName() const 
{ 
    return m_strNoteFileName;
}

void CNormalBurnJobInfo::SetNoteFileName(std::string val)
{ 
    m_strNoteFileName = val;
}

bool CNormalBurnJobInfo::GetBurnNoteFile() const
{ 
    return m_bBurnNoteFile;
}

void CNormalBurnJobInfo::SetBurnNoteFile(bool val) 
{
    m_bBurnNoteFile = val; 
}

std::string CNormalBurnJobInfo::GetAutorunFileRemoteIP() const 
{ 
    return m_strAutorunFileRemoteIP; 
}

void CNormalBurnJobInfo::SetAutorunFileRemoteIP(std::string val) 
{ 
    m_strAutorunFileRemoteIP = val;
}

std::string CNormalBurnJobInfo::GetAutorunFileRemotePort() const
{ 
    return m_strAutorunFileRemotePort;
}

void CNormalBurnJobInfo::SetAutorunFileRemotePort(std::string val) 
{ 
    m_strAutorunFileRemotePort = val;
}

std::string CNormalBurnJobInfo::GetAutorunFilePath() const 
{
    return m_strAutorunFilePath; 
}

void CNormalBurnJobInfo::SetAutorunFilePath(std::string val) 
{ 
    m_strAutorunFilePath = val;
}

std::string CNormalBurnJobInfo::GetVideoFileRemoteIP() const 
{ 
    return m_strVideoFileRemoteIP;
}

void CNormalBurnJobInfo::SetVideoFileRemoteIP(std::string val)
{
    m_strVideoFileRemoteIP = val;
}

std::string CNormalBurnJobInfo::GetVideoFileRemotePort() const
{ 
    return m_strVideoFileRemotePort; 
}

void CNormalBurnJobInfo::SetVideoFileRemotePort(std::string val)
{ 
    m_strVideoFileRemotePort = val;
}

std::string CNormalBurnJobInfo::GetVideoFileRelativePath() const
{ 
    return m_strVideoFileRelativePath;
}

void CNormalBurnJobInfo::SetVideoFileRelativePath(std::string val) 
{ 
    m_strVideoFileRelativePath = val; 
}

const std::vector<CLocationInfo> & CNormalBurnJobInfo::GetVecLocationInfo() const 
{
    return m_vecVideoLocationInfo;
}

std::vector<CLocationInfo> & CNormalBurnJobInfo::GetVecLocationInfo()
{
    return m_vecVideoLocationInfo;
}

void CNormalBurnJobInfo::SetVecVideoLocationInfo(std::vector<CLocationInfo> val)
{
    m_vecVideoLocationInfo = val;
}

std::string CNormalBurnJobInfo::GetPlayListContent() const 
{
    return m_strPlayListContent;
}

void CNormalBurnJobInfo::SetPlayListContent(std::string val) 
{ 
    m_strPlayListContent = val; 
}

const std::map<std::string,std::string> & CNormalBurnJobInfo::GetMapLabelFieldText() const
{
    return m_mapLabelFieldText;
}

std::map<std::string,std::string> & CNormalBurnJobInfo::GetMapLabelFieldText()
{
    return m_mapLabelFieldText;
}

void CNormalBurnJobInfo::SetMapLabelFieldText(std::map<std::string,std::string> val)
{
    m_mapLabelFieldText = val;
}

const CPlaylistFileInfo & CNormalBurnJobInfo::GetPlaylistFileInfo() const 
{ 
    return m_PlaylistFileInfo;
}

CPlaylistFileInfo & CNormalBurnJobInfo::GetPlaylistFileInfo()
{
    return m_PlaylistFileInfo;
}

void CNormalBurnJobInfo::SetPlaylistFileInfo(CPlaylistFileInfo val)
{
    m_PlaylistFileInfo = val;
}

std::string CNormalBurnJobInfo::GetUpServerIP() const
{
    return m_strUpServerIP; 
}

void CNormalBurnJobInfo::SetUpServerIP(std::string val) 
{
    m_strUpServerIP = val;
}

std::string CNormalBurnJobInfo::GetUpServerPort() const
{
    return m_strUpServerPort;
}

void CNormalBurnJobInfo::SetUpServerPort(std::string val) 
{
    m_strUpServerPort = val; 
}

//////////////////////////////////////////////////////////////////////////
CNormalBurnInfo::CNormalBurnInfo():
m_strBurnFlag(BURN_FLAG_CMS)
{
}

CNormalBurnInfo::~CNormalBurnInfo()
{
}

CNormalBurnInfo::CNormalBurnInfo(const CNormalBurnInfo &normalBurnInfo):
m_vecBurnServerInfo(normalBurnInfo.GetVecBurnServerInfo()),
m_strBurnFlag(normalBurnInfo.GetBurnFlag()),
m_CommonBurnParam(normalBurnInfo.GetCommonBurnParam()),
m_PlayerInfo(normalBurnInfo.GetPlayerInfo()),
m_vecNormalBurnJobInfoEx(normalBurnInfo.GetVecNormalBurnJobInfo())
{

}

CNormalBurnInfo & CNormalBurnInfo::operator=(const CNormalBurnInfo &normalBurnInfo)
{
    m_vecBurnServerInfo=normalBurnInfo.GetVecBurnServerInfo();
    m_strBurnFlag=normalBurnInfo.GetBurnFlag();
    m_CommonBurnParam=normalBurnInfo.GetCommonBurnParam();
    m_PlayerInfo=normalBurnInfo.GetPlayerInfo();
    m_vecNormalBurnJobInfoEx=normalBurnInfo.GetVecNormalBurnJobInfo();

    return *this;
}

const std::vector<CBurnServerInfo> & CNormalBurnInfo::GetVecBurnServerInfo() const 
{ 
    return m_vecBurnServerInfo;
}

std::vector<CBurnServerInfo> & CNormalBurnInfo::GetVecBurnServerInfo()
{
    return m_vecBurnServerInfo;
}

void CNormalBurnInfo::SetVecBurnServerInfo(std::vector<CBurnServerInfo> val)
{ 
    m_vecBurnServerInfo = val;
}

std::string  CNormalBurnInfo::GetBurnFlag() const
{
    return m_strBurnFlag;
}

void CNormalBurnInfo::SetBurnFlag(std::string val)
{
    if ((val != BURN_FLAG_CMS) &&
        (val != BURN_FLAG_COS))
    {
        val=BURN_FLAG_CMS;
    }

    m_strBurnFlag = val;
}

const CCommonBurnParam  & CNormalBurnInfo::GetCommonBurnParam() const 
{
    return m_CommonBurnParam;
}

CCommonBurnParam & CNormalBurnInfo::GetCommonBurnParam()
{
    return m_CommonBurnParam;
}

void CNormalBurnInfo::SetCommonBurnParam(CCommonBurnParam val) 
{
    m_CommonBurnParam = val;
}

const CPlayerInfo & CNormalBurnInfo:: GetPlayerInfo() const 
{ 
    return m_PlayerInfo;
}

CPlayerInfo & CNormalBurnInfo:: GetPlayerInfo()
{
    return m_PlayerInfo;
}

void CNormalBurnInfo::SetPlayerInfo(CPlayerInfo val)
{
    m_PlayerInfo = val;
}

const std::vector<CNormalBurnJobInfoEx> & CNormalBurnInfo::GetVecNormalBurnJobInfo() const 
{ 
    return m_vecNormalBurnJobInfoEx;
}

std::vector<CNormalBurnJobInfoEx> & CNormalBurnInfo::GetVecNormalBurnJobInfo()
{
    return m_vecNormalBurnJobInfoEx;
}

void CNormalBurnInfo::SetVecNormalBurnJobInfo(const std::vector<CNormalBurnJobInfoEx> &val) 
{ 
    m_vecNormalBurnJobInfoEx = val;
}

bool CNormalBurnInfo::GetDefaultBurnServer(CBurnServerInfo &burnServerInfo)
{
    std::vector<CBurnServerInfo>::iterator iter;
    for (iter=m_vecBurnServerInfo.begin();iter!=m_vecBurnServerInfo.end();++iter)
    {
        if ("1" == iter->GetIsDefault())
        {
            burnServerInfo=(*iter);
            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////
CNormalBurnJobInfoEx::CNormalBurnJobInfoEx() : 
m_strCopies(""),
m_strDiscType(""),
m_strSpeed(""),
m_strCloseDisc(""),
m_strDeleteFile(""),
m_nFileTotalSize(0),
m_nFileDownloadSize(0)
{
}

CNormalBurnJobInfoEx::~CNormalBurnJobInfoEx()
{
}

INT64 CNormalBurnJobInfoEx::GetFileTotalSize() const 
{ 
    return m_nFileTotalSize;
}

void CNormalBurnJobInfoEx::SetFileTotalSize(INT64 val) 
{
    m_nFileTotalSize = val;
}

INT64 CNormalBurnJobInfoEx::GetFileDownloadSize() const 
{
    return m_nFileDownloadSize;
}

void CNormalBurnJobInfoEx::SetFileDownloadSize(INT64 val) 
{
    m_nFileDownloadSize = val;
}

CBurnServerInfoEx::CBurnServerInfoEx(const CBurnServerInfo &burnServerInfo):
CBurnServerInfo(burnServerInfo),
m_vecDevSingle(burnServerInfo.GetVecCDROMDriverInfo()),
m_vecDevMultiSYN(burnServerInfo.GetVecCDROMDriverInfo()),
m_vecDevMultiASYN(burnServerInfo.GetVecCDROMDriverInfo())
{
}

CBurnServerInfoEx::CBurnServerInfoEx(const CBurnServerInfoEx &burnServerInfoEx):
m_vecDevSingle(burnServerInfoEx.GetVecCDROMInfo0()),
m_vecDevMultiSYN(burnServerInfoEx.GetVecCDROMInfo1()),
m_vecDevMultiASYN(burnServerInfoEx.GetVecCDROMInfo2())
{
    m_strIP=burnServerInfoEx.GetIP();
    m_strPort=burnServerInfoEx.GetPort();
    m_strBurnServerName=burnServerInfoEx.GetBurnServerName();
    m_strDescription=burnServerInfoEx.GetDescription();
    m_vecCDROMInfo=burnServerInfoEx.GetVecCDROMInfo0();
    m_strBurnServerType=burnServerInfoEx.GetBurnServerType();
    m_strBurnServerTypeDescription=burnServerInfoEx.GetBurnServerTypeDescription();
    m_strIsDefault=burnServerInfoEx.GetIsDefault();
    m_strIsAvailable=burnServerInfoEx.GetIsAvailable();
}

CBurnServerInfoEx::~CBurnServerInfoEx()
{
}

CBurnServerInfoEx & CBurnServerInfoEx::operator=(const CBurnServerInfoEx &burnServerInfoEx)
{
    if ((void *)(this) != (void *)(&burnServerInfoEx))
    {
        CBurnServerInfo::operator =(burnServerInfoEx);

        m_vecDevSingle=burnServerInfoEx.GetVecCDROMInfo0();
        m_vecDevMultiSYN=burnServerInfoEx.GetVecCDROMInfo1();
        m_vecDevMultiASYN=burnServerInfoEx.GetVecCDROMInfo2();
    }

    return *this;
}

const std::vector<CCDROMDriverInfo> & CBurnServerInfoEx::GetVecCDROMInfo0() const 
{
    return m_vecDevSingle;
}

std::vector<CCDROMDriverInfo> & CBurnServerInfoEx::GetVecCDROMInfo0()
{
    return m_vecDevSingle;
}

void CBurnServerInfoEx::SetVecCDROMInfo0(std::vector<CCDROMDriverInfo> val) 
{
    m_vecDevSingle = val;
}

const std::vector<CCDROMDriverInfo> & CBurnServerInfoEx::GetVecCDROMInfo1() const
{ 
    return m_vecDevMultiSYN; 
}

std::vector<CCDROMDriverInfo> & CBurnServerInfoEx::GetVecCDROMInfo1()
{

    return m_vecDevMultiSYN; 
}

void CBurnServerInfoEx::SetVecCDROMInfo1(std::vector<CCDROMDriverInfo> val)
{ 
    m_vecDevMultiSYN = val;
}

const std::vector<CCDROMDriverInfo> & CBurnServerInfoEx::GetVecCDROMInfo2() const
{ 
    return m_vecDevMultiASYN; 
}

std::vector<CCDROMDriverInfo> & CBurnServerInfoEx::GetVecCDROMInfo2()
{
    return m_vecDevMultiASYN; 
}
void CBurnServerInfoEx::SetVecCDROMInfo2(std::vector<CCDROMDriverInfo> val) 
{
    m_vecDevMultiASYN = val;
}

//vecCDROMDriverInfo : dev from BurnServer
void CBurnServerInfoEx::UpdateCDROMDriverState(const std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo)
{
    LOG_INFO(("\r\n\r\n---------[CBurnServerInfoEx::UpdateCDROMDriverState]------------,%d\r\n",__LINE__));
    std::vector<CCDROMDriverInfo>::iterator iterT;
    for (iterT=m_vecDevSingle.begin();iterT!=m_vecDevSingle.end();++iterT)
    {
        LOG_INFO(("Single Dev id : %s,is selected : %s\r\n",iterT->GetID().c_str(),iterT->GetIsSelected().c_str()));
    }
    for (iterT=m_vecDevMultiSYN.begin();iterT!=m_vecDevMultiSYN.end();++iterT)
    {
        LOG_INFO(("SYN Dev id : %s,is selected : %s\r\n",iterT->GetID().c_str(),iterT->GetIsSelected().c_str()));
    }
    for (iterT=m_vecDevMultiASYN.begin();iterT!=m_vecDevMultiASYN.end();++iterT)
    {
        LOG_INFO(("ASYN Dev id : %s,is selected : %s\r\n",iterT->GetID().c_str(),iterT->GetIsSelected().c_str()));
    }
    LOG_INFO(("--------------------------------------------------\r\n\r\n"));

    std::vector<CCDROMDriverInfo>::iterator iterDevFromServer,iterDevFromUpper;
    std::vector<CCDROMDriverInfo>::const_iterator iterConst;
    std::vector<CCDROMDriverInfo> tempDevInfo;

    //如果上层协议给的光驱数量超过了实际的，那么以BurnServer给的为准

    // Single
    tempDevInfo=vecCDROMDriverInfo;
    for (iterDevFromServer=tempDevInfo.begin();iterDevFromServer!=tempDevInfo.end();
        ++iterDevFromServer)
    {
        iterDevFromServer->SetIsSelected(DEV_SELECTED_NO);

        for (iterDevFromUpper=m_vecDevSingle.begin();iterDevFromUpper!=m_vecDevSingle.end();
            ++iterDevFromUpper)
        {
            if (iterDevFromUpper->GetID() == iterDevFromServer->GetID())
            {
                iterDevFromServer->SetIsSelected(DEV_SELECTED_YES);

                break;
            }
        }
    }
    m_vecDevSingle=tempDevInfo;


    // SYN
    tempDevInfo=vecCDROMDriverInfo;
    for (iterDevFromServer=tempDevInfo.begin();iterDevFromServer!=tempDevInfo.end();
        ++iterDevFromServer)
    {
        iterDevFromServer->SetIsSelected(DEV_SELECTED_NO);

        for (iterDevFromUpper=m_vecDevMultiSYN.begin();iterDevFromUpper!=m_vecDevMultiSYN.end();
            ++iterDevFromUpper)
        {
            if (iterDevFromUpper->GetID() == iterDevFromServer->GetID())
            {
                iterDevFromServer->SetIsSelected(DEV_SELECTED_YES);

                break;
            }
        }
    }
    m_vecDevMultiSYN=tempDevInfo;


    // ASYN
    tempDevInfo=vecCDROMDriverInfo;
    for (iterDevFromServer=tempDevInfo.begin();iterDevFromServer!=tempDevInfo.end();
        ++iterDevFromServer)
    {
        iterDevFromServer->SetIsSelected(DEV_SELECTED_NO);

        for (iterDevFromUpper=m_vecDevMultiASYN.begin();iterDevFromUpper!=m_vecDevMultiASYN.end();
            ++iterDevFromUpper)
        {
            if (iterDevFromUpper->GetID() == iterDevFromServer->GetID())
            {
                iterDevFromServer->SetIsSelected(DEV_SELECTED_YES);

                break;
            }
        }
    }
    m_vecDevMultiASYN=tempDevInfo;

    LOG_INFO(("\r\n\r\n---------[CBurnServerInfoEx::UpdateCDROMDriverState]------------,%d\r\n",__LINE__));
    for (iterT=m_vecDevSingle.begin();iterT!=m_vecDevSingle.end();++iterT)
    {
        LOG_INFO(("Single Dev id : %s,is selected : %s\r\n",iterT->GetID().c_str(),iterT->GetIsSelected().c_str()));
    }
    for (iterT=m_vecDevMultiSYN.begin();iterT!=m_vecDevMultiSYN.end();++iterT)
    {
        LOG_INFO(("SYN Dev id : %s,is selected : %s\r\n",iterT->GetID().c_str(),iterT->GetIsSelected().c_str()));
    }
    for (iterT=m_vecDevMultiASYN.begin();iterT!=m_vecDevMultiASYN.end();++iterT)
    {
        LOG_INFO(("ASYN Dev id : %s,is selected : %s\r\n",iterT->GetID().c_str(),iterT->GetIsSelected().c_str()));
    }
    LOG_INFO(("--------------------------------------------------\r\n\r\n"));

    AdjustDevSelectLogic();
}

CBurnServerInfoEx::CBurnServerInfoEx():CBurnServerInfo()
{
}

void CBurnServerInfoEx::AdjustDevSelectLogic()
{
    //单盘刻录同一时刻只能选择一个光驱
    bool bSelOne=false;
    std::vector<CCDROMDriverInfo>::iterator iter;
    for (iter=m_vecDevSingle.begin();iter!=m_vecDevSingle.end();++iter)
    {
        if (DEV_SELECTED_YES == iter->GetIsSelected())
        {
            if (bSelOne)
            {
                iter->SetIsSelected(DEV_SELECTED_NO);
            }
            else
            {
                bSelOne=true;
            }
        }
    }
    //默认选中第一个光驱
    if ((!bSelOne) &&
        (!m_vecDevSingle.empty()) &&
        (m_vecDevSingle.size() > 0))
    {
        m_vecDevSingle[0].SetIsSelected(DEV_SELECTED_YES);
    }

    //多盘同刻、多盘续刻默认选择所有光驱
    bSelOne=false;
    for (iter=m_vecDevMultiSYN.begin();iter!=m_vecDevMultiSYN.end();++iter)
    {
        if (DEV_SELECTED_YES== iter->GetIsSelected())
        {
            bSelOne=true;

            break;
        }
    }
    if (!bSelOne)
    {
        LOG_WARNING(("[CBurnServerInfoEx::AdjustDevSelectLogic] No dev selected,%d\r\n",__LINE__));

        for (iter=m_vecDevMultiSYN.begin();iter!=m_vecDevMultiSYN.end();++iter)
        {
            iter->SetIsSelected(DEV_SELECTED_YES);
        }
    }

    bSelOne=false;
    for (iter=m_vecDevMultiASYN.begin();iter!=m_vecDevMultiASYN.end();++iter)
    {
        if (DEV_SELECTED_YES== iter->GetIsSelected())
        {
            bSelOne=true;

            break;
        }
    }
    if (!bSelOne)
    {
        LOG_WARNING(("[CBurnServerInfoEx::AdjustDevSelectLogic] No dev selected,%d\r\n",__LINE__));

        for (iter=m_vecDevMultiASYN.begin();iter!=m_vecDevMultiASYN.end();++iter)
        {
            iter->SetIsSelected(DEV_SELECTED_YES);
        }
    }
}
