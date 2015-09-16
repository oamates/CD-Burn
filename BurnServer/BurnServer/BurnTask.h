#ifndef _BURN_TASK_H_
#define _BURN_TASK_H_

#include "UUID.h"
#include "FileUtil.h"
#include "DirectoryUtil.h"
#include "TaskModel.h"
#include "ConfigurableFile.h"

#include "SocketUtil.h"

#include <string>
#include <vector>
#include <map>

#include "Burn_SDK.h"
#include "Burn_Stream.h"

#include <stdio.h>
#include <string.h>

//#define TEST_RT_BURN_CHARSET


#ifdef WIN32
#else
#include <pthread.h>

// #define RT_BURN_DISK_BACKUP

#endif

//一个任务下载文件本地存放路径为DOWNLOAD_DIR目录下的一个文件夹，文件夹名字就是任务id
#ifdef WIN32
#define DOWNLOAD_DIR "C:\\BurnDownload\\"
#else
#define DOWNLOAD_DIR "/mnt/HD0/BurnDownload/"
#endif


typedef void (* BURNCALLBACK)(BURN_RUN_STATE_T *state,void *p);

#define TASK_PROPERTY_NORMAL        "5000"
#define TASK_PROPERTY_RT            "5001"
#define TASK_PROPERTY_RT_BACKUP     "5002"

typedef struct _BURN_STATE_PARAMETER_ 
{
    //strStateFlag //《刻录协议》被动类型接口2.7.4
#define STATE_SINGLE_DEV_ERR_JOB_FAILED     "-3"
#define STATE_SINGLE_DEV_ERR                "-2"
#define STATE_BURN_ERR                      "-1"
#define STATE_DEFAULT                       "0"
#define STATE_BURNING                       "1"
#define STATE_SINGLE_DISC_BURNED            "2"
#define STATE_DOWNLOADING                   "3"
#define STATE_WILL_CLOSE_DISC               "4"
#define STATE_CHANGE_DISC                   "5"
#define STATE_TASK_OVER                     "6"
#define STATE_JOB_OVER                      "7"
#define STATE_JOB_FINISHED                  "8"
#define STATE_RTJOB_PAUSED                  "9"
#define STATE_RTJOB_STOPPED                 "10"

#define STATE_RTTASK_PAUSED                 "11"
#define STATE_RTTASK_STOPPED                "12"
#define STATE_WILL_NO_SPACE                 "13"
#define STATE_RTTASK_STOPPING               "14"
    std::string strStateFlag;

#define EXTRA_STATE_RTBURN_BACKUP_NONE          "49"
#define EXTRA_STATE_RTBURN_BACKUP_YES           "50"
#define EXTRA_STATE_RTBURN_BACKUP_NO            "51"
#define EXTRA_STATE_RTBURN_BACKUP_RESTART_YES   "52"
#define EXTRA_STATE_RTBURN_BACKUP_RESTART_NO    "53"
    std::string strExtraStateFlag;

    std::string strStateDescription;

    std::string strTaskID;

    std::string strJobID;

    std::string strUpperIP;
    std::string strUpperPort;//参考BURN_PARAMETER

    std::string strTaskPropertyFlag;

    _BURN_STATE_PARAMETER_()
    {
        strStateFlag=STATE_DEFAULT;
        strExtraStateFlag=EXTRA_STATE_RTBURN_BACKUP_NONE;
        strStateDescription="";
        strTaskID="";
        strJobID="";
        strUpperIP="";
        strUpperPort="";
        strTaskPropertyFlag=TASK_PROPERTY_NORMAL;
    }
}BURN_STATE_PARAMETER;
typedef void (*BURN_TASK_CALLBACK)(DEV_HANDLE handle,BURN_STATE_PARAMETER state,void *pUsr);

typedef struct _BURN_DATA_SOURCE_
{
#define SOURCE_TYPE_NORMAL_FILE     "1"
#define SOURCE_TYPE_DIR             "2"
#define SOURCE_TYPE_URL             "3"
    std::string strType;//"1"：普通文件；"2"：目录；"3"：实时刻录url

    std::string strSourceUrl;

}BURN_DATA_SOURCE;


