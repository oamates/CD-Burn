#ifdef WIN32
#include "Primera.h"
#include "BurnServerEPSONCtrl.h"
#include "NetUtil.h"

#include "WindowsProcess.h"
#else     
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <signal.h>
#endif

#include "InterfaceProtocol.h"
#include "BurnServerAgent.h"
#include "FileUtil.h"
#include "CharsetConvert.h"
#include "UDPClient.h"
#include <string>
#include <cstdio>

typedef struct
{
    void *pThreadData;//协议内容
    void *pUserParam;//使用者指定的参数

    void *p1;//预留参数
    void *p2;
}THREAD_PARAM;

#define RTBURN_BACKUP_YES   "1"
#define RTBURN_BACKUP_NO    "2"

#define RTBURN_BACKUP_AFTER_RESTART_YES   "3"
#define RTBURN_BACKUP_AFTER_RESTART_NO    "4"

#ifdef WIN32
DWORD WINAPI ProcessProtocolThread(LPVOID lpParameter);//windows下处理上层协议的线程
DWORD WINAPI StartNormalBurnThread(LPVOID lpParameter);
DWORD WINAPI DeleteDownloadTaskThread(LPVOID lpParameter);
#else
void * ProcessProtocolThread(void *pThreadParam);//linux下处理上层协议的线程
void * StartNormalBurnThread(void *pThreadParam);
void * DeleteDownloadTaskThread(void *pThreadParam);
#endif

BurnServerAgent::BurnServerAgent():
m_mutexJobVec("NormalJobVec"),
m_mutexDevVec("DevVec")
{
}
BurnServerAgent::~BurnServerAgent()
{
}

std::string BurnServerAgent::GetCurDir()
{
    char szCwd[1024]={0};

#ifdef WIN32
    GetModuleFileNameA(NULL,szCwd,sizeof(szCwd)/sizeof(char));

    int nLen=strlen(szCwd);
    while (nLen > 0 &&
        szCwd[nLen-1] != '\\')
    {
        szCwd[nLen-1]='\0';
        nLen=strlen(szCwd);
    }
#else
    int nLen=readlink("/proc/self/exe",szCwd,sizeof(szCwd));
    if (nLen > 0)
    {
        szCwd[nLen]='\0';

        while (nLen > 0 &&
            szCwd[nLen-1] != '/')
        {
            szCwd[nLen-1]='\0';

            nLen=strlen(szCwd);
        }
    }
#endif

    return std::string(szCwd);
}

void BurnServerAgent::OnRecvProtocol(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam,void *pUsrParam)
{
    if (NULL == pCBParam ||
        NULL == pUsrParam ||
        NULL == hNCXServer)
    {
        LOG_ERROR(("[BurnServerAgent::OnRecvProtocol] Parameter error\r\n"));
        return;
    }

    const char *pszContent=pCBParam->sProtocolContent;

    std::string strCommandType=CInterfaceProtocol::GetCommandType(pszContent);
    //对于需要立即返回结果的协议，在原线程处理
    if ("getCDROMInfo" == strCommandType)//获取光驱信息
    {
        LOG_INFO(("[BurnServerAgent::OnRecvProtocol] Receive command \"getCDROMInfo\"\r\n"));

        BurnServerAgent *pBurnServerAgent=(BurnServerAgent *)pUsrParam;

        pBurnServerAgent->UpdateDevInfo();

        pBurnServerAgent->SendCDROMDriverInfo(hNCXServer,pCBParam);

        return;
    }
    else if ("getBurnServerInfo" == strCommandType)//获取刻录服务器信息
    {
        LOG_INFO(("[BurnServerAgent::OnRecvProtocol] Receive command \"getBurnServerInfo\"\r\n"));

        BurnServerAgent *pBurnServerAgent=(BurnServerAgent *)pUsrParam;
        pBurnServerAgent->SendBurnServerInfo(hNCXServer,pCBParam);

        return;
    }
    else if ("sendBurnCtrlCMD" == strCommandType)
    {
        BurnServerAgent *pBurnServerAgent=(BurnServerAgent *)pUsrParam;
        std::string strProtocolContent=pszContent;
        LOG_INFO(("[BurnServerAgent::OnRecvProtocol] Protocol content : %d\r\n%s\r\n",
            __LINE__,strProtocolContent.c_str()));

        std::string strJobID=CInterfaceProtocol::GetJobID(strProtocolContent.c_str());
        std::string strUpperIP=CInterfaceProtocol::GetTagValue(strProtocolContent,"upServerIP");
        std::string strUpperPort=CInterfaceProtocol::GetTagValue(strProtocolContent,"upServerPort");
        std::string strCtrlCMD=CInterfaceProtocol::GetTagValue(strProtocolContent,"ctrlCMD");

        if ("pauseRTBurn" == strCtrlCMD)//暂停实时刻录
        {
            LOG_INFO(("[BurnServerAgent::OnRecvProtocol] Receive command \"pauseRTBurn\"\r\n"));

            pBurnServerAgent->PauseRTBurnJob(strJobID);
        }
        else if ("resumeRTBurn" == strCtrlCMD)//恢复实时刻录
        {
            LOG_INFO(("[BurnServerAgent::OnRecvProtocol] Receive command \"resumeRTBurn\"\r\n"));

            pBurnServerAgent->ResumeRTburn(strJobID);
        }
        else if ("stopRTBurn" == strCtrlCMD)//停止实时刻录或者事后刻录
        {
            LOG_INFO(("[BurnServerAgent::OnRecvProtocol] Receive command \"stopRTBurn\"\r\n"));

            if (pBurnServerAgent->IsJobExist(strJobID))
            {
                pBurnServerAgent->StopRTBurn(strJobID);
            }
            else
            {
                LOG_ERROR(("[BurnServerAgent::OnRecvProtocol] Receive command \"stopRTBurn\", but job: %s does not exist!\r\n"
                    ,strJobID.c_str()));
                pBurnServerAgent->SendCurrentBurnStateToUpper(strUpperIP,strUpperPort,
                    strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","jobnotexist"),false);
            }

//             //如果这个job存在刻录备份的task，那么这时激活这些task的时机
//             //对应情况：开庭过程中实时刻录出现异常，结束庭审时，上层发送停止实时刻录的协议，此时可以开始补刻
//             if (m_BurnTask.HasRTBurnBackupTask(strJobID))
//             {
//                 StartRTBurnBackup(strJobID);
//             }
        }
        return;
    }

    THREAD_PARAM *pThreadParam=new THREAD_PARAM;
    if (pThreadParam != NULL)
    {
        int nLen=strlen(pCBParam->sProtocolContent);
        pThreadParam->pThreadData=new char[nLen+1];
        if (pThreadParam->pThreadData != NULL)
        {
            strcpy((char *)(pThreadParam->pThreadData),pCBParam->sProtocolContent);
            ((char *)(pThreadParam->pThreadData))[nLen]='\0';

            pThreadParam->pUserParam=pUsrParam;

#ifdef WIN32
            HANDLE hThread=CreateThread(NULL,0,ProcessProtocolThread,pThreadParam,0,NULL);
            if (NULL == hThread)
            {
                LOG_ERROR(("[BurnServerAgent::OnRecvProtocol] ProcessProtocolThread failed!\r\n"));
                delete [] (char *)(pThreadParam->pThreadData);
                pThreadParam->pThreadData=NULL;

                delete pThreadParam;
                pThreadParam=NULL;
            }
            else
            {
                CloseHandle(hThread);
            }
#else
            pthread_t t;
            if (pthread_create(&t,NULL,ProcessProtocolThread,pThreadParam) != 0)
            {
                LOG_ERROR(("[BurnServerAgent::OnRecvProtocol] Create thread error\r\n"));

                delete [] (char *)(pThreadParam->pThreadData);
                pThreadParam->pThreadData=NULL;

                delete pThreadParam;
                pThreadParam=NULL;
            }
            else
            {
                pthread_detach(t);
            }
#endif
        }
        else
        {
            delete pThreadParam;
            pThreadParam=NULL;
        }
    }
    else
    {
        LOG_ERROR(("[BurnServerAgent::OnRecvProtocol] new THREAD_PARAM failed!\r\n"));
    }
}

#ifdef WIN32
DWORD WINAPI ProcessProtocolThread(LPVOID lpParameter)
{
    if (lpParameter != NULL)
    {
        THREAD_PARAM *pThreadParam=(THREAD_PARAM *)lpParameter;
        std::string strProtocolContent=(char *)(pThreadParam->pThreadData);
        BurnServerAgent *pBurnServerAgent=(BurnServerAgent *)(pThreadParam->pUserParam);

        //释放new分配的内存
        if (pThreadParam->pThreadData != NULL)
        {
            delete [] (char *)(pThreadParam->pThreadData);
            pThreadParam->pThreadData=NULL;
        }
        delete pThreadParam;
        pThreadParam=NULL;

        if (pBurnServerAgent != NULL &&
            !strProtocolContent.empty())
        {
            pBurnServerAgent->ProccessProtocol(strProtocolContent);
        }
        else
        {
            LOG_ERROR(("[ThreadProc] Thread parameter error\r\n"));
        }
    }

    return 0;
}
#else
void * ProcessProtocolThread(void *pThreadParam)
{
    if (pThreadParam != NULL)
    {
        THREAD_PARAM *p=(THREAD_PARAM *)pThreadParam;
        std::string strProtocolContent=(char *)(p->pThreadData);
        BurnServerAgent *pBurnServerAgent=(BurnServerAgent *)(p->pUserParam);

        //释放new分配的内存
        if (p->pThreadData != NULL)
        {
            delete [] (char *)(p->pThreadData);
            p->pThreadData=NULL;
        }
        delete p;
        p=NULL;

        if (pBurnServerAgent != NULL &&
            !strProtocolContent.empty())
        {
            pBurnServerAgent->ProccessProtocol(strProtocolContent);
        }
        else
        {
            LOG_ERROR(("[ThreadProc] Thread parameter error\r\n"));
        }
    }

    return NULL;
}
#endif

void BurnServerAgent::Init()
{
#ifdef WIN32
#else
    //进程运行时的当前目录可能不是可执行文件所在目录
    //由于底层有 system("./Get_CDRom_Dev_Info.sh");这样的语句，所以添加这一语句 
    chdir(BurnServerAgent::GetCurDir().c_str());
#endif

    UpdateDevInfo();

    CInterfaceProtocol::SetCallBack(BurnServerAgent::OnRecvProtocol,this);

    m_BurnTask.Create();

    m_BurnTask.SetBurnTaskCallBack(BurnCallBack,this);

    m_BurnTask.SetDevBurnFilesCallBack(BurnServerAgent::BurnFilesCallBack,this);

    m_BurnSendStateTask.Create();

#ifdef WIN32

    //设置开机启动
    WindowsProcess::AutoStart(true);

    m_bIsEPSONEnvOK=( 0 == CEPSONCtrl::InitEPSONEnv() );
    m_pEPSONCtrl=NULL;

#else
#endif
}

void BurnServerAgent::UnInit()
{
#ifdef WIN32
    if (m_pEPSONCtrl != NULL)
    {
        delete m_pEPSONCtrl;
        m_pEPSONCtrl=NULL;
    }
    CEPSONCtrl::DestroyEPSONEnv();
#endif

    m_BurnTask.Close();
    m_BurnSendStateTask.Close();
}

void BurnServerAgent::ProccessProtocol(std::string strProtocolContent)
{
    LOG_INFO(("[BurnServerAgent::ProccessProtocol] Protocol content : %d\r\n%s\r\n",
        __LINE__,strProtocolContent.c_str()));

    std::string strCommandType=CInterfaceProtocol::GetCommandType(strProtocolContent.c_str());
    std::string strJobID=CInterfaceProtocol::GetJobID(strProtocolContent.c_str());

    std::string strUpperIP=CInterfaceProtocol::GetTagValue(strProtocolContent,"upServerIP");
    std::string strUpperPort=CInterfaceProtocol::GetTagValue(strProtocolContent,"upServerPort");

    if ("startRTBurn" == strCommandType)//开始实时刻录
    {
        LOG_INFO(("[BurnServerAgent::ProccessProtocol] Receive command \"startRTBurn\"\r\n"));

        CRTBurnInfoEx rtJob;
        CInterfaceProtocol::AnalyzeProtocolStartRTBurnToBurnServer(
            std::string(strProtocolContent.c_str()),rtJob);

        if (!IsJobExist(strJobID))
        {
            {
                ZOSMutexLocker locker(&m_mutexJobVec);

                m_vecRTBurnInfo.push_back(rtJob);
            }

#ifdef WIN32
            LOG_ERROR(("[BurnServerAgent::ProccessProtocol] Windows no rt burn,%d\r\n",__LINE__));

            SendCurrentBurnStateToUpper(strUpperIP,strUpperPort,strJobID,STATE_BURN_ERR,
                gBurnStateCfgFile.GetValue("info","winnort"),false);

            CleanJob(strJobID);
#else
            if (IsCDROMDriverAvailable(strJobID))
            {
                StartRTBurn(rtJob);

                if (CopyLocalFileOrDir(rtJob))
                {
                    //已经将需要的文件复制到指定文件夹
                    LOG_INFO(("[BurnServerAgent::ProccessProtocol] All data local,%d\r\n",__LINE__));
                }
                else
                {
                    //因为开始实时刻录的时候，不能保证笔录文件已经存在于服务器
                    DownloadFile(strJobID,RT_DOWNLOAD_FLAG_NO_NOTE_FILE);
                }
            }
            else
            {
                LOG_ERROR(("[BurnServerAgent::ProccessProtocol] Dev is not available,%d\r\n",__LINE__));

                SendCurrentBurnStateToUpper(rtJob.GetUpServerIP(),rtJob.GetUpServerPort(),
                    strJobID,STATE_SINGLE_DEV_ERR_JOB_FAILED,gBurnStateCfgFile.GetValue("info","devunavailable"));

                CleanJob(strJobID);
            }
#endif
        }
        else
        {
            LOG_ERROR(("[BurnServerAgent::ProccessProtocol] Job is running,%d\r\n",__LINE__));

            SendCurrentBurnStateToUpper(rtJob.GetUpServerIP(),rtJob.GetUpServerPort(),
                strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","running"),false);
        }

    }
//     else if ("sendBurnCtrlCMD" == strCommandType)
//     {
//         std::string strCtrlCMD=CInterfaceProtocol::GetTagValue(strProtocolContent,"ctrlCMD");
// 
//         if ("pauseRTBurn" == strCtrlCMD)//暂停实时刻录
//         {
//             LOG_INFO(("[BurnServerAgent::ProccessProtocol] Receive command \"pauseRTBurn\"\r\n"));
// 
//             PauseRTBurnJob(strJobID);
//         }
//         else if ("resumeRTBurn" == strCtrlCMD)//恢复实时刻录
//         {
//             LOG_INFO(("[BurnServerAgent::ProccessProtocol] Receive command \"resumeRTBurn\"\r\n"));
// 
//             ResumeRTburn(strJobID);
//         }
//         else if ("stopRTBurn" == strCtrlCMD)//停止实时刻录或者事后刻录
//         {
//             LOG_INFO(("[BurnServerAgent::ProccessProtocol] Receive command \"stopRTBurn\"\r\n"));
// 
//             if (IsJobExist(strJobID))
//             {
//                 StopRTBurn(strJobID);
//             }
//             else
//             {
//                 LOG_ERROR(("[BurnServerAgent::ProccessProtocol] Receive command \"stopRTBurn\", but job: %s does not exist!\r\n"
//                     ,strJobID.c_str()));
//                 SendCurrentBurnStateToUpper(strUpperIP,strUpperPort,
//                     strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","jobnotexist"),false);
//             }
// 
//             //如果这个job存在刻录备份的task，那么这时激活这些task的时机
//             //对应情况：开庭过程中实时刻录出现异常，结束庭审时，上层发送停止实时刻录的协议，此时可以开始补刻
//             if (m_BurnTask.HasRTBurnBackupTask(strJobID))
//             {
//                 StartRTBurnBackup(strJobID);
//             }
//         }
//     }
    else if ("startBurn" == strCommandType)//开始事后刻录
    {
        CNormalBurnJobInfoEx normalJob;

        CInterfaceProtocol::AnalyzeProtocolStartBurnToBurnServer(strProtocolContent,normalJob);//解析协议

        std::string strJobID=normalJob.GetJobID();
        std::string strBurnServerType=normalJob.GetBurnServerInfo().GetBurnServerType();
        std::string strUpperServerIP=normalJob.GetUpServerIP();
        std::string strUpperServerPort=normalJob.GetUpServerPort();

        LOG_INFO(("~~~~~~~~[BurnServerAgent::ProccessProtocol] Receive command \"startBurn\",job id : %s\r\n",
            strJobID.c_str()));

        if (!IsJobExist(strJobID))
        {
            {
                ZOSMutexLocker lockerNormal(&m_mutexJobVec);

                m_vecNormalJob.push_back(normalJob);
            }

            if (IsCDROMDriverAvailable(strJobID))
            {
                if (GetJobFilesInformation(strJobID))
                {
                    if (CopyLocalFileOrDir(normalJob))
                    {
                        //所有数据准备好了，可以开始刻录，创建新的线程
                        THREAD_PARAM *pThreadParam=new THREAD_PARAM;
                        if (pThreadParam != NULL)
                        {
                            CNormalBurnJobInfoEx *pNormalBurnJobInfoEx=new CNormalBurnJobInfoEx(m_vecNormalJob.back());
                            if (pNormalBurnJobInfoEx != NULL)
                            {
                                pThreadParam->pThreadData=pNormalBurnJobInfoEx;
                                pThreadParam->pUserParam=this;
#ifdef WIN32
                                HANDLE hThread=CreateThread(NULL,0,StartNormalBurnThread,pThreadParam,0,NULL);
                                if (NULL == hThread)
                                {
                                    delete pNormalBurnJobInfoEx;
                                    pNormalBurnJobInfoEx=NULL;

                                    delete pThreadParam;
                                    pThreadParam=NULL;
                                }
                                else
                                {
                                    CloseHandle(hThread);
                                }
#else

                                LOG_INFO(("[BurnServerAgent::UpdateDownloadState] Create a thread to start normal burn...\r\n"));

                                pthread_t t;
                                if (pthread_create(&t,NULL,StartNormalBurnThread,pThreadParam) != 0)
                                {
                                    delete pNormalBurnJobInfoEx;
                                    pNormalBurnJobInfoEx=NULL;

                                    delete pThreadParam;
                                    pThreadParam=NULL;
                                }
                                else
                                {
                                    pthread_detach(t);
                                }
#endif
                            }
                            else
                            {
                                delete pThreadParam;
                                pThreadParam=NULL;
                            }
                        }
                    }
                    else
                    {
                        DownloadFile(strJobID);

                        SendCurrentBurnStateToUpper(strUpperServerIP,strUpperServerPort,
                            strJobID,STATE_DOWNLOADING,gBurnStateCfgFile.GetValue("info","downloading"));
                    }
                }
                else
                {
                    LOG_ERROR(("[BurnServerAgent::ProccessProtocol] Get job size error\r\n"));

                    //存在fileAnywhere无法识别的文件，刻录失败
                    SendCurrentBurnStateToUpper(strUpperServerIP,strUpperServerPort,
                        strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","downloadfailed"));

                    CleanJob(strJobID);
                }
            }
            else
            {
                LOG_ERROR(("~~~~~~~~[BurnServerAgent::ProccessProtocol] Dev unavailable,%s,%s\r\n~~~~~~~~",
                    strUpperServerIP.c_str(),strUpperServerPort.c_str()));

                SendCurrentBurnStateToUpper(strUpperServerIP,strUpperServerPort,
                    strJobID,STATE_SINGLE_DEV_ERR_JOB_FAILED,gBurnStateCfgFile.GetValue("info","devunavailable"));

                CleanJob(strJobID);
            }
        }
        else
        {
            LOG_ERROR(("~~~~~~~~[BurnServerAgent::ProccessProtocol] Job is running,%s,%s~~~~~~~~\r\n",
                strUpperServerIP.c_str(),strUpperServerPort.c_str()));

            SendCurrentBurnStateToUpper(strUpperServerIP,strUpperServerPort,
                strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","running"),false);
        }
    }
    else if ("getJobState" == strCommandType)
    {
#ifdef WIN32
#else
#endif
    }
    else if ("sendRTBurnBackupResponse" == strCommandType)
    {
        std::string strResponse=CInterfaceProtocol::GetTagValue(strProtocolContent,"response");

        LOG_INFO(("[BurnServerAgent::ProccessProtocol] Upper response : %s,%d\r\n",
            strResponse.c_str(),__LINE__));

        if (RTBURN_BACKUP_YES == strResponse)
        {
            ConfirmRTBurnBackup(strJobID);
        }
        else if (RTBURN_BACKUP_NO == strResponse)
        {
            CancelRTBurnBackup(strJobID);
        }
    }
    else if ("sendRTBurnBackupAfterRestartResponse" == strCommandType)
    {
        std::string strResponse=CInterfaceProtocol::GetTagValue(strProtocolContent,"response");

        LOG_INFO(("[BurnServerAgent::ProccessProtocol] Upper response : %s,%d\r\n",
            strResponse.c_str(),__LINE__));

        if (RTBURN_BACKUP_AFTER_RESTART_YES == strResponse)
        {
            //保留任务的数据，并且自动生成一个对应的任务文件
            SaveJob(strJobID);
        }
        else if (RTBURN_BACKUP_AFTER_RESTART_NO == strResponse)
        {
            //删除任务的备份xml
            CleanRTBurnBackupXML(strJobID);
            CleanRTJobBackupData(strJobID);
        }
    }
}

bool BurnServerAgent::GetJobUpperAddress(std::string strJobID,std::string &strIP,
                                         std::string &strPort)
{
    bool bRet=false;

    return bRet;
}

bool BurnServerAgent::IsJobExist(std::string strJobID)
{
    ZOSMutexLocker lockerNormal(&m_mutexJobVec);

    bool bRet=false;

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormal;
    for (iterNormal=m_vecNormalJob.begin();iterNormal!=m_vecNormalJob.end();
        ++iterNormal)
    {
        if (iterNormal->GetJobID() == strJobID)
        {
            bRet=true;

            break;
        }
    }

    if (!bRet)
    {
        std::vector<CRTBurnInfoEx>::iterator iterRT;
        for (iterRT=m_vecRTBurnInfo.begin();iterRT!=m_vecRTBurnInfo.end();++iterRT)
        {
            if (iterRT->GetJobID() == strJobID)
            {
                bRet=true;

                break;
            }
        }
    }

    return bRet;
}

bool BurnServerAgent::GetRtJobInfo(std::string strJobID, CRTBurnInfoEx &RTBurnInfoEx)
{
    ZOSMutexLocker lockerNormal(&m_mutexJobVec);

    bool bRet=false;

    std::vector<CRTBurnInfoEx>::iterator iterRT;
    for (iterRT=m_vecRTBurnInfo.begin();iterRT!=m_vecRTBurnInfo.end();++iterRT)
    {
        if (iterRT->GetJobID() == strJobID)
        {
            bRet=true;
            RTBurnInfoEx = (*iterRT);
            break;
        }
    }

    return bRet;
}

bool BurnServerAgent::IsPrimeraEnvironmentOK()
{
#ifdef WIN32
    return IsPrimeraOK();
#else
    return false;
#endif
}

bool BurnServerAgent::IsEPSONEnvironmentOK()
{
#ifdef WIN32
    return m_bIsEPSONEnvOK;
#else
    return false;
#endif
}

bool BurnServerAgent::IsBurnEnvironmentOK(std::string strBurnServerType)
{
    bool bRet=false;

    if ("2" == strBurnServerType)
    {
        //派美雅刻录
        bRet=IsPrimeraEnvironmentOK();

        if (bRet)
        {
            LOG_INFO(("[BurnServerAgent::IsBurnEnvironmentOK] Primera is ok\r\n"));
        }
    } 
    else if("4" == strBurnServerType)
    {
        //爱普生刻录
        bRet=IsEPSONEnvironmentOK();
    }

    return bRet;
}

bool BurnServerAgent::IsCDROMDriverAvailable(std::string strJobID)
{
    ZOSMutexLocker lockerNormal(&m_mutexJobVec);

    bool bRet=true;
    bool bJobFound=false;

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormal;
    for (iterNormal=m_vecNormalJob.begin();iterNormal!=m_vecNormalJob.end();
        ++iterNormal)
    {
        if (iterNormal->GetJobID() == strJobID)
        {
            bJobFound=true;

            if (iterNormal->GetBurnServerInfo().GetBurnServerType() != BURNSERVER_TYPE_NORMAL)
            {
                //批量刻录机器管理着自己的光驱，所以不会出现被占用的情况
                bRet=true;
                
                return bRet;
            }

            std::vector<CCDROMDriverInfo> &vecDev=iterNormal->GetBurnServerInfo().GetVecCDROMDriverInfo();
            std::vector<CCDROMDriverInfo>::iterator iterDev;
            for (iterDev=vecDev.begin();iterDev!=vecDev.end();++iterDev)
            {
                if (IsCDROMDriverExist(iterDev->GetID()))
                {
                    if (!IsCDROMDriverIdle(iterDev->GetID()))
                    {
                        LOG_ERROR(("[BurnServerAgent::IsCDROMDriverAvailable] Dev %s is being used,%d\r\n",
                            iterDev->GetID().c_str(),__LINE__));

                        iterDev->SetStateFlag("-2");
                        iterDev->SetStateDescription(gBurnStateCfgFile.GetValue("info","devunavailable"));

                        bRet=false;
                    }
                }
                else
                {
                    LOG_ERROR(("[BurnServerAgent::IsCDROMDriverAvailable] Dev %s is not found,%d\r\n",
                        iterDev->GetID().c_str(),__LINE__));

                    iterDev->SetStateFlag("-2");
                    iterDev->SetStateDescription(gBurnStateCfgFile.GetValue("info","devnotfound"));

                    bRet=false;
                }
            }

            //将任务指定的光驱分配给它
            if (bRet)
            {
                ZOSMutexLocker locker(&m_mutexDevVec);

                for (iterDev=vecDev.begin();iterDev!=vecDev.end();
                    ++iterDev)
                {
                    for (size_t j=0;j<m_vecDev.size();++j)
                    {
                        if (iterDev->GetID() == m_vecDev.at(j).GetID())
                        {
                            m_vecDev.at(j).SetJobID(strJobID);

                            break;
                        }
                    }
                }
            }

            break;
        }
    }

    if (!bJobFound)
    {
        std::vector<CRTBurnInfoEx>::iterator iterRT;
        for (iterRT=m_vecRTBurnInfo.begin();iterRT!=m_vecRTBurnInfo.end();++iterRT)
        {
            if (iterRT->GetJobID() == strJobID)
            {
                bJobFound=true;

                std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo=iterRT->GetBurnServerInfo().GetVecCDROMDriverInfo();
                std::vector<CCDROMDriverInfo>::iterator iterDev;
                for (iterDev=vecCDROMDriverInfo.begin();iterDev!=vecCDROMDriverInfo.end();
                    ++iterDev)
                {
                    std::string strDevID=iterDev->GetID();
                    if (IsCDROMDriverExist(strDevID))
                    {
                        if (!IsCDROMDriverIdle(strDevID))
                        {
                            LOG_ERROR(("[BurnServerAgent::IsCDROMDriverAvailable] Dev %s not idle,%d\r\n",
                                strDevID.c_str(),__LINE__));

                            iterDev->SetStateFlag("-2");
                            iterDev->SetStateDescription(gBurnStateCfgFile.GetValue("info","devunavailable"));

                            bRet=false;
                        }
                    }
                    else
                    {
                        LOG_ERROR(("[BurnServerAgent::IsCDROMDriverAvailable] Dev %s not exist,%d\r\n",
                            strDevID.c_str(),__LINE__));

                        iterDev->SetStateFlag("-2");
                        iterDev->SetStateDescription(gBurnStateCfgFile.GetValue("info","devnotfound"));

                        bRet=false;
                    }
                }

                //将任务指定的光驱分配给它
                if (bRet)
                {
                    ZOSMutexLocker locker(&m_mutexDevVec);

                    for (iterDev=vecCDROMDriverInfo.begin();iterDev!=vecCDROMDriverInfo.end();
                        ++iterDev)
                    {
                        for (size_t j=0;j<m_vecDev.size();++j)
                        {
                            if (iterDev->GetID() == m_vecDev.at(j).GetID())
                            {
                                m_vecDev.at(j).SetJobID(strJobID);

                                break;
                            }
                        }
                    }
                }

                break;
            }
        }

    }        

    return bRet;
}

bool BurnServerAgent::IsCDROMDriverExist(std::string strCDROMID)
{
    bool bRet=false;

    ZOSMutexLocker locker(&m_mutexDevVec);

    std::vector<CCDROMDriverInfo>::const_iterator iterDev;
    for (iterDev=m_vecDev.begin();iterDev!=m_vecDev.end();++iterDev)
    {
        if (iterDev->GetID() == strCDROMID)
        {
            bRet=true;

            break;
        }
    }

    return bRet;
}

bool BurnServerAgent::IsCDROMDriverIdle(std::string strCDROMID)
{
    bool bRet=false;

    ZOSMutexLocker locker(&m_mutexDevVec);

    std::vector<CCDROMDriverInfo>::const_iterator iterDev;
    for (iterDev=m_vecDev.begin();iterDev!=m_vecDev.end();++iterDev)
    {
        if (iterDev->GetID() == strCDROMID)
        {
            if ("" == iterDev->GetJobID())
            {
                bRet=true;

                break;
            }
        }
    }

    return bRet;
}

void BurnServerAgent::StartRTBurn(const CRTBurnInfoEx &rtJob, std::string strLastTaskID)
{
    std::string strJobID=rtJob.GetJobID();

    LOG_INFO(("[BurnServerAgent::StartRTBurn] Start rt job %s,%d\r\n",strJobID.c_str(),
        __LINE__));

    // 此时，无论哪种刻录方式，每一个光驱包含的内容相同，需要在刻录前（或刻录中）做处理（至少现在是这样）
    // 实时刻录在开始时不添加视频流以外的数据，在封盘前的回调函数中，临时添加，一个原因是笔录在实时地更新
    std::string strBurnType=rtJob.GetCommonBurnParam().GetBurnType();
    const std::vector<CCDROMDriverInfo> &vecDev=rtJob.GetBurnServerInfo().GetVecCDROMDriverInfo();

    size_t nDevNum=vecDev.size();
    LOG_INFO(("[BurnServerAgent::StartRTBurn] Dev count : %d,%d\r\n",nDevNum,
        __LINE__));

    assert(nDevNum >0);

    if (0 == nDevNum)
    {
        LOG_ERROR(("[BurnServerAgent::StartRTBurn] Dev count error,%d\r\n",__LINE__));

        SendCurrentBurnStateToUpper(rtJob.GetUpServerIP(),rtJob.GetUpServerPort(),
            strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","protocolparamerr"));

        CleanJob(strJobID);

        return;
    }

    if (BURN_TYPE_SINGLE == strBurnType)//单盘刻录
    {
        const std::vector<CLocationInfo> &vecLocationInfo=vecDev.at(0).GetVecLocationInfo();
        size_t nLocationCount=vecLocationInfo.size();

        assert(nLocationCount > 0);

        if (0 == nLocationCount)
        {
            LOG_ERROR(("[BurnServerAgent::StartRTBurn] Location count error,%d\r\n",__LINE__));

            SendCurrentBurnStateToUpper(rtJob.GetUpServerIP(),rtJob.GetUpServerPort(),
                strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","protocolparamerr"));

            CleanJob(strJobID);

            return;
        }

        BURN_PARAMETER param;
        param.nTaskRunningFlag=TASK_STATE_READY_TO_RUN;
        param.nTaskControlCmd=TASK_CONTROL_STATE_DEFAULT;
        param.nTaskExtraFlag=TASK_EXTRA_STATE_DEFAULT;
        param.strLastTaskID=strLastTaskID;
        param.burnMode=MEDIAFILE_BURN_STREAM;
        param.strTaskID=UUID_GetUUID();
        param.vecDevHandle.push_back(GetDeviceHandle(vecDev.at(0).GetID()));
        param.nIndex=0;

        for (size_t i=0;i<nLocationCount;++i)
        {
            std::string strLocationID=vecLocationInfo.at(i).GetID();

            BURN_DATA_SOURCE source;
            source.strType=SOURCE_TYPE_URL;
            source.strSourceUrl=vecLocationInfo.at(i).GetURL();

            LOG_INFO(("[BurnServerAgent::StartRTBurn] BURN_TYPE_SINGLE,add url : %s,%d\r\n",
                source.strSourceUrl.c_str(),__LINE__));

            if (strLocationID != "" &&
                source.strSourceUrl != "")
            {
                param.mapBurnDataSource[strLocationID].push_back(source);
            }
        }

        if (param.mapBurnDataSource.empty())
        {
            LOG_ERROR(("[BurnServerAgent::StartRTBurn] Stream source error,%d\r\n",__LINE__));

            SendCurrentBurnStateToUpper(rtJob.GetUpServerIP(),rtJob.GetUpServerPort(),
                strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","protocolparamerr"));

            CleanJob(strJobID);

            return;
        }

        PushExtraDataSource(rtJob,param.mapBurnDataSource);

        param.strJobID=strJobID;
        param.strDiscLabel=rtJob.GetCommonBurnParam().GetCDName();
        param.strRtNoteUpdateFlag=NOTE_FILE_UPDATED_NO;
        param.strUpperIP=rtJob.GetUpServerIP();
        param.strUpperPort=rtJob.GetUpServerPort();
        param.strTaskPropertyFlag=TASK_PROPERTY_RT;

        param.strNewFileName=rtJob.GetNewFileName();
        std::string str=rtJob.GetBurnBufferSize();
        LOG_INFO(("[BurnServerAgent::StartRTBurn] BurnBufferSize :%s M, lineNum : %d\r\n",str.c_str(), __LINE__));
        if (str != "")
        {
            param.ulBufferSize=StringToInt(str)*1024*1024;//协议中要求单位为M
        }
        str=rtJob.GetCDAlarmLimit();
        if (str != "")
        {
            param.nAlarmSize=StringToInt(str);
        }

        if (!AddTask(param))
        {
            CleanJob(strJobID);
        }
    }
    else if (BURN_TYPE_MULTI_SYN == strBurnType)//多盘同刻
    {
        std::vector<CCDROMDriverInfo>::const_iterator iterDev;
        for (iterDev=vecDev.begin();iterDev!=vecDev.end();++iterDev)
        {
            const std::vector<CLocationInfo> &vecLocationInfo=iterDev->GetVecLocationInfo();
            size_t nLocationCount=vecLocationInfo.size();

            assert(nLocationCount > 0);

            if (0 == nLocationCount)
            {
                LOG_ERROR(("[BurnServerAgent::StartRTBurn] Location count error,%d\r\n",__LINE__));

                SendCurrentBurnStateToUpper(rtJob.GetUpServerIP(),rtJob.GetUpServerPort(),
                    strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","protocolparamerr"));

                CleanJob(strJobID);

                return;
            }

            BURN_PARAMETER param;
            param.nTaskRunningFlag=TASK_STATE_READY_TO_RUN;
            param.nTaskControlCmd=TASK_CONTROL_STATE_DEFAULT;
            param.nTaskExtraFlag=TASK_EXTRA_STATE_DEFAULT;
            param.strLastTaskID=strLastTaskID;
            param.burnMode=MEDIAFILE_BURN_STREAM;
            param.strTaskID=UUID_GetUUID();
            param.nIndex=0;
            param.vecDevHandle.push_back(GetDeviceHandle(iterDev->GetID()));

            for (size_t i=0;i<nLocationCount;++i)
            {
                std::string strLocationID=vecLocationInfo.at(i).GetID();

                BURN_DATA_SOURCE source;
                source.strType=SOURCE_TYPE_URL;
                source.strSourceUrl=vecLocationInfo.at(i).GetURL();

                if (strLocationID != "" &&
                    source.strSourceUrl != "")
                {
                    param.mapBurnDataSource[strLocationID].push_back(source);
                }
            }

            if (param.mapBurnDataSource.empty())
            {
                LOG_ERROR(("[BurnServerAgent::StartRTBurn] Stream source error,%d\r\n",__LINE__));

                SendCurrentBurnStateToUpper(rtJob.GetUpServerIP(),rtJob.GetUpServerPort(),
                    strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","protocolparamerr"));

                CleanJob(strJobID);

                return;
            }

            PushExtraDataSource(rtJob,param.mapBurnDataSource);

            param.strJobID=strJobID;
            param.strDiscLabel=rtJob.GetCommonBurnParam().GetCDName();
            param.strRtNoteUpdateFlag=NOTE_FILE_UPDATED_NO;
            param.strUpperIP=rtJob.GetUpServerIP();
            param.strUpperPort=rtJob.GetUpServerPort();
            param.strTaskPropertyFlag=TASK_PROPERTY_RT;

            param.strNewFileName=rtJob.GetNewFileName();
            std::string str=rtJob.GetBurnBufferSize();
            LOG_INFO(("[BurnServerAgent::StartRTBurn] BurnBufferSize :%s M, lineNum : %d\r\n",str.c_str(), __LINE__));
            if (str != "")
            {
                param.ulBufferSize=StringToInt(str)*1024*1024;
            }
            str=rtJob.GetCDAlarmLimit();
            if (str != "")
            {
                param.nAlarmSize=StringToInt(str);
            }

            if (!AddTask(param))
            {
                CleanJob(strJobID);
            }
        }

    }
    else if (BURN_TYPE_MULTI_ASYN == strBurnType)//多盘续刻
    {
        BURN_PARAMETER param;
        param.nTaskRunningFlag=TASK_STATE_READY_TO_RUN;
        param.nTaskControlCmd=TASK_CONTROL_STATE_DEFAULT;
        param.nTaskExtraFlag=TASK_EXTRA_STATE_ASYN_SINGLE;
        param.strLastTaskID=strLastTaskID;
        param.nIndex=0;
        param.burnMode=MEDIAFILE_BURN_STREAM;
        param.strTaskID=UUID_GetUUID();

        std::vector<CCDROMDriverInfo>::const_iterator iterDev;
        for (iterDev=vecDev.begin();iterDev!=vecDev.end();++iterDev)
        {
            param.vecDevHandle.push_back(GetDeviceHandle(iterDev->GetID()));
        }

        if ((int)vecDev.size() == 1)
        {//为了保证多盘续刻在只选择一个光驱的时候不触发下一个任务
            param.nTaskExtraFlag = TASK_EXTRA_STATE_DEFAULT;
        }
        const std::vector<CLocationInfo> &vecLocationInfo=vecDev.at(0).GetVecLocationInfo();
        size_t nLocationCount=vecLocationInfo.size();

        assert(nLocationCount > 0);

        if (0 == nLocationCount)
        {
            LOG_ERROR(("[BurnServerAgent::StartRTBurn] Location count error,%d\r\n",__LINE__));

            SendCurrentBurnStateToUpper(rtJob.GetUpServerIP(),rtJob.GetUpServerPort(),
                strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","protocolparamerr"));

            CleanJob(strJobID);

            return;
        }

        for (size_t i=0;i<nLocationCount;++i)
        {
            std::string strLocationID=vecLocationInfo.at(i).GetID();

            BURN_DATA_SOURCE source;
            source.strType=SOURCE_TYPE_URL;
            source.strSourceUrl=vecLocationInfo.at(i).GetURL();

            if (strLocationID != "" &&
                source.strSourceUrl != "")
            {
                param.mapBurnDataSource[strLocationID].push_back(source);
            }
        }

        if (param.mapBurnDataSource.empty())
        {
            LOG_ERROR(("[BurnServerAgent::StartRTBurn] Stream source error,%d\r\n",__LINE__));

            SendCurrentBurnStateToUpper(rtJob.GetUpServerIP(),rtJob.GetUpServerPort(),
                strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","protocolparamerr"));

            CleanJob(strJobID);

            return;
        }

        PushExtraDataSource(rtJob,param.mapBurnDataSource);

        param.strJobID=strJobID;
        param.strDiscLabel=rtJob.GetCommonBurnParam().GetCDName();
        param.strRtNoteUpdateFlag=NOTE_FILE_UPDATED_NO;
        param.strUpperIP=rtJob.GetUpServerIP();
        param.strUpperPort=rtJob.GetUpServerPort();
        param.strTaskPropertyFlag=TASK_PROPERTY_RT;

        param.strNewFileName=rtJob.GetNewFileName();
        std::string str=rtJob.GetBurnBufferSize();
        LOG_INFO(("[BurnServerAgent::StartRTBurn] BurnBufferSize :%s M, lineNum : %d\r\n",str.c_str(), __LINE__));
        if (str != "")
        {
            param.ulBufferSize=StringToInt(str)*1024*1024;
        }
        str=rtJob.GetCDAlarmLimit();
        if (str != "")
        {
            param.nAlarmSize=StringToInt(str);
        }

        if (!AddTask(param))
        {
            CleanJob(strJobID);
        }
    }
}

void BurnServerAgent::BurnCallBack(DEV_HANDLE handle,BURN_STATE_PARAMETER state,void *pUsr)
{
    if (pUsr != NULL)
    {
        BurnServerAgent *p=(BurnServerAgent *)pUsr;

        p->DealWithCallBackMsg(handle,state);
    }
}

void BurnServerAgent::BurnFilesCallBack(const DEV_BURN_FILES &devBurnFiles,void *pUsr)
{
    if (pUsr != NULL)
    {
        BurnServerAgent *p=(BurnServerAgent *)pUsr;

        p->AddDevBurnFilesInfo(devBurnFiles);
    }
}

void BurnServerAgent::SetWinJobBurnFileState(std::string strJobID,std::string strDevID,std::string strState)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormal;
    for (iterNormal=m_vecNormalJob.begin();iterNormal!=m_vecNormalJob.end();
        ++iterNormal)
    {
        if (iterNormal->GetJobID() == strJobID)
        {
            std::vector<CCDROMDriverInfo> &vecDev=iterNormal->GetBurnServerInfo().GetVecCDROMDriverInfo();
            std::vector<CCDROMDriverInfo>::iterator iterDev;
            for (iterDev=vecDev.begin();iterDev!=vecDev.end();++iterDev)
            {
                if (iterDev->GetID() == strDevID)
                {
                    std::vector<CLocationInfo> &vecLocation=iterDev->GetVecLocationInfo();
                    size_t nLocationCount=vecLocation.size();
                    for (size_t i=0;i<nLocationCount;++i)
                    {
                        std::vector<CSingleFileInfo> &vecFile=vecLocation.at(i).GetVecFileInfo();
                        size_t nFileCount=vecFile.size();
                        for (size_t j=0;j<nFileCount;++j)
                        {
                            vecFile.at(j).SetBurnState(strState);
                        }
                    }

                    break;
                }
            }

            break;
        }
    }
}

void BurnServerAgent::DealWithCallBackMsg(DEV_HANDLE handle,BURN_STATE_PARAMETER state)
{
    std::string strJobID            =   state.strJobID;
    std::string strTaskID           =   state.strTaskID;
    std::string strStateFlag        =   state.strStateFlag;
    std::string strExtraStateFlag   =   state.strExtraStateFlag;
    std::string strStateDescription =   state.strStateDescription;
    std::string strTaskPropertyFlag =   state.strTaskPropertyFlag;
    std::string strUpperIP          =   state.strUpperIP;
    std::string strUpperPort        =   state.strUpperPort;

    LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] StateInfo : strStateFlag %s, strExtraStateFlag %s, strUpperIP %s, strUpperPort %s, lineNum %d\r\n",
        state.strStateFlag.c_str(),state.strExtraStateFlag.c_str(),state.strUpperIP.c_str(),state.strUpperPort.c_str(),__LINE__));

    std::string strJobType          =   GetJobType(strJobID);
    std::string strJobControlState  =   GetRtJobControlState(strJobID);


    if (STATE_SINGLE_DEV_ERR == strStateFlag)//单个光驱异常导致刻录失败
    {
        LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] STATE_SINGLE_DEV_ERR,%d\r\n",__LINE__));

        SetTaskState(strJobID,strTaskID,strStateFlag,strStateDescription);

        SetDevBurnState(handle,strStateFlag,strStateDescription);

        //////////////////////////////////////////////////////////////////////////
        //实时刻录补刻时检测刻录设备失败，询问用户是否重启后刻录
        if (EXTRA_STATE_RTBURN_BACKUP_RESTART_YES == strExtraStateFlag)
        {
            LOG_WARNING(("[BurnServerAgent::DealWithCallBackMsg] Will remind user %s,%s restart dev,%d\r\n",
                state.strUpperIP.c_str(),state.strUpperPort.c_str(),__LINE__));

            SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                state.strJobID,state.strExtraStateFlag,strStateDescription);

            return;
        }
        //////////////////////////////////////////////////////////////////////////

        int nRet=IsJobDone(strJobID);
        if (-1 == nRet)
        {
            LOG_ERROR(("[BurnServerAgent::DealWithCallBackMsg] Job %s failed,%d\r\n",
                strJobID.c_str(),__LINE__));

            SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                strJobID,STATE_BURN_ERR,strStateDescription);

            CleanJob(strJobID);

            if (JOB_TYPE_NORMAL == strJobType)
            {
                CleanJobData(strJobID);
            }
            else if (JOB_TYPE_RT == strJobType &&
                !m_BurnTask.HasRTBurnBackupTask(strJobID))
            {
                CleanJobData(strJobID);

//                 CleanRTJobBackupData(strJobID);
            }
        }
        else if (2 == nRet)
        {
            LOG_WARNING(("[BurnServerAgent::DealWithCallBackMsg] Job %s finished,%d\r\n",
                strJobID.c_str(),__LINE__));

            SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                state.strJobID,STATE_JOB_FINISHED,gBurnStateCfgFile.GetValue("info","justburndone"));

            CleanJob(strJobID);

            if (JOB_TYPE_NORMAL == strJobType)
            {
                CleanJobData(strJobID);
            }
        }
        else if(1 == nRet)
        {
            //增加正在停止和正在暂停的描述信息

            std::string strT;
            if (RT_CONTROL_STATE_PAUSE == strJobControlState)
            {
                //strT=gBurnStateCfgFile.GetValue("info","pausing");
            } 
            else if(RT_CONTROL_STATE_STOP == strJobControlState)
            {
                //strT=gBurnStateCfgFile.GetValue("info","stopping");
            }

            if (strT != "" &&
                strStateDescription != "")
            {
                strT+=",";
            }

            SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                strJobID,STATE_BURNING,gBurnStateCfgFile.GetValue("info","burning"),true);
        }
        else
        {
            assert(false);
        }
    }
    else if (STATE_WILL_CLOSE_DISC == strStateFlag)
    {
        //实时刻录准备封盘
        LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] Will close disc,it's time to update note, \
                  job : %s,%d\r\n",strJobID.c_str(),__LINE__));

        SetTaskState(strJobID,strTaskID,strStateFlag,strStateDescription);

        std::vector<CRTBurnInfoEx>::iterator iterRtJob;
        for (iterRtJob=m_vecRTBurnInfo.begin();iterRtJob!=m_vecRTBurnInfo.end();++iterRtJob)
        {
            if (strJobID == iterRtJob->GetJobID())
            {
                std::string strNoteFileRemoteIP=iterRtJob->GetNoteFileRemoteIP();
                std::string strNoteFileRemotePort=iterRtJob->GetNoteFileRemotePort();
                std::string strRemotePath=iterRtJob->GetNoteFileDownloadURL();

                std::string strHostIP=BurnServerAgent::GetHostIP();

                LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] %s,%s,%s,%s,%d\r\n",
                    strHostIP.c_str(),strNoteFileRemoteIP.c_str(),strNoteFileRemotePort.c_str(),
                    strRemotePath.c_str(),__LINE__));

                if (strHostIP == strNoteFileRemoteIP)
                {
                    std::string strLocalDownloadDir=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)
                        +iterRtJob->GetJobID(),true);

                    LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] Copy note file,%d\r\n",__LINE__));
