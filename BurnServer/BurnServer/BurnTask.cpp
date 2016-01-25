#ifdef WIN32
#include <afx.h>

#include "CharsetConvertMFC.h"
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
#endif

#include "BurnTask.h"
#include <iostream>

#ifdef WIN32
#else
#define NDEBUG
#endif
#include "tinyxml.h"

#include <stdio.h>

#include <algorithm>

#include "CommonUtil.h"

INT64 BurnTask::m_nLimitSize = -1;

static const int DEFAULT_DISC_ALARM_SIZE=500; 

BurnTask::BurnTask():ZOSThread("BurnTask"),
m_mutexBurnTaskParameter("mutexBurnTaskParameter"),
m_cb(NULL),
m_pUsr(NULL),
m_cbDevBurnFiles(NULL),
m_pUsrDevBurnFiles(NULL)
{
}

BurnTask::~BurnTask()
{

}

#ifdef WIN32
DWORD WINAPI BurnTaskThread(LPVOID lpParameter)
{
    if (lpParameter != NULL)
    {
        BURN_THREAD_PARAM *pBurnThreadParam=(BURN_THREAD_PARAM *)lpParameter;

        BurnTask *pBurnTask=(BurnTask *)pBurnThreadParam->pUsr;
        BURN_PARAMETER *pBurnParam=(BURN_PARAMETER *)pBurnThreadParam->pBurnParam;

        if (pBurnTask != NULL &&
            pBurnParam != NULL)
        {
            pBurnTask->BurnDisc(*pBurnParam);

            delete pBurnParam;
        }

        delete pBurnThreadParam;
    }

    return 0;
}
#else
void * BurnTaskThread(void *pThreadParam)
{
    if (pThreadParam != NULL)
    {
        BURN_THREAD_PARAM *pBurnThreadParam=(BURN_THREAD_PARAM *)pThreadParam;

        BurnTask *pBurnTask=(BurnTask *)pBurnThreadParam->pUsr;
        BURN_PARAMETER *pBurnParam=(BURN_PARAMETER *)pBurnThreadParam->pBurnParam;

        if (pBurnTask != NULL &&
            pBurnParam != NULL)
        {
            pBurnTask->BurnDisc(*pBurnParam);

            delete pBurnParam;
        }

        delete pBurnThreadParam;
    }

    return NULL;
}
#endif

BOOL BurnTask::Create()
{
    if (ZOSThread::Start())
    {
        return TRUE;
    }

    return FALSE;
}

BOOL BurnTask::Close()
{
    if (ZOSThread::Stop(TRUE))
    {
        return TRUE;
    }

    return FALSE;
}

BOOL BurnTask::OnThreadStart()
{
    return TRUE;
}

BOOL BurnTask::OnThreadEntry()
{
    if (!m_bStop)
    {
        HandleTask();

        ZOSThread::Sleep(20);
    }

    return TRUE;
}

BOOL BurnTask::OnThreadStop()
{
    return TRUE;
}

BOOL BurnTask::AddTask(const BURN_PARAMETER &param)
{
    BOOL bRet=FALSE;

    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    if (param.vecDevHandle.empty())
    {
        LOG_ERROR(("[BurnTask::AddTask] Dev empty,%d\r\n",__LINE__));

        return FALSE;
    }

    bool bDevUsed=false;

    if ((int)param.vecDevHandle.size() > 1)
    {//多盘续刻
        if (!IsTaskExsitByJobID(param.strJobID))
        {//多盘任务第一次添加，不是触发的
            std::vector<DEV_HANDLE>::const_iterator iterHandle;
            for (iterHandle=param.vecDevHandle.begin();iterHandle!=param.vecDevHandle.end();++iterHandle)
            {
                std::vector<BURN_PARAMETER>::const_iterator iterParam;
                for (iterParam=m_vecBurnParameter.begin();iterParam!=m_vecBurnParameter.end();++iterParam)
                {
                    if (std::find(iterParam->vecDevHandle.begin(),iterParam->vecDevHandle.end(),*iterHandle) !=
                        iterParam->vecDevHandle.end())
                    {
                        bDevUsed=true;

                        LOG_ERROR(("[BurnTask::AddTask] Dev is being used,task id : %s,job id : %s,%d\r\n",
                            iterParam->strTaskID.c_str(),iterParam->strJobID.c_str(),__LINE__));

                        break;
                    }
                }

                if (bDevUsed)
                {
                    break;
                }
            }
        }
        else
        {//多盘续刻触发的新任务
            LOG_INFO(("[BurnTask::AddTask] BURN_TYPE_MULTI_ASYN, A new task of job id : %s ,task id : %s start, %d\r\n",
                param.strJobID.c_str(),param.strTaskID.c_str(),__LINE__));
        }
    }
    else
    {
        std::vector<DEV_HANDLE>::const_iterator iterHandle;
        for (iterHandle=param.vecDevHandle.begin();iterHandle!=param.vecDevHandle.end();++iterHandle)
        {
            std::vector<BURN_PARAMETER>::const_iterator iterParam;
            for (iterParam=m_vecBurnParameter.begin();iterParam!=m_vecBurnParameter.end();++iterParam)
            {
                if (std::find(iterParam->vecDevHandle.begin(),iterParam->vecDevHandle.end(),*iterHandle) !=
                    iterParam->vecDevHandle.end())
                {
                    bDevUsed=true;

                    LOG_ERROR(("[BurnTask::AddTask] Dev is being used,task id : %s,job id : %s,%d\r\n",
                        iterParam->strTaskID.c_str(),iterParam->strJobID.c_str(),__LINE__));

                    break;
                }
            }

            if (bDevUsed)
            {
                break;
            }
        }
    }

    if (!bDevUsed || 
        TASK_PROPERTY_RT_BACKUP == param.strTaskPropertyFlag)
    {
        m_vecBurnParameter.push_back(param);

        bRet=TRUE;
    }

    return bRet;
}

void BurnTask::HandleTask()
{
    BURN_PARAMETER *pBurnParam=NULL;
    bool bTaskFound=false;

#ifdef WIN32
#else
    //static int t=0;
    //while (clock()-t > 10000)
    //{
    //    ListLeftTasks();

    //    t=clock();
    //}
#endif

    //查找空闲的任务，同时清理掉废弃的task
    {
        ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

        std::vector<BURN_PARAMETER>::iterator iter;
        for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
        {
            if (TASK_STATE_ERR == iter->nTaskRunningFlag ||
                TASK_STATE_NO_USE == iter->nTaskRunningFlag)
            {
                LOG_WARNING(("[BurnTask::HandleTask] Warning,begin to delete task : %s,%d\r\n",
                    iter->strTaskID.c_str(),__LINE__));

#ifdef RT_BURN_DISK_BACKUP
                if (MEDIAFILE_BURN_LOCAL_FILE == iter->burnMode &&
                    TASK_PROPERTY_RT_BACKUP == iter->strTaskPropertyFlag)
                {
                    size_t nDevNum=iter->vecDevHandle.size();
                    for (size_t i=0;i<nDevNum;++i)
                    {
                        DEV_HANDLE h=iter->vecDevHandle.at(i);

                        StopDiskBackup(h);
                    }
                }
#endif

                m_vecBurnParameter.erase(iter);

                ListLeftTasks();

                break;
            }
            else if (TASK_STATE_PAUSED == iter->nTaskRunningFlag)
            {
                continue;
            }
            else if (TASK_STATE_READY_TO_RUN == iter->nTaskRunningFlag)
            {
                bTaskFound=true;

                LOG_INFO(("[BurnTask::HandleTask] Find idle task : %s,%d\r\n",
                    iter->strTaskID.c_str(),__LINE__));

                iter->nTaskRunningFlag=TASK_STATE_RUNNING;

                pBurnParam=new BURN_PARAMETER;
                if (pBurnParam != NULL)
                {
                    *pBurnParam=*iter;
                }

                break;
            }
            else if (TASK_STATE_RUNNING == iter->nTaskRunningFlag)
            {
                continue;
            }
            else if (TASK_STATE_STOPPED == iter->nTaskRunningFlag)
            {
                LOG_WARNING(("[BurnTask::HandleTask] Warning,begin to delete task : %s,%d\r\n",
                    iter->strTaskID.c_str(),__LINE__));

                m_vecBurnParameter.erase(iter);

                ListLeftTasks();

                break;
            }
            else if (TASK_STATE_BURN_LOCAL == iter->nTaskRunningFlag)
            {
                continue;
            }
            else if (TASK_STATE_BURN_STREAM == iter->nTaskRunningFlag)
            {
                continue;
            }
#ifdef RT_BURN_DISK_BACKUP
            else if (TASK_STATE_BURN_BACKUP_READY == iter->nTaskRunningFlag)
            {
                bTaskFound=true;

                //检测到激活了的实时刻录本地备份文件的task
                static std::map<std::string,int> mapTaskLogInfo;
                if ( mapTaskLogInfo.empty() ||
                    ( !mapTaskLogInfo.empty() && mapTaskLogInfo.find(iter->strTaskID) == mapTaskLogInfo.end() ))
                {
                    LOG_INFO(("[BurnTask::HandleTask] Detect rt burn backup task : %d\r\n",__LINE__));
                    LOG_INFO(("#### task id : %s\r\n",iter->strTaskID.c_str()));
                    LOG_INFO(("#### job id : %s\r\n",iter->strJobID.c_str()));
                    LOG_INFO(("#### Upper ip : %s\r\n",iter->strUpperIP.c_str()));
                    LOG_INFO(("#### Upper port : %s\r\n",iter->strUpperPort.c_str()));
                    LOG_INFO(("#### data source : \r\n"));

                    std::map< std::string,std::vector<BURN_DATA_SOURCE> >::const_iterator iterData;
                    for (iterData=iter->mapBurnDataSource.begin();iterData!=iter->mapBurnDataSource.end();
                        ++iterData)
                    {
                        std::string strLocationID=iterData->first;
                        size_t nDataNum=iterData->second.size();
                        if (strLocationID != "" &&
                            nDataNum > 0)
                        {
                            if (iterData->second.at(0).strSourceUrl != "")
                            {
                                LOG_INFO(("%s, %s\r\n",iterData->first.c_str(),
                                    iterData->second.at(0).strSourceUrl.c_str()));
                            }
                        }
                    }
                    LOG_INFO(("\r\n\r\n"));

                    mapTaskLogInfo[iter->strTaskID]=0;
                }

                iter->nTaskRunningFlag=TASK_STATE_RUNNING;

                pBurnParam=new BURN_PARAMETER;
                if (pBurnParam != NULL)
                {
                    *pBurnParam=*iter;
                }

                break;
            }
#endif
        }
    }

    if (bTaskFound &&
        pBurnParam != NULL)
    {
        BURN_THREAD_PARAM *pBurnThreadParam=new BURN_THREAD_PARAM;

        if (NULL == pBurnThreadParam)
        {
            return;
        }

        pBurnThreadParam->pBurnParam=pBurnParam;
        pBurnThreadParam->pUsr=this;

#ifdef WIN32
        HANDLE hThread=CreateThread(NULL,0,BurnTaskThread,pBurnThreadParam,0,NULL);
        if (NULL == hThread)
        {
            delete pBurnParam;
            delete pBurnThreadParam;

            LOG_ERROR(("[BurnTask::HandleTask] Create burn task thread failed,%d\r\n",__LINE__));
        }
        else
        {
            CloseHandle(hThread);
        }
#else
        pthread_t t;
        if (pthread_create(&t,NULL,BurnTaskThread,pBurnThreadParam) != 0)
        {
            delete pBurnParam;
            delete pBurnThreadParam;

            LOG_ERROR(("[BurnTask::HandleTask] Create burn task thread failed\r\n"));
        }
        else
        {
            pthread_detach(t);
        }
#endif
    }
}

void BurnTask::SaveRTBurnBackupTask(BURN_PARAMETER burnParam)
{
#ifdef WIN32
#else
    std::string strJobID=burnParam.strJobID;
    std::string strDiscLabel=burnParam.strDiscLabel;
    std::string strUpperIP=burnParam.strUpperIP;
    std::string strUpperPort=burnParam.strUpperPort;

    std::string strProtocolContent;
    strProtocolContent+="<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n";

    strProtocolContent+="<burnControl>";
    strProtocolContent+="<jobs>";

    strProtocolContent+="<job>";

    strProtocolContent+="<jobID>";
    strProtocolContent+=strJobID;
    strProtocolContent+="</jobID>";

    strProtocolContent+="<discLabel>";
    strProtocolContent+=strDiscLabel;
    strProtocolContent+="</discLabel>";

    strProtocolContent+="<upperIP>";
    strProtocolContent+=strUpperIP;
    strProtocolContent+="</upperIP>";

    strProtocolContent+="<upperPort>";
    strProtocolContent+=strUpperPort;
    strProtocolContent+="</upperPort>";


    strProtocolContent+="</job>";

    strProtocolContent+="</jobs>";
    strProtocolContent+="</burnControl>";

    TiXmlDocument xmlDocument;
    /*const char *p=*/xmlDocument.Parse(strProtocolContent.c_str(),NULL,TIXML_ENCODING_UTF8);

    char szCwd[1024]={0};
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

    std::string strPath=std::string(szCwd);
    strPath+=UUID_GetUUID()+strJobID+"RTBURNBACKUP"+".xml";

    if (xmlDocument.SaveFile(strPath.c_str()))
    {
        LOG_ERROR(("[BurnTask::SaveRTBurnBackupTask] Success,%d\r\n",__LINE__));
    }
    else
    {
        LOG_ERROR(("[BurnTask::SaveRTBurnBackupTask] Failed,%d\r\n",__LINE__));
    }
#endif
}

void BurnTask::SetBurnTaskCallBack(BURN_TASK_CALLBACK cb,void *p)
{
    m_cb=cb;
    m_pUsr=p;
}

void BurnTask::SetDevBurnFilesCallBack(DEV_BURN_FILE_CALLBACK cb,void *p)
{
    m_cbDevBurnFiles=cb;
    m_pUsrDevBurnFiles=p;
}

bool BurnTask::IsDevReady(DEV_HANDLE handle,BURN_PARAMETER burnParam,BURN_MODE mode)
{
    LOG_INFO(("[BurnTask::IsDevReady] Begin to check dev %p,%d\r\n",handle,__LINE__));

    assert(handle != NULL);

    BURN_STATE_PARAMETER stateParam;

    if (NULL == handle)
    {
        stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","invalidhandle");;
        stateParam.strJobID=burnParam.strJobID;
        stateParam.strTaskID=burnParam.strTaskID;
        stateParam.strUpperIP=burnParam.strUpperIP;
        stateParam.strUpperPort=burnParam.strUpperPort;
        stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

        if (m_cb != NULL)
        {
            m_cb(handle,stateParam,m_pUsr);
        }

        SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

        SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

        return false;
    }

    int nTaskControlState;
    int nRet;
    int nCheckCount=0;
    int nCheckDiscCanWriteCount=0;
    while (true)
    {
        //////////////////////////////////////////////////////////////////////////
        //为了快速响应暂停和停止实时刻录命令
        nTaskControlState=GetTaskControlState(burnParam.strTaskID);

        LOG_INFO(("[BurnTask::IsDevReady] Task control state : %d,%d\r\n",nTaskControlState,__LINE__));

        if (TASK_CONTROL_STATE_STOP == nTaskControlState)
        {
            SetTaskRunningState(burnParam.strTaskID,TASK_STATE_STOPPED);

            stateParam.strStateFlag=STATE_RTTASK_STOPPED;
            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopped");
            stateParam.strJobID=burnParam.strJobID;
            stateParam.strTaskID=burnParam.strTaskID;
            stateParam.strUpperIP=burnParam.strUpperIP;
            stateParam.strUpperPort=burnParam.strUpperPort;
            stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

            if (m_cb != NULL)
            {
                DEV_HANDLE handle=GetCurDev(burnParam.strTaskID);
                m_cb(handle,stateParam,m_pUsr);
            }

            SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

            return false;
        }
        else if (TASK_CONTROL_STATE_PAUSE == nTaskControlState)
        {
            SetTaskRunningState(burnParam.strTaskID,TASK_STATE_PAUSED);

            stateParam.strStateFlag=STATE_RTTASK_PAUSED;
            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","paused");
            stateParam.strJobID=burnParam.strJobID;
            stateParam.strTaskID=burnParam.strTaskID;
            stateParam.strUpperIP=burnParam.strUpperIP;
            stateParam.strUpperPort=burnParam.strUpperPort;
            stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

            if (m_cb != NULL)
            {
                DEV_HANDLE handle=GetCurDev(burnParam.strTaskID);
                m_cb(handle,stateParam,m_pUsr);
            }

            nTaskControlState=GetTaskControlState(burnParam.strTaskID);
            while (TASK_CONTROL_STATE_PAUSE == nTaskControlState)
            {
                ZOSThread::Sleep(1000);

                nTaskControlState=GetTaskControlState(burnParam.strTaskID);
            }

            nCheckCount=0;

            stateParam.strStateFlag=STATE_BURNING;
            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burning");
            stateParam.strJobID=burnParam.strJobID;
            stateParam.strTaskID=burnParam.strTaskID;
            stateParam.strUpperIP=burnParam.strUpperIP;
            stateParam.strUpperPort=burnParam.strUpperPort;
            stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

            if (m_cb != NULL)
            {
                m_cb(handle,stateParam,m_pUsr);
            }
        }

        stateParam.strStateFlag=STATE_BURNING;
        stateParam.strTaskID=burnParam.strTaskID;
        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","checkdev");
        stateParam.strJobID=burnParam.strJobID;
        stateParam.strUpperIP=burnParam.strUpperIP;
        stateParam.strUpperPort=burnParam.strUpperPort;
        stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

        if (m_cb != NULL)
        {
            m_cb(handle,stateParam,m_pUsr);
        }
        //////////////////////////////////////////////////////////////////////////

        nRet=Burn_Ctrl_Dev_Get_HaveDisc(handle);
        nCheckCount++;

        while (true)
        {
            //////////////////////////////////////////////////////////////////////////
            //为了快速响应暂停和停止实时刻录命令
            nTaskControlState=GetTaskControlState(burnParam.strTaskID);

            LOG_INFO(("[BurnTask::IsDevReady] Task control state : %d,%d\r\n",nTaskControlState,__LINE__));

            if (TASK_CONTROL_STATE_STOP == nTaskControlState)
            {
                SetTaskRunningState(burnParam.strTaskID,TASK_STATE_STOPPED);

                stateParam.strStateFlag=STATE_RTTASK_STOPPED;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopped");
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                if (m_cb != NULL)
                {
                    m_cb(handle,stateParam,m_pUsr);
                }

                SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                return false;
            }
            else if (TASK_CONTROL_STATE_PAUSE == nTaskControlState)
            {
                SetTaskRunningState(burnParam.strTaskID,TASK_STATE_PAUSED);

                stateParam.strStateFlag=STATE_RTTASK_PAUSED;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","paused");
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                if (m_cb != NULL)
                {
                    m_cb(handle,stateParam,m_pUsr);
                }

                nTaskControlState=GetTaskControlState(burnParam.strTaskID);
                while (TASK_CONTROL_STATE_PAUSE == nTaskControlState)
                {
                    ZOSThread::Sleep(1000);

                    nTaskControlState=GetTaskControlState(burnParam.strTaskID);
                }

                nCheckCount=0;

                stateParam.strStateFlag=STATE_BURNING;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burning");
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                if (m_cb != NULL)
                {
                    m_cb(handle,stateParam,m_pUsr);
                }
            }

            stateParam.strStateFlag=STATE_BURNING;
            stateParam.strTaskID=burnParam.strTaskID;
            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","checkdev");
            stateParam.strJobID=burnParam.strJobID;
            stateParam.strUpperIP=burnParam.strUpperIP;
            stateParam.strUpperPort=burnParam.strUpperPort;

            if (m_cb != NULL)
            {
                m_cb(handle,stateParam,m_pUsr);
            }
            //////////////////////////////////////////////////////////////////////////

            if (BURN_SUCCESS == nRet)
            {
                LOG_INFO(("[BurnTask::IsDevReady] Detect disc exist,%d\r\n",__LINE__));

                break;
            } 
            else
            {
                LOG_WARNING(("[BurnTask::IsDevReady] Can not detect disc ,try count %d\r\n",
                    nCheckCount));

                Burn_Ctrl_DevTray(handle, B_DEV_TRAY_OPEN);

                if (nCheckCount > 3)
                {
                    LOG_ERROR(("[BurnTask::IsDevReady] Error,no disc,%d\r\n",__LINE__));

                    SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                    stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                    if (mode == MEDIAFILE_BURN_STREAM)
                    {
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","nodiscburnfailedrt");;
                    }
                    else
                    {
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","nodiscburnfailed");;
                    }
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;
                    stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                    if (TASK_PROPERTY_RT_BACKUP == stateParam.strTaskPropertyFlag)
                    {
                        stateParam.strExtraStateFlag=EXTRA_STATE_RTBURN_BACKUP_RESTART_YES;
                    }

                    if (m_cb != NULL)
                    {
                        m_cb(handle,stateParam,m_pUsr);
                    }

                    SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                    return false;
                }

                ZOSThread::Sleep(30*1000);

                nRet=Burn_Ctrl_Dev_Get_HaveDisc(handle);
                ++nCheckCount;
            }
        }// while


#ifdef WIN32
#else
        Burn_Ctrl_Dev_LoadDisc(handle);
#endif

        nRet=Burn_Ctrl_Dev_Get_DiscCanWrite(handle);
        ++nCheckDiscCanWriteCount;
        if (nRet == BURN_SUCCESS)
        {
            break;
        }
        else
        {
            LOG_WARNING(("[BurnTask::IsDevReadyToBurn] Disc can not be written, please insert another disc\r\n"));

            stateParam.strStateFlag=STATE_BURNING;
            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","discreadonly");;
            stateParam.strJobID=burnParam.strJobID;
            stateParam.strTaskID=burnParam.strTaskID;
            stateParam.strUpperIP=burnParam.strUpperIP;
            stateParam.strUpperPort=burnParam.strUpperPort;
            stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;
            if (m_cb != NULL)
            {
                m_cb(handle,stateParam,m_pUsr);
            }

            if (nCheckDiscCanWriteCount > 3)
            {
                SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","discreadonly");
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                if (TASK_PROPERTY_RT_BACKUP == stateParam.strTaskPropertyFlag)
                {
                    stateParam.strExtraStateFlag=EXTRA_STATE_RTBURN_BACKUP_RESTART_YES;
                }

                if (m_cb != NULL)
                {
                    m_cb(handle,stateParam,m_pUsr);
                }

                SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                return false;
            }
            else
            {
                Burn_Ctrl_DevTray(handle,B_DEV_TRAY_OPEN);

                ZOSThread::Sleep(20000);

                continue;
            }
        }

    }//while

    return true;
}

