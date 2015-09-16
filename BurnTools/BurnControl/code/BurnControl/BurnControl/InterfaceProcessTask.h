#ifndef _INTERFACE_PROCESS_TASK_H_
#define _INTERFACE_PROCESS_TASK_H_

class InterfaceProcessTask : public ZTask
{
public:
    void SetNCXServerHandle(NCXSERVERHANDLE hNCXServer);
    BOOL AddRequest(NCXServerCBParam * pNCXServerCBParam);
    virtual BOOL    Create();
    virtual BOOL    Close();
protected:
    virtual int Run(int nEvent = 0);
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