#ifdef WIN32
#else
                    if (FileUtil::FileExist(strRemotePath.c_str()))
                    {
                        std::vector<std::string> vecFiles;
                        vecFiles.push_back(strRemotePath);
                        DirectoryUtil::CopyFileToDir(strLocalDownloadDir.c_str(),vecFiles);
                    }
#endif
                } 
                else
                {
                    if (strNoteFileRemoteIP != "" &&
                        strNoteFileRemotePort != "" &&
                        strRemotePath != "")
                    {
                        std::string strNoteFileUpdateFlag=m_BurnTask.GetNoteUpdateFlag(strTaskID);
                        if (NOTE_FILE_UPDATED_YES == strNoteFileUpdateFlag)
                        {
                            LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] Note file already updated, \
                                      not need to update now,%d\r\n",__LINE__));
                        } 
                        else
                        {
                            LOG_INFO(("[[BurnServerAgent::DealWithCallBackMsg]] Will download note file,%d\r\n",
                                __LINE__));

                            m_BurnTask.SetNoteUpdateFlagByTaskID(strTaskID,
                                NOTE_FILE_UPDATED_NO);

                            SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                                strJobID,strStateFlag,gBurnStateCfgFile.GetValue("info","updatingnote"));

                            DownloadFile(iterRtJob->GetJobID(),RT_DOWNLOAD_FLAG_ONLY_NOTE_FILE);
                        }
                    }
                    else
                    {
                        //实时刻录可能不需要刻录笔录
                        m_BurnTask.SetNoteUpdateFlagByTaskID(strTaskID,NOTE_FILE_UPDATED_YES);
                        LOG_WARNING(("[BurnServerAgent::BurnCallBack] Will not update note file,%d\r\n",
                            __LINE__));
                    }
                }

                break;
            }
        }
    }
    else if (STATE_SINGLE_DISC_BURNED == strStateFlag)
    {
        //单张光盘刻录完成

        LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] Single disc burn success\r\n"));

        SetDevBurnState(handle,strStateFlag,strStateDescription);
        SetTaskState(strJobID,strTaskID,strStateFlag,strStateDescription);

        if (IsJobStopping(strJobID))
        {
            SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,strJobID,
                STATE_RTTASK_STOPPING,gBurnStateCfgFile.GetValue("info","stopping"),true);
        }
        else
        {
            int nRet=IsJobDone(strJobID);
            if (-1 == nRet)
            {
                LOG_ERROR(("[BurnServerAgent::DealWithCallBackMsg] Job %s failed,%d\r\n",
                    strJobID.c_str(),__LINE__));

                SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                    strJobID,STATE_BURN_ERR,strStateDescription);

                CleanJob(strJobID);

                if (JOB_TYPE_NORMAL == strJobType)
                {
                    CleanJobData(strJobID);
                }
                else if (JOB_TYPE_RT == strJobType &&
                    !m_BurnTask.HasRTBurnBackupTask(strJobID))
                {
                    CleanJobData(strJobID);
                }
            }
            else if (2 == nRet)
            {
                LOG_WARNING(("[BurnServerAgent::DealWithCallBackMsg] Job %s finished,%d\r\n",
                    strJobID.c_str(),__LINE__));

                SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                    state.strJobID,STATE_JOB_FINISHED,gBurnStateCfgFile.GetValue("info","justburndone"));

                CleanJob(strJobID);

                if (JOB_TYPE_NORMAL == strJobType)
                {
                    CleanJobData(strJobID);
                }
            }
            else if(1 == nRet)
            {
                SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                    strJobID,STATE_BURNING,gBurnStateCfgFile.GetValue("info","burning"),true);
            }
            else if (7 == nRet)
            {
                SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,strJobID,
                    strStateFlag,strStateDescription,true);
            }
        }
    }
    else if (STATE_BURNING == strStateFlag)
    {
        //正常刻录阶段的状态信息

        SetDevBurnState(handle,strStateFlag,strStateDescription);
        SetTaskState(strJobID,strTaskID,strStateFlag,strStateDescription);

        std::string strT;

        if (RT_CONTROL_STATE_PAUSE == strJobControlState)
        {
            //strT=gBurnStateCfgFile.GetValue("info","pausing");
        } 
        else if(RT_CONTROL_STATE_STOP == strJobControlState)
        {
            //strT=gBurnStateCfgFile.GetValue("info","stopping");
        }

        if (strT != "" &&
            strStateDescription != "")
        {
            strT+=",";
        }

#ifdef WIN32
        //Windows下设置所有文件为正在刻录的状态
#else
#endif
        if (IsJobRunning(strJobID))
        {
            if (IsJobStopping(strJobID))
            {
                SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,strJobID,
                    STATE_RTTASK_STOPPING,gBurnStateCfgFile.GetValue("info","stopping"),true);
            }
            else
            {
                SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,strJobID,
                    strStateFlag,strT+strStateDescription,true);
            }
        }
    }
    else if (STATE_RTTASK_STOPPING == strStateFlag)
    {
        SetDevBurnState(handle,strStateFlag,strStateDescription);

        SetTaskState(strJobID,strTaskID,strStateFlag,strStateDescription);

        SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,strJobID,
            strStateFlag,strStateDescription,true);
    }
    else if (STATE_TASK_OVER == strStateFlag)
    {
        //一个刻录task成功被执行

        if (TASK_PROPERTY_RT_BACKUP == strTaskPropertyFlag)
        {
#ifdef WIN32
#else
            SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                state.strJobID,STATE_JOB_OVER,gBurnStateCfgFile.GetValue("info","burnsuccess"));

            //删除刻录备份的xml文件
            CleanRTBurnBackupXML(strJobID);

            std::string strBackupDir=BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerRTBackupDir"), true)+strJobID+"/";

            LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] Will delete dir : %s,%s,%d\r\n",
                strBackupDir.c_str(),strJobID.c_str(),__LINE__));
            DirectoryUtil::DeleteDir(strBackupDir.c_str());

            return;