bool BurnTask::BurnDisc(BURN_PARAMETER burnParam)
{
    BURN_STATE_PARAMETER stateParam;

    LOG_INFO(("[BurnTask::BurnDisc] Start to handle task : %s\r\n",burnParam.strTaskID.c_str()));

    size_t nDevCount=burnParam.vecDevHandle.size();
    LOG_INFO(("[BurnTask::BurnDisc] Dev count : %d\r\n",nDevCount));

    int nAvailableDevIndex = 0;
    if (nDevCount <= 0)
    {
        stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","devnumerr");
        stateParam.strJobID=burnParam.strJobID;
        stateParam.strTaskID=burnParam.strTaskID;
        stateParam.strUpperIP=burnParam.strUpperIP;
        stateParam.strUpperPort=burnParam.strUpperPort;
        if (m_cb != NULL)
        {
            m_cb(NULL,stateParam,m_pUsr);
        }

        if (burnParam.nTaskExtraFlag == TASK_EXTRA_STATE_ASYN_SINGLE)
        {//如果被触发的任务失败，重置触发的任务状态
            if (burnParam.strLastTaskID != "")
            {
                SetTaskExtraFlag(burnParam.strLastTaskID, TASK_EXTRA_STATE_ASYN_SINGLE);
            }
        }
        return false;
    }
    else if (nDevCount > 1)
    {
        if (MEDIAFILE_BURN_STREAM == burnParam.burnMode)
        {
            nAvailableDevIndex = GetFirstUnusedDev(burnParam.vecDevHandle);
            if (nAvailableDevIndex == -1)
            {//如果找不到可用的光驱，则置该任务为可用，同时重置触发的任务状态
                LOG_WARNING(("[BurnTask::BurnDisc] No available device! %d\r\n",__LINE__));
                SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","devunavailable");
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                if (m_cb != NULL)
                {
                    m_cb(NULL,stateParam,m_pUsr);
                }

                if (burnParam.nTaskExtraFlag == TASK_EXTRA_STATE_ASYN_SINGLE)
                {//如果被触发的任务失败，重置触发的任务状态
                    if (burnParam.strLastTaskID != "")
                    {
                        SetTaskExtraFlag(burnParam.strLastTaskID, TASK_EXTRA_STATE_ASYN_SINGLE);
                    }
                }
                return false;
            }
        }
    }

    if (burnParam.strLastTaskID != "")
    {//与触发任务的控制命令保持统一
        int nLastTaskControlStatus = GetTaskControlState(burnParam.strLastTaskID);
        burnParam.nTaskControlCmd = nLastTaskControlStatus;
    }


#ifdef RT_BURN_DISK_BACKUP
    //每次开始实时刻录时，在检查光驱之前，默认开启硬盘备份
    if (MEDIAFILE_BURN_STREAM == burnParam.burnMode)
    {
        DISK_BACKUP_PARAM diskBackupParam;//备份参数可以从配置文件读取，或者由刻录协议传送

        diskBackupParam.useDev=BURN_TRUE;
        diskBackupParam.backupSize=1024*1024;
        diskBackupParam.alarmSize=100;
        diskBackupParam.strDiskPath=Path(gBasicParamCfgFile.GetValue("info","burnServerRTBackupDir"), true)+burnParam.strJobID;

        if (!IsFileOrDirExist(diskBackupParam.strDiskPath,true))
        {
            DirectoryUtil::CreateDir(diskBackupParam.strDiskPath.c_str());
        }

        diskBackupParam.handle=burnParam.vecDevHandle.at(nAvailableDevIndex);
        diskBackupParam.strBackupFileName=UUID_GetUUID()+".ts";

        if (StartDiskBackup(diskBackupParam))
        {
            LOG_INFO(("[BurnTask::BurnDisc] StartDiskBackup success,handle : %p\r\n",diskBackupParam.handle));

            //添加游离的实时刻录补刻Task
            BURN_PARAMETER rtBurnBackupTaskParam;
            rtBurnBackupTaskParam.burnMode=MEDIAFILE_BURN_LOCAL_FILE;

            rtBurnBackupTaskParam.mapBurnDataSource.clear();
            std::map< std::string,std::vector<BURN_DATA_SOURCE> >::iterator iterBurnDataSource;
            for (iterBurnDataSource=burnParam.mapBurnDataSource.begin();
                iterBurnDataSource!=burnParam.mapBurnDataSource.end();++iterBurnDataSource)
            {
                if (!iterBurnDataSource->second.empty())
                {
                    if (SOURCE_TYPE_URL == iterBurnDataSource->second.at(0).strType)
                    {
                        std::string strLocationID="88888888";

                        BURN_DATA_SOURCE source;
                        source.strType=SOURCE_TYPE_NORMAL_FILE;

                        std::string strBackupDir=diskBackupParam.strDiskPath;
                        if (strBackupDir != "" &&
                            strBackupDir.at(strBackupDir.length()-1) != '/')
                        {
                            strBackupDir+="/";
                        }

                        source.strSourceUrl=strBackupDir+diskBackupParam.strBackupFileName;

                        rtBurnBackupTaskParam.mapBurnDataSource[strLocationID].push_back(source);

                        LOG_INFO(("[BurnTask::BurnDisc] Location id : %s,%d\r\n",
                            strLocationID.c_str(),__LINE__));
                        LOG_INFO(("[BurnTask::BurnDisc] Location file : %s,%d\r\n",
                            source.strSourceUrl.c_str(),__LINE__));
                    }
                    else
                    {
                        //除了刻录备份的.ts文件以外，播放器、playlist等也需要被刻录
                        rtBurnBackupTaskParam.mapBurnDataSource[iterBurnDataSource->first]=iterBurnDataSource->second;
                    }
                }
            }

            rtBurnBackupTaskParam.nIndex=0;
            rtBurnBackupTaskParam.nTaskControlCmd=TASK_CONTROL_STATE_DEFAULT;
            rtBurnBackupTaskParam.nTaskRunningFlag=TASK_STATE_BURN_BACKUP_DRIFT;
            rtBurnBackupTaskParam.strDiscLabel=burnParam.strDiscLabel;
            rtBurnBackupTaskParam.strJobID=burnParam.strJobID;
            rtBurnBackupTaskParam.strRtNoteUpdateFlag=NOTE_FILE_UPDATED_NO;
            rtBurnBackupTaskParam.strTaskID=burnParam.strTaskID+UUID_GetUUID();//带有父级task的标识
            rtBurnBackupTaskParam.vecDevHandle.clear();
            rtBurnBackupTaskParam.vecDevHandle.push_back(diskBackupParam.handle);

            rtBurnBackupTaskParam.strTaskPropertyFlag=TASK_PROPERTY_RT_BACKUP;
            rtBurnBackupTaskParam.strUpperIP=burnParam.strUpperIP;
            rtBurnBackupTaskParam.strUpperPort=burnParam.strUpperPort;

            LOG_INFO(("[BurnTask::BurnDisc] Add rt burn backup task : \r\n"));
            LOG_INFO(("Task id : %s\r\n",rtBurnBackupTaskParam.strTaskID.c_str()));
            LOG_INFO(("Job id : %s\r\n",rtBurnBackupTaskParam.strJobID.c_str()));
            LOG_INFO(("Upper ip : %s\r\n",rtBurnBackupTaskParam.strUpperIP.c_str()));
            LOG_INFO(("Upper port : %s\r\n",rtBurnBackupTaskParam.strUpperPort.c_str()));


            AddTask(rtBurnBackupTaskParam);
        }
        else
        {
            LOG_ERROR(("[BurnTask::BurnDisc] StartDiskBackup failed,handle : %p\r\n",diskBackupParam.handle));
        }
    }
#endif

#ifdef RT_BURN_DISK_BACKUP
#if 0
    //以下代码的作用仅仅是测试实时刻录补刻
    if (MEDIAFILE_BURN_STREAM == burnParam.burnMode)
    {
        stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","closediscfailed");
        stateParam.strJobID=burnParam.strJobID;
        stateParam.strTaskID=burnParam.strTaskID;

        stateParam.strUpperIP=burnParam.strUpperIP;
        stateParam.strUpperPort=burnParam.strUpperPort;
        stateParam.strExtraStateFlag=EXTRA_STATE_RTBURN_BACKUP_YES;

        SaveRTBurnBackupTask(burnParam);

        if (m_cb != NULL)
        {
            m_cb(burnParam.vecDevHandle.at(0),stateParam,m_pUsr);
        }

        SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

        SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_BURN_BACKUP_SLEEP);

        return false;
    }
