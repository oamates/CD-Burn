#pragma once
#include <vector>
#include <string>
#include <map>

//////////////////////////////////////////////////////////////////////////
class CSingleFileInfo
{
public:
    CSingleFileInfo();
    ~CSingleFileInfo();

    std::string GetFileName() const;
    void SetFileName(std::string val);

    std::string GetNewFileName() const;
    void SetNewFileName(std::string val);

#define FILE_BURN_STATE_WAIT    "0"
#define FILE_BURN_STATE_BURNING "1"
#define FILE_BURN_STATE_DONE    "2"
    std::string GetBurnState() const;
    void SetBurnState(std::string val);

    std::string GetSize() const;
    void SetSize(std::string val);

    std::string GetPercent() const;
    void SetPercent(std::string val);
private:
    std::string     m_strFileName;//全路径
    std::string     m_strNewFileName;//用于重命名
    std::string     m_strBurnState;
    std::string     m_strSize;
    std::string     m_strPercent;//下载进度
};

//////////////////////////////////////////////////////////////////////////
#define BURN_YES    "1"
#define BURN_NO     "2"
class CLocationInfo
{
public:
    CLocationInfo(void);
    ~CLocationInfo(void);
    CLocationInfo(const CLocationInfo &location);
    CLocationInfo & operator=(const CLocationInfo &videoLocationInfo);

    std::string GetID() const;
    void SetID(std::string val);

    std::string GetDescription() const;
    void SetDescription(std::string val);

    std::string GetBurnOrNot() const;
    void SetBurnOrNot(std::string val);

    std::string GetStreamType() const;
    void SetStreamType(std::string val);

    std::string GetURL() const;
    void SetURL(std::string val);

    std::vector<CSingleFileInfo>  & GetVecFileInfo();
    const std::vector<CSingleFileInfo>  & GetVecFileInfo() const;
    void SetVecFileInfo(std::vector<CSingleFileInfo> val);
private:
    std::string                     m_strID;
    std::string                     m_strDescription;
    std::string                     m_strBurnOrNot;//是否默认要刻录，非1：否，1：是
    std::string                     m_strStreamType;//视频流类型，具体值待确认????
    std::string                     m_strURL;//用于实时刻录
    std::vector<CSingleFileInfo>    m_vecFileInfo;//用于事后刻录
};


//////////////////////////////////////////////////////////////////////////
//规定笔录、播放器、播放列表文件合并成一个CLocationInfo，放在最后的位置，m_strID=-1
// 
#define DEV_SELECTED_YES    "1"
#define DEV_SELECTED_NO     "2"
class CCDROMDriverInfo
{
public:
    CCDROMDriverInfo(void);
    ~CCDROMDriverInfo(void);

    CCDROMDriverInfo(const CCDROMDriverInfo &CDROMDriverInfo);
    CCDROMDriverInfo & operator=(const CCDROMDriverInfo &CDROMDriverInfo);

    std::string GetID() const;
    void SetID(std::string val);

    std::string GetDescription() const;
    void SetDescription(std::string val);

    std::string GetIsSelected() const;
    void SetIsSelected(std::string val);

    std::vector<CLocationInfo>  & GetVecLocationInfo();
    const std::vector<CLocationInfo> & GetVecLocationInfo() const;
    void SetVecLocationInfo(std::vector<CLocationInfo> val);

#define DEV_STATE_AVAILABLE     "0"
#define DEV_STATE_BUSY          "1"
#define DEV_STATE_UNAVAILABLE   "-1"
    std::string GetStateFlag() const;
    void SetStateFlag(std::string val);

    std::string GetStateDescription() const;
    void SetStateDescription(std::string val);

    std::string GetJobID() const;
    void SetJobID(std::string val);

#define DEV_HAS_DISC_YES    "1"
#define DEV_HAS_DISC_NO     "0"
    std::string GetHasCD() const ;
    void SetHasCD(std::string val);

    std::string GetCDLeftCapacity() const;
    void SetCDLeftCapacity(std::string val);

