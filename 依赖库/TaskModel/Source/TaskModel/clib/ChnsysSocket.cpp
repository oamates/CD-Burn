#include "ChnsysSocket.h"
#include "ZTCPSocket.h"
#include "ZUDPSocket.h"
#include "ZTCPListenerSocket.h"
#include "CLibTCPListenerEvent.h"

VOID OS_SOCKET_Init()
{
    ZSocket::Initialize();
}

VOID OS_SOCKET_Uninit()
{
    ZSocket::StopEventThread();
}

OS_UDPSOCKET_HANDLE OS_UDPSOCKET_CreateInstance()
{
    ZUDPSocket *pUdpSocket = NEW ZUDPSocket;
    if (pUdpSocket != NULL)
    {
        return (OS_UDPSOCKET_HANDLE)pUdpSocket;
    }

    return NULL;
}

VOID OS_UDPSOCKET_DestroyInstance(OS_UDPSOCKET_HANDLE hOSUdpSocket)
{
    if (hOSUdpSocket != NULL)
    {
        ZUDPSocket *pUdpSocket = (ZUDPSocket *)hOSUdpSocket;
        SAFE_DELETE(pUdpSocket);
    }
}

CHNSYS_BOOL OS_UDPSOCKET_Create(OS_UDPSOCKET_HANDLE hOSUdpSocket, CHNSYS_INT nAddrFamily)
{
    if (hOSUdpSocket != NULL)
    {
        ZUDPSocket *pUdpSocket = (ZUDPSocket *)hOSUdpSocket;
        return pUdpSocket->Create(nAddrFamily);
    }

    return FALSE;
}

CHNSYS_BOOL OS_UDPSOCKET_Close(OS_UDPSOCKET_HANDLE hOSUdpSocket)
{
    if (hOSUdpSocket != NULL)
    {
        ZUDPSocket *pUdpSocket = (ZUDPSocket *)hOSUdpSocket;
        return pUdpSocket->Close();
    }

    return FALSE;
}

CHNSYS_INT OS_UDPSOCKET_SendTo(OS_UDPSOCKET_HANDLE hOSUdpSocket, CONST CHNSYS_CHAR* sData, CONST CHNSYS_INT nData, 
                               CHNSYS_UINT nRemoteAddr, CHNSYS_UINT nRemotePort)
{
    if (hOSUdpSocket != NULL)
    {
        ZUDPSocket *pUdpSocket = (ZUDPSocket *)hOSUdpSocket;
        return pUdpSocket->SendTo(sData, nData, nRemoteAddr, nRemotePort);
    }

    return -1;
}

CHNSYS_INT OS_UDPSOCKET_RecvFrom(OS_UDPSOCKET_HANDLE hOSUdpSocket, CHNSYS_CHAR* sData, CHNSYS_INT nData, 
                                 CHNSYS_UINT *pRemoteAddr, CHNSYS_UINT *pRemotePort)
{
    if (hOSUdpSocket != NULL)
    {
        ZUDPSocket *pUdpSocket = (ZUDPSocket *)hOSUdpSocket;
        return pUdpSocket->RecvFrom(sData, nData, pRemoteAddr, pRemotePort);
    }

    return -1;
}

CHNSYS_BOOL OS_UDPSOCKET_GetRemoteAddr(OS_UDPSOCKET_HANDLE hOSUdpSocket, CHNSYS_UINT *pRemoteAddr, CHNSYS_UINT *pRemotePort)
{
    if (hOSUdpSocket != NULL)
    {
        ZUDPSocket *pUdpSocket = (ZUDPSocket *)hOSUdpSocket;
        return pUdpSocket->GetRemoteAddr(pRemoteAddr, pRemotePort);
    }

    return FALSE;
}

CHNSYS_BOOL OS_UDPSOCKET_JoinMulticast(OS_UDPSOCKET_HANDLE hOSUdpSocket, CHNSYS_UINT nRemoteAddr)
{
    if (hOSUdpSocket != NULL)
    {
        ZUDPSocket *pUdpSocket = (ZUDPSocket *)hOSUdpSocket;
        return pUdpSocket->JoinMulticast(nRemoteAddr);
    }

    return FALSE;
}

CHNSYS_BOOL OS_UDPSOCKET_LeaveMulticast(OS_UDPSOCKET_HANDLE hOSUdpSocket, CHNSYS_UINT nRemoteAddr)
{
    if (hOSUdpSocket != NULL)
    {
        ZUDPSocket *pUdpSocket = (ZUDPSocket *)hOSUdpSocket;
        return pUdpSocket->LeaveMulticast(nRemoteAddr);
    }

    return FALSE;
}

