#include "NCX.h"
#include "NCXServer.h"
#include "NCXClient.h"

static bool g_bEnvironmentInit = false;
static ZLog g_log;

void ncxInitNCXEnvironment()
{
    if (!g_bEnvironmentInit)
    {
        g_bEnvironmentInit = true;
#ifdef WIN32
        WORD	wsVersion	= MAKEWORD(1,1);
        WSADATA	wsData;
        (void)::WSAStartup(wsVersion,&wsData);
#endif
        ZOS::Initialize();
    }
}

void ncxUnInitNCXEnvironment()
{
    if (g_bEnvironmentInit)
    {
        ZOS::Uninitialize();
    }
}

void ncxSetLogDir(const char * sLogDir)
{
    LOG_SET_PATH(sLogDir);
}

NCXSERVERHANDLE ncxCreateNCXServer()
{
    NCXServer *pNCXServer = new NCXServer;
    NCXSERVERHANDLE hNcxServer = (NCXSERVERHANDLE)pNCXServer;
    return hNcxServer;
}

void ncxDestroyNCXServer(NCXSERVERHANDLE hNcxServer)
{
    NCXServer *pNCXServer = (NCXServer*)hNcxServer;
    if (pNCXServer != NULL)
    {
        delete pNCXServer;
        pNCXServer = NULL;
    }
}

int ncxSetNCXServerPort(NCXSERVERHANDLE hNcxServer, int nListenPort)
{
    NCXServer *pNCXServer = (NCXServer*)hNcxServer;
    if (pNCXServer != NULL)
    {
        pNCXServer->SetListenPort(nListenPort);
    }

    return 0;
}

int ncxSetNCXServerMaxClient(NCXSERVERHANDLE hNcxServer, int nMaxClient)
{
    NCXServer *pNCXServer = (NCXServer*)hNcxServer;
    if (pNCXServer != NULL)
    {
        pNCXServer->SetClientProcessorCount(nMaxClient);
    }

    return 0;
}

int ncxSetNCXServerOuterMostTagName(NCXSERVERHANDLE hNcxServer, char * sTagName)
{
    NCXServer *pNCXServer = (NCXServer*)hNcxServer;
    if (pNCXServer != NULL)
    {
        pNCXServer->SetOuterMostTagName(sTagName);
    }

    return 0;
}

int ncxSetNCXServerCallBack(NCXSERVERHANDLE hNcxServer, NCXEventCallBack ncxServerCallBack, void *pContext)
{
    NCXServer *pNCXServer = (NCXServer*)hNcxServer;
    if (pNCXServer != NULL)
    {
        pNCXServer->SetCallBack(ncxServerCallBack, pContext);
    }

    return 0;
}

int ncxSendProtocolResponse(NCXSERVERHANDLE hNcxServer, NCXServerCBParam ncxServerCBParam, const char *sResponse, int nResponseLength)
{
    NCXServer *pNCXServer = (NCXServer*)hNcxServer;
    if (pNCXServer != NULL)
    {
        pNCXServer->SendProtocolResponse(ncxServerCBParam, sResponse, nResponseLength);
    }

    return 0;
}

int ncxNCXServerStart(NCXSERVERHANDLE hNcxServer)
{
    NCXServer *pNCXServer = (NCXServer*)hNcxServer;
    if (pNCXServer != NULL)
    {
        pNCXServer->Create();
    }

    return 0;
}

int ncxNCXServerStop(NCXSERVERHANDLE hNcxServer)
{
    NCXServer *pNCXServer = (NCXServer*)hNcxServer;
    if (pNCXServer != NULL)
    {
        pNCXServer->Close();
    }

    return 0;
}

NCXCLIENTHANDLE ncxCreateNCXClient()
{
    NCXClient *pNcxClient = NEW NCXClient;
    NCXCLIENTHANDLE hNcxClient = (NCXCLIENTHANDLE)pNcxClient;
    if (pNcxClient != NULL)
    {
        pNcxClient->Create();
    }

    return hNcxClient;
}

void ncxDestroyNCXClient(NCXCLIENTHANDLE hNcxClient)
{
    NCXClient *pNcxClient = (NCXClient*)hNcxClient;

    if (pNcxClient != NULL)
    {
        pNcxClient->Close();
        SAFE_DELETE(pNcxClient);
    }
}

int ncxSetNCXClientOuterMostTagName(NCXCLIENTHANDLE hNcxClient, char* sTagName)
{
    NCXClient *pNcxClient = (NCXClient *)hNcxClient;

    if (pNcxClient != NULL)
    {
        pNcxClient->SetOuterMostTagName(sTagName);
    }
    
    return 0;
}

bool ncxConnectServer(NCXCLIENTHANDLE hNcxClient, const char* sServerIP, 
                      int nServerPort, int nWaitSecond)
{
    NCXClient *pNcxClient = (NCXClient *)hNcxClient;

    if (pNcxClient != NULL)
    {
        if (pNcxClient->ConnectServer(sServerIP, nServerPort, nWaitSecond))
        {
            return true;
        }
    }

    return false;
}

int ncxCommunicateWithServer(NCXCLIENTHANDLE hNcxClient, const char* sProtocol,
                             int nProtocol, int nWaitSecond)
{
    NCXClient *pNcxClient = (NCXClient *)hNcxClient;

    if (pNcxClient != NULL)
    {
        return pNcxClient->Communicate(sProtocol, nProtocol, nWaitSecond);
    }

    return -11;
}

char * ncxGetRespondProtocol(NCXCLIENTHANDLE hNcxClient)
{
    NCXClient *pNcxClient = (NCXClient *)hNcxClient;

    if (pNcxClient != NULL)
    {
        return pNcxClient->GetRespondProtocol();
    }

    return NULL;
}
//////////////////////////////////////////////////////////////////////////
