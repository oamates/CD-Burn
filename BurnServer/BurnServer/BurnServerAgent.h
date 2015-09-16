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

    //��ʼ�������ȡ������Ϣ
    void Init();

    void UnInit();

    void ProccessProtocol(std::string strProtocolContent);

    //���͹���״̬��Ϣ������״̬����������������¼���ļ���Ϣ��ϸ��״̬����Ҳ����m_vecCDROMDriver��������Ϣ
    void SendCDROMDriverInfo(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam);

    void SendBurnServerInfo(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam);

    static std::string GetCurDir();

    static void OnRecvProtocol(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam,
        void *pUsrParam);//���������յ�Э��󣬽���˻ص�

    bool GetJobUpperAddress(std::string strJobID,std::string &strIP,std::string &strPort);

    bool IsJobExist(std::string strJobID);//�ж������Ƿ����ڽ���

    bool GetRtJobInfo(std::string strJobID, CRTBurnInfoEx &RTBurnInfoEx);

    bool IsPrimeraEnvironmentOK();

    bool IsEPSONEnvironmentOK();

    //�ж���ط����Ƿ�����
    bool IsBurnEnvironmentOK(std::string strBurnServerType);

    //�жϸ����Ĺ����б��Ƿ����,������ã���ô��������
    bool IsCDROMDriverAvailable(std::string strJobID);

    bool IsCDROMDriverExist(std::string strCDROMID);
    bool IsCDROMDriverIdle(std::string strCDROMID);

    void StartRTBurn(const CRTBurnInfoEx &RTBurnInfoEx, std::string strLastTaskID = "");//���õײ�ӿڣ���ʼʵʱ��¼
    void StartNormalBurn(const CNormalBurnJobInfoEx &normalJob);//���õײ�ӿڣ���ʼ�º��¼

#define JOB_TYPE_RT         "2"
#define JOB_TYPE_NORMAL     "1"
#define JOB_TYPE_UNKNOWN    "-1"
    std::string GetJobType(std::string strJobID);

    //��ȡʵʱ��¼����״̬�������ж��Ƿ������һ��ʵʱ��¼����
    std::string GetRtJobControlState(std::string strJobID);
    void SetRtJobControlState(std::string strJobID,std::string strControlState);

    void DeleteRTJobDownloadTasks(std::string strJobID);

    void PauseRTBurnJob(std::string strJobID);
    void ResumeRTburn(std::string strJobID);
    void StopRTBurn(std::string strJobID);

    static std::string ExtractFileOrDirName(std::string strPath);

    //�������ص����ļ����ļ��е�Э�飬��������id
    std::string DownloadSingleFileOrDir(std::string strIP,std::string strPort,
        std::string strRemotePath,std::string strLocalRootDir,std::string strNewFileName,bool bIsFolder);

    bool CopyLocalFileOrDir(const CNormalBurnJobInfoEx &normalJob);//����������ݶ��Ǳ��صģ���ô����true

    bool CopyLocalFileOrDir(const CRTBurnInfoEx &rtJob);

    //�º��¼��������Ҫ��¼���ļ�����������Ǳ����ļ������ļ��У���ôִ�и��Ʋ���
    void DownloadFile(std::string strJobID);

    //nFlag��0  ���ز�������playlist��autorun��¼��1 �����±�¼,����Ǳ��صģ���ô����
#define RT_DOWNLOAD_FLAG_ONLY_NOTE_FILE    1
#define RT_DOWNLOAD_FLAG_NO_NOTE_FILE      0
    void DownloadFile(std::string strJobID,int nFlag);

    void DeleteTransmissionTask(std::string strTransID);

    //��ȡ������Ҫ���ص��ļ�����Ϣ�������һ���ļ����޷���ȡ���Ĵ�С����������-1���˺�������һ���������ж����ػ����Ƿ�׼����
    bool GetJobFilesInformation(std::string strJobID);

    //��֤�ļ���·������зָ���
    std::string FixDirPath(std::string strDir);

    void AttachPrimeraJobID(std::string strOrgJobID,std::string strPriJobID);

    std::string GetPrimeraJobID(std::string strOrgJobID);

    //������ȡ��¼״̬��Ŀǰ�����ڵ�������¼
    bool GetJobState(std::string strJobID,std::string &strStateFlag,std::string &stateDescription);

    const std::vector<CNormalBurnJobInfoEx> & GetNormalJobInfoVec() const;
    std::vector<CNormalBurnJobInfoEx> & GetNormalJobInfoVec();

    const std::vector<CRTBurnInfoEx> & GetRTJobInfoVec() const;
    std::vector<CRTBurnInfoEx> & GetRTJobInfoVec();

    //��¼��ɺ�ɾ�����񣬽������ռ��
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

    //��ȡ�ļ���ռ�ÿռ��С
    INT64 GetFileListSize(std::string strIP,std::string strPort,std::string strRemotePath);

    INT64 GetFileSize(std::string strIP,std::string strPort,std::string strRemotePath);

    //�����������ؽ��ȣ�����Ѿ�������ɣ���ô���Կ�ʼ��¼
    //��ǰ������Ϊ��λͳ�����ؽ��ȣ��Ժ����չΪ�Թ���Ϊ��λ
    //strPercent��"-1"��ʾ����ʧ�ܣ������ʾ�������ؽ���
    void UpdateNormalJobDownloadState(std::string strUUID,std::string strPercent="-1");

    void UpdateRtJobDownloadState(std::string strUUID,std::string strPercent="-1");

    //����ǰ�Ŀ�¼״̬������״̬����¼�ļ�״̬�ȣ����͸��ϲ㣬���͵���Ϣ������ÿһ������Ĺ�����Ϣ��
    //�����ģ���������Ҫ�����������״̬�����ڵ�������¼����ͨ��¼������
    //bDevInfo:�Ƿ��͹�����Ϣ
    void SendCurrentBurnStateToUpper(std::string strUpperIP,std::string strUpperPort,
        std::string strJobID,std::string strState,std::string strStateDescription,
        bool bDevInfo=true);

    bool IsDevBeingUsed(DEV_HANDLE handle);

    //���õײ�ӿڣ���BurnServer���ƵĹ�����Ϣ����ڳ�Աm_vecCDROMDriver��
    void UpdateDevInfo();

    std::string GetHostIP();

    static void BurnCallBack(DEV_HANDLE handle,BURN_STATE_PARAMETER state,void *pUsr);//BurnTask�Ļص�����

    //�������̣�ÿ�����������ļ��Ļص�
    static void BurnFilesCallBack(const DEV_BURN_FILES &devBurnFiles,void *pUsr);

