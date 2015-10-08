#ifndef _CLIBTHREAD_H_
#define _CLIBTHREAD_H_
#include "ZOSThread.h"

typedef BOOL (*Thread_CallBackFunc)(void* pSelf, int nThreadPhase, VOID *pContext);

class CLibThread : public ZOSThread
{
public:
    CLibThread(CONST CHAR *sThreadName);
    virtual ~CLibThread();
public:
    virtual BOOL OnThreadStart();
    virtual BOOL OnThreadEntry();
    virtual BOOL OnThreadStop();
    void SetCallBack(Thread_CallBackFunc cbf, void *pContext);
private:
    Thread_CallBackFunc     m_funcCallBack;
    void*                   m_pCallBackContext;
};


#endif //_CLIBTHREAD_H_