#endif
#endif

    BURN_MODE mode=burnParam.burnMode;
    switch (mode)
    {
    case MEDIAFILE_BURN_LOCAL_FILE:
        {//事后刻录

            LOG_INFO(("[BurnTask::BurnDisc] Defore IsDevReady,%d\r\n",__LINE__));

            if (!IsDevReady(burnParam.vecDevHandle.at(0),burnParam,mode))
            {
                //清理废弃的task的操作全由HandleTask执行
                LOG_ERROR(("[BurnTask::BurnDisc] IsDevReady return false,%d\r\n",__LINE__));

                return false;
            }

            LOG_INFO(("[BurnTask::BurnDisc] After IsDevReady,%d\r\n",__LINE__));

            //进入检测光盘信息状态
            stateParam.strStateFlag=STATE_BURNING;
            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","checkdisc");;
            stateParam.strJobID=burnParam.strJobID;
            stateParam.strTaskID=burnParam.strTaskID;
            stateParam.strUpperIP=burnParam.strUpperIP;
            stateParam.strUpperPort=burnParam.strUpperPort;

            if (m_cb != NULL)
            {
                m_cb(burnParam.vecDevHandle.at(0),stateParam,m_pUsr);
            }

            BURN_DISC_INFO_T discInfo;
            if (GetDiscInformation(burnParam.vecDevHandle.at(0),discInfo))
            {
                LOG_INFO(("[BurnTask::BurnDisc] Get disc information success\r\n"));
                LOG_INFO(("-------------------- DISC INFO ----------------------------\r\n"));
                LOG_INFO(("Disc space : %d\r\n",discInfo.discsize));
                LOG_INFO(("Disc used : %d\r\n",discInfo.usedsize));
                LOG_INFO(("Disc free : %d\r\n",discInfo.freesize));
                LOG_INFO(("-----------------------------------------------------------\r\n"));
            }
            else
            {
                LOG_ERROR(("[BurnTask::BurnDisc] Get disc information failed\r\n"));

                SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","checkdiscfailed");;
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                if (m_cb != NULL)
                {
                    m_cb(burnParam.vecDevHandle.at(0),stateParam,m_pUsr);
                }

                SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                return false;
            }

            stateParam.strStateFlag=STATE_BURNING;
            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","preparedata");
            stateParam.strJobID=burnParam.strJobID;
            stateParam.strTaskID=burnParam.strTaskID;
            stateParam.strUpperIP=burnParam.strUpperIP;
            stateParam.strUpperPort=burnParam.strUpperPort;

            if (m_cb != NULL)
            {
                m_cb(burnParam.vecDevHandle.at(0),stateParam,m_pUsr);
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

            //分配刻录数据
            while (true)
            {
                bAddOver=true;

                ++locationIndex;

                vecSinglePollDataSource.clear();

                std::map< std::string,std::vector<BURN_DATA_SOURCE> >::iterator iter;
                for (iter=burnParam.mapBurnDataSource.begin();iter!=burnParam.mapBurnDataSource.end();++iter)
                {
                    if ("note" == iter->first)
                    {
                        if (0 == nMemLastPartSize)
                        {
                            assert(1 == iter->second.size());

                            nTemp=GetFileOrDirSize(iter->second.at(0).strSourceUrl,false);

                            //nSingleTaskSize+=nTemp;

                            vecSinglePollDataSource.push_back(iter->second.at(0));

                            LOG_INFO(("[BurnTask::BurnDisc] Prepare note : %s,size : %u\r\n",
                                iter->second.at(0).strSourceUrl.c_str(),nTemp));
                        }
                    }
                    else if ("player" == iter->first)
                    {
                        if (0 == nMemLastPartSize)
                        {
                            assert(1 == iter->second.size());

                            nTemp=GetFileOrDirSize(iter->second.at(0).strSourceUrl,true);

                            //nSingleTaskSize+=nTemp;

                            vecSinglePollDataSource.push_back(iter->second.at(0));

                            LOG_INFO(("[BurnTask::BurnDisc] Prepare player : %s,size : %u\r\n",
                                iter->second.at(0).strSourceUrl.c_str(),nTemp));
                        }
                    } 
                    else if("autorun" == iter->first)
                    {
                        if (0 == nMemLastPartSize)
                        {
                            assert(1 == iter->second.size());

                            nTemp=GetFileOrDirSize(iter->second.at(0).strSourceUrl,false);

                            //nSingleTaskSize+=nTemp;

                            vecSinglePollDataSource.push_back(iter->second.at(0));

                            LOG_INFO(("[BurnTask::BurnDisc] Prepare autorun : %s,size : %u\r\n",
                                iter->second.at(0).strSourceUrl.c_str(),nTemp));
                        }
                    }
                    else if ("auth" == iter->first)
                    {
                        if (0 == nMemLastPartSize)
                        {
                            assert(1 == iter->second.size());

                            nTemp=GetFileOrDirSize(iter->second.at(0).strSourceUrl,false);

                            //nSingleTaskSize+=nTemp;

                            vecSinglePollDataSource.push_back(iter->second.at(0));

                            LOG_INFO(("[BurnTask::BurnDisc] Prepare auth : %s,size : %u\r\n",
                                iter->second.at(0).strSourceUrl.c_str(),nTemp));
                        }
                    }
                    else if ("playlist" == iter->first)
                    {
                        if (0 == nMemLastPartSize)
                        {
                            assert(1 == iter->second.size());

                            strPlayListFullPath=iter->second.at(0).strSourceUrl;
                        }
                    }
                    else
                    {
                        LOG_INFO(("[BurnTask::BurnDisc] Location file count : %d\r\n",iter->second.size()));

                        //每个画面的同一位置的文件
                        if (static_cast<int>(iter->second.size()-1) > locationIndex)
                        {
                            bAddOver=false;
                        }

                        if (static_cast<int>(iter->second.size()) > locationIndex)
                        {
                            std::string strT=iter->second.at(locationIndex).strSourceUrl.c_str();
                            INT64 nSizeT=GetFileOrDirSize(strT,false);

                            nSingleTaskSize+=nSizeT;

                            vecSinglePollDataSource.push_back(iter->second.at(locationIndex));


                            LOG_INFO(("[BurnTask::BurnDisc] Prepare file(dir) : %s,%lld\r\n",
                                iter->second.at(locationIndex).strSourceUrl.c_str(),nSizeT));
                        }
                    }
                }// For single poll

                INT64 nDiscFreeSize = GetLimitSize();

                if (m_nLimitSize == -1)
                {
                    nDiscFreeSize=static_cast<INT64>(discInfo.freesize);
                }
                LOG_INFO(("[BurnTask::BurnDisc] Burn every disc free size: %d\r\n", nDiscFreeSize));

                if (nSingleTaskSize/1024/1024 < nDiscFreeSize)
                {
                    if (bAddOver)
                    {
                        while(!vecSinglePollDataSource.empty())
                        {
                            vecSingleTaskDataSource.push_back(vecSinglePollDataSource.back());

                            vecSinglePollDataSource.pop_back();
                        }

                        if (strPlayListFullPath != "" &&
                            BurnTask::InsertPlayListFile(vecSingleTaskDataSource,strPlayListFullPath))
                        {
                            vecSingleDiscDataSource.push_back(vecSingleTaskDataSource);

                            vecSingleTaskDataSource.clear();
                            nSingleTaskSize=0;
                            nMemLastPartSize=0;
                            bAddOver=true;

                            LOG_INFO(("[BurnTask::BurnDisc] ~~~~ Add over ~~~~\r\n"));
                        }
                        else
                        {
                            vecSingleDiscDataSource.push_back(vecSingleTaskDataSource);

                            LOG_ERROR(("[BurnTask::BurnDisc] CreatePlayListFile failed,%d\r\n",__LINE__));

                            assert(false);
                        }
                    }
                    else
                    {
                        // Try to add next location
                        LOG_INFO(("[BurnTask::BurnDisc] ~~~~ Next poll ~~~~,%d\r\n",__LINE__));

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
                        LOG_ERROR(("[BurnTask::BurnDisc] No space,free : %lu M,data size : %lld M\r\n",
                            discInfo.freesize,nSingleTaskSize/1024/1024));

                        stateParam.strStateFlag=STATE_BURN_ERR;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","tomuchdata");
                        stateParam.strJobID=burnParam.strJobID;
                        stateParam.strTaskID=burnParam.strTaskID;
                        stateParam.strUpperIP=burnParam.strUpperIP;
                        stateParam.strUpperPort=burnParam.strUpperPort;

                        if (m_cb != NULL)
                        {
                            m_cb(burnParam.vecDevHandle.at(0),stateParam,m_pUsr);
                        }

                        SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                        return false;
                    }
                    else
                    {
                        if (strPlayListFullPath != "" &&
                            BurnTask::InsertPlayListFile(vecSingleTaskDataSource,strPlayListFullPath))
                        {
                            vecSingleDiscDataSource.push_back(vecSingleTaskDataSource);

                            vecSingleTaskDataSource.clear();
                            nSingleTaskSize=0;
                            nMemLastPartSize=0;
                            bAddOver=false;

                            --locationIndex;

                            LOG_INFO(("[BurnTask::BurnDisc] ^^^^ Next poll ^^^^\r\n"));

                            continue;
                        }
                        else
                        {

                            LOG_ERROR(("[BurnTask::BurnDisc] CreatePlayListFile failed,%d\r\n",__LINE__));

                            assert(false);
                        }

                    }
                }

                if (bAddOver)
                {
                    break;
                }


            }//while(true)

            size_t nDiscCount=vecSingleDiscDataSource.size();

            //开始刻录
            //////////////////////////////////////////////////////////////////////////
            LOG_INFO(("===========================================================\r\n"));
            for (size_t i=0;i<nDiscCount;++i)
            {
                LOG_INFO(("Disc %d : \r\n",i+1));

                const std::vector<BURN_DATA_SOURCE> &vecDataSource=vecSingleDiscDataSource.at(i);
                size_t dataCount=vecDataSource.size();
                for (size_t j=0;j<dataCount;++j)
                {
                    LOG_INFO(("file %d : %s\r\n",j+1,vecDataSource.at(j).strSourceUrl.c_str()));
                }

                LOG_INFO(("\r\n"));
            }
            LOG_INFO(("===========================================================\r\n"));


            if (nDevCount > 1)
            {
                DEV_BURN_FILES devBurnFiles;
                devBurnFiles.strJobID=burnParam.strJobID;
                devBurnFiles.strUpperIP=burnParam.strUpperIP;
                devBurnFiles.strUpperPort=burnParam.strUpperPort;

                for (size_t i=0;i<nDiscCount;++i)
                {
                    DEV_HANDLE h=burnParam.vecDevHandle.at(i%nDevCount);

                    const std::vector<BURN_DATA_SOURCE> &vecDataSource=vecSingleDiscDataSource.at(i);
                    size_t dataCount=vecDataSource.size();
                    std::string strSourceType;
                    std::string strSourceUrl;
                    for (size_t j=0;j<dataCount;++j)
                    {
                        strSourceType=vecDataSource.at(j).strType;
                        strSourceUrl=vecDataSource.at(j).strSourceUrl;

                        if (strSourceType != "player" &&
                            strSourceType != "playlist" &&
                            strSourceType != "auth" &&
                            strSourceType != "autorun" &&
                            strSourceType != "note")
                        {
                            devBurnFiles.mapDevBurnFiles[h].push_back(vecDataSource.at(j));
                        }
                    }
                }

                if (m_cbDevBurnFiles != NULL)
                {
                    m_cbDevBurnFiles(devBurnFiles,m_pUsrDevBurnFiles);
                }
            }

#if WIN32
#else
            //为了保证光盘名字不出现中文
            burnParam.strDiscLabel = "csBurn";
#endif
            std::string strDiscLabel=burnParam.strDiscLabel;
            LOG_INFO(("[BurnTask::BurnDisc] Disc label : %s,%d\r\n",strDiscLabel.c_str(),__LINE__));

            int nDiscIndex=0;//当前task正在使用第几张盘

            if (1 == nDevCount)//单盘续刻
            {
                DEV_HANDLE handle=burnParam.vecDevHandle.at(0);

                nDiscIndex=0;

                std::vector< std::vector<BURN_DATA_SOURCE> >::iterator iterSingleDiscDataSource;
                for (iterSingleDiscDataSource=vecSingleDiscDataSource.begin();
                    iterSingleDiscDataSource!=vecSingleDiscDataSource.end();
                    ++iterSingleDiscDataSource,++nDiscIndex)
                {
                    if (iterSingleDiscDataSource != vecSingleDiscDataSource.begin())
                    {
                        stateParam.strStateFlag=STATE_BURNING;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","changedisc");;
                        stateParam.strJobID=burnParam.strJobID;
                        stateParam.strTaskID=burnParam.strTaskID;
                        stateParam.strUpperIP=burnParam.strUpperIP;
                        stateParam.strUpperPort=burnParam.strUpperPort;

                        if (m_cb != NULL)
                        {
                            m_cb(handle,stateParam,m_pUsr);
                        }

                        if (!IsDevReady(handle,burnParam,mode))
                        {
                            return false;
                        }
                    }

                    LOG_INFO(("[BurnTask::BurnDisc] Start single disc burn ,handle %p\r\n",
                        (void *)(burnParam.vecDevHandle.at(0))));

                    stateParam.strStateFlag=STATE_BURNING;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burning");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;

                    if (m_cb != NULL)
                    {
                        m_cb(handle,stateParam,m_pUsr);
                    }


                    burnParam.strDiscLabel=strDiscLabel+"_"+BurnTask::IntToString(nDiscIndex);

#if 1
                    if (!StartSpecificDeviceToBurn(handle,*iterSingleDiscDataSource,mode,burnParam))
                    {
                        SetDevUsed(handle,DEV_FLAG_USED_NO);

                        stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burnfailed");;
                        stateParam.strJobID=burnParam.strJobID;
                        stateParam.strTaskID=burnParam.strTaskID;
                        stateParam.strUpperIP=burnParam.strUpperIP;
                        stateParam.strUpperPort=burnParam.strUpperPort;

                        if (m_cb != NULL)
                        {
                            m_cb(handle,stateParam,m_pUsr);
                        }

                        SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                        return false;
                    }
                    else
                    {
                        SetDevUsed(handle,DEV_FLAG_USED_NO);
                    }
#endif


#if 0
                    // 模拟单盘刻录
                    ZOSThread::Sleep(6000);
#endif

                    Burn_Ctrl_DevTray(handle,B_DEV_TRAY_OPEN);

                    stateParam.strStateFlag=STATE_SINGLE_DISC_BURNED;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burnsuccess");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;

                    if (m_cb != NULL)
                    {
                        m_cb(handle,stateParam,m_pUsr);
                    }

                    LOG_INFO(("[BurnTask::BurnDisc] Single disc burn over , nDiscIndex: %d!!!!\r\n", nDiscIndex));

                }

                LOG_INFO(("[BurnTask::BurnDisc] Single task burn over !!!!\r\n"));
            }
            else//多盘续刻
            {
                LOG_INFO(("[BurnTask::BurnDisc] N syn,%d\r\n",__LINE__));

                DEV_HANDLE handle;
                size_t nDevIndex=burnParam.nIndex;
                nDiscIndex=0;

                std::vector< std::vector<BURN_DATA_SOURCE> >::iterator iterSingleDiscDataSource;
                for (iterSingleDiscDataSource=vecSingleDiscDataSource.begin();
                    iterSingleDiscDataSource!=vecSingleDiscDataSource.end();
                    ++iterSingleDiscDataSource,++nDiscIndex)
                {
                    //判断是否插入光盘后继续刻录

                    handle=burnParam.vecDevHandle.at(nDevIndex);

                    SetCurDev(burnParam.strTaskID,nDevIndex);

                    LOG_INFO(("[BurnTask::HandleTask] Dev index : %d\r\n",nDevIndex));

                    if (iterSingleDiscDataSource != vecSingleDiscDataSource.begin())
                    {
                        stateParam.strStateFlag=STATE_BURNING;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","nextdisc");;
                        stateParam.strJobID=burnParam.strJobID;
                        stateParam.strTaskID=burnParam.strTaskID;
                        stateParam.strUpperIP=burnParam.strUpperIP;
                        stateParam.strUpperPort=burnParam.strUpperPort;

                        if (m_cb != NULL)
                        {
                            m_cb(handle,stateParam,m_pUsr);
                        }

                        if (!IsDevReady(handle,burnParam,mode))
                        {
                            return false;
                        }
                    }

                    LOG_INFO(("[BurnTask::BurnDisc] Start single disc burn ,handle %p\r\n",
                        (void *)(burnParam.vecDevHandle.at(nDevIndex))));

                    stateParam.strStateFlag=STATE_BURNING;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burning");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;

                    if (m_cb != NULL)
                    {
                        m_cb(handle,stateParam,m_pUsr);
                    }

#if 1
                    burnParam.strDiscLabel=strDiscLabel+"_"+BurnTask::IntToString(nDiscIndex);

                    if (StartSpecificDeviceToBurn(handle,*iterSingleDiscDataSource,mode,burnParam))
                    {
                        SetDevUsed(handle,DEV_FLAG_USED_NO);

                        //弹出光盘，使用下一个光驱，等待光驱识别光盘
                        Burn_Ctrl_DevTray(handle,B_DEV_TRAY_OPEN);

                        stateParam.strStateFlag=STATE_SINGLE_DISC_BURNED;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burnsuccess");
                        stateParam.strJobID=burnParam.strJobID;
                        stateParam.strTaskID=burnParam.strTaskID;
                        stateParam.strUpperIP=burnParam.strUpperIP;
                        stateParam.strUpperPort=burnParam.strUpperPort;

                        if (m_cb != NULL)
                        {
                            m_cb(handle,stateParam,m_pUsr);
                        }

                        nDevIndex++;
                        nDevIndex%=nDevCount;
                    }
                    else
                    {
                        LOG_INFO(("[BurnTask::BurnDisc] Single disc burn failed,%d\r\n",__LINE__));

                        SetDevUsed(handle,DEV_FLAG_USED_NO);

                        stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burnfailed");
                        stateParam.strJobID=burnParam.strJobID;
                        stateParam.strTaskID=burnParam.strTaskID;
                        stateParam.strUpperIP=burnParam.strUpperIP;
                        stateParam.strUpperPort=burnParam.strUpperPort;

                        if (m_cb != NULL)
                        {
                            m_cb(handle,stateParam,m_pUsr);
                        }

                        SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                        return false;
                    }
#endif


#if 0
                    // 用于测试多盘续刻的逻辑
                    ZOSThread::Sleep(120000);

                    Burn_Ctrl_DevTray(handle,B_DEV_TRAY_OPEN);

                    stateParam.strStateFlag=STATE_BURNING;
                    stateParam.strStateDescription=gCfgFile.GetValue("info","burnsuccess");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;

                    if (m_cb != NULL)
                    {
                        m_cb(handle,stateParam,m_pUsr);
                    }

                    devIndex++;
                    devIndex%=nDevCount;
#endif

                }// For every single disc data

                LOG_INFO(("[BurnTask::HandleTask] Burn over !!!!\r\n"));

            }//N盘续刻
        }
        break;

    case MEDIAFILE_BURN_STREAM:
        {
            //一个实时刻录task恢复时，需要置笔录状态为未更新
            SetNoteUpdateFlagByTaskID(burnParam.strTaskID,
                NOTE_FILE_UPDATED_NO);

            // 实时刻录 MEDIAFILE_BURN_STREAM 222324

            LOG_INFO(("[BurnTask::BurnDisc] MEDIAFILE_BURN_STREAM,%d\r\n",__LINE__));

            if (1 == nDevCount)
            {
                //单盘刻录（多盘同刻本质上属于这种类型）

                LOG_INFO(("[BurnTask::BurnDisc] Defore IsDevReady,%d\r\n",__LINE__));

                if (!IsDevReady(burnParam.vecDevHandle.at(0),burnParam,mode))
                {
                    //清理废弃的task的操作全由HandleTask执行
                    LOG_ERROR(("[BurnTask::BurnDisc] IsDevReady return false,%d\r\n",__LINE__));

                    return false;
                }

                LOG_INFO(("[BurnTask::BurnDisc] After IsDevReady,%d\r\n",__LINE__));

                //进入检测光盘信息状态
                stateParam.strStateFlag=STATE_BURNING;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","checkdisc");;
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;

                if (m_cb != NULL)
                {
                    m_cb(burnParam.vecDevHandle.at(0),stateParam,m_pUsr);
                }

                BURN_DISC_INFO_T discInfo;
                if (GetDiscInformation(burnParam.vecDevHandle.at(0),discInfo))
                {
                    LOG_INFO(("[BurnTask::BurnDisc] Get disc information success\r\n"));
                    LOG_INFO(("-------------------- DISC INFO ----------------------------\r\n"));
                    LOG_INFO(("Disc space : %d\r\n",discInfo.discsize));
                    LOG_INFO(("Disc used : %d\r\n",discInfo.usedsize));
                    LOG_INFO(("Disc free : %d\r\n",discInfo.freesize));
                    LOG_INFO(("-----------------------------------------------------------\r\n"));
                }
                else
                {
                    LOG_ERROR(("[BurnTask::BurnDisc] Get disc information failed\r\n"));

                    SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                    stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","checkdiscfailed");;
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;
                    stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                    if (m_cb != NULL)
                    {
                        m_cb(burnParam.vecDevHandle.at(0),stateParam,m_pUsr);
                    }

                    SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                    return false;
                }

                stateParam.strStateFlag=STATE_BURNING;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","preparedata");
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;

                if (m_cb != NULL)
                {
                    m_cb(burnParam.vecDevHandle.at(0),stateParam,m_pUsr);
                }

                assert(1 == burnParam.vecDevHandle.size());

                std::vector<BURN_DATA_SOURCE> vecDataSource;
                std::map< std::string,std::vector<BURN_DATA_SOURCE> >::iterator iter;
                for (iter=burnParam.mapBurnDataSource.begin();iter!=burnParam.mapBurnDataSource.end();
                    ++iter)
                {
                    const std::vector<BURN_DATA_SOURCE> &T=iter->second;
                    std::vector<BURN_DATA_SOURCE>::const_iterator iterT;
                    for (iterT=T.begin();iterT!=T.end();++iterT)
                    {
                        if (SOURCE_TYPE_URL == iterT->strType &&
                            iterT->strSourceUrl != "")
                        {
                            vecDataSource.push_back(*iterT);
                        }
                    }                    
                }

                //////////////////////////////////////////////////////////////////////////
                //检查实时刻录视频源的连通性
                //////////////////////////////////////////////////////////////////////////
                std::vector<BURN_DATA_SOURCE> vecStreamSource;
                std::vector<BURN_DATA_SOURCE>::iterator iterT;
                for (iterT=vecDataSource.begin();iterT!=vecDataSource.end();++iterT)
                {
                    if (SOURCE_TYPE_URL == iterT->strType)
                    {
                        vecStreamSource.push_back(*iterT);
                    }
                }

                DEV_HANDLE handle=burnParam.vecDevHandle.at(0);

                size_t nCheckConnectableCount=0;
                while (true)
                {
                    if (CheckStreamConnectable(vecStreamSource))
                    {
                        LOG_INFO(("[BurnTask::BurnDisc] Stream source ok,%d\r\n",__LINE__));

                        break;
                    } 
                    else
                    {
                        ++nCheckConnectableCount;

                        LOG_INFO(("[BurnTask::BurnDisc] %d,%d\r\n",nCheckConnectableCount,__LINE__));

                        if (nCheckConnectableCount > 4)
                        {
                            break;
                        }
                        else
                        {
                            ZOSThread::Sleep(5000);
                        }
                    }
                }

                if (nCheckConnectableCount > 4)
                {
                    SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                    stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","streamunconnectable");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;

                    if (m_cb != NULL)
                    {
                        m_cb(handle,stateParam,m_pUsr);
                    }

                    SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                    return false;
                }

                LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] %d,%d\r\n",nCheckConnectableCount,__LINE__));

                //////////////////////////////////////////////////////////////////////////
                printf("===========================================================\r\n");
                printf("RT source : \r\n");
                std::vector<BURN_DATA_SOURCE>::iterator iterDataSource;
                int nSourceIndex=0;
                for (iterDataSource=vecDataSource.begin();iterDataSource!=vecDataSource.end();++iterDataSource)
                {
                    printf("Source %d : %s\r\n",nSourceIndex,iterDataSource->strSourceUrl.c_str());

                    ++nSourceIndex;
                }
                printf("===========================================================\r\n");
                //////////////////////////////////////////////////////////////////////////


#if 1
                stateParam.strStateFlag=STATE_BURNING;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burning");
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                if (m_cb != NULL)
                {
                    m_cb(handle,stateParam,m_pUsr);
                }

                //这个很重要，停止实时流刻录对于没有开启流刻录的设备会出错
                SetCurDev(burnParam.strTaskID,0);

#if 0
                //用于测试实时刻录控制逻辑
                if (true)
                {
                    ZOSThread::Sleep(22000);

                    SetDevUsed(handle,DEV_FLAG_USED_NO);

                    //实时刻录出现没有刻录任何数据但是每个刻录的接口都返回成功的情况，在此添加验证的代码
                    //TODO

                    Burn_Ctrl_DevTray(handle,B_DEV_TRAY_OPEN);

                    stateParam.strStateFlag=STATE_SINGLE_DISC_BURNED;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burnsuccess");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;
                    stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                    if (m_cb != NULL)
                    {
                        m_cb(handle,stateParam,m_pUsr);
                    }

                    //暂停或者停止实时刻录后，必须删除刻录备份task
                    SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                }
#endif

#if 1
                if (StartSpecificDeviceToBurn(handle,vecDataSource,MEDIAFILE_BURN_STREAM,burnParam))
                {
                    //停止实时刻录，向光盘添加附加数据，封盘，设置任务为空闲状态

                    UINT64 nCurTime=ZOS::milliseconds();
                    while (true)
                    {
                        //检查笔录文件是否已经更新成功
                        if (NOTE_FILE_UPDATED_YES == GetNoteUpdateFlag(GetBurningTaskID(handle)))
                        {
                            LOG_INFO(("[BurnTask::BurnDisc] Update note file success,%d ........................\r\n",
                                __LINE__));

                            break;
                        }

                        ZOSThread::Sleep(1000);

                        //给更新笔录留4分钟时间
                        if (ZOS::milliseconds() - nCurTime > 4*60*1000)
                        {
                            LOG_WARNING(("[BurnTask::BurnDisc] Update note file failed,%d ........................\r\n",
                                __LINE__));
                            break;
                        }
                    }

                    LOG_INFO(("[BurnTask::BurnDisc] Start burn extra data...,%d\r\n",__LINE__));

                    stateParam.strStateFlag=STATE_RTTASK_STOPPING;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopping");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;
                    stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                    if (m_cb != NULL)
                    {
                        m_cb(handle,stateParam,m_pUsr);
                    }

                    RTBurnExtraData(handle,burnParam.mapBurnDataSource);

                    stateParam.strStateFlag=STATE_RTTASK_STOPPING;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","closedisc");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;
                    stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                    if (m_cb != NULL)
                    {
                        m_cb(handle,stateParam,m_pUsr);
                    }

                    while(false)
                    {
                        if (!IsTaskCanCloseDisc(burnParam.strJobID, burnParam.strTaskID))
                        {
                            Sleep(10*1000);
                        }
                        else
                        {
                            break;
                        }
                    }

                    if (BURN_SUCCESS == Burn_Ctrl_CloseDisc(handle))
                    {
                        SetDevUsed(handle,DEV_FLAG_USED_NO);

                        //实时刻录出现没有刻录任何数据但是每个刻录的接口都返回成功的情况，在此添加验证的代码
                        //TODO

                        Burn_Ctrl_DevTray(handle,B_DEV_TRAY_OPEN);

                        stateParam.strStateFlag=STATE_SINGLE_DISC_BURNED;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burnsuccess");
                        stateParam.strJobID=burnParam.strJobID;
                        stateParam.strTaskID=burnParam.strTaskID;
                        stateParam.strUpperIP=burnParam.strUpperIP;
                        stateParam.strUpperPort=burnParam.strUpperPort;
                        stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                        if (m_cb != NULL)
                        {
                            m_cb(handle,stateParam,m_pUsr);
                        }

                        //暂停或者停止实时刻录后，必须删除刻录备份task
                        SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                    }
                    else
                    {
                        SetDevUsed(handle,DEV_FLAG_USED_NO);

                        stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","closediscfailed");
                        stateParam.strJobID=burnParam.strJobID;
                        stateParam.strTaskID=burnParam.strTaskID;
                        stateParam.strUpperIP=burnParam.strUpperIP;
                        stateParam.strUpperPort=burnParam.strUpperPort;
                        stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                        //////////////////////////////////////////////////////////////////////////
                        stateParam.strExtraStateFlag=EXTRA_STATE_RTBURN_BACKUP_YES;//此时应该通知BurnControl进行补刻
                        SaveRTBurnBackupTask(burnParam);
                        //////////////////////////////////////////////////////////////////////////

                        if (m_cb != NULL)
                        {
                            m_cb(handle,stateParam,m_pUsr);
                        }

                        SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

#ifdef RT_BURN_DISK_BACKUP
                        //封盘失败后，如果用户选择补刻，那么刻录备份task需要被直接启动（跳过激活）
                        //目前的做法是在激活的操作中直接将task的状态等级提升一级
                        SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_BURN_BACKUP_SLEEP);
#endif

                        return false;
                    }

                    LOG_INFO(("[BurnTask::BurnDisc] Single disc RT burn over,%d !!!\r\n",__LINE__));
                }
#endif
                else
                {
                    SetDevUsed(handle,DEV_FLAG_USED_NO);

                    SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                    SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                    return false;
                }
#endif

                int nTaskControlState=GetTaskControlState(burnParam.strTaskID);
                if (nTaskControlState != TASK_CONTROL_STATE_PAUSE &&
                    nTaskControlState != TASK_CONTROL_STATE_STOP)
                {
                    LOG_INFO(("[BurnTask::BurnDisc] Will reset task : %s,%d\r\n",
                        burnParam.strTaskID.c_str(),__LINE__));

                    ResetTask(burnParam.strTaskID);

                    return true;//实时刻录如果不暂停、不停止、不异常，那么task就不被当作完成
                }
                else
                {
                    LOG_INFO(("[BurnTask::BurnDisc] Task success,control state : %d,%d\r\n",
                        nTaskControlState,__LINE__));
                }
            } 
            else if(nDevCount > 1)
            {
                //多盘续刻

                assert(burnParam.vecDevHandle.size() > 0);

                std::vector<BURN_DATA_SOURCE> vecDataSource;
                std::map< std::string,std::vector<BURN_DATA_SOURCE> >::iterator iter;
                for (iter=burnParam.mapBurnDataSource.begin();iter!=burnParam.mapBurnDataSource.end();
                    ++iter)
                {
                    const std::vector<BURN_DATA_SOURCE> &vecBurnDataSource=iter->second;
                    std::vector<BURN_DATA_SOURCE>::const_iterator iterT;
                    for (iterT=vecBurnDataSource.begin();iterT!=vecBurnDataSource.end();++iterT)
                    {
                        if (SOURCE_TYPE_URL == iterT->strType &&
                            iterT->strSourceUrl != "")
                        {
                            vecDataSource.push_back(*iterT);
                        }
                    }
                }

                //////////////////////////////////////////////////////////////////////////
                //检查实时刻录视频源的连通性
                //////////////////////////////////////////////////////////////////////////
                std::vector<BURN_DATA_SOURCE> vecStreamSource;
                std::vector<BURN_DATA_SOURCE>::iterator iterT;
                for (iterT=vecDataSource.begin();iterT!=vecDataSource.end();++iterT)
                {
                    if (SOURCE_TYPE_URL == iterT->strType)
                    {
                        vecStreamSource.push_back(*iterT);
                    }
                }

                DEV_HANDLE handle=burnParam.vecDevHandle.at(0);

                size_t nCheckConnectableCount=0;
                while (true)
                {
                    if (CheckStreamConnectable(vecStreamSource))
                    {
                        LOG_INFO(("[BurnTask::BurnDisc] Stream source ok,%d\r\n",__LINE__));

                        break;
                    } 
                    else
                    {
                        ++nCheckConnectableCount;

                        LOG_INFO(("[BurnTask::BurnDisc] %d,%d\r\n",nCheckConnectableCount,__LINE__));

                        if (nCheckConnectableCount > 4)
                        {
                            break;
                        }
                        else
                        {
                            ZOSThread::Sleep(5000);
                        }
                    }
                }

                if (nCheckConnectableCount > 4)
                {
                    SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                    stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","streamunconnectable");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;

                    if (m_cb != NULL)
                    {
                        m_cb(burnParam.vecDevHandle.at(burnParam.nIndex),stateParam,m_pUsr);
                    }

                    SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                    if (burnParam.nTaskExtraFlag == TASK_EXTRA_STATE_ASYN_SINGLE)
                    {//如果被触发的任务失败，重置触发的任务状态
                        if (burnParam.strLastTaskID != "")
                        {
                            SetTaskExtraFlag(burnParam.strLastTaskID, TASK_EXTRA_STATE_ASYN_SINGLE);
                        }
                    }
                    return false;
                }

                LOG_INFO(("[BurnTask::BurnDisc] %d,%d\r\n",nCheckConnectableCount,__LINE__));

                //////////////////////////////////////////////////////////////////////////
                printf("===========================================================\r\n");
                printf("RT source : \r\n");
                std::vector<BURN_DATA_SOURCE>::iterator iterDataSource;
                int nSourceIndex=0;
                for (iterDataSource=vecDataSource.begin();iterDataSource!=vecDataSource.end();++iterDataSource)
                {
                    printf("Source %d : %s\r\n",nSourceIndex,iterDataSource->strSourceUrl.c_str());

                    ++nSourceIndex;
                }
                printf("===========================================================\r\n");
                //////////////////////////////////////////////////////////////////////////

                if (nAvailableDevIndex >= 0)
                {
                    handle=burnParam.vecDevHandle.at(nAvailableDevIndex);
                    LOG_INFO(("[BurnTask::BurnDisc] Use vecDevHandle[%d] %p to burn!\r\n",nAvailableDevIndex,handle));

                    LOG_INFO(("[BurnTask::BurnDisc] Defore IsDevReady,%d\r\n",__LINE__));

                    if (!IsDevReady(burnParam.vecDevHandle.at(nAvailableDevIndex),burnParam,mode))
                    {
                        //清理废弃的task的操作全由HandleTask执行
                        LOG_ERROR(("[BurnTask::BurnDisc] IsDevReady return false,%d\r\n",__LINE__));

                        if (burnParam.nTaskExtraFlag == TASK_EXTRA_STATE_ASYN_SINGLE)
                        {//如果被触发的任务失败，重置触发的任务状态
                            if (burnParam.strLastTaskID != "")
                            {
                                SetTaskExtraFlag(burnParam.strLastTaskID, TASK_EXTRA_STATE_ASYN_SINGLE);
                            }
                        }
                        return false;
                    }

                    LOG_INFO(("[BurnTask::BurnDisc] After IsDevReady,%d\r\n",__LINE__));

                    //进入检测光盘信息状态
                    stateParam.strStateFlag=STATE_BURNING;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","checkdisc");;
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;

                    if (m_cb != NULL)
                    {
                        m_cb(burnParam.vecDevHandle.at(nAvailableDevIndex),stateParam,m_pUsr);
                    }

                    BURN_DISC_INFO_T discInfo;
                    if (GetDiscInformation(burnParam.vecDevHandle.at(nAvailableDevIndex),discInfo))
                    {
                        LOG_INFO(("[BurnTask::BurnDisc] Get disc information success\r\n"));
                        LOG_INFO(("-------------------- DISC INFO ----------------------------\r\n"));
                        LOG_INFO(("Disc space : %d\r\n",discInfo.discsize));
                        LOG_INFO(("Disc used : %d\r\n",discInfo.usedsize));
                        LOG_INFO(("Disc free : %d\r\n",discInfo.freesize));
                        LOG_INFO(("-----------------------------------------------------------\r\n"));
                    }
                    else
                    {
                        LOG_ERROR(("[BurnTask::BurnDisc] Get disc information failed\r\n"));

                        SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                        stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","checkdiscfailed");;
                        stateParam.strJobID=burnParam.strJobID;
                        stateParam.strTaskID=burnParam.strTaskID;
                        stateParam.strUpperIP=burnParam.strUpperIP;
                        stateParam.strUpperPort=burnParam.strUpperPort;
                        stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                        if (m_cb != NULL)
                        {
                            m_cb(burnParam.vecDevHandle.at(nAvailableDevIndex),stateParam,m_pUsr);
                        }

                        SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                        if (burnParam.nTaskExtraFlag == TASK_EXTRA_STATE_ASYN_SINGLE)
                        {//如果被触发的任务失败，重置触发的任务状态
                            if (burnParam.strLastTaskID != "")
                            {
                                SetTaskExtraFlag(burnParam.strLastTaskID, TASK_EXTRA_STATE_ASYN_SINGLE);
                            }
                        }
                        return false;
                    }

                    stateParam.strStateFlag=STATE_BURNING;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","preparedata");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;

                    if (m_cb != NULL)
                    {
                        m_cb(burnParam.vecDevHandle.at(nAvailableDevIndex),stateParam,m_pUsr);
                    }

                    stateParam.strStateFlag=STATE_BURNING;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burning");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;
                    stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                    if (m_cb != NULL)
                    {
                        m_cb(handle,stateParam,m_pUsr);
                    }

                    SetCurDev(burnParam.strTaskID,nAvailableDevIndex);

                    if (StartSpecificDeviceToBurn(handle,vecDataSource,MEDIAFILE_BURN_STREAM,burnParam))
                    {
                        //停止实时刻录，向光盘添加附加数据，封盘，设置任务为空闲状态

                        UINT64 nCurTime=ZOS::milliseconds();
                        while (true)
                        {
                            //检查笔录文件是否已经更新成功
                            if (NOTE_FILE_UPDATED_YES == GetNoteUpdateFlag(GetBurningTaskID(handle)))
                            {
                                LOG_INFO(("[BurnTask::BurnDisc] Update note file success ........................\r\n"));

                                break;
                            }

                            ZOSThread::Sleep(1000);

                            if (ZOS::milliseconds() - nCurTime > 4*60*1000)
                            {
                                LOG_WARNING(("[BurnTask::BurnDisc] Update note file failed,%d ........................\r\n",
                                    __LINE__));

                                break;
                            }
                        }

                        stateParam.strStateFlag=STATE_RTTASK_STOPPING;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopping");
                        stateParam.strJobID=burnParam.strJobID;
                        stateParam.strTaskID=burnParam.strTaskID;
                        stateParam.strUpperIP=burnParam.strUpperIP;
                        stateParam.strUpperPort=burnParam.strUpperPort;
                        stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                        if (m_cb != NULL)
                        {
                            m_cb(handle,stateParam,m_pUsr);
                        }

                        RTBurnExtraData(handle,burnParam.mapBurnDataSource);

                        stateParam.strStateFlag=STATE_RTTASK_STOPPING;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","closedisc");
                        stateParam.strJobID=burnParam.strJobID;
                        stateParam.strTaskID=burnParam.strTaskID;
                        stateParam.strUpperIP=burnParam.strUpperIP;
                        stateParam.strUpperPort=burnParam.strUpperPort;
                        stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                        if (m_cb != NULL)
                        {
                            m_cb(handle,stateParam,m_pUsr);
                        }

                        if (BURN_SUCCESS == Burn_Ctrl_CloseDisc(handle))
                        {
                            SetDevUsed(handle,DEV_FLAG_USED_NO);

                            //实时刻录出现没有刻录任何数据但是每个刻录的接口都返回成功的情况，在此添加验证的代码
                            //TODO
                            // 

                            Burn_Ctrl_DevTray(handle,B_DEV_TRAY_OPEN);

                            stateParam.strStateFlag=STATE_SINGLE_DISC_BURNED;
                            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burnsuccess");
                            stateParam.strJobID=burnParam.strJobID;
                            stateParam.strTaskID=burnParam.strTaskID;
                            stateParam.strUpperIP=burnParam.strUpperIP;
                            stateParam.strUpperPort=burnParam.strUpperPort;
                            stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                            if (m_cb != NULL)
                            {
                                m_cb(handle,stateParam,m_pUsr);
                            }

                            SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                        }
                        else
                        {
                            SetDevUsed(handle,DEV_FLAG_USED_NO);

                            //此时应该通知BurnControl进行补刻

                            stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","closediscfailed");
                            stateParam.strJobID=burnParam.strJobID;
                            stateParam.strTaskID=burnParam.strTaskID;
                            stateParam.strUpperIP=burnParam.strUpperIP;
                            stateParam.strUpperPort=burnParam.strUpperPort;
                            stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                            //////////////////////////////////////////////////////////////////////////
                            stateParam.strExtraStateFlag=EXTRA_STATE_RTBURN_BACKUP_YES;
                            SaveRTBurnBackupTask(burnParam);
                            //////////////////////////////////////////////////////////////////////////

                            if (m_cb != NULL)
                            {
                                m_cb(handle,stateParam,m_pUsr);
                            }

                            SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

#ifdef RT_BURN_DISK_BACKUP
                            SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_BURN_BACKUP_SLEEP);
#endif

                            if (burnParam.nTaskExtraFlag == TASK_EXTRA_STATE_ASYN_SINGLE)
                            {//如果被触发的任务失败，重置触发的任务状态
                                if (burnParam.strLastTaskID != "")
                                {
                                    SetTaskExtraFlag(burnParam.strLastTaskID, TASK_EXTRA_STATE_ASYN_SINGLE);
                                }
                            }
                            return false;
                        }

                        LOG_INFO(("[BurnTask::BurnDisc] Single disc RT burn over !!!\r\n"));

                    }
                    else
                    {
                        SetDevUsed(handle,DEV_FLAG_USED_NO);

                        SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                        SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                        if (burnParam.nTaskExtraFlag == TASK_EXTRA_STATE_ASYN_SINGLE)
                        {//如果被触发的任务失败，重置触发的任务状态
                            if (burnParam.strLastTaskID != "")
                            {
                                SetTaskExtraFlag(burnParam.strLastTaskID, TASK_EXTRA_STATE_ASYN_SINGLE);
                            }
                        }
                        return false;
                    }

                    int nTaskControlState=GetTaskControlState(burnParam.strTaskID);
                    LOG_INFO(("[BurnTask::BurnDisc] Rt burn task control state : %d,%d\r\n",
                        nTaskControlState,__LINE__));

                    if (nTaskControlState == TASK_CONTROL_STATE_PAUSE ||
                        nTaskControlState == TASK_CONTROL_STATE_STOP)
                    {
                        LOG_INFO(("[BurnTask::BurnDisc] Rt single task success, strTaskID: %s,  nDevIndex: %d\r\n",
                            burnParam.strTaskID.c_str(), nAvailableDevIndex));
                    }
                }
                else if (nAvailableDevIndex == -1)
                {
                    LOG_WARNING(("[BurnTask::BurnDisc] No available device! %d\r\n",__LINE__));
                    SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                    SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                    stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","devunavailable");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;
                    if (m_cb != NULL)
                    {
                        m_cb(NULL,stateParam,m_pUsr);
                    }

                    if (burnParam.nTaskExtraFlag == TASK_EXTRA_STATE_ASYN_SINGLE)
                    {//如果被触发的任务失败，重置触发的任务状态
                        if (burnParam.strLastTaskID != "")
                        {
                            SetTaskExtraFlag(burnParam.strLastTaskID, TASK_EXTRA_STATE_ASYN_SINGLE);
                        }
                    }
                    return false;
                }
            }
            else
            {
                LOG_ERROR(("[BurnTask::BurnDisc] Dev count wrong\r\n"));
                assert(false);
            }

            //置实时刻录任务为停止状态
            //在暂停操作时，会将实时刻录任务状态设置为0，添加此判断为了防止冲突
            int nRtTaskControlState=GetTaskControlState(burnParam.strTaskID);
            if (TASK_CONTROL_STATE_PAUSE == nRtTaskControlState)
            {
                SetTaskRunningState(burnParam.strTaskID,TASK_STATE_PAUSED);

                stateParam.strStateFlag=STATE_RTTASK_PAUSED;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","paused");
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                if (m_cb != NULL)
                {
                    DEV_HANDLE handle=GetCurDev(burnParam.strTaskID);
                    m_cb(handle,stateParam,m_pUsr);
                }

                nRtTaskControlState=GetTaskControlState(burnParam.strTaskID);
                while (TASK_CONTROL_STATE_PAUSE == nRtTaskControlState)
                {
                    ZOSThread::Sleep(1000);

                    nRtTaskControlState=GetTaskControlState(burnParam.strTaskID);
                }
            }
            else if (TASK_CONTROL_STATE_STOP == nRtTaskControlState)
            {
                SetTaskRunningState(burnParam.strTaskID,TASK_STATE_STOPPED);

                LOG_INFO(("[BurnTask::BurnDisc]Task %s is stopped,%d\r\n",
                    burnParam.strTaskID.c_str(),__LINE__));

                stateParam.strStateFlag=STATE_RTTASK_STOPPED;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopped");
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                if (m_cb != NULL)
                {
                    DEV_HANDLE handle=GetCurDev(burnParam.strTaskID);
                    m_cb(handle,stateParam,m_pUsr);
                }

                SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);
                return true;
            }
        }//case rt
        break;

    default:
        break;
    }

    //一个刻录的task完成，对实时刻录和事后刻录都适用

    SetTaskRunningState(burnParam.strTaskID,TASK_STATE_STOPPED);

    SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

    stateParam.strStateFlag=STATE_TASK_OVER;
    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burnsuccess");
    stateParam.strTaskID=burnParam.strTaskID;
    stateParam.strJobID=burnParam.strJobID;

    stateParam.strUpperIP=burnParam.strUpperIP;
    stateParam.strUpperPort=burnParam.strUpperPort;
    stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

    if (m_cb != NULL)
    {
        m_cb(NULL,stateParam,m_pUsr);
    }

    return true;
}