CHNSYS_BOOL OS_UDPSOCKET_SetMulticastInterface(OS_UDPSOCKET_HANDLE hOSUdpSocket, CHNSYS_UINT nLocalAddr)
{
    if (hOSUdpSocket != NULL)
    {
        ZUDPSocket *pUdpSocket = (ZUDPSocket *)hOSUdpSocket;
        return pUdpSocket->SetMulticastInterface(nLocalAddr);
    }

    return FALSE;
}

CHNSYS_BOOL OS_UDPSOCKET_SetMulticastTTL(OS_UDPSOCKET_HANDLE hOSUdpSocket, CHNSYS_UINT nTTL)
{
    if (hOSUdpSocket != NULL)
    {
        ZUDPSocket *pUdpSocket = (ZUDPSocket *)hOSUdpSocket;
        return pUdpSocket->SetMulticastTTL(nTTL);
    }

    return FALSE;
}

CHNSYS_INT OS_SOCKET_GetLowerSocket(OS_SOCKET_HANDLE hOSSocket)
{
    if (hOSSocket != NULL)
    {
        ZSocket *pSocket = (ZSocket *)hOSSocket;
        return pSocket->GetSocket();
    }

    return -1;
}

CHNSYS_BOOL OS_SOCKET_Bind(OS_SOCKET_HANDLE hOSSocket, CHNSYS_UINT nAddr, CHNSYS_UINT nPort)
{
    if (hOSSocket != NULL)
    {
        ZSocket *pSocket = (ZSocket *)hOSSocket;
        return pSocket->Bind(nAddr, nPort);
    }

    return FALSE;
}

CHNSYS_BOOL OS_SOCKET_SetNonBlocking(OS_SOCKET_HANDLE hOSSocket, CHNSYS_BOOL bNonBlocking)
{
    if (hOSSocket != NULL)
    {
        ZSocket *pSocket = (ZSocket *)hOSSocket;
        return pSocket->SetNonBlocking(bNonBlocking);
    }

    return FALSE;
}

CHNSYS_BOOL OS_SOCKET_SetReuseAddr(OS_SOCKET_HANDLE hOSSocket, CHNSYS_BOOL bReuseAddr)
{
    if (hOSSocket != NULL)
    {
        ZSocket *pSocket = (ZSocket *)hOSSocket;
        return pSocket->SetReuseAddr(bReuseAddr);
    }

    return FALSE;
}

CHNSYS_BOOL OS_SOCKET_SetSendBufferSize(OS_SOCKET_HANDLE hOSSocket, CHNSYS_INT nBufferSize)
{
    if (hOSSocket != NULL)
    {
        ZSocket *pSocket = (ZSocket *)hOSSocket;
        return pSocket->SetSendBufferSize(nBufferSize);
    }

    return FALSE;
}

CHNSYS_BOOL OS_SOCKET_SetRecvBufferSize(OS_SOCKET_HANDLE hOSSocket, CHNSYS_INT nBufferSize)
{
    if (hOSSocket != NULL)
    {
        ZSocket *pSocket = (ZSocket *)hOSSocket;
        return pSocket->SetRecvBufferSize(nBufferSize);
    }

    return FALSE;
}

CHNSYS_BOOL OS_SOCKET_SetTimeout(OS_SOCKET_HANDLE hOSSocket, CHNSYS_INT nMilliSecSend, CHNSYS_INT nMilliSecRecv)
{
    if (hOSSocket != NULL)
    {
        ZSocket *pSocket = (ZSocket *)hOSSocket;
        return pSocket->SetTimeOut(nMilliSecSend, nMilliSecRecv);
    }
    return FALSE;
}

CHNSYS_UINT OS_SOCKET_ConvertAddrToUINT(CONST CHNSYS_CHAR *sAddr)
{
    return ZSocket::ConvertAddr(sAddr);
}

CHNSYS_CHAR* OS_SOCKET_ConvertAddrToSTRING(CHNSYS_UINT nAddr)
{
    return ZSocket::ConvertAddr(nAddr);
}

CHNSYS_INT  OS_SOCKET_GetLowSocket(OS_SOCKET_HANDLE hOSSocket)
{
    if (hOSSocket != NULL)
    {
        ZSocket *pSocket = (ZSocket *)hOSSocket;
        return pSocket->GetHandle();
    }

    return -1;
}