    std::string GetCDTotalCapacity() const;
    void SetCDTotalCapacity(std::string val);
private:
    std::string                     m_strID;
    std::string                     m_strDescription;
    std::string                     m_strIsSelected;//用于界面逻辑，非1：未选择，1：被选择
    std::vector<CLocationInfo>      m_vecVideoLocationInfo;//刻录的视频画面列表
    std::string                     m_strStateFlag;//在界面显示时，1 正在被使用，0 可用；在刻录过程中表示具体刻录状态 -1不可用
    std::string                     m_strStateDescription;

    std::string                     m_strJobID;//光驱正在被哪一个任务使用
    std::string                     m_strHasCD;//是否插入光盘
    std::string                     m_strCDLeftCapacity;//光盘剩余容量
    std::string                     m_strCDTotalCapacity;//光盘总容量
};

//////////////////////////////////////////////////////////////////////////
class CPlaylistFileInfo
{
public:
    CPlaylistFileInfo();
    ~CPlaylistFileInfo();

    std::string GetDownloadURL() const;
    void SetDownloadURL(std::string val);

    std::string GetRemoteIP() const;
    void SetRemoteIP(std::string val);

    std::string GetRemotePort() const;
    void SetRemotePort(std::string val);
private:
    std::string m_strDownloadURL;
    std::string m_strRemoteIP;
    std::string m_strRemotePort;
};

//////////////////////////////////////////////////////////////////////////
class CPlayerInfo
{
public:
    CPlayerInfo(void);
    ~CPlayerInfo(void);

    std::string GetVersion() const;
    void SetVersion(std::string val);

    std::string GetDownloadURL() const;
    void SetDownloadURL(std::string val);

    std::string GetRemoteIP() const;
    void SetRemoteIP(std::string val);

    std::string GetRemotePort() const;
    void SetRemotePort(std::string val);
private:
    std::string m_strVersion;
    std::string m_strDownloadURL;
    std::string m_strRemoteIP;
    std::string m_strRemotePort;
};

//////////////////////////////////////////////////////////////////////////
class CPrintPictureInfo
{
public:
    CPrintPictureInfo();
    ~CPrintPictureInfo();

    std::string GetDownloadURL() const;
    void SetDownloadURL(std::string val);

    std::string GetRemoteIP() const;
    void SetRemoteIP(std::string val);

    std::string GetRemotePort() const;
    void SetRemotePort(std::string val);
private:
    std::string m_strDownloadURL;
    std::string m_strRemoteIP;
    std::string m_strRemotePort;
};

//////////////////////////////////////////////////////////////////////////

#define BURNSERVER_TYPE_NORMAL      "1"
#define BURNSERVER_TYPE_PRIMERA     "2"
#define BURNSERVER_TYPE_EPSON       "4"

#define BURNSERVER_AVAILABLE_YES    "1"
#define BURNSERVER_AVAILABLE_NO     "0"

//class CBurnServerInfoEx;
class CBurnServerInfo
{
public:
    CBurnServerInfo();
    ~CBurnServerInfo();

    CBurnServerInfo(const CBurnServerInfo &burnServerInfo);
    CBurnServerInfo & operator=(const CBurnServerInfo &burnServerInfo);

    std::string GetIP() const;
    void SetIP(std::string val);

    std::string GetPort() const;
    void SetPort(std::string val);

    std::string GetBurnServerName() const;
    void SetBurnServerName(std::string val);

    std::string GetDescription() const;
    void SetDescription(std::string val);

    const std::vector<CCDROMDriverInfo> & GetVecCDROMDriverInfo() const;
    std::vector<CCDROMDriverInfo> & GetVecCDROMDriverInfo();
    void SetVecCDROMInfo(const std::vector<CCDROMDriverInfo> &val);

    std::string GetBurnServerType() const;
    void SetBurnServerType(std::string val);

    std::string GetBurnServerTypeDescription() const;
    void SetBurnServerTypeDescription(std::string val);

    std::string GetIsDefault() const;
    void SetIsDefault(std::string val);

