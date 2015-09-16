//dvd光驱实时刻录SDK测试程序


//支持打开大文件，2G以上的文件
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



#include "LibDVDSDK.h"

static char * cdr_Name = "/dev/dvdrw";
static XKD_DVDDRV cdr_dev = NULL;
static LVDVD_DEV_INFO_T pDevInfo0;
static LVDVD_DISC_INFO_T pDiscInfo;
static Xkd_DVDSDK_DIR pMediaDir = NULL;

static void getcdrDeviceType(int ntype)
{
	switch(ntype)
	{
		case DVDDRIVER_UNKNOWN :	// 未知光驱类型
			printf("device type is DVDDRIVER_UNKNOWN\n");
			break;
		case DVDDRIVER_PRVIDVD:		// 专用刻录DVD光盘
			printf("device type is DVDDRIVER_PRVIDVD\n");
			break;
		case DVDDRIVER_CDR:			// CD只读光驱
			printf("device type is DVDDRIVER_CDR\n");
			break;		
		case DVDDRIVER_CDRW:		// CD读写光驱
			printf("device type is DVDDRIVER_CDRW\n");
			break;			
		case DVDDRIVER_DVDR:		// DVD只读光驱
			printf("device type is DVDDRIVER_DVDR\n");
			break;			
		case DVDDRIVER_DVDRW:		// DVD读写光驱
			printf("device type is DVDDRIVER_DVDRW\n");
			break;			
		case DVDDRIVER_BLUER:		// 蓝光只读光驱
			printf("device type is DVDDRIVER_BLUER\n");
			break;			
		case DVDDRIVER_BLUERW:		// 蓝光读写光驱
			printf("device type is DVDDRIVER_BLUERW\n");
			break;		
	}
}

#define MINPACKETED 32* 1024
#define DEFAULTPACKED 32 * 1024

static void WriteFileToDisk(Xkd_DVDSDK_FILE FileHandle, char * fileName)
{
	char buffer[DEFAULTPACKED * 2];
	int size;
	int fd = open(fileName,O_RDONLY);
	
	memset(buffer, 0 , DEFAULTPACKED);
	size = read(fd, buffer, DEFAULTPACKED);
	while( size )
	{
		Xkd_DVDSDK_WriteData(cdr_dev, FileHandle, buffer, size);
		usleep(1000);
		size = read(fd, buffer, DEFAULTPACKED);
	}
	
	close(fd);
	printf(" WriteFileToDisk %s is ok!\n", fileName );
}