void BurnTask::CleanTask(std::string strTaskID)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();)
    {
        if (iter->strTaskID == strTaskID)
        {
            LOG_INFO(("[BurnTask::CleanTask] Task %s is cleaned\r\n",strTaskID.c_str()));

            iter=m_vecBurnParameter.erase(iter);

            ListLeftTasks();

            break;
        }
        else
        {
            ++iter;
        }
    }
}

//刻录底层的回调函数
/*
typedef enum
{
B_RUNNING_OK = 0,              // 运行正常
B_TRAY_IS_OPEN,                // 托盘是开启的
B_NO_HAVE_DISC,                // 没有光盘
B_DISC_FORMAT_FAILED,          // 光盘格式化失败
B_DISC_HAS_NO_SPACE,           // 光盘没有空间
B_DISC_CAN_NOT_WRITE,          // 光盘不能写操作

B_CREATE_FILE_FAILED,          // 光盘上创建文件失败
B_WRITE_FILE_FAILED,           // 光盘上写文件失败
B_CLOSE_FILE_FAILED,           // 光盘上关闭文件失败
B_CLOSE_DISC_FAILED,           // 光盘封盘失败

B_LOCAL_FILE_NO_FIND,          // 本地文件无法找到
B_LOCAL_FILE_READ_FAILED,      // 本地文件无法读取
B_LOCAL_DIR_NO_FIND,           // 本地目录无法找到
B_DISK_HAS_NO_SPACE,           // 硬盘没有空间
}RUNNING_INFO;
*/
int BurnTask::BurnCoreCallBack(DEV_HANDLE hDEV,const BURN_RUN_STATE_T *pState,void *p)
{
    if (NULL == pState ||
        NULL == p ||
        NULL == hDEV)
    {
        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p, Param error,%d\r\n",(void*)hDEV,__LINE__));

        assert(false);

        return 0;
    }
    BURN_STATE_PARAMETER stateParam;

    RUNNING_STATE runningState=pState->running_state;
    LOG_INFO(("[BurnTask::BurnCoreCallBack] dev hDEV: %p, runningState: %d,%d\r\n",(void*)hDEV,runningState,__LINE__));
    if (B_ABNORMAL == runningState)
    {
        //暂停、停止、异常、光盘空间不足的信号处理

        RUNNING_INFO   runningInfo=pState->running_info;
        switch (runningInfo)
        {
        case B_RUNNING_OK:
            break;

        case ERR_TRAY_IS_OPEN:
            break;

        case ERR_NO_HAVE_DISC:
            break;

        case ERR_DISC_FORMAT_FAILED:
            break;

        case ERR_DISC_HAS_NO_SPACE:
            {
                LOG_WARNING(("[BurnTask::BurnCoreCallBack] dev hDEV: %p NO SPACE SIGNAL,%d\r\n",(void *)hDEV,__LINE__));

#if 0
                //用于测试实时刻录控制的逻辑
                ZOSThread::Sleep(2000);
#endif

#if 1
                BurnTask *pBurnTask=(BurnTask *)p;
                if(pBurnTask != NULL)
                {
                    std::string strTaskID=pBurnTask->GetBurningTaskID(hDEV);

                    assert(strTaskID != "");

                    if ("" == strTaskID)
                    {
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Can not find task,%d\r\n",
                            (void *)hDEV,__LINE__));
                        return -1;
                    }
#ifdef WIN32
#else
                    int nRtTaskRunningState=pBurnTask->GetTaskRunningState(strTaskID);
                    int nRtTaskControlState=pBurnTask->GetTaskControlState(strTaskID);

                    LOG_INFO(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Task  id : %s, running state : %d,\
                              control state : %d, %d\r\n",
                        (void*)hDEV,strTaskID.c_str(),nRtTaskRunningState,nRtTaskControlState,__LINE__));

                    switch (nRtTaskRunningState)
                    {
                    case TASK_STATE_PAUSED:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Task state error,cur is paused,%d\r\n",
                            (void*)hDEV,__LINE__));
                        break;

                    case TASK_STATE_READY_TO_RUN:
                    case TASK_STATE_RUNNING:
                        {
                            if (TASK_CONTROL_STATE_PAUSE == nRtTaskControlState)
                            {
                                //stateParam.strStateFlag=STATE_BURNING;
                                //stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","pausing");
                                //stateParam.strTaskID=strTaskID;
                                //stateParam.strJobID=pBurnTask->GetJobID(hDEV);

                                //if (pBurnTask->m_cb != NULL)
                                //{
                                //    pBurnTask->m_cb(hDEV,stateParam,pBurnTask->m_pUsr);
                                //}
                            } 
                            else if(TASK_CONTROL_STATE_STOP == nRtTaskControlState)
                            {
                                //pBurnTask->SetRTBurnBackupTaskRunningState(strTaskID,TASK_STATE_NO_USE);

                                //pBurnTask->SetTaskRunningState(strTaskID,TASK_CONTROL_STATE_STOP);

                                //stateParam.strStateFlag=STATE_BURNING;
                                //stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopping");
                                //stateParam.strTaskID=pBurnTask->GetTaskID(hDEV);
                                //stateParam.strJobID=pBurnTask->GetJobID(hDEV);

                                //if (pBurnTask->m_cb != NULL)
                                //{
                                //    pBurnTask->m_cb(hDEV,stateParam,pBurnTask->m_pUsr);
                                //}
                            }
                        }
                        break;

                    case TASK_STATE_STOPPED:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Task state error,%d\r\n",(void *)hDEV,__LINE__));
                        break;

                    case TASK_STATE_BURN_LOCAL:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Task state error,%d\r\n",(void *)hDEV,__LINE__));
                        break;

                    case TASK_STATE_BURN_STREAM:
                        {
                            //首先设置好任务的运行状态，这样在退出刻录实时流的函数后，判断是循环等待，还是封盘
                            if (TASK_CONTROL_STATE_PAUSE == nRtTaskControlState)
                            {
                                //stateParam.strStateFlag=STATE_BURNING;
                                //stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","pausing");
                                //stateParam.strTaskID=strTaskID;
                                //stateParam.strJobID=pBurnTask->GetJobID(hDEV);

                                //if (pBurnTask->m_cb != NULL)
                                //{
                                //    pBurnTask->m_cb(hDEV,stateParam,pBurnTask->m_pUsr);
                                //}
                            } 
//                             else if(TASK_CONTROL_STATE_STOP == nRtTaskControlState)
                            {
                                pBurnTask->SetRTBurnBackupTaskRunningState(strTaskID,TASK_STATE_NO_USE);

                                //设置实时刻录为刻录本地文件的状态
                                pBurnTask->SetTaskRunningState(pBurnTask->GetBurningTaskID(hDEV),TASK_STATE_BURN_LOCAL);

                                stateParam.strStateFlag=STATE_WILL_CLOSE_DISC;
                                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopping");
                                stateParam.strTaskID=pBurnTask->GetBurningTaskID(hDEV);
                                stateParam.strJobID=pBurnTask->GetBurningJobID(hDEV);

                                if (pBurnTask->m_cb != NULL)
                                {
                                    pBurnTask->m_cb(hDEV,stateParam,pBurnTask->m_pUsr);
                                }
                            }

                            //如果现在为恢复刻录状态，则会新建一个ts文件，继续刻录，因此需要重设状态来停止该任务
                            pBurnTask->SetTaskControlState(pBurnTask->GetBurningTaskID(hDEV), TASK_CONTROL_STATE_DEFAULT);

                            if (BURN_SUCCESS == Stop_Stream_Burning(hDEV))
                            {
                                LOG_INFO(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Stop rt burn success,%d\r\n",
                                    (void *)hDEV,__LINE__));
                            } 
                            else
                            {
                                pBurnTask->SetTaskRunningState(strTaskID,TASK_STATE_BURN_STREAM);

                                LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Stop rt burn failed,%d\r\n",(void *)hDEV,__LINE__));

                                stateParam.strStateFlag=STATE_BURNING;
                                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stoprtburnfailed");
                                stateParam.strTaskID=pBurnTask->GetBurningTaskID(hDEV);
                                stateParam.strJobID=pBurnTask->GetBurningJobID(hDEV);

                                if (pBurnTask->m_cb != NULL)
                                {
                                    pBurnTask->m_cb(hDEV,stateParam,pBurnTask->m_pUsr);
                                }

                                assert(false);
                            }
                        }
                        break;

                    case TASK_STATE_ERR:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Task state error,%d\r\n",(void *)hDEV,__LINE__));
                        break;

                    case TASK_STATE_NO_USE:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Task state error,%d\r\n",(void *)hDEV,__LINE__));
                        break;

                    default:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Task state error : %d,%d\r\n",
                            (void *)hDEV,nRtTaskRunningState,__LINE__));
                        break;
                    }
#endif

                }
