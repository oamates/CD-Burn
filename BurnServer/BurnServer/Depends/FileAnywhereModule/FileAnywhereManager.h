#ifndef _FILEANYWHERE_MANAGER_H_
#define _FILEANYWHERE_MANAGER_H_

#include "TaskModel.h"
#include "FileAnywhereTaskParameter.h"
#include "FileAnywhereObserver.h"
#include <vector>

class FileAnywhereManager : public ZOSThread
{
public:
    static FileAnywhereManager *Initialize();
    static void Uninitialize();
    static FileAnywhereManager *GetInstance();
public:
    //interface
    int AddTransmissionTask(FileAnywhereTaskParameter parameter);
    int DeleteTransmissionTask(std::string strTaskID);
public:
    int OnFileAnywhereFeedBack(FileAnywhereObserverParameter parameter);
protected:
    BOOL    Create();
    BOOL    Close();
protected:
    virtual BOOL	OnThreadStart();
    virtual BOOL	OnThreadEntry();
    virtual BOOL	OnThreadStop();
    //virtual int Run(int nEvent = 0);
    BOOL            DoStartProcess();
    BOOL            GetNotStartedTask(FileAnywhereTaskParameter &taskParameter);
    BOOL            UpdateTaskParameter(FileAnywhereTaskParameter taskParameter);
    BOOL            DoRetrieve();
public:
    FileAnywhereManager();
    ~FileAnywhereManager();
private:
    static FileAnywhereManager                  *m_pInstance;
    std::vector<FileAnywhereTaskParameter>      m_vectFATaskParameter;
    ZOSMutex                                    m_mutexFATaskParameterVect;
};

#endif//_FILEANYWHERE_MANAGER_H_
