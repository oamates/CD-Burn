#include "NCXServerAgent.h"
#include <string.h>
#include <stdio.h>
#include "TaskModel.h"

NCXServerAgent::NCXServerAgent()
{
    m_hNCXServer = NULL;
}

NCXServerAgent::~NCXServerAgent()
{
    //
}

int NCXServerAgent::Start(int nListenPort)
{
    m_hNCXServer = ncxCreateNCXServer();
    if (m_hNCXServer != NULL)
    {
        ncxSetNCXServerPort(m_hNCXServer, nListenPort);
        ncxSetNCXServerMaxClient(m_hNCXServer, 50);
        ncxSetNCXServerOuterMostTagName(m_hNCXServer, "tvw");
        ncxSetNCXServerCallBack(m_hNCXServer, NCXServerCallBack, this);
        ncxNCXServerStart(m_hNCXServer);
    }

    return 0;
}

int NCXServerAgent::Stop()
{
    if (m_hNCXServer != NULL)
    {
        ncxNCXServerStop(m_hNCXServer);
        ncxDestroyNCXServer(m_hNCXServer);
    }

    return 0;
}

void NCXServerAgent::NCXServerCallBack(int nEventType, void *pParam, void *pContext)
{
    if (pContext != NULL)
    {
        NCXServerAgent * pAgent = (NCXServerAgent*)pContext;
        pAgent->NCXServerCallBack(nEventType, pParam);
    }
}

void NCXServerAgent::NCXServerCallBack(int nEventType, void *pParam)
{
    if (nEventType == 1)
    {
        ProcessClientProtocol((NCXServerCBParam*)pParam);
    }
}

void NCXServerAgent::ProcessClientProtocol(NCXServerCBParam *pCBParam)
{
    printf("%s \r\n", pCBParam->sProtocolContent);
    char *pRespondString = "<tvw><retcode>0</retcode></tvw>";
    ncxSendProtocolResponse(m_hNCXServer, *pCBParam, pRespondString, strlen(pRespondString)+1);
}

//////////////////////////////////////////////////////////////////////////