    std::string GetIsAvailable() const;
    void SetIsAvailable(std::string val);

protected:
    std::string             m_strIP;
    std::string             m_strPort;
    std::string             m_strBurnServerName;
    std::string             m_strDescription;
    std::vector<CCDROMDriverInfo> m_vecCDROMInfo;
    std::string             m_strBurnServerType;//刻录服务器类型，例如是否支持批量刻录 “1”普通刻录服务器，默认的；“2”派美雅刻录服务器；“4”爱普生刻录服务器
    std::string             m_strBurnServerTypeDescription;//刻录服务器类型描述信息
    std::string             m_strIsDefault;//是否作为默认的刻录服务器，在有多个刻录服务器，而某一项任务没有指定刻录服务器的情况下使用，0：否，1：是
    std::string             m_strIsAvailable;//是否可用，界面展示之前会检查各个刻录服务器的连通性,“0”否，“1”是
};

//////////////////////////////////////////////////////////////////////////
class CCommonBurnParam
{
public:
    CCommonBurnParam();
    ~CCommonBurnParam();

#define BURN_TYPE_SINGLE    "1"
#define BURN_TYPE_MULTISYN  "2"
#define BURN_TYPE_MULTIASYN "3"
    std::string GetBurnType() const;
    void SetBurnType(std::string val);

    std::string GetBurnPassword() const;
    void SetBurnPassword(std::string val);

    std::string GetContentPassword() const;
    void SetContentPassword(std::string val);

    std::string GetCDName() const;
    void SetCDName(std::string val);

private:
    std::string     m_strBurnType;//刻录方式，1：单盘刻录，2：多盘同刻，3：多盘续刻
    std::string     m_strBurnPassword;
    std::string     m_strContentPassword;
    std::string     m_strCDName;
};

//////////////////////////////////////////////////////////////////////////
#define JOB_FLAG_SAVE           "2"
#define JOB_FLAG_SAVE_AND_BURN  "1"
class CRTBurnInfo
{
public:
    CRTBurnInfo(void);
    ~CRTBurnInfo(void);

    CRTBurnInfo(const CRTBurnInfo &RTBurnInfo);
    CRTBurnInfo & operator=(const CRTBurnInfo &RTBurnInfo);

    std::string GetJobID() const;
    void SetJobID(std::string val);

    std::string GetJobType() const;
    void SetJobType(std::string val);

    const CBurnServerInfo & GetBurnServerInfo() const;
    CBurnServerInfo  & GetBurnServerInfo();
    void SetBurnServerInfo(const CBurnServerInfo &val);

    const CCommonBurnParam & GetCommonBurnParam() const;
    CCommonBurnParam & GetCommonBurnParam();
    void SetCommonBurnParam(const CCommonBurnParam &val) ;

    const std::vector<CLocationInfo> & GetVecVideoLocationInfo() const;
    std::vector<CLocationInfo> & GetVecVideoLocationInfo();
    void SetVecVideoLocationInfo(const std::vector<CLocationInfo> &val);

    std::string GetUpServerIP() const ;
    void SetUpServerIP(std::string val) ;

    std::string GetUpServerPort() const ;
    void SetUpServerPort(std::string val);

    std::string GetNewFileName() const;
    void SetNewFileName(std::string val);

    std::string GetJobFlag() const;
    void SetJobFlag(std::string val);

    std::string GetStateFlag() const;
    void SetStateFlag(std::string val);

    std::string GetStateDescription() const;
    void SetStateDescription(std::string val);

    const CPlayerInfo & GetPlayerInfo() const;
    CPlayerInfo & GetPlayerInfo();
    void SetPlayerInfo(CPlayerInfo val);

    std::string GetPlayListContent() const;
    void SetPlayListContent(std::string val);

    std::string GetNoteFileRemoteIP() const;
    void SetNoteFileRemoteIP(std::string val);

    std::string GetNoteFileRemotePort() const;
    void SetNoteFileRemotePort(std::string val);

    std::string GetNoteFileDownloadURL() const;
    void SetNoteFileDownloadURL(std::string val);

    const CPlaylistFileInfo & GetPlaylistFileInfo() const;
    CPlaylistFileInfo & GetPlaylistFileInfo();
    void SetPlaylistFileInfo(const CPlaylistFileInfo &val);

