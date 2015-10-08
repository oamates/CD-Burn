#ifndef _CLIBTASK_H_
#define _CLIBTASK_H_

#include "ChnsysTypes.h"
#include "ZTimeoutTask.h"

typedef int (*Task_CallBackFunc)(int nEventType, void* pContext);

class CLibTask : public ZTimeoutTask
{
public:
    CLibTask();
    virtual ~CLibTask();
public:
    virtual BOOL Create();
    virtual BOOL Close();
    void SetCallBack(Task_CallBackFunc cbf, void* pContext);
protected:
    virtual int Run(int nEvent = 0);
private:
    Task_CallBackFunc   m_funcCallBack;
    void*               m_pCallBackContext;
};

#endif //_CLIBTASK_H_