//用于多盘续刻时，每一个光驱关联文件的信息的回调
typedef std::map< DEV_HANDLE,std::vector<BURN_DATA_SOURCE> > MAP_DEV_BURN_FILES;
typedef struct _DEV_BURN_FILES_ 
{
    std::string strJobID;
    std::string strUpperIP;
    std::string strUpperPort;
    MAP_DEV_BURN_FILES mapDevBurnFiles;
}DEV_BURN_FILES;
typedef void (*DEV_BURN_FILE_CALLBACK)(const DEV_BURN_FILES &mapDevBurnFiles,void *pUsr);

//正在刻录的文件信息的回调
typedef void (*DEV_BURN_FILE_STATE_CALLBACK)(const DEV_BURN_FILES &mapDevBurnFiles,void *pUsr);

typedef struct _BURN_THREAD_PARAM_
{
    void *pBurnParam;
    void *pUsr;
}BURN_THREAD_PARAM;

typedef struct _BURN_PARAMETER_
{
    // 0：挂起状态
    // 1：准备就绪状态
    // 2：运行状态
    // 3：停止状态
    // 4：实时刻录刻录本地文件状态
    // 5：实时刻录已经开始刻录stream的状态
    // -1：异常状态
    // -2:废弃状态，表示此任务可以被删除
#define TASK_STATE_PAUSED           0
#define TASK_STATE_READY_TO_RUN     1
#define TASK_STATE_RUNNING          2
#define TASK_STATE_STOPPED          3
#define TASK_STATE_BURN_LOCAL       4
#define TASK_STATE_BURN_STREAM      5
#define TASK_STATE_WILL_CLOSE_DISC  6
#define TASK_STATE_ERR              -1
#define TASK_STATE_NO_USE           -2
#define TASK_STATE_UNKNOWN          -3

#ifdef RT_BURN_DISK_BACKUP
#define TASK_STATE_BURN_BACKUP_DRIFT    22
#define TASK_STATE_BURN_BACKUP_SLEEP    23
#define TASK_STATE_BURN_BACKUP_READY    24
#endif
    int nTaskRunningFlag;

    //保存当前的任务控制命令，因为刻录控制命令不是即刻被执行，所以增加此变量存储
    // 0 ：暂停
    // 1 ：开始或者恢复刻录
    // 3 ：停止
    // 
#define TASK_CONTROL_STATE_UNKNOWN              -2
#define TASK_CONTROL_STATE_DEFAULT              -1
#define TASK_CONTROL_STATE_PAUSE                0
#define TASK_CONTROL_STATE_START_OR_RESUME      1
#define TASK_CONTROL_STATE_STOP                 3
    int nTaskControlCmd;

    //保存当前任务的附加运行状态
    // -1 ：刻录任务默认状态
    //  0 ：多盘续刻任务没有激发下一光驱任务
    //  1 ：多盘续刻任务已经激发下一光驱任务
    // 

#define TASK_EXTRA_STATE_DEFAULT              -1
#define TASK_EXTRA_STATE_ASYN_SINGLE          0
#define TASK_EXTRA_STATE_ASYN_MULTI           1

    int nTaskExtraFlag;

    std::string strTaskID;

    std::string strLastTaskID;//触发该任务的任务ID

    std::vector<DEV_HANDLE> vecDevHandle;//任务线程需要使用的光驱列表

    int nIndex;//多盘续刻时正在使用哪一个光驱

    BURN_MODE burnMode;//实时刻录或者事后刻录

    // 任务需要刻录文件或者流，key表示不同画面
    // 如果key == "note"，表示笔录
    // 如果key == "player"，表示播放器
    // 如果key == "autorun"，表示自动运行文件
    // 如果key == "auth"，表示光盘访问控制文件
    // 如果key == "playlist"，表示播放列表文件
    std::map< std::string,std::vector<BURN_DATA_SOURCE> > mapBurnDataSource;

    std::string strJobID;

    std::string strDiscLabel;

#define NOTE_FILE_UPDATED_YES   "1"
#define NOTE_FILE_UPDATED_NO    "0"
    std::string strRtNoteUpdateFlag;//用于标记实时刻录是否已经更新好了笔录，"1" 是，非"1" 否

    std::string strUpperIP;
    std::string strUpperPort;//实时刻录补刻会遇到这样的情况，将要发送补刻消息时，对应的job已经被删除，从而找不到上层的地址

    std::string strTaskPropertyFlag;//标志刻录task的属性

#define DEV_FLAG_USED_YES   "0"
#define DEV_FLAG_USED_NO    "1"
    std::string strBurnDevFlag;//标记task有没有进入刻录的状态，如果有，那么调用获取光驱信息的接口被限制使用

    int nPauseCount;//暂停次数，用于命名暂停后新的ts文件

    std::string strNewFileName;
    unsigned long ulBufferSize;
    int nAlarmSize;

    _BURN_PARAMETER_()
    {
        nTaskRunningFlag=TASK_STATE_UNKNOWN;
        nTaskControlCmd=TASK_CONTROL_STATE_UNKNOWN;
        nTaskExtraFlag=TASK_EXTRA_STATE_DEFAULT;
        nIndex=-1;
        burnMode=MEDIAFILE_BURN_LOCAL_FILE;
        strRtNoteUpdateFlag=NOTE_FILE_UPDATED_NO;
        strTaskPropertyFlag=TASK_PROPERTY_NORMAL;
        strBurnDevFlag=DEV_FLAG_USED_NO;
        nPauseCount=0;

        strNewFileName="trial.ts";
        ulBufferSize=100;
        nAlarmSize=500;
    }
}BURN_PARAMETER;

