#ifndef _BURN_SERVER_AGENT_
#define _BURN_SERVER_AGENT_

#include <string>
#include <vector>
#include <map>

#include "FileAnywhereObserver.h"

#ifdef WIN32
#else
#define NDEBUG
#endif
#include "tinyxml.h"

#include "UUID.h"
#include "DirectoryUtil.h"
#include "FileUtil.h"
#include "FileAnywhereManager.h"
#include "CommonUtil.h"
#include "ConfigurableFile.h"

#include "BurnInfo.h"

#include "BurnTask.h"
#include "BurnSendStateTask.h"

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#else
#include <pthread.h>
#endif

#ifdef WIN32
#define BURN_SERVER_CFG_DIR "C:\\CS\\BurnServer\\"
#else
#endif

extern ConfigurableFile gBurnStateCfgFile;

extern ConfigurableFile gBasicParamCfgFile;

class CBurnServerEPSONCtrl;

class BurnServerAgent : public FileAnywhereObserver
{
public:
    BurnServerAgent();
    ~BurnServerAgent();

    //初始化，如获取光驱信息
    void Init();

    void UnInit();

    void ProccessProtocol(std::string strProtocolContent);

    //发送光驱状态信息（整体状态），不包括光驱刻录的文件信息（细节状态），也就是m_vecCDROMDriver包含的信息
    void SendCDROMDriverInfo(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam);

    void SendBurnServerInfo(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam);

    static std::string GetCurDir();

    static void OnRecvProtocol(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam,
        void *pUsrParam);//服务器接收到协议后，进入此回调

    bool GetJobUpperAddress(std::string strJobID,std::string &strIP,std::string &strPort);

    bool IsJobExist(std::string strJobID);//判断任务是否正在进行

    bool GetRtJobInfo(std::string strJobID, CRTBurnInfoEx &RTBurnInfoEx);

    bool IsPrimeraEnvironmentOK();

    bool IsEPSONEnvironmentOK();

    //判断相关服务是否正常
    bool IsBurnEnvironmentOK(std::string strBurnServerType);

    //判断给定的光驱列表是否可用,如果可用，那么锁定光驱
    bool IsCDROMDriverAvailable(std::string strJobID);

    bool IsCDROMDriverExist(std::string strCDROMID);
    bool IsCDROMDriverIdle(std::string strCDROMID);

    void StartRTBurn(const CRTBurnInfoEx &RTBurnInfoEx, std::string strLastTaskID = "");//调用底层接口，开始实时刻录
    void StartNormalBurn(const CNormalBurnJobInfoEx &normalJob);//调用底层接口，开始事后刻录

#define JOB_TYPE_RT         "2"
#define JOB_TYPE_NORMAL     "1"
#define JOB_TYPE_UNKNOWN    "-1"
    std::string GetJobType(std::string strJobID);

    //获取实时刻录控制状态，用于判断是否该清理一个实时刻录任务
    std::string GetRtJobControlState(std::string strJobID);
    void SetRtJobControlState(std::string strJobID,std::string strControlState);

    void DeleteRTJobDownloadTasks(std::string strJobID);

    void PauseRTBurnJob(std::string strJobID);
    void ResumeRTburn(std::string strJobID);
    void StopRTBurn(std::string strJobID);

    static std::string ExtractFileOrDirName(std::string strPath);

    //发送下载单个文件或文件夹的协议，返回任务id
    std::string DownloadSingleFileOrDir(std::string strIP,std::string strPort,
        std::string strRemotePath,std::string strLocalRootDir,std::string strNewFileName,bool bIsFolder);

    bool CopyLocalFileOrDir(const CNormalBurnJobInfoEx &normalJob);//如果所有数据都是本地的，那么返回true

    bool CopyLocalFileOrDir(const CRTBurnInfoEx &rtJob);

    //事后刻录，下载需要刻录的文件，如果发现是本地文件或者文件夹，那么执行复制操作
    void DownloadFile(std::string strJobID);

    //nFlag：0  下载播放器、playlist、autorun笔录；1 仅更新笔录,如果是本地的，那么复制
#define RT_DOWNLOAD_FLAG_ONLY_NOTE_FILE    1
#define RT_DOWNLOAD_FLAG_NO_NOTE_FILE      0
    void DownloadFile(std::string strJobID,int nFlag);

    void DeleteTransmissionTask(std::string strTransID);

    //获取任务需要下载的文件的信息，如果有一个文件，无法获取它的大小，函数返回-1；此函数的另一个作用是判断下载环境是否准备好
    bool GetJobFilesInformation(std::string strJobID);

    //保证文件夹路径最后有分隔符
    std::string FixDirPath(std::string strDir);

    void AttachPrimeraJobID(std::string strOrgJobID,std::string strPriJobID);

    std::string GetPrimeraJobID(std::string strOrgJobID);

    //主动获取刻录状态，目前仅用于第三方刻录
    bool GetJobState(std::string strJobID,std::string &strStateFlag,std::string &stateDescription);

    const std::vector<CNormalBurnJobInfoEx> & GetNormalJobInfoVec() const;
    std::vector<CNormalBurnJobInfoEx> & GetNormalJobInfoVec();

    const std::vector<CRTBurnInfoEx> & GetRTJobInfoVec() const;
    std::vector<CRTBurnInfoEx> & GetRTJobInfoVec();

    //刻录完成后，删除任务，解除光驱占用
    void CleanJob(std::string strJobID);

    void CleanJobData(std::string strJobID);

    void CleanRTJobBackupData(std::string strJobID);

    void ListLeftJobs();

    void SaveJob(std::string strJobID);

    void CleanRTBurnBackupXML(std::string strJobID);

    void ReadRTBurnBackupXML();

