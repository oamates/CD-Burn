#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Burn_SDK.h>
#include <Burn_Stream.h>
// #include <RingBuffer.h>
#include <Database.h>

#ifdef ARM_LINUX
#include <ChnsysRTSP.h>
#else
#include <ChnsysRTSP_fan.h>
#endif
#include <locale.h>

#include "StreamSelfInclude.h"

#ifdef WIN32
#else
#include <unistd.h>
#endif

#if 0
DEV_HANDLE get_dev_handle(RTSPTS_HANDLE *hStream);
int Fill_Data_To_Dev_Buf(DEV_HANDLE hBurnDEV, char *buf_ptr, int len, int block);
int config_rtsp_and_ts_param(DEV_HANDLE hBurnDEV);
RTSPTS_HANDLE *get_stream_handle(DEV_HANDLE hBurnDEV);
int	rtsptsmutexinit();
int create_stream_database(DEV_HANDLE hBurnDEV);
int Write_Dev_Buf_Data_To_File(DEV_HANDLE hBurnDEV, FILE_HANDLE hFile);
int Fill_Data_To_Dev_Buf(DEV_HANDLE hBurnDEV, char *buf_ptr, int len, int block);
#endif

//#define RTSP_TEST
#define	ZFILE	unsigned int
//RTSPTS_HANDLE*  hStream_A;
#if 0
void initrtsp(RTSP_CLIENT_CONFIG cfg,char* name)
{
	strcpy(cfg.sUrl,name);
	cfg.nTransportType = 1;
	cfg.bSynchronizationMode = TRUE;
	cfg.nWaitSecond = 10;
	cfg.bReconnect = TRUE;
	cfg.nReconnectTryTimes = 0x7FFFFFFF;
	cfg.nReconnectInterval = 10;
	cfg.nRRPacketTimeInterval = 3;
	cfg.bAACRawDataOutputWithAdtsHeader = 1;
}
#endif

#if 0
//用于注册回调函数
ZFILE*	open(const char* sname,const char* args)
{
	return (ZFILE*)sname;
}

//用于注册回调函数
ZFILE read(void* data,ZFILE len,ZFILE count,ZFILE* handle)
{
	return 0;
}

//用于注册回调函数,TS生产出数据后该函数会被调用
ZFILE write(const void* data,ZFILE len,ZFILE count,ZFILE* handle)
{
    int i, ret;
    int cnt = 0;
    DEV_HANDLE hBurnDEV;
	RTSPTS_HANDLE*	hStream	= (RTSPTS_HANDLE*)handle;

	printf("write %d %d\r\n",len,count);

#if 0
    hBurnDEV = get_dev_handle(hStream);
    if(hBurnDEV == NULL)
    {
        printf("[Write] Get hBurnDEV Failed\n");
        return 0;
    }

    for(i = 0; i < count; i++)
    {
        //将数据写入设备的缓冲中
        ret = Fill_Data_To_Dev_Buf(hBurnDEV, data, len, BURN_FLASE);
        if(ret == BURN_FAILURE)
        {
            printf("Fill Data To Dev Buf Failed\n");
        }
        else
            cnt += len;
    }
#endif
	return cnt;
}

//用于注册回调函数
int	close(ZFILE* handle)
{
	if(handle != 0 && handle != (ZFILE*)0xFFFFFFFF)
	{
		RTSPTS_HANDLE*	phandle	= (RTSPTS_HANDLE*)handle;
		printf("close %s\r\n",phandle->gMediaName);
		{
			fclose(phandle->gMediaHandle);
		}
	}
	return 0;
}
#else
ZFILE*	myopen(const char* sname,const char* aaa)
{
	{
		RTSPTS_HANDLE*	phandle	= (RTSPTS_HANDLE*)sname;

		printf("=========== open %s ==========\r\n",phandle->gMediaName);
#ifdef RTSP_TEST		
		phandle->gMediaHandle	= fopen(phandle->gMediaName,"wb");
#endif
	}
	return (ZFILE*)sname;
}
ZFILE	myread(void* data,ZFILE len,ZFILE count,ZFILE* handle)
{
	{
//		RTSPTS_HANDLE*	phandle	= (RTSPTS_HANDLE*)handle;
//		printf("========== read %d %d ===========\r\n",len,count);
//		len	= fread(data,len,count,phandle->gMediaHandle);
	}
	return len;
}
ZFILE	mywrite(const void* data,ZFILE len,ZFILE count,ZFILE* handle)
{
	    int ret;
	    DEV_HANDLE hBurnDEV;
	    RTSPTS_HANDLE*  hStream = (RTSPTS_HANDLE*)handle;

//		printf("========= write %d %d ==========\r\n",len,count);

#ifdef RTSP_TEST
		RTSPTS_HANDLE*	phandle	= (RTSPTS_HANDLE*)handle;
		fwrite(data,len,count,phandle->gMediaHandle);
		fflush(phandle->gMediaHandle);
#endif
#ifndef RTSP_TEST
#if 1
    hBurnDEV = get_dev_handle(hStream);
    if(hBurnDEV == NULL)
    {
        printf("[Write] Get hBurnDEV Failed\n");
        return 0;
    }
//		printf("========= write %d %d ==========\r\n",len,count);

        //将数据写入设备的缓冲中
        ret = Fill_Data_To_Dev_Buf(hBurnDEV, (char*)data, count, BURN_FLASE);
        if(ret == BURN_FAILURE)
        {
            printf("Fill Data To Dev Buf Failed\n");
        }
        else
		{
//			printf("Fill Data [%d] To Dev Buf Success\n", count);
		}
#endif
#endif
    return len;
}
int		myclose(ZFILE* handle)
{
	if(handle != 0 && handle != (ZFILE*)0xFFFFFFFF)
	{
//		RTSPTS_HANDLE*	phandle	= (RTSPTS_HANDLE*)handle;
//		printf("============ close %s ============\r\n",phandle->gMediaName);
		{
//			fclose(phandle->gMediaHandle);
		}
	}
	return 0;
}