extern ConfigurableFile gBurnStateCfgFile;

extern ConfigurableFile gBasicParamCfgFile;

bool IsFileOrDirExist(std::string str,bool bFolder);
INT64 GetFileOrDirSize(std::string str,bool bFolder);

std::string RenameFile(const char *pOldFilePath,const char *pNewFileNameNoExtent,const char *pExtent=NULL);

class BurnTask : public ZOSThread
{
public:
    BurnTask();
    ~BurnTask();

    BOOL    Create();
    BOOL    Close();

    virtual BOOL OnThreadStart();
    virtual BOOL OnThreadEntry();
    virtual BOOL OnThreadStop();

    BOOL AddTask(const BURN_PARAMETER &param);


    //处理刻录任务的线程单位,包含单个光驱刻录过程的逻辑处理
    void HandleTask();

    //将刻录备份task信息写入硬盘，用于重启机器后还可以补刻
    void SaveRTBurnBackupTask(BURN_PARAMETER burnParam);

    void SetBurnTaskCallBack(BURN_TASK_CALLBACK cb,void *p);

    void SetDevBurnFilesCallBack(DEV_BURN_FILE_CALLBACK cb,void *p);

    bool IsDevReady(DEV_HANDLE handle,BURN_PARAMETER burnParam,BURN_MODE mode);

    //处理单个刻录任务
    bool BurnDisc(BURN_PARAMETER param);

    void CleanTask(std::string strTaskID);

    //清除一个实时刻录task的伴随的刻录备份task
    void CleanRTBurnBackupTask(std::string strFatherTaskID);

    //用于控制实时刻录的流程，当笔录更新完之后才可以停止实时刻录，更新笔录对于同刻
    //的多个task适用
    // 
    void SetNoteUpdateFlag(std::string strJobID,std::string strFlag);
    std::string GetNoteUpdateFlag(std::string strTaskID);

    std::string GetBurningTaskID(DEV_HANDLE handle);

    std::string GetBurningJobID(DEV_HANDLE handle);

    void SetNoteUpdateFlagByTaskID(std::string strTaskID,std::string strFlag);

    //返回值 : 0 正常操作，等待每一个task返回结果；1没有找到task，表示指定job暂停成功
    int PauseRTTasksByJobID(std::string strJobID);

    //返回值 : 0 正常操作，等待每一个task返回结果；1没有找到task，表示指定job停止成功
    int StopRTJob(std::string strJobID);

    //返回值 : 0 正常操作，已经挂起的task恢复运行；1没有找到task，需要重新生成刻录task；-1表示task正在运行
    int ResumeRTJob(std::string strJobID);

    //获得刻录当前的控制状态，用于判断是否继续刻录
    int GetTaskControlState(std::string strRtTaskID);

    void SetTaskControlState(std::string strRtTaskID,int nState);

    void SetTaskRunningState(std::string strRtTaskID,int nState);