    std::string GetAutorunFileRemoteIP() const;
    void SetAutorunFileRemoteIP(std::string val);

    std::string GetAutorunFileRemotePort() const;
    void SetAutorunFileRemotePort(std::string val);

    std::string GetAutorunFilePath() const;
    void SetAutorunFilePath(std::string val);

    std::string GetHardDiskBackup() const;
    void SetHardDiskBackup(std::string val);

    std::string GetSpeed() const;
    void SetSpeed(std::string val);

    std::string GetCDAlarmLimit() const;
    void SetCDAlarmLimit(std::string val);

    std::string GetCDAutoFormat() const;
    void SetCDAutoFormat(std::string val);

    std::string GetBurnBufferSize() const;
    void SetBurnBufferSize(std::string val);

    std::string GetHardDiskBakAlarmLimit() const;
    void SetHardDiskBakAlarmLimit(std::string val);

    std::string GetStreamType() const;
    void SetStreamType(std::string val);

    std::string GetMd5() const;
    void SetMd5(std::string val);

protected:
    std::string                     m_strJobID;
    std::string                     m_strJobType;//发送此协议仅仅为了配置实时刻录参数"2"，还是配置刻录参数然后可以立即开始刻录"1"
    CBurnServerInfo                 m_BurnServerInfo;
    CCommonBurnParam                m_CommonBurnParam;//刻录方式、刻录密码设置、光盘名称，实时刻录协议仅仅用第二项
    std::vector<CLocationInfo>      m_vecLocationInfo;
    std::string                     m_strUpServerIP;
    std::string                     m_strUpServerPort;
    std::string                     m_strNewFileName;//TS文件重命名
    std::string                     m_strJobFlag;//是要保存配置，还是请求刻录
    std::string                     m_strStateFlag;//刻录状态
    std::string                     m_strStateDescription;
    CPlayerInfo                     m_PlayerInfo;
    std::string                     m_strPlayListContent;
    std::string                     m_strNoteFileRemoteIP;
    std::string                     m_strNoteFileRemotePort;
    std::string                     m_strNoteFilePath;
    std::string                     m_strAutorunFileRemoteIP;//向BurnServer传递的autorun.inf文件信息
    std::string                     m_strAutorunFileRemotePort;
    std::string                     m_strAutorunFilePath;
    CPlaylistFileInfo               m_PlaylistFileInfo;
    std::string                     m_strHardDiskBackup;
    std::string                     m_strSpeed;
    std::string                     m_strCDAlarmLimit;
    std::string                     m_strCDAutoFormat;
    std::string                     m_strBurnBufferSize;
    std::string                     m_strHardDiskBakAlarmLimit;
    std::string                     m_strStreamType;
    std::string                     m_strMd5;
};

//////////////////////////////////////////////////////////////////////////
class CNormalBurnJobInfo
{
public:
    CNormalBurnJobInfo();
    ~CNormalBurnJobInfo();

    CNormalBurnJobInfo(const CNormalBurnJobInfo &normalBurnJobInfo);
    CNormalBurnJobInfo & operator=(const CNormalBurnJobInfo &normalBurnJobInfo);

    std::string GetJobID() const;
    void SetJobID(std::string val);

    std::string GetJobName() const;
    void SetJobName(std::string val);

    std::string GetJobDescription() const;
    void SetJobDescription(std::string val);

    void CalculateJobSize();
    double GetJobSize() const;
    void SetJobSize(double val);

    const CBurnServerInfo & GetBurnServerInfo() const;
    CBurnServerInfo & GetBurnServerInfo();
    void SetBurnServerInfo(CBurnServerInfo val);

    const CCommonBurnParam & GetCommonBurnParam() const;
    CCommonBurnParam & GetCommonBurnParam();
    void SetCommonBurnParam(CCommonBurnParam val);

    const CPlayerInfo & GetPlayerInfo() const;
    CPlayerInfo & GetPlayerInfo();
    void SetPlayerInfo(CPlayerInfo val);

