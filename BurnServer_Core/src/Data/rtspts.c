#include "rtspts.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef	WIN32
static	int					nLocalMutexInit	= 0;
static	pthread_mutexattr_t	sLocalMutexAttr;
static	pthread_mutexattr_t	*sMutexAttr		= &sLocalMutexAttr;
static pthread_once_t		sMutexAttrInit	= PTHREAD_ONCE_INIT;
static void MutexAttrInit();
void MutexAttrInit()
{
	memset(sMutexAttr,0,sizeof(pthread_mutexattr_t));
	pthread_mutexattr_init(sMutexAttr);
	pthread_mutexattr_settype(sMutexAttr,PTHREAD_MUTEX_ERRORCHECK);
	pthread_mutexattr_settype(sMutexAttr,PTHREAD_MUTEX_RECURSIVE);
}
#endif	//WIN32
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int	rtsptsmutexinit()
{
#ifndef	WIN32
	if(nLocalMutexInit == 0)
	{
		pthread_once(&sMutexAttrInit,MutexAttrInit);
		nLocalMutexInit	= 1;
	}
#endif	//WIN32
	return 0;
}
int rtsptssetcallback(FILEOPEN open,FILEREAD read,FILEWRITE write,FILECLOSE close)
{
	SetMediaFileSystem(open,read,write,close);
	return 0;
}

FILE *g_hH264File = NULL;
//bool g_bPlaySuccess = 0; 
int  g_nH264StreamNo = 1;