#endif



//配置RTSP的URL和TS通道
int config_rtsp_url_and_ts_channel(RTSPTS_HANDLE *hStream, char *stream_url, int ts_channel)
{
    char url[1000];

    strcpy(url, stream_url);

    //调用RTSP提供的函数，配置URL和TS通道
//    initrtsp(hStream->gRTSPClientConfig[ts_channel], url);

	strcpy(hStream->gRTSPClientConfig[ts_channel].sUrl, url);
	hStream->gRTSPClientConfig[ts_channel].nTransportType = 2;
	hStream->gRTSPClientConfig[ts_channel].bSynchronizationMode = TRUE;
	hStream->gRTSPClientConfig[ts_channel].nWaitSecond = 10;
	hStream->gRTSPClientConfig[ts_channel].bReconnect = TRUE;
	hStream->gRTSPClientConfig[ts_channel].nReconnectTryTimes = 0x7FFFFFFF;
	hStream->gRTSPClientConfig[ts_channel].nReconnectInterval = 10;
	hStream->gRTSPClientConfig[ts_channel].nRRPacketTimeInterval = 3;
	hStream->gRTSPClientConfig[ts_channel].bAACRawDataOutputWithAdtsHeader = 1;

    printf("Set Url[%s] And Channel[%d] Success\n", url, ts_channel);

    return BURN_SUCCESS;
}

//配置数据流相关参数
int config_stream_parameters(DEV_HANDLE hBurnDEV)
{
    int ret;

    ret = config_rtsp_and_ts_param(hBurnDEV);

    return ret;
}


void Print_Stream_Struct(RTSPTS_HANDLE *hStream)
{
    int i;
    
    printf("Stream Struct Set Is:\n");
    for(i = 0; i < 4; i++)
    {
        printf(" TS Channel[%d]'s  Url Is [%s], TransportType Is[%d], MediaName Is [%s]\n", i, hStream->gRTSPClientConfig[i].sUrl, hStream->gRTSPClientConfig[i].nTransportType, hStream->gMediaName);
    }
}

void initrtsp(RTSP_CLIENT_CONFIG *cfg,char* name)
{
	strcpy(cfg->sUrl,name);
	cfg->nTransportType = 1;
	cfg->bSynchronizationMode = TRUE;
	cfg->nWaitSecond = 10;
	cfg->bReconnect = TRUE;
	cfg->nReconnectTryTimes = 0x7FFFFFFF;
	cfg->nReconnectInterval = 10;
	cfg->nRRPacketTimeInterval = 3;
	cfg->bAACRawDataOutputWithAdtsHeader = 1;

}

#if 0
char* TransformUnicodeToUTF8M(const wchar_t* _str)
{
    char* result = NULL;
    int textlen = 0;
    if (_str)
    {
        textlen = WideCharToMultiByte( CP_UTF8, 0, _str, -1, NULL, 0, NULL, NULL );
        result =(char *)malloc((textlen+1)*sizeof(char));
        memset(result, 0, sizeof(char) * ( textlen + 1 ) );
        WideCharToMultiByte( CP_UTF8, 0, _str, -1, result, textlen, NULL, NULL );
    }
    return result;
}

int cUxG(char* pOut,WCHAR* pText)
{ 
    int ret = 0; 
    if(pText[0]<0x80){        // ASCII  0x00 ~ 0x7f 
        pOut[0] = (char)pText[0]; 
    }else{ 
        ::WideCharToMultiByte(CP_ACP,0,pText,1,pOut,sizeof(WCHAR),NULL,NULL); 
        ret = 1; 
    } 
    return ret; 
} 
#endif


size_t get_executable_path( char* processdir,char* processname, size_t len)
{
	char* path_end;
	if(readlink("/proc/self/exe", processdir,len) <=0)
		return -1;
	path_end = strrchr(processdir,  '/');
	if(path_end == NULL)
		return -1;
	++path_end;
	strcpy(processname, path_end);
	*path_end = '\0';
	return (size_t)(path_end - processdir);
}




