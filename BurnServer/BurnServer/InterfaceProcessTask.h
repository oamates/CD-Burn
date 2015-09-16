#ifndef _INTERFACE_PROCESS_TASK_H_
#define _INTERFACE_PROCESS_TASK_H_


#include "TaskModel.h"

#include "NCX.h"

class InterfaceProcessTask : public ZOSThread
{
public:
    void SetNCXServerHandle(NCXSERVERHANDLE hNCXServer);
    BOOL AddRequest(NCXServerCBParam * pNCXServerCBParam);

    BOOL    Create();
    BOOL    Close();

protected:
    virtual BOOL OnThreadStart();
    virtual BOOL OnThreadEntry();
    virtual BOOL OnThreadStop();

    BOOL ProcessRequest();
public:
    InterfaceProcessTask();
    virtual ~InterfaceProcessTask();
private:
    ZOSArray<NCXServerCBParam*>         m_arrayCBParam;
    ZOSMutex                            m_mutexArrayCBParam;

    NCXSERVERHANDLE                     m_hNCXServer;
};

#endif //_INTERFACE_PROCES_TASK_H_