#endif

            }
            break;

        case ERR_DISC_CAN_NOT_WRITE:
            break;

        case ERR_CREATE_FILE_FAILED:
            break;

        case ERR_WRITE_FILE_FAILED:
            break;

        case ERR_CLOSE_FILE_FAILED:
            break;

        case ERR_CLOSE_DISC_FAILED:
            break;

        case ERR_LOCAL_FILE_NO_FIND:
            break;

        case ERR_LOCAL_FILE_READ_FAILED:
            break;

        case ERR_LOCAL_DIR_NO_FIND:
            break;

        case ERR_DISK_HAS_NO_SPACE:
            break;
        }
    } 
    else if(B_NORMALLY == runningState)
    {
        RUNNING_INFO   runningInfo=pState->running_info;
        switch (runningInfo)
        {
        case WARNING_DISC_WILL_NO_SPACE:
            {
#if 1
                BurnTask *pBurnTask=(BurnTask *)p;
                if(pBurnTask != NULL)
                {
                    std::string strTaskID=pBurnTask->GetBurningTaskID(hDEV);

                    assert(strTaskID != "");

                    if ("" == strTaskID)
                    {
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Can not find task,%d\r\n",
                            (void *)hDEV,__LINE__));
                        return -1;
                    }
#ifdef WIN32
#else
                    int nRtTaskRunningState=pBurnTask->GetTaskRunningState(strTaskID);
                    int nRtTaskControlState=pBurnTask->GetTaskControlState(strTaskID);
                    int nTaskExtraFlag=pBurnTask->GetTaskExtraFlag(strTaskID);

                    switch (nRtTaskRunningState)
                    {
                    case TASK_STATE_PAUSED:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p WARNING_DISC_WILL_NO_SPACE Task state error,cur is paused,%d\r\n",
                            (void *)hDEV,__LINE__));
                        break;

                    case TASK_STATE_READY_TO_RUN:
                    case TASK_STATE_RUNNING:
                        break;

                    case TASK_STATE_STOPPED:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p WARNING_DISC_WILL_NO_SPACE Task state error,%d\r\n",(void *)hDEV,__LINE__));
                        break;

                    case TASK_STATE_BURN_LOCAL:
                    case TASK_STATE_BURN_STREAM:
                        {
                            if (pBurnTask->GetTaskControlState(strTaskID) == TASK_CONTROL_STATE_DEFAULT
                                || pBurnTask->GetTaskControlState(strTaskID) == TASK_CONTROL_STATE_START_OR_RESUME)
                            {
                                LOG_INFO(("[BurnTask::BurnCoreCallBack] dev hDEV: %p WARNING_DISC_WILL_NO_SPACE!Task id : %s, running state : %d, control state : %d, nTaskExtraFlag: %d, %d\r\n",
                                    (void *)hDEV,strTaskID.c_str(),nRtTaskRunningState,nRtTaskControlState,nTaskExtraFlag,__LINE__));

                                if (nTaskExtraFlag == TASK_EXTRA_STATE_ASYN_SINGLE)
                                {
                                    LOG_INFO(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Job %s: start another device to burn,%d\r\n",(void *)hDEV,(pBurnTask->GetBurningJobID(hDEV)).c_str(),__LINE__));
                                    pBurnTask->SetTaskExtraFlag(strTaskID, TASK_EXTRA_STATE_ASYN_MULTI);

                                    stateParam.strStateFlag=STATE_WILL_NO_SPACE;
                                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopping");
                                    stateParam.strTaskID=pBurnTask->GetBurningTaskID(hDEV);
                                    stateParam.strJobID=pBurnTask->GetBurningJobID(hDEV);

                                    if (pBurnTask->m_cb != NULL)
                                    {
                                        pBurnTask->m_cb(hDEV,stateParam,pBurnTask->m_pUsr);
                                    }
                                }
                            }
                        }
                        break;

                    case TASK_STATE_ERR:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p WARNING_DISC_WILL_NO_SPACE Task state error,%d\r\n",(void *)hDEV,__LINE__));
                        break;

                    case TASK_STATE_NO_USE:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p WARNING_DISC_WILL_NO_SPACE Task state error,%d\r\n",(void *)hDEV,__LINE__));
                        break;

                    default:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p WARNING_DISC_WILL_NO_SPACE Task state error : %d,%d\r\n",
                            (void *)hDEV,nRtTaskRunningState,__LINE__));
                        break;
                    }
#endif

                }
#endif
            }
            break;
        case B_RUNNING_OK:
            {
#if 0
                BurnTask *pBurnTask=(BurnTask *)p;
                if(pBurnTask != NULL)
                {
                    std::string strTaskID=pBurnTask->GetBurningTaskID(hDEV);

                    assert(strTaskID != "");

                    if ("" == strTaskID)
                    {
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p Can not find task,%d\r\n",
                            (void *)hDEV,__LINE__));
                        return -1;
                    }
#ifdef WIN32
#else
                    int nRtTaskRunningState=pBurnTask->GetTaskRunningState(strTaskID);
                    int nRtTaskControlState=pBurnTask->GetTaskControlState(strTaskID);
                    int nTaskExtraFlag=pBurnTask->GetTaskExtraFlag(strTaskID);

                    switch (nRtTaskRunningState)
                    {
                    case TASK_STATE_PAUSED:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p B_RUNNING_OK Task state error,cur is paused,%d\r\n",
                            (void *)hDEV,__LINE__));
                        break;

                    case TASK_STATE_READY_TO_RUN:
                    case TASK_STATE_RUNNING:
                        break;

                    case TASK_STATE_STOPPED:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p B_RUNNING_OK Task state error,%d\r\n",(void *)hDEV,__LINE__));
                        break;

                    case TASK_STATE_BURN_LOCAL:
                    case TASK_STATE_BURN_STREAM:
                        {
                            if (pBurnTask->GetTaskControlState(strTaskID) == TASK_CONTROL_STATE_DEFAULT
                                || pBurnTask->GetTaskControlState(strTaskID) == TASK_CONTROL_STATE_START_OR_RESUME)
                            {
                                LOG_INFO(("[BurnTask::BurnCoreCallBack] dev hDEV: %p B_RUNNING_OK!Task id : %s, running state : %d, control state : %d, nTaskExtraFlag: %d, %d\r\n",
                                    (void *)hDEV,strTaskID.c_str(),nRtTaskRunningState,nRtTaskControlState,nTaskExtraFlag,__LINE__));

                                stateParam.strStateFlag=STATE_BURNING;
                                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burning");
                                stateParam.strTaskID=pBurnTask->GetBurningTaskID(hDEV);
                                stateParam.strJobID=pBurnTask->GetBurningJobID(hDEV);

                                if (pBurnTask->m_cb != NULL)
                                {
                                    pBurnTask->m_cb(hDEV,stateParam,pBurnTask->m_pUsr);
                                }
                            }
                        }
                        break;

                    case TASK_STATE_ERR:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p B_RUNNING_OK Task state error,%d\r\n",(void *)hDEV,__LINE__));
                        break;

                    case TASK_STATE_NO_USE:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p B_RUNNING_OK Task state error,%d\r\n",(void *)hDEV,__LINE__));
                        break;

                    default:
                        LOG_ERROR(("[BurnTask::BurnCoreCallBack] dev hDEV: %p B_RUNNING_OK Task state error : %d,%d\r\n",
                            (void *)hDEV,nRtTaskRunningState,__LINE__));
                        break;
                    }
#endif

                }
#endif
            }
            break;
        }
        //刻录过程状态的信号处理
    }

    return 0;
}

void BurnTask::RTBurnExtraData(DEV_HANDLE handle,const std::map< std::string,std::vector<BURN_DATA_SOURCE> > &mapData)
{
    LOG_INFO(("[BurnTask::RTBurnExtraData] Begin ...,%d\r\n",__LINE__));

    if (mapData.empty())
    {
        LOG_ERROR(("[BurnTask::RTBurnExtraData] No data\r\n"));

        return;
    }

    std::string strSource;
    std::map< std::string,std::vector<BURN_DATA_SOURCE> >::const_iterator iter;
#if 1
    //playlist
    if ( (iter=mapData.find("playlist")) != mapData.end())
    {
        if (1 == iter->second.size())
        {
            strSource=iter->second.at(0).strSourceUrl;

            if (IsFileOrDirExist(strSource.c_str(),false))
            {
                if (BURN_SUCCESS == Burn_File_Form_Local_File(handle,NULL,
                    const_cast<char *>(strSource.c_str())))
                {
                    LOG_INFO(("[BurnTask::RTBurnExtraData] RT burn,burn play list file success: %s,%d\r\n",
                        iter->second.at(0).strSourceUrl.c_str(),__LINE__));
                } 
                else
                {
                    LOG_ERROR(("[BurnTask::RTBurnExtraData] RT burn,burn play list file failed: %s,%d\r\n",
                        iter->second.at(0).strSourceUrl.c_str(),__LINE__));
                }
            } 
            else
            {
                LOG_ERROR(("[BurnTask::RTBurnExtraData] File not exist : %s,%d\r\n",
                    strSource.c_str(),__LINE__));
            }
        } 
        else
        {
            LOG_ERROR(("[BurnTask::RTBurnExtraData] Data source num error : %d,%d\r\n",
                iter->second.size(),__LINE__));
        }
    }
    else
    {
        LOG_WARNING(("[BurnTask::RTBurnExtraData] No playlist,%d\r\n",__LINE__));
    }
#endif
    Burn_File_Form_Local_File(handle, NULL, "/root/README");
    Burn_File_Form_Local_File(handle, NULL, "/root/README1");
    Burn_File_Form_Local_File(handle, NULL, "/root/README2");
    Burn_File_Form_Local_File(handle, NULL, "/root/README3");

    //note
    if ( (iter=mapData.find("note")) != mapData.end())
    {
        if (1 == iter->second.size())
        {
            strSource=iter->second.at(0).strSourceUrl;

            if (IsFileOrDirExist(strSource.c_str(),false))
            {
                if (BURN_SUCCESS == Burn_File_Form_Local_File(handle,NULL,
                    const_cast<char *>(strSource.c_str())))
                {
                    int size=static_cast<int>(FileUtil::FileSize(strSource.c_str()));
                    LOG_INFO(("[BurnTask::RTBurnExtraData] RT burn,burn note file success: %s,%d,%d\r\n",
                        strSource.c_str(),size,__LINE__));
                } 
                else
                {
                    LOG_ERROR(("[BurnTask::RTBurnExtraData] RT burn,burn note file failed: %s,%d\r\n",
                        strSource.c_str(),__LINE__));
                }
            }
            else
            {
                LOG_ERROR(("[BurnTask::RTBurnExtraData] File not exist : %s,%d\r\n",
                    strSource.c_str(),__LINE__));
            }
        } 
        else
        {
            LOG_ERROR(("[BurnTask::RTBurnExtraData] Data source num error : %d,%d\r\n",
                iter->second.size(),__LINE__));
        }
    }
    else
    {
        LOG_WARNING(("[BurnTask::RTBurnExtraData] No note,%d\r\n",__LINE__));
    }
#if 1
    //autorun
    if ( (iter=mapData.find("autorun")) != mapData.end())
    {
        if (1 == iter->second.size())
        {
            strSource=iter->second.at(0).strSourceUrl;

            if (IsFileOrDirExist(strSource.c_str(),false))
            {
                if (BURN_SUCCESS == Burn_File_Form_Local_File(handle,NULL,
                    const_cast<char *>(strSource.c_str())))
                {
                    LOG_INFO(("[BurnTask::RTBurnExtraData] RT burn,burn autorun file success: %s,%d\r\n",
                        iter->second.at(0).strSourceUrl.c_str(),__LINE__));
                } 
                else
                {
                    LOG_ERROR(("[BurnTask::RTBurnExtraData] RT burn,burn autorun file failed: %s,%d\r\n",
                        iter->second.at(0).strSourceUrl.c_str(),__LINE__));
                }
            }
            else
            {
                LOG_ERROR(("[BurnTask::RTBurnExtraData] File not exist : %s,%d\r\n",
                    strSource.c_str(),__LINE__));
            }
        } 
        else
        {
            LOG_ERROR(("[BurnTask::RTBurnExtraData] Data source num error : %d,%d\r\n",
                iter->second.size(),__LINE__));
        }
    }
    else
    {
        LOG_WARNING(("[BurnTask::RTBurnExtraData] No autorun,%d\r\n",__LINE__));
    }

    //auth
    if ( (iter=mapData.find("auth")) != mapData.end())
    {
        if (1 == iter->second.size())
        {
            strSource=iter->second.at(0).strSourceUrl;

            if (IsFileOrDirExist(strSource.c_str(),false))
            {
                if (BURN_SUCCESS == Burn_File_Form_Local_File(handle,NULL,
                    const_cast<char *>(strSource.c_str())))
                {
                    LOG_INFO(("[BurnTask::RTBurnExtraData] RT burn,burn auth file success: %s,%d\r\n",
                        iter->second.at(0).strSourceUrl.c_str(),__LINE__));
                } 
                else
                {
                    LOG_ERROR(("[BurnTask::RTBurnExtraData] RT burn,burn auth file failed: %s,%d\r\n",
                        iter->second.at(0).strSourceUrl.c_str(),__LINE__));
                }
            }
            else
            {
                LOG_ERROR(("[BurnTask::RTBurnExtraData] File not exist : %s,%d\r\n",
                    strSource.c_str(),__LINE__));
            }
        } 
        else
        {
            LOG_ERROR(("[BurnTask::RTBurnExtraData] Data source num error : %d,%d\r\n",
                iter->second.size(),__LINE__));
        }
    }
    else
    {
        LOG_WARNING(("[BurnTask::RTBurnExtraData] No auth,%d\r\n",__LINE__));
    }
	//player
    if ( (iter=mapData.find("player")) != mapData.end())
    {
        if (1 == iter->second.size())
        {
            strSource=iter->second.at(0).strSourceUrl;
            if (IsFileOrDirExist(strSource.c_str(),true))
            {
                if (BURN_SUCCESS == Burn_Dir_Form_Local_Dir(handle,NULL,
                    const_cast<char *>(strSource.c_str())))
                {
                    LOG_INFO(("[BurnTask::RTBurnExtraData] RT burn,burn player success: %s,%d\r\n",
                        iter->second.at(0).strSourceUrl.c_str(),__LINE__));
                }
                else
                {
                    LOG_ERROR(("[BurnTask::RTBurnExtraData] RT burn,burn player failed: %s,%d\r\n",
                        iter->second.at(0).strSourceUrl.c_str(),__LINE__));
                }
            }
            else
            {
                LOG_ERROR(("[BurnTask::RTBurnExtraData] Find no player : %s,%d\r\n",
                    strSource.c_str(),__LINE__));
            }
        }
        else
        {
            LOG_ERROR(("[BurnTask::RTBurnExtraData] Data source num error : %d,%d\r\n",
                iter->second.size(),__LINE__));
        }
    }
    else
    {
        LOG_WARNING(("[BurnTask::RTBurnExtraData] No player，%d\r\n",__LINE__));
    }
#endif

//	Burn_File_Form_Local_File(handle, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED.playlist");
//	Burn_File_Form_Local_File(handle, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/AutoRun.inf");
//	Burn_File_Form_Local_File(handle, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/Auth.exe");


//	Burn_Dir_Form_Local_Dir(handle, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/PlayBack" );
}

void BurnTask::ResetTask(std::string strTaskID)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        if (iter->strTaskID == strTaskID)
        {
            iter->nTaskRunningFlag=TASK_STATE_READY_TO_RUN;

            LOG_INFO(("[BurnTask::ResetTask] Reset task : %s\r\n",strTaskID.c_str()));

            break;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        LOG_INFO(("[BurnTask::ResetTask] Can not find task : %s\r\n",strTaskID.c_str()));

        assert(false);
    }
}



bool BurnTask::InsertPlayListFile(std::vector<BURN_DATA_SOURCE> &vecDataSource,std::string strFullPlaylistFilePath)
{
    bool bRet=false;

    std::string strSingleTaskPlaylistFilePath=CreatePlayListFile(vecDataSource,strFullPlaylistFilePath);
    if(strSingleTaskPlaylistFilePath != "")
    {
        BURN_DATA_SOURCE data;
        data.strType=SOURCE_TYPE_NORMAL_FILE;
        data.strSourceUrl=strSingleTaskPlaylistFilePath;
        vecDataSource.push_back(data);

        bRet=true;;
    }

    return bRet;
}

bool BurnTask::GetDiscInformation(DEV_HANDLE handle,BURN_DISC_INFO_T &discInfo)
{
    bool bRet=false;

    memset(&discInfo,0,sizeof(discInfo));

#ifdef WIN32
    DISC_INFORMATION d;
    if (GetDiscInfo(handle->hDVD,handle->dev_id,d))
    {
        discInfo.freesize=static_cast<unsigned int>(d.uiFreeBytes/1024/1024);
        discInfo.usedsize=static_cast<unsigned int>(d.uiUsedBytes/1024/1024);
        discInfo.discsize=discInfo.freesize+discInfo.usedsize;

        bRet=true;
    }
#else
    bRet=( BURN_SUCCESS == Burn_Get_DiscInfo(handle,&discInfo) );
#endif

    return bRet;
}

std::string BurnTask::CreatePlayListFile(const std::vector<BURN_DATA_SOURCE> &vecDataSource,std::string strPlaylistFullPath)
{
    std::string strTempPlaylistFullPath;

#ifdef WIN32
    CStringW strTempDir=CharsetConvertMFC::UTF8ToUTF16(Path(gBasicParamCfgFile.GetValue("info","burnServerDownloadDir"),true).c_str());

    CStringW strPlaylistFullPathW=CharsetConvertMFC::UTF8ToUTF16(strPlaylistFullPath.c_str());

    std::vector<std::wstring> files;
    files.push_back(strPlaylistFullPathW.GetBuffer());

    DirectoryUtil::CopyFileToDirW(strTempDir.GetBuffer(),files);

    std::wstring strFileName;
    strFileName = FileUtil::GetFileNameW(strPlaylistFullPathW.GetBuffer());

    CStringW strUUID=CStringW(CStringA(UUID_GetUUID().c_str()));
    strUUID.Replace(L'-',L'_');

    CStringW strNewTempFileName=strTempDir+strUUID+L".playlist";

    MoveFile(strTempDir+CStringW(strFileName.c_str()),strNewTempFileName);

    strTempPlaylistFullPath=CharsetConvertMFC::UTF16ToUTF8(strNewTempFileName).GetBuffer();

#else
    std::string strParentDir=DirectoryUtil::GetParentDir(strPlaylistFullPath);

    std::string strDir=strParentDir+UUID_GetUUID();

    size_t pos=strDir.find('-');
    while (pos != std::string::npos)
    {
        strDir.replace(strDir.begin()+pos,strDir.begin()+pos+1,1,'_');

        pos=strDir.find('-',pos);
    }

    if (0 != mkdir(strDir.c_str(),0777))
    {
        LOG_ERROR(("[BurnTask::CreatePlayListFile] Create temp dir for playlist failed,%s\r\n",strDir.c_str()));
    }

    strTempPlaylistFullPath=strDir+"/"+FileUtil::GetFileName(strPlaylistFullPath);

    LOG_INFO(("[BurnTask::CreatePlayListFile] Temp play list full path : %s\r\n",strTempPlaylistFullPath.c_str()));
#endif


    TiXmlDocument xmlDoc;
    bool bLoadFile=false;

#ifdef WIN32
    bLoadFile=xmlDoc.LoadFile(strTempPlaylistFullPath.c_str(),TIXML_ENCODING_LEGACY);
#else
    bLoadFile=xmlDoc.LoadFile(strPlaylistFullPath.c_str(),TIXML_ENCODING_LEGACY);
#endif

    if (bLoadFile)
    {
        LOG_INFO(("[BurnTask::CreatePlayListFile] Load org play list file success\r\n"));

        std::map<std::string,std::string> mapFileName;
        std::vector<BURN_DATA_SOURCE>::const_iterator iter;
        for (iter=vecDataSource.begin();iter!=vecDataSource.end();++iter)
        {
            LOG_INFO(("####[BurnTask::CreatePlayListFile] Cur data source : %s\r\n",iter->strSourceUrl.c_str()));

            mapFileName[FileUtil::GetFileName(iter->strSourceUrl)]="";
        }

        TiXmlNode *pRoot=xmlDoc.RootElement();
        if (pRoot != NULL)
        {
            TiXmlNode *pView=NULL;
            for (pView=pRoot->FirstChildElement("view");pView!=NULL;)
            {
                TiXmlNode *pFile=NULL;
                for (pFile=pView->FirstChildElement();pFile!=NULL;)
                {
                    LOG_INFO(("[BurnTask::CreatePlayListFile] File element~\r\n"));

                    std::string strFileName=pFile->ToElement()->GetText();

                    LOG_INFO(("[BurnTask::CreatePlayListFile] File name : %s,pFile value : %s\r\n",
                        strFileName.c_str(),pFile->Value()));

                    if (!mapFileName.empty() &&
                        mapFileName.find(strFileName) != mapFileName.end())
                    {
                        pFile=pFile->NextSiblingElement();
                    }
                    else
                    {
                        pView->RemoveChild(pFile);

                        pFile=pView->FirstChildElement();
                    }
                }

                pFile=pView->FirstChildElement();
                if (NULL == pFile)
                {
                    pRoot->RemoveChild(pView);

                    pView=pRoot->FirstChildElement("view");
                } 
                else
                {
                    pView=pView->NextSiblingElement("view");
                }
            }
        }

#ifdef WIN32
        if (!xmlDoc.SaveFile())
        {
            strTempPlaylistFullPath="";
        }
#else
        if (!xmlDoc.SaveFile(strTempPlaylistFullPath.c_str()))
        {
            strTempPlaylistFullPath="";
        }
#endif
    }
    else
    {
        LOG_ERROR(("[BurnTask::CreatePlayListFile] Load org file error : %s\r\n",strPlaylistFullPath.c_str()));

        strTempPlaylistFullPath="";
    }

#ifdef WIN32
    files.clear();
    files.push_back(strNewTempFileName.GetBuffer());

    DirectoryUtil::CopyFileToDirW(DirectoryUtil::GetParentDirW(strPlaylistFullPathW.GetBuffer()).c_str(),files);

    strTempPlaylistFullPath=CharsetConvertMFC::UTF16ToUTF8(DirectoryUtil::GetParentDirW(strPlaylistFullPathW.GetBuffer()).c_str()+
        strUUID+L".playlist");

    DeleteFile(strNewTempFileName);
#endif

    return strTempPlaylistFullPath;
}