#endif
        }

        LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] Job type : %s,%d\r\n",
            strJobType.c_str(),__LINE__));

        SetDevBurnState(handle,strStateFlag,gBurnStateCfgFile.GetValue("info","burnsuccess"));

        SetTaskState(strJobID,strTaskID,strStateFlag,strStateDescription);

        LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] Single task burn success\r\n"));

        int nRet=IsJobDone(state.strJobID);

        switch (nRet)
        {
        case -1:
            break;

        case 1:
            break;

        case 2:
            {
                if (JOB_TYPE_RT == strJobType)
                {
                    if (RT_CONTROL_STATE_STOP == GetRtJobControlState(strJobID))
                    {
                        LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] Rt job %s is stopped\r\n",
                            state.strJobID.c_str()));
                    }
                    else
                    {
                        LOG_WARNING(("[BurnServerAgent::DealWithCallBackMsg] Job %s finished,%d\r\n",
                            strJobID.c_str(),__LINE__));
                    }

                    SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                        state.strJobID,STATE_JOB_FINISHED,gBurnStateCfgFile.GetValue("info","justburndone"));

                    CleanJob(strJobID);
                }
                else if(JOB_TYPE_NORMAL == strJobType)
                {
                    LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] Job %s is done,but not perfect,%d\r\n",
                        strJobID.c_str(),__LINE__));

                    SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                        state.strJobID,STATE_JOB_FINISHED,gBurnStateCfgFile.GetValue("info","justburndone"));

                    CleanJob(strJobID);
                    CleanJobData(strJobID);
                }
            }
            break;

        case 7:
            {
                if (JOB_TYPE_RT == strJobType)
                {
                    if (RT_CONTROL_STATE_STOP == GetRtJobControlState(strJobID))
                    {
                        LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] Rt job %s is stopped\r\n",
                            state.strJobID.c_str()));

                        SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                            state.strJobID,STATE_JOB_OVER,gBurnStateCfgFile.GetValue("info","burnsuccess"));

                        CleanJob(strJobID);
                    }
                    else
                    {
                        LOG_WARNING(("[BurnServerAgent::DealWithCallBackMsg] Job %s finished,%d\r\n",
                            strJobID.c_str(),__LINE__));

                        SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                            state.strJobID,STATE_JOB_FINISHED,gBurnStateCfgFile.GetValue("info","justburndone"));

                        CleanJob(strJobID);
                    }
                }
                else if (JOB_TYPE_NORMAL == strJobType)
                {
                    LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] OK,job %s is done,%d\r\n",
                        strJobID.c_str(),__LINE__));

                    SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                        state.strJobID,STATE_JOB_OVER,gBurnStateCfgFile.GetValue("info","burnsuccess"));

                    CleanJob(strJobID);
                    CleanJobData(strJobID);
                }
            }
            break;

        default:
            break;
        }

    }
    else if (STATE_RTTASK_PAUSED == strStateFlag)
    {
        //单个实时task已经挂起

        SetDevBurnState(handle,strStateFlag,gBurnStateCfgFile.GetValue("info","paused"));

        SetTaskState(strJobID,strTaskID,strStateFlag,strStateDescription);

        if (IsJobPaused(strJobID))
        {
            LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] Job %s is paused\r\n",state.strJobID.c_str()));

            SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                state.strJobID,STATE_RTJOB_PAUSED,gBurnStateCfgFile.GetValue("info","paused"),true);
        }
    }
    else if (STATE_RTTASK_STOPPED == strStateFlag)
    {
        //单个实时task已经停止

        SetDevBurnState(handle,strStateFlag,gBurnStateCfgFile.GetValue("info","stopped"));

        SetTaskState(strJobID,strTaskID,strStateFlag,strStateDescription);

        if (IsJobStopped(strJobID))
        {
            LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] Job %s is stopped,%d\r\n",
                state.strJobID.c_str(),__LINE__));

            SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
                state.strJobID,STATE_RTJOB_STOPPED,gBurnStateCfgFile.GetValue("info","stopped"));

            CleanJob(strJobID);
            CleanJobData(strJobID);
        }

        CleanRTBurnBackupXML(strJobID);
        CleanRTJobBackupData(strJobID);
    }
    else if (STATE_WILL_NO_SPACE == strStateFlag)
    {
        //一个光盘即将到达光盘门限，启动下一个光盘刻录任务
        CRTBurnInfoEx rtBurnInfo;
        if (GetRtJobInfo(strJobID, rtBurnInfo))
        {
            LOG_INFO(("[BurnServerAgent::DealWithCallBackMsg] STATE_WILL_NO_SPACE, Another Task ofjobID: %s will start, %d\r\n"
                ,strJobID.c_str(),__LINE__));
            StartRTBurn(rtBurnInfo, strTaskID);
        }
        else
        {
            LOG_ERROR(("[BurnServerAgent::DealWithCallBackMsg] Can not find RtJob info of jobID: %s, %d\r\n",strJobID.c_str(),__LINE__));
        }
    }
    if (EXTRA_STATE_RTBURN_BACKUP_YES == state.strExtraStateFlag)
    {
        SendCurrentBurnStateToUpper(state.strUpperIP,state.strUpperPort,
            state.strJobID,state.strExtraStateFlag,strStateDescription);
    }
}

void BurnServerAgent::StartNormalBurn(const CNormalBurnJobInfoEx &normalJob)
{
    std::string strJobID=normalJob.GetJobID();
    std::string strBurnType=normalJob.GetCommonBurnParam().GetBurnType();
    std::string strBurnServerType=normalJob.GetBurnServerInfo().GetBurnServerType();

    std::string strUpServerIP=normalJob.GetUpServerIP();
    std::string strUpServerPort=normalJob.GetUpServerPort();

    LOG_INFO(("[BurnServerAgent::StartNormalBurn] Job id : %s,burn type : %s,burn server type : %s\r\n",
        strJobID.c_str(),strBurnType.c_str(),strBurnServerType.c_str()));

    CLocationInfo locationInfo;
    std::vector<CLocationInfo> vecLocationInfo;
    INT64 nFileSize=0;

    if (BURNSERVER_TYPE_NORMAL == strBurnServerType)
    {//调用普通刻录接口

        if (BURN_TYPE_SINGLE == strBurnType)
        {//单盘刻录

            LOG_INFO(("[BurnServerAgent::StartNormalBurn] Single dev burn,%d\r\n",__LINE__));

            size_t nDevCount=normalJob.GetBurnServerInfo().GetVecCDROMDriverInfo().size();
            assert(1 == nDevCount);

            if (0 == nDevCount)
            {
                LOG_ERROR(("[BurnServerAgent::StartNormalBurn] Dev count error,%d\r\n",__LINE__));

                SendCurrentBurnStateToUpper(normalJob.GetUpServerIP(),normalJob.GetUpServerPort(),
                    strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","protocolparamerr"));

                CleanJob(strJobID);

                return;
            }

            const CCDROMDriverInfo &dev=normalJob.GetBurnServerInfo().GetVecCDROMDriverInfo().at(0);

            BURN_PARAMETER burnParam;
            burnParam.strTaskID=UUID_GetUUID();
            burnParam.vecDevHandle.push_back(GetDeviceHandle(dev.GetID()));
            burnParam.burnMode=MEDIAFILE_BURN_LOCAL_FILE;
            burnParam.nIndex = 0;

            size_t nLocationCount=dev.GetVecLocationInfo().size();
            for (size_t i=0;i<nLocationCount;++i)
            {
                locationInfo=dev.GetVecLocationInfo().at(i);

                std::string strLocationID=locationInfo.GetID();

                BURN_DATA_SOURCE source;

                size_t nFileCount=locationInfo.GetVecFileInfo().size();
                for (size_t j=0;j<nFileCount;++j)
                {
                    source.strType=SOURCE_TYPE_NORMAL_FILE;
                    source.strSourceUrl=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)
                        +strJobID+"/"+strJobID+"/"+locationInfo.GetVecFileInfo().at(j).GetFileName(),false);

                    nFileSize=GetFileOrDirSize(source.strSourceUrl,false);
                    if (nFileSize > 0)
                    {
                        burnParam.mapBurnDataSource[strLocationID].push_back(source);
                        LOG_INFO(("[BurnServerAgent::StartNormalBurn] Add file : %s,%lld,%d\r\n",
                            source.strSourceUrl.c_str(),nFileSize,__LINE__));
                    }
                    else
                    {
                        LOG_WARNING(("[BurnServerAgent::StartNormalBurn] Not add file,%s,%lld,%d\r\n",
                            source.strSourceUrl.c_str(),nFileSize,__LINE__));
                    }

                    locationInfo.GetVecFileInfo().at(j).SetSize(IntToString(static_cast<int>(nFileSize)));
                }

                vecLocationInfo.push_back(locationInfo);
            }

            PushExtraDataSource(normalJob,burnParam.mapBurnDataSource);

            burnParam.nTaskRunningFlag=TASK_STATE_READY_TO_RUN;
            burnParam.strJobID=strJobID;
            burnParam.strDiscLabel=normalJob.GetCommonBurnParam().GetCDName();
            burnParam.strUpperIP=normalJob.GetUpServerIP();
            burnParam.strUpperPort=normalJob.GetUpServerPort();

            if (!AddTask(burnParam))
            {
                CleanJob(strJobID);
            }
            else
            {
                AddDevBurnFilesInfo(normalJob.GetJobID(),dev.GetID(),vecLocationInfo);

                //for test
#if 0
                SendCurrentBurnStateToUpper(normalJob.GetUpServerIP(),normalJob.GetUpServerPort(),
                    normalJob.GetJobID(),STATE_BURNING,gBurnStateCfgFile.GetValue("info","burning"),true);
#endif
            }
        } 
        else if(BURN_TYPE_MULTI_SYN == strBurnType)
        {
            //多盘同刻
            size_t nDevCount=normalJob.GetBurnServerInfo().GetVecCDROMDriverInfo().size();

            assert(nDevCount > 0);

            if (0 == nDevCount)
            {
                LOG_ERROR(("[BurnServerAgent::StartNormalBurn] Dev count error,%d\r\n",__LINE__));

                SendCurrentBurnStateToUpper(normalJob.GetUpServerIP(),normalJob.GetUpServerPort(),
                    strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","protocolparamerr"));

                CleanJob(strJobID);

                return;
            }

            const std::vector<CCDROMDriverInfo> &vecDev=normalJob.GetBurnServerInfo().GetVecCDROMDriverInfo();
            std::vector<CCDROMDriverInfo>::const_iterator iterDev;
            for (iterDev=vecDev.begin();iterDev!=vecDev.end();++iterDev)
            {
                const CCDROMDriverInfo &CDROMDriverInfo=*iterDev;

                BURN_PARAMETER burnParam;
                burnParam.strTaskID=UUID_GetUUID();
                burnParam.vecDevHandle.push_back(GetDeviceHandle(CDROMDriverInfo.GetID()));
                burnParam.burnMode=MEDIAFILE_BURN_LOCAL_FILE;
                burnParam.nIndex = 0;

                size_t nLocationCount=CDROMDriverInfo.GetVecLocationInfo().size();
                for (size_t i=0;i<nLocationCount;++i)
                {
                    locationInfo=CDROMDriverInfo.GetVecLocationInfo().at(i);

                    std::string strLocationID=locationInfo.GetID();

                    BURN_DATA_SOURCE source;

                    size_t nFileCount=locationInfo.GetVecFileInfo().size();
                    for (size_t j=0;j<nFileCount;++j)
                    {
                        source.strType=SOURCE_TYPE_NORMAL_FILE;
                        source.strSourceUrl=
                            BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)+strJobID+"/"+strJobID+"/"+
                            locationInfo.GetVecFileInfo().at(j).GetFileName(),false);

                        nFileSize=GetFileOrDirSize(source.strSourceUrl,false);
                        if (nFileSize > 0)
                        {
                            burnParam.mapBurnDataSource[strLocationID].push_back(source);

                            LOG_INFO(("[BurnServerAgent::StartNormalBurn] Add file : %s\r\n",
                                source.strSourceUrl.c_str()));
                        }
                        else
                        {
                            LOG_WARNING(("[BurnServerAgent::StartNormalBurn] Not add file,%s,%lld,%d\r\n",
                                source.strSourceUrl.c_str(),nFileSize,__LINE__));
                        }

                        locationInfo.GetVecFileInfo().at(j).SetSize(IntToString(static_cast<int>(nFileSize)));
                    }

                    vecLocationInfo.push_back(locationInfo);
                }

                PushExtraDataSource(normalJob,burnParam.mapBurnDataSource);

                burnParam.nTaskRunningFlag=TASK_STATE_READY_TO_RUN;

                burnParam.strJobID=strJobID;
                burnParam.strDiscLabel=normalJob.GetCommonBurnParam().GetCDName();
                burnParam.strUpperIP=normalJob.GetUpServerIP();
                burnParam.strUpperPort=normalJob.GetUpServerPort();

                if (!AddTask(burnParam))
                {
                    CleanJob(strJobID);
                }
                else
                {
                    AddDevBurnFilesInfo(normalJob.GetJobID(),CDROMDriverInfo.GetID(),vecLocationInfo);
                }
            }
        }
        else if (BURN_TYPE_MULTI_ASYN == strBurnType)
        {
            //多盘续刻

            size_t nDevCount=normalJob.GetBurnServerInfo().GetVecCDROMDriverInfo().size();

            assert(nDevCount > 0);

            if (0 == nDevCount)
            {
                LOG_ERROR(("[BurnServerAgent::StartNormalBurn] Dev count error,%d\r\n",__LINE__));

                SendCurrentBurnStateToUpper(normalJob.GetUpServerIP(),normalJob.GetUpServerPort(),
                    strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","protocolparamerr"));

                CleanJob(strJobID);

                return;
            }

            //认为每一个光驱类对象保存了相同的数据
            const CCDROMDriverInfo &dev=normalJob.GetBurnServerInfo().GetVecCDROMDriverInfo().at(0);

            BURN_PARAMETER burnParam;
            burnParam.strTaskID=UUID_GetUUID();

            const std::vector<CCDROMDriverInfo> &vecDev=normalJob.GetBurnServerInfo().GetVecCDROMDriverInfo();
            std::vector<CCDROMDriverInfo>::const_iterator iterDev;
            for (iterDev=vecDev.begin();iterDev!=vecDev.end();++iterDev)
            {
                burnParam.vecDevHandle.push_back(GetDeviceHandle(iterDev->GetID()));

                LOG_INFO(("[BurnServerAgent::StartNormalBurn] Multi continuous push handle : %p\r\n",GetDeviceHandle(iterDev->GetID())));
            }

            burnParam.nIndex = 0;
            burnParam.burnMode=MEDIAFILE_BURN_LOCAL_FILE;

            size_t nLocationCount=dev.GetVecLocationInfo().size();
            for (size_t i=0;i<nLocationCount;++i)
            {
                locationInfo=dev.GetVecLocationInfo().at(i);

                std::string strLocationID=locationInfo.GetID();

                BURN_DATA_SOURCE source;

                size_t nFileCount=locationInfo.GetVecFileInfo().size();
                for (size_t j=0;j<nFileCount;++j)
                {
                    source.strType=SOURCE_TYPE_NORMAL_FILE;
                    source.strSourceUrl=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)+
                        strJobID+"/"+strJobID+"/"+locationInfo.GetVecFileInfo().at(j).GetFileName(),false);

                    nFileSize=GetFileOrDirSize(source.strSourceUrl,false);
                    if (nFileSize > 0)
                    {
                        burnParam.mapBurnDataSource[strLocationID].push_back(source);

                        LOG_INFO(("[BurnServerAgent::StartNormalBurn] Multi continuous add file : %s\r\n",
                            source.strSourceUrl.c_str()));
                    } 
                    else
                    {
                        LOG_WARNING(("[BurnServerAgent::StartNormalBurn] Not add file,%s,%lld,%d\r\n",
                            source.strSourceUrl.c_str(),nFileSize,__LINE__));
                    }

                    locationInfo.GetVecFileInfo().at(j).SetSize(IntToString(static_cast<int>(nFileSize)));
                }

                vecLocationInfo.push_back(locationInfo);
            }

            PushExtraDataSource(normalJob,burnParam.mapBurnDataSource);

            burnParam.nTaskRunningFlag=TASK_STATE_READY_TO_RUN;

            burnParam.strJobID=strJobID;
            burnParam.strDiscLabel=normalJob.GetCommonBurnParam().GetCDName();
            burnParam.strUpperIP=normalJob.GetUpServerIP();
            burnParam.strUpperPort=normalJob.GetUpServerPort();

            if (!AddTask(burnParam))
            {
                CleanJob(strJobID);
            }
            else
            {
                //多盘续刻的文件状态信息用第一个光驱承载
                AddDevBurnFilesInfo(normalJob.GetJobID(),vecDev.at(0).GetID(),vecLocationInfo);
            }
        }
        else
        {
            LOG_ERROR(("[BurnServerAgent::StartNormalBurn] Unknown burn type\r\n"));
        }
    }
    else if (BURNSERVER_TYPE_PRIMERA == strBurnServerType || BURNSERVER_TYPE_EPSON == strBurnServerType)
    {
#ifdef WIN32
        INT64 nDiscFreeSize;
        if (BURNSERVER_TYPE_PRIMERA == strBurnServerType)
        {
            nDiscFreeSize = StringToInt(gBasicParamCfgFile.GetValue("info","burnServerPrimeraDiscSize"));
            LOG_INFO(("[BurnServerAgent::StartNormalBurn] BURNSERVER_TYPE_PRIMERA nDiscFreeSize : %d\r\n",nDiscFreeSize));
        }
        else
        {
            nDiscFreeSize = StringToInt(gBasicParamCfgFile.GetValue("info","burnServerEpsonDiscSize"));
            LOG_INFO(("[BurnServerAgent::StartNormalBurn] BURNSERVER_TYPE_EPSON nDiscFreeSize : %d\r\n",nDiscFreeSize));
        }

        std::string strPlayListFullPath;

        std::vector< std::vector<BURN_DATA_SOURCE> > vecSingleDiscDataSource;

        std::vector<BURN_DATA_SOURCE> vecSingleTaskDataSource;//保存每一次轮询添加的数据
        INT64 nSingleTaskSize=0;
        INT64 nMemLastPartSize=0;
        INT64 nTemp=0;
        bool bAddOver=true;

        std::vector<BURN_DATA_SOURCE> vecSinglePollDataSource;
        int locationIndex=-1;

        BURN_DATA_SOURCE dataSource;
        //分配刻录数据
        while (true)
        {
            bAddOver=true;

            ++locationIndex;

            vecSinglePollDataSource.clear();

            if (0 == nMemLastPartSize)
            {
                //加入附加信息
                std::string strFilePath;

                //笔录
                if (normalJob.GetNoteFileName() != "")
                {
                    strFilePath=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)
                        +strJobID+"/"+strJobID+"/"+normalJob.GetNoteFileName(),false);

                    dataSource.strType = SOURCE_TYPE_NORMAL_FILE;
                    dataSource.strSourceUrl = strFilePath;
                    vecSinglePollDataSource.push_back(dataSource);
                    LOG_INFO(("[BurnServerAgent::StartNormalBurn] Add note : %s\r\n",strFilePath.c_str()));
                }
                else
                {
                    LOG_WARNING(("[BurnServerAgent::StartNormalBurn] No note\r\n"));
                }


                //播放器
                strFilePath=DirectoryUtil::EnsureNoSlashEnd(BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)+strJobID+"/"+"PlayBack",true));
                if (DirectoryUtil::GetDirSize(strFilePath.c_str()) > 0)
                {
                    dataSource.strType = SOURCE_TYPE_DIR;
                    dataSource.strSourceUrl = strFilePath;
                    vecSinglePollDataSource.push_back(dataSource);
                    LOG_INFO(("[BurnServerAgent::StartNormalBurn] Add player : %s\r\n",strFilePath.c_str()));
                } 
                else
                {
                    LOG_WARNING(("[BurnServerAgent::StartNormalBurn] No player,path : %s,size : %d\r\n",
                        strFilePath.c_str(),DirectoryUtil::GetDirSize(strFilePath.c_str())));
                }


                //自动运行文件
                if (normalJob.GetAutorunFilePath() != "")
                {
                    strFilePath=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)+strJobID+
                        "/AutoRun.inf",false);

                    dataSource.strType = SOURCE_TYPE_NORMAL_FILE;
                    dataSource.strSourceUrl = strFilePath;
                    vecSinglePollDataSource.push_back(dataSource);

                    LOG_INFO(("[BurnServerAgent::StartNormalBurn] Add autorun : %s\r\n",strFilePath.c_str()));
                }
                else
                {
                    LOG_WARNING(("[BurnServerAgent::StartNormalBurn] No autorun\r\n"));
                }

                // playlist文件
                if (normalJob.GetPlaylistFileInfo().GetDownloadURL() != "")
                {
                    strFilePath=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)+strJobID+"/"+strJobID+
                        "/"+ExtractFileOrDirName(BurnTask::Path(normalJob.GetPlaylistFileInfo().GetDownloadURL(),false)),false);

                    strPlayListFullPath = strFilePath;

                    LOG_INFO(("[BurnServerAgent::StartNormalBurn] Add play list file : %s\r\n",normalJob.GetPlaylistFileInfo().GetDownloadURL().c_str()));
                } 
                else
                {
                    LOG_WARNING(("[BurnServerAgent::StartNormalBurn] No play list\r\n"));
                }

                //光盘访问控制文件
                strFilePath=CharsetConvertMFC::UTF16ToUTF8(CStringW(CStringA(GetCurDir().c_str()))).GetBuffer()+std::string("Auth.exe");
                dataSource.strType = SOURCE_TYPE_NORMAL_FILE;
                dataSource.strSourceUrl = strFilePath;
                vecSinglePollDataSource.push_back(dataSource);
            }


            const CCDROMDriverInfo &dev=normalJob.GetBurnServerInfo().GetVecCDROMDriverInfo().at(0);

            size_t nLocationCount=dev.GetVecLocationInfo().size();
            for (size_t i=0;i<nLocationCount;++i)
            {
                locationInfo=dev.GetVecLocationInfo().at(i);

                std::string strLocationID=locationInfo.GetID();
                std::string strLocationPath;

                int nFileCount=locationInfo.GetVecFileInfo().size();
                LOG_INFO(("[BurnServerAgent::StartNormalBurn] Location file count : %d\r\n",nFileCount));

                //收集每个画面的同一位置的文件
                if (nFileCount - 1 > locationIndex)
                {
                    bAddOver=false;
                }

                if (nFileCount > locationIndex)
                {
                    strLocationPath=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)
                        +strJobID+"/"+strJobID+"/"+locationInfo.GetVecFileInfo().at(locationIndex).GetFileName(),false);

                    INT64 nSizeT=GetFileOrDirSize(strLocationPath,false);

                    nSingleTaskSize+=nSizeT;

                    dataSource.strType = SOURCE_TYPE_NORMAL_FILE;
                    dataSource.strSourceUrl = strLocationPath;
                    vecSinglePollDataSource.push_back(dataSource);


                    LOG_INFO(("[BurnServerAgent::StartNormalBurn] Prepare file(dir) : %s,%lld\r\n",
                        strLocationPath.c_str(),nSizeT));
                }
            }

            if (nSingleTaskSize/1024/1024 < nDiscFreeSize)
            {
                if (bAddOver)
                {
                    while(!vecSinglePollDataSource.empty())
                    {
                        vecSingleTaskDataSource.push_back(vecSinglePollDataSource.back());

                        vecSinglePollDataSource.pop_back();
                    }

                    vecSingleDiscDataSource.push_back(vecSingleTaskDataSource);

                    vecSingleTaskDataSource.clear();
                    nSingleTaskSize=0;
                    nMemLastPartSize=0;
                    bAddOver=true;

                    LOG_INFO(("[BurnServerAgent::StartNormalBurn] ~~~~ Add over ~~~~\r\n"));
                }
                else
                {
                    // Try to add next location
                    LOG_INFO(("[BurnServerAgent::StartNormalBurn] ~~~~ Next poll ~~~~,%d\r\n",__LINE__));

                    nMemLastPartSize=nSingleTaskSize;

                    while(!vecSinglePollDataSource.empty())
                    {
                        vecSingleTaskDataSource.push_back(vecSinglePollDataSource.back());

                        vecSinglePollDataSource.pop_back();
                    }

                    continue;
                }
            }
            else if (nSingleTaskSize/1024/1024 > nDiscFreeSize)
            {
                if (0 == nMemLastPartSize)
                {
                    LOG_ERROR(("[BurnServerAgent::StartNormalBurn] No space,free : %lu M,data size : %d M\r\n",
                        nDiscFreeSize,nSingleTaskSize/1024/1024));

                    SendCurrentBurnStateToUpper(normalJob.GetUpServerIP(),normalJob.GetUpServerPort(),
                        strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","tomuchdata"));

                    CleanJob(strJobID);

                    return ;
                }
                else
                {
                    vecSingleDiscDataSource.push_back(vecSingleTaskDataSource);

                    vecSingleTaskDataSource.clear();
                    nSingleTaskSize=0;
                    nMemLastPartSize=0;
                    bAddOver=false;

                    --locationIndex;

                    LOG_INFO(("[BurnServerAgent::StartNormalBurn] ^^^^ Next poll ^^^^\r\n"));

                    continue;
                }
            }

            if (bAddOver)
            {
                break;
            }
        }//while(true)

        //更新Playlist
        for (int i=0;i<(int)vecSingleDiscDataSource.size();i++)
        {
            if (strPlayListFullPath != "" &&
                BurnTask::InsertPlayListFile(vecSingleDiscDataSource.at(i),strPlayListFullPath))
            {
                LOG_INFO(("[BurnServerAgent::StartNormalBurn] CreatePlayListFile succes\r\n"));
            }
            else
            {
                LOG_ERROR(("[BurnServerAgent::StartNormalBurn] CreatePlayListFile failed,%d\r\n",__LINE__));

                assert(false);
            }
        }

        LOG_INFO(("[BurnServerAgent::StartNormalBurn] ==================================================\r\n"));
        for (int i=0;i<(int)vecSingleDiscDataSource.size();i++)
        {
            LOG_INFO(("Disc %d : \r\n",i+1));

            const std::vector<BURN_DATA_SOURCE> &vecDataSource=vecSingleDiscDataSource.at(i);
            for (int j=0;j<(int)vecDataSource.size();j++)
            {
                LOG_INFO(("file %d : %s\r\n",j+1,vecDataSource.at(j).strSourceUrl.c_str()));
            }

            LOG_INFO(("\r\n"));
        }
        LOG_INFO(("[BurnServerAgent::StartNormalBurn] ==================================================\r\n"));

