#include "UDPClient.h"
#include "TaskModel.h"

UDPClient::UDPClient()
{
    //
}

UDPClient::~UDPClient()
{
    //
}

int UDPClient::Send(const char *sIP, int nPort, const char *sData, int nData)
{
    ZUDPSocket      udpSocket;
    int             nReturn;
    int             nBroadcast = 1;

    if (udpSocket.Create())
    {
        setsockopt(udpSocket.GetHandle(), SOL_SOCKET, SO_BROADCAST, (char *)&nBroadcast, sizeof(int));
        nReturn = udpSocket.SendTo(sData, nData, ZSocket::ConvertAddr(sIP), nPort);
        udpSocket.Close();
    }
    else
    {
        nReturn = -2;
    }

    return nReturn;
}