bool BurnTask::GetAddrFromURL(const std::string &strURL,std::string &strIP,std::string &strPort)
{
    strIP="";
    strPort="554";

    if ("" == strURL)
    {
        return false;
    }

    char *pszProtocol  = NULL;
    char *pszHost      = NULL;
    char *pszPort      = NULL;
    char *pszUser      = NULL;
    char *pszPass      = NULL;
    char *pszPath      = NULL;

    char szTemp[1024]={0};
    strncpy(szTemp,strURL.c_str(),sizeof(szTemp)/sizeof(char));

    if(ZOS::DecodeURL(szTemp,&pszProtocol,&pszHost,&pszPort,&pszUser,
        &pszPass,&pszPath))
    {
        if (pszHost != NULL)
        {
            strIP=std::string(pszHost);
        }
        
        if (pszPort != NULL)
        {
            strPort=std::string(pszPort);
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool BurnTask::CheckStreamConnectable(const std::vector<BURN_DATA_SOURCE> &vecStreamSource)
{
    size_t n=0;

#ifdef WIN32
#else
    std::vector<BURN_DATA_SOURCE>::const_iterator iterStreamSource;
    for (iterStreamSource=vecStreamSource.begin();iterStreamSource!=vecStreamSource.end();++iterStreamSource)
    {
        assert(SOURCE_TYPE_URL == iterStreamSource->strType);

        std::string strIP;
        std::string strPort;

        if (GetAddrFromURL(iterStreamSource->strSourceUrl,strIP,strPort))
        {
            if (!SocketUtil::CheckTcpPortConnectable(strIP.c_str(),atoi(strPort.c_str()),1000))
            {
                LOG_ERROR(("[BurnTask::CheckStreamConnectable] Connect source failed : %s,%s,%s,%d\r\n",
                    iterStreamSource->strSourceUrl.c_str(),strIP.c_str(),strPort.c_str(),__LINE__));

                break;
            }
            else
            {
                ++n;
            }
        } 
        else
        {
            LOG_ERROR(("[BurnTask::CheckStreamConnectable] Get address failed : %s,%d\r\n",
                iterStreamSource->strSourceUrl.c_str(),__LINE__));

            break;
        }
    }
#endif

    return (n>0 && n==vecStreamSource.size());
}

void BurnTask::UpdateRtTaskPlaylist(std::string strTaskID,int nPauseCount)
{
    LOG_INFO(("[BurnTask::UpdateRtTaskPlaylist] Rt burn,update playlist file,%d\r\n",__LINE__))

    if (nPauseCount < 1)
    {
        return;
    }

    std::string strPlaylistPath;
    std::string strTsFileName;

#ifdef WIN32
#else
    {
        ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

        std::vector<BURN_PARAMETER>::iterator iter;
        for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();)
        {
            if (iter->strTaskID == strTaskID)
            {
                if ( !iter->mapBurnDataSource.empty() &&
                    iter->mapBurnDataSource.find("playlist") != iter->mapBurnDataSource.end() )
                {
                    std::map< std::string,std::vector<BURN_DATA_SOURCE> >::iterator iterTemp=iter->mapBurnDataSource.find("playlist");
                    if (1 == iterTemp->second.size())
                    {
                        strPlaylistPath=iterTemp->second.at(0).strSourceUrl;
                    }
                }

                strTsFileName=iter->strNewFileName;

                break;
            }
            else
            {
                ++iter;
            }
        }

        if (iter == m_vecBurnParameter.end())
        {
            LOG_ERROR(("[BurnTask::UpdateRtTaskPlaylist] Can not find task : %s,%d\r\n",
                strTaskID.c_str(),__LINE__));
        }
    }

    if (strPlaylistPath != "")
    {
        TiXmlDocument xmlDoc;

        bool bLoadFile=false;
        bLoadFile=xmlDoc.LoadFile(strPlaylistPath.c_str(),TIXML_ENCODING_LEGACY);

        if (bLoadFile)
        {
            LOG_INFO(("[BurnTask::UpdateRtTaskPlaylist] Load playlist file success,%d\r\n",__LINE__));

            TiXmlNode *pRoot=xmlDoc.RootElement();
            if (pRoot != NULL)
            {
                TiXmlNode *pView=NULL;
                for (pView=pRoot->FirstChildElement("view");pView!=NULL;pView=pView->NextSiblingElement("view"))
                {
                    for (int i = 1; i <= nPauseCount; ++i)
                    {
                        size_t nPos=strTsFileName.rfind('.');
                        if (nPos != std::string::npos)
                        {
                            std::string strFileName=strTsFileName;

                            std::string strTemp=std::string("_")+IntToString(i)+".";
                            strFileName.replace(nPos,1,strTemp.c_str());

                            TiXmlText xmlText(strFileName.c_str());

                            TiXmlElement xmlElement("url");
                            xmlElement.InsertEndChild(xmlText);

                            pView->InsertEndChild(xmlElement);
                        }
                    }
                    
                }//for
            }

#ifdef WIN32
#else
            if (!xmlDoc.SaveFile())
            {
                LOG_ERROR(("[BurnTask::UpdateRtTaskPlaylist] Save playlist file error : %s\r\n",strPlaylistPath.c_str()));
            }
#endif
        }
        else
        {
            LOG_ERROR(("[BurnTask::UpdateRtTaskPlaylist] Load playlist file error : %s\r\n",strPlaylistPath.c_str()));
        }
    }
#endif
}

bool BurnTask::StartSpecificDeviceToBurn(DEV_HANDLE hBurnDEV,std::vector<BURN_DATA_SOURCE> vecDataSource,
                                         BURN_MODE mode,BURN_PARAMETER burnParam)
{
    LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev hBurnDEV[%d]: %p start~~~\r\n", burnParam.nIndex, (void *)hBurnDEV));
    SetDevUsed(hBurnDEV,DEV_FLAG_USED_YES);

#if 1
    BURN_STATE_PARAMETER stateParam;

    if (MEDIAFILE_BURN_STREAM == mode)
    {
#ifdef WIN32
        assert(false);

        stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","winnort");;
        stateParam.strJobID=burnParam.strJobID;
        stateParam.strTaskID=burnParam.strTaskID;
        stateParam.strUpperIP=burnParam.strUpperIP;
        stateParam.strUpperPort=burnParam.strUpperPort;
        if (m_cb != NULL)
        {
            m_cb(hBurnDEV,stateParam,m_pUsr);
        }

        return false;
#else
        if(BURN_SUCCESS != Init_Stream_Subsystem(hBurnDEV))
        {
            LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Init_Stream_Subsystem failed\r\n",burnParam.nIndex,(void *)hBurnDEV));

            //如果不需要启动实时刻录补刻任务，那么应该删除掉补刻的task，其他位置同理
            SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

            stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","initfailed");;
            stateParam.strJobID=burnParam.strJobID;
            stateParam.strTaskID=burnParam.strTaskID;
            stateParam.strUpperIP=burnParam.strUpperIP;
            stateParam.strUpperPort=burnParam.strUpperPort;
            if (m_cb != NULL)
            {
                m_cb(hBurnDEV,stateParam,m_pUsr);
            }

            Clear_Burn_Stream_List(hBurnDEV);

            return false;
        }
        else
        {
            LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Init_Stream_Subsystem success\r\n",burnParam.nIndex,(void *)hBurnDEV));
        }
#endif
    }

    switch (mode)
    {
    case MEDIAFILE_BURN_LOCAL_FILE:
        {
            if(BURN_SUCCESS != Set_Local_File_Service_Mode(hBurnDEV))
            {
                LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Set_Local_File_Service_Mode failed,%d\r\n",
                    burnParam.nIndex,(void *)hBurnDEV, __LINE__));

                stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","setmodefailed");;
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                if (m_cb != NULL)
                {
                    m_cb(hBurnDEV,stateParam,m_pUsr);
                }

                return false;
            }
            else
            {
                LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Set_Local_File_Service_Mode success\r\n",burnParam.nIndex,(void*)hBurnDEV));
            }
        }
        break;

    case MEDIAFILE_BURN_STREAM:
        {
#ifdef WIN32
#else
            if(BURN_SUCCESS != Set_Stream_Service_Mode(hBurnDEV))
            {
                LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Set_Stream_Service_Mode failed,%d\r\n",
                    burnParam.nIndex,(void*)hBurnDEV,__LINE__));

                SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","setmodefailed");;
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                if (m_cb != NULL)
                {
                    m_cb(hBurnDEV,stateParam,m_pUsr);
                }

                Clear_Burn_Stream_List(hBurnDEV);
                return false;
            }
            else
            {
                LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Set_Service_Mode success\r\n",burnParam.nIndex,(void*)hBurnDEV));
            }

#endif
        }
        break;

    default:
        assert(false);
        break;
    }

    Burn_Set_Event_Callback(hBurnDEV,BurnCoreCallBack,this);

    int nDiscAlarmSize=burnParam.nAlarmSize;
    switch (mode)
    {
    case MEDIAFILE_BURN_LOCAL_FILE:
        break;

    case MEDIAFILE_BURN_STREAM:
        if (nDiscAlarmSize < DEFAULT_DISC_ALARM_SIZE)
        {
            nDiscAlarmSize=DEFAULT_DISC_ALARM_SIZE;
        }
        break;

    default:
        assert(false);
        break;
    }

    int nDiscAlarmSizeLimit = StringToInt(gBasicParamCfgFile.GetValue("info","burnServerDiscAlarmSize"));
    if (nDiscAlarmSize < nDiscAlarmSizeLimit)
    {
        nDiscAlarmSize = nDiscAlarmSizeLimit;
    }
    LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Alarm size : %d\r\n",burnParam.nIndex,(void*)hBurnDEV,nDiscAlarmSize));
    Burn_Set_DiscAlarmSize(hBurnDEV,nDiscAlarmSize);


    int nAlarmWarningSize = StringToInt(gBasicParamCfgFile.GetValue("info","burnServerDiscAlarmWarningSize"));
    LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Alarm warning size : %d\r\n",burnParam.nIndex,(void*)hBurnDEV,nAlarmWarningSize));
    Burn_Set_DiscAlarmWarningSize(hBurnDEV,nAlarmWarningSize);

    unsigned long ulBurnBufferSize=burnParam.ulBufferSize;
    LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p ulBurnBufferSize size : %d\r\n",burnParam.nIndex,(void*)hBurnDEV,ulBurnBufferSize));
    unsigned long ulBurnBufferSizeLimit = StringToInt(gBasicParamCfgFile.GetValue("info","burnServerBufferSize")) * 1024 *1024;
//     if (ulBurnBufferSize < ulBurnBufferSizeLimit)
    {
        ulBurnBufferSize = ulBurnBufferSizeLimit;
    }
    LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p set buffer size : %d MB\r\n",burnParam.nIndex,(void*)hBurnDEV,ulBurnBufferSize/1024/1024));

    //Buffer大小的设置需要在Create_Burn_Environment之前
    Set_Burn_Buffer_Size(hBurnDEV, ulBurnBufferSize);

    if(BURN_SUCCESS != Create_Burn_Environment(hBurnDEV))
    {
        LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Create_Burn_Environment failed\r\n",burnParam.nIndex,(void*)hBurnDEV));

        SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

        stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","createenv");;
        stateParam.strJobID=burnParam.strJobID;
        stateParam.strTaskID=burnParam.strTaskID;
        stateParam.strUpperIP=burnParam.strUpperIP;
        stateParam.strUpperPort=burnParam.strUpperPort;
        if (m_cb != NULL)
        {
            m_cb(hBurnDEV,stateParam,m_pUsr);
        }
#ifdef WIN32
#else
        if (mode == MEDIAFILE_BURN_STREAM)
        {
            Clear_Burn_Stream_List(hBurnDEV);
        }
#endif
        return false;
    }
    else
    {
        LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Create_Burn_Environment success\r\n",burnParam.nIndex,(void*)hBurnDEV));
    }

#ifdef WIN32
#else
#if 1
    //实时刻录设置光盘名称在格式化光盘前被调用
    if (burnParam.strDiscLabel != "")
    {
        Burn_Set_DiscName(hBurnDEV,const_cast<char *>(burnParam.strDiscLabel.c_str()));

        LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Disc label : %s,%d\r\n",
            burnParam.nIndex,(void*)hBurnDEV,burnParam.strDiscLabel.c_str(),__LINE__));
    }

#endif
#endif

    bool bAddDataSourceOK=false;
    int nIndex=0;
    switch (mode)
    {
    case MEDIAFILE_BURN_LOCAL_FILE:
        {
#ifdef WIN32
            CreateDataProject(hBurnDEV->hDVD);

            //windows下必须在CreateDataProject后调用
            if (burnParam.strDiscLabel != "")
            {
                Burn_Set_DiscName(hBurnDEV,const_cast<char *>(burnParam.strDiscLabel.c_str()));
            }
#else
#endif

            //添加事后刻录数据
            std::vector<BURN_DATA_SOURCE>::iterator iter;
            for (iter=vecDataSource.begin();iter!=vecDataSource.end();++iter)
            {
                if (SOURCE_TYPE_NORMAL_FILE == iter->strType)
                {
                    INT64 nFileSize=GetFileOrDirSize(iter->strSourceUrl,false);
                    if (IsFileOrDirExist(iter->strSourceUrl,false))
                    {
                        bAddDataSourceOK= ( BURN_SUCCESS == Burn_Ctrl_AddBurnLocalFile(
                            hBurnDEV,NULL,const_cast<char *>(iter->strSourceUrl.c_str())) );

                        if (bAddDataSourceOK)
                        {
                            LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Add file success : %s,%d,%d\r\n",
                                burnParam.nIndex,(void*)hBurnDEV,iter->strSourceUrl.c_str(),nFileSize,__LINE__));
                        }
                        else
                        {
                            LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Add file failed : %s,%d,%d\r\n",
                                burnParam.nIndex,(void*)hBurnDEV,iter->strSourceUrl.c_str(),nFileSize,__LINE__));
                        }
                    }
                    else
                    {
                        LOG_ERROR(("[[BurnTask::StartSpecificDeviceToBurn]] dev handle[%d] : %p File not found : %s,%d\r\n",
                            burnParam.nIndex,(void*) hBurnDEV,iter->strSourceUrl.c_str(),__LINE__));
                    }
                }
                else if(SOURCE_TYPE_DIR == iter->strType)
                {
                    INT64 nDirSize=GetFileOrDirSize(iter->strSourceUrl,true);
                    if (IsFileOrDirExist(iter->strSourceUrl,true))
                    {
                        std::string strDirPath=iter->strSourceUrl.c_str();
                        char chT;
                        if (strDirPath != "")
                        {
                            chT=strDirPath.at(strDirPath.length()-1);
                        }
                        if ('\\' == chT)
                        {
                            strDirPath.at(strDirPath.length()-1)='\0';
                        }

                        bAddDataSourceOK= ( BURN_SUCCESS == Burn_Ctrl_AddBurnLocalDir(
                            hBurnDEV,NULL,const_cast<char *>(strDirPath.c_str())));

                        if (bAddDataSourceOK)
                        {
                            LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Add dir success : %s,%d\r\n",
                                burnParam.nIndex,(void*)hBurnDEV,strDirPath.c_str(),nDirSize));
                        }
                        else
                        {
                            LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Add dir failed : %s,%d\r\n",
                                burnParam.nIndex,(void*)hBurnDEV,iter->strSourceUrl.c_str(),nDirSize));
                        }
                    }
                    else
                    {
                        LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Dir not found : %s,%d\r\n",
                            burnParam.nIndex,(void*)hBurnDEV,iter->strSourceUrl.c_str(),__LINE__));
                    }
                } 
                else
                {
                    LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Data source wrong,%d\r\n",burnParam.nIndex,(void*)hBurnDEV,__LINE__));
                    assert(false);
                }

                if (!bAddDataSourceOK)
                {
                    break;
                }
            }

        }
        break;



    case MEDIAFILE_BURN_STREAM:
        {
            //添加实时刻录数据
#ifdef WIN32
#else
            std::vector<BURN_DATA_SOURCE>::iterator iter;
            for (iter=vecDataSource.begin();iter!=vecDataSource.end();++iter)
            {
                if (SOURCE_TYPE_URL == iter->strType)
                {
                    //底层仅能刻录4路流
                    if (nIndex >= 4)
                    {
                        bAddDataSourceOK=false;
                        LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Add the 5th stream, donnot support!\r\n",burnParam.nIndex,(void*)hBurnDEV));
                        break;
                    }

                    //简单判断流地址是否正确
                    if ("" == iter->strSourceUrl)
                    {
                        bAddDataSourceOK=false;

                        break;
                    }

                    STREAM_INFO_T stream;
                    stream.stream_id    = nIndex;
                    stream.reconn_time  = 30;
                    stream.protocl_type = RTSP;
                    stream.ts_channel   = nIndex;
                    strcpy(stream.url,iter->strSourceUrl.c_str());

                    LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Add stream,stream id : %d, \
                              channel : %d,url : %s\r\n",
                        burnParam.nIndex,(void*)hBurnDEV,stream.stream_id,stream.ts_channel,stream.url));

                    bAddDataSourceOK= ( BURN_SUCCESS == Add_Stream_To_Burn(hBurnDEV,&stream) );

                    if (bAddDataSourceOK)
                    {
                        LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Add url success : %s\r\n",
                            burnParam.nIndex,(void*)hBurnDEV,iter->strSourceUrl.c_str()));
                    }
                    else
                    {
                        LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Add url failed : %s\r\n",
                            burnParam.nIndex,(void*)hBurnDEV,iter->strSourceUrl.c_str()));
                    }

                    ++nIndex;
                }
                else
                {
                    LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Data source wrong,%d\r\n",burnParam.nIndex,(void*)hBurnDEV,__LINE__));
                    assert(false);
                }

                if (!bAddDataSourceOK)
                {
                    break;
                }
            }
#endif
        }
        break;

    default:
        LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Unknown burn mode\r\n",burnParam.nIndex,(void*)hBurnDEV));
        assert(false);
        break;
    }

    if (!bAddDataSourceOK)
    {
        LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Add data source failed\r\n",burnParam.nIndex,(void*)hBurnDEV));

#ifdef WIN32
#else
        if (mode == MEDIAFILE_BURN_STREAM)
        {
            Clear_Burn_Stream_List(hBurnDEV);
        }
#endif

        SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

        stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","adddatafailed");
        if (nIndex >= 4)        
        {
            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","streamtoomuch");
        }
        stateParam.strJobID=burnParam.strJobID;
        stateParam.strTaskID=burnParam.strTaskID;
        stateParam.strUpperIP=burnParam.strUpperIP;
        stateParam.strUpperPort=burnParam.strUpperPort;

        if (m_cb != NULL)
        {
            m_cb(hBurnDEV,stateParam,m_pUsr);
        }

        return false;
    }

    //刻录是一个持续时间相对长的过程，当线程发送状态消息不按顺序到达时，正在刻录的状态就不能及时更新
    stateParam.strStateFlag=STATE_BURNING;
    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burning");
    stateParam.strJobID=burnParam.strJobID;
    stateParam.strTaskID=burnParam.strTaskID;

    stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;
    stateParam.strUpperIP=burnParam.strUpperIP;
    stateParam.strUpperPort=burnParam.strUpperPort;

    if (m_cb != NULL)
    {
        m_cb(hBurnDEV,stateParam,m_pUsr);
    }

#ifdef WIN32
#else
    //设置光驱状态为非录制状态
    Burn_Ctrl_SetRecordStatus(hBurnDEV, FALSE);
    if(BURN_SUCCESS != Burn_Ctrl_Dev_FormatDisc(hBurnDEV))
    {
        LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Burn_Ctrl_Dev_FormatDisc failed,%d\r\n",
            burnParam.nIndex,(void*)hBurnDEV,__LINE__));

        SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

        stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","formatdiscfailed");;
        stateParam.strJobID=burnParam.strJobID;
        stateParam.strTaskID=burnParam.strTaskID;
        stateParam.strUpperIP=burnParam.strUpperIP;
        stateParam.strUpperPort=burnParam.strUpperPort;
        if (m_cb != NULL)
        {
            m_cb(hBurnDEV,stateParam,m_pUsr);
        }

        if (mode == MEDIAFILE_BURN_STREAM)
        {
            Clear_Burn_Stream_List(hBurnDEV);
        }

        Burn_Ctrl_LockDoor(hBurnDEV, FALSE);
        return false;
    }
    else
    {
        LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Burn_Ctrl_Dev_FormatDisc success\r\n",burnParam.nIndex,(void*)hBurnDEV));
    }