#endif

        if (BURNSERVER_TYPE_PRIMERA == strBurnServerType)
        {
            //调用派美雅刻录接口
#ifdef WIN32
            bool bJobSucess = true;
            std::string strCDName = normalJob.GetCommonBurnParam().GetCDName();
            CNormalBurnJobInfoEx normalJobBurn = normalJob;
            for (int i = 0; i < (int)vecSingleDiscDataSource.size(); i++)
            {
                std::vector<std::string> vectDataSource;
                for (int j = 0; j < (int)vecSingleDiscDataSource.at(i).size(); j++)
                {
                    vectDataSource.push_back(vecSingleDiscDataSource.at(i).at(j).strSourceUrl);
                }
                if ((int)vecSingleDiscDataSource.size() > 1)
                {//多盘重设光盘名称
                    normalJobBurn.GetCommonBurnParam().SetCDName(strCDName + "_" + IntToString(i+1));
                }
                std::string strRet=GeneratePrimeraBurnJob(normalJobBurn,vectDataSource);
                if ("" == strRet)
                {
                    SendCurrentBurnStateToUpper(strUpServerIP,strUpServerPort,
                        strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","generatejobfilefailed"),false);
                }
                else
                {
                    AttachPrimeraJobID(strJobID,strRet);

                    while (true)
                    {
                        std::string strStateFlag;
                        std::string strStateDescription;

                        GetJobState(strJobID,strStateFlag,strStateDescription);

                        if ("" == strStateFlag &&
                            "" == strStateDescription)
                        {
                        }
                        else
                        {
                            if (STATE_BURN_ERR == strStateFlag)
                            {
                                bJobSucess = false;
                            }
                            if (i == (int)vecSingleDiscDataSource.size()-1
                                && (STATE_BURN_ERR == strStateFlag
                                || STATE_SINGLE_DISC_BURNED == strStateFlag))
                            {
                                if (bJobSucess)
                                {
                                    strStateFlag = STATE_JOB_OVER;
                                }
                                else
                                {
                                    strStateFlag = STATE_JOB_FINISHED;
                                }
                            }

                            SendCurrentBurnStateToUpper("","",strJobID,strStateFlag,strStateDescription);

                            if (STATE_BURN_ERR == strStateFlag ||
                                STATE_JOB_OVER == strStateFlag ||
                                STATE_JOB_FINISHED == strStateFlag ||
                                STATE_SINGLE_DISC_BURNED == strStateFlag)
                            {
                                break;
                            }
                        }

                        ZOSThread::Sleep(5000);
                    }
                }
            }


            CleanJob(strJobID);
            CleanJobData(strJobID);

#else
            //linux无法运行派美雅刻录
            LOG_ERROR(("[BurnServerAgent::StartNormalBurn] Linux,no primer,%d\r\n",__LINE__));

            SendCurrentBurnStateToUpper(normalJob.GetUpServerIP(),normalJob.GetUpServerPort(),
                strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","linuxnoprimera"));

            CleanJob(strJobID);

            CleanJobData(strJobID);

            return;
#endif
        } 
        else if(BURNSERVER_TYPE_EPSON == strBurnServerType)
        {
            //调用爱普生刻录接口
#ifdef WIN32
            bool bJobSucess = true;
            std::string strCDName = normalJob.GetCommonBurnParam().GetCDName();
            CNormalBurnJobInfoEx normalJobBurn = normalJob;

            for (int i = 0; i < (int)vecSingleDiscDataSource.size(); i++)
            {
                std::vector<std::string> vectDataSource;
                for (int j = 0; j < (int)vecSingleDiscDataSource.at(i).size(); j++)
                {
                    vectDataSource.push_back(vecSingleDiscDataSource.at(i).at(j).strSourceUrl);
                }
                if ((int)vecSingleDiscDataSource.size() > 1)
                {//多盘重设光盘名称
                    normalJobBurn.GetCommonBurnParam().SetCDName(strCDName + "_" + IntToString(i+1));
                }

                if (m_pEPSONCtrl != NULL)
                {
                    delete m_pEPSONCtrl;
                    m_pEPSONCtrl=NULL;
                }

                m_pEPSONCtrl=new CBurnServerEPSONCtrl;
                if (m_pEPSONCtrl != NULL)
                {
                    m_pEPSONCtrl->SetBurnJobInformation(normalJobBurn);

                    if (0 == m_pEPSONCtrl->InitParameter())
                    {
                        CStringW strDataPath=CharsetConvertMFC::UTF8ToUTF16((BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)
                            + strJobID).c_str());

                        if (0 == m_pEPSONCtrl->CreateJob(CharsetConvertMFC::UTF8ToUTF16(normalJob.GetJobID().c_str()),
                            vectDataSource))
                        {
#if 0
                            ZOSThread::Sleep(30000);
#endif


#if 1
                            if (0 == m_pEPSONCtrl->SubmitJob())
                            {
                                LOG_INFO(("[BurnServerAgent::StartNormalBurn] Submit job success\r\n"));

                                SendCurrentBurnStateToUpper(strUpServerIP,strUpServerPort,strJobID,STATE_DEFAULT,gBurnStateCfgFile.GetValue("info","submitjobsuccess"));

                                while (true)
                                {
                                    std::string strStateFlag;
                                    std::string strStateDescription;

                                    GetJobState(strJobID,strStateFlag,strStateDescription);

                                    if ("" == strStateFlag &&
                                        "" == strStateDescription)
                                    {
                                    }
                                    else
                                    {
                                        if (STATE_BURN_ERR == strStateFlag)
                                        {
                                            bJobSucess = false;
                                        }
                                        if (i == (int)vecSingleDiscDataSource.size()-1
                                            && (STATE_BURN_ERR == strStateFlag
                                            || STATE_SINGLE_DISC_BURNED == strStateFlag))
                                        {
                                            if (bJobSucess)
                                            {
                                                strStateFlag = STATE_JOB_OVER;
                                            }
                                            else
                                            {
                                                strStateFlag = STATE_JOB_FINISHED;
                                            }
                                        }

                                        SendCurrentBurnStateToUpper("","",strJobID,strStateFlag,strStateDescription);

                                        if (STATE_BURN_ERR == strStateFlag ||
                                            STATE_JOB_OVER == strStateFlag ||
                                            STATE_JOB_FINISHED == strStateFlag ||
                                            STATE_SINGLE_DISC_BURNED == strStateFlag)
                                        {
                                            break;
                                        }
                                    }

                                    ZOSThread::Sleep(5000);
                                }
                            }
                            else
                            {
                                SendCurrentBurnStateToUpper(strUpServerIP,strUpServerPort,strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","submitjobfailed"));
                            }
#endif

                        }
                        else
                        {
                            SendCurrentBurnStateToUpper(strUpServerIP,strUpServerPort,strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","createjobfailed"));
                        }
                    }
                    else
                    {
                        SendCurrentBurnStateToUpper(strUpServerIP,strUpServerPort,strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","initparamfailed"));
                    }
                }
            }

            CleanJob(strJobID);
            CleanJobData(strJobID);
#else
            //linux无法运行爱普生刻录
            LOG_ERROR(("[BurnServerAgent::StartNormalBurn] Linux,no epson,%d\r\n",__LINE__));

            SendCurrentBurnStateToUpper(normalJob.GetUpServerIP(),normalJob.GetUpServerPort(),
                strJobID,STATE_BURN_ERR,gBurnStateCfgFile.GetValue("info","linuxnoepson"));

            CleanJob(strJobID);

            CleanJobData(strJobID);

            return;
#endif
        }
    }
    else
    {
        LOG_ERROR(("[BurnServerAgent::StartNormalBurn] Unknown burn server type,%d\r\n",__LINE__));
        assert(false);
    }
}

std::string BurnServerAgent::GetJobType(std::string strJobID)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    std::string strType=JOB_TYPE_UNKNOWN;

    bool bJobFound=false;

    std::vector<CNormalBurnJobInfoEx>::const_iterator iterNormalJob;
    for (iterNormalJob=m_vecNormalJob.begin(); iterNormalJob!=m_vecNormalJob.end(); ++iterNormalJob)
    {
        if (iterNormalJob->GetJobID() == strJobID)
        {
            bJobFound=true;
            strType=JOB_TYPE_NORMAL;
            break;
        }
    }

    if (!bJobFound)
    {
        std::vector<CRTBurnInfoEx>::const_iterator iterRtJob;
        for (iterRtJob=m_vecRTBurnInfo.begin(); iterRtJob!=m_vecRTBurnInfo.end();
            ++iterRtJob)
        {
            if (iterRtJob->GetJobID() == strJobID)
            {
                strType=JOB_TYPE_RT;
                break;
            }
        }
    }

    return strType;
}

std::string BurnServerAgent::GetRtJobControlState(std::string strJobID)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    std::string strRet=RT_CONTROL_STATE_UNKNOWN;

    std::vector<CRTBurnInfoEx>::iterator iterRTJob;
    for (iterRTJob=m_vecRTBurnInfo.begin();iterRTJob!=m_vecRTBurnInfo.end();++iterRTJob)
    {
        if (iterRTJob->GetJobID() == strJobID)
        {
            strRet=iterRTJob->GetControlState();

            break;
        }
    }

    if (iterRTJob == m_vecRTBurnInfo.end())
    {
        //下一层task可能从多处反馈一个失败的task消息，如StartSpecificDeviceToBurn过程中的异常，
        //所以当进入此函数时，可能job已经被清除了

        /*LOG_WARNING(("[BurnServerAgent::GetRtJobControlState] Can not find job : %s,%d\r\n",
            strJobID.c_str(),__LINE__));*/
    }

    return strRet;
}

void BurnServerAgent::SetRtJobControlState(std::string strJobID,std::string strRTJobControlState)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    std::vector<CRTBurnInfoEx>::iterator iterRTJob;
    for (iterRTJob=m_vecRTBurnInfo.begin();iterRTJob!=m_vecRTBurnInfo.end();++iterRTJob)
    {
        if (iterRTJob->GetJobID() == strJobID)
        {
            iterRTJob->SetControlState(strRTJobControlState);

            break;
        }
    }

    if (iterRTJob == m_vecRTBurnInfo.end())
    {
        LOG_ERROR(("[BurnServerAgent::SetRtJobControlState] Can not find job : %s,%d\r\n",
            strJobID.c_str(),__LINE__));
        assert(false);
    }
}

void BurnServerAgent::DeleteRTJobDownloadTasks(std::string strJobID)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    std::vector<CRTBurnInfoEx>::const_iterator iterRtJob;
    for (iterRtJob=m_vecRTBurnInfo.begin();iterRtJob!=m_vecRTBurnInfo.end();++iterRtJob)
    {
        if (iterRtJob->GetJobID() == strJobID)
        {
            const std::map<std::string,FILE_DOWNLOAD_INFO > &mapDownloadInfo=
                iterRtJob->GetMapFileDownloadState();

            std::map<std::string,FILE_DOWNLOAD_INFO >::const_iterator iterDownloadInfo;
            for (iterDownloadInfo=mapDownloadInfo.begin();iterDownloadInfo!=mapDownloadInfo.end();
                ++iterDownloadInfo)
            {
                std::string strPercent=iterDownloadInfo->second.strPercentage;
                if (strPercent != "-1" &&
                    strPercent != "100")
                {
//                     DeleteTransmissionTask(iterDownloadInfo->first);
                    if (iterDownloadInfo->first != "")
                    {
                        int nRet=FileAnywhereManager::GetInstance()->DeleteTransmissionTask(iterDownloadInfo->first);
                        if (0 == nRet)
                        {
                            LOG_INFO(("[DeleteDownloadTaskThread] Delete download task success : %s,%d\r\n",
                                iterDownloadInfo->first.c_str(),__LINE__));
                        }
                        else
                        {
                            LOG_ERROR(("[DeleteDownloadTaskThread] Delete download task failed : %s,%d\r\n",
                                iterDownloadInfo->first.c_str(),__LINE__));
                        }
                    }
                }
            }
            break;
        }
    }
}

void BurnServerAgent::PauseRTBurnJob(std::string strJobID)
{
    if ("" == strJobID)
    {
        return;
    }

    if (JOB_TYPE_RT == GetJobType(strJobID))
    {
        //暂停实时刻录，但是不暂停下载
        if (1 == m_BurnTask.PauseRTTasksByJobID(strJobID))
        {
            SendCurrentBurnStateToUpper("","",strJobID,STATE_RTJOB_PAUSED,
                gBurnStateCfgFile.GetValue("info","paused"));

            return;
        }

        SetRtJobControlState(strJobID,RT_CONTROL_STATE_PAUSE);
    }
    else
    {
        LOG_ERROR(("[BurnServerAgent::PauseRTBurnJob] Job %s type is not rt,%d\r\n",
            strJobID.c_str(),__LINE__));

        assert(false);
    }
}

void BurnServerAgent::ResumeRTburn(std::string strJobID)
{
    if (JOB_TYPE_RT == GetJobType(strJobID))
    {
        if (RT_CONTROL_STATE_PAUSE == GetRtJobControlState(strJobID))
        {
            int nRet=m_BurnTask.ResumeRTJob(strJobID);

            if (1 == nRet)
            {
                {
                    ZOSMutexLocker locker(&m_mutexJobVec);

                    std::vector<CRTBurnInfoEx>::iterator iterRTJob;
                    for (iterRTJob=m_vecRTBurnInfo.begin();iterRTJob!=m_vecRTBurnInfo.end();++iterRTJob)
                    {
                        if (iterRTJob->GetJobID() == strJobID)
                        {

                            break;
                        }
                    }

                    if (iterRTJob == m_vecRTBurnInfo.end())
                    {
                        LOG_ERROR(("[BurnServerAgent::ResumeRTburn] Can not find job : %s,%d\r\n",
                            strJobID.c_str(),__LINE__));

                        assert(false);
                    }
                    else
                    {
                        m_BurnTask.ResumeRTJob(strJobID);
                    }
                }
            }

            if (nRet != -1)
            {
                SetRtJobControlState(strJobID,RT_CONTROL_STATE_START);
            }

            LOG_WARNING(("[BurnServerAgent::ResumeRTburn] ######################### ResumeRTburn,%d\r\n",
                __LINE__));
        }
        else
        {
            LOG_ERROR(("[BurnServerAgent::ResumeRTburn] Cur control state is not pause,%d\r\n",
                __LINE__));
        }
    }
}

void BurnServerAgent::StopRTBurn(std::string strJobID)
{
    std::string strJobType=GetJobType(strJobID);

    LOG_INFO(("[BurnServerAgent::StopRTBurn] Job type : %s\r\n",strJobType.c_str()));

    if (JOB_TYPE_RT == strJobType)
    {
        //删除正在执行的下载任务
        DeleteRTJobDownloadTasks(strJobID);

        int nRet=m_BurnTask.StopRTJob(strJobID);

        if (1 == nRet)
        {
            SendCurrentBurnStateToUpper("","",strJobID,STATE_RTJOB_STOPPED,
                gBurnStateCfgFile.GetValue("info","stopped"));

            CleanJob(strJobID);
        }
        else
        {
            SetRtJobControlState(strJobID,RT_CONTROL_STATE_STOP);

            LOG_INFO(("[BurnServerAgent::StopRTBurn] Set job %s stopped\r\n",
                strJobID.c_str()));
        }
    }
    else
    {
        LOG_ERROR(("[BurnServerAgent::StopRTBurn] Job type error,%d\r\n",__LINE__));

        assert(false);
    }
}

void BurnServerAgent::SendCDROMDriverInfo(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam)
{
    std::string strResponse;

    {
        ZOSMutexLocker locker(&m_mutexJobVec);

        strResponse=CInterfaceProtocol::GenerateProtocolCDROMDriverInfo(m_vecDev);
    }

    if (0 != ncxSendProtocolResponse(hNCXServer,*pCBParam,strResponse.c_str(),strResponse.length()+1))
    {
        LOG_ERROR(("[BurnServerAgent::SendCDROMDriverInfo] Send CDROM driver information failed,remote ip : %s\r\n",pCBParam->szRemoteIP));
    }
    else
    {
        LOG_INFO(("[BurnServerAgent::SendCDROMDriverInfo] Send CDROM driver information success\r\n"));

        LOG_INFO(("[BurnServerAgent::SendCDROMDriverInfo] CDROM driver information : \r\n%s\r\n",strResponse.c_str()));
    }
}

void BurnServerAgent::SendBurnServerInfo(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam)
{
    std::string strConfigFilePath;

#ifdef WIN32
    strConfigFilePath=std::string(BURN_SERVER_CFG_DIR)+"config\\"+"BurnServerConfig.xml";
#else
    strConfigFilePath=BurnTask::Path(GetCurDir()+"config/"+"BurnServerConfig.xml",false);
#endif

    TiXmlDocument xmlDocument;
    if (xmlDocument.LoadFile(strConfigFilePath.c_str(),TIXML_ENCODING_UTF8))
    {
        TiXmlPrinter xmlPrinter;
        xmlDocument.Accept(&xmlPrinter);
        std::string strXmlContent=xmlPrinter.CStr();

        if (ncxSendProtocolResponse(hNCXServer,*pCBParam,strXmlContent.c_str(),strXmlContent.length()+1) != 0)
        {
            LOG_ERROR(("[BurnServerAgent::SendBurnServerInfo] Send burn server information failed,remote ip : %s\r\n",
                pCBParam->szRemoteIP));
        }
        else
        {
            LOG_INFO(("[BurnServerAgent::SendBurnServerInfo] Send burn server information success\r\n"));
        }
    }
    else
    {
        LOG_ERROR(("[BurnServerAgent::SendBurnServerInfo] Load file error : %s\r\n",
            strConfigFilePath.c_str()));
    }
}

std::string BurnServerAgent::ExtractFileOrDirName(std::string strPath)
{
    std::string strRet="";

    if(strPath != "")
    {
        size_t nLen=strPath.length();

        char *pszTemp=new char[nLen+1];
        if (pszTemp != NULL)
        {
            strcpy(pszTemp,strPath.c_str());

            char *p=NULL;
            char *q=NULL;

            char c;
            for (p=pszTemp;p != pszTemp+nLen;++p)
            {
                c=*p;
                if ('/' == c ||
                    '\\' == c)
                {
                    *p='\0';
                }
            }

            for (p = pszTemp, q = pszTemp+nLen-1; q != p; --q)
            {
                char c=*q;
                if (c != '\0')
                {
                    break;
                }
            }

            if (p == q)
            {
                if ('\0' == *p)
                {
                    strRet="/";
                }
            }
            else
            {
                char *t=q;
                for (t=t-1;t!=p;--t)
                {
                    if ('\0' == *t)
                    {
                        break;
                    }
                }

                if ('\0' == *t)
                {
                    ++t;
                }


                strRet=t;
            }

            delete [] pszTemp;
        }
    }

    return strRet;
}

std::string BurnServerAgent::DownloadSingleFileOrDir(std::string strIP,std::string strPort,
                                                     std::string strRemotePath,std::string strLocalPath,
                                                     std::string strNewFileName,bool bIsFolder)
{
    /*    strRemotePath的格式:
    "/root/home/"   
    "/root/home/test.out" 
    "c:\\burn\\temp\\"
    "c:\\burn\\temp\\test.exe"

    strLocalPath的格式：
    "/root/home/"
    "c:\\burn\\temp\\"
    */

    if ("" == strIP ||
        "" == strPort ||
        "" == strRemotePath ||
        "" == strLocalPath)
    {
        LOG_ERROR(("[BurnServerAgent::DownloadSingleFile] Parameter error,%d\r\n",
            __LINE__));

        return "";
    }

    LOG_INFO(("[BurnServerAgent::DownloadSingleFileOrDir] Remote ip : %s,remote port : %s \
              remote path : %s,local path : %s,new file name : %s,%s,%d\r\n",
              strIP.c_str(),strPort.c_str(),strRemotePath.c_str(),strLocalPath.c_str(),
              strNewFileName.c_str(),bIsFolder ? "is folder" : "is file",__LINE__));

    std::string strFileOrDirName=ExtractFileOrDirName(strRemotePath);
    LOG_INFO(("[BurnServerAgent::DownloadSingleFileOrDir] File or dir name : %s\r\n",
        strFileOrDirName.c_str()));

    //允许文件重命名
    if (strNewFileName != "")
    {
        strFileOrDirName=strNewFileName;
    }

    FileAnywhereTaskParameter fileAnywhereTaskParam;
    std::string strTaskID=UUID_GetUUID();
    fileAnywhereTaskParam.TaskID(strTaskID);
    fileAnywhereTaskParam.RemoteIP(strIP);
    fileAnywhereTaskParam.RemotePort(atoi(strPort.c_str()));
    fileAnywhereTaskParam.Direction("get");
    fileAnywhereTaskParam.RemoteFile(strRemotePath);
    fileAnywhereTaskParam.LocalFile(strLocalPath+strFileOrDirName);
    fileAnywhereTaskParam.IsFolder(bIsFolder);
    fileAnywhereTaskParam.Observer(this);//接收下载状态信息
    FileAnywhereManager::GetInstance()->AddTransmissionTask(fileAnywhereTaskParam);

    return strTaskID;
}

bool BurnServerAgent::CopyLocalFileOrDir(const CNormalBurnJobInfoEx &normalJob)
{
    bool bAllLocal=true;

    std::string strRootDir=BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true);

    std::string strHostIP=GetHostIP();
    LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Host ip : %s,%d\r\n",
        strHostIP.c_str(),__LINE__));

    std::string strRemoteIP;
    std::string strRemotePort;
    std::string strRemotePath;
    std::string strLocalDataDir;

    //笔录文件
    strRemoteIP=normalJob.GetNoteFileRemoteIP();
    strRemotePort=normalJob.GetNoteFileRemotePort();

    std::string strNoteFileRelativePath=FixDirPath(normalJob.GetNoteRelativePath());//笔录文件所在文件夹
    strRemotePath=strNoteFileRelativePath+normalJob.GetNoteFileName();

    strLocalDataDir=BurnTask::Path(strRootDir+normalJob.GetJobID()+"\\"+normalJob.GetJobID(),true);

    LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Note file : Remote ip : %s,remote port : %s, \
              remote path : %s,local dir : %s,%d\r\n",strRemoteIP.c_str(),
              strRemotePort.c_str(),strRemotePath.c_str(),strLocalDataDir.c_str(),__LINE__));

    if (strRemoteIP == strHostIP)
    {
        LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Copy note file,%d\r\n",
            __LINE__));

#ifdef WIN32
        if (FileUtil::FileExistW(CharsetConvertMFC::UTF8ToUTF16(strRemotePath.c_str()).GetBuffer()))
        {
            std::vector<std::wstring> files;
            files.push_back(CharsetConvertMFC::UTF8ToUTF16(strRemotePath.c_str()).GetBuffer());
            DirectoryUtil::CopyFileToDirW(CharsetConvertMFC::UTF8ToUTF16(strLocalDataDir.c_str()),files);
        }
#else
        if (FileUtil::FileExist(strRemotePath.c_str()))
        {
            std::vector<std::string> vecFiles;
            vecFiles.push_back(strRemotePath);
            DirectoryUtil::CopyFileToDir(strLocalDataDir.c_str(),vecFiles);
        }
#endif
    }
    else if (strRemoteIP != "")
    {
        bAllLocal=false;
    }

    //播放器文件夹
    strRemoteIP=normalJob.GetPlayerInfo().GetRemoteIP();
    strRemotePort=normalJob.GetPlayerInfo().GetRemotePort();

    strRemotePath=FixDirPath(normalJob.GetPlayerInfo().GetDownloadURL());
    if (strRemotePath != "")
    {
        strRemotePath.at(strRemotePath.length()-1)='\0';
    }

    strLocalDataDir=BurnTask::Path(strRootDir+normalJob.GetJobID(),true);

    LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Playback: Remote ip : %s,remote port : %s, \
              remote path : %s,local dir : %s,%d\r\n",strRemoteIP.c_str(),
              strRemotePort.c_str(),strRemotePath.c_str(),strLocalDataDir.c_str(),__LINE__));

    if (strRemoteIP == strHostIP ||
        "" == strRemoteIP)
    {
        if ("" == strRemotePath)
        {
            strRemotePath=BurnServerAgent::GetCurDir()+"PlayBack";
        }

        LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Copy player dir,%d\r\n",
            __LINE__));

#ifdef WIN32
        if (DirectoryUtil::IsDirExistW(CStringW(CStringA(strRemotePath.c_str()))))
        {
            DirectoryUtil::CopyDirToDirW(CharsetConvertMFC::UTF8ToUTF16(strLocalDataDir.c_str()),
                CStringW(CStringA(strRemotePath.c_str())));
        }
#else
        if (DirectoryUtil::IsDirExist(strRemotePath.c_str()))
        {
            DirectoryUtil::CopyDirToDir(strLocalDataDir.c_str(),strRemotePath.c_str());
        }
#endif
    }
    else if (strRemoteIP != "")
    {
        bAllLocal=false;
    }

    //播放列表文件
    strRemoteIP=normalJob.GetPlaylistFileInfo().GetRemoteIP();
    strRemotePort=normalJob.GetPlaylistFileInfo().GetRemotePort();
    strRemotePath=normalJob.GetPlaylistFileInfo().GetDownloadURL();
    strLocalDataDir=BurnTask::Path(strRootDir+normalJob.GetJobID()+"\\"+normalJob.GetJobID(),true);

    LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Playlist file : Remote ip : %s,remote port : %s, \
              remote path : %s,local dir : %s,%d\r\n",strRemoteIP.c_str(),
              strRemotePort.c_str(),strRemotePath.c_str(),strLocalDataDir.c_str(),__LINE__));

    if (strRemoteIP == strHostIP)
    {
        LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Copy play list file,%d\r\n",
            __LINE__));

#ifdef WIN32
        if (FileUtil::FileExistW(CharsetConvertMFC::UTF8ToUTF16(strRemotePath.c_str()).GetBuffer()))
        {
            std::vector<std::wstring> files;
            files.push_back(CharsetConvertMFC::UTF8ToUTF16(strRemotePath.c_str()).GetBuffer());
            DirectoryUtil::CopyFileToDirW(CharsetConvertMFC::UTF8ToUTF16(strLocalDataDir.c_str()),files);
        }
#else
        if (FileUtil::FileExist(strRemotePath.c_str()))
        {
            std::vector<std::string> vecFiles;
            vecFiles.push_back(strRemotePath);
            DirectoryUtil::CopyFileToDir(strLocalDataDir.c_str(),vecFiles);
        }
#endif
    }
    else if (strRemoteIP != "")
    {
        bAllLocal=false;
    }

    //autorun文件
    strRemoteIP=normalJob.GetAutorunFileRemoteIP();
    strRemotePort=normalJob.GetAutorunFileRemotePort();
    strRemotePath=normalJob.GetAutorunFilePath();
    strLocalDataDir=BurnTask::Path(strRootDir+normalJob.GetJobID(),true);

    LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Autorun file: Remote ip : %s,remote port : %s, \
              remote path : %s,local dir : %s,%d\r\n",strRemoteIP.c_str(),
              strRemotePort.c_str(),strRemotePath.c_str(),strLocalDataDir.c_str(),__LINE__));

    if (strRemoteIP == strHostIP)
    {
        LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Copy autorun file,%d\r\n",
            __LINE__));

        LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Original AutoRun file path: %s.\r\n", strRemotePath.c_str()));
#ifdef WIN32
        int nPos = strRemotePath.find("Linux");
        if (nPos != std::string::npos)
        {
            strRemotePath.replace(nPos, 5, "");
        }

        if (FileUtil::FileExistW(CharsetConvertMFC::UTF8ToUTF16(strRemotePath.c_str()).GetBuffer()))
        {
            std::vector<std::wstring> vecFiles;
            vecFiles.push_back(CharsetConvertMFC::UTF8ToUTF16(strRemotePath.c_str()).GetBuffer());
            std::vector<std::wstring> vecDstFiles;
            vecDstFiles.push_back(_T("AutoRun.int"));
            DirectoryUtil::CopyFileToDirW(CharsetConvertMFC::UTF8ToUTF16(strLocalDataDir.c_str()),vecFiles);
        }
#else
        if (FileUtil::FileExist(strRemotePath.c_str()))
        {
            std::vector<std::string> vecFiles;
            vecFiles.push_back(strRemotePath);
            std::vector<std::string> vecDstFiles;
            vecDstFiles.push_back("AutoRun.inf");
            DirectoryUtil::CopyFileToDir(strLocalDataDir.c_str(),vecDstFiles);
        }
#endif
        LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Changed AutoRun file path: %s.\r\n", strRemotePath.c_str()));
    }
    else if (strRemoteIP != "")
    {
        bAllLocal=false;
    }

    //auth文件
    strRemotePath=BurnServerAgent::GetCurDir()+"Auth.exe";
    strLocalDataDir=BurnTask::Path(strRootDir+normalJob.GetJobID(),true);

    LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] AuthCS.exe: Remote ip : %s,remote port : %s, \
              remote path : %s,local dir : %s,%d\r\n",strRemoteIP.c_str(),
              strRemotePort.c_str(),strRemotePath.c_str(),strLocalDataDir.c_str(),__LINE__));

#ifdef WIN32
    if (FileUtil::FileExistW(CStringW(CStringA(strRemotePath.c_str())).GetBuffer()))
    {
        std::vector<std::wstring> files;
        files.push_back(CStringW(CStringA(strRemotePath.c_str())).GetBuffer());
        bool bRet=DirectoryUtil::CopyFileToDirW(CharsetConvertMFC::UTF8ToUTF16(strLocalDataDir.c_str()),files);
    }
#else
    if (FileUtil::FileExist(strRemotePath.c_str()))
    {
        std::vector<std::string> files;
        files.push_back(strRemotePath);
        DirectoryUtil::CopyFileToDir(strLocalDataDir.c_str(),files);
    }