    void ClearBackupBurnTasks(std::string strFatherTaskID);

    static std::string UTF8ToGB18030(std::string strSrc);

private:
    virtual int FileAnywhere_FeedBack(FileAnywhereObserverParameter parameter);

    //获取文件夹占用空间大小
    INT64 GetFileListSize(std::string strIP,std::string strPort,std::string strRemotePath);

    INT64 GetFileSize(std::string strIP,std::string strPort,std::string strRemotePath);

    //更新任务下载进度，如果已经下载完成，那么可以开始刻录
    //当前以任务为单位统计下载进度，以后可扩展为以光驱为单位
    //strPercent："-1"表示下载失败，否则表示正常下载进度
    void UpdateNormalJobDownloadState(std::string strUUID,std::string strPercent="-1");

    void UpdateRtJobDownloadState(std::string strUUID,std::string strPercent="-1");

    //将当前的刻录状态（光驱状态、刻录文件状态等）发送给上层，发送的信息包含在每一个任务的光驱信息中
    //函数的３个参数简要描述了任务的状态，对于第三方刻录和普通刻录都适用
    //bDevInfo:是否传送光驱信息
    void SendCurrentBurnStateToUpper(std::string strUpperIP,std::string strUpperPort,
        std::string strJobID,std::string strState,std::string strStateDescription,
        bool bDevInfo=true);

    bool IsDevBeingUsed(DEV_HANDLE handle);

    //调用底层接口，将BurnServer控制的光驱信息存放在成员m_vecCDROMDriver中
    void UpdateDevInfo();

    std::string GetHostIP();

    static void BurnCallBack(DEV_HANDLE handle,BURN_STATE_PARAMETER state,void *pUsr);//BurnTask的回调函数

    //多盘续刻，每个光驱关联文件的回调
    static void BurnFilesCallBack(const DEV_BURN_FILES &devBurnFiles,void *pUsr);

#define BURN_FILE_STATE_WAIT    "0"
#define BURN_FILE_STATE_BURNING "1"
#define BURN_FILE_STATE_DONE    "2"
    void SetWinJobBurnFileState(std::string strJobID,std::string strDevID,std::string strState);

    void DealWithCallBackMsg(DEV_HANDLE handle,BURN_STATE_PARAMETER state);

    bool GetCDROMDriverInformationFromHandle(DEV_HANDLE h,CCDROMDriverInfo &info);//通过设备句柄获得光驱对象
    void SetCDROMDriverInformation(DEV_HANDLE h,const CCDROMDriverInfo &info);//设置指定光驱的信息，便于在刻录过程中实时更新光驱状态或者初始化时添加新的光驱
    DEV_HANDLE GetDeviceHandle(std::string strID);

    void PushExtraDataSource(const CNormalBurnJobInfoEx &normalJobInfo,std::map< std::string,std::vector<BURN_DATA_SOURCE> > &mapBurnDataSource);

    void PushExtraDataSource(const CRTBurnInfoEx &rtJobInfo,std::map< std::string,std::vector<BURN_DATA_SOURCE> > &mapBurnDataSource);

    //设置指定job的指定task的状态信息
    void SetTaskState(std::string strJobID,std::string strTaskID,std::string strStateFlag,std::string strStateDescription);

    //设置指定光驱的状态信息
    bool SetDevBurnState(DEV_HANDLE handle,std::string strStateFlag,std::string strStateDescription);

    bool AddTask(const BURN_PARAMETER &param);

    void ConfirmRTBurnBackup(std::string strJobID);
    void CancelRTBurnBackup(std::string strJobID);
    void StartRTBurnBackup(std::string strJobID);

    /*
    *判断一个job的所有task的执行情况
    *返回值
    *       -1：刻录失败，所有task执行失败
    *       1 正在运行（至少有一个task线程没有退出，可能其中一个task执行失败），如果获取任务状态失败，则认为任务正在运行
    *       2：刻录完成，但是不是所有task执行成功
    *       7：job被成功执行
    */
    int IsJobDone(std::string strJobID);

    bool IsJobRunning(std::string strJobID);

    bool IsJobPaused(std::string strJobID);

    bool IsJobStopped(std::string strJobID);

    bool IsJobStopping(std::string strJobID);

    void AddDevBurnFilesInfo(const DEV_BURN_FILES &devBurnFiles);

    //更新与每个光驱关联的需要被刻录的文件信息
    void AddDevBurnFilesInfo(std::string strJobID,std::string strDevID,
        const std::vector<CLocationInfo> &vecLocationInfo);

    std::vector<CNormalBurnJobInfoEx> m_vecNormalJob;//当前正在进行的普通刻录任务列表
    std::vector<CRTBurnInfoEx> m_vecRTBurnInfo;//当前正在进行的实时刻录任务列表

    ZOSMutex m_mutexJobVec;

    //BurnServer可控制的光驱列表，包含每一个光驱的状态信息，不包括刻录的文件信息，并且实时更新每一个光驱的状态！！！！
    std::vector<CCDROMDriverInfo> m_vecDev;

    ZOSMutex m_mutexDevVec;

    std::map<DEV_HANDLE,std::string> m_mapDev;//key：光驱设备句柄；value：光驱id

    typedef struct _NCXSERVER_PARAM
    {
        NCXSERVERHANDLE hNCXServer;
        NCXServerCBParam *pCBParam;
    }NCXSERVER_PARAM;
    std::vector<NCXSERVER_PARAM> m_vecNCXServerParam;

#ifdef WIN32
    CBurnServerEPSONCtrl *m_pEPSONCtrl;
    bool m_bIsEPSONEnvOK;
#endif

    BurnTask m_BurnTask;
    BurnSendStateTask m_BurnSendStateTask;
};
#endif
