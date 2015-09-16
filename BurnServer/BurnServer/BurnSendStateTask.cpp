#ifdef WIN32
#include <afx.h>

#include "CharsetConvertMFC.h"
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
#endif

#include "BurnSendStateTask.h"
#include <iostream>

#ifdef WIN32
#else
#define NDEBUG
#endif

#include <stdio.h>

#include <algorithm>

#include "InterfaceProtocol.h"

BurnSendStateTask::BurnSendStateTask():ZOSThread("BurnSendStateTask"),
m_BurnSendStateTask("mutexBurnSendStateTask")
{
    m_vecBurnSendStateParameter.clear();
}

BurnSendStateTask::~BurnSendStateTask()
{

}

BOOL BurnSendStateTask::Create()
{
    if (ZOSThread::Start())
    {
        return TRUE;
    }

    return FALSE;
}

BOOL BurnSendStateTask::Close()
{
    if (ZOSThread::Stop(TRUE))
    {
        return TRUE;
    }

    return FALSE;
}

BOOL BurnSendStateTask::OnThreadStart()
{
    return TRUE;
}

BOOL BurnSendStateTask::OnThreadEntry()
{
    if (!m_bStop)
    {
        HandleTask();

        ZOSThread::Sleep(20);
    }

    return TRUE;
}

BOOL BurnSendStateTask::OnThreadStop()
{
    return TRUE;
}

void BurnSendStateTask::AddTask(const BURNSENDSTATE_PARAMETER &param)
{
    ZOSMutexLocker  locker(&m_BurnSendStateTask);
    m_vecBurnSendStateParameter.push_back(param);
}

void BurnSendStateTask::HandleTask()
{
    ZOSMutexLocker  locker(&m_BurnSendStateTask);
    std::vector<BURNSENDSTATE_PARAMETER>::iterator iter;
    for (iter=m_vecBurnSendStateParameter.begin();iter!=m_vecBurnSendStateParameter.end();)
    {
        std::string strUpperIP=iter->strUpIP;
        std::string strUpperPort=iter->strUpPort;
        std::string strProtocolContent=iter->strProtocalContent;

        if (strUpperIP != "" &&
            strUpperPort != "" &&
            strProtocolContent != "")
        {
            std::string strResponse;
            strResponse=CInterfaceProtocol::InterfaceCommunicate(strUpperIP,atoi(strUpperPort.c_str()),strProtocolContent,BURN_PROTOCOL_TAG);
            if (strResponse.length() > 0)
            {
                LOG_INFO(("[BurnSendStateTask::HandleTask] Send burn state success,upper : IP: %s, Port:%s\r\n",
                    strUpperIP.c_str(),strUpperPort.c_str()));
            }
            else
            {
                LOG_ERROR(("[BurnSendStateTask::HandleTask] Send burn state failed strResponse: %s,upper : IP: %s, Port:%s \r\n",
                    strResponse.c_str(),strUpperIP.c_str(),strUpperPort.c_str()));
            }
        }
        iter = m_vecBurnSendStateParameter.erase(iter);
    }
}




