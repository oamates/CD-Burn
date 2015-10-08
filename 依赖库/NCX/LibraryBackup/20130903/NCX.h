#ifndef _NCX_H_
#define _NCX_H_

#ifdef WIN32

#ifdef NCXDLL_EXPORTS
#define NCXDLL_IN_EXPORT _declspec(dllexport)
#else
#define NCXDLL_IN_EXPORT _declspec(dllimport)
#endif//NCXDLL_EXPORTS

#else

#define NCXDLL_IN_EXPORT

#endif//WIN32

#ifdef __cplusplus
extern "C"
{
#endif//__cplusplus

//////////////////////////////////////////////////////////////////////////
// NCX Data Structure
//////////////////////////////////////////////////////////////////////////
typedef void* NCXSERVERHANDLE;

typedef void* NCXCLIENTHANDLE;

typedef void* NCXXMLNODEHANDLE;

typedef struct _NCXSERVER_CB_PARAM_
{
	char szRemoteIP[16];
	void *pClientProcessor;
	char *sProtocolContent;
	int	nProtocolLength;
}NCXServerCBParam;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Common function
//////////////////////////////////////////////////////////////////////////
NCXDLL_IN_EXPORT void ncxInitNCXEnvironment();
NCXDLL_IN_EXPORT void ncxUnInitNCXEnvironment();

NCXDLL_IN_EXPORT void ncxSetLogDir(const char * sLogDir);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// NCXServer relevant function
//////////////////////////////////////////////////////////////////////////
NCXDLL_IN_EXPORT NCXSERVERHANDLE ncxCreateNCXServer();

NCXDLL_IN_EXPORT void ncxDestroyNCXServer(NCXSERVERHANDLE hNcxServer);

// 0: success
// -1: port be occupied by other program
NCXDLL_IN_EXPORT int ncxSetNCXServerPort(NCXSERVERHANDLE hNcxServer, int nListenPort);

// eg. sTagName = tvw,then protocol will be like
//
// <tvw>
//    ......
// </tvw>
NCXDLL_IN_EXPORT int ncxSetNCXServerOuterMostTagName(NCXSERVERHANDLE hNcxServer, char * sTagName);

// nEventType = 1; pParam = NCXServerCBParam*s
typedef void (*NCXEventCallBack)(int nEventType, void *pParam, void* pContext);

NCXDLL_IN_EXPORT int ncxSetNCXServerCallBack(NCXSERVERHANDLE hNcxServer, NCXEventCallBack ncxServerCallBack, void *pContext);

// return byte send
NCXDLL_IN_EXPORT int ncxSendProtocolResponse(NCXSERVERHANDLE hNcxServer, NCXServerCBParam ncxServerCBParam, const char *sResponse, int nResponseLength);

NCXDLL_IN_EXPORT int ncxNCXServerStart(NCXSERVERHANDLE hNcxServer);

NCXDLL_IN_EXPORT int ncxNCXServerStop(NCXSERVERHANDLE hNcxServer);
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// NCXClient relevant function
//////////////////////////////////////////////////////////////////////////
NCXDLL_IN_EXPORT NCXCLIENTHANDLE ncxCreateNCXClient();

NCXDLL_IN_EXPORT void ncxDestroyNCXClient(NCXCLIENTHANDLE hNcxClient);

NCXDLL_IN_EXPORT int ncxSetNCXClientOuterMostTagName(NCXCLIENTHANDLE hNcxClient, char* sTagName);

NCXDLL_IN_EXPORT bool ncxConnectServer(NCXCLIENTHANDLE hNcxClient, const char* sServerIP, 
					  int nServerPort, int nWaitSecond);

NCXDLL_IN_EXPORT int ncxCommunicateWithServer(NCXCLIENTHANDLE hNcxClient, const char* sProtocol,
							 int nProtocol, int nWaitSecond);

NCXDLL_IN_EXPORT char * ncxGetRespondProtocol(NCXCLIENTHANDLE hNcxClient);

//////////////////////////////////////////////////////////////////////////
// NCXXMLNode
//////////////////////////////////////////////////////////////////////////
NCXDLL_IN_EXPORT NCXXMLNODEHANDLE ncxCreateXMLNode();

NCXDLL_IN_EXPORT void ncxDestroyXMLNode(NCXXMLNODEHANDLE hXMLNode);

//////////////////////////////////////////////////////////////////////////
// the following functions is used for construct xml node.
// when destroy only need to destroy the outer most node. 
//////////////////////////////////////////////////////////////////////////
NCXDLL_IN_EXPORT void ncxXMLSetNodeName(NCXXMLNODEHANDLE hXMLNode, char *sNodeName);

NCXDLL_IN_EXPORT void ncxXMLSetNodeValue(NCXXMLNODEHANDLE hXMLNode, char *sNodeValue);

NCXDLL_IN_EXPORT void ncxXMLAddChildNode(NCXXMLNODEHANDLE hXMLNode, NCXXMLNODEHANDLE hXMLChildNode);

NCXDLL_IN_EXPORT int ncxXMLGetStringLength(NCXXMLNODEHANDLE hXMLNode);

NCXDLL_IN_EXPORT char *ncxXMLToString(NCXXMLNODEHANDLE hXMLNode, char *sString, int *nStringLength);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// the following functions is used for get xml node value.
//////////////////////////////////////////////////////////////////////////
NCXDLL_IN_EXPORT int ncxXMLFromString(NCXXMLNODEHANDLE hXMLNode, char *sString);

NCXDLL_IN_EXPORT NCXXMLNODEHANDLE ncxXMLGetChildNode(NCXXMLNODEHANDLE hXMLNode, char *sNodeName);

NCXDLL_IN_EXPORT NCXXMLNODEHANDLE ncxXMLGetFirstChildNode(NCXXMLNODEHANDLE hXMLNode);

NCXDLL_IN_EXPORT NCXXMLNODEHANDLE ncxXMLGetNextChildNode(NCXXMLNODEHANDLE hXMLNode, NCXXMLNODEHANDLE hXMLChildNode);

NCXDLL_IN_EXPORT char *ncxXMLGetNodeName(NCXXMLNODEHANDLE hXMLNode);

NCXDLL_IN_EXPORT char *ncxXMLGetNodeValue(NCXXMLNODEHANDLE hXMLNode);

#ifdef __cplusplus
};
#endif//__cplusplus

#endif //_NCX_H_
//////////////////////////////////////////////////////////////////////////
