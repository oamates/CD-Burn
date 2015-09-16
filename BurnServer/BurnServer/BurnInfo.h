#ifndef _BURN_INFO_H_
#define _BURN_INFO_H_

#include <vector>
#include <string>
#include <map>

#include "FileUtil.h"

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

    std::string GetBurnState() const;
    void SetBurnState(std::string val);

    std::string GetSize() const;
    void SetSize(std::string val);

    std::string GetDownloadPercent() const;
    void SetDownloadPercent(std::string val);
private:
    std::string     m_strFileName;//全路径
    std::string     m_strNewFileName;//用于重命名
    std::string     m_strBurnState;//刻录状态，0：等待刻录，1：正在刻录，2：刻录完毕
    std::string     m_strSize;
    std::string     m_strDownloadPercent;//下载进度
};

//////////////////////////////////////////////////////////////////////////
class CLocationInfo
{
public:
    CLocationInfo(void);
    ~CLocationInfo(void);
    CLocationInfo(const CLocationInfo &videoLocationInfo);
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
    std::string                 m_strID;
    std::string                 m_strDescription;
    std::string                 m_strBurnOrNot;//是否默认要刻录，0：否，1：是
    std::string                 m_strStreamType;//视频流类型，具体值待确认????
    std::string                 m_strURL;//用于实时刻录
    std::vector<CSingleFileInfo> m_vecFileInfo;//用于事后刻录
};


//////////////////////////////////////////////////////////////////////////
//规定笔录、播放器、播放列表文件合并成一个CLocationInfo，放在最后的位置，m_strID=-1
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

    std::string GetStateFlag() const;
    void SetStateFlag(std::string val);

    std::string GetStateDescription() const;
    void SetStateDescription(std::string val);

    std::string GetJobID() const;
    void SetJobID(std::string val);

    std::string GetHasCD() const ;
    void SetHasCD(std::string val);

    std::string GetCDLeftCapacity() const;
    void SetCDLeftCapacity(std::string val);

    std::string GetCDTotalCapacity() const;
    void SetCDTotalCapacity(std::string val);
private:
    std::string                     m_strID;
    std::string                     m_strDescription;
    std::string                     m_strIsSelected;//用于界面逻辑，：2未选择，1：被选择
    std::vector<CLocationInfo>      m_vecVideoLocationInfo;//刻录的视频画面列表
    std::string                     m_strStateFlag;//《刻录协议》被动类型接口2.7.4
    std::string                     m_strStateDescription;

    std::string                     m_strJobID;//光驱正在被哪一个任务使用

#define DEV_HAS_CD_YES  "1"
#define DEV_HAS_CD_NO   "0"
    std::string                     m_strHasCD;//是否插入光盘，1：是，非1：否

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
    void SetVecCDROMInfo(std::vector<CCDROMDriverInfo> val);

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

    std::string GetBurnType() const;
    void SetBurnType(std::string val);

    std::string GetBurnPassword() const;
    void SetBurnPassword(std::string val);

    std::string GetContentPassword() const;
    void SetContentPassword(std::string val);

    std::string GetCDName() const;
    void SetCDName(std::string val);

private:
#define BURN_TYPE_SINGLE        "1"
#define BURN_TYPE_MULTI_SYN     "2"
#define BURN_TYPE_MULTI_ASYN    "3"
    std::string     m_strBurnType;

    std::string     m_strBurnPassword;
    std::string     m_strContentPassword;
    std::string     m_strCDName;
};