#endif

    //记录已经添加到下载任务列表的文件，防止重复处理
    std::map<std::string,std::string> mapFileDownload;

    strRemoteIP=normalJob.GetVideoFileRemoteIP();
    strRemotePort=normalJob.GetVideoFileRemotePort();
    std::string strVideoFileDir=normalJob.GetVideoFileRelativePath();

    LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Remote ip : %s,remote port : %s, \
              remote video dir : %s,%d\r\n",strRemoteIP.c_str(),
              strRemotePort.c_str(),strVideoFileDir.c_str(),__LINE__));

    const std::vector<CCDROMDriverInfo> &vecDev=normalJob.GetBurnServerInfo().GetVecCDROMDriverInfo();
    std::vector<CCDROMDriverInfo>::const_iterator iterDev;
    for (iterDev=vecDev.begin();iterDev!=vecDev.end();++iterDev)
    {
        const std::vector<CLocationInfo> &vecLocationInfo=iterDev->GetVecLocationInfo();
        std::vector<CLocationInfo>::const_iterator iterLocationInfo;
        for (iterLocationInfo=vecLocationInfo.begin();iterLocationInfo!=vecLocationInfo.end();
            ++iterLocationInfo)
        {
            const std::vector<CSingleFileInfo> &vecFile=iterLocationInfo->GetVecFileInfo();
            std::vector<CSingleFileInfo>::const_iterator iterFile;
            for (iterFile=vecFile.begin();iterFile!=vecFile.end();++iterFile)
            {
                strRemotePath=FixDirPath(strVideoFileDir)+iterFile->GetFileName();
                strLocalDataDir=BurnTask::Path(strRootDir+normalJob.GetJobID()+"\\"+normalJob.GetJobID(),true);

                std::string strKey=strRemoteIP+strRemotePort+strRemotePath+strLocalDataDir;
                if (!mapFileDownload.empty() &&
                    mapFileDownload.find(strKey) != mapFileDownload.end())
                {
                    //这个文件已经被处理
                    continue;
                }
                else
                {
                    mapFileDownload[strKey]="";
                }

                if (strRemoteIP == strHostIP)
                {
#ifdef WIN32
                    if (FileUtil::FileExistW(CharsetConvertMFC::UTF8ToUTF16(strRemotePath.c_str()).GetBuffer()))
                    {
                        std::vector<std::wstring> files;
                        files.push_back(CharsetConvertMFC::UTF8ToUTF16(strRemotePath.c_str()).GetBuffer());
                        DirectoryUtil::CopyFileToDirW(CharsetConvertMFC::UTF8ToUTF16(strLocalDataDir.c_str()),files);
                    }
#else
                    if (FileUtil::FileExist(strRemotePath.c_str()))
                    {
                        std::vector<std::string> vecFiles;
                        vecFiles.push_back(strRemotePath);
                        DirectoryUtil::CopyFileToDir(strLocalDataDir.c_str(),vecFiles);
                    }
#endif
                }
                else if (strRemoteIP != "")
                {
                    bAllLocal=false;
                }
            }
        }
    }

    return bAllLocal;
}

bool BurnServerAgent::CopyLocalFileOrDir(const CRTBurnInfoEx &rtJob)
{
    bool bAllLocal=true;

    std::string strRootDir=BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true);

    std::string strHostIP=BurnServerAgent::GetHostIP();
    LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Host ip : %s,%d\r\n",
        strHostIP.c_str(),__LINE__));

    std::string strRemoteIP;
    std::string strRemotePort;
    std::string strRemotePath;
    std::string strLocalDataDir;

    //播放器
    strRemoteIP=rtJob.GetPlayerInfo().GetRemoteIP();
    strRemotePort=rtJob.GetPlayerInfo().GetRemotePort();
    strRemotePath=rtJob.GetPlayerInfo().GetDownloadURL();//
    strLocalDataDir=BurnTask::Path(strRootDir+rtJob.GetJobID(),true);

    LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Remote ip : %s,remote port : %s, \
              remote path : %s,local dir : %s,%d\r\n",strRemoteIP.c_str(),
              strRemotePort.c_str(),strRemotePath.c_str(),strLocalDataDir.c_str(),__LINE__));

    if (strHostIP == strRemoteIP ||
        "" == strRemoteIP)
    {
        LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Copy player dir,%d\r\n",__LINE__));

        if ("" == strRemotePath)
        {
            strRemotePath=BurnServerAgent::GetCurDir()+"PlayBack";
        }

#ifdef WIN32
#else
        if (DirectoryUtil::IsDirExist(strRemotePath.c_str()))
        {
            DirectoryUtil::CopyDirToDir(strLocalDataDir.c_str(),strRemotePath.c_str());
        }
#endif
    }
    else if(strRemoteIP != "")
    {
        bAllLocal=false;
    }

    //播放列表文件
    strRemoteIP=rtJob.GetPlaylistFileInfo().GetRemoteIP();
    strRemotePort=rtJob.GetPlaylistFileInfo().GetRemotePort();
    strRemotePath=rtJob.GetPlaylistFileInfo().GetDownloadURL();
    strLocalDataDir=BurnTask::Path(strRootDir+rtJob.GetJobID(),true);

    LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Remote ip : %s,remote port : %s, \
              remote path : %s,local dir : %s,%d\r\n",strRemoteIP.c_str(),
              strRemotePort.c_str(),strRemotePath.c_str(),strLocalDataDir.c_str(),__LINE__));

    if (strHostIP == strRemoteIP)
    {
        LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Copy play list file，%d\r\n",__LINE__));

#ifdef WIN32
#else
        if (FileUtil::FileExist(strRemotePath.c_str()))
        {
            std::vector<std::string> vecFiles;
            vecFiles.push_back(strRemotePath);
            DirectoryUtil::CopyFileToDir(strLocalDataDir.c_str(),vecFiles);
        }
#endif
    }
    else if(strRemoteIP != "")
    {
        bAllLocal=false;
    }

    //autorun文件
    strRemoteIP=rtJob.GetAutorunFileRemoteIP();
    strRemotePort=rtJob.GetAutorunFileRemotePort();
    strRemotePath=rtJob.GetAutorunFilePath();
    strLocalDataDir=BurnTask::Path(strRootDir+rtJob.GetJobID(),true);

    LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Remote ip : %s,remote port : %s, \
              remote path : %s,local dir : %s,%d\r\n",strRemoteIP.c_str(),
              strRemotePort.c_str(),strRemotePath.c_str(),strLocalDataDir.c_str(),__LINE__));

    if (strHostIP == strRemoteIP)
    {
        LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Copy autorun file，%d\r\n",__LINE__));

#ifdef WIN32
#else
        if (FileUtil::FileExist(strRemotePath.c_str()))
        {
            std::vector<std::string> vecFiles;
            vecFiles.push_back(strRemotePath);
            DirectoryUtil::CopyFileToDir(strLocalDataDir.c_str(),vecFiles);
        }
#endif
    }
    else if(strRemoteIP != "")
    {
        bAllLocal=false;
    }

    //auth文件
    strRemotePath=BurnServerAgent::GetCurDir()+"Auth.exe";
    strLocalDataDir=BurnTask::Path(strRootDir+rtJob.GetJobID(),true);
#ifdef WIN32
#else
    if (FileUtil::FileExist(strRemotePath.c_str()))
    {
        std::vector<std::string> vecFiles;
        vecFiles.push_back(strRemotePath);
        DirectoryUtil::CopyFileToDir(strLocalDataDir.c_str(),vecFiles);
    }
#endif

    //笔录文件
    strRemoteIP=rtJob.GetNoteFileRemoteIP();
    strRemotePort=rtJob.GetNoteFileRemotePort();
    strRemotePath=rtJob.GetNoteFileDownloadURL();
    strLocalDataDir=BurnTask::Path(strRootDir+rtJob.GetJobID(),true);

    LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Remote ip : %s,remote port : %s, \
              remote path : %s,local dir : %s,%d\r\n",strRemoteIP.c_str(),
              strRemotePort.c_str(),strRemotePath.c_str(),strLocalDataDir.c_str(),__LINE__));

    if (strHostIP == strRemoteIP)
    {
        LOG_INFO(("[BurnServerAgent::CopyLocalFileOrDir] Copy note file，%d\r\n",__LINE__));

#ifdef WIN32
#else
        if (FileUtil::FileExist(strRemotePath.c_str()))
        {
            std::vector<std::string> vecFiles;
            vecFiles.push_back(strRemotePath);
            DirectoryUtil::CopyFileToDir(strLocalDataDir.c_str(),vecFiles);
        }
#endif
    }
    else if(strRemoteIP != "")
    {
        //笔录文件例外
    }
    else
    {
        //不刻录笔录文件
    }

    return bAllLocal;
}

void BurnServerAgent::DownloadFile(std::string strJobID)
{
    ZOSMutexLocker lockerNormal(&m_mutexJobVec);   

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormalJob;
    for (iterNormalJob=m_vecNormalJob.begin();
        iterNormalJob!=m_vecNormalJob.end();++iterNormalJob)
    {
        if (iterNormalJob->GetJobID() == strJobID)
        {
            break;
        }
    }

    assert(iterNormalJob != m_vecNormalJob.end());

    std::string strHostIP=GetHostIP();

    std::string strRootDir=BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true);

    std::string strRemoteIP;
    std::string strRemotePort;
    std::string strRemotePath;
    std::string strLocalPath;

    std::string strUUID;
    FILE_DOWNLOAD_INFO info;

    //笔录文件
    strRemoteIP=iterNormalJob->GetNoteFileRemoteIP();
    strRemotePort=iterNormalJob->GetNoteFileRemotePort();
    std::string strNoteRelativePath=iterNormalJob->GetNoteRelativePath();//笔录文件所在文件夹

    strRemotePath=FixDirPath(strNoteRelativePath)+iterNormalJob->GetNoteFileName();

    strLocalPath=BurnTask::Path(strRootDir+iterNormalJob->GetJobID()+"\\"+iterNormalJob->GetJobID(),true);
    if (strRemoteIP != strHostIP)
    {
        strUUID=DownloadSingleFileOrDir(strRemoteIP,strRemotePort,strRemotePath,
            strLocalPath,"",false);
        if (strUUID != "")
        {
            info.strJobID=iterNormalJob->GetJobID();
            info.strType=TYPE_NOTE_FILE;
            info.strFileName=strRemotePath;

            iterNormalJob->GetMapFileDownloadState()[strUUID]=info;
        }
    }

    //播放器文件夹
    strRemoteIP=iterNormalJob->GetPlayerInfo().GetRemoteIP();
    strRemotePort=iterNormalJob->GetPlayerInfo().GetRemotePort();

    strRemotePath=iterNormalJob->GetPlayerInfo().GetDownloadURL();

    strLocalPath=BurnTask::Path(strRootDir+iterNormalJob->GetJobID(),true);
    if (strRemoteIP != strHostIP)
    {
        //strRemotePath=Path(strRemotePath,false);

        LOG_INFO(("[BurnServerAgent::DownloadFile] ~~~~ Start to download player ~~~~,%s,%s,%s,%s\r\n",
            strRemoteIP.c_str(),strRemotePort.c_str(),strRemotePath.c_str(),strLocalPath.c_str()));
        strUUID=DownloadSingleFileOrDir(strRemoteIP,strRemotePort,strRemotePath,
            strLocalPath,"",true);
        if (strUUID != "")
        {
            info.strJobID=iterNormalJob->GetJobID();
            info.strType=TYPE_PLAYER;
            info.strFileName=strRemotePath;

            iterNormalJob->GetMapFileDownloadState()[strUUID]=info;
        }
        else
        {
            LOG_INFO(("[BurnServerAgent::DownloadFile] ~~~~ Download player failed ~~~~\r\n"));
        }
    }

    //播放列表文件
    strRemoteIP=iterNormalJob->GetPlaylistFileInfo().GetRemoteIP();
    strRemotePort=iterNormalJob->GetPlaylistFileInfo().GetRemotePort();

    strRemotePath=iterNormalJob->GetPlaylistFileInfo().GetDownloadURL();

    strLocalPath=BurnTask::Path(strRootDir+iterNormalJob->GetJobID()+"\\"+iterNormalJob->GetJobID(),true);
    if (strRemoteIP != strHostIP)
    {
        strUUID=DownloadSingleFileOrDir(strRemoteIP,strRemotePort,strRemotePath,
            strLocalPath,"",false);
        if (strUUID != "")
        {
            info.strJobID=iterNormalJob->GetJobID();
            info.strType=TYPE_PLAY_LIST;
            info.strFileName=strRemotePath;

            iterNormalJob->GetMapFileDownloadState()[strUUID]=info;
        }
    }

    //autorun文件
    strRemoteIP=iterNormalJob->GetAutorunFileRemoteIP();
    strRemotePort=iterNormalJob->GetAutorunFileRemotePort();

    strRemotePath=iterNormalJob->GetAutorunFilePath();
    LOG_INFO(("[BurnServerAgent::DownloadFile] Original AutoRun file path: %s.\r\n", strRemotePath.c_str()));
#ifdef WIN32
    int nPos = strRemotePath.find("Linux");
    if (nPos != std::string::npos)
    {
        strRemotePath.replace(nPos, 5, "");
    }
#else
#endif
    LOG_INFO(("[BurnServerAgent::DownloadFile] Changed AutoRun file path: %s.\r\n", strRemotePath.c_str()));

    strLocalPath=BurnTask::Path(strRootDir+iterNormalJob->GetJobID(),true);
    if (strRemoteIP != strHostIP)
    {
        strUUID=DownloadSingleFileOrDir(strRemoteIP,strRemotePort,strRemotePath,
            strLocalPath,"AutoRun.inf",false);
        if (strUUID != "")
        {
            info.strJobID=iterNormalJob->GetJobID();
            info.strType=TYPE_AUTORUN;
            info.strFileName=strRemotePath;

            iterNormalJob->GetMapFileDownloadState()[strUUID]=info;
        }
    }


    //记录已经添加到下载任务列表的文件，防止重复下载
    std::map<std::string,std::string> mapFileDownload;

    strRemoteIP=iterNormalJob->GetVideoFileRemoteIP();
    strRemotePort=iterNormalJob->GetVideoFileRemotePort();
    std::string strVideoFileDir=iterNormalJob->GetVideoFileRelativePath();

    if (strRemoteIP != strHostIP)
    {
        const std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo=
            iterNormalJob->GetBurnServerInfo().GetVecCDROMDriverInfo();

        std::vector<CCDROMDriverInfo>::const_iterator iterCDROMDriverInfo;
        for (iterCDROMDriverInfo=vecCDROMDriverInfo.begin();iterCDROMDriverInfo!=vecCDROMDriverInfo.end();
            ++iterCDROMDriverInfo)
        {
            const std::vector<CLocationInfo> &vecLocationInfo=iterCDROMDriverInfo->GetVecLocationInfo();
            std::vector<CLocationInfo>::const_iterator iterLocationInfo;
            for (iterLocationInfo=vecLocationInfo.begin();iterLocationInfo!=vecLocationInfo.end();
                ++iterLocationInfo)
            {
                const std::vector<CSingleFileInfo> &vecSingleFileInfo=iterLocationInfo->GetVecFileInfo();
                std::vector<CSingleFileInfo>::const_iterator iterSingleFileInfo;
                for (iterSingleFileInfo=vecSingleFileInfo.begin();iterSingleFileInfo!=vecSingleFileInfo.end();
                    ++iterSingleFileInfo)
                {
                    strRemotePath=FixDirPath(strVideoFileDir)+iterSingleFileInfo->GetFileName();
                    strLocalPath=BurnTask::Path(strRootDir+iterNormalJob->GetJobID()+"\\"+iterNormalJob->GetJobID(),true);

                    std::string strKey=strRemoteIP+strRemotePort+strRemotePath+strLocalPath;
                    if (!mapFileDownload.empty() &&
                        mapFileDownload.find(strKey) != mapFileDownload.end())
                    {
                        continue;
                    }
                    else
                    {
                        mapFileDownload[strKey]="";
                    }

                    if (iterSingleFileInfo->GetNewFileName() != "")
                    {
                        strUUID=DownloadSingleFileOrDir(strRemoteIP,strRemotePort,strRemotePath,
                            strLocalPath,iterSingleFileInfo->GetNewFileName(),false);
                    }
                    else
                    {
                        strUUID=DownloadSingleFileOrDir(strRemoteIP,strRemotePort,strRemotePath,
                            strLocalPath,"",false);
                    }

                    if (strUUID != "")
                    {
                        info.strJobID=iterNormalJob->GetJobID();
                        info.strType=TYPE_VIDEO;
                        info.strFileName=strRemotePath;
                        if (BURN_TYPE_SINGLE == iterNormalJob->GetCommonBurnParam().GetBurnType() ||
                            BURN_TYPE_MULTI_SYN == iterNormalJob->GetCommonBurnParam().GetBurnType() ||
                            BURN_TYPE_MULTI_ASYN == iterNormalJob->GetCommonBurnParam().GetBurnType())
                        {
                        } 
                        else
                        {
                            info.strCDROMDriverID=iterCDROMDriverInfo->GetID();
                        }

                        iterNormalJob->GetMapFileDownloadState()[strUUID]=info;
                    }
                }
            }
        }//for
    }
}

void BurnServerAgent::DownloadFile(std::string strJobID,int nFlag)
{
    ZOSMutexLocker lockerNormal(&m_mutexJobVec);

    std::vector<CRTBurnInfoEx>::iterator iterRtJob;
    for (iterRtJob=m_vecRTBurnInfo.begin();iterRtJob!=m_vecRTBurnInfo.end();
        ++iterRtJob)
    {
        if (iterRtJob->GetJobID() == strJobID)
        {
            break;
        }
    }

    //可能出现刻录任务因为出错已经被删除，但是还没有开始下载的情况
    if (iterRtJob == m_vecRTBurnInfo.end())
    {
        return;
    }

    int nFileCount=0;
    FILE_DOWNLOAD_INFO info;

    //一个任务下载文件本地存放路径为DOWNLOAD_DIR目录下的一个文件夹，文件夹名字就是任务id
    std::string strRootDir=BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true);

    std::string strHostIP=BurnServerAgent::GetHostIP();
    LOG_INFO(("[BurnServerAgent::DownloadFile] Host ip : %s,%d\r\n",strHostIP.c_str(),__LINE__));

    std::string strRemoteIP;
    std::string strRemotePort;
    std::string strRemotePath;
    std::string strLocalDownloadDir;
    std::string strUUID;

    if (RT_DOWNLOAD_FLAG_NO_NOTE_FILE == nFlag)
    {
        //播放器文件夹
        strRemoteIP=iterRtJob->GetPlayerInfo().GetRemoteIP();
        strRemotePort=iterRtJob->GetPlayerInfo().GetRemotePort();
        strRemotePath=iterRtJob->GetPlayerInfo().GetDownloadURL();
        strLocalDownloadDir=BurnTask::Path(strRootDir+iterRtJob->GetJobID(),true);

        LOG_INFO(("[BurnServerAgent::DownloadFile] Rt Burn,player info : %s,%s,%s,%s\r\n",
            strRemoteIP.c_str(),strRemotePort.c_str(),strRemotePath.c_str(),
            strLocalDownloadDir.c_str()));

        if(strRemoteIP != strHostIP &&
            strRemoteIP != "")
        {
            strUUID=DownloadSingleFileOrDir(strRemoteIP,strRemotePort,strRemotePath,
                strLocalDownloadDir,"",true);

            if (strUUID != "")
            {
                info.strJobID=iterRtJob->GetJobID();
                info.strType=TYPE_PLAYER;
                info.strFileName=strRemotePath;

                iterRtJob->GetMapFileDownloadState()[strUUID]=info;

                ++nFileCount;
            }
        }

        //播放列表文件
        strRemoteIP=iterRtJob->GetPlaylistFileInfo().GetRemoteIP();
        strRemotePort=iterRtJob->GetPlaylistFileInfo().GetRemotePort();
        strRemotePath=iterRtJob->GetPlaylistFileInfo().GetDownloadURL();
        strLocalDownloadDir=BurnTask::Path(strRootDir+iterRtJob->GetJobID(),true);

        LOG_INFO(("[BurnServerAgent::DownloadFile] Play list file : %s,%s,%s,%s\r\n",
            strRemoteIP.c_str(),strRemotePort.c_str(),strRemotePath.c_str(),
            strLocalDownloadDir.c_str()));

        if (strHostIP != strRemoteIP &&
            strRemoteIP != "")
        {
            strUUID=DownloadSingleFileOrDir(strRemoteIP,strRemotePort,strRemotePath,
                strLocalDownloadDir,"",false);

            if (strUUID != "")
            {
                info.strJobID=iterRtJob->GetJobID();
                info.strType=TYPE_PLAY_LIST;
                info.strFileName=strRemotePath;

                iterRtJob->GetMapFileDownloadState()[strUUID]=info;

                ++nFileCount;
            }
        }

        //autorun文件
        strRemoteIP=iterRtJob->GetAutorunFileRemoteIP();
        strRemotePort=iterRtJob->GetAutorunFileRemotePort();
        strRemotePath=iterRtJob->GetAutorunFilePath();
        strLocalDownloadDir=BurnTask::Path(strRootDir+iterRtJob->GetJobID(),true);

        if (strHostIP != strRemoteIP &&
            strRemoteIP != "")
        {
            strUUID=DownloadSingleFileOrDir(strRemoteIP,strRemotePort,strRemotePath,
                strLocalDownloadDir,"",false);

            if (strUUID != "")
            {
                info.strJobID=iterRtJob->GetJobID();
                info.strType=TYPE_AUTORUN;
                info.strFileName=strRemotePath;

                iterRtJob->GetMapFileDownloadState()[strUUID]=info;

                ++nFileCount;
            }

        }
    }
    else if(1 == nFlag)
    {
        //笔录文件
        strRemoteIP=iterRtJob->GetNoteFileRemoteIP();
        strRemotePort=iterRtJob->GetNoteFileRemotePort();
        strRemotePath=iterRtJob->GetNoteFileDownloadURL();
        strLocalDownloadDir=BurnTask::Path(strRootDir+iterRtJob->GetJobID(),true);

        LOG_INFO(("[BurnServerAgent::DownloadFile] Note file download information : Remote ip : %s, \
                  remote port : %s,remote path : %s,local path : %s\r\n",
                  strRemoteIP.c_str(),strRemotePort.c_str(),strRemotePath.c_str(),
                  strLocalDownloadDir.c_str()));

        if (strHostIP == strRemoteIP)
        {
#ifdef WIN32
            //windows没有实时刻录，所以不会运行到此
#else
            std::vector<std::string> vecFiles;
            vecFiles.push_back(strRemotePath);
            DirectoryUtil::CopyFileToDir(strLocalDownloadDir.c_str(),vecFiles);
#endif
        }
        else if(strRemoteIP != "")
        {
            strUUID=DownloadSingleFileOrDir(strRemoteIP,strRemotePort,strRemotePath,
                strLocalDownloadDir,"note_1.doc",false);

            if (strUUID != "")
            {
                info.strJobID=iterRtJob->GetJobID();
                info.strType=TYPE_NOTE_FILE;
                info.strFileName=strRemotePath;

                iterRtJob->GetMapFileDownloadState()[strUUID]=info;
                //笔录文件不纳入下载计数
            }
        }
    }

    iterRtJob->SetTotalCount(nFileCount);
}

void BurnServerAgent::DeleteTransmissionTask(std::string strTransID)
{
    THREAD_PARAM *pThreadParam=new THREAD_PARAM;
    if (pThreadParam != NULL)
    {
        int nContentLen=strTransID.length();
        pThreadParam->pThreadData=new char[nContentLen+1];

        if (NULL == pThreadParam->pThreadData)
        {
            delete pThreadParam;
            pThreadParam=NULL;

            return;
        }
        strcpy((char *)(pThreadParam->pThreadData),strTransID.c_str());

        pThreadParam->pUserParam=this;

#ifdef WIN32
        HANDLE hThread=CreateThread(NULL,0,DeleteDownloadTaskThread,pThreadParam,0,NULL);
        if (NULL == hThread)
        {
            delete [] (char *)(pThreadParam->pThreadData);
            pThreadParam->pThreadData=NULL;

            delete pThreadParam;
            pThreadParam=NULL;

            return;
        }
        else
        {
            CloseHandle(hThread);
        }
#else
        pthread_t t;
        if (pthread_create(&t,NULL,DeleteDownloadTaskThread,pThreadParam) != 0)
        {
            delete [] (char *)(pThreadParam->pThreadData);
            pThreadParam->pThreadData=NULL;

            delete pThreadParam;
            pThreadParam=NULL;

            return;
        }
        else
        {
            pthread_detach(t);
        }
#endif
    }
}

bool BurnServerAgent::GetJobFilesInformation(std::string strJobID)
{
    ZOSMutexLocker lockerNormal(&m_mutexJobVec);

    std::string strHostIP=BurnServerAgent::GetHostIP();

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormalJob;
    for (iterNormalJob=m_vecNormalJob.begin();
        iterNormalJob!=m_vecNormalJob.end();++iterNormalJob)
    {
        if (iterNormalJob->GetJobID() == strJobID)
        {
            break;
        }
    }

    assert(iterNormalJob != m_vecNormalJob.end());

    std::string strRemoteIP;
    std::string strRemotePort;
    std::string strRemotePath;

    INT64 fileSize;
    INT64 totalFileSize=0;
    int fileCount=0;

    //笔录文件
    strRemoteIP=iterNormalJob->GetNoteFileRemoteIP();
    strRemotePort=iterNormalJob->GetNoteFileRemotePort();
    std::string strNoteRelativePath=iterNormalJob->GetNoteRelativePath();//笔录文件所在文件夹

    strRemotePath=FixDirPath(strNoteRelativePath)+iterNormalJob->GetNoteFileName();

    fileSize=GetFileSize(strRemoteIP,strRemotePort,strRemotePath);
    if (fileSize > 0)
    {
        totalFileSize+=fileSize;

        if (strRemoteIP != strHostIP)
        {
            fileCount++;
        }
    }
    else if (strRemoteIP != "" ||
        strRemotePort != "" ||
        strRemotePath != "")
    {
        LOG_ERROR(("[BurnServerAgent::GetJobFilesInformation] Get note file size failed,%s,%s,%s\r\n",
            strRemotePath.c_str(),strRemoteIP.c_str(),strRemotePort.c_str()));
        return false;
    }

    //播放器文件夹
    strRemoteIP=iterNormalJob->GetPlayerInfo().GetRemoteIP();
    strRemotePort=iterNormalJob->GetPlayerInfo().GetRemotePort();

    strRemotePath=iterNormalJob->GetPlayerInfo().GetDownloadURL();

    fileSize=GetFileListSize(strRemoteIP,strRemotePort,strRemotePath);
    if (fileSize > 0)
    {
        totalFileSize+=fileSize;

        if (strRemoteIP != strHostIP)
        {
            fileCount++;
        }
    }
    else if (strRemoteIP != "" ||
        strRemotePort != "" ||
        strRemotePath != "")
    {
        LOG_ERROR(("[BurnServerAgent::GetJobFilesInformation] Get Player size failed,%s,%s,%s\r\n",
            strRemoteIP.c_str(),strRemotePort.c_str(),strRemotePath.c_str()));
        return false;
    }

    //播放列表文件
    strRemoteIP=iterNormalJob->GetPlaylistFileInfo().GetRemoteIP();
    strRemotePort=iterNormalJob->GetPlaylistFileInfo().GetRemotePort();

    strRemotePath=iterNormalJob->GetPlaylistFileInfo().GetDownloadURL();

    fileSize=GetFileSize(strRemoteIP,strRemotePort,strRemotePath);
    if (fileSize > 0)
    {
        totalFileSize+=fileSize;

        if (strRemoteIP != strHostIP)
        {
            fileCount++;
        }
    }
    else if (strRemoteIP != "" ||
        strRemotePort != "" ||
        strRemotePath != "")
    {
        LOG_ERROR(("[BurnServerAgent::GetJobFilesInformation] Get playlist file size failed,%s,%s,%s\r\n",
            strRemoteIP.c_str(),strRemotePort.c_str(),strRemotePath.c_str()));
        return false;
    }

    //自动运行文件
    strRemoteIP=iterNormalJob->GetAutorunFileRemoteIP();
    strRemotePort=iterNormalJob->GetAutorunFileRemotePort();

    strRemotePath=iterNormalJob->GetAutorunFilePath();
#ifdef WIN32
    int nPos = strRemotePath.find("Linux");
    if (nPos != std::string::npos)
    {
        strRemotePath.replace(nPos, 5, "");
    }
#else
#endif

    fileSize=GetFileSize(strRemoteIP,strRemotePort,strRemotePath);
    if (fileSize > 0)
    {
        totalFileSize+=fileSize;

        if (strRemoteIP != strHostIP)
        {
            fileCount++;
        }
    }
    else if (strRemoteIP != "" ||
        strRemotePort != "" ||
        strRemotePath != "")
    {
        LOG_ERROR(("[BurnServerAgent::GetJobTotalFileSize] Get autorun file size failed,ip:%s,port:%s,path:%s\r\n",
            strRemoteIP.c_str(),strRemotePort.c_str(),strRemotePath.c_str()));
        return false;
    }

    //记录已经添加到下载任务列表的文件，防止重复下载
    std::map<std::string,std::string> mapFileDownload;

    strRemoteIP=iterNormalJob->GetVideoFileRemoteIP();
    strRemotePort=iterNormalJob->GetVideoFileRemotePort();
    std::string strVideoFileDir=iterNormalJob->GetVideoFileRelativePath();

    LOG_INFO(("[BurnServerAgent::GetJobFilesInformation] Video : %s,%s,%s\r\n",
        strRemoteIP.c_str(),strRemotePort.c_str(),strVideoFileDir.c_str()));

    const std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo=
        iterNormalJob->GetBurnServerInfo().GetVecCDROMDriverInfo();

    LOG_INFO(("[BurnServerAgent::GetJobFilesInformation] Dev count : %d\r\n",vecCDROMDriverInfo.size()));

    std::vector<CCDROMDriverInfo>::const_iterator iterCDROMDriverInfo;
    for (iterCDROMDriverInfo=vecCDROMDriverInfo.begin();iterCDROMDriverInfo!=vecCDROMDriverInfo.end();
        ++iterCDROMDriverInfo)
    {
        const std::vector<CLocationInfo> &vecLocationInfo=iterCDROMDriverInfo->GetVecLocationInfo();

        LOG_INFO(("[BurnServerAgent::GetJobFilesInformation] Location count %d\r\n",vecLocationInfo.size()));

        std::vector<CLocationInfo>::const_iterator iterLocationInfo;
        for (iterLocationInfo=vecLocationInfo.begin();iterLocationInfo!=vecLocationInfo.end();
            ++iterLocationInfo)
        {
            const std::vector<CSingleFileInfo> &vecSingleFileInfo=iterLocationInfo->GetVecFileInfo();
            LOG_INFO(("[BurnServerAgent::GetJobFilesInformation] Single file count : %d\r\n",
                vecSingleFileInfo.size()));

            std::vector<CSingleFileInfo>::const_iterator iterSingleFileInfo;
            for (iterSingleFileInfo=vecSingleFileInfo.begin();iterSingleFileInfo!=vecSingleFileInfo.end();
                ++iterSingleFileInfo)
            {
                strRemotePath=FixDirPath(strVideoFileDir)+iterSingleFileInfo->GetFileName();

                std::string strLocalPath=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)
                    +strJobID+"\\"+strJobID+"\\",true);
                LOG_INFO(("[BurnServerAgent::GetJobFilesInformation] Local path : %s\r\n",strLocalPath.c_str()));

                std::string strKey=strRemoteIP+strRemotePort+strRemotePath+strLocalPath;
                if (!mapFileDownload.empty() &&
                    mapFileDownload.find(strKey) != mapFileDownload.end())
                {
                    continue;
                }
                else
                {
                    mapFileDownload[strKey]="";
                }


                fileSize=GetFileSize(strRemoteIP,strRemotePort,strRemotePath);

                LOG_INFO(("[BurnServerAgent::GetJobFilesInformation] File size :%s,%s,%s,%llu\r\n",
                    strRemoteIP.c_str(),strRemotePort.c_str(),strRemotePath.c_str(),fileSize));

                if (fileSize > 0)
                {
                    totalFileSize+=fileSize;

                    if (strRemoteIP != strHostIP)
                    {
                        fileCount++;
                    }
                }
                else if (strRemoteIP != "" ||
                    strRemotePort != "" ||
                    strRemotePath != "")
                {
                    LOG_ERROR(("[BurnServerAgent::GetJobTotalFileSize] Get file size error,remote path : %s\r\n",
                        strRemotePath.c_str()));

                    return false;
                }
            }
        }
    }

    iterNormalJob->SetFileTotalSize(totalFileSize);
    iterNormalJob->SetTotalCount(fileCount);

    LOG_INFO(("[BurnServerAgent::ProccessProtocol] Total job size : %s\r\n",
        Int64ToString(totalFileSize).c_str()));

    return true;
}

