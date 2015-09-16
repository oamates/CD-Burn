#ifndef _INTERFACE_SERVER_H_
#define _INTERFACE_SERVER_H_

#include <stdio.h>
#include "InterfaceProcessTask.h"

#include <string>

#define BURN_SERVER_PORT 8888
#define FILE_ANY_WHERE_PORT 8889

//////////////////////////////////////////////////////////////////////////
// InterfaceServer, InterfaceProcessTask, InterfaceProtocolSpecific these 3
// classes process ncx specific protocol.
// 
// InterfaceServer wrap ncx lib, do start ncx server, listen and on callback
// get ncx client request parameter.
//
// InterfaceProcessTask start a task to process each request in array.
//
// InterfaceProtocolSpecific process specific protocol.
//
//////////////////////////////////////////////////////////////////////////

class InterfaceServer
{
public:
    static void Initialize(const char *sLogDir = NULL);
    static void UnInitialize();
    int GetPort();
    void SetPort(int nPort);
    int Start();
    int Stop();

    void SetMostTagName(std::string strMostTagName);
private:
    static void NCXServerCallBack(int nEventType, void *pParam, void *pContext);
    void Process(NCXServerCBParam *pCBParam);
public:
    InterfaceServer();
    ~InterfaceServer();

private:
    NCXSERVERHANDLE         m_hNCXServer;
    InterfaceProcessTask    m_interfaceProcessTask;
    std::string             m_strMostTagName;
    int				        m_nInterfacePort;
};

#endif //_INTERFACE_SERVER_H_