    std::string GetNoteFileRemoteIP() const;
    void SetNoteFileRemoteIP(std::string val);

    std::string GetNoteFileRemotePort() const;
    void SetNoteFileRemotePort(std::string val);

    std::string GetNoteRelativePath() const;
    void SetNoteRelativePath(std::string val);

    std::string GetNoteFileName() const;
    void SetNoteFileName(std::string val);

    bool GetBurnNoteFile() const;
    void SetBurnNoteFile(bool val);

    std::string GetAutorunFileRemoteIP() const;
    void SetAutorunFileRemoteIP(std::string val);

    std::string GetAutorunFileRemotePort() const;
    void SetAutorunFileRemotePort(std::string val);

    std::string GetAutorunFilePath() const;
    void SetAutorunFilePath(std::string val);

    std::string GetVideoFileRemoteIP() const;
    void SetVideoFileRemoteIP(std::string val);

    std::string GetVideoFileRemotePort() const;
    void SetVideoFileRemotePort(std::string val);

    std::string GetVideoFileRelativePath() const;
    void SetVideoFileRelativePath(std::string val);

    const std::vector<CLocationInfo> & GetVecLocationInfo() const;
    std::vector<CLocationInfo> & GetVecLocationInfo();
    void SetVecVideoLocationInfo(std::vector<CLocationInfo> val);

    std::string GetPlayListContent() const;
    void SetPlayListContent(std::string val);

    const std::map<std::string,std::string> & GetMapLabelFieldText() const;
    std::map<std::string,std::string> & GetMapLabelFieldText();
    void SetMapLabelFieldText(std::map<std::string,std::string> val);

    const CPlaylistFileInfo & GetPlaylistFileInfo() const;
    CPlaylistFileInfo & GetPlaylistFileInfo();
    void SetPlaylistFileInfo(CPlaylistFileInfo val);

    std::string GetUpServerIP() const;
    void SetUpServerIP(std::string val);

    std::string GetUpServerPort() const;
    void SetUpServerPort(std::string val);
protected:
    std::string                         m_strJobID;
    std::string                         m_strJobName;
    std::string                         m_strJobDescription;
    double                              m_dJobSize;

    CBurnServerInfo                     m_BurnServerInfo;
    CCommonBurnParam                    m_CommonBurnParam;
    CPlayerInfo                         m_PlayerInfo;

    std::string                         m_strNoteFileRemoteIP;//存放笔录文件的服务器地址
    std::string                         m_strNoteFileRemotePort;
    std::string                         m_strNoteRelativePath;
    std::string                         m_strNoteFileName;
    bool                                m_bBurnNoteFile;//COS事后刻录是否刻笔录文件

    std::string                         m_strAutorunFileRemoteIP;//向BurnServer传递的autorun.inf文件信息
    std::string                         m_strAutorunFileRemotePort;
    std::string                         m_strAutorunFilePath;
    std::string                         m_strVideoFileRemoteIP;//存放视频文件的服务器地址
    std::string                         m_strVideoFileRemotePort;
    std::string                         m_strVideoFileRelativePath;//视频文件相对路径
    std::vector<CLocationInfo>          m_vecVideoLocationInfo;
    std::string                         m_strPlayListContent;//用于生成playlist文件的信息
    std::map<std::string,std::string>   m_mapLabelFieldText;//封面打印字段
    CPlaylistFileInfo                   m_PlaylistFileInfo;//传给BurnServer的playlist文件信息
    std::string                         m_strUpServerIP;//将需要反馈的信息反馈给上层时使用的地址
    std::string                         m_strUpServerPort;
};

//////////////////////////////////////////////////////////////////////////
class CNormalBurnJobInfoEx : public CNormalBurnJobInfo//传给BurnServer的事后刻录附加参数
{
public:
    CNormalBurnJobInfoEx();
    ~CNormalBurnJobInfoEx();

    INT64 GetFileTotalSize() const;
    void SetFileTotalSize(INT64 val);
    INT64 GetFileDownloadSize() const;
    void SetFileDownloadSize(INT64 val);
private:
    std::string m_strCopies;
    std::string m_strDiscType;
    std::string m_strSpeed;
    std::string m_strCloseDisc;
    std::string m_strDeleteFile;