std::string BurnServerAgent::FixDirPath(std::string strDir)
{
    if (strDir.empty())
    {
        return "";
    }

    std::string str;
    if (strDir.find(":") != std::string::npos)
    {
        str="\\";
    }
    else
    {
        str="/";
    }

    size_t nLen=strDir.length();
    if (strDir.at(nLen-1) != str.at(0))
    {
        strDir+=str;
    }

    if ("\\" == str)
    {
        size_t nPos=strDir.find('/');
        while (nPos != std::string::npos)
        {
            strDir.replace(nPos,1,"\\");

            nPos=strDir.find('/');
        }
    }
    else
    {
        size_t nPos=strDir.find('\\');
        while (nPos != std::string::npos)
        {
            strDir.replace(nPos,1,"/");

            nPos=strDir.find('\\');
        }
    }

    return strDir;
}

void BurnServerAgent::AttachPrimeraJobID(std::string strOrgJobID,std::string strPriJobID)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormalJob;
    for (iterNormalJob=m_vecNormalJob.begin();iterNormalJob!=m_vecNormalJob.end();++iterNormalJob)
    {
        if (iterNormalJob->GetJobID() == strOrgJobID)
        {
            iterNormalJob->SetPrimeraJobID(strPriJobID);

            break;
        }
    }
}

std::string BurnServerAgent::GetPrimeraJobID(std::string strOrgJobID)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    std::string strRet;

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormalJob;
    for (iterNormalJob=m_vecNormalJob.begin();iterNormalJob!=m_vecNormalJob.end();++iterNormalJob)
    {
        if (iterNormalJob->GetJobID() == strOrgJobID)
        {
            strRet=iterNormalJob->GetPrimeraJobID();

            break;
        }
    }

    return strRet;
}

//strStateFlag //《刻录协议》被动类型接口2.7.4
bool BurnServerAgent::GetJobState(std::string strJobID,std::string &strStateFlag,std::string &strStateDescription)
{
    bool bRet=true;

    ZOSMutexLocker locker(&m_mutexJobVec);

    bool bJobFound=false;

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormal;
    for (iterNormal=m_vecNormalJob.begin();iterNormal!=m_vecNormalJob.end();
        ++iterNormal)
    {
        if (strJobID == iterNormal->GetJobID())
        {
            bJobFound=true;

            std::string strBurnServerType=iterNormal->GetBurnServerInfo().GetBurnServerType();

            if (BURNSERVER_TYPE_NORMAL == strBurnServerType)
            {
            } 
            else if(BURNSERVER_TYPE_PRIMERA == strBurnServerType)
            {
#ifdef WIN32
                CStringW strErr;

                std::string strPrimeraJobID=GetPrimeraJobID(strJobID);

                int n=GetPrimeraJobStatus(CharsetConvertMFC::UTF8ToUTF16(strPrimeraJobID.c_str()),strErr);
                switch (n)
                {
                case -1:
                    bRet=false;
                    strStateFlag=iterNormal->GetStateFlag();
                    strStateDescription=iterNormal->GetStateDescription();
                    break;

                case 0:
                    strStateFlag=STATE_DEFAULT;
                    strStateDescription=gBurnStateCfgFile.GetValue("info","preparedata");
                    break;

                case 1:
                    strStateFlag=STATE_BURNING;
                    strStateDescription=gBurnStateCfgFile.GetValue("info","burning");
                    break;

                case 2:
                    strStateFlag=STATE_SINGLE_DISC_BURNED;
                    strStateDescription=gBurnStateCfgFile.GetValue("info","burnsuccess");
                    break;

                case 3:
                    strStateFlag=STATE_BURN_ERR;
                    strStateDescription=CharsetConvertMFC::UTF16ToUTF8(strErr).GetBuffer();
                    break;
                }
#endif
            }
            else if (BURNSERVER_TYPE_EPSON == strBurnServerType)
            {
#ifdef WIN32
                if (m_pEPSONCtrl != NULL)
                {
                    int n=m_pEPSONCtrl->GetEPSONJobStatus();
                    switch (n)
                    {
                    case -1:
                        strStateFlag=iterNormal->GetStateFlag();
                        strStateDescription=iterNormal->GetStateDescription();
                        bRet=false;
                        break;

                    case 0:
                        strStateFlag=STATE_DEFAULT;
                        strStateDescription=gBurnStateCfgFile.GetValue("info","preparedata");
                        break;

                    case 1:
                        strStateFlag=STATE_BURNING;
                        strStateDescription=gBurnStateCfgFile.GetValue("info","burning");
                        break;

                    case 2:
                        strStateFlag=STATE_SINGLE_DISC_BURNED;
                        strStateDescription=gBurnStateCfgFile.GetValue("info","burnsuccess");
                        break;

                    case 3:
                        strStateFlag=STATE_BURN_ERR;
                        strStateDescription=gBurnStateCfgFile.GetValue("info","burnfailed");
                        break;

                    default:
                        assert(false);
                    }
                }
#endif
            }
            else
            {
                assert(false);
            }

            break;
        }
    }

    if (!bJobFound)
    {
        std::vector<CRTBurnInfoEx>::iterator iterRTJob;
        for (iterRTJob=m_vecRTBurnInfo.begin();iterRTJob!=m_vecRTBurnInfo.end();++iterRTJob)
        {
            if (iterRTJob->GetJobID() == strJobID)
            {
                strStateFlag=iterRTJob->GetStateFlag();
                strStateDescription=iterRTJob->GetStateDescription();

                break;
            }
        }
    }

    return bRet;
}

const std::vector<CNormalBurnJobInfoEx> & BurnServerAgent::GetNormalJobInfoVec() const
{
    return m_vecNormalJob;
}
std::vector<CNormalBurnJobInfoEx> & BurnServerAgent::GetNormalJobInfoVec()
{
    return m_vecNormalJob;
}

const std::vector<CRTBurnInfoEx> & BurnServerAgent::GetRTJobInfoVec() const
{
    return m_vecRTBurnInfo;
}
std::vector<CRTBurnInfoEx> & BurnServerAgent::GetRTJobInfoVec()
{
    return m_vecRTBurnInfo;
}

void BurnServerAgent::CleanJob(std::string strJobID)
{
    {
        ZOSMutexLocker locker(&m_mutexJobVec);

        bool bJobFound=false;

        std::vector<CNormalBurnJobInfoEx>::iterator iterNormal;
        for (iterNormal=m_vecNormalJob.begin();iterNormal!=m_vecNormalJob.end();++iterNormal)
        {
            if (iterNormal->GetJobID() == strJobID)
            {
                bJobFound=true;

                if (BURNSERVER_TYPE_EPSON == iterNormal->GetBurnServerInfo().GetBurnServerType())
                {
#ifdef WIN32
                    if (m_pEPSONCtrl != NULL)
                    {
                        delete m_pEPSONCtrl;

                        m_pEPSONCtrl=NULL;
                    }
#endif
                }

                m_vecNormalJob.erase(iterNormal);

                LOG_INFO(("[BurnServerAgent::CleanJob] Job %s is cleaned,%d\r\n",
                    strJobID.c_str(),__LINE__));

                break;
            }
        }


        if (!bJobFound)
        {
            std::vector<CRTBurnInfoEx>::iterator iterRT;
            for (iterRT=m_vecRTBurnInfo.begin();iterRT!=m_vecRTBurnInfo.end();++iterRT)
            {
                if (iterRT->GetJobID() == strJobID)
                {
                    m_vecRTBurnInfo.erase(iterRT);

                    LOG_INFO(("[BurnServerAgent::CleanJob] Job %s is cleaned,%d\r\n",
                        strJobID.c_str(),__LINE__));

                    break;
                }
            }
        }
    }

    {
        ZOSMutexLocker locker(&m_mutexDevVec);

        size_t nDevCount=m_vecDev.size();
        for (size_t i=0;i<nDevCount;++i)
        {
            if (m_vecDev.at(i).GetJobID() == strJobID)
            {
                m_vecDev.at(i).SetJobID("");
            }
        }
    }
}

void BurnServerAgent::CleanJobData(std::string strJobID)
{
    std::string strJobDir=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)+strJobID,true);
    if (IsFileOrDirExist(strJobDir.c_str(),true))
    {
        LOG_INFO(("[BurnServerAgent::CleanJobData] Will delete dir : %s,%d\r\n",
            strJobDir.c_str(),__LINE__));

#ifdef WIN32
        DirectoryUtil::DeleteDirW(CharsetConvertMFC::UTF8ToUTF16(strJobDir.c_str()));
#else
        DirectoryUtil::DeleteDir(strJobDir.c_str());
#endif
    }
}

void BurnServerAgent::CleanRTJobBackupData(std::string strJobID)
{
#ifdef RT_BURN_DISK_BACKUP
    std::string strRTJobBackupDir=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerRTBackupDir"), true)+strJobID,true);
    if (DirectoryUtil::IsDirExist(strRTJobBackupDir.c_str()))
    {
        LOG_INFO(("[BurnServerAgent::CleanRTJobBackupData] Will delete dir : %s,%d\r\n",
            strRTJobBackupDir.c_str(),__LINE__));

        DirectoryUtil::DeleteDir(strRTJobBackupDir.c_str());
    }
#endif
}

void BurnServerAgent::ListLeftJobs()
{

}

void BurnServerAgent::SaveJob(std::string strJobID)
{

}

void BurnServerAgent::CleanRTBurnBackupXML(std::string strJobID)
{
    LOG_INFO(("[BurnServerAgent::CleanRTBurnBackupXML] Will clean backup xml,job id : %s,%d\r\n",
        strJobID.c_str(),__LINE__));

    std::string strCurDir=BurnServerAgent::GetCurDir();
    LOG_INFO(("[BurnServerAgent::CleanRTBurnBackupXML] Cur dir : %s,%d\r\n",
        strCurDir.c_str(),__LINE__));

    std::vector<std::string> vecFile;
    if (DirectoryUtil::GetFileList(strCurDir,vecFile))
    {
        std::vector<std::string>::iterator iter;
        for (iter=vecFile.begin();iter!=vecFile.end();++iter)
        {
            if (iter->find("RTBURNBACKUP") != std::string::npos &&
                iter->find(".xml") != std::string::npos &&
                iter->find(strJobID) != std::string::npos)
            {
                if (FileUtil::FileExist(iter->c_str()))
                {
                    LOG_INFO(("[BurnServerAgent::CleanRTBurnBackupXML] Will delete file : %s,%d\r\n",
                        iter->c_str(),__LINE__));

                    FileUtil::DelFile(iter->c_str());
                }
            }
        }
    }
}

void BurnServerAgent::ReadRTBurnBackupXML()
{
#ifdef WIN32
    assert(false);
#else
    std::string strJobID;
    std::string strDiscLabel;
    std::string strUpperIP;
    std::string strUpperPort;

    std::string strCurDir=BurnServerAgent::GetCurDir();

    std::vector<std::string> vecFile;
    if (DirectoryUtil::GetFileList(strCurDir,vecFile))
    {
        std::vector<std::string>::iterator iter;
        for (iter=vecFile.begin();iter!=vecFile.end();++iter)
        {
            if (iter->find("RTBURNBACKUP") != std::string::npos &&
                iter->find(".xml") != std::string::npos)
            {
                {
                    TiXmlDocument xmlDoc;
                    bool bLoadFile=false;

                    bLoadFile=xmlDoc.LoadFile(iter->c_str(),TIXML_ENCODING_UTF8);
                    if (bLoadFile)
                    {
                        TiXmlNode *pRoot=xmlDoc.RootElement();
                        if (pRoot != NULL)
                        {
                            TiXmlNode *pJobVec=pRoot->FirstChildElement("jobs");
                            if (pJobVec != NULL)
                            {
                                TiXmlNode *pJob=pJobVec->FirstChildElement("job");;
                                if (pJob != NULL)
                                {
                                    TiXmlNode *pTempNode=NULL;
                                    const char *p=NULL;

                                    pTempNode=pJob->FirstChildElement("jobID");
                                    if (pTempNode != NULL)
                                    {
                                        p=pTempNode->ToElement()->GetText();
                                        if (p != NULL)
                                        {
                                            strJobID=std::string(p);
                                        }
                                    }

                                    pTempNode=pJob->FirstChildElement("discLabel");
                                    if (pTempNode != NULL)
                                    {
                                        p=pTempNode->ToElement()->GetText();
                                        if (p != NULL)
                                        {
                                            strDiscLabel=std::string(p);
                                        }
                                    }

                                    pTempNode=pJob->FirstChildElement("upperIP");
                                    if (pTempNode != NULL)
                                    {
                                        p=pTempNode->ToElement()->GetText();
                                        if (p != NULL)
                                        {
                                            strUpperIP=std::string(p);
                                        }
                                    }

                                    pTempNode=pJob->FirstChildElement("upperPort");
                                    if (pTempNode != NULL)
                                    {
                                        p=pTempNode->ToElement()->GetText();
                                        if (p != NULL)
                                        {
                                            strUpperPort=std::string(p);
                                        }
                                    }

                                    BURN_PARAMETER burnParam;
                                    burnParam.nTaskRunningFlag=TASK_STATE_READY_TO_RUN;
                                    burnParam.nTaskControlCmd=TASK_CONTROL_STATE_DEFAULT;
                                    burnParam.strTaskID=UUID_GetUUID();

                                    {
                                        ZOSMutexLocker locker(&m_mutexDevVec);

                                        if (!m_vecDev.empty())
                                        {
                                            burnParam.vecDevHandle.push_back(GetDeviceHandle(m_vecDev.at(0).GetID()));
                                        }
                                    }

                                    burnParam.burnMode=MEDIAFILE_BURN_LOCAL_FILE;

                                    BURN_DATA_SOURCE source;
                                    source.strType=SOURCE_TYPE_DIR;
                                    source.strSourceUrl=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerRTBackupDir"), true)+"/"+strJobID+"/",true);

                                    burnParam.mapBurnDataSource["RTBURNBACKUP"].push_back(source);
                                    burnParam.nTaskRunningFlag=TASK_STATE_READY_TO_RUN;
                                    burnParam.strJobID=strJobID;
                                    burnParam.strDiscLabel=strDiscLabel;
                                    burnParam.strUpperIP=strUpperIP;
                                    burnParam.strUpperPort=strUpperPort;
                                    burnParam.strTaskPropertyFlag=TASK_PROPERTY_NORMAL;

                                    printf("---------- Task param --------\r\n");
                                    printf("Job id : %s\r\n",strJobID.c_str());
                                    printf("Task id : %s\r\n",burnParam.strTaskID.c_str());
                                    printf("Data : %s\r\n",source.strSourceUrl.c_str());
                                    printf("Disc label : %s\r\n",burnParam.strDiscLabel.c_str());
                                    printf("Upper ip : %s\r\n",burnParam.strUpperIP.c_str());
                                    printf("Upper port : %s\r\n",burnParam.strUpperPort.c_str());
                                    printf("----------------------------------\r\n");

                                    AddTask(burnParam);
                                }
                            }
                        }
                    }
                    else
                    {
                    }
                }

#if 1
                if (FileUtil::FileExist(iter->c_str()))
                {
                    FileUtil::DelFile(iter->c_str());
                }
#endif
            }
        }
    }
#endif
}

void BurnServerAgent::ClearBackupBurnTasks(std::string strFatherTaskID)
{

}

// static bool MultiByteToUTF16(unsigned char *pMultiByte, int nMultiByteSize, unsigned char *pUTF16, int *pUTF16Size, int nCodePage);
std::string BurnServerAgent::UTF8ToGB18030(std::string strSrc)
{
    std::string strRet;

    unsigned char *pUTF8=NULL;
    unsigned char *pUTF16=NULL;
    unsigned char *pGB18030=NULL;

    size_t nLen=strSrc.length();
    if (nLen > 0)
    {
        pUTF8=new unsigned char[nLen+1];

        int nUTF16BufSize=2*nLen+1;
        pUTF16=new unsigned char[2*nLen+1];

        if (pUTF8 != NULL &&
            pUTF16 != NULL)
        {
            strcpy((char *)pUTF8,strSrc.c_str());

            if (CharsetConvert::MultiByteToUTF16(pUTF8,nLen,pUTF16,&nUTF16BufSize,
                CharsetConvert::UTF8_CODEPAGE_ID))
            {
                int nGB18030BufSize=2*nUTF16BufSize+1;
                pGB18030=new unsigned char[nGB18030BufSize];

                if (pGB18030 != NULL)
                {
                    if (CharsetConvert::UTF16ToMultiByte(pUTF16,nUTF16BufSize,pGB18030,
                        &nGB18030BufSize,CharsetConvert::GB18030_CODEPAGE_ID))
                    {
                        strRet=std::string((char *)(pGB18030));
                    }
                    else
                    {
                        LOG_ERROR(("[BurnServerAgent::UTF8ToGB18030] UTF16ToMultiByte failed,%d\r\n",__LINE__));
                    }
                }
            }
            else
            {
                LOG_ERROR(("[BurnServerAgent::UTF8ToGB18030] MultiByteToUTF16 failed,%d\r\n",__LINE__));
            }
        }
    }

    if (pUTF8 != NULL)
    {
        delete [] pUTF8;
        pUTF8=NULL;
    }

    if (pUTF16 != NULL)
    {
        delete [] pUTF16;
        pUTF16=NULL;
    }

    if (pGB18030 != NULL)
    {
        delete [] pGB18030;
        pGB18030=NULL;
    }

    if (strRet != "")
    {
        LOG_INFO(("[BurnServerAgent::UTF8ToGB18030] Success,%d\r\n",__LINE__));
    }

    return strRet;
}

int BurnServerAgent::FileAnywhere_FeedBack(FileAnywhereObserverParameter parameter)
{
    // 1 - progress
    // 2 - success
    // 3 - failed
    // 4 - local file anywhere is not start
    // 5 - user delete download task

    std::string strTaskID=parameter.TaskID();
    std::string strPercent="-1";
    char szPercent[8]={0};

    //UINT64 nStartTime = ZOS::milliseconds();

    switch (parameter.EventType())
    {
    case 5:
        LOG_INFO(("[BurnServerAgent::FileAnywhere_FeedBack] ******* Delete download task : %s\r\n *******\r\n",
            strTaskID.c_str()));
        break;

    case 3:
    case 4:
        break;

    case 1:
    case 2:
        sprintf(szPercent,"%d",parameter.Percentage());
        strPercent=std::string(szPercent);

        if ("100" == strPercent)
        {
            LOG_INFO(("[BurnServerAgent::FileAnywhere_FeedBack] Single file download success\r\n"));
        }
        break;

    default:
        break;
    }

    if ("-1" == strPercent ||
        "100" == strPercent)
    {
        UpdateRtJobDownloadState(strTaskID,strPercent);
        UpdateNormalJobDownloadState(strTaskID,strPercent);
    }

    //LOG_DEBUG(("[BurnServerAgent::FileAnywhere_FeedBack] time used %"F_NUM_64"d\r\n", ZOS::milliseconds() - nStartTime));

    return 0;
}

INT64 BurnServerAgent::GetFileListSize(std::string strIP,std::string strPort,std::string strRemotePath)
{
    INT64 nRet=0;

    if ("" == strIP ||
        "" == strPort ||
        "" == strRemotePath)
    {
        LOG_WARNING(("[BurnServerAgent::GetFileSize] Parameter error\r\n"));
        return nRet;
    }

    std::string strHostIP=GetHostIP();

    if (strIP == strHostIP)
    {
        //本地文件
        nRet=DirectoryUtil::GetDirSize(strRemotePath.c_str());

        LOG_INFO(("[BurnServerAgent::GetFileListSize] Dir size : %llu\r\n",
            nRet));
    }
    else
    {
        std::string strProtocolContent;
        strProtocolContent+="<fileAnywhere>\r\n";

        strProtocolContent+="<commandType>";
        strProtocolContent+="getFileList";
        strProtocolContent+="</commandType>\r\n";

        strProtocolContent+="<fileDir>";
        strProtocolContent+=strRemotePath;
        strProtocolContent+="</fileDir>\r\n";

        strProtocolContent+="</fileAnywhere>\r\n";

        std::string strResponse;
        strResponse=CInterfaceProtocol::InterfaceCommunicate(strIP,atoi(strPort.c_str()),
            strProtocolContent,DOWNLOAD_PROTOCOL_TAG);
        if (strResponse.length() > 0)
        {
            if (0 == StringToInt(CInterfaceProtocol::GetTagValue(strResponse,"returnCode")))
            {
                nRet=StringToInt64(CInterfaceProtocol::GetTagValue(strResponse,"totalSize"));
            }
            else
            {
                LOG_ERROR(("[BurnServerAgent::GetFileSize] Get folder size error(return code error),folder path : %s\r\n",
                    strRemotePath.c_str()));
            }
        }
        else
        {
            LOG_ERROR(("[BurnServerAgent::GetFileSize] Get folder size error(no response),folder path : %s\r\n",
                strRemotePath.c_str()));
        }
    }

    return nRet;
}

INT64 BurnServerAgent::GetFileSize(std::string strIP,std::string strPort,std::string strRemotePath)
{
    INT64 nRet=0;

    if ("" == strIP ||
        "" == strPort ||
        "" == strRemotePath)
    {
        LOG_WARNING(("[BurnServerAgent::GetFileSize] Parameter error\r\n"));
        return nRet;
    }

    std::string strHostIP=GetHostIP();

    if (strIP == strHostIP)
    {
        //本地文件
        nRet=FileUtil::FileSize(strRemotePath.c_str());
    } 
    else
    {
        //网络文件
        std::string strProtocolContent;
        strProtocolContent+="<fileAnywhere>\r\n";

        strProtocolContent+="<commandType>";
        strProtocolContent+="getFileProperty";
        strProtocolContent+="</commandType>\r\n";

        strProtocolContent+="<filePath>";
        strProtocolContent+=strRemotePath;
        strProtocolContent+="</filePath>\r\n";

        strProtocolContent+="</fileAnywhere>\r\n";

        std::string strResponse;
        strResponse=CInterfaceProtocol::InterfaceCommunicate(strIP,atoi(strPort.c_str()),
            strProtocolContent,DOWNLOAD_PROTOCOL_TAG);
        if (strResponse.length() > 0)
        {
            if (0 == StringToInt(CInterfaceProtocol::GetTagValue(strResponse,"returnCode")))
            {
                nRet=StringToInt64(CInterfaceProtocol::GetTagValue(strResponse,"fileTotalSize"));
            }
            else
            {
                LOG_ERROR(("[BurnServerAgent::GetFileSize] Get file size error(return code error),file name : %s\r\n",
                    strRemotePath.c_str()));
            }
        }
        else
        {
            LOG_ERROR(("[BurnServerAgent::GetFileSize] Get file size error(no response),file name : %s\r\n",
                strRemotePath.c_str()));
        }
    }

    return nRet;
}

#ifdef WIN32
DWORD WINAPI StartNormalBurnThread(LPVOID lpParameter)
{
    if (lpParameter != NULL)
    {
        THREAD_PARAM *pThreadParam=(THREAD_PARAM *)lpParameter;

        CNormalBurnJobInfoEx *pNormalJob=(CNormalBurnJobInfoEx *)(pThreadParam->pThreadData);
        BurnServerAgent *pBurnServerAgent=(BurnServerAgent *)(pThreadParam->pUserParam);

        pBurnServerAgent->StartNormalBurn(*pNormalJob);

        delete pNormalJob;
        delete pThreadParam;
    }

    return 0;
}

