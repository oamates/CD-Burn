#ifndef _UDP_CLIENT_H_
#define _UDP_CLIENT_H_

class UDPClient
{
public:
    /**
     *
     * restrict:
     * 1 nData should less than 1472 bytes. or the udp data will be fragmentation
     *   in tcp/ip protocol stack.
     *
     * return nData; success
     * return -1; send failed
     * return -2; socket not init
     * 
     */
    static int Send(const char *sIP, int nPort, const char *sData, int nData);
private:
    UDPClient();
    ~UDPClient();
};

#endif //_UDP_CLIENT_H_