int Start_Stream_Burning(DEV_HANDLE hBurnDEV)
{
    INTERFACE_TYPE type;
    RTSPTS_HANDLE *hStream;
    FILE_HANDLE    hFile;
    int            ret;
//    char          *file_name = "test.ts";
    char          file_name[1000];
	printf("enter Start_Stream_Burning\n");
    {
        type = B_STREAM;

        hStream = get_stream_handle(hBurnDEV);
        if(hStream == NULL)
        {
            printf("Get Stream Handle Failed\n");
            return BURN_FAILURE;            
        }
		
		char ts_bk_file_name[256];
		sprintf(ts_bk_file_name, "/mnt/HD0/ts_write_%d.ts", hBurnDEV->dev_id);
		strcpy(hStream->gMediaName,ts_bk_file_name);

        ret = config_stream_parameters(hBurnDEV);

        Print_Stream_Struct(hStream);

        Burn_Get_RT_File_Name(file_name);
#ifdef RTSP_TEST
		char process_path[256] = {0};
		char process_name[256] = {0};
		get_executable_path(process_path, process_name, sizeof(process_path));
		char readme_file_path[512] = {0};
		sprintf(readme_file_path, "%sREADME.TXT", process_path);
		printf("process_path = %s\n", process_path);
		Burn_File_Form_Local_File(hBurnDEV, NULL, readme_file_path);
#endif
#if 1
        //光盘上创建文件
        printf("Create File [%s] On Disc\n", file_name);
        sleep(2);
        hFile = Burn_Ctrl_CreateFile(hBurnDEV, NULL, file_name);
        if(!hFile)
        {
            char file_update_name[1000];
            printf("Get File Handle Failed, Try Change Name\n");
            Set_RT_File_Name_Update();
            Burn_Get_RT_File_Name(file_update_name);
            hFile = Burn_Ctrl_CreateFile(hBurnDEV, NULL, file_update_name);
            if(!hFile)
            {
                printf("Get File Handle Failed\n");
                return BURN_FAILURE;
            }
        }
#endif	
        //启动RTSP与TS模块
        printf("Start Running RTSP And Ts ...\n");
        rtsptsinitialization(hStream);
        printf("Start Running RTSP And Ts Success...\n");

        printf("Write Dev Buf Data To File\n");
		usleep(500*1000);
        ret = Write_Dev_Buf_Data_To_File(hBurnDEV, hFile);
        if(ret == 1)
        {
            printf("Write_Dev_Buf_Data_To_File Failed\n");
            return BURN_FAILURE;
        }
        else if (ret == -1)
        {
            printf("Write_Dev_Buf_Data_To_File Burn_Ctrl_WriteData Failed\n");
            Stop_Stream_Burning(hBurnDEV);
            printf("out Stop_Stream_Burning Failed\n");
            if (Burn_Ctrl_CloseFile(hBurnDEV, hFile) != 0)
            {
                printf("out Burn_Ctrl_CloseFile Failed\n");
            }
            else
            {
                printf("out Burn_Ctrl_CloseFile Success\n");
            }
            return BURN_FAILURE;
        }
    }

    return BURN_SUCCESS;
}


int Stop_Stream_Burning(DEV_HANDLE hBurnDEV)
{
    RTSPTS_HANDLE *hStream;

	printf("Stop Burning Disc...\n");
	if(Get_Running_Burn_State(hBurnDEV) == B_STOP)
	{
		printf("BurnServer Is Stop Now\n");
		return BURN_SUCCESS;	
	}

    //针对流模式的处理
    {
        hStream = get_stream_handle(hBurnDEV);
        if(hStream == NULL)
        {
            printf("Get Stream Handle Failed\n");
            return BURN_FAILURE;            
        }

    printf("====== Set RTSP And TS Stoped ======\n");
    rtsptsuninitialization(hStream);
    printf("====== Set RTSP And TS Stoped Success ======\n");
        //TODO Release hStream
    }

    return Burn_Ctrl_StopBurn(hBurnDEV);
}

int Init_Stream_Subsystem(DEV_HANDLE hBurnDEV)
{
    int ret;

    ret = create_stream_database(hBurnDEV);
    if(ret != BURN_SUCCESS)
    {
        printf("Init Stream Subsystem Failed\n");
        return BURN_FAILURE;
    }

    return BURN_SUCCESS;
}

int Init_Task_Module(const char* log_path)
{
#if 1
    //RTSP模块提供的函数，进行RTSP初始化
    printf("Start Set RTSP Init ...\n");
	//两个分支，
	//如果定义了ARM_LINUX宏，则是3531版本，使用阚工库函数接口，
	//否则使用范工版本
#ifdef ARM_LINUX
    RTSP_RTSP_Init(log_path);
#else
    RTSP_RTSP_Init(log_path, 20);
#endif

#endif
    //RTSP模块提供的函数，进行TS初始化
    printf("Start Set TS Init ...\n");
    rtsptsmutexinit();        

    //RTSP模块提供的函数，加载回调函数
	//回调函数的作用，是ts模块输出数据到ringbuffer
    printf("Start Set RTSP CallBack ...\n");
    rtsptssetcallback(myopen,myread,mywrite,myclose);    

    return BURN_SUCCESS;
}

int Unint_Task_Module(void)
{
	RTSP_RTSP_Uninit();
    return BURN_SUCCESS;
}

