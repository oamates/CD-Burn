#include "InterfaceProtocol.h"
#include "InterfaceServer.h"
#include "BurnServerAgent.h"
#include "FileAnywhereManager.h"

#ifdef WIN32
#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#else
#include "unistd.h"
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#endif

ZLog theLog;

ConfigurableFile gBurnStateCfgFile;

ConfigurableFile gPrimeraCfgFile;

ConfigurableFile gEpsonCfgFile;

ConfigurableFile gBasicParamCfgFile;
#ifdef WIN32
#else
///////////////////////////////////////////////////////////////////////////////
//#include	"Common.h"
//#include	"ZServer.h"
///////////////////////////////////////////////////////////////////////////////
#if defined(_LINUX_) || defined(_MINGW_)
///////////////////////////////////////////////////////////////////////////////
#if defined(_LINUX_)
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#endif	//defined(_LINUX_)
///////////////////////////////////////////////////////////////////////////////
#define	LINUX_DEBUG		1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool g_bStop=false;

extern	int ZServerStop(const char* sConfigure,int nCommand);
extern	int ZServerRun(const char* sConfigure,int nCommand);

int ZServerStop(const char* sConfigure,int nCommand)
{
    g_bStop=true;

    return 0;
}
int ZServerRun(const char* sConfigure,int nCommand)
{
    std::string strCurDir=BurnServerAgent::GetCurDir();

    std::string strBasicParamCfgFilePath=BurnTask::Path(BurnServerAgent::GetCurDir()+"config/"+"BasicParamConfig.xml",false);
    bool bBasicParamCfgFile = gBasicParamCfgFile.LoadFile(strBasicParamCfgFilePath);

    std::string strLogPath = "";
    if (gBasicParamCfgFile.GetValue("info","burnServerCreateLog") == "true")
    {
        if (strCurDir != "")
        {
            strLogPath=strCurDir+"log/";
            printf("Log path : %s,%d\r\n",strLogPath.c_str(),__LINE__);
        }
    }

    Init_Task_Module(strLogPath.c_str());

    if (bBasicParamCfgFile)
    {
        LOG_INFO(("[ZServerRun] Load basic param config file success\r\n"));
    }
    else
    {
        LOG_ERROR(("[ZServerRun] Load basic param config file failed : %s\r\n",strBasicParamCfgFilePath.c_str()));
    }


    std::string strCfgFilePath=BurnTask::Path(BurnServerAgent::GetCurDir()+"config/"+"BurnStateInformation.xml",false);
    if (gBurnStateCfgFile.LoadFile(strCfgFilePath))
    {
        LOG_INFO(("[ZServerRun] Load information config file success\r\n"));
    }

    //处理协议，并且实现刻录的事务
    BurnServerAgent burnServerAgent;

    burnServerAgent.Init();

    InterfaceServer::Initialize();

    FileAnywhereManager::Initialize();

    {
        //用于接收刻录协议
        InterfaceServer interfaceServerForBurn;
        interfaceServerForBurn.SetPort(BURN_SERVER_PORT);
        interfaceServerForBurn.SetMostTagName(BURN_PROTOCOL_TAG);

        //用于接收FileAnywhere下载反馈
        InterfaceServer interfaceServerForDownload;
        interfaceServerForDownload.SetPort(FILE_ANY_WHERE_PORT);
        interfaceServerForDownload.SetMostTagName(DOWNLOAD_PROTOCOL_TAG);

        if (0 == interfaceServerForBurn.Start() &&
            0 == interfaceServerForDownload.Start())
        {
            LOG_INFO(("[ZServerRun] Server start success\r\n"));

            burnServerAgent.ReadRTBurnBackupXML();

            while(!g_bStop)
            {
                sleep(1);
            }

            interfaceServerForBurn.Stop();
            interfaceServerForDownload.Stop();
        }
        else
        {
            LOG_ERROR(("[ZServerRun] Server start failed\r\n"));
        }
    }

    FileAnywhereManager::Uninitialize();

    InterfaceServer::UnInitialize();

    burnServerAgent.UnInit();

    Unint_Task_Module();

    return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ShutLinux(BOOL bReboot)
{
    if(bReboot)
    {
        system("reboot");
    }else{
        system("shutdown now");
    }
    return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(_LINUX_)
static	pid_t	gParentPID	= 0;
static	pid_t	gChildPID	= 0;
///////////////////////////////////////////////////////////////////////////////
void outlinuxlog(const char* stype,int ncode,pid_t nPID)
{
#ifdef	LINUX_DEBUG
    FILE*	f		= fopen("/home/dvs-crash.sig","a");
    char	s[256];
    if(f != 0)
    {
        struct tm*	Time;
        time_t		time;
        ::time(&time);                
        Time	= ::localtime(&time); 
        sprintf(s,"time = %04d-%02d-%02d %02d:%02d:%02d %s = %d; pid = %d\r\n",
            (Time->tm_year+1900),(Time->tm_mon+1),Time->tm_mday,Time->tm_hour,Time->tm_min,Time->tm_sec,
            stype,ncode,nPID);
        fwrite(s,1,strlen(s),f);
        fclose(f);
    }
#endif	//LINUX_DEBUG

}
int sendtochild(int sig, pid_t nPID)
{
    if(gParentPID == nPID) // this is the parent
    {
        if(gChildPID != 0)
        {
            ::kill(gChildPID,sig);
            return 0;
        }
    }

    return -1;
}
void sigcatcher(int sig,int /*sinfo*/,struct sigcontext* /*sctxt*/)
{
    pid_t	nPID	= getpid();

    outlinuxlog("signal",sig,nPID);

    if(sig == SIGHUP)
    {
        sendtochild(sig,nPID);
    }
    if(sig == SIGINT)
    {
        sendtochild(sig,nPID);
    }
    if(sig == SIGTERM || sig == SIGQUIT)
    {
        if(sendtochild(sig,nPID)!=0)
        {
            ZServerStop("",0);
        }
    }

}
#endif	//defined(_LINUX_)
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int LinuxInit()
{
    return 0;
}
int LinuxServer(const int nCommand,const char* sConfigure)
{
    if(nCommand == COMMAND_DEBUG)
    {
        ZServerRun(sConfigure,nCommand);
    }
    if(nCommand == COMMAND_RUN)
    {
#if defined(_LINUX_)
        struct	sigaction	act;
        struct	rlimit		rl;
        pid_t	processID	= 0;
        int		status		= 0;
        int		pid			= 0;

        gParentPID	= 0;
        gChildPID	= 0;
        sigemptyset(&act.sa_mask);
        act.sa_flags	= 0;
        act.sa_handler	= (void(*)(int))&sigcatcher;

        (void)::sigaction(SIGPIPE,	&act,	NULL);
        (void)::sigaction(SIGHUP,	&act,	NULL);
        (void)::sigaction(SIGINT,	&act,	NULL);
        (void)::sigaction(SIGTERM,	&act,	NULL);
        (void)::sigaction(SIGQUIT,	&act,	NULL);
        (void)::sigaction(SIGALRM,	&act,	NULL);

        //rl.rlim_cur	= RLIM_INFINITY;
        //rl.rlim_max	= RLIM_INFINITY;
        rl.rlim_cur	= 10240;
        rl.rlim_max	= 10240;

        setrlimit(RLIMIT_NOFILE,&rl);
#ifdef	LINUX_DEBUG
        rl.rlim_cur	= RLIM_INFINITY;
        rl.rlim_max	= RLIM_INFINITY;

        setrlimit(RLIMIT_CORE,&rl);
#endif	//LINUX_DEBUG
        if (daemon(0,0) != 0)
        {
            exit(-1);
        }

        do
        {
            gParentPID	= getpid();
            processID	= fork();
            //ASSERT((processID >= 0));
            assert(processID >= 0);
            if(processID > 0)
            {
                gChildPID	= processID;
                status		= 0;
                while(status==0)
                {
                    pid	=::wait(&status);
                    int	exitStatus	= (BYTE)WEXITSTATUS(status);
                    if(WIFEXITED(status) && pid > 0 && status != 0)
                    {
                        if(exitStatus == -1)
                        {
                            PRINT_OUT(("LinuxServer Child exit\r\n"));
                            exit(EXIT_FAILURE); 
                        }
                        outlinuxlog("exit code",WEXITSTATUS(status),pid);
                        break;
                    }
                    if(WIFSIGNALED(status))
                    {
                        outlinuxlog("exit signal",WTERMSIG(status),pid);
                        break;
                    }
                    if(WIFSTOPPED(status))
                    {
                        outlinuxlog("stop signal",WSTOPSIG(status),pid);
                        break;
                    }
                    if(pid == -1 && status == 0)
                    {
                        continue;
                    }
                    if(pid > 0 && status == 0)
                    {
                        exit(EXIT_SUCCESS);
                    }
                    exit(EXIT_FAILURE);
                }
            }else if(processID == 0)
            {
                break;
            }else
            {
                exit(EXIT_FAILURE);
            }
            sleep(1);
        }while(TRUE);
        if(processID != 0)
        {
            exit(EXIT_SUCCESS); 
        }else{
            gChildPID	= 0;

            (void)::sigaction(SIGPIPE,	&act,	NULL);
            (void)::sigaction(SIGHUP,	&act,	NULL);
            (void)::sigaction(SIGINT,	&act,	NULL);
            (void)::sigaction(SIGTERM,	&act,	NULL);
            (void)::sigaction(SIGQUIT,	&act,	NULL);
            ZServerRun(sConfigure,nCommand);
            exit(EXIT_SUCCESS); 
        }
#else
        ZServerRun(sConfigure,nCommand);
#endif	//defined(_LINUX_)
    }

    return 0;
}
int LinuxUninit()
{
    return 0;
}
///////////////////////////////////////////////////////////////////////////////
#endif	//
///////////////////////////////////////////////////////////////////////////////

#endif

int main(int argc,char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "limit") == 0)
        {
            INT64 nLimitSize = -1;
            if (i+1 < argc)
            {
#ifdef WIN32
                nLimitSize = _atoi64(argv[i+1]);
#else
                nLimitSize = strtoull(argv[i+1], NULL, 10);
#endif
            }


            BurnTask::SetLimitSize(nLimitSize);
            break;
        }
    }