//////////////////////////////////////////////////////////////////////////
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
    void SetBurnServerInfo(CBurnServerInfo val);

    const CCommonBurnParam & GetCommonBurnParam() const;
    CCommonBurnParam & GetCommonBurnParam();
    void SetCommonBurnParam(CCommonBurnParam val) ;

    const std::vector<CLocationInfo> & GetVecVideoLocationInfo() const;
    std::vector<CLocationInfo> & GetVecVideoLocationInfo();
    void SetVecVideoLocationInfo(std::vector<CLocationInfo> val);

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
    void SetPlaylistFileInfo(CPlaylistFileInfo val);

    std::string GetViewCount() const;
    void SetViewCount(std::string val);

    std::string GetAutorunFileRemoteIP() const;
    void SetAutorunFileRemoteIP(std::string val);

    std::string GetAutorunFileRemotePort() const;
    void SetAutorunFileRemotePort(std::string val);

    std::string GetAutorunFilePath() const;
    void SetAutorunFilePath(std::string val);
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
    std::string                     m_strStateFlag;
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
    std::string                     m_strViewCount;
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

    std::string GetViewCount() const;
    void SetViewCount(std::string val);

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

    std::string GetStateFlag() const;
    void SetStateFlag(std::string val);

    std::string GetStateDescription() const;
    void SetStateDescription(std::string val);
protected:
    std::string                         m_strJobID;
    std::string                         m_strJobName;
    std::string                         m_strJobDescription;
    CBurnServerInfo                     m_BurnServerInfo;
    CCommonBurnParam                    m_CommonBurnParam;
    CPlayerInfo                         m_PlayerInfo;
    std::string                         m_strNoteFileRemoteIP;//存放笔录文件的服务器地址
    std::string                         m_strNoteFileRemotePort;
    std::string                         m_strNoteRelativePath;
    std::string                         m_strNoteFileName;
    std::string                         m_strAutorunFileRemoteIP;//向BurnServer传递的autorun.inf文件信息
    std::string                         m_strAutorunFileRemotePort;
    std::string                         m_strAutorunFilePath;
    std::string                         m_strVideoFileRemoteIP;//存放视频文件的服务器地址
    std::string                         m_strVideoFileRemotePort;
    std::string                         m_strVideoFileRelativePath;//视频文件相对路径
    std::vector<CLocationInfo>          m_vecVideoLocationInfo;
    std::string                         m_strPlayListContent;//用于生成playlist文件的信息
    std::string                         m_strViewCount;
    std::map<std::string,std::string>   m_mapLabelFieldText;//封面打印字段
    CPlaylistFileInfo                   m_PlaylistFileInfo;//传给BurnServer的playlist文件信息
    std::string                         m_strUpServerIP;//将需要反馈的信息反馈给上层时使用的地址
    std::string                         m_strUpServerPort;

    std::string                         m_strStateFlag;
    std::string                         m_strStateDescription;

};

//
//记录每一个文件的下载状态，first表示下载任务id,second存放FILE_DOWNLOAD_INFO
typedef struct _FILE_DOWNLOAD_INFO
{
    std::string strJobID;

#define TYPE_NOTE_FILE  "0"
#define TYPE_PLAYER     "1"
#define TYPE_PLAY_LIST  "2"
#define TYPE_VIDEO      "3"
#define TYPE_AUTORUN    "4"
#define TYPE_AUTH       "5"
    std::string strType;//0：笔录，1：播放器，2：playlist，3：视频文件，4：autorun文件，5：auth文件

    std::string strFileName;//
    std::string strOldPercentage;
    std::string strPercentage;//如果为"-1",表示下载异常
    std::string strCDROMDriverID;//光驱id，如果为"ALL"，表示属于所有光驱
    INT64 nFileTotalSize;
}FILE_DOWNLOAD_INFO;

//////////////////////////////////////////////////////////////////////////
class CNormalBurnJobInfoEx : public CNormalBurnJobInfo//传给BurnServer的事后刻录附加参数
{
public:
    CNormalBurnJobInfoEx();
    ~CNormalBurnJobInfoEx();

    CNormalBurnJobInfoEx(const CNormalBurnJobInfoEx &normalBurnJobInfoEx);
    CNormalBurnJobInfoEx & operator=(const CNormalBurnJobInfoEx &normalBurnJobInfoEx);

    INT64 GetFileTotalSize() const;
    void SetFileTotalSize(INT64 val);

    INT64 GetFileDownloadSize() const;
    void SetFileDownloadSize(INT64 val);

    int GetTotalCount() const;
    void SetTotalCount(int val);

    int GetDownloadCount() const;
    void SetDownloadCount(int val);

