#ifndef _BURNSENDSTATE_TASK_H_
#define _BURNSENDSTATE_TASK_H_

#include "UUID.h"
#include "FileUtil.h"
#include "DirectoryUtil.h"
#include "TaskModel.h"
#include "ConfigurableFile.h"

#include "SocketUtil.h"

#include <string>
#include <vector>
#include <map>

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#else
#include <pthread.h>
#endif

typedef struct _BURNSENDSTATE_PARAMETER_
{
    std::string strUpIP;
    std::string strUpPort;
    std::string strProtocalContent;
}BURNSENDSTATE_PARAMETER;
class BurnSendStateTask : public ZOSThread
{
public:
    BurnSendStateTask();
    ~BurnSendStateTask();

    BOOL    Create();
    BOOL    Close();

    virtual BOOL OnThreadStart();
    virtual BOOL OnThreadEntry();
    virtual BOOL OnThreadStop();

    void AddTask(const BURNSENDSTATE_PARAMETER &param);

    //处理发送刻录状态的线程单位
    void HandleTask();

private:
    std::vector<BURNSENDSTATE_PARAMETER> m_vecBurnSendStateParameter;
    ZOSMutex m_BurnSendStateTask;

};

#endif
