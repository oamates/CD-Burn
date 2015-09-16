#include "InterfaceServer.h"

InterfaceServer::InterfaceServer()
: m_hNCXServer(NULL)
, m_interfaceProcessTask()
{

}

InterfaceServer::~InterfaceServer()
{
    //
}

void InterfaceServer::Initialize(const char *sLogDir)
{
#ifdef WIN32
    if (sLogDir != NULL)
    {
        ncxSetLogDir(sLogDir);
    }
    ncxInitNCXEnvironment();
#else
    ncxInitNCXEnvironment();
#endif
}

void InterfaceServer::UnInitialize()
{
    ncxUnInitNCXEnvironment();
}

int InterfaceServer::GetPort()
{
    return m_nInterfacePort;
}

void InterfaceServer::SetPort(int nPort)
{
    m_nInterfacePort = nPort;
}

int InterfaceServer::Start()
{
    int nReturn = -1;

    if (m_hNCXServer == NULL)
    {
        m_hNCXServer = ncxCreateNCXServer();
        if (m_hNCXServer != NULL)
        {
            ncxSetNCXServerPort(m_hNCXServer, m_nInterfacePort);
            ncxSetNCXServerOuterMostTagName(m_hNCXServer,const_cast<char *>(m_strMostTagName.c_str()));
            ncxSetNCXServerCallBack(m_hNCXServer, NCXServerCallBack, this);
            nReturn = ncxNCXServerStart(m_hNCXServer);
            m_interfaceProcessTask.SetNCXServerHandle(m_hNCXServer);
            m_interfaceProcessTask.Create();
        }
    }

    return nReturn;
}

int InterfaceServer::Stop()
{
    if (m_hNCXServer != NULL)
    {
        m_interfaceProcessTask.Close();
        ncxNCXServerStop(m_hNCXServer);
        ncxDestroyNCXServer(m_hNCXServer);
        m_hNCXServer = NULL;
    }

    return 0;
}

void InterfaceServer::SetMostTagName(std::string strMostTagName)
{
    m_strMostTagName=strMostTagName;
}

void InterfaceServer::NCXServerCallBack(int nEventType, void *pParam, void *pContext)
{
    if (nEventType == 1)
    {
        if (pContext != NULL)
        {
            ((InterfaceServer *)pContext)->Process((NCXServerCBParam*)pParam);
        }
    }
}

void InterfaceServer::Process(NCXServerCBParam *pCBParam)
{
    m_interfaceProcessTask.AddRequest(pCBParam);
}