    INT64 m_nFileTotalSize;//当前任务需要下载的文件总大小
    INT64 m_nFileDownloadSize;//已经下载大小

};

//////////////////////////////////////////////////////////////////////////
#define BURN_FLAG_COS   "1"
#define BURN_FLAG_CMS   "2"
class CNormalBurnInfo
{
public:
    CNormalBurnInfo();
    ~CNormalBurnInfo();

    CNormalBurnInfo(const CNormalBurnInfo &normalBurnInfo);
    CNormalBurnInfo & operator=(const CNormalBurnInfo &normalBurnInfo);

    const std::vector<CBurnServerInfo> & GetVecBurnServerInfo() const;
    std::vector<CBurnServerInfo> & GetVecBurnServerInfo();
    void SetVecBurnServerInfo(std::vector<CBurnServerInfo> val);

    std::string  GetBurnFlag() const;
    void SetBurnFlag(std::string val);

    const CCommonBurnParam &  GetCommonBurnParam() const;
    CCommonBurnParam &  GetCommonBurnParam();
    void SetCommonBurnParam(CCommonBurnParam val);

    const CPlayerInfo &  GetPlayerInfo() const;
    CPlayerInfo &  GetPlayerInfo();
    void SetPlayerInfo(CPlayerInfo val);

    const std::vector<CNormalBurnJobInfoEx> & GetVecNormalBurnJobInfo() const;
    std::vector<CNormalBurnJobInfoEx> & GetVecNormalBurnJobInfo();
    void SetVecNormalBurnJobInfo(const std::vector<CNormalBurnJobInfoEx> &val);

    bool GetDefaultBurnServer(CBurnServerInfo &burnServerInfo);

private:
    std::vector<CBurnServerInfo> m_vecBurnServerInfo;//刻录服务器列表
    std::string m_strBurnFlag;
    CCommonBurnParam m_CommonBurnParam;//各任务通用参数，这里仅使用刻录方式、刻录密码
    CPlayerInfo m_PlayerInfo;
    std::vector<CNormalBurnJobInfoEx> m_vecNormalBurnJobInfoEx;//任务列表，其中每一个元素包含一项任务的所有信息
};

//一个刻录服务器在不同的刻录方式可以选择不同的光驱
class CBurnServerInfoEx : public CBurnServerInfo
{
public:
    explicit CBurnServerInfoEx(const CBurnServerInfo &burnServerInfo);
    CBurnServerInfoEx(const CBurnServerInfoEx &burnServerInfoEx);
    ~CBurnServerInfoEx();

    CBurnServerInfoEx & operator=(const CBurnServerInfoEx &);

    const std::vector<CCDROMDriverInfo> & GetVecCDROMInfo0() const;
    std::vector<CCDROMDriverInfo> & GetVecCDROMInfo0();
    void SetVecCDROMInfo0(std::vector<CCDROMDriverInfo> val);

    const std::vector<CCDROMDriverInfo> & GetVecCDROMInfo1() const;
    std::vector<CCDROMDriverInfo> & GetVecCDROMInfo1();
    void SetVecCDROMInfo1(std::vector<CCDROMDriverInfo> val);

    const std::vector<CCDROMDriverInfo> & GetVecCDROMInfo2() const;
    std::vector<CCDROMDriverInfo> & GetVecCDROMInfo2();
    void SetVecCDROMInfo2(std::vector<CCDROMDriverInfo> val);

    //vecCDROMDriverInfo包含从BurnServer获取的当前光驱状态信息
    void UpdateCDROMDriverState(const std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo);
private:
    CBurnServerInfoEx();

    //调整刻录方式和光驱选择的逻辑
    void AdjustDevSelectLogic();

    std::vector<CCDROMDriverInfo> m_vecDevSingle;
    std::vector<CCDROMDriverInfo> m_vecDevMultiSYN;
    std::vector<CCDROMDriverInfo> m_vecDevMultiASYN;
};