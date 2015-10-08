#include "../ChnsysRTSP.h"

#ifdef WIN32
#else
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#endif//WIN32

#include <stdio.h>
#include <string.h>
#include <time.h>

RTSP_SERVER_HANDLE  g_hRTSPServer = NULL;
FILE *g_hNalFile = NULL;

int ReadNalData(char *sNalData, int *pnNalSize)
{
    if (pnNalSize == NULL || sNalData == NULL)
    {
        return -1;
    }
    
    if (g_hNalFile == NULL)
    {
        g_hNalFile = fopen("/home/video.nal", "r+b");
    }
    
    if (g_hNalFile == NULL)
    {
        return -1;
    }

    int nRealRead = 0;
    // read 4 bytes nal size
    nRealRead = fread(pnNalSize, 1, 4, g_hNalFile);
    if (nRealRead != 4)
    {
        if (feof(g_hNalFile))
        {
            fseek(g_hNalFile, 0, SEEK_SET);
            return -1;
        }
        else
        {
            return -1;
        }
    }
    nRealRead = fread(sNalData, 1, *pnNalSize, g_hNalFile);
    if (nRealRead != *pnNalSize)
    {
        return -1;
    }

    return nRealRead;
}

void DoReadNalAndSend()
{
    char *sNalData = new char[1024*1024*3];
    int nNalSize = 0;
    while (1)
    {
        //struct timeval	curTime;
        //::gettimeofday(&curTime, NULL);
        //curTime.tv_sec;
        //curTime.tv_usec;

        if (ReadNalData(sNalData, &nNalSize) > 0)
        {
            //printf("nalSize %d %.2x %.2x %.2x %.2x %.2x\r\n", 
            //    nNalSize, sNalData[0], sNalData[1],
            //    sNalData[2], sNalData[3], sNalData[4]);
            struct timespec	curTime;
            clock_gettime(CLOCK_MONOTONIC, &curTime);
            //printf("time %d %d\r\n", curTime.tv_sec, curTime.tv_nsec);

            unsigned int nPts = (90000*curTime.tv_sec);
            nPts += (unsigned int)((2.0*90000*(curTime.tv_nsec/1000000*1000)+1000000.0)/2000000);

            //
            RTSP_SERVER_PushH264Data(g_hRTSPServer, nPts, sNalData, nNalSize);
        }

        usleep(30000);
    }
}

//int My_RTSP_SERVER_StatusCallBackFunc(RTSP_SERVER_STATUSCBDATA scbd)
//{
//    scbd.hRTSPServer
//}

int MY_RTSP_SERVER_RequestCallBackFunc(RTSP_SERVER_REQUESTCBDATA rcbd)
{
    *rcbd.phRTSPServer = g_hRTSPServer;
}

int main()
{
#ifdef WIN32
    RTSP_RTSP_Init("c:\\rtsptest\\");
#else
    struct	sigaction	act;
    struct	rlimit		rl;

    act.sa_flags	= 0;
    act.sa_handler	= SIG_IGN;

    (void)::sigaction(SIGPIPE,	&act,	NULL);
    (void)::sigaction(SIGHUP,	&act,	NULL);
    (void)::sigaction(SIGINT,	&act,	NULL);
    (void)::sigaction(SIGTERM,	&act,	NULL);
    (void)::sigaction(SIGQUIT,	&act,	NULL);
    (void)::sigaction(SIGALRM,	&act,	NULL);

    rl.rlim_cur	= 10240;
    rl.rlim_max	= 10240;
    setrlimit(RLIMIT_NOFILE,&rl);

    rl.rlim_cur	= RLIM_INFINITY;
    rl.rlim_max	= RLIM_INFINITY;
    setrlimit(RLIMIT_CORE,&rl);

    RTSP_RTSP_Init("/home/RTSPLib");
#endif//WIN32

    g_hRTSPServer = RTSP_SERVER_CreateInstance();
    if (g_hRTSPServer == NULL)
    {
        return 0;
    }

    RTSP_SERVER_CONFIG serverConfig;
    memset(&serverConfig, 0, sizeof(serverConfig));
    serverConfig.nSendType = 1;//unicast
    serverConfig.nAudioEncodeType = 1;//aac
    serverConfig.nSampleRate = 48000;
    serverConfig.nChannelNo = 2;
    serverConfig.nProfileLevelID = 0x15;
    serverConfig.nMode = 1;
    serverConfig.nPayloadFormatType = 1;//mpeg4-generic
    serverConfig.sSPS = NULL;
    serverConfig.nSPS = 0;
    serverConfig.sPPS = NULL;
    serverConfig.nPPS = 0;
    serverConfig.nPacketizationMode = 1;
    serverConfig.nSRPacketTimeInterval = 1;
    serverConfig.nRRWaitThreshold = 1;

    RTSP_SERVER_SetConfig(g_hRTSPServer, serverConfig);

    RTSP_SERVER_SetRequestCallBack(MY_RTSP_SERVER_RequestCallBackFunc, NULL);

    printf("-------------\r\n");
    RTSP_SERVER_Start(g_hRTSPServer);

    printf("---------before nal and send----\r\n");
    DoReadNalAndSend();
    printf("---------after nal and send----\r\n");

    RTSP_SERVER_Stop(g_hRTSPServer);

    return 0;
}