int rtspdatacalloback(RTSP_CLIENT_RAWDATACBDATA rdcbd)
{
#if 0

    if (g_hH264File == NULL)
    {
        g_hH264File = fopen("/home/sephrioth/Test/BurnServer_Test/fanrl.h264", "wb");
    }

//    if (g_bPlaySuccess)
    {
        if (rdcbd.nStreamNo == g_nH264StreamNo)
        {
            //printf("[MyRawDataCallBack] h264 seconds %d useconds %d\r\n", rdcbd.nTimestampSecond, rdcbd.nTimestampUSecond);
            if (g_hH264File != NULL)
            {
                fwrite(rdcbd.sData, 1, rdcbd.nData, g_hH264File);
                fflush(g_hH264File);
            }
        }
#if 0
        else if (rdcbd.nStreamNo == g_nAACStreamNo)
        {
            //printf("[MyRawDataCallBack] aac seconds %d useconds %d\r\n", rdcbd.nTimestampSecond, rdcbd.nTimestampUSecond);
            if (g_hAACFile != NULL)
            {
                fwrite(rdcbd.sData, 1, rdcbd.nData, g_hAACFile);
                fflush(g_hAACFile);
            }
        }
#endif
    }
//    return 0;
#else
	int				i				= 0;
	RTSPTS_HANDLE*	pHandle			= (RTSPTS_HANDLE*)rdcbd.pContext;
	UINT64			ntime			= (((UINT64)rdcbd.nTimestampSecond * (UINT64)1000) + (UINT64)(rdcbd.nTimestampUSecond / 1000));

	if(pHandle != NULL)
	{
		//printf("data %d\r\n",rdcbd.nData);
#ifdef	WIN32
		::EnterCriticalSection(&(pHandle->gSection));
#else
		int error = pthread_mutex_lock(&(pHandle->gMutex));
		if(error != 0)
		{
			printf("lock error \r\n");
		}
#endif	//WIN32
		if(rdcbd.sData != 0 && rdcbd.nData > 0)
		{
			for(i = 0; i < RTSP_CLIENT_MAX; i ++)
			{
				if(rdcbd.hRTSPClient == pHandle->gRTSPClient[i])
				{
					if(rdcbd.nStreamNo == pHandle->gMediaData[i].gMediaRTSPAudioStream)
					{
//						if(pHandle->gMediaTSInstance,pHandle->gMediaData[i].gMediaRTSPAudioTime == 0)
						if(pHandle->gMediaData[i].gMediaRTSPAudioTime == 0)
						{
							pHandle->gMediaData[i].gMediaRTSPAudioTime	= ntime;
						}
						ntime	-= pHandle->gMediaData[i].gMediaRTSPAudioTime;
						MEDIA_STREAM_SAMPLE	audiosample	= {0,0,ntime,ntime,rdcbd.nData,rdcbd.sData,rdcbd.nData};
						MediaWriteSample(pHandle->gMediaTSInstance,pHandle->gMediaData[i].gMediaTSAudioStream,&audiosample);
					}else
					if(rdcbd.nStreamNo == pHandle->gMediaData[i].gMediaRTSPVideoStream)
					{
//						if(pHandle->gMediaTSInstance,pHandle->gMediaData[i].gMediaRTSPVideoTime == 0)
						if(pHandle->gMediaData[i].gMediaRTSPVideoTime == 0)
						{
							pHandle->gMediaData[i].gMediaRTSPVideoTime	= ntime;
						}
						ntime	-= pHandle->gMediaData[i].gMediaRTSPVideoTime;
						MEDIA_STREAM_SAMPLE	videosample	= {0,0,ntime,ntime,rdcbd.nData,rdcbd.sData,rdcbd.nData};
						MediaWriteSample(pHandle->gMediaTSInstance,pHandle->gMediaData[i].gMediaTSVideoStream,&videosample);
					}
					break;
				}
			}
		}
#ifdef	WIN32
		::LeaveCriticalSection(&(pHandle->gSection));
#else
		pthread_mutex_unlock(&(pHandle->gMutex));
#endif	//WIN32
	}
	return 0;
#endif
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int rtsptsinitialization(RTSPTS_HANDLE* pHandle)
{
	int		i	= 0;
	int		j	= 0;
	int		n	= 0;
	int		t	= 0;
	MEDIA_STREAM_HEADER	audioheader	= {MEDIA_TYPE_AUDIO,MEDIA_CODEC_AAC,0,0,0,0};
	MEDIA_STREAM_HEADER	videoheader	= {MEDIA_TYPE_VIDEO,MEDIA_CODEC_AVC1,0,0,0,0};

	if(pHandle != 0)
	{
		if(pHandle->gMediaTSInstance == 0)
		{
			pHandle->gMediaTSInstance	= CreateMediaInstance();
			if(pHandle->gMediaTSInstance != 0)
			{
				MediaOpen(pHandle->gMediaTSInstance,(const char*)pHandle);
				pHandle->gMediaProgram	= MediaAddProgram(pHandle->gMediaTSInstance);
			}else{
				printf("CreateMediaInstance ERROR\r\n");
			}
		}
#ifdef	WIN32
		::InitializeCriticalSection(&(pHandle->gSection));
		::EnterCriticalSection(&(pHandle->gSection));
#else
		  pthread_mutex_init(&(pHandle->gMutex),sMutexAttr);
		  pthread_mutex_lock(&(pHandle->gMutex));
#endif	//WIN32
		for(i = 0; i < RTSP_CLIENT_MAX; i ++)
		{
			if(pHandle->gRTSPClientConfig[i].sUrl[0] != 0)
			{
                printf("==============rtsptsinitialization gRTSPClientConfig[%d] \r\n",i);
				pHandle->gRTSPClient[i]	= RTSP_CLIENT_CreateInstance();
                printf("==============rtsptsinitialization gRTSPClientConfig[%d] RTSP_CLIENT_CreateInstance\r\n",i);
				RTSP_CLIENT_SetConfig(pHandle->gRTSPClient[i],pHandle->gRTSPClientConfig[i]);
                printf("==============rtsptsinitialization gRTSPClientConfig[%d] RTSP_CLIENT_SetConfig\r\n",i);
				RTSP_CLIENT_Play(pHandle->gRTSPClient[i]);
                printf("==============rtsptsinitialization gRTSPClientConfig[%d] RTSP_CLIENT_Play\r\n",i);
				n = RTSP_CLIENT_GetStreamCount(pHandle->gRTSPClient[i]);
                printf("==============rtsptsinitialization gRTSPClientConfig[%d] RTSP_CLIENT_GetStreamCount\r\n",i);
				for(j = 0; j < n; j ++)
				{
					t = RTSP_CLIENT_GetStreamType(pHandle->gRTSPClient[i],j);
                    printf("==============rtsptsinitialization gRTSPClientConfig[%d] RTSP_CLIENT_GetStreamType\r\n",i);
					if(t == 100)
					{ 
						pHandle->gMediaData[i].gMediaRTSPVideoStream	= j;
						pHandle->gMediaData[i].gMediaTSVideoStream		= MediaAddStream(pHandle->gMediaTSInstance,pHandle->gMediaProgram,&videoheader);
                        printf("==============rtsptsinitialization gRTSPClientConfig[%d] j:%d t:%d MediaAddStream\r\n",i,j,t);
					}else
					if(t == 200 || t == 201)
					{
						pHandle->gMediaData[i].gMediaRTSPAudioStream	= j;
						pHandle->gMediaData[i].gMediaTSAudioStream		= MediaAddStream(pHandle->gMediaTSInstance,pHandle->gMediaProgram,&audioheader);
                        printf("==============rtsptsinitialization gRTSPClientConfig[%d] j:%d t:%d MediaAddStream\r\n",i,j,t);
					}
				}
			}
		}
        printf("==============rtsptsinitialization out for \r\n");
		for(i = 0; i < RTSP_CLIENT_MAX; i ++)
		{
			if(pHandle->gRTSPClient[i]	!= 0)
			{
				RTSP_CLIENT_SetRawDataCallBack(pHandle->gRTSPClient[i],rtspdatacalloback,(VOID*)pHandle);
                printf("==============rtsptsinitialization gRTSPClient[%d] RTSP_CLIENT_SetRawDataCallBack\r\n",i);
			}
		}
        printf("==============rtsptsinitialization out for RTSP_CLIENT_SetRawDataCallBack\r\n",i);
		if(pHandle->gMediaTSInstance != 0)
		{
			if(pHandle->gMediaProgram > 0)
			{
			}else{
				printf("MediaAddProgram ERROR\r\n");
			}
		}
#ifdef	WIN32
		::LeaveCriticalSection(&(pHandle->gSection));
#else
		pthread_mutex_unlock(&(pHandle->gMutex));
#endif	//WIN32
	}
	return -1;
}
int rtsptsuninitialization(RTSPTS_HANDLE* pHandle)
{
	int	i		= 0;
	if(pHandle != 0)
	{
		for(i = 0; i < RTSP_CLIENT_MAX; i ++)
		{
			if(pHandle->gRTSPClient[i] != 0)
			{
				RTSP_CLIENT_SetRawDataCallBack(pHandle->gRTSPClient[i],0,(VOID*)0);
			}
		}
#ifdef	WIN32
		::EnterCriticalSection(&(pHandle->gSection));
#else
		pthread_mutex_lock(&(pHandle->gMutex));
#endif	//WIN32
		for(i = 0; i < RTSP_CLIENT_MAX; i ++)
		{
			if(pHandle->gRTSPClient[i] != 0)
			{
				RTSP_CLIENT_Stop(pHandle->gRTSPClient[i]);
			}
		}
		for(i = 0; i < RTSP_CLIENT_MAX; i ++)
		{
			if(pHandle->gRTSPClient[i] != 0)
			{
				RTSP_CLIENT_DestroyInstance(pHandle->gRTSPClient[i]);
			}
		}
#ifdef	WIN32
		::LeaveCriticalSection(&(pHandle->gSection));
		::DeleteCriticalSection(&(pHandle->gSection));
#else
		pthread_mutex_unlock(&(pHandle->gMutex));
		pthread_mutex_destroy(&(pHandle->gMutex));
#endif	//WIN32
		if(pHandle->gMediaTSInstance != 0)
		{
			MediaClose(pHandle->gMediaTSInstance);
			CloseMediaInstance(pHandle->gMediaTSInstance);
			pHandle->gMediaTSInstance	= 0;
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef	DEBUG_RTSPTS
#include <stdio.h>
#include <string.h>
RTSPTS_HANDLE	handle1 = {0};
RTSPTS_HANDLE	handle2	= {0};
void initrtsp(RTSP_CLIENT_CONFIG& cfg,char* name)
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
ZFILE*	open(const char* sname,const char*)
{
	{
		RTSPTS_HANDLE*	phandle	= (RTSPTS_HANDLE*)sname;

		printf("open %s\r\n",phandle->gMediaName);
		phandle->gMediaHandle	= fopen(phandle->gMediaName,"wb");
	}
	return (ZFILE*)sname;
}
ZFILE	read(void* data,ZFILE len,ZFILE count,ZFILE* handle)
{
	{
		RTSPTS_HANDLE*	phandle	= (RTSPTS_HANDLE*)handle;
		//printf("read %d %d\r\n",len,count);
		len	= fread(data,len,count,phandle->gMediaHandle);
	}
	return len;
}
ZFILE	write(const void* data,ZFILE len,ZFILE count,ZFILE* handle)
{
	{
		RTSPTS_HANDLE*	phandle	= (RTSPTS_HANDLE*)handle;
		//printf("write %d %d\r\n",len,count);
		len	= fwrite(data,len,count,phandle->gMediaHandle);
	}
	return len;
}
int		close(ZFILE* handle)
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
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	strcpy(handle1.gMediaName,"./test11.ts");
	strcpy(handle2.gMediaName,"./test12.ts");
	//init play url
	initrtsp(handle1.gRTSPClientConfig[0], "rtsp://10.14.1.15/1");
	initrtsp(handle1.gRTSPClientConfig[1], "rtsp://10.14.1.15/2");
	initrtsp(handle2.gRTSPClientConfig[0], "rtsp://10.14.1.15/1");

	//rtsp init
	RTSP_RTSP_Init("./", 20);
	//mutex init
	rtsptsmutexinit();
	//set file call back
	rtsptssetcallback(open,read,write,close);
	//init ts
	rtsptsinitialization(&handle1);
	rtsptsinitialization(&handle2);

	getchar();

	//unint ts
	rtsptsuninitialization(&handle1);
	rtsptsuninitialization(&handle2);
	//uninit
	RTSP_RTSP_Uninit();

	return 0;
}
#endif	//DEBUG_RTSPTS