CHNSYS_BOOL OS_TCPSOCKET_SetNoDelay(OS_SOCKET_HANDLE hOSSocket, CHNSYS_BOOL bNoDelay)
{
    if (hOSSocket != NULL)
    {
        ZSocket *pSocket = (ZSocket *)hOSSocket;
        return pSocket->SetNoDelay(bNoDelay);
    }
    return FALSE;
}

CHNSYS_BOOL OS_TCPSOCKET_SetKeepAlive(OS_SOCKET_HANDLE hOSSocket, CHNSYS_BOOL bKeepAlive)
{
    if (hOSSocket != NULL)
    {
        ZSocket *pSocket = (ZSocket *)hOSSocket;
        return pSocket->SetKeepAlive(bKeepAlive);
    }
    return FALSE;
}

OS_TCPSOCKET_HANDLE OS_TCPSOCKET_CreateInstance()
{
    ZTCPSocket *pTcpSocket = NEW ZTCPSocket;
    if (pTcpSocket != NULL)
    {
        return (OS_TCPSOCKET_HANDLE)pTcpSocket;
    }

    return NULL;
}

VOID OS_TCPSOCKET_DestroyInstance(OS_TCPSOCKET_HANDLE hOSTcpSocket)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        SAFE_DELETE(pTcpSocket);
    }
}

CHNSYS_BOOL OS_TCPSOCKET_Create(OS_TCPSOCKET_HANDLE hOSTcpSocket, CHNSYS_INT nAddrFamily)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->Create(nAddrFamily);
    }
    return FALSE;
}

CHNSYS_BOOL OS_TCPSOCKET_Close(OS_TCPSOCKET_HANDLE hOSTcpSocket)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->Close();
    }

    return FALSE;
}

CHNSYS_BOOL OS_TCPSOCKET_Listen(OS_TCPSOCKET_HANDLE hOSTcpSocket, CHNSYS_UINT nListenCount)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->Listen(nListenCount);
    }
    return FALSE;
}

CHNSYS_BOOL OS_TCPSOCKET_Accept(OS_TCPSOCKET_HANDLE hOSTcpSocket, CHNSYS_INT *phSocket, 
                                CHNSYS_UINT *pRemoteAddr, CHNSYS_UINT *pRemotePort)
{
    if (
        (hOSTcpSocket != NULL)
        && (phSocket != NULL)
        && (pRemoteAddr != NULL)
        && (pRemotePort != NULL)
        )
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        *phSocket = pTcpSocket->Accept(pRemoteAddr, pRemotePort);
        return TRUE;
    }
    return FALSE;
}

CHNSYS_BOOL OS_TCPSOCKET_Connect(OS_TCPSOCKET_HANDLE hOSTcpSocket, CHNSYS_UINT nAddr, CHNSYS_UINT nPort)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->Connect(nAddr, nPort);
    }

    return FALSE;
}

CHNSYS_BOOL OS_TCPSOCKET_ConnectAble(OS_TCPSOCKET_HANDLE hOSTcpSocket, CHNSYS_UINT nMilliSecond)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->ConnectAble(nMilliSecond*1000);
    }
    return FALSE;
}

CHNSYS_BOOL OS_TCPSOCKET_Attach(OS_TCPSOCKET_HANDLE hOSTcpSocket, CHNSYS_INT hSocket, CHNSYS_UINT nRemoteAddr, CHNSYS_UINT nRemotePort)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;

        struct	sockaddr_in	remoteAddr;
        ::memset(&remoteAddr,0,sizeof(remoteAddr));
        remoteAddr.sin_family		= AF_INET;
        remoteAddr.sin_port		    = htons(nRemotePort);
        remoteAddr.sin_addr.s_addr	= htonl(nRemoteAddr);
        return pTcpSocket->Attach(hSocket, &remoteAddr);
    }
    return FALSE;
}

CHNSYS_BOOL OS_TCPSOCKET_Detach(OS_TCPSOCKET_HANDLE hOSTcpSocket)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->Detach();
    }
    return FALSE;
}

CHNSYS_INT OS_TCPSOCKET_Send(OS_TCPSOCKET_HANDLE hOSTcpSocket, CONST CHNSYS_CHAR *sData, CONST CHNSYS_INT nData)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->Send(sData, nData);
    }
    return -100;
}

CHNSYS_INT OS_TCPSOCKET_Recv(OS_TCPSOCKET_HANDLE hOSTcpSocket, CHNSYS_CHAR *sData, CONST CHNSYS_INT nData)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->Recv(sData, nData);
    }
    return -100;
}