DWORD WINAPI DeleteDownloadTaskThread(LPVOID lpParameter)
{
    if (lpParameter != NULL)
    {
        THREAD_PARAM *pThreadParam=(THREAD_PARAM *)(lpParameter);

        if (pThreadParam->pThreadData != NULL)
        {
            std::string strDownloadTaskID=std::string((char *)(pThreadParam->pThreadData));

            if (strDownloadTaskID != "")
            {
                int nRet=FileAnywhereManager::GetInstance()->DeleteTransmissionTask(strDownloadTaskID);
                if (0 == nRet)
                {
                    LOG_INFO(("[DeleteDownloadTaskThread] Delete download task success : %s,%d\r\n",
                        strDownloadTaskID.c_str(),__LINE__));
                }
                else
                {
                    LOG_ERROR(("[DeleteDownloadTaskThread] Delete download task failed : %s,%d\r\n",
                        strDownloadTaskID.c_str(),__LINE__));
                }
            }

            delete [] (char *)(pThreadParam->pThreadData);
        }

        delete pThreadParam;
    }

    return 0;
}
#else
void * StartNormalBurnThread(void *pThreadParam)
{
    if (pThreadParam != NULL)
    {
        THREAD_PARAM *p=(THREAD_PARAM *)pThreadParam;

        CNormalBurnJobInfoEx *pNormalJob=(CNormalBurnJobInfoEx *)(p->pThreadData);
        BurnServerAgent *pBurnServerAgent=(BurnServerAgent *)(p->pUserParam);

        pBurnServerAgent->StartNormalBurn(*pNormalJob);

        delete pNormalJob;
        delete p;
    }

    return NULL;
}

void * DeleteDownloadTaskThread(void *p)
{
    if (p != NULL)
    {
        THREAD_PARAM *pThreadParam=(THREAD_PARAM *)(p);

        std::string strDownloadTaskID=std::string((char *)(pThreadParam->pThreadData));

        if (strDownloadTaskID != "")
        {
            LOG_INFO(("[DeleteDownloadTaskThread] +++++++ Delete download task : %s +++++++\r\n",strDownloadTaskID.c_str()));

            FileAnywhereManager::GetInstance()->DeleteTransmissionTask(strDownloadTaskID);
        }

        delete [] (char *)(pThreadParam->pThreadData);

        delete pThreadParam;
    }

    return NULL;
}
#endif

void BurnServerAgent::UpdateNormalJobDownloadState(std::string strUUID,std::string strPercent)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    //strStateFlag //《刻录协议》被动类型接口2.7.4
    std::string strStateFlag;
    std::string strStateDescription;

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormalJob;
    for (iterNormalJob=m_vecNormalJob.begin();
        iterNormalJob!=m_vecNormalJob.end();)
    {
        std::string strJobID=iterNormalJob->GetJobID();

        const std::map<std::string,FILE_DOWNLOAD_INFO> &mapFileDownload=
            iterNormalJob->GetMapFileDownloadState();
        if (!mapFileDownload.empty())
        {
            if (mapFileDownload.find(strUUID) != mapFileDownload.end())
            {
                if ("100" == strPercent)
                {
                    iterNormalJob->SetDownloadCount(iterNormalJob->GetDownloadCount()+1);

                    if (iterNormalJob->GetDownloadCount() == iterNormalJob->GetTotalCount())
                    {
                        LOG_INFO(("[BurnServerAgent::UpdateDownloadState] Job download success\r\n"));

                        strStateFlag=STATE_DEFAULT;
                        strStateDescription="Download success,start to burn";

                        SendCurrentBurnStateToUpper(iterNormalJob->GetUpServerIP(),iterNormalJob->GetUpServerPort(),
                            strJobID,strStateFlag,gBurnStateCfgFile.GetValue("info","downloadsuccess"));

                        //下载完成，可以开始刻录，创建新的线程
                        THREAD_PARAM *pThreadParam=new THREAD_PARAM;
                        if (pThreadParam != NULL)
                        {
                            CNormalBurnJobInfoEx *pNormalBurnJobInfoEx=new CNormalBurnJobInfoEx(*iterNormalJob);
                            if (pNormalBurnJobInfoEx != NULL)
                            {
                                pThreadParam->pThreadData=pNormalBurnJobInfoEx;
                                pThreadParam->pUserParam=this;
#ifdef WIN32
                                HANDLE hThread=CreateThread(NULL,0,StartNormalBurnThread,pThreadParam,0,NULL);
                                if (NULL == hThread)
                                {
                                    delete pNormalBurnJobInfoEx;
                                    pNormalBurnJobInfoEx=NULL;

                                    delete pThreadParam;
                                    pThreadParam=NULL;
                                }
                                else
                                {
                                    CloseHandle(hThread);
                                }
#else

                                LOG_INFO(("[BurnServerAgent::UpdateDownloadState] Create a thread to start normal burn...\r\n"));

                                pthread_t t;
                                if (pthread_create(&t,NULL,StartNormalBurnThread,pThreadParam) != 0)
                                {
                                    delete pNormalBurnJobInfoEx;
                                    pNormalBurnJobInfoEx=NULL;

                                    delete pThreadParam;
                                    pThreadParam=NULL;
                                }
                                else
                                {
                                    pthread_detach(t);
                                }
#endif
                            }
                            else
                            {
                                delete pThreadParam;
                                pThreadParam=NULL;
                            }
                        }
                    }
                    else
                    {
                        strStateFlag=STATE_DOWNLOADING;

                        //LOG_INFO(("已经下载 %d 个文件，总共 %d 个文件\r\n",iterNormalBurnJobInfoEx->GetDownloadCount(),
                        //    iterNormalBurnJobInfoEx->GetTotalCount()));

                        int nDownloadSuccess=iterNormalJob->GetDownloadCount();
                        int nDownloadTotal=iterNormalJob->GetTotalCount();

                        strStateDescription=gBurnStateCfgFile.GetValue("info","downloadinformation");

                        size_t nPos=strStateDescription.find("#");
                        strStateDescription.replace(nPos,1,IntToString(nDownloadSuccess).c_str());

                        nPos=strStateDescription.find("#");
                        strStateDescription.replace(nPos,1,IntToString(nDownloadTotal).c_str());

                        SendCurrentBurnStateToUpper(iterNormalJob->GetUpServerIP(),iterNormalJob->GetUpServerPort(),
                            strJobID,strStateFlag,strStateDescription);
                    }
                } 
                else if("-1" == strPercent)
                {
                    strStateFlag=STATE_BURN_ERR;
                    strStateDescription=gBurnStateCfgFile.GetValue("info","downloadfailed");

                    std::string strDownloadErrorInfo=gBurnStateCfgFile.GetValue("info","downloadfailed");

                    

                    std::map<std::string,FILE_DOWNLOAD_INFO>::const_iterator iterDownloadFile;
                    iterDownloadFile=mapFileDownload.find(strUUID);

                    if (iterDownloadFile->second.strFileName != "")
                    {
                        strDownloadErrorInfo+=",";
                        strDownloadErrorInfo+=iterDownloadFile->second.strFileName;
                    }

                    SendCurrentBurnStateToUpper(iterNormalJob->GetUpServerIP(),iterNormalJob->GetUpServerPort(),
                        strJobID,strStateFlag,strDownloadErrorInfo);

                    //下载出错，删除当前任务
                    iterNormalJob=m_vecNormalJob.erase(iterNormalJob);

                    LOG_INFO(("[BurnServerAgent::UpdateDownloadState] Download failed\r\n"));

                    //下载完成后删除任务
                    CleanJob(strJobID);
                }

                break;
            }
        }
        else
        {
            LOG_WARNING(("[BurnServerAgent::UpdateDownloadState] No download task\r\n"));
        }

        ++iterNormalJob;
    }
}

void BurnServerAgent::UpdateRtJobDownloadState(std::string strUUID,std::string strPercent)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    assert(strUUID != "");

    //strStateFlag //《刻录协议》被动类型接口2.7.4
    std::string strStateFlag;
    std::string strStateDescription;

    FILE_DOWNLOAD_INFO info;

    std::vector<CRTBurnInfoEx>::iterator iterRtJob;
    for (iterRtJob=m_vecRTBurnInfo.begin();iterRtJob!=m_vecRTBurnInfo.end(); )
    {
        std::string strJobID=iterRtJob->GetJobID();

        const std::map<std::string,FILE_DOWNLOAD_INFO> &mapFileDownload=
            iterRtJob->GetMapFileDownloadState();

        std::map<std::string,FILE_DOWNLOAD_INFO>::const_iterator iterFileDownloadInfo;

        if (!mapFileDownload.empty())
        {
            iterFileDownloadInfo=mapFileDownload.find(strUUID);

            if (iterFileDownloadInfo != mapFileDownload.end())
            {
                FILE_DOWNLOAD_INFO fileDownloadInfo=iterRtJob->GetMapFileDownloadState()[strUUID];

                if ("100" == strPercent)
                {
                    std::string strType=iterFileDownloadInfo->second.strType;

                    assert(strType != "");
                    LOG_INFO(("[BurnServerAgent::UpdateRtJobDownloadState] File type : %s\r\n",strType.c_str()));

                    if (TYPE_NOTE_FILE == strType)
                    {
                        m_BurnTask.SetNoteUpdateFlag(strJobID,NOTE_FILE_UPDATED_YES);

                        LOG_INFO(("[BurnServerAgent::UpdateRtJobDownloadState] Update note file success ........................\r\n"));

                        //仅仅是更新完笔录，实时刻录笔录更新不纳入下载统计中
                        return;
                    } 

                    iterRtJob->SetDownloadCount(iterRtJob->GetDownloadCount()+1);

                    LOG_INFO(("[BurnServerAgent::UpdateRtJobDownloadState] Rt burn download,total : %d,download : %d\r\n",
                        iterRtJob->GetTotalCount(),iterRtJob->GetDownloadCount()));

                    if (iterRtJob->GetDownloadCount() == iterRtJob->GetTotalCount())
                    {
                        LOG_INFO(("[BurnServerAgent::UpdateRtJobDownloadState] Job download success\r\n"));

                        strStateFlag=STATE_DEFAULT;
                        strStateDescription="Download success,start to burn";

                        //已经下载完了实时刻录需要下载的文件
                        LOG_INFO(("[BurnServerAgent::UpdateRtJobDownloadState] All files downloaded...\r\n"));

                        break;
                    }
                    else
                    {
                        strStateFlag=STATE_DOWNLOADING;

                        int nDownloadSuccess=iterRtJob->GetDownloadCount();
                        int nDownloadTotal=iterRtJob->GetTotalCount();

                        strStateDescription=gBurnStateCfgFile.GetValue("info","downloadinformation");

                        size_t nPos=strStateDescription.find("#");
                        if (nPos != std::string::npos)
                        {
                            strStateDescription.replace(nPos,1,IntToString(nDownloadSuccess).c_str());
                        }
                        else
                        {
                            LOG_WARNING(("[BurnServerAgent::UpdateDownloadState] Get information error : %s\r\n",
                                strStateDescription.c_str()));
                        }

                        nPos=strStateDescription.find("#");
                        if (nPos != std::string::npos)
                        {
                            strStateDescription.replace(nPos,1,IntToString(nDownloadTotal).c_str());
                        }
                        else
                        {
                            LOG_WARNING(("[BurnServerAgent::UpdateDownloadState] Get information error : %s\r\n",
                                strStateDescription.c_str()));
                        }

                    }
                } 
                else if("-1" == strPercent)
                {
                    strStateFlag=STATE_BURN_ERR;
                    strStateDescription=gBurnStateCfgFile.GetValue("info","downloadfailed");

                    std::string strType=iterFileDownloadInfo->second.strType;

                    assert(strType != "");
                    LOG_INFO(("[BurnServerAgent::UpdateRtJobDownloadState] File type : %s\r\n",strType.c_str()));

                    if (TYPE_NOTE_FILE == strType)
                    {
                        //如果更新笔录失败，应该立即返回，刻录本地其他数据，而不应该阻塞或者导致刻录失败
                        m_BurnTask.SetNoteUpdateFlag(strJobID,NOTE_FILE_UPDATED_YES);

                        LOG_ERROR(("[BurnServerAgent::UpdateRtJobDownloadState] Update note file failed ........................\r\n"));

                        return;
                    }

                    //实时刻录，即使下载失败，也不会删除当前任务，会继续刻录

                    LOG_ERROR(("[BurnServerAgent::UpdateRtJobDownloadState] Download failed\r\n"));
                }

                break;
            }
        }

        ++iterRtJob;
    }
}

void BurnServerAgent::SendCurrentBurnStateToUpper(std::string strUpperIP,
                                                  std::string strUpperPort,
                                                  std::string strJobID,
                                                  std::string strState,
                                                  std::string strStateDescription,
                                                  bool bDevInfo)
{
    LOG_INFO(("[BurnServerAgent::SendCurrentBurnStateToUpper] Job id : %s, \
              ip : %s,port : %s,%s,%s,%d\r\n",
              strJobID.c_str(),strUpperIP.c_str(),strUpperPort.c_str(),strState.c_str(),
              strStateDescription.c_str(),__LINE__));
    std::string strUpperType = "";

    std::vector<CCDROMDriverInfo> vecDevInfo;
    bool bJobFound=false;

    {
        ZOSMutexLocker locker(&m_mutexJobVec);

        //事后刻录
        std::vector<CNormalBurnJobInfoEx>::iterator iterNormalJob;
        for (iterNormalJob=m_vecNormalJob.begin();
            iterNormalJob!=m_vecNormalJob.end();++iterNormalJob)
        {
            if (iterNormalJob->GetJobID() == strJobID)
            {
                bJobFound=true;

                if ("" == strUpperType)
                {
                    strUpperType="tcp";
                }

                if ("" == strUpperIP)
                {
                    strUpperIP=iterNormalJob->GetUpServerIP();
                }

                if ("" == strUpperPort)
                {
                    strUpperPort=iterNormalJob->GetUpServerPort();
                }

                vecDevInfo=iterNormalJob->GetBurnServerInfo().GetVecCDROMDriverInfo();

                break;
            }
        }

        //实时刻录任务
        if (!bJobFound)
        {
            std::vector<CRTBurnInfoEx>::iterator iterRTJob;
            for (iterRTJob=m_vecRTBurnInfo.begin();iterRTJob!=m_vecRTBurnInfo.end();
                ++iterRTJob)
            {
                if (iterRTJob->GetJobID() == strJobID)
                {
                    bJobFound=true;

                    if ("" == strUpperType)
                    {
                        strUpperType=iterRTJob->GetUpServerType();
                    }

                    if ("" == strUpperIP)
                    {
                        strUpperIP=iterRTJob->GetUpServerIP();
                    }

                    if ("" == strUpperPort)
                    {
                        strUpperPort=iterRTJob->GetUpServerPort();
                    }

                    vecDevInfo=iterRTJob->GetBurnServerInfo().GetVecCDROMDriverInfo();

                    break;
                }
            }
        }
    }   

    if ("" == strUpperIP ||
        "" == strUpperPort)
    {
        return;
    }

    if (!bJobFound)
    {
        //如果找不到Job，那么推断这应该是一个刻录备份的task反馈的状态信息
        strJobID="";
    }

    if (!bDevInfo)
    {
        vecDevInfo.clear();
    }

    std::string strProtocolContent;
    strProtocolContent=CInterfaceProtocol::GenerateProtocolSendBurnState(strJobID,
        strState,strStateDescription,vecDevInfo);

    if (strProtocolContent != "")
    {
#if 0
        THREAD_PARAM *pThreadParam=new THREAD_PARAM;
        if (pThreadParam != NULL)
        {
            int nContentLen=strProtocolContent.length();
            pThreadParam->pThreadData=new char[nContentLen+1];

            if (NULL == pThreadParam->pThreadData)
            {
                delete pThreadParam;
                pThreadParam=NULL;

                return;
            }
            strcpy((char *)(pThreadParam->pThreadData),strProtocolContent.c_str());

            pThreadParam->pUserParam=this;

            pThreadParam->p1=new char[128];
            if (NULL == pThreadParam->p1)
            {
                delete [] (char *)(pThreadParam->pThreadData);
                pThreadParam->pThreadData=NULL;

                delete pThreadParam;
                pThreadParam=NULL;

                return;
            }
            strcpy((char *)(pThreadParam->p1),strUpperIP.c_str());

            pThreadParam->p2=new char[128];
            if (NULL == pThreadParam->p2)
            {
                delete [] (char *)(pThreadParam->pThreadData);
                pThreadParam->pThreadData=NULL;

                delete [] (char *)(pThreadParam->p1);
                pThreadParam->p1=NULL;

                delete pThreadParam;
                pThreadParam=NULL;

                return;
            }
            strcpy((char *)(pThreadParam->p2),strUpperPort.c_str());

#ifdef WIN32
            HANDLE hThread=CreateThread(NULL,0,SendJobStateThread,pThreadParam,0,NULL);
            if (NULL == hThread)
            {
                delete [] (char *)(pThreadParam->pThreadData);
                pThreadParam->pThreadData=NULL;

                delete [] (char *)(pThreadParam->p1);
                pThreadParam->p1=NULL;

                delete pThreadParam;
                pThreadParam=NULL;

                delete [] (char *)(pThreadParam->p2);
                pThreadParam->p2=NULL;

                delete pThreadParam;
                pThreadParam=NULL;

                return;
            }
            else
            {
                CloseHandle(hThread);
            }
#else
            pthread_t t;
            printf("[BurnServerAgent::SendCurrentBurnStateToUpper] Before pthread_create Job id : %s, \
                      ip : %s,port : %s,%s,%s,%d\r\n",
                      strJobID.c_str(),strUpperIP.c_str(),strUpperPort.c_str(),strState.c_str(),
                      strStateDescription.c_str(),__LINE__);
            if (pthread_create(&t,NULL,SendJobStateThread,pThreadParam) != 0)
            {
                delete [] (char *)(pThreadParam->pThreadData);
                pThreadParam->pThreadData=NULL;

                delete [] (char *)(pThreadParam->p1);
                pThreadParam->p1=NULL;

                delete pThreadParam;
                pThreadParam=NULL;

                delete [] (char *)(pThreadParam->p2);
                pThreadParam->p2=NULL;

                delete pThreadParam;
                pThreadParam=NULL;

                return;
            }
            else
            {
                pthread_detach(t);
            }
#endif
        }
#endif

#if 1
        if("udp" == strUpperType)
        {
            //rtb.state=jobID,jobState,jobStateDescription\n
            std::string strContent;
            strContent = "rtb.state=";
            strContent += strJobID;
            strContent += ",";
            strContent += strState;
            strContent += ",";
            strContent += strStateDescription;
            strContent += "\n";
            UDPClient::Send(strUpperIP.c_str(),StringToInt(strUpperPort),strContent.c_str(),strContent.length());
        }
        else
        {
            BURNSENDSTATE_PARAMETER burnSendStateParam;
            burnSendStateParam.strUpIP = strUpperIP;
            burnSendStateParam.strUpPort = strUpperPort;
            burnSendStateParam.strProtocalContent = strProtocolContent;
            m_BurnSendStateTask.AddTask(burnSendStateParam);
        }
#endif
    }
}

bool BurnServerAgent::IsDevBeingUsed(DEV_HANDLE handle)
{
    bool bRet=false;

    return bRet;
}

void BurnServerAgent::UpdateDevInfo()
{
    ZOSMutexLocker locker(&m_mutexDevVec);

    if (!m_vecDev.empty())
    {
        //更新光驱信息：是否正在被使用、是否插入光盘、光盘总容量、光盘剩余容量
#if 0
        //底层SDK的Burn_Ctrl_Dev_Get_HaveDisc函数返回时间太长，阻塞协议返回
        //暂时业务中以下更新的信息不关心，先不检测
        size_t nDevNum=m_vecDev.size();
        for (size_t i=0;i<nDevNum;++i)
        {
            std::string strDevID=m_vecDev.at(i).GetID();

            DEV_HANDLE h=NULL;

            std::map<DEV_HANDLE,std::string>::const_iterator iter;
            for (iter=m_mapDev.begin();iter!=m_mapDev.end();++iter)
            {
                if (iter->second == strDevID)
                {
                    h=iter->first;

                    break;
                }
            }

            if (h != NULL)
            {
                BURN_DISC_INFO_T discInfo;
                memset(&discInfo,0,sizeof(discInfo));

#ifdef WIN32
#else
                if (!m_BurnTask.GetDevUsed(h))
                {
                    LOG_INFO(("[BurnServerAgent::UpdateDevInfo] `   xxxxxxxx Dev %p not being used xxxxxxxx,%d\r\n",
                        h,__LINE__));

                    m_vecDev.at(i).SetHasCD(
                        (ERR_NO_HAVE_DISC == Burn_Ctrl_Dev_Get_HaveDisc(h)) ? DEV_HAS_CD_NO : DEV_HAS_CD_YES);

                    LOG_INFO(("************** %d\r\n",__LINE__));

                    if (ERR_NO_HAVE_DISC == Burn_Ctrl_Dev_Get_HaveDisc(h))
                    {
                        LOG_WARNING(("[BurnServerAgent::UpdateDevInfo] Detect no disc,%p,%d\r\n",
                            h,__LINE__));
                    }
                    else
                    {
                    }

                    LOG_INFO(("************** %d\r\n",__LINE__));
                }
                else
                {
                    m_vecDev.at(i).SetHasCD(DEV_HAS_CD_YES);

                    LOG_INFO(("[BurnServerAgent::UpdateDevInfo] xxxxxxxx Dev %p is being used xxxxxxxx,%d\r\n",
                        h,__LINE__));
                }

                if (DEV_HAS_CD_YES == m_vecDev.at(i).GetHasCD() &&
                    BURN_SUCCESS == Burn_Get_DiscInfo(h,&discInfo))
                {
                    m_vecDev.at(i).SetCDLeftCapacity(IntToString((int)(discInfo.freesize)));
                    m_vecDev.at(i).SetCDTotalCapacity(IntToString((int)(discInfo.freesize+discInfo.usedsize)));
                }

                printf("-------- Dev %s information : --------\r\n",strDevID.c_str());
                printf("Free size : %u\r\n",discInfo.freesize);
                printf("Has disc : %s\r\n",(DEV_HAS_CD_YES == m_vecDev.at(i).GetHasCD()) ? "yes" : "no");
                printf("Used size : %u\r\n",discInfo.usedsize);
                printf("-------- Dev %s information : --------\r\n",strDevID.c_str());
#endif
            }
        }
#endif
    } 
    else
    {
        //调用底层接口，获取光驱列表

        LOG_INFO(("[BurnServerAgent::UpdateDevInfo] m_vecDev.empty! \r\n"));
        DEV_SYS_INFO_T info;
        if (BURN_SUCCESS == Burn_Get_DeviceNum(&info))
        {
            LOG_INFO(("[BurnServerAgent::UpdateDevInfo] Burn_Get_DeviceNum! \r\n"));
            std::vector<DEV_HANDLE> vecHandle;

            vecHandle.push_back(Burn_Dev_Init(info.dev1));
            vecHandle.push_back(Burn_Dev_Init(info.dev2));
            vecHandle.push_back(Burn_Dev_Init(info.dev3));
            vecHandle.push_back(Burn_Dev_Init(info.dev4));

            LOG_INFO(("[BurnServerAgent::UpdateDevInfo] Burn_Dev_Init End! \r\n"));
            size_t nCount=vecHandle.size();
            for (size_t i=0;i<nCount;++i)
            {
                DEV_HANDLE h=vecHandle.at(i);

                if (h != NULL)
                {
                    LOG_INFO(("[BurnServerAgent::UpdateDev] Dev handle :%p\r\n",(void *)h));

                    CCDROMDriverInfo info;
                    char szDevID[128]={0};
                    sprintf(szDevID,"%d",m_vecDev.size());
                    info.SetID(szDevID);

                    std::string strDevID=std::string(szDevID);
                    m_mapDev[h]=strDevID;

                    //先解锁仓门
                    Burn_Ctrl_LockDoor(h, FALSE);
                    m_vecDev.push_back(info);

#if 0
                    //底层SDK的Burn_Ctrl_Dev_Get_HaveDisc函数返回时间太长，阻塞协议返回
                    //暂时业务中以下更新的信息不关心，先不检测
#ifdef WIN32
                    //Windows下检测光盘信息非常缓慢
#else
                    LOG_INFO(("[BurnServerAgent::UpdateDevInfo] Dev Info begin! \r\n"));
                    m_vecDev.back().SetHasCD(
                        (ERR_NO_HAVE_DISC == Burn_Ctrl_Dev_Get_HaveDisc(h)) ? DEV_HAS_CD_NO : DEV_HAS_CD_YES);

                    BURN_DISC_INFO_T discInfo;
                    memset(&discInfo,0,sizeof(discInfo));

                    if (DEV_HAS_CD_YES == m_vecDev.back().GetHasCD() &&
                        BURN_SUCCESS == Burn_Get_DiscInfo(h,&discInfo))
                    {
                        m_vecDev.back().SetCDLeftCapacity(IntToString((int)(discInfo.freesize)));
                        m_vecDev.back().SetCDTotalCapacity(IntToString((int)(discInfo.freesize+discInfo.usedsize)));
                    }

                    printf("-------- Dev %s information : --------\r\n",strDevID.c_str());
                    printf("Free size : %u\r\n",discInfo.freesize);
                    printf("Has disc : %s\r\n",(DEV_HAS_CD_YES == m_vecDev.back().GetHasCD()) ? "yes" : "no");
                    printf("Used size : %u\r\n",discInfo.usedsize);
                    printf("-------- Dev %s information : --------\r\n",strDevID.c_str());
                    LOG_INFO(("[BurnServerAgent::UpdateDevInfo] Dev Info end! \r\n"));

#endif
#endif
                }
            }

            LOG_INFO(("-----------------Dev list-----------------\r\n"));
            std::vector<CCDROMDriverInfo>::iterator iter;
            for (iter=m_vecDev.begin();iter!=m_vecDev.end();++iter)
            {
                LOG_INFO(("Dev id : %s,handle : %p\r\n",
                    iter->GetID().c_str(),GetDeviceHandle(iter->GetID())));
            }
            LOG_INFO(("------------------------------------------\r\n"));

        }
        else
        {
            LOG_ERROR(("[BurnServerAgent::UpdateDevInfo] Burn_Get_DeviceNum failed,%d\r\n",
                __LINE__));
        }
    }
}

std::string BurnServerAgent::GetHostIP()
{
    std::string strHostIP;

#ifdef WIN32
    strHostIP=NetUtil::GetHostIP();
#else
    struct ifaddrs * ifAddrStruct=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct!=NULL)
    {
        if (ifAddrStruct->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

            if(strcmp(ifAddrStruct->ifa_name, "eth0") == 0)
            {
                //printf("%s IPV4 Address %s\n", ifAddrStruct->ifa_name, addressBuffer);
                strHostIP=std::string(addressBuffer);
            }
        }

        ifAddrStruct=ifAddrStruct->ifa_next;
    }
#endif

    return strHostIP;
}


bool BurnServerAgent::GetCDROMDriverInformationFromHandle(DEV_HANDLE h,CCDROMDriverInfo &info)
{
    bool bRet=false;

    if (!m_mapDev.empty())
    {
        if (m_mapDev.find(h) != m_mapDev.end())
        {
            {
                ZOSMutexLocker locker(&m_mutexJobVec);

                std::vector<CCDROMDriverInfo>::iterator iter;
                for (iter=m_vecDev.begin();iter!=m_vecDev.end();++iter)
                {
                    if (m_mapDev[h] == iter->GetID())
                    {
                        info=*iter;

                        bRet=true;
                        break;
                    }
                }
            }
        }
        else
        {
            bRet=false;
        }
    }
    else
    {
        LOG_ERROR(("[BurnServerAgent::GetCDROMDriverInformationFromHandle] No CDROM driver\r\n"));
        bRet=false;
    }

    return bRet;
}
void BurnServerAgent::SetCDROMDriverInformation(DEV_HANDLE h,const CCDROMDriverInfo &info)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    if (!m_mapDev.empty())
    {
        if (m_mapDev.find(h) != m_mapDev.end())
        {
            std::vector<CCDROMDriverInfo>::iterator iter;
            for (iter=m_vecDev.begin();iter!=m_vecDev.end();++iter)
            {
                if (m_mapDev[h] == iter->GetID())
                {
                    *iter=info;
                    break;
                }
            }
        }
        else
        {
            if (h != NULL)
            {
                m_mapDev[h]=info.GetID();
                m_vecDev.push_back(info);
            }
        }
    }
    else
    {
        if (h != NULL)
        {
            m_mapDev[h]=info.GetID();
            m_vecDev.push_back(info);
        }
    }
}