    void SetTaskPauseCount(std::string strRtTaskID,int nCount);

    void SetRTBurnBackupTaskRunningState(std::string strFatherTaskID,int nState);

    int GetTaskRunningState(std::string strRtTaskID);

    int GetTaskExtraFlag(std::string strTaskID);

    void SetTaskExtraFlag(std::string strTaskID, int nState);

    void ActivateRTBurnBackupTasks(std::string strJobID);
    void CancelRTBurnBackupTasks(std::string strJobID);
    void StartRTBurnBackupTasks(std::string strJobID);

    bool HasRTBurnBackupTask(std::string strJobID);

    //判断指定的光驱有没有正在被使用，因为一个光驱正在刻录时，获取光驱信息的相关接口被限制调用，
    //所以在调用这些接口之前作此判断
    bool GetDevUsed(DEV_HANDLE handle);
    void SetDevUsed(DEV_HANDLE handle,std::string strBurnDevFlag);

    void GetBurningFiles(DEV_HANDLE handle,std::vector<std::string> &vecFiles);

    //找到第一个未使用的光驱
    int GetFirstUnusedDev(std::vector<DEV_HANDLE> vectDev);

    //是否存在strJobID对应的任务
    bool IsTaskExsitByJobID(std::string strJobID);

    //光盘刻录限定大小
    static INT64 GetLimitSize();
    static void SetLimitSize(INT64 val);

    bool IsTaskCanCloseDisc(std::string strJobID, std::string strTaskID);

    //将路径转换为当前系统(win32或linux)下的格式，如果是文件夹，保证最后加上"\\"或"/"
    static std::string Path(std::string strPath,bool bIsFolder);

    static bool InsertPlayListFile(std::vector<BURN_DATA_SOURCE> &vecDataSource,std::string strFullPlaylistFilePath);

    //返回新建的playlist文件全路径
    static std::string CreatePlayListFile(const std::vector<BURN_DATA_SOURCE> &vecDataSource,
        std::string strFullPlaylistFilePath);

private:

    static int BurnCoreCallBack(DEV_HANDLE hDEV,const BURN_RUN_STATE_T *state,void *p);

    void RTBurnExtraData(DEV_HANDLE handle,const std::map< std::string,std::vector<BURN_DATA_SOURCE> > &mapData);

    void ResetTask(std::string strTaskID);

    bool GetDiscInformation(DEV_HANDLE handle,BURN_DISC_INFO_T &discInfo);

    bool GetAddrFromURL(const std::string &strURL,std::string &strIP,std::string &strPort);

    bool CheckStreamConnectable(const std::vector<BURN_DATA_SOURCE> &vecStreamSource);

    void UpdateRtTaskPlaylist(std::string strTaskID,int nPauseCount);

    //单张光盘单次刻录过程实现
    bool StartSpecificDeviceToBurn(DEV_HANDLE hDev,std::vector<BURN_DATA_SOURCE> vecDataSource,
        BURN_MODE mode,BURN_PARAMETER burnParam);

    void PauseRTBurnTask(std::string strTaskID);

    void StopRTBurnTask(std::string strTaskID);

    void ResumeRTBurnTask(std::string strTaskID);

    void SetCurDev(std::string strTaskID,int nIndex);

    DEV_HANDLE GetCurDev(std::string strTaskID);

    void ListLeftTasks();

    typedef struct _DISK_BACKUP_PARAM_
    {
        DEV_HANDLE handle;
        BURN_BOOL useDev;
        int backupSize;
        int alarmSize;
        std::string strDiskPath;
        std::string strBackupFileName;
    }DISK_BACKUP_PARAM;
    bool StartDiskBackup(const DISK_BACKUP_PARAM &param);
    bool StopDiskBackup(DEV_HANDLE handle);

    static std::string IntToString(int nValue);


private:
    std::vector<BURN_PARAMETER> m_vecBurnParameter;
    ZOSMutex m_mutexBurnTaskParameter;

    BURN_TASK_CALLBACK m_cb;
    void *m_pUsr;

    DEV_BURN_FILE_CALLBACK m_cbDevBurnFiles;
    void *m_pUsrDevBurnFiles;

    static INT64 m_nLimitSize;//光盘刻录限定大小
};

#endif
