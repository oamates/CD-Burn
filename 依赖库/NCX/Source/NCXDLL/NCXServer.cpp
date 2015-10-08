#include "NCXServer.h"

NCXServer::NCXServer()
: ZOSThread("NCXServerThread")
, m_nListenPort(0)
, m_tcpSocketListen()
, m_nClientProcessorCount(20)
, m_cbFunction(NULL)
, m_pContext(NULL)
, m_vectClientProcessor()
, m_mutexClientProcessorVect("MutexClientProcessorVect")
{
    memset(m_sOuterMostTagName, 0, MAX_TAGNAME_LENGTH);
}

NCXServer::~NCXServer()
{
    Close();
}

void NCXServer::SetListenPort(int nPort)
{
    m_nListenPort = nPort;
}

void NCXServer::SetClientProcessorCount(int nCount)
{
    m_nClientProcessorCount = nCount;
}

void NCXServer::SetOuterMostTagName(const char *sTagName)
{
    strncpy(m_sOuterMostTagName, sTagName, MAX_TAGNAME_LENGTH);
    m_sOuterMostTagName[MAX_TAGNAME_LENGTH-1] = '\0';
}

int NCXServer::SetCallBack(NCXEventCallBack ncxEventCallBack, void *pContext)
{
    m_cbFunction = ncxEventCallBack;
    m_pContext = pContext;

    return 0;
}

BOOL NCXServer::Create()
{
    BOOL bReturn = FALSE;

    if (m_tcpSocketListen.Create())
    {
        if (m_tcpSocketListen.Bind(0, m_nListenPort))
        {
            m_tcpSocketListen.SetNonBlocking(TRUE);
            if (m_tcpSocketListen.Listen(2048))
            {
                bReturn = TRUE;
            }
            else
            {
                LOG_ERROR(("[NCXServer::Create] Listen failed\r\n"));
            }
        }
        else
        {
            LOG_ERROR(("[NCXServer::Create] bind %d failed\r\n", m_nListenPort));
        }
    }
    else
    {
        LOG_ERROR(("[NCXServer::Create] may be socket not init\r\n"));
    }

    if (bReturn)
    {
        if (StartClientProcessorThreadPool() == 0)
        {
            //start thread;
            bReturn = ZOSThread::Start();
        }
    }

    if (!bReturn)
    {
        Close();
    }

    return bReturn;
}

BOOL NCXServer::Close()
{
    m_tcpSocketListen.Close();
    ZOSThread::Stop();
    StopClientProcessorThreadPool();

    return TRUE;
}

int NCXServer::SendProtocolResponse(NCXServerCBParam ncxServerCBParam, const char *sResponse, int nResponseLength)
{
    if (ncxServerCBParam.pClientProcessor != NULL)
    {
        ((ClientProcessor*)(ncxServerCBParam.pClientProcessor))->SendRespond(sResponse, nResponseLength);
        return 0;
    }

    return -1;
}

int NCXServer::StartClientProcessorThreadPool()
{
    ZOSMutexLocker  locker(&m_mutexClientProcessorVect);
    for (int i = 0; i < m_nClientProcessorCount; i ++)
    {
        ClientProcessor *pClientProcessor = new ClientProcessor;
        if (pClientProcessor != NULL)
        {
            pClientProcessor->Create();
            pClientProcessor->SetListener(this);
            pClientProcessor->SetOuterMostTagName(m_sOuterMostTagName);
            m_vectClientProcessor.push_back(pClientProcessor);
        }
    }

    return 0;
}

int NCXServer::StopClientProcessorThreadPool()
{
    ClientProcessor *ppClientProcessor[1024] = {0};

    ZOSMutexLocker  locker(&m_mutexClientProcessorVect);
    for (size_t i = 0; i < m_vectClientProcessor.size(); i ++)
    {
        if (m_vectClientProcessor.at(i) != NULL)
        {
            ppClientProcessor[i] = m_vectClientProcessor.at(i);
            m_vectClientProcessor.at(i)->Close(FALSE);
        }
    }

#ifdef MAXIMUM_WAIT_OBJECTS
    UINT    nMax    = 0;
    UINT    nCount  = 0;
    int     nIndex;
    nMax = (MAXIMUM_WAIT_OBJECTS - 1);
    for(nIndex = 0; nIndex < m_nClientProcessorCount; nIndex += nMax)
    {
        nCount = MIN((UINT)m_nClientProcessorCount-nIndex,nMax);
        ZOSThread::WaitForAllThread(nCount,(ZOSThread**)&ppClientProcessor[nIndex]);
    }
#else
    ZOSThread::WaitForAllThread(m_nClientProcessorCount,(ZOSThread**)ppClientProcessor);
#endif //MAXIMUM_WAIT_OBJECTS

    return TRUE;
}

BOOL NCXServer::OnThreadStart()
{
    return TRUE;
}

BOOL NCXServer::OnThreadEntry()
{
    if (!m_bStop)
    {
        if (!PorcessAccept())
        {
            ZOSThread::Sleep(10);
        }
    }

    return TRUE;
}

BOOL NCXServer::OnThreadStop()
{
    return TRUE;
}

bool NCXServer::PorcessAccept()
{
    bool    bReturn = false;
    UINT    nRemoteAddr;
    UINT    nRemotePort;
    int     nClientSocket;

    nClientSocket = m_tcpSocketListen.Accept(&nRemoteAddr, &nRemotePort);
    if (nClientSocket != -1)
    {//client connect
        //set to ClientProcessor;
        ZOSMutexLocker      locker(&m_mutexClientProcessorVect);
        ClientProcessor     *pClientProcessor = NULL;
        for (size_t i = 0; i < m_vectClientProcessor.size(); i ++)
        {
            if (!m_vectClientProcessor.at(i)->IsBeUsed())
            {
                pClientProcessor = m_vectClientProcessor.at(i);
                break;
            }
        }
        if (pClientProcessor != NULL)
        {
            pClientProcessor->SetClient(nClientSocket, nRemoteAddr, nRemotePort);
            bReturn = true;
        }
        else
        {
            ZTCPSocket::TCPClose(nClientSocket);
            LOG_ERROR(("[NCXServer::PorcessAccept] too much client, max is %d\r\n", m_nClientProcessorCount));
        }
    }

    return bReturn;
}

BOOL NCXServer::OnReceiveCompleteProtocol(ClientProcessor *pClientProcessor, const char *sProtocol, int nProtocolLength)
{
    if (m_cbFunction != NULL)
    {
        NCXServerCBParam param;
        param.pClientProcessor = pClientProcessor;
        strncpy(param.szRemoteIP, pClientProcessor->GetRemoteIP(), MAX_IP_LENGTH);
        param.sProtocolContent = (char*)sProtocol;
        param.nProtocolLength = nProtocolLength;
        m_cbFunction(1, &param, m_pContext);
    }

    return TRUE;
}
//////////////////////////////////////////////////////////////////////////