    std::string GetPrimeraJobID() const;
    void SetPrimeraJobID(std::string val);

    const std::map<std::string,FILE_DOWNLOAD_INFO> & GetMapFileDownloadState() const;
    std::map<std::string,FILE_DOWNLOAD_INFO> & GetMapFileDownloadState();
    void SetMapFileDownloadState(const std::map<std::string,FILE_DOWNLOAD_INFO> &val);

    const std::map<std::string,std::string> & GetMapBurnTask() const;
    std::map<std::string,std::string> & GetMapBurnTask();
    void SetMapBurnTask(const std::map<std::string,std::string> &val);
private:
    std::string m_strCopies;
    std::string m_strDiscType;
    std::string m_strSpeed;
    std::string m_strCloseDisc;
    std::string m_strDeleteFile;

    INT64 m_nFileTotalSize;//当前任务需要下载的文件总大小
    INT64 m_nFileDownloadSize;//已经下载大小

    int m_nTotalCount;
    int m_nDownloadCount;

    std::map<std::string,FILE_DOWNLOAD_INFO> m_mapFileDownloadState;

    std::map<std::string,std::string> m_mapBurnTask;//一个刻录任务可能分为多个task，通过增加这个成员变量来记录任务的整体运行状态

    //单纯的任务id在派美雅获取任务状态时会出问题，当重复执行同一个任务时，
    //获取的可能是上一次执行的状态
    std::string m_strPrimeraJobID;

};

//////////////////////////////////////////////////////////////////////////
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
    void SetVecNormalBurnJobInfo(std::vector<CNormalBurnJobInfoEx> val);

    bool GetDefaultBurnServer(CBurnServerInfo &burnServerInfo);

private:
    std::vector<CBurnServerInfo> m_vecBurnServerInfo;//刻录服务器列表
    std::string m_strBurnFlag;//刻录标记，用于区分调用环境。0：CMS事后刻录，1：COS事后刻录
    CCommonBurnParam m_CommonBurnParam;//各任务通用参数，这里仅使用刻录方式、刻录密码
    CPlayerInfo m_PlayerInfo;
    std::vector<CNormalBurnJobInfoEx> m_vecNormalBurnJobInfoEx;//任务列表，其中每一个元素包含一项任务的所有信息
};

//传给BurnServer的实时刻录附加参数
#define RT_CONTROL_STATE_PAUSE      "9"
#define RT_CONTROL_STATE_STOP       "10"
#define RT_CONTROL_STATE_START      "11"
#define RT_CONTROL_STATE_UNKNOWN    "0"
class CRTBurnInfoEx : public CRTBurnInfo
{
public:
    CRTBurnInfoEx();
    ~CRTBurnInfoEx();

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

    const std::map<std::string,std::string> & GetMapBurnTask() const;
    std::map<std::string,std::string> & GetMapBurnTask();
    void SetMapBurnTask(const std::map<std::string,std::string> &val);

    const std::map<std::string,FILE_DOWNLOAD_INFO> & GetMapFileDownloadState() const;
    std::map<std::string,FILE_DOWNLOAD_INFO> & GetMapFileDownloadState();
    void SetMapFileDownloadState(const std::map<std::string,FILE_DOWNLOAD_INFO> &val);

    int GetTotalCount() const;
    void SetTotalCount(int val);

    int GetDownloadCount() const;
    void SetDownloadCount(int val);

    std::string GetControlState() const;
    void SetControlState(std::string val);
private:
    std::string m_strHardDiskBackup;
    std::string m_strSpeed;
    std::string m_strCDAlarmLimit;
    std::string m_strCDAutoFormat;
    std::string m_strBurnBufferSize;
    std::string m_strHardDiskBakAlarmLimit;
    std::string m_strStreamType;
    std::string m_strMd5;

    std::map<std::string,std::string> m_mapBurnTask;

    std::map<std::string,FILE_DOWNLOAD_INFO> m_mapFileDownloadState;

    int m_nTotalCount;
    int m_nDownloadCount;

    std::string m_strControlState;//用于标记实时刻录控制状态
};
#endif