#define BURN_FILE_STATE_WAIT    "0"
#define BURN_FILE_STATE_BURNING "1"
#define BURN_FILE_STATE_DONE    "2"
    void SetWinJobBurnFileState(std::string strJobID,std::string strDevID,std::string strState);

    void DealWithCallBackMsg(DEV_HANDLE handle,BURN_STATE_PARAMETER state);

    bool GetCDROMDriverInformationFromHandle(DEV_HANDLE h,CCDROMDriverInfo &info);//ͨ���豸�����ù�������
    void SetCDROMDriverInformation(DEV_HANDLE h,const CCDROMDriverInfo &info);//����ָ����������Ϣ�������ڿ�¼������ʵʱ���¹���״̬���߳�ʼ��ʱ����µĹ���
    DEV_HANDLE GetDeviceHandle(std::string strID);

    void PushExtraDataSource(const CNormalBurnJobInfoEx &normalJobInfo,std::map< std::string,std::vector<BURN_DATA_SOURCE> > &mapBurnDataSource);

    void PushExtraDataSource(const CRTBurnInfoEx &rtJobInfo,std::map< std::string,std::vector<BURN_DATA_SOURCE> > &mapBurnDataSource);

    //����ָ��job��ָ��task��״̬��Ϣ
    void SetTaskState(std::string strJobID,std::string strTaskID,std::string strStateFlag,std::string strStateDescription);

    //����ָ��������״̬��Ϣ
    bool SetDevBurnState(DEV_HANDLE handle,std::string strStateFlag,std::string strStateDescription);

    bool AddTask(const BURN_PARAMETER &param);

    void ConfirmRTBurnBackup(std::string strJobID);
    void CancelRTBurnBackup(std::string strJobID);
    void StartRTBurnBackup(std::string strJobID);

    /*
    *�ж�һ��job������task��ִ�����
    *����ֵ
    *       -1����¼ʧ�ܣ�����taskִ��ʧ��
    *       1 �������У�������һ��task�߳�û���˳�����������һ��taskִ��ʧ�ܣ��������ȡ����״̬ʧ�ܣ�����Ϊ������������
    *       2����¼��ɣ����ǲ�������taskִ�гɹ�
    *       7��job���ɹ�ִ��
    */
    int IsJobDone(std::string strJobID);

    bool IsJobRunning(std::string strJobID);

    bool IsJobPaused(std::string strJobID);

    bool IsJobStopped(std::string strJobID);

    bool IsJobStopping(std::string strJobID);

    void AddDevBurnFilesInfo(const DEV_BURN_FILES &devBurnFiles);

    //������ÿ��������������Ҫ����¼���ļ���Ϣ
    void AddDevBurnFilesInfo(std::string strJobID,std::string strDevID,
        const std::vector<CLocationInfo> &vecLocationInfo);

    std::vector<CNormalBurnJobInfoEx> m_vecNormalJob;//��ǰ���ڽ��е���ͨ��¼�����б�
    std::vector<CRTBurnInfoEx> m_vecRTBurnInfo;//��ǰ���ڽ��е�ʵʱ��¼�����б�

    ZOSMutex m_mutexJobVec;

    //BurnServer�ɿ��ƵĹ����б�����ÿһ��������״̬��Ϣ����������¼���ļ���Ϣ������ʵʱ����ÿһ��������״̬��������
    std::vector<CCDROMDriverInfo> m_vecDev;

    ZOSMutex m_mutexDevVec;

    std::map<DEV_HANDLE,std::string> m_mapDev;//key�������豸�����value������id

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
