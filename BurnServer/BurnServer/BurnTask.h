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

//һ�����������ļ����ش��·��ΪDOWNLOAD_DIRĿ¼�µ�һ���ļ��У��ļ������־�������id
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
    //strStateFlag //����¼Э�顷�������ͽӿ�2.7.4
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
    std::string strUpperPort;//�ο�BURN_PARAMETER

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
    std::string strType;//"1"����ͨ�ļ���"2"��Ŀ¼��"3"��ʵʱ��¼url

    std::string strSourceUrl;

}BURN_DATA_SOURCE;


//���ڶ�������ʱ��ÿһ�����������ļ�����Ϣ�Ļص�
typedef std::map< DEV_HANDLE,std::vector<BURN_DATA_SOURCE> > MAP_DEV_BURN_FILES;
typedef struct _DEV_BURN_FILES_ 
{
    std::string strJobID;
    std::string strUpperIP;
    std::string strUpperPort;
    MAP_DEV_BURN_FILES mapDevBurnFiles;
}DEV_BURN_FILES;
typedef void (*DEV_BURN_FILE_CALLBACK)(const DEV_BURN_FILES &mapDevBurnFiles,void *pUsr);

//���ڿ�¼���ļ���Ϣ�Ļص�
typedef void (*DEV_BURN_FILE_STATE_CALLBACK)(const DEV_BURN_FILES &mapDevBurnFiles,void *pUsr);

typedef struct _BURN_THREAD_PARAM_
{
    void *pBurnParam;
    void *pUsr;
}BURN_THREAD_PARAM;

