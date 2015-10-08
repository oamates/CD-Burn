#include "CLibThread.h"

CLibThread::CLibThread(CONST CHAR *sThreadName)
: ZOSThread(sThreadName)
{
    m_funcCallBack = NULL;
    m_pCallBackContext = NULL;
}

CLibThread::~CLibThread()
{

}

BOOL CLibThread::OnThreadStart()
{
    if (m_funcCallBack != NULL)
    {
        return m_funcCallBack(this, 1, m_pCallBackContext);
    }
    return FALSE;
}

BOOL CLibThread::OnThreadEntry()
{
    if (m_funcCallBack != NULL)
    {
        return m_funcCallBack(this, 2, m_pCallBackContext);
    }
    return FALSE;
}

BOOL CLibThread::OnThreadStop()
{
    if (m_funcCallBack != NULL)
    {
        return m_funcCallBack(this, 3, m_pCallBackContext);
    }
    return FALSE;
}

void CLibThread::SetCallBack(Thread_CallBackFunc cbf, void *pContext)
{
    m_funcCallBack = cbf;
    m_pCallBackContext = pContext;
}