#ifdef WIN32

#ifdef _DEBUG
    ::_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    ::AllocConsole();
    freopen("CON", "w", stdout);
#endif

    std::string strBasicParamCfgFilePath=BurnTask::Path(
        std::string(BURN_SERVER_CFG_DIR)+"config\\"+"BasicParamConfig.xml",false);
    bool bBasicParamCfgFile = gBasicParamCfgFile.LoadFile(strBasicParamCfgFilePath);

    if (gBasicParamCfgFile.GetValue("info","burnServerCreateLog") == "true")
    {
        std::string strLogPath=std::string(BURN_SERVER_CFG_DIR)+"log\\";
        LOG_SET_PATH(strLogPath.c_str());
        LOG_SET_LEVEL(ZLog::LOG_INFORMATION);
    }

    if (bBasicParamCfgFile)
    {
        LOG_INFO(("[main] Load basic param config file success\r\n"));
    }
    else
    {
        LOG_ERROR(("[main] Load basic param config file failed : %s\r\n",strBasicParamCfgFilePath.c_str()));
    }

    LOG_INFO(("[main] Single disk limit: %d M\r\n", BurnTask::GetLimitSize()));

    std::string strBurnStateInfoCfgFilePath=BurnTask::Path(
        std::string(BURN_SERVER_CFG_DIR)+"config\\"+"BurnStateInformation.xml",false);
    if (gBurnStateCfgFile.LoadFile(strBurnStateInfoCfgFilePath))
    {
        LOG_INFO(("[main] Load information config file success\r\n"));
    }
    else
    {
        LOG_ERROR(("[main] Load information config file failed : %s\r\n",strBurnStateInfoCfgFilePath.c_str()));
    }

    std::string strPrimeraCfgFilePath=BurnTask::Path(
        std::string(BURN_SERVER_CFG_DIR)+"config\\"+"primera.xml",false);
    if (gPrimeraCfgFile.LoadFile(strPrimeraCfgFilePath))
    {
        LOG_INFO(("[main] Load primera config file success\r\n"));
    }
    else
    {
        LOG_ERROR(("[main] Load primera config file failed : %s\r\n",strPrimeraCfgFilePath.c_str()));
    }

    std::string strEpsonCfgFilePath=BurnTask::Path(
        std::string(BURN_SERVER_CFG_DIR)+"config\\"+"Epson.xml",false);
    if (gEpsonCfgFile.LoadFile(strEpsonCfgFilePath))
    {
        LOG_INFO(("[main] Load epson config file success\r\n"));
    }
    else
    {
        LOG_ERROR(("[main] Load epson config file failed : %s\r\n",strEpsonCfgFilePath.c_str()));
    }

    ZOS::Initialize();

    //处理协议，并且实现刻录的事务
    BurnServerAgent burnServerAgent;

    burnServerAgent.Init();

    InterfaceServer::Initialize("C:\\CS\\BurnServer\\log\\ncx\\");

    FileAnywhereManager::Initialize();

    {
        //用于接收刻录协议
        InterfaceServer interfaceServerForBurn;
        interfaceServerForBurn.SetPort(BURN_SERVER_PORT);
        interfaceServerForBurn.SetMostTagName(BURN_PROTOCOL_TAG);

        //用于接收FileAnywhere下载反馈
        InterfaceServer interfaceServerForDownload;
        interfaceServerForDownload.SetPort(FILE_ANY_WHERE_PORT);
        interfaceServerForDownload.SetMostTagName(DOWNLOAD_PROTOCOL_TAG);

        if (0 == interfaceServerForBurn.Start() &&
            0 == interfaceServerForDownload.Start())
        {
            LOG_INFO(("[main] Server start success\r\n"));

            while (true)
            {
                Sleep(40000);
            }

            interfaceServerForBurn.Stop();
            interfaceServerForDownload.Stop();
        }
        else
        {
            LOG_ERROR(("[main] Server start failed\r\n"));
        }
    }

    FileAnywhereManager::Uninitialize();

    InterfaceServer::UnInitialize();

    burnServerAgent.UnInit();

    ZOS::Uninitialize();
#else

#if 0
    //用于测试重命名函数
    if (RenameFile("/home/BurnServer1.0.0.4.tar.gz","BurnServer1.0.0.4_Rename",".tar.gz"))
    {
        LOG_INFO(("Rename success\r\n"));
    } 
    else
    {
        LOG_ERROR(("Rename failed\r\n"));
    }
    return 0;
#endif

//     LinuxServer(COMMAND_DEBUG,NULL);

    LinuxServer(COMMAND_RUN,NULL);
#endif

    return 0;
}