int main(int argc, char * argv[] )
{
	Xkd_DVDSDK_FILE tmpFile;
	
	if( argc == 2 )
	{
		cdr_Name = argv[1];
	}
	else if( argc == 3 )
	{
		cdr_Name = argv[2];
	}
	
	printf(" dvd recorder device: %s \n", cdr_Name);
	
	//加载dvd光驱
	cdr_dev = Xkd_DVDSDK_Load(cdr_Name);
	if( !cdr_dev )
	{
		printf("load device failed \n");
		return -1;
	}
		
	//获取设备信息
	if( Xkd_DVDSDK_GetDevInfo(cdr_dev, &pDevInfo0) != ERROR_DVD_OK )
	{
		printf("Xkd_DVDSDK_GetDevInfo is failed\n");
		goto UNLOADDEV;
	}
	
	//打印设备信息
	getcdrDeviceType(pDevInfo0.drvtype);
	printf("pDevInfo0.szVender=%s \n", pDevInfo0.szVender);
	printf("pDevInfo0.discsupts=%d \n", pDevInfo0.discsupts);
	//printf("pDevInfo0.disclist=%s \n", pDevInfo0.disclist);
	
	//获取托盘状态
	if( Xkd_DVDSDK_GetTrayState(cdr_dev) ) 
	{
		printf("dvd tray is open, you must close it!\n");
		Xkd_DVDSDK_Tray(cdr_dev, 0); //关闭光驱
	}

	//锁定托盘门，防止意外打开
//	if( Xkd_DVDSDK_LockDoor(cdr_dev, 1) != ERROR_DVD_OK )
	{
///		printf("LockDoor cdrom door is failed!\n");
	}
	printf("LockDoor cdrom door is ok!\n");
	sleep(1);
	
	//判断是否有光盘
	if( Xkd_DVDSDK_HaveDisc(cdr_dev) == ERROR_DVD_NODISC )
	{
		printf("cdrom have not disk!\n");
		Xkd_DVDSDK_Tray(cdr_dev, 1);  //打开光驱
		goto UNLOADDEV;	
	}
	printf("find dvd Disc!\n");
	
	Xkd_DVDSDK_LoadDisc(cdr_dev);

	//printf("MediaBasicType:%d \n", Xkd_DVDSDK_GetMediaBasicType(cdr_dev) );
	printf("MediaExactType:%d \n", Xkd_DVDSDK_GetMediaExactType(cdr_dev) );
	
	//获取光盘信息
	if( Xkd_DVDSDK_GetDiscInfo(cdr_dev, &pDiscInfo) != ERROR_DVD_OK )
	{
		printf("Xkd_DVDSDK_GetDiscInfo is failed!\n");
		goto UNLOADDEV;		
	}
	//打印光盘类型
	printf(" disk info Media type:%d \n", pDiscInfo.ntype );
	printf(" disk info   maxpeed :%d \n", pDiscInfo.maxpeed );
	printf(" disk info discsize:%d MB\n", pDiscInfo.discsize );
	printf(" disk info usedsize:%d MB\n", pDiscInfo.usedsize );
	printf(" disk info freesize:%d MB\n", pDiscInfo.freesize );


	//判断光盘是否可写
	if( Xkd_DVDSDK_DiscCanWrite(cdr_dev) != ERROR_DVD_OK )
	{
		printf("Disc Can not be Writed! \n");
		goto UNLOADDEV;	
	}
	printf(" disk info is new , can be writed!\n" );
	
#if 0	
	 //默认可以不用设置，默认速度是可以满足实时性需要
	//设定光盘刻录8倍速
	if( Xkd_DVDSDK_SetWriteSpeed(cdr_dev, 12, pDiscInfo.ntype) != ERROR_DVD_OK )
	{
		printf("SetWriteSpeed is failed! \n");
		goto UNLOADDEV;		
	}
	printf("SetWriteSpeed is 8 , disktype = %d \n", pDiscInfo.ntype);
#endif	
	
#if 0	//默认不创建目录，则文件就会在光盘根目录下
	//创建媒体存放目录
	pMediaDir = Xkd_DVDSDK_CreateDir(cdr_dev, "/Media");
	if( !pMediaDir )
	{
		printf("CreateDir is failed! \n");
		goto UNLOADDEV;		
	}
	printf("CreateDir /Media is ok! \n");
#endif


	//格式化光盘，对于DVD+R、DVD_R类型，会准备刻录的数据和对光盘做预处理
	Xkd_DVDSDK_FormatDisc(cdr_dev,"dvdtest");
	printf("Xkd_DVDSDK_FormatDisc  is ok! \n");
	
	sleep(1);
	//刻录前，预热启动
	//Xkd_DVDSDK_FillEmptyDataOnFirst(cdr_dev, 0); 
	//printf("Xkd_DVDSDK_FillEmptyDataOnFirst  is ok! \n");
	
	//创建文件,实时刻录时filesize可以为0，光盘备份刻录时，必须是文件的实际大小
	tmpFile = Xkd_DVDSDK_CreateFile(cdr_dev, pMediaDir,"cd1.mkv", 0);
	if( !tmpFile )
	{
		printf("Xkd_DVDSDK_CreateFile is fail! \n");
	}
	printf("Xkd_DVDSDK_CreateFile  is ok! \n");
	//设定文件位置 (写文件前都需要调用这个函数)
	Xkd_DVDSDK_SetFileLoca(cdr_dev, tmpFile);
	printf("Xkd_DVDSDK_SetFileLoca  is ok! \n");
	//刻录文件，模拟实时刻录，从文件中读取数据，刻录到光盘上
	WriteFileToDisk(tmpFile,"cd1.mkv");
	printf("WriteFileToDisk cd1.mkv is ok! \n");
	//关闭文件
	Xkd_DVDSDK_CloseFile(cdr_dev,tmpFile);
	printf("Xkd_DVDSDK_CloseFile  is ok! \n");
	
	//创建文件
	tmpFile = Xkd_DVDSDK_CreateFile(cdr_dev, pMediaDir,"cd2.mkv", 0);
	//设定文件位置 (写文件前都需要调用这个函数)
	Xkd_DVDSDK_SetFileLoca(cdr_dev, tmpFile);	
	//刻录文件，模拟实时刻录，从文件中读取数据，刻录到光盘上
	WriteFileToDisk(tmpFile,"cd2.mkv");
	//关闭文件
	Xkd_DVDSDK_CloseFile(cdr_dev,tmpFile);	

	//封盘，封闭后光盘不能再写入了
	Xkd_DVDSDK_CloseDisc(cdr_dev);
	printf("CloseDisc is ok! \n");
	
	sleep(2);

	
UNLOADDEV:	
    //解锁
	Xkd_DVDSDK_LockDoor(cdr_dev, 0);

	//卸载dvd光驱
	Xkd_DVDSDK_UnLoad(cdr_dev);
	
	return 0;
}