#endif

    switch (mode)
    {
    case MEDIAFILE_BURN_LOCAL_FILE:
        {
            if(BURN_SUCCESS != Start_Local_File_Burning(hBurnDEV))
            {
                LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Start_Local_File_Burning failed\r\n",burnParam.nIndex,(void*)hBurnDEV));

                stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","startburnfailed");;
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;

                if (m_cb != NULL)
                {
                    m_cb(hBurnDEV,stateParam,m_pUsr);
                }
#ifdef LINUX
                Burn_Ctrl_LockDoor(hBurnDEV, FALSE);
#endif
                return false;
            }
        }
        break;

    case MEDIAFILE_BURN_STREAM:
        {
            //11223344
#ifdef WIN32
#else
            int nTaskControlState=GetTaskControlState(burnParam.strTaskID);

            if (TASK_CONTROL_STATE_PAUSE == nTaskControlState)
            {
                stateParam.strStateFlag=STATE_RTTASK_PAUSED;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","paused");
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                if (m_cb != NULL)
                {
                    DEV_HANDLE handle=GetCurDev(burnParam.strTaskID);
                    m_cb(handle,stateParam,m_pUsr);
                }

                //暂停时循环等待
                nTaskControlState=GetTaskControlState(burnParam.strTaskID);
                while (nTaskControlState == TASK_CONTROL_STATE_PAUSE)
                {
                    ZOSThread::Sleep(4000);
                }

                if (nTaskControlState == TASK_CONTROL_STATE_START_OR_RESUME)
                {
                    //恢复为刻录流的状态，表示上层恢复实时刻录
                }
                else if (nTaskControlState == TASK_CONTROL_STATE_STOP)
                {
                    //停止实时刻录

                    SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                    SetTaskRunningState(burnParam.strTaskID,TASK_STATE_STOPPED);

                    stateParam.strStateFlag=STATE_RTJOB_STOPPED;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopped");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;
                    stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                    if (m_cb != NULL)
                    {
                        DEV_HANDLE handle=GetCurDev(burnParam.strTaskID);
                        m_cb(handle,stateParam,m_pUsr);
                    }

                    Burn_Ctrl_LockDoor(hBurnDEV, FALSE);
                    return false;
                }
            }

            //应对有时实时刻录会出现的没有刻录进去内容的情况
            ZOSThread::Sleep(10000);


            if (TASK_CONTROL_STATE_STOP == nTaskControlState)
            {
                SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                SetTaskRunningState(burnParam.strTaskID,TASK_STATE_STOPPED);

                stateParam.strStateFlag=STATE_RTTASK_STOPPED;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopped");
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                if (m_cb != NULL)
                {
                    DEV_HANDLE handle=GetCurDev(burnParam.strTaskID);
                    m_cb(handle,stateParam,m_pUsr);
                }
                Burn_Ctrl_LockDoor(hBurnDEV, FALSE);
                return false;
            }

            if (burnParam.strNewFileName != "")
            {
                Burn_Set_RT_File_Name(const_cast<char *>(burnParam.strNewFileName.c_str()));
            }

            int nPauseCount=0;
            while (true)
            {
                stateParam.strStateFlag=STATE_BURNING;
                stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burning");
                stateParam.strJobID=burnParam.strJobID;
                stateParam.strTaskID=burnParam.strTaskID;
                stateParam.strUpperIP=burnParam.strUpperIP;
                stateParam.strUpperPort=burnParam.strUpperPort;
                stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                if (m_cb != NULL)
                {
                    DEV_HANDLE handle=GetCurDev(burnParam.strTaskID);
                    m_cb(handle,stateParam,m_pUsr);
                }

                //设置实时刻录为刻录stream的状态，很重要，用于判断是否可以安全停止实时刻录流
                SetTaskRunningState(burnParam.strTaskID,TASK_STATE_BURN_STREAM);

                LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Will start burn stream,%d\r\n",burnParam.nIndex,(void*)hBurnDEV,__LINE__));

                if (BURN_SUCCESS != Start_Burning_State_WatchDog(hBurnDEV))
                {
                    LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Start_Burning_State_WatchDog failed,%d\r\n",
                        burnParam.nIndex,(void*)hBurnDEV,__LINE__));

                    SetRTBurnBackupTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                    stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","watchfailed");;
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;
                    if (m_cb != NULL)
                    {
                        m_cb(hBurnDEV,stateParam,m_pUsr);
                    }

                    Clear_Burn_Stream_List(hBurnDEV);
                    Burn_Ctrl_LockDoor(hBurnDEV, FALSE);
                    return false;
                }
		printf("now ready to start stream Burning\n!!!!!!!!!!!!");
		sleep(2);
                if(BURN_SUCCESS != Start_Stream_Burning(hBurnDEV))
                {
//                     SetTaskRunningState(burnParam.strTaskID,TASK_STATE_NO_USE);

                    //此时应该通知BurnControl进行补刻

                    LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Start_Stream_Burning failed,\
                               start burn backup,%d\r\n",burnParam.nIndex,(void*)hBurnDEV,__LINE__));

                    stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","startburnfailed");;
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;
#if 0
                    //暂不支持补刻
                    stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;
                    stateParam.strExtraStateFlag=EXTRA_STATE_RTBURN_BACKUP_YES;

                    //保存用于刻录备份的任务文件
                    SaveRTBurnBackupTask(burnParam);
#endif

                    if (m_cb != NULL)
                    {
                        m_cb(hBurnDEV,stateParam,m_pUsr);
                    }

                    Clear_Burn_Stream_List(hBurnDEV);
                    Burn_Ctrl_LockDoor(hBurnDEV, FALSE);
                    return false;
                }
                else
                {
                    LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Start_Stream_Burning break out,%d\r\n",burnParam.nIndex,(void*)hBurnDEV,__LINE__));

                    int nTaskControlState=GetTaskControlState(burnParam.strTaskID);

                    LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p GetTaskControlState return %d\r\n",burnParam.nIndex,(void*)hBurnDEV,nTaskControlState));

                    if (TASK_CONTROL_STATE_PAUSE == nTaskControlState)
                    {
                        ++nPauseCount;

                        SetTaskRunningState(burnParam.strTaskID,TASK_STATE_PAUSED);

                        stateParam.strStateFlag=STATE_RTTASK_PAUSED;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","paused");
                        stateParam.strJobID=burnParam.strJobID;
                        stateParam.strTaskID=burnParam.strTaskID;
                        stateParam.strUpperIP=burnParam.strUpperIP;
                        stateParam.strUpperPort=burnParam.strUpperPort;
                        stateParam.strTaskPropertyFlag=burnParam.strTaskPropertyFlag;

                        if (m_cb != NULL)
                        {
                            DEV_HANDLE handle=GetCurDev(burnParam.strTaskID);
                            m_cb(handle,stateParam,m_pUsr);
                        }
                    }
                    while (TASK_CONTROL_STATE_PAUSE == nTaskControlState)
                    {
                        ZOSThread::Sleep(4000);

                        nTaskControlState=GetTaskControlState(burnParam.strTaskID);
                    }

                    if (TASK_CONTROL_STATE_START_OR_RESUME == nTaskControlState)
                    {
                        if (burnParam.strNewFileName != "")
                        {
                            std::string strTsFileName=burnParam.strNewFileName;
                            size_t nPos=strTsFileName.rfind('.');
                            if (nPos != std::string::npos)
                            {
                                std::string strTemp=std::string("_")+IntToString(nPauseCount)+".";
                                if (nPauseCount > 0)
                                {
                                    strTsFileName.replace(nPos,1,strTemp.c_str());

                                    Burn_Set_RT_File_Name(const_cast<char *>(strTsFileName.c_str()));
                                    LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Pause number: %d, newFileName: %s, line number: %d\r\n",burnParam.nIndex,(void*)hBurnDEV, nPauseCount, strTsFileName.c_str(), __LINE__));
                                }
                            }
                        }

                        continue;
                    }

                    //停止实时刻录后，修正playlist文件，每一张光盘可能包含的playlist文件不同
                    LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %pWill update playlist file,%d\r\n",burnParam.nIndex,(void*)hBurnDEV,__LINE__));
                    UpdateRtTaskPlaylist(burnParam.strTaskID,nPauseCount);

                    Clear_Burn_Stream_List(hBurnDEV);

                    break;
                }
            }//while
#endif
        }
        break;

    default:
        LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Unknown burn mode\r\n",burnParam.nIndex,(void*)hBurnDEV));
        assert(false);
        break;
    }

    if (MEDIAFILE_BURN_LOCAL_FILE == mode)
    {
        Burn_File_Form_Local_File(hBurnDEV, NULL, "/root/README");
        Burn_File_Form_Local_File(hBurnDEV, NULL, "/root/README1");
        Burn_File_Form_Local_File(hBurnDEV, NULL, "/root/README2");
        Burn_File_Form_Local_File(hBurnDEV, NULL, "/root/README3");


        if (BURN_SUCCESS != Burn_Ctrl_CloseDisc(hBurnDEV))
        {
            LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Burn_Ctrl_CloseDisc failed\r\n",burnParam.nIndex,(void*)hBurnDEV));

            stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","closediscfailed");
            stateParam.strJobID=burnParam.strJobID;
            stateParam.strTaskID=burnParam.strTaskID;
            stateParam.strUpperIP=burnParam.strUpperIP;
            stateParam.strUpperPort=burnParam.strUpperPort;

            if (m_cb != NULL)
            {
                m_cb(hBurnDEV,stateParam,m_pUsr);
            }

            return false;
        }


#ifdef WIN32
        //Windows刻录，当插入较多光驱时，会发生没有刻录数据立即返回刻录成功的情况
        //规避方法：每次刻录完成后，检查光盘里面是否有内容，如果没有，那么认定为刻录失败

        int nCheckDiscInfoCount=0;

        BURNHANDLE hWinBurnDev=hBurnDEV->hDVD;
        int nWinBurnDevINdex=hBurnDEV->dev_id;
        LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] Dev handle : %p,dev index : %d,%d\r\n",
            (void *)hWinBurnDev,nWinBurnDevINdex,__LINE__));

        while (true)
        {
            DISC_INFORMATION discInfo;


            if (GetDiscInfo(hWinBurnDev,nWinBurnDevINdex,discInfo))
            {
                LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle : %p Get disc information success\r\n",(void*)hWinBurnDev));

                unsigned __int64 nUsedBytes=discInfo.uiUsedBytes;
                if (0 == nUsedBytes)
                {
                    stateParam.strStateFlag=STATE_SINGLE_DEV_ERR;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","burnfailed");
                    stateParam.strJobID=burnParam.strJobID;
                    stateParam.strTaskID=burnParam.strTaskID;
                    stateParam.strUpperIP=burnParam.strUpperIP;
                    stateParam.strUpperPort=burnParam.strUpperPort;

                    if (m_cb != NULL)
                    {
                        m_cb(hBurnDEV,stateParam,m_pUsr);
                    }

                    return false;
                }

                break;
            }
            else
            {
                ++nCheckDiscInfoCount;

                LOG_ERROR(("[BurnTask::StartSpecificDeviceToBurn] dev handle : %p Get disc information failed, try time %d\r\n",(void*)hWinBurnDev,nCheckDiscInfoCount));

                if (nCheckDiscInfoCount > 4)
                {
                    return false;
                }
            }
        }
#endif

    }
#endif

    LOG_INFO(("[BurnTask::StartSpecificDeviceToBurn] dev handle[%d] : %p Success,%d\r\n",burnParam.nIndex,(void*)hBurnDEV,__LINE__));

    return true;
}

void BurnTask::CleanRTBurnBackupTask(std::string strFatherTaskID)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();)
    {
        if (iter->strTaskID != strFatherTaskID)
        {
            if (iter->strTaskID.find(strFatherTaskID) != std::string::npos)
            {
                LOG_INFO(("[BurnTask::CleanRTBurnBackupTask] Task %s is cleaned,%d\r\n",
                    iter->strTaskID.c_str(),__LINE__));

                iter=m_vecBurnParameter.erase(iter);

                ListLeftTasks();

                break;
            } 
            else
            {
                ++iter;
            }
        }
        else
        {
            ++iter;
        }
    }
}

void BurnTask::SetNoteUpdateFlag(std::string strJobID,std::string strFlag)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    bool bJobFound=false;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        if (strJobID == iter->strJobID)
        {
            bJobFound=true;

            iter->strRtNoteUpdateFlag=strFlag;
        }
    }

    if (!bJobFound)
    {
        LOG_ERROR(("[BurnTask::SetNoteUpdateFlag] Can not find job : %s\r\n",
            strJobID.c_str()));;
        assert(false);
    }
}

std::string BurnTask::GetNoteUpdateFlag(std::string strTaskID)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::string strFlag=NOTE_FILE_UPDATED_NO;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        //LOG_INFO(("[BurnTask::GetNoteUpdateFlag] Task id : %s\r\n",iter->strTaskID.c_str()));

        if (strTaskID == iter->strTaskID)
        {
            if (NOTE_FILE_UPDATED_YES == iter->strRtNoteUpdateFlag)
            {
                strFlag=NOTE_FILE_UPDATED_YES;
            }

            break;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        LOG_ERROR(("[BurnTask::GetNoteUpdateFlag] Can not find task %s\r\n",
            strTaskID.c_str()));

        assert(false);
    }

    return strFlag;
}

std::string BurnTask::GetBurningTaskID(DEV_HANDLE handle)
{
    if (NULL == handle)
    {
        return "";
    }

    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::string strTaskID;
    std::vector<std::string> vecTaskID;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin(); iter!=m_vecBurnParameter.end();
        ++iter)
    {
        std::vector<DEV_HANDLE> vecHandle=iter->vecDevHandle;
        if (!vecHandle.empty() &&
            std::find (vecHandle.begin(),vecHandle.end(),handle) != vecHandle.end())
        {
            if (vecHandle.at(iter->nIndex) == handle && iter->strBurnDevFlag == DEV_FLAG_USED_YES)
            {
                strTaskID=iter->strTaskID;

                if (GetTaskRunningState(strTaskID) != TASK_STATE_NO_USE)
                {
                    //LOG_INFO(("[BurnTask::GetTaskID] Task id : %s,handle : %p,%d\r\n",
                    //    iter->strTaskID.c_str(),handle,__LINE__));

                    vecTaskID.push_back(strTaskID);
                }
                else
                {
                    //LOG_WARNING(("[BurnTask::GetTaskID] Task %s,state no use,%d\r\n",
                    //    strTaskID.c_str(),__LINE__));
                }
            }
        }
    }

    //实时刻录时，刻录备份task和父级task包含相同的设备句柄，由于两种task不可能同时被执行，
    //而且，如果需要返回备份task时，父级task的状态必须是NO USE，因此优先返回父级task的id
    int nTaskIDCount=vecTaskID.size();

    assert(nTaskIDCount <= 2);

    if (2 == nTaskIDCount)
    {
        if (vecTaskID.at(0).length() < vecTaskID.at(1).length())
        {
            strTaskID=vecTaskID.at(0);
        }
        else
        {
            strTaskID=vecTaskID.at(1);
        }
    }
    else if (1 == nTaskIDCount)
    {
        strTaskID=vecTaskID.at(0);
    }

    return strTaskID;
}

std::string BurnTask::GetBurningJobID(DEV_HANDLE handle)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::string strJobID;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        std::vector<DEV_HANDLE> vecHandle=iter->vecDevHandle;
        if (vecHandle.at(iter->nIndex) == handle && iter->strBurnDevFlag == DEV_FLAG_USED_YES)
        {
            strJobID=iter->strJobID;
            break;
        }
    }

    return strJobID;
}

void BurnTask::SetNoteUpdateFlagByTaskID(std::string strTaskID,std::string strFlag)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        if (strTaskID == iter->strTaskID)
        {
            iter->strRtNoteUpdateFlag=strFlag;

            break;
        }
    }
}

int BurnTask::PauseRTTasksByJobID(std::string strJobID)
{
    int nRet=0;

    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    bool bTaskFound=false;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin(); iter!=m_vecBurnParameter.end();
        ++iter)
    {
        if (strJobID == iter->strJobID &&
            TASK_PROPERTY_RT == iter->strTaskPropertyFlag)
        {
            bTaskFound=true;

            LOG_INFO(("[BurnTask::PauseRTTasksByJobID] Before PauseRTBurnTask %s,%d\r\n",
                iter->strTaskID.c_str(),__LINE__));

            PauseRTBurnTask(iter->strTaskID);
        }
    }

    if (!bTaskFound)
    {
        LOG_WARNING(("[BurnTask::PauseRTTasksByJobID] Find no task with jobID %s,%d\r\n",
            strJobID.c_str(),__LINE__));

        nRet=1;
    }

    return nRet;
}

int BurnTask::StopRTJob(std::string strJobID)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    int nRet=0;
    bool bTaskFound=false;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {

        if (strJobID == iter->strJobID &&
            MEDIAFILE_BURN_STREAM == iter->burnMode)//确保BurnTask::StopRTJob不直接干预刻录备份task的生命周期
        {
            StopRTBurnTask(iter->strTaskID);

            bTaskFound=true;
        }
    }

    if (!bTaskFound)
    {
        //没有找到指定job的task，因此可以认为job的所有task已经停止
        LOG_WARNING(("[BurnTask::StopRTJob] Find no task with jobID %s,%d\r\n",
            strJobID.c_str(),__LINE__));
        nRet=1;
    }

    return nRet;
}

int BurnTask::ResumeRTJob(std::string strJobID)
{
    int nRet=0;

    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    bool bTaskFound=false;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin(); iter!=m_vecBurnParameter.end();
        ++iter)
    {
        if (strJobID == iter->strJobID &&
            TASK_PROPERTY_RT == iter->strTaskPropertyFlag)
        {
            bTaskFound=true;

            if (TASK_STATE_PAUSED == GetTaskRunningState(iter->strTaskID))
            {
                LOG_INFO(("[BurnTask::ResumeRTJob] Before ResumeRTBurnTask,%d\r\n",__LINE__));

                ResumeRTBurnTask(iter->strTaskID);
            }
            else
            {
                nRet = -1;

                LOG_ERROR(("[BurnTask::ResumeRTJob] Task: %s with jobID %s is not paused,%d\r\n",
                    iter->strTaskID.c_str(),strJobID.c_str(),__LINE__));
            }
        }
    }

    if (!bTaskFound)
    {
        LOG_WARNING(("[BurnTask::ResumeRTJob] Find no task with jobID %s,%d\r\n",
            strJobID.c_str(),__LINE__));

        nRet=1;
    }

    return nRet;
}

int BurnTask::GetTaskControlState(std::string strRtTaskID)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    int nRet=TASK_CONTROL_STATE_UNKNOWN;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        if (iter->strTaskID == strRtTaskID)
        {
            nRet=iter->nTaskControlCmd;

            break;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        LOG_WARNING(("[BurnTask::GetTaskControlState] Can not find task : %s\r\n",
            strRtTaskID.c_str()));
    }

    return nRet;
}

void BurnTask::SetTaskControlState(std::string strRtTaskID,int nState)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();)
    {
        if (iter->strTaskID == strRtTaskID)
        {
            iter->nTaskControlCmd=nState;

            break;
        }
        else
        {
            ++iter;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        LOG_ERROR(("[BurnTask::SetTaskControlState] Can not find task : %s\r\n",
            strRtTaskID.c_str()));

        assert(false);
    }
}

void BurnTask::SetTaskRunningState(std::string strRtTaskID,int nState)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();)
    {
        if (iter->strTaskID == strRtTaskID)
        {
            iter->nTaskRunningFlag=nState;

            break;
        }
        else
        {
            ++iter;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        //当一个task停止或者出错，在反馈的回调函数里面，可能已经清理掉了一个job下所有tasks
        //所以再设置task为废弃状态，想通过HandleTask删除时，已经找不到这个task了

        LOG_WARNING(("[BurnTask::SetTaskRunningState] Can not find task : %s, nState: %d %d\r\n",
            strRtTaskID.c_str(),nState,__LINE__));
    }
}

void BurnTask::SetRTBurnBackupTaskRunningState(std::string strFatherTaskID,int nState)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        if (iter->strTaskID != strFatherTaskID)
        {
            if (iter->strTaskID.find(strFatherTaskID) != std::string::npos)
            {
                iter->nTaskRunningFlag=nState;
            }
        }
    }
}

int BurnTask::GetTaskRunningState(std::string strRtTaskID)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    int nRet=-1;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();)
    {
        if (iter->strTaskID == strRtTaskID)
        {
            nRet=iter->nTaskRunningFlag;

            break;
        }
        else
        {
            ++iter;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        LOG_ERROR(("[BurnTask::GetTaskRunningState] Can not find task : %s\r\n",
            strRtTaskID.c_str()));

        assert(false);
    }

    return nRet;
}

int BurnTask::GetTaskExtraFlag(std::string strTaskID)
{
    int nState = TASK_EXTRA_STATE_DEFAULT;

    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();)
    {
        if (iter->strTaskID == strTaskID)
        {
            nState=iter->nTaskExtraFlag;

            break;
        }
        else
        {
            ++iter;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        LOG_ERROR(("[BurnTask::GetTaskExtraFlag] Can not find task : %s,%d\r\n",
            strTaskID.c_str(),__LINE__));
    }

    return nState;
}

void BurnTask::SetTaskExtraFlag(std::string strTaskID, int nState)
{
    LOG_INFO(("[BurnTask::SetTaskExtraFlag] strTaskID: %s, nState: %d, %d.\r\n",
        strTaskID.c_str(), nState, __LINE__));
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();)
    {
        if (iter->strTaskID == strTaskID)
        {
            iter->nTaskExtraFlag=nState;

            break;
        }
        else
        {
            ++iter;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        LOG_ERROR(("[BurnTask::SetTaskExtraFlag] Can not find task : %s,%d\r\n",
            strTaskID.c_str(),__LINE__));
    }
}

void BurnTask::ActivateRTBurnBackupTasks(std::string strJobID)
{
#ifdef RT_BURN_DISK_BACKUP
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        LOG_INFO(("[BurnTask::ActivateRTBurnBackupTasks] Job id : %s,task id : %s,%d\r\n",
            iter->strJobID.c_str(),iter->strTaskID.c_str(),__LINE__));

        if (iter->strJobID == strJobID)
        {
            if (TASK_STATE_BURN_BACKUP_DRIFT == iter->nTaskRunningFlag)
            {
                //对应刻录流数据时得到刻录备份的肯定回答的情况的处理方法，在收到停止实时刻录时，会将状态继续提升
                iter->nTaskRunningFlag=TASK_STATE_BURN_BACKUP_SLEEP;
            } 
            else if(TASK_STATE_BURN_BACKUP_SLEEP == iter->nTaskRunningFlag)
            {
                //对应封盘失败的task的处理方法
                iter->nTaskRunningFlag=TASK_STATE_BURN_BACKUP_READY;
            }
        }
    }
#endif
}

