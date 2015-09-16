#include "SocketUtil.h"
#include "TaskModel.h"

int SocketUtil::GetUnusedTCPListenPort(int nPortBegin, int nPortEnd)
{
    int nPortCurrent = nPortBegin;

    do 
    {
        ZTCPListenerSocket      tcpSocket;
        if (tcpSocket.Create(0, nPortCurrent))
        {
            tcpSocket.Close();
            break;
        }
        if (nPortCurrent > nPortEnd)
        {
            nPortCurrent = nPortBegin;
        }
        else
        {
            nPortCurrent ++;
        }
    } while (true);

    return nPortCurrent;
}

int SocketUtil::GetUnusedUDPBindPort(int nPortBegin, int nPortEnd)
{
    int nPortCurrent = nPortBegin;

    do 
    {
        ZUDPSocket      udpSocket;
        if (udpSocket.Create())
        {
            if (udpSocket.Bind(0, nPortCurrent))
            {
                udpSocket.Unbind();
                break;
            }

            if (nPortCurrent > nPortEnd)
            {
                nPortCurrent = nPortBegin;
            }
            else
            {
                nPortCurrent ++;
            }
        }
        else
        {
            nPortCurrent = 0;
            break;
        }

    } while (true);

    return nPortCurrent;
}

bool SocketUtil::CheckTcpPortConnectable(const char *sIP,  int nPort, int nMilliSecond)
{
    bool bConnectable = false;

    ZTCPSocket      tcpSocket;

    if (tcpSocket.Create())
    {
        tcpSocket.SetNonBlocking(TRUE);
        if (!tcpSocket.Connect(ZSocket::ConvertAddr(sIP), nPort))
        {
        }
        if (tcpSocket.ConnectAble(nMilliSecond*1000))
        {
            bConnectable = true;
        }
    }

    return bConnectable;
}