CHNSYS_BOOL OS_TCPSOCKET_GetRemoteAddr(OS_TCPSOCKET_HANDLE hOSTcpSocket, CHNSYS_UINT *pRemoteAddr, CHNSYS_UINT *pRemotePort)
{
    if (
        (hOSTcpSocket != NULL)
        && (pRemoteAddr != NULL)
        && (pRemotePort != NULL)
        )
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->GetRemoteAddr(pRemoteAddr, pRemotePort);
    }
    return FALSE;
}

CHNSYS_BOOL OS_TCPSOCKET_StreamCreate(OS_TCPSOCKET_HANDLE hOSTcpSocket)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        if (pTcpSocket->StreamCreate() == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

CHNSYS_BOOL OS_TCPSOCKET_StreamClose(OS_TCPSOCKET_HANDLE hOSTcpSocket)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        if (pTcpSocket->StreamClose() == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

CHNSYS_INT OS_TCPSOCKET_StreamRead(OS_TCPSOCKET_HANDLE hOSTcpSocket, CHNSYS_CHAR *sData, CHNSYS_INT nData)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->StreamRead(sData, nData);
    }
    return -100;
}

CHNSYS_INT OS_TCPSOCKET_StreamWrite(OS_TCPSOCKET_HANDLE hOSTcpSocket, CONST CHNSYS_CHAR *sData, CHNSYS_INT nData)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->StreamWrite(sData, nData);
    }
    return -100;
}

CHNSYS_BOOL OS_TCPSOCKET_StreamReadable(OS_TCPSOCKET_HANDLE hOSTcpSocket)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->StreamReadable();
    }
    return FALSE;
}

CHNSYS_BOOL OS_TCPSOCKET_StreamWriteable(OS_TCPSOCKET_HANDLE hOSTcpSocket)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->StreamWriteable();
    }
    return FALSE;
}

CHNSYS_BOOL OS_TCPSOCKET_StreamError(OS_TCPSOCKET_HANDLE hOSTcpSocket)
{
    if (hOSTcpSocket != NULL)
    {
        ZTCPSocket *pTcpSocket = (ZTCPSocket *)hOSTcpSocket;
        return pTcpSocket->StreamError();
    }
    return TRUE;
}

OS_TCPLISTENERSOCKET_HANDLE OS_TCPLISTENERSOCKET_CreateInstance()
{
    ZTCPListenerSocket *pListenerSocket = NEW ZTCPListenerSocket;
    CLibTCPListenerEvent *pTcpListenerEvent = NEW CLibTCPListenerEvent;
    if (
        (pListenerSocket != NULL)
        && (pTcpListenerEvent != NULL)
        )
    {
        pListenerSocket->SetListenerEvent(pTcpListenerEvent);
        return (OS_TCPLISTENERSOCKET_HANDLE)pListenerSocket;
    }
    
    return NULL;
}

VOID OS_TCPLISTENERSOCKET_DestroyInstance(OS_TCPLISTENERSOCKET_HANDLE hOSTcpListenerSocket)
{
    if (hOSTcpListenerSocket != NULL)
    {
        ZTCPListenerSocket *pListenerSocket = (ZTCPListenerSocket *)hOSTcpListenerSocket;
        CLibTCPListenerEvent *pTcpListenerEvent = (CLibTCPListenerEvent *)pListenerSocket->GetListenerEvent();
        SAFE_DELETE(pTcpListenerEvent);
        SAFE_DELETE(pListenerSocket);
    }
}

CHNSYS_BOOL OS_TCPLISTENERSOCKET_Create(OS_TCPLISTENERSOCKET_HANDLE hOSTcpListenerSocket, OS_TCPLISTENERSOCKET_CallBackFunc cbf, 
                                        VOID *pContext, CHNSYS_UINT nAddr, CHNSYS_UINT nPort)
{
    if (hOSTcpListenerSocket != NULL)
    {
        ZTCPListenerSocket *pListenerSocket = (ZTCPListenerSocket *)hOSTcpListenerSocket;
        CLibTCPListenerEvent *pTcpListenerEvent = (CLibTCPListenerEvent *)pListenerSocket->GetListenerEvent();
        pTcpListenerEvent->SetCallBack(cbf, pContext);
        return pListenerSocket->Create(nAddr, nPort);
    }
    return FALSE;
}

CHNSYS_BOOL OS_TCPLISTENERSOCKET_Close(OS_TCPLISTENERSOCKET_HANDLE hOSTcpListenerSocket)
{
    if (hOSTcpListenerSocket != NULL)
    {
        ZTCPListenerSocket *pListenerSocket = (ZTCPListenerSocket *)hOSTcpListenerSocket;
        return pListenerSocket->Close();
    }
    return FALSE;
}
