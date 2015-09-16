#ifndef _SOCKET_UTIL_H_
#define _SOCKET_UTIL_H_

#include <string>

class SocketUtil
{
public:
    static int GetUnusedTCPListenPort(int nPortBegin, int nPortEnd);
    static int GetUnusedUDPBindPort(int nPortBegin, int nPortEnd);
    static bool CheckTcpPortConnectable(const char *sIP,  int nPort, int nMilliSecond);
};


#endif //_SOCKET_UTIL_H_