typedef struct _BURN_PARAMETER_
{
    // 0������״̬
    // 1��׼������״̬
    // 2������״̬
    // 3��ֹͣ״̬
    // 4��ʵʱ��¼��¼�����ļ�״̬
    // 5��ʵʱ��¼�Ѿ���ʼ��¼stream��״̬
    // -1���쳣״̬
    // -2:����״̬����ʾ��������Ա�ɾ��
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

    //���浱ǰ��������������Ϊ��¼��������Ǽ��̱�ִ�У��������Ӵ˱����洢
    // 0 ����ͣ
    // 1 ����ʼ���߻ָ���¼
    // 3 ��ֹͣ
    // 
#define TASK_CONTROL_STATE_UNKNOWN              -2
#define TASK_CONTROL_STATE_DEFAULT              -1
#define TASK_CONTROL_STATE_PAUSE                0
#define TASK_CONTROL_STATE_START_OR_RESUME      1
#define TASK_CONTROL_STATE_STOP                 3
    int nTaskControlCmd;

    //���浱ǰ����ĸ�������״̬
    // -1 ����¼����Ĭ��״̬
    //  0 ��������������û�м�����һ��������
    //  1 ���������������Ѿ�������һ��������
    // 

#define TASK_EXTRA_STATE_DEFAULT              -1
#define TASK_EXTRA_STATE_ASYN_SINGLE          0
#define TASK_EXTRA_STATE_ASYN_MULTI           1

    int nTaskExtraFlag;

    std::string strTaskID;

    std::string strLastTaskID;//���������������ID

    std::vector<DEV_HANDLE> vecDevHandle;//�����߳���Ҫʹ�õĹ����б�

    int nIndex;//��������ʱ����ʹ����һ������

    BURN_MODE burnMode;//ʵʱ��¼�����º��¼

    // ������Ҫ��¼�ļ���������key��ʾ��ͬ����
    // ���key == "note"����ʾ��¼
    // ���key == "player"����ʾ������
    // ���key == "autorun"����ʾ�Զ������ļ�
    // ���key == "auth"����ʾ���̷��ʿ����ļ�
    // ���key == "playlist"����ʾ�����б��ļ�
    std::map< std::string,std::vector<BURN_DATA_SOURCE> > mapBurnDataSource;

    std::string strJobID;

    std::string strDiscLabel;

#define NOTE_FILE_UPDATED_YES   "1"
#define NOTE_FILE_UPDATED_NO    "0"
    std::string strRtNoteUpdateFlag;//���ڱ��ʵʱ��¼�Ƿ��Ѿ����º��˱�¼��"1" �ǣ���"1" ��

    std::string strUpperIP;
    std::string strUpperPort;//ʵʱ��¼���̻������������������Ҫ���Ͳ�����Ϣʱ����Ӧ��job�Ѿ���ɾ�����Ӷ��Ҳ����ϲ�ĵ�ַ

    std::string strTaskPropertyFlag;//��־��¼task������

#define DEV_FLAG_USED_YES   "0"
#define DEV_FLAG_USED_NO    "1"
    std::string strBurnDevFlag;//���task��û�н����¼��״̬������У���ô���û�ȡ������Ϣ�Ľӿڱ�����ʹ��

    int nPauseCount;//��ͣ����������������ͣ���µ�ts�ļ�

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


    //�����¼������̵߳�λ,��������������¼���̵��߼�����
    void HandleTask();

    //����¼����task��Ϣд��Ӳ�̣��������������󻹿��Բ���
    void SaveRTBurnBackupTask(BURN_PARAMETER burnParam);

    void SetBurnTaskCallBack(BURN_TASK_CALLBACK cb,void *p);

    void SetDevBurnFilesCallBack(DEV_BURN_FILE_CALLBACK cb,void *p);

    bool IsDevReady(DEV_HANDLE handle,BURN_PARAMETER burnParam,BURN_MODE mode);

    //��������¼����
    bool BurnDisc(BURN_PARAMETER param);

    void CleanTask(std::string strTaskID);

    //���һ��ʵʱ��¼task�İ���Ŀ�¼����task
    void CleanRTBurnBackupTask(std::string strFatherTaskID);

    //���ڿ���ʵʱ��¼�����̣�����¼������֮��ſ���ֹͣʵʱ��¼�����±�¼����ͬ��
    //�Ķ��task����
    // 
    void SetNoteUpdateFlag(std::string strJobID,std::string strFlag);
    std::string GetNoteUpdateFlag(std::string strTaskID);

    std::string GetBurningTaskID(DEV_HANDLE handle);

    std::string GetBurningJobID(DEV_HANDLE handle);

    void SetNoteUpdateFlagByTaskID(std::string strTaskID,std::string strFlag);

    //����ֵ : 0 �����������ȴ�ÿһ��task���ؽ����1û���ҵ�task����ʾָ��job��ͣ�ɹ�
    int PauseRTTasksByJobID(std::string strJobID);

    //����ֵ : 0 �����������ȴ�ÿһ��task���ؽ����1û���ҵ�task����ʾָ��jobֹͣ�ɹ�
    int StopRTJob(std::string strJobID);

    //����ֵ : 0 �����������Ѿ������task�ָ����У�1û���ҵ�task����Ҫ�������ɿ�¼task��-1��ʾtask��������
    int ResumeRTJob(std::string strJobID);

    //��ÿ�¼��ǰ�Ŀ���״̬�������ж��Ƿ������¼
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

    //�ж�ָ���Ĺ�����û�����ڱ�ʹ�ã���Ϊһ���������ڿ�¼ʱ����ȡ������Ϣ����ؽӿڱ����Ƶ��ã�
    //�����ڵ�����Щ�ӿ�֮ǰ�����ж�
    bool GetDevUsed(DEV_HANDLE handle);
    void SetDevUsed(DEV_HANDLE handle,std::string strBurnDevFlag);

    void GetBurningFiles(DEV_HANDLE handle,std::vector<std::string> &vecFiles);

    //�ҵ���һ��δʹ�õĹ���
    int GetFirstUnusedDev(std::vector<DEV_HANDLE> vectDev);

    //�Ƿ����strJobID��Ӧ������
    bool IsTaskExsitByJobID(std::string strJobID);

    //���̿�¼�޶���С
    static INT64 GetLimitSize();
    static void SetLimitSize(INT64 val);

    bool IsTaskCanCloseDisc(std::string strJobID, std::string strTaskID);

    //��·��ת��Ϊ��ǰϵͳ(win32��linux)�µĸ�ʽ��������ļ��У���֤������"\\"��"/"
    static std::string Path(std::string strPath,bool bIsFolder);

    static bool InsertPlayListFile(std::vector<BURN_DATA_SOURCE> &vecDataSource,std::string strFullPlaylistFilePath);

    //�����½���playlist�ļ�ȫ·��
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

    //���Ź��̵��ο�¼����ʵ��
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

    static INT64 m_nLimitSize;//���̿�¼�޶���С
};

#endif