DEV_HANDLE BurnServerAgent::GetDeviceHandle(std::string strID)
{
    DEV_HANDLE h=NULL;

    std::map<DEV_HANDLE,std::string>::iterator iter;
    for (iter=m_mapDev.begin();iter!=m_mapDev.end();++iter)
    {
        if (iter->second == strID)
        {
            h=iter->first;
            break;
        }
    }

    if (iter == m_mapDev.end())
    {
        LOG_WARNING(("[BurnServerAgent::GetDeviceHandle] Find no handle,id : %s\r\n",strID.c_str()));
    }

    return h;
}

void BurnServerAgent::PushExtraDataSource(const CNormalBurnJobInfoEx &normalJobInfo,std::map< std::string,std::vector<BURN_DATA_SOURCE> > &mapBurnDataSource)
{
    std::string strJobID=normalJobInfo.GetJobID();

    LOG_INFO(("[BurnServerAgent::PushExtraDataSource] Enter!\r\n"));

    if ("" == strJobID)
    {
        LOG_ERROR(("[BurnServerAgent::PushExtraDataSource] Job id is empty\r\n"));

        return;
    }

    BURN_DATA_SOURCE data;

    //笔录
    if (normalJobInfo.GetNoteFileName() != "")
    {
        data.strType=SOURCE_TYPE_NORMAL_FILE;
        data.strSourceUrl=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)
            +strJobID+"/"+strJobID+"/"+normalJobInfo.GetNoteFileName(),false);


        mapBurnDataSource["note"].push_back(data);

        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] Add note : %s\r\n",data.strSourceUrl.c_str()));
    }
    else
    {
        LOG_WARNING(("[BurnServerAgent::PushExtraDataSource] No note\r\n"));
    }


    //播放器
    data.strType=SOURCE_TYPE_DIR;
    data.strSourceUrl=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)+strJobID+"/"+"PlayBack/",true);
    if (DirectoryUtil::GetDirSize(data.strSourceUrl.c_str()) > 0)
    {
        mapBurnDataSource["player"].push_back(data);

        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] Add player : %s\r\n",data.strSourceUrl.c_str()));
    } 
    else
    {
        LOG_WARNING(("[BurnServerAgent::PushExtraDataSource] No player,path : %s,size : %d\r\n",
            data.strSourceUrl.c_str(),DirectoryUtil::GetDirSize(data.strSourceUrl.c_str())));
    }


    //自动运行文件
    if (normalJobInfo.GetAutorunFilePath() != "")
    {
        data.strType=SOURCE_TYPE_NORMAL_FILE;
        data.strSourceUrl=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)+strJobID+
            "/AutoRun.inf",false);

        mapBurnDataSource["autorun"].push_back(data);

        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] Add autorun : %s\r\n",data.strSourceUrl.c_str()));
    }
    else
    {
        LOG_WARNING(("[BurnServerAgent::PushExtraDataSource] No autorun\r\n"));
    }

    // playlist文件
    if (normalJobInfo.GetPlaylistFileInfo().GetDownloadURL() != "")
    {
        data.strType=SOURCE_TYPE_NORMAL_FILE;
        data.strSourceUrl=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)+strJobID+"/"+strJobID+
            "/"+ExtractFileOrDirName(BurnTask::Path(normalJobInfo.GetPlaylistFileInfo().GetDownloadURL(),false)),false);

        mapBurnDataSource["playlist"].push_back(data);

        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] Add play list file : %s\r\n",normalJobInfo.GetPlaylistFileInfo().GetDownloadURL().c_str()));
    } 
    else
    {
        LOG_WARNING(("[BurnServerAgent::PushExtraDataSource] No play list\r\n"));
    }

    //光盘访问控制文件
    data.strType=SOURCE_TYPE_NORMAL_FILE;

#ifdef WIN32
    std::string strTemp=CharsetConvertMFC::UTF16ToUTF8(CStringW(CStringA(GetCurDir().c_str()))).GetBuffer()+std::string("Auth.exe");
    data.strSourceUrl=strTemp;
#else
    data.strSourceUrl=BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)+strJobID+"/"+"Auth.exe";
#endif
    mapBurnDataSource["auth"].push_back(data);
}

void BurnServerAgent::PushExtraDataSource(const CRTBurnInfoEx &rtJobInfo,std::map< std::string,std::vector<BURN_DATA_SOURCE> > &mapBurnDataSource)
{
    std::string strJobID=rtJobInfo.GetJobID();

    if ("" == strJobID)
    {
        LOG_ERROR(("[BurnServerAgent::PushExtraDataSource] Job id is empty\r\n"));

        assert(false);

        return;
    }

    BURN_DATA_SOURCE data;

    //笔录
    if (rtJobInfo.GetNoteFileDownloadURL() != "")
    {
        //防止笔录有中文名字
        std::string strNoteFileName=ExtractFileOrDirName(rtJobInfo.GetNoteFileDownloadURL());
        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] Note file name : %s\r\n",
            strNoteFileName.c_str()));

        data.strType=SOURCE_TYPE_NORMAL_FILE;
        data.strSourceUrl=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)+strJobID+"/"+"note_1.doc",false);

        //将笔录路经转换为GB编码
#ifdef TEST_RT_BURN_CHARSET
        data.strSourceUrl=BurnServerAgent::UTF8ToGB18030(data.strSourceUrl);
#endif

        mapBurnDataSource["note"].push_back(data);

        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] Add note : %s\r\n",data.strSourceUrl.c_str()));
    }
    else
    {
        LOG_WARNING(("[BurnServerAgent::PushExtraDataSource] No note\r\n"));
    }


    //播放器
    //注意 ：此时可能还没有下载播放器
    data.strType=SOURCE_TYPE_DIR;
    data.strSourceUrl=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)+strJobID+"/"+"PlayBack/",true);
    //UINT64 nPlayerSize=DirectoryUtil::GetDirSize(data.strSource.c_str());
    if (/*nPlayerSize > 0*/true)
    {
        mapBurnDataSource["player"].push_back(data);

        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] Add player : %s\r\n",data.strSourceUrl.c_str()));
    } 
    //else
    //{
    //    LOG_WARNING(("[BurnServerAgent::PushExtraDataSource] No player,source : %s,player dir size : %d\r\n",
    //        data.strSource.c_str(),nPlayerSize));
    //}


    //自动运行文件
    if (rtJobInfo.GetAutorunFilePath() != "")
    {
        std::string str1=rtJobInfo.GetAutorunFilePath();
        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] str1 : %s\r\n",str1.c_str()));

        std::string str2=BurnTask::Path(rtJobInfo.GetAutorunFilePath(),false);
        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] str2 : %s\r\n",str2.c_str()));

        std::string str3=ExtractFileOrDirName(BurnTask::Path(rtJobInfo.GetAutorunFilePath(),false));
        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] str3 : %s\r\n",str3.c_str()));

        std::string str4=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)
            +strJobID+"/"+ExtractFileOrDirName(BurnTask::Path(rtJobInfo.GetAutorunFilePath(),false)),false);
        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] str4 : %s\r\n",str4.c_str()));

        data.strType=SOURCE_TYPE_NORMAL_FILE;
        data.strSourceUrl=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)
            +strJobID+"/"+ExtractFileOrDirName(BurnTask::Path(rtJobInfo.GetAutorunFilePath(),false)),false);

        mapBurnDataSource["autorun"].push_back(data);

        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] Add autorun : %s\r\n",data.strSourceUrl.c_str()));
    }
    else
    {
        LOG_WARNING(("[BurnServerAgent::PushExtraDataSource] No autorun\r\n"));
    }

    // playlist文件
    if (rtJobInfo.GetPlaylistFileInfo().GetDownloadURL() != "")
    {
        data.strType=SOURCE_TYPE_NORMAL_FILE;
        data.strSourceUrl=BurnTask::Path(BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)
            +strJobID+"/"+ExtractFileOrDirName(BurnTask::Path(rtJobInfo.GetPlaylistFileInfo().GetDownloadURL(),false)),false);

        mapBurnDataSource["playlist"].push_back(data);

        LOG_INFO(("[BurnServerAgent::PushExtraDataSource] Add play list file : %s\r\n",data.strSourceUrl.c_str()));
    } 
    else
    {
        LOG_WARNING(("[BurnServerAgent::PushExtraDataSource] No play list\r\n"));
    }

    //光盘访问控制文件
    data.strType=SOURCE_TYPE_NORMAL_FILE;

#ifdef WIN32
    std::string strTemp=CharsetConvertMFC::UTF16ToUTF8(CStringW(CStringA(GetCurDir().c_str()))).GetBuffer()+std::string("Auth.exe");
    data.strSourceUrl=strTemp;
#else
    data.strSourceUrl=BurnTask::Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true)
        +strJobID+"/"+"Auth.exe";
#endif
    mapBurnDataSource["auth"].push_back(data);
}

void BurnServerAgent::SetTaskState(std::string strJobID,std::string strTaskID,std::string strStateFlag,std::string strStateDescription)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    bool bJobFound=false;

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormalJob;
    for (iterNormalJob=m_vecNormalJob.begin();iterNormalJob!=m_vecNormalJob.end();++iterNormalJob)
    {
        if (iterNormalJob->GetJobID() == strJobID)
        {
            bJobFound=true;

            std::map<std::string,std::string> &mapTask=iterNormalJob->GetMapBurnTask();
            if (mapTask.find(strTaskID) != mapTask.end())
            {
                mapTask[strTaskID]=strStateFlag;
                LOG_INFO(("[BurnServerAgent::SetTaskState] m_vecNormalJob strJobID: %s,strTaskID: %s,strStateFlag: %s,strStateDescription: %s\r\n"
                    ,strJobID.c_str(),strTaskID.c_str(),strStateFlag.c_str(),strStateDescription.c_str()));
            }

            break;
        }
    }

    if (!bJobFound)
    {
        std::vector<CRTBurnInfoEx>::iterator iterRTJob;
        for (iterRTJob=m_vecRTBurnInfo.begin();iterRTJob!=m_vecRTBurnInfo.end();++iterRTJob)
        {
            if (iterRTJob->GetJobID() == strJobID)
            {
                std::map<std::string,std::string> &mapTask=iterRTJob->GetMapBurnTask();
                if (mapTask.find(strTaskID) != mapTask.end())
                {
                    mapTask[strTaskID]=strStateFlag;
                    LOG_INFO(("[BurnServerAgent::SetTaskState] m_vecRTBurnInfo strJobID: %s,strTaskID: %s,strStateFlag: %s,strStateDescription: %s\r\n"
                        ,strJobID.c_str(),strTaskID.c_str(),strStateFlag.c_str(),strStateDescription.c_str()));
                }

                break;
            }
        }
    }
}

bool BurnServerAgent::SetDevBurnState(DEV_HANDLE handle,std::string strStateFlag,std::string strStateDescription)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    bool bRet=false;

    std::vector<CCDROMDriverInfo>::iterator iterDev;

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormalJob;
    for (iterNormalJob=m_vecNormalJob.begin();iterNormalJob!=m_vecNormalJob.end();++iterNormalJob)
    {
        std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo=iterNormalJob->GetBurnServerInfo().GetVecCDROMDriverInfo();

        bool bDevFound=false;

        for (iterDev=vecCDROMDriverInfo.begin();iterDev!=vecCDROMDriverInfo.end();++iterDev)
        {
            if (GetDeviceHandle(iterDev->GetID()) == handle)
            {
                iterDev->SetStateFlag(strStateFlag);
                iterDev->SetStateDescription(strStateDescription);

                LOG_INFO(("[BurnServerAgent::SetDevBurnState] Dev id : %s,state flag : %s,state description : %s \r\n",
                    iterDev->GetID().c_str(),strStateFlag.c_str(),strStateDescription.c_str()));

                bDevFound=true;

                bRet=true;

                break;
            }
        }

        if (bDevFound)
        {
            break;
        }
    }

    std::vector<CRTBurnInfoEx>::iterator iterRTJob;
    for (iterRTJob=m_vecRTBurnInfo.begin();iterRTJob!=m_vecRTBurnInfo.end();++iterRTJob)
    {
        std::vector<CCDROMDriverInfo> &vecDevInfo=iterRTJob->GetBurnServerInfo().GetVecCDROMDriverInfo();

        bool bDevFound=false;

        for (iterDev=vecDevInfo.begin();iterDev!=vecDevInfo.end();++iterDev)
        {
            if (GetDeviceHandle(iterDev->GetID()) == handle)
            {
                iterDev->SetStateFlag(strStateFlag);
                iterDev->SetStateDescription(strStateDescription);

                LOG_INFO(("[BurnServerAgent::SetDevBurnState] Dev id : %s,state flag : %s,state description : %s \r\n",
                    iterDev->GetID().c_str(),strStateFlag.c_str(),strStateDescription.c_str()));

                bDevFound=true;

                bRet=true;

                break;
            }
        }

        if (bDevFound)
        {
            break;
        }
    }

    return bRet;
}

bool BurnServerAgent::AddTask(const BURN_PARAMETER &param)
{
    if (!m_BurnTask.AddTask(param))
    {
        LOG_ERROR(("[BurnServerAgent::AddTask] AddTask failed,%d\r\n",__LINE__));

        return false;
    }

    LOG_INFO(("[BurnServerAgent::AddTask] Add task success,%d\r\n",__LINE__));

    ZOSMutexLocker locker(&m_mutexJobVec);

    bool bJobFound=false;

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormalJob;
    for (iterNormalJob=m_vecNormalJob.begin();iterNormalJob!=m_vecNormalJob.end();++iterNormalJob)
    {
        if (iterNormalJob->GetJobID() == param.strJobID)
        {
            bJobFound=true;

            iterNormalJob->GetMapBurnTask().insert(std::make_pair(param.strTaskID,STATE_DEFAULT));
        }
    }

    if (!bJobFound)
    {
        std::vector<CRTBurnInfoEx>::iterator iterRTJob;
        for (iterRTJob=m_vecRTBurnInfo.begin();iterRTJob!=m_vecRTBurnInfo.end();++iterRTJob)
        {
            if (iterRTJob->GetJobID() == param.strJobID)
            {
                iterRTJob->GetMapBurnTask().insert(std::make_pair(param.strTaskID,STATE_DEFAULT));
            }
        }
    }

    return true;
}

void BurnServerAgent::ConfirmRTBurnBackup(std::string strJobID)
{
    LOG_INFO(("[BurnServerAgent::ConfirmRTBurnBackup] Upper activate rt burn backup task(s) in job %s\r\n",
        strJobID.c_str()));

    m_BurnTask.ActivateRTBurnBackupTasks(strJobID);
}

void BurnServerAgent::CancelRTBurnBackup(std::string strJobID)
{
    LOG_INFO(("[BurnServerAgent::CancelRTBurnBackup] Upper cancel rt burn backup task(s) in job %s\r\n",
        strJobID.c_str()));

    m_BurnTask.CancelRTBurnBackupTasks(strJobID);
}

void BurnServerAgent::StartRTBurnBackup(std::string strJobID)
{
    m_BurnTask.StartRTBurnBackupTasks(strJobID);
}


int BurnServerAgent::IsJobDone(std::string strJobID)
{
    int nRet=-1;

    ZOSMutexLocker locker(&m_mutexJobVec);

    bool bJobFound=false;

    size_t nTaskCount=0;
    size_t nTaskRunningCount=0;
    size_t nTaskFailedCount=0;
    size_t nTaskSuccessCount=0;

    std::vector<CNormalBurnJobInfoEx>::iterator iterNormalJob;
    for (iterNormalJob=m_vecNormalJob.begin();iterNormalJob!=m_vecNormalJob.end();++iterNormalJob)
    {
        if (iterNormalJob->GetJobID() == strJobID)
        {
            bJobFound=true;

            const std::map<std::string,std::string> &mapTask=iterNormalJob->GetMapBurnTask();

            nTaskCount=mapTask.size();

            std::map<std::string,std::string>::const_iterator iter;
            for (iter=mapTask.begin();iter!=mapTask.end();++iter)
            {
                if (STATE_TASK_OVER == iter->second 
                    || STATE_SINGLE_DISC_BURNED == iter->second)
                {
                    nTaskSuccessCount++;
                }
                else if (STATE_SINGLE_DEV_ERR == iter->second)
                {
                    nTaskFailedCount++;
                } 
                else if(STATE_BURNING == iter->second ||
                    STATE_DEFAULT == iter->second)
                {
                    nTaskRunningCount++;
                }
            }

            break;
        }
    }

    if (!bJobFound)
    {
        std::vector<CRTBurnInfoEx>::iterator iterRTJob;
        for (iterRTJob=m_vecRTBurnInfo.begin();iterRTJob!=m_vecRTBurnInfo.end();++iterRTJob)
        {
            if (iterRTJob->GetJobID() == strJobID)
            {
                bJobFound=true;

                const std::map<std::string,std::string> &mapTask=iterRTJob->GetMapBurnTask();

                nTaskCount=mapTask.size();

                std::map<std::string,std::string>::const_iterator iter;
                for (iter=mapTask.begin();iter!=mapTask.end();++iter)
                {
                    if (STATE_TASK_OVER == iter->second ||
                        STATE_RTTASK_STOPPED == iter->second ||
                        STATE_SINGLE_DISC_BURNED == iter->second)//实时刻录task停止了，也表示一个task完成
                    {
                        nTaskSuccessCount++;
                    }
                    else if (STATE_SINGLE_DEV_ERR == iter->second)
                    {
                        nTaskFailedCount++;
                    } 
                    else if(STATE_BURNING == iter->second ||
                        STATE_DEFAULT == iter->second ||
                        STATE_RTTASK_PAUSED == iter->second ||
                        STATE_WILL_CLOSE_DISC == iter->second ||
                        STATE_RTTASK_STOPPING == iter->second)//实时刻录task暂停了，也表示一个task正在运行
                    {
                        nTaskRunningCount++;
                    }
                }

                break;
            }
        }
    }

    LOG_INFO(("[BurnServerAgent::IsJobDone] Task count : %d,success : %d,failed : %d,running : %d\r\n",
        nTaskCount,nTaskSuccessCount,nTaskFailedCount,nTaskRunningCount));

    assert(nTaskCount == nTaskSuccessCount+nTaskFailedCount+nTaskRunningCount);

    if (nTaskFailedCount+nTaskSuccessCount == nTaskCount)//已经执行完成
    {
        if (nTaskFailedCount == nTaskCount)
        {
            nRet=-1;
        }
        else if (nTaskSuccessCount == nTaskCount)
        {
            nRet=7;
        }
        else if (nTaskSuccessCount < nTaskCount)
        {
            nRet=2;
        }
    }
    else//正在执行
    {
        if (nTaskRunningCount > 0)
        {
            nRet=1;
        }
    }

    return nRet;
}

bool BurnServerAgent::IsJobRunning(std::string strJobID)
{
    bool bRet=false;

    size_t nTotal=0;
    size_t nRunning=0;//不包括刻录暂停的task

    ZOSMutexLocker locker(&m_mutexJobVec);

    std::vector<CRTBurnInfoEx>::iterator iterRtJob;
    for (iterRtJob=m_vecRTBurnInfo.begin();iterRtJob!=m_vecRTBurnInfo.end();++iterRtJob)
    {
        if (iterRtJob->GetJobID() == strJobID)
        {
            const std::map<std::string,std::string> &mapTasks=iterRtJob->GetMapBurnTask();

            nTotal=mapTasks.size();

            std::map<std::string,std::string>::const_iterator iterTask;
            for (iterTask=mapTasks.begin();iterTask!=mapTasks.end();++iterTask)
            {
                std::string strTaskState=iterTask->second;

                if (STATE_RTTASK_PAUSED != strTaskState)
                {
                    ++nRunning;
                }
            }

            break;
        }
    }

    if (nRunning == nTotal)
    {
        bRet=true;
    }
    else
    {
        bRet=false;
    }

    return bRet;
}

bool BurnServerAgent::IsJobPaused(std::string strJobID)
{
    bool bRet=false;

    size_t nTotal=0;
    size_t nPaused=0;//包括刻录失败的task

    ZOSMutexLocker locker(&m_mutexJobVec);

    std::vector<CRTBurnInfoEx>::iterator iterRtJob;
    for (iterRtJob=m_vecRTBurnInfo.begin();iterRtJob!=m_vecRTBurnInfo.end();++iterRtJob)
    {
        if (iterRtJob->GetJobID() == strJobID)
        {
            const std::map<std::string,std::string> &mapTasks=iterRtJob->GetMapBurnTask();

            nTotal=mapTasks.size();

            std::map<std::string,std::string>::const_iterator iterTask;
            for (iterTask=mapTasks.begin();iterTask!=mapTasks.end();++iterTask)
            {
                std::string strTaskState=iterTask->second;

                if (STATE_SINGLE_DEV_ERR == strTaskState ||
                    STATE_RTTASK_PAUSED == strTaskState)
                {
                    ++nPaused;
                }
            }

            break;
        }
    }

    assert(iterRtJob != m_vecRTBurnInfo.end());

    if (nPaused == nTotal)
    {
        bRet=true;
    }
    else
    {
        bRet=false;
    }

    return bRet;
}

bool BurnServerAgent::IsJobStopped(std::string strJobID)
{
    bool bRet=false;

    size_t nTotal=0;
    size_t nStopped=0;//包括刻录失败的task

    ZOSMutexLocker locker(&m_mutexJobVec);

    std::vector<CRTBurnInfoEx>::iterator iterRtJob;
    for (iterRtJob=m_vecRTBurnInfo.begin();iterRtJob!=m_vecRTBurnInfo.end();++iterRtJob)
    {
        if (iterRtJob->GetJobID() == strJobID)
        {
            const std::map<std::string,std::string> &mapTasks=iterRtJob->GetMapBurnTask();

            nTotal=mapTasks.size();

            std::map<std::string,std::string>::const_iterator iterTask;
            for (iterTask=mapTasks.begin();iterTask!=mapTasks.end();++iterTask)
            {
                std::string strTaskState=iterTask->second;

                if (STATE_SINGLE_DEV_ERR == strTaskState ||
                    STATE_RTTASK_STOPPED == strTaskState)
                {
                    ++nStopped;
                }
            }

            break;
        }
    }

    assert(iterRtJob != m_vecRTBurnInfo.end());

    if (nStopped == nTotal)
    {
        bRet=true;
    }
    else
    {
        bRet=false;
    }

    LOG_INFO(("[BurnServerAgent::IsJobStopped] Total : %u,stopped : %u,%d\r\n",
        nTotal,nStopped,__LINE__));

    return bRet;
}

bool BurnServerAgent::IsJobStopping(std::string strJobID)
{
    ZOSMutexLocker locker(&m_mutexJobVec);

    std::vector<CRTBurnInfoEx>::iterator iterRtJob;
    for (iterRtJob=m_vecRTBurnInfo.begin();iterRtJob!=m_vecRTBurnInfo.end();++iterRtJob)
    {
        if (iterRtJob->GetJobID() == strJobID)
        {
            const std::map<std::string,std::string> &mapTasks=iterRtJob->GetMapBurnTask();

            std::map<std::string,std::string>::const_iterator iterTask;
            for (iterTask=mapTasks.begin();iterTask!=mapTasks.end();++iterTask)
            {
                std::string strTaskState=iterTask->second;

                if (STATE_RTTASK_STOPPING == strTaskState)
                {
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

void BurnServerAgent::AddDevBurnFilesInfo(const DEV_BURN_FILES &devBurnFiles)
{
    std::string strJobID=devBurnFiles.strJobID;

    std::map< DEV_HANDLE,std::vector<BURN_DATA_SOURCE> >::const_iterator iter;
    for (iter=devBurnFiles.mapDevBurnFiles.begin();iter!=devBurnFiles.mapDevBurnFiles.end();++iter)
    {
        DEV_HANDLE hDev=iter->first;
        std::string strDevID;
        if (m_mapDev.find(hDev) != m_mapDev.end())
        {
            strDevID=m_mapDev[hDev];
        }

        const std::vector<BURN_DATA_SOURCE> &vecData=iter->second;
        size_t nDataCount=vecData.size();

        CLocationInfo location;
        location.SetID("0");
        
        std::vector<CSingleFileInfo> vecFile;
        for (size_t i=0;i<nDataCount;++i)
        {
            CSingleFileInfo file;

            std::string strFilePath=vecData.at(i).strSourceUrl;

#ifdef WIN32
            CStringW strFilePathW=CharsetConvertMFC::UTF8ToUTF16(strFilePath.c_str());

            std::wstring strFileName;
            strFileName = FileUtil::GetFileNameW(strFilePathW.GetBuffer());

            file.SetFileName(CharsetConvertMFC::UTF16ToUTF8(CString(strFileName.c_str())).GetBuffer());
#else
#endif

            file.SetSize(IntToString(static_cast<int>(GetFileOrDirSize(file.GetFileName(),false))));

            vecFile.push_back(file);
        }

        location.SetVecFileInfo(vecFile);

        {
            ZOSMutexLocker locker(&m_mutexJobVec);

            std::vector<CNormalBurnJobInfoEx>::iterator iterNormalJob;
            for (iterNormalJob=m_vecNormalJob.begin();iterNormalJob!=m_vecNormalJob.end();++iterNormalJob)
            {
                if (iterNormalJob->GetJobID() == strJobID)
                {
                    std::vector<CCDROMDriverInfo> &vecDev=iterNormalJob->GetBurnServerInfo().GetVecCDROMDriverInfo();
                    size_t nDevCount=vecDev.size();
                    size_t i;
                    for (i=0;i<nDevCount;++i)
                    {
                        if (vecDev.at(i).GetID() == strDevID)
                        {
                            vecDev.at(i).GetVecLocationInfo().clear();

                            vecDev.at(i).GetVecLocationInfo().push_back(location);

                            break;
                        }
                    }

                    break;
                }
            }
        }
    }
}

void BurnServerAgent::AddDevBurnFilesInfo(std::string strJobID,std::string strDevID,
                                          const std::vector<CLocationInfo> &vecLocationInfo)
{
    bool bJobFound=false;

    {
        ZOSMutexLocker locker(&m_mutexJobVec);

        //事后刻录
        std::vector<CNormalBurnJobInfoEx>::iterator iterNormalJob;
        for (iterNormalJob=m_vecNormalJob.begin();
            iterNormalJob!=m_vecNormalJob.end();++iterNormalJob)
        {
            if (iterNormalJob->GetJobID() == strJobID)
            {
                bJobFound=true;

                std::vector<CCDROMDriverInfo> &vecDev=
                    iterNormalJob->GetBurnServerInfo().GetVecCDROMDriverInfo();
                size_t nDevCount=vecDev.size();
                for (size_t i=0;i<nDevCount;++i)
                {
                    if (strDevID == vecDev.at(i).GetID())
                    {
                        vecDev.at(i).SetVecLocationInfo(vecLocationInfo);
                    }
                }

                break;
            }
        }

        //实时刻录任务
        if (!bJobFound)
        {
            std::vector<CRTBurnInfoEx>::iterator iterRTJob;
            for (iterRTJob=m_vecRTBurnInfo.begin();iterRTJob!=m_vecRTBurnInfo.end();
                ++iterRTJob)
            {
                if (iterRTJob->GetJobID() == strJobID)
                {
                    bJobFound=true;

                    std::vector<CCDROMDriverInfo> &vecDev=
                        iterRTJob->GetBurnServerInfo().GetVecCDROMDriverInfo();
                    size_t nDevCount=vecDev.size();
                    for (size_t i=0;i<nDevCount;++i)
                    {
                        if (strDevID == vecDev.at(i).GetID())
                        {
                            vecDev.at(i).SetVecLocationInfo(vecLocationInfo);
                        }
                    }

                    break;
                }
            }
        }
    }
}