void BurnTask::CancelRTBurnBackupTasks(std::string strJobID)
{
#ifdef RT_BURN_DISK_BACKUP
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    bool bHasRunningBackupTask=false;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        if (iter->strJobID == strJobID &&
            iter->burnMode == MEDIAFILE_BURN_LOCAL_FILE)
        {
            int n=iter->nTaskRunningFlag;
            if (TASK_STATE_BURN_BACKUP_READY == n ||
                TASK_STATE_RUNNING == n)
            {
                bHasRunningBackupTask=true;

                break;
            }
        }
    }

    if (!bHasRunningBackupTask)
    {
        for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
        {
            if (iter->strJobID == strJobID &&
                iter->burnMode == MEDIAFILE_BURN_LOCAL_FILE)
            {
                iter->nTaskRunningFlag=TASK_STATE_NO_USE;

            }
        }

        std::string strRTJobBackupDir=Path(gBasicParamCfgFile.GetValue("info","burnServerRTBackupDir"), true)+strJobID;
        if (DirectoryUtil::IsDirExist(strRTJobBackupDir.c_str()))
        {
            LOG_INFO(("[BurnTask::CleanJobData] Will delete dir : %s,%d\r\n",
                strRTJobBackupDir.c_str(),__LINE__));

            DirectoryUtil::DeleteDir(strRTJobBackupDir.c_str());
        }
    }

#endif
}

void BurnTask::StartRTBurnBackupTasks(std::string strJobID)
{
#ifdef RT_BURN_DISK_BACKUP
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        if (iter->strJobID == strJobID)
        {
            if (TASK_STATE_BURN_BACKUP_SLEEP == iter->nTaskRunningFlag)
            {
                iter->nTaskRunningFlag=TASK_STATE_BURN_BACKUP_READY;

                LOG_INFO(("[BurnTask::StartRTBurnBackupTasks] "
                    "Set rt burn backup TASK_STATE_BURN_BACKUP_READY,job id %s,task id : %s\r\n",
                    strJobID.c_str(),iter->strTaskID.c_str()));
            } 
        }
    }
#endif
}

bool BurnTask::HasRTBurnBackupTask(std::string strJobID)
{
    bool bRet=false;

#ifdef RT_BURN_DISK_BACKUP
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        if (iter->strJobID == strJobID &&
            TASK_STATE_BURN_BACKUP_SLEEP == iter->nTaskRunningFlag)
        {
            bRet=true;

            break;
        }
    }
#endif

    return bRet;
}


void BurnTask::PauseRTBurnTask(std::string strTaskID)
{
#ifdef WIN32
#else
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    BURN_STATE_PARAMETER stateParam;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin(); iter != m_vecBurnParameter.end();
        ++iter)
    {
        if (iter->strTaskID == strTaskID &&
            TASK_PROPERTY_RT == iter->strTaskPropertyFlag)
        {
            int nRtTaskRunningState=GetTaskRunningState(strTaskID);
            switch (nRtTaskRunningState)
            {
            case TASK_STATE_PAUSED:
                break;

            case TASK_STATE_READY_TO_RUN:
            case TASK_STATE_RUNNING:
                {
                    LOG_INFO(("[BurnTask::PauseRTBurnTask] Set control state TASK_CONTROL_STATE_PAUSE,%d\r\n",
                        __LINE__));

                    iter->nTaskControlCmd=TASK_CONTROL_STATE_PAUSE;
                }
                break;

            case TASK_STATE_STOPPED:
                break;

            case TASK_STATE_BURN_LOCAL:
                break;

            case TASK_STATE_BURN_STREAM:
                {
                    DEV_HANDLE handle=GetCurDev(strTaskID);

                    assert(handle != NULL);

                    LOG_INFO(("[BurnTask::PauseRTBurnTask] Set control state TASK_CONTROL_STATE_PAUSE,%d\r\n",
                        __LINE__));

                    iter->nTaskControlCmd=TASK_CONTROL_STATE_PAUSE;

                    if (Stop_Stream_Burning(handle) == BURN_SUCCESS)
                    {
                        LOG_INFO(("[BurnTask::PauseRTBurnTask] Stop burn stream success,%d\r\n",
                            __LINE__));
                    } 
                    else
                    {
                        LOG_ERROR(("[BurnTask::PauseRTBurnTask] Stop burn stream failed,%d\r\n",
                            __LINE__));
                    }
                }
                break;

            case TASK_STATE_ERR:
                break;

            case TASK_STATE_NO_USE:
                break;

            default:
                break;
            }

            break;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        LOG_ERROR(("[BurnTask::PauseRTBurnTask] Can not find task : %s,%d\r\n",
            strTaskID.c_str(),__LINE__));
    }
#endif
}

bool IsFileOrDirExist(std::string str,bool bFolder)
{
    bool bRet=false;

#ifdef WIN32
    if (bFolder)
    {
        bRet=DirectoryUtil::IsDirExistW(CharsetConvertMFC::UTF8ToUTF16(str.c_str()));
    }
    else
    {
        bRet=FileUtil::FileExistW(CharsetConvertMFC::UTF8ToUTF16(str.c_str()));
    }
#else
    if (bFolder)
    {
        bRet=DirectoryUtil::IsDirExist(str.c_str());
    }
    else
    {
        bRet=FileUtil::FileExist(str.c_str());
    }
#endif

    if (bRet)
    {
        INT64 nSize=0;


#ifdef WIN32
        if (bFolder)
        {
            nSize=DirectoryUtil::GetDirSizeW(CharsetConvertMFC::UTF8ToUTF16(str.c_str()));
        } 
        else
        {
            nSize=FileUtil::FileSizeW(CharsetConvertMFC::UTF8ToUTF16(str.c_str()));
        }
#else
        if (bFolder)
        {
            nSize=DirectoryUtil::GetDirSize(str.c_str());
        } 
        else
        {
            nSize=FileUtil::FileSize(str.c_str());
        }
#endif

#ifdef WIN32
        LOG_INFO(("[IsFileOrDirExist] %s,size : %l64d\r\n",str.c_str(),nSize));
#else
        LOG_INFO(("[IsFileOrDirExist] %s,size : %lld\r\n",str.c_str(),nSize));
#endif
    }

    return bRet;
}

INT64 GetFileOrDirSize(std::string str,bool bFolder)
{
    INT64 nRet=0;

    if (IsFileOrDirExist(str,bFolder))
    {
#ifdef WIN32
        if (bFolder)
        {
            nRet=DirectoryUtil::GetDirSizeW(CharsetConvertMFC::UTF8ToUTF16(str.c_str()));
        } 
        else
        {
            nRet=FileUtil::FileSizeW(CharsetConvertMFC::UTF8ToUTF16(str.c_str()));
        }
#else
        if (bFolder)
        {
            nRet=DirectoryUtil::GetDirSize(str.c_str());
        } 
        else
        {
            nRet=FileUtil::FileSize(str.c_str());
        }
#endif
    }

    return nRet;
}

std::string RenameFile(const char *pOldFilePath,const char *pNewFileNameNoExtent,const char *pExtent)
{
    std::string strRet;

    std::string strOldFilePath=pOldFilePath;
    std::string strNewFilePath;

    std::string strOldFileName;
    std::string strNewFileName=pNewFileNameNoExtent;

#ifdef WIN32
#else
    size_t nPosition;

    nPosition=strOldFilePath.rfind('/');
    if (nPosition != std::string::npos)
    {
        std::string strRelativePath=std::string(strOldFilePath.begin(),strOldFilePath.begin()+nPosition+1);
        strOldFileName=std::string(strOldFilePath.begin()+nPosition+1,strOldFilePath.end());

        std::string strExtent;
        if (pExtent != NULL)
        {
            strExtent=pExtent;
        }
        else
        {
            nPosition=strOldFileName.rfind('.');
            if (nPosition != std::string::npos)
            {
                strExtent=std::string(strOldFileName.begin()+nPosition,strOldFileName.end());
            }
        }

        strNewFilePath=strRelativePath+strNewFileName+strExtent;

        if(0 == rename(pOldFilePath,strNewFilePath.c_str()))
        {
            strRet=strNewFilePath;
        }
    }
#endif

    return strRet;
}

bool BurnTask::GetDevUsed(DEV_HANDLE handle)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::string strBurnDevFlag;
    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        std::vector<DEV_HANDLE> &vecDevHandle=iter->vecDevHandle;
        size_t nDevCount=vecDevHandle.size();
        for (size_t i=0;i<nDevCount;++i)
        {
            if (vecDevHandle.at(i) == handle)
            {
                strBurnDevFlag=iter->strBurnDevFlag;
            }
        }
    }

    return ( strBurnDevFlag ==DEV_FLAG_USED_YES );
}

void BurnTask::SetDevUsed(DEV_HANDLE handle,std::string strBurnDevFlag)
{
    LOG_INFO(("[BurnTask::SetDevUsed] handle: %p, strBurnDevFlag: %s\r\n", (void *)handle, strBurnDevFlag.c_str()));
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        std::vector<DEV_HANDLE> &vecDevHandle=iter->vecDevHandle;
        if (iter->nIndex >= 0 && iter->nIndex < (int)vecDevHandle.size())
        {
            if (vecDevHandle.at(iter->nIndex) == handle)
            {
                iter->strBurnDevFlag=strBurnDevFlag;
                break;
            }
        }
        else
        {
            LOG_ERROR(("[BurnTask::SetDevUsed]nIndex: %d, vecDevHandle.size(): %d, Out of range!\r\n", iter->nIndex, (int)vecDevHandle.size()));
        }
    }
}

void BurnTask::StopRTBurnTask(std::string strTaskID)
{
#ifdef WIN32
#else
    LOG_INFO(("[BurnTask::StopRTBurnTask] Begin!! strTaskID: %s\r\n",strTaskID.c_str()))
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    BURN_STATE_PARAMETER stateParam;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin(); iter != m_vecBurnParameter.end();
        ++iter)
    {
        if (iter->strTaskID == strTaskID)
        {
            iter->nTaskControlCmd=TASK_CONTROL_STATE_STOP;

            DEV_HANDLE handle=GetCurDev(strTaskID);
            assert(handle != NULL);

            stateParam.strStateFlag=STATE_RTTASK_STOPPING;
            stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopping");
            stateParam.strJobID=iter->strJobID;
            stateParam.strTaskID=iter->strTaskID;
            stateParam.strUpperIP=iter->strUpperIP;
            stateParam.strUpperPort=iter->strUpperPort;
            stateParam.strTaskPropertyFlag=iter->strTaskPropertyFlag;

            if (m_cb != NULL)
            {
                m_cb(handle,stateParam,m_pUsr);
            }

            int nRtTaskRunningState=GetTaskRunningState(strTaskID);
            switch (nRtTaskRunningState)
            {
            case TASK_STATE_PAUSED:
                {
                    stateParam.strStateFlag=STATE_WILL_CLOSE_DISC;
                    stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopping");
                    stateParam.strTaskID=iter->strTaskID;
                    stateParam.strJobID=iter->strJobID;
                    stateParam.strUpperIP=iter->strUpperIP;
                    stateParam.strUpperPort=iter->strUpperPort;
                    stateParam.strTaskPropertyFlag=iter->strTaskPropertyFlag;

                    if (m_cb != NULL)
                    {
                        m_cb(handle,stateParam,m_pUsr);
                    }
                    break;
                }

            case TASK_STATE_READY_TO_RUN:
            case TASK_STATE_RUNNING:
                break;

            case TASK_STATE_STOPPED:
                break;

            case TASK_STATE_BURN_LOCAL:
                break;

            case TASK_STATE_BURN_STREAM:
                {
                    LOG_INFO(("[BurnTask::StopRTBurnTask] Before Stop_Stream_Burning!! strTaskID: %s, handle:%p\r\n",strTaskID.c_str(), (void*)handle))
                    if (Stop_Stream_Burning(handle) == BURN_SUCCESS)
                    {
                        stateParam.strStateFlag=STATE_WILL_CLOSE_DISC;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stopping");
                        stateParam.strTaskID=iter->strTaskID;
                        stateParam.strJobID=iter->strJobID;
                        stateParam.strUpperIP=iter->strUpperIP;
                        stateParam.strUpperPort=iter->strUpperPort;
                        stateParam.strTaskPropertyFlag=iter->strTaskPropertyFlag;

                        if (m_cb != NULL)
                        {
                            m_cb(handle,stateParam,m_pUsr);
                        }
                        LOG_INFO(("[BurnTask::StopRTBurnTask] Stop burn stream success,%d\r\n",
                            __LINE__));
                    } 
                    else
                    {
                        LOG_ERROR(("[BurnTask::StopRTBurnTask] Stop burn stream failed,%d\r\n",
                            __LINE__));

                        stateParam.strStateFlag=STATE_BURNING;
                        stateParam.strStateDescription=gBurnStateCfgFile.GetValue("info","stoprtburnfailed");
                        stateParam.strTaskID=iter->strTaskID;
                        stateParam.strJobID=iter->strJobID;
                        stateParam.strUpperIP=iter->strUpperIP;
                        stateParam.strUpperPort=iter->strUpperPort;
                        stateParam.strTaskPropertyFlag=iter->strTaskPropertyFlag;

                        if (m_cb != NULL)
                        {
                            m_cb(handle,stateParam,m_pUsr);
                        }

                        assert(false);
                    }
                }
                break;

            case TASK_STATE_ERR:
                break;

            case TASK_STATE_NO_USE:
                break;

            default:
                break;
            }

            break;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        LOG_ERROR(("[BurnTask::StopRTBurnTask] Can not find task : %s,%d\r\n",
            strTaskID.c_str(),__LINE__));
    }
#endif
}

void BurnTask::ResumeRTBurnTask(std::string strTaskID)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin(); iter!=m_vecBurnParameter.end();
        ++iter)
    {
        if (iter->strTaskID == strTaskID)
        {
            if (TASK_STATE_PAUSED == iter->nTaskRunningFlag)
            {
                SetTaskControlState(strTaskID,TASK_CONTROL_STATE_START_OR_RESUME);
            }

            break;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        LOG_ERROR(("[BurnTask::ResumeRTBurnTask] Can not find task : %s,%d\r\n",
            strTaskID.c_str(),__LINE__));
    }
}

void BurnTask::SetCurDev(std::string strTaskID,int nIndex)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();)
    {
        if (iter->strTaskID == strTaskID)
        {
            iter->nIndex=nIndex;
            break;
        }
        else
        {
            ++iter;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        LOG_ERROR(("[BurnTask::SetCurDev] Can not find task : %s,%d\r\n",strTaskID.c_str(),__LINE__));
    }
}

DEV_HANDLE BurnTask::GetCurDev(std::string strTaskID)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    DEV_HANDLE handle=NULL;

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();)
    {
        if (iter->strTaskID == strTaskID)
        {
            size_t nDevCount=iter->vecDevHandle.size();

            assert(static_cast<int>(nDevCount) > iter->nIndex);
            assert(iter->nIndex >= 0);

            handle=iter->vecDevHandle.at(iter->nIndex);

            break;
        }
        else
        {
            ++iter;
        }
    }

    if (iter == m_vecBurnParameter.end())
    {
        LOG_ERROR(("[BurnTask::GetCurDev] Can not find task : %s,%d\r\n",
            strTaskID.c_str(),__LINE__));
    }

    return handle;
}

void BurnTask::ListLeftTasks()
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    LOG_INFO(("-------- Task list --------\r\n"));
    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        LOG_INFO(("Task id : %s\r\n",iter->strTaskID.c_str()));
        LOG_INFO(("Task running flag : %d\r\n",iter->nTaskRunningFlag));
        LOG_INFO(("Task control command : %d\r\n",iter->nTaskControlCmd));
        LOG_INFO(("Task dev num : %d\r\n",iter->vecDevHandle.size()));
        LOG_INFO(("Task is belong to job : %s\r\n\r\n",iter->strJobID.c_str()));
    }
    LOG_INFO(("------------------------\r\n\r\n"));
}

bool BurnTask::StartDiskBackup(const DISK_BACKUP_PARAM &param)
{
    bool bRet=true;

    if (BURN_SUCCESS == Set_Disk_Param(param.handle,param.useDev,param.backupSize,param.alarmSize,const_cast<char *>(param.strDiskPath.c_str())))
    {
        if (BURN_SUCCESS == Disk_Create_BackUpFile(param.handle,param.strBackupFileName.c_str()))
        {
            if (BURN_SUCCESS == Start_Disk_BackUp(param.handle))
            {
                bRet=true;
            }
            else
            {
                bRet=false;
            }
        }
        else
        {
            bRet=false;
        }
    }
    else
    {
        bRet=false;
    }

    return bRet;
}

bool BurnTask::StopDiskBackup(DEV_HANDLE handle)
{
    bool bRet=false;

    if (BURN_SUCCESS == Disk_Close_BackUpFile(handle))
    {

        bRet=true;
    }

    if (bRet)
    {
        LOG_INFO(("[BurnTask::StopDiskBackup] Stop disk backup success\r\n"));
    }
    else
    {
        LOG_ERROR(("[BurnTask::StopDiskBackup] Stop disk backup failed\r\n"));
    }

    return bRet;
}

int BurnTask::GetFirstUnusedDev(std::vector<DEV_HANDLE> vectDev)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    //找到所以正在使用的光驱
    std::vector<int> vectUsedDevIndex;
    for (size_t i=0;i<vectDev.size();i++)
    {
        std::vector<BURN_PARAMETER>::iterator iter;
        for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
        {
            std::vector<DEV_HANDLE> &vecDevHandle=iter->vecDevHandle;
            size_t nDevCount=vecDevHandle.size();
            if (nDevCount == 1)
            {
                if (vecDevHandle.at(0) == vectDev.at(i))
                {
                    if (iter->strBurnDevFlag == DEV_FLAG_USED_YES)
                    {
                        vectUsedDevIndex.push_back(i);
                        LOG_INFO(("[BurnTask::GetFirstUnusedDev] Dev[%d] is used in vectDev, %d\r\n", i, __LINE__));
                    }
                }
            }
            else if (nDevCount > 1)
            {
                if (vecDevHandle.at(iter->nIndex) == vectDev.at(i))
                {
                    if (iter->strBurnDevFlag == DEV_FLAG_USED_YES)
                    {
                        vectUsedDevIndex.push_back(i);
                        LOG_INFO(("[BurnTask::GetFirstUnusedDev] Dev[%d] is used in vectDev, %d\r\n", i, __LINE__));
                    }
                }
            }
        }
    }

    //找到下一个可使用的光驱
    if ((int)vectUsedDevIndex.size() > 0 && vectUsedDevIndex.size() != vectDev.size())
    {
        int nDevIndex = vectUsedDevIndex.at((int)vectUsedDevIndex.size()-1);
        for (int i = 0; i < (int)vectDev.size(); i++)
        {
            if (nDevIndex == (int)vectDev.size() - 1)
            {
                nDevIndex = 0;
            }
            else if (nDevIndex < (int)vectDev.size() - 1)
            {
                nDevIndex += 1;
            }
            int j = 0;
            for (; j < (int)vectUsedDevIndex.size(); j++)
            {
                if (nDevIndex == vectUsedDevIndex.at(j))
                {
                    break;
                }
            }
            if (j == (int)vectUsedDevIndex.size())
            {
                return nDevIndex;
            }
        }
    }
    else if ((int)vectUsedDevIndex.size() == 0)
    {
        return 0;
    }
    return -1;
}

bool BurnTask::IsTaskExsitByJobID(std::string strJobID)
{
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();++iter)
    {
        if (iter->strJobID == strJobID)
        {
            return true;
        }
    }
    return false;
}

std::string BurnTask::IntToString(int nValue)
{
    char szValue[1024];
    memset(szValue, 0, 1024);
    sprintf(szValue, "%d", nValue);
    return std::string(szValue);
}

INT64 BurnTask::GetLimitSize()
{
    return m_nLimitSize;
}

void BurnTask::SetLimitSize( INT64 val )
{
    m_nLimitSize = val;
}

bool BurnTask::IsTaskCanCloseDisc(std::string strJobID, std::string strTaskID)
{
    bool bRet = true;
    ZOSMutexLocker  locker(&m_mutexBurnTaskParameter);

    std::vector<BURN_PARAMETER>::iterator iter;
    for (iter=m_vecBurnParameter.begin();iter!=m_vecBurnParameter.end();)
    {
        if (iter->strJobID == strJobID && iter->nTaskRunningFlag == TASK_STATE_WILL_CLOSE_DISC)
        {
            bRet = false;

            break;
        }
        else
        {
            ++iter;
        }
    }

    if (bRet)
    {
        SetTaskRunningState(strTaskID, TASK_STATE_WILL_CLOSE_DISC);
    }
    return bRet;
}

std::string BurnTask::Path(std::string strPath,bool bIsFolder)
{
    if ("" == strPath)
    {
        return "";
    }

    size_t nIndex=0;

#ifdef WIN32
    nIndex=strPath.find('/');
    while (nIndex != std::string::npos)
    {
        strPath=strPath.replace(nIndex,1,"\\");

        nIndex=strPath.find('/');
    }

    nIndex=strPath.find("\\\\");
    while (nIndex != std::string::npos)
    {
        strPath=strPath.replace(nIndex,2,"\\");

        nIndex=strPath.find("\\\\");
    }

    int nLen=strPath.length();
    if (bIsFolder &&
        nLen > 0 &&
        strPath.at(nLen-1) != '\\')
    {
        strPath+="\\";
    }
#else
    nIndex=strPath.find('\\');
    while (nIndex != std::string::npos)
    {
        strPath=strPath.replace(nIndex,1,"/");

        nIndex=strPath.find('\\');
    }

    nIndex=strPath.find("//");
    while (nIndex != std::string::npos)
    {
        strPath=strPath.replace(nIndex,2,"/");

        nIndex=strPath.find("//");
    }

    int nLen=strPath.length();
    if (bIsFolder &&
        nLen > 0 &&
        strPath.at(nLen-1) != '/')
    {
        strPath+="/";
    }
#endif

    return strPath;
}
