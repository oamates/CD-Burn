#ifndef _NCXSERVERAGENT_H_
#define _NCXSERVERAGENT_H_

#include "NCX.h"

class NCXServerAgent
{
public:
    NCXServerAgent();
    ~NCXServerAgent();
public:
    int Start(int nListenPort);
    int Stop();
protected:
    static void NCXServerCallBack(int nEventType, void *pParam, void *pContext);
    void NCXServerCallBack(int nEventType, void *pParam);
    void ProcessClientProtocol(NCXServerCBParam *pCBParam);
private:
    NCXSERVERHANDLE         m_hNCXServer;
};

#endif //_NCXSERVERAGENT_H_
//////////////////////////////////////////////////////////////////////////
