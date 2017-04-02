#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Burn_SDK.h>
// #include <RingBuffer.h>
#include <Database.h>

#include "BurnSelfInclude.h"

//int create_dev_database(DEV_HANDLE hBurnDEV);
//int set_tray_state(DEV_HANDLE hBurnDEV, DEV_TRAY_STAT state);
//int set_burn_state(DEV_HANDLE hBurnDEV, BURN_STAT state);


int read_sys_version_info(char *file_path, char *buf)
{
	int size;
	FILE *fd;

	fd = fopen(file_path, "r");
	if (fd == NULL)
	{
		printf("open file error!\n");
		return -1;
	}
	size =  fread(buf, 1, 1024, fd);
	if (size <= 0)
	{
		printf("read file error!\n");
		fclose(fd);
		return -1;
	}
	buf[size] = '\0';
	fclose(fd);

	return 0;
}

int extract_str(const char *head,  char *end, 
		 char *src,  char *buffer)
{
	int i = 0;
	int hn = 0, en = 0, sn = 0;
	 char *hp, *ep;

	hn = strlen(head);
	en = strlen(end);
	sn = strlen(src);
	hp = strstr(src, head);
	ep = strstr(src, end);

	if ((hp != NULL) && (ep != NULL) && (hp < ep))
	{
		hp = hp + hn;
		while(hp < ep)
		{
			*(buffer+i) = *(hp++);
			i++;
		}
	}
	else
	{
		printf("error :strings head or end \n");
		return -1;
	}

	*(buffer+i) = '\0';

	return 0;
}

int Burn_Get_DeviceNum(DEV_SYS_INFO_T *dev_sys_info_ptr)
{
	if(dev_sys_info_ptr == NULL)
	{
		printf("[Burn_Get_DeviceNum] dev_sys_info_ptr Is NULL\n");	
		return BURN_FAILURE;
	}
#ifdef LINUX
	char buf[2000];

	system("./Get_CDRom_Dev_Info.sh");

	if(read_sys_version_info("CDRom_List", buf) != 0)
	{
		printf("[Burn_Get_DeviceNum] read_sys_version_info Is Failed\n");	
		return BURN_FAILURE;
	}

	dev_sys_info_ptr->dev_num = 0;
	if(extract_str("<dev1>", "</dev1>", buf, dev_sys_info_ptr->dev1) == 0)
	{
		dev_sys_info_ptr->dev_num++;
		printf("dev1 = %s\n", dev_sys_info_ptr->dev1);
	}

	if(extract_str("<dev2>", "</dev2>", buf, dev_sys_info_ptr->dev2) == 0)
	{
		dev_sys_info_ptr->dev_num++;
		printf("dev2 = %s\n", dev_sys_info_ptr->dev2);
	}

	if(extract_str("<dev3>", "</dev3>", buf, dev_sys_info_ptr->dev3) == 0)
	{
		dev_sys_info_ptr->dev_num++;
		printf("dev3 = %s\n", dev_sys_info_ptr->dev3);
	}

	if(extract_str("<dev4>", "</dev4>", buf, dev_sys_info_ptr->dev4) == 0)
	{
		dev_sys_info_ptr->dev_num++;
		printf("dev4 = %s\n", dev_sys_info_ptr->dev4);
	}		
#endif

#ifdef WINDOWS
    BURNHANDLE handle=NULL;
    handle=CreateBurnHandle();
	dev_sys_info_ptr->dev_num = GetDeviceCount(handle);
    DestroyBurnHandle(handle);
#endif

	return BURN_SUCCESS;
}

/*光驱设备初始化*/
DEV_HANDLE Burn_Dev_Init(const char *DevName)
{
    DEV_HANDLE       hBurnDEV;
	BURN_DEV_INFO_T *b_dev_info_ptr;
	static int Init_Dev_Cnt = 0;
	static int Device_Cnt = 0;
    int ret;
	
	//为句柄申请空间
#if 1
    hBurnDEV = (DEV_HANDLE)calloc(1, sizeof(struct Dev_Object));
    if (hBurnDEV == NULL) 
    {
        printf("Failed to allocate space for Dev Object\n");
        return NULL;
    }
#endif


#ifdef LINUX
	if(DevName == NULL)//空指针
	{
		printf("Device Name Is NULL Error\n");	
		free(hBurnDEV);
		return NULL;
	}

    if(strlen(DevName) < 4)//“/dev”后面没东西
    {
        printf("0000000000000000000000\n");
		free(hBurnDEV);
        return NULL;
    }

	memcpy(hBurnDEV->dev_path, DevName, strlen(DevName));
	/* DVDSDK 加载设备并返回句柄 */
    hBurnDEV->hDVD = Xkd_DVDSDK_Load(DevName);
    if (hBurnDEV->hDVD == NULL) 
    {
        printf("Failed to Load CD-ROM\n");
		free(hBurnDEV);
        return NULL;
    }

#if 0
//	Burn_Ctrl_DevTray(hBurnDEV, 1);
	ret = Xkd_DVDSDK_GetTrayState(hBurnDEV->hDVD);
	printf("<<<<<< %d >>>>>>>\n", ret);
	ret = Xkd_DVDSDK_HaveDisc(hBurnDEV->hDVD);
	printf("<<<<<< %d >>>>>>>\n", ret);
#endif

	//获取光驱设备信息
	// 光驱设备信息详见DVDSDK.h
	//typedef struct
	//{
	//	char szVender[128];					// 厂家名称
	//	int  drvtype;						// 光驱类型			DVDDRIVER_TYPE
	//	int  discsupts;						// 光盘支持总数
	//	unsigned char disclist[255];		// 光盘里类型列表，见DVDDISC_TYPE
	//}LVDVD_DEV_INFO_T;

    DVD_DEV_INFO_T pDevInfo0;
	if( Xkd_DVDSDK_GetDevInfo(hBurnDEV->hDVD, &pDevInfo0) != ERROR_DVD_OK )
	{
		printf("Xkd_DVDSDK_GetDevInfo is failed\n");
	}
	else
	{
		//打印设备信息
    	printf("====================================================\n");
	    printf("DVD Record Info:\n");
		printf(" Manufacturer is  [%s] \n", pDevInfo0.szVender);
		printf(" Device Type  is  [%d] \n", pDevInfo0.drvtype);
		printf(" Support Disk Num [%d] \n", pDevInfo0.discsupts);
	    printf("====================================================\n");
	}

	if(pDevInfo0.drvtype != DVDDRIVER_DVDRW)//光驱不可写
	{
		printf("Dev [%s] Is Can Not Write Disc ...\n", DevName);	
		free(hBurnDEV);
		return NULL;
	}
#endif

#ifdef WINDOWS
//    if(Init_Dev_Cnt == 0)
    {
    	hBurnDEV->hDVD = CreateBurnHandle();
        if (hBurnDEV->hDVD == NULL) 
        {
            printf("Failed to Load CD-ROM\n");
			free(hBurnDEV);
            return NULL;
        }
    }

	ScanDevice(hBurnDEV->hDVD);

	ret = GetDeviceCount(hBurnDEV->hDVD);
	printf("Dev Count Is [%d]\n", ret);
	//如果已初始化的设备数大于或等于检测到的光驱数，则不能初始化
	if(Init_Dev_Cnt >= ret)
	{
		printf("Init Dev Num [%d], It Is Greater than Dev Num [%d]\n", Init_Dev_Cnt, ret);
		free(hBurnDEV);
		return NULL;
	}

	//获取设备信息
	BURN_DEVICE_INFORMATION devInfo;
	GetDeviceInfo(hBurnDEV->hDVD, Init_Dev_Cnt, devInfo);

	//打印设备信息
   	printf("====================================================\n");
    printf("DVD Record Info:\n");
	printf(" szVender is      [%s] \n", devInfo.szVender);
	printf(" szIdentifier is  [%s] \n", devInfo.szIdentifier);
	printf(" szName is        [%s] \n", devInfo.szName);
	printf(" szRevision is    [%s] \n", devInfo.szRevision);
    printf("====================================================\n");

    if(Init_Dev_Cnt == 0)
    	 CreateDataProject(hBurnDEV->hDVD);

     Init_Dev_Cnt++;
#endif

	 /* 创建数据库，并保存光驱信息 */
    ret = create_dev_database(hBurnDEV);
    if(ret == BURN_SUCCESS)
	{
		//配置光驱信息
		b_dev_info_ptr = get_dev_info(hBurnDEV);
#ifdef LINUX
		memcpy(b_dev_info_ptr->szVender, pDevInfo0.szVender, sizeof(b_dev_info_ptr->szVender));
#endif

#ifdef WINDOWS
		memcpy(b_dev_info_ptr->szVender, devInfo.szVender, sizeof(b_dev_info_ptr->szVender));
#endif
	   	printf("====================================================\n");
    	printf("DVD Record Info2:\n");
		printf(" Manufacturer is  [%s] \n", b_dev_info_ptr->szVender);
    	printf("====================================================\n");		


		return hBurnDEV;
	}
    else
	{
		free(hBurnDEV);
        return NULL;
	}
}

int	Burn_Ctrl_DevTray(DEV_HANDLE hBurnDEV, DEV_TRAY_STAT bOpen)
{
    int ret;


#ifdef LINUX		
    if(bOpen == B_DEV_TRAY_OPEN)
        ret = Xkd_DVDSDK_Tray(hBurnDEV->hDVD, BURN_TRUE);
    else
		ret = Xkd_DVDSDK_Tray(hBurnDEV->hDVD, BURN_FLASE);
#endif

#ifdef WINDOWS
    if(bOpen == B_DEV_TRAY_OPEN)
	{
        EjectDisc(hBurnDEV->hDVD, hBurnDEV->dev_id);
		ret = BURN_SUCCESS;
	}
    else
		printf("No Supprot Dev Tray\n");
#endif

	if (ret == BURN_SUCCESS) 
	{
		set_tray_state(hBurnDEV, bOpen);
		return BURN_SUCCESS;
	}

    printf("Failed to Ctrl Tray\n");
    return BURN_FAILURE;
}

int Burn_Ctrl_LoadDisc(DEV_HANDLE hBurnDEV)
{
    int ret=0;


#ifdef LINUX		
	ret = Xkd_DVDSDK_LoadDisc(hBurnDEV->hDVD);
	if (ret == BURN_SUCCESS) 
		return BURN_FAILURE;
#endif

#ifdef WINDOWS		
	LoadDisc(hBurnDEV->hDVD, hBurnDEV->dev_id);
	return BURN_FAILURE;
#endif

    printf("Failed to Load Disc\n");
    return BURN_FAILURE;
}

int Burn_Ctrl_LockDoor(DEV_HANDLE hBurnDEV, int bLocked)
{
    int ret = BURN_SUCCESS;

#ifdef LINUX		
    ret = Xkd_DVDSDK_LockDoor(hBurnDEV->hDVD, bLocked);
    if (ret != 0) 
        ret = BURN_FAILURE;
#endif
    return ret;
}

int Burn_Ctrl_FormatDisc(DEV_HANDLE hBurnDEV, char *szDiscName)
{
    int ret=0;


#ifdef LINUX		
	ret = Xkd_DVDSDK_FormatDisc(hBurnDEV->hDVD, szDiscName);
	if (ret == BURN_SUCCESS) 
		return BURN_SUCCESS;
#endif

#ifdef WINDOWS		
//	ret = FormatDisc(hBurnDEV->hDVD, hBurnDEV->dev_id);
	//if (ret == BURN_TURE) 
		return BURN_SUCCESS;
#endif

    printf("Failed to Format Disc\n");
    return BURN_FAILURE;
}

int Burn_Ctrl_StartBurn(DEV_HANDLE hBurnDEV, INTERFACE_TYPE type)
{
    int ret=0;


	printf("Burn Ctrl Start Burn\n");
	if(type == B_STREAM)
	{
		hBurnDEV->interface1 = type;
		set_burn_state(hBurnDEV, B_START);
#ifdef LINUX
//                    Start_Burn_Stream_Buf(hBurnDEV);
#endif

#ifdef WINDOWS
		printf("Windows No Supprot Burn Stream\n");
#endif
	}
	else if(type == B_FILE)
	{
		hBurnDEV->interface1 = type;
		set_burn_state(hBurnDEV, B_START);
#ifdef LINUX
		printf("Start Linux Burn...\n");
		ret = Start_Burn_Local_List(hBurnDEV);
#endif

#ifdef WINDOWS
		printf("Start Windows Burn Dev[%d] Handle[%x][%x]...\n", hBurnDEV->dev_id, hBurnDEV, hBurnDEV->hDVD);
		BURN_RUN_STATE_T *b_run_state_ptr;
        BURN_DEVICE_INFORMATION devInfo;

		b_run_state_ptr = get_run_state(hBurnDEV);
		GetDeviceInfo(hBurnDEV->hDVD, hBurnDEV->dev_id, devInfo);

        //打印设备信息
        printf("====================================================\n");
        printf("DVD Record Info:\n");
        printf(" Manufacturer is  [%s] \n", devInfo.szVender);
        printf("====================================================\n");

		StartBurn(hBurnDEV->hDVD, hBurnDEV->dev_id);


        BURN_STATE_INFORMATION burnStateInfo;

        while (1)
        {
            bzero(burnStateInfo.szBurnStateDescription, sizeof(burnStateInfo.szBurnStateDescription));
            GetBurnStateInfo(hBurnDEV->hDVD, burnStateInfo);
//            printf("%s\n", burnStateInfo.szBurnStateDescription);
			if(b_run_state_ptr != NULL)
			{
				bzero(b_run_state_ptr->describe, sizeof(b_run_state_ptr->describe));
				memcpy(b_run_state_ptr->describe, burnStateInfo.szBurnStateDescription, sizeof(b_run_state_ptr->describe));
			}

            if(burnStateInfo.nState == -1)
            {
                printf("Burn State Return Failed\n");
				Burn_Call_CallBack(hBurnDEV, B_DEV_TRAY_CLOSE, B_ABNORMAL, ERR_WRITE_DISC_FAILED);
    			set_burn_state(hBurnDEV, B_STOP);
                return BURN_FAILURE;
            }
            else if(burnStateInfo.nState == 2)
            {
                printf("Burn State Return Burn OK\n");
    			set_burn_state(hBurnDEV, B_STOP);
                return BURN_SUCCESS;
            }
            else
            {
                printf("Burn State Return Burning\n");
            }

            sleep(10000);
        }

#endif		
	}
	else
		return BURN_FAILURE;


    set_burn_state(hBurnDEV, B_STOP);
    return ret;
}

int Burn_Ctrl_StopBurn(DEV_HANDLE hBurnDEV)
{
    int ret=0;

    set_burn_state(hBurnDEV, B_STOP);
    printf("Burn Ctrl to Stop Burn hBurnDEV:%p\n",(void*)hBurnDEV);
    return BURN_SUCCESS;
}


int Burn_Ctrl_CopyDisc(DEV_HANDLE hDevSrc, DEV_HANDLE hDevDst)
{
    return BURN_SUCCESS;
}

int Burn_Ctrl_ResumeDisc(DEV_HANDLE hBurnDEV, char *DiscName, char *DirName, char *FileName, int FillSize)
{
    return BURN_SUCCESS;
}

int Burn_Ctrl_GetReserveData(DEV_HANDLE hBurnDEV, unsigned char **pBuffer, int *pSize)
{
    return BURN_SUCCESS;
}

int Burn_Ctrl_GetReserveBuffer(DEV_HANDLE hBurnDEV, unsigned char **pBuffer, int *pSize)
{
    return BURN_SUCCESS;
}

int Burn_Ctrl_CloseDisc(DEV_HANDLE hBurnDEV)
{
    int ret=0;

	printf("dev[%d] enter Burn_Ctrl_CloseDisc\n",hBurnDEV->dev_id);
#if 0
	if(Get_Running_Burn_State(hBurnDEV) == B_CLOSED)
	{
		printf("BurnServer Is Closed Now\n");
		return BURN_FAILURE;	
	}
#endif

	set_burn_state(hBurnDEV, B_CLOSED);

#ifdef LINUX
    Xkd_DVDSDK_CloseDisc(hBurnDEV->hDVD);
#endif

#ifdef WIN32
    CloseDataProject(hBurnDEV->hDVD);
//	DestroyBurnHandle(hBurnDEV->hDVD);
#endif
   
	BURN_DATA_T *data_ptr;

    data_ptr = get_burn_data(hBurnDEV);
    if(data_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }
	printf("");
    ret = Ring_Buffer_Release(data_ptr->hBuf);
	if (ret == BURN_SUCCESS)
	{
		printf("Ring_Buffer_Release Success!\n");
	}

	printf("Burn_Ctrl_CloseDisc Success Return\n");
	return BURN_SUCCESS;

}

/*
 * 文件操作类
 */
#if 0
Xkd_DVDSDK_DIR Burn_Ctrl_CreateDir(DEV_HANDLE hBurnDEV, char *DirName)
{
    int ret;
    char name[300];

    bzero(name, sizeof(name));
    strcpy(name, DirName);

    switch(hBurnDEV->kernel)
    {
        case BURN_LINUX:
                printf("BurnDir == %s\n", name);
                return Xkd_DVDSDK_CreateDir(hBurnDEV->hDVD, name);

            break;

        case BURN_WINDOWS:
            break;

        case BURN_EPSON:
            break;

        case BURN_PRIMERA:
            break;

        default:
            printf("Kernel Type [%d] Is Err", hBurnDEV->kernel);
            break;
    }

    printf("Failed to Create Dir\n");
    return NULL;
}
#endif

FILE_HANDLE Burn_Ctrl_CreateFile(DEV_HANDLE hBurnDEV, DIR_HANDLE hBurnDir, char *file_name)
{
    int  ret=0;
    char name[300];
	FILE_HANDLE hFile=NULL;

    bzero(name, sizeof(name));
    strcpy(name, file_name);


	printf("hBurnDir == %s, file_name == %s\n", hBurnDir, name);
#ifdef LINUX			
	hFile = Xkd_DVDSDK_CreateFile (hBurnDEV->hDVD, hBurnDir, name, 0);
	if(!hFile)
	{
		printf("Get File Handle Failed\n");
		return NULL;
	}

	//设定文件位置 (写文件前都需要调用这个函数)
	Xkd_DVDSDK_SetFileLoca(hBurnDEV->hDVD, hFile);
	printf("Xkd_DVDSDK_SetFileLocal  is ok! \n");

	return hFile;
#endif

    printf("Failed to Create File\n");
    return NULL;
}

#define MB_SIZE  1*1024*1024
int	Burn_Ctrl_WriteData(DEV_HANDLE hBurnDEV, FILE_HANDLE hBurnFile, char *buf, int size)
{
    int ret=0;
	BURN_DISC_INFO_T *b_disc_info_ptr;
	
#if 1
	b_disc_info_ptr = get_disc_info(hBurnDEV);
	if(b_disc_info_ptr != NULL)
	{
		b_disc_info_ptr->writecnt  += size;	

		if(b_disc_info_ptr->writecnt >= MB_SIZE)
		{
			b_disc_info_ptr->writecnt -= MB_SIZE;	
			b_disc_info_ptr->usedsize++;
			b_disc_info_ptr->freesize--;
		}
	}
#endif
#ifdef LINUX		
   	return Xkd_DVDSDK_WriteData (hBurnDEV->hDVD, hBurnFile, buf, size);
#endif


    printf("Failed to Write Data\n");
    return BURN_FAILURE;
}

int	Burn_Ctrl_CloseFile(DEV_HANDLE hBurnDEV, FILE_HANDLE hBurnFile)
{

#ifdef LINUX		
	return Xkd_DVDSDK_CloseFile(hBurnDEV->hDVD, hBurnFile);
#endif


    printf("Failed to Close File\n");
    return BURN_FAILURE;
}

int	Burn_Ctrl_AddBurnLocalFile(DEV_HANDLE hBurnDEV, char *burn_dir, char *file_path)
{
    int i;
    BURN_DATA_T *data_ptr;

#ifdef WINDOWS
	if(AddFile(hBurnDEV->hDVD, NULL, file_path) == BURN_TRUE)
	{
		printf("================ Add File [%s] Success ===================\n", file_path);	
		return BURN_SUCCESS;
	}
	else
		return BURN_FAILURE;
#endif

    data_ptr = get_burn_data(hBurnDEV);

    for(i = 0; i < DATA_INFO_SIZE; i++)
    {
        if(data_ptr->file_wait_list[i][0] == NULL)
        {
            printf("File List Add [%s] To ID [%d]\n", file_path, i);

            //为刻录文件队列指针申请内存，并将路径复制到内存中
            data_ptr->file_wait_list[i][0] = (char*)calloc(1, FILE_PATH_MAX);
            if(data_ptr->file_wait_list[i][0] == NULL)
            {
                printf("File Wait List Get Mem Failed\n");
                return BURN_FAILURE;
            }
            memcpy(data_ptr->file_wait_list[i][0], file_path, strlen(file_path));

            //为刻录文件刻录在光盘上的路径队列指针申请内存，并将路径复制到内存中
            if(burn_dir != NULL)
            {
                data_ptr->file_wait_list[i][1] = (char*)calloc(1, FILE_PATH_MAX);
                if(data_ptr->file_wait_list[i][1] == NULL)
                {
                    printf("File Wait List Get Mem Failed\n");
                    return BURN_FAILURE;
                }
                memcpy(data_ptr->file_wait_list[i][1], burn_dir, strlen(burn_dir));
            }
            data_ptr->file_wait_num++;
            return BURN_SUCCESS;
        }

    }
    
    printf("File Wait List Is Full\n");
    return BURN_FAILURE;
}

int	Burn_Ctrl_DelBurnLocalFile(DEV_HANDLE hBurnDEV, char *file_path)
{
    int i;
    BURN_DATA_T *data_ptr;

#ifdef WINDOWS
		return BURN_SUCCESS;
#endif

    data_ptr = get_burn_data(hBurnDEV);

    printf("In Burn_Ctrl_DelBurnLocalFile\n");
    for(i = 0; i < DATA_INFO_SIZE; i++)
    {
        if(data_ptr->file_wait_list[i][0] != NULL)
        {
            printf("List = %s, find = %s\n", data_ptr->file_wait_list[i][0], file_path);
            if(strcmp(data_ptr->file_wait_list[i][0], file_path) == 0)
            {
                printf("[Burn_Ctrl_DelBurnLocalFile] Del Wait File ID = %d\n", i);
                free(data_ptr->file_wait_list[i][0]);
                free(data_ptr->file_wait_list[i][1]);
                printf("[Burn_Ctrl_DelBurnLocalFile] After Del Ptr = %s\n", data_ptr->file_wait_list[i][0]);
                data_ptr->file_wait_list[i][0] = NULL;
                data_ptr->file_wait_list[i][1] = NULL;

                data_ptr->file_wait_num--;
                return BURN_SUCCESS;
            }
        }
    }
    
    printf("File Wait List No Found File\n");
    return BURN_FAILURE;
}


int	Burn_Ctrl_AddBurnLocalDir(DEV_HANDLE hBurnDEV, char *burn_dir, char *dir_path)
{
    int i;
    BURN_DATA_T *data_ptr;

#ifdef WINDOWS
	if( AddDirectory(hBurnDEV->hDVD, NULL, dir_path) != NULL)
	{
		printf("============== Add Dir [%s] Success==============\n", dir_path);	
		return BURN_SUCCESS;
	}
	else
		return BURN_FAILURE;
#endif

    data_ptr = get_burn_data(hBurnDEV);

    for(i = 0; i < DATA_INFO_SIZE; i++)
    {
        if(data_ptr->dir_wait_list[i][0] == NULL)
        {
            //为刻录目录队列指针申请内存，并将路径复制到内存中
            data_ptr->dir_wait_list[i][0] = (char*)calloc(1, FILE_PATH_MAX);
            if(data_ptr->dir_wait_list[i][0] == NULL)
            {
                printf("File Wait List Get Mem Failed\n");
                return BURN_FAILURE;
            }
            memcpy(data_ptr->dir_wait_list[i][0], dir_path, strlen(dir_path));

            //为刻录文件刻录在光盘上的路径队列指针申请内存，并将路径复制到内存中
            if(burn_dir != NULL)
            {
                data_ptr->dir_wait_list[i][1] = (char*)calloc(1, FILE_PATH_MAX);
                if(data_ptr->dir_wait_list[i][1] == NULL)
                {
                    printf("File Wait List Get Mem Failed\n");
                    return BURN_FAILURE;
                }
                memcpy(data_ptr->dir_wait_list[i][1], burn_dir, strlen(burn_dir));
            }

            data_ptr->dir_wait_num++;
            return BURN_SUCCESS;
        }
    }
    
    printf("Dir Wait List Is Full\n");
    return BURN_FAILURE;
}

int	Burn_Ctrl_DelBurnLocalDir(DEV_HANDLE hBurnDEV, char *dir_path)
{
    int i;
    BURN_DATA_T *data_ptr;

    printf("In Burn_Ctrl_DelBurnLocalDir \n");

#ifdef WINDOWS
		return BURN_SUCCESS;
#endif
			
    data_ptr = get_burn_data(hBurnDEV);

    for(i = 0; i < DATA_INFO_SIZE; i++)
    {
        if(data_ptr->dir_wait_list[i][0] != NULL)
        {   
            printf("[Burn_Ctrl_DelBurnLocalDir]List = %s, find = %s\n", data_ptr->dir_wait_list[i][0], dir_path);
            if(strcmp(data_ptr->dir_wait_list[i][0], dir_path) == 0)
            {
                printf("[Burn_Ctrl_DelBurnLocalDir] Free Dir Mem\n");
                free(data_ptr->dir_wait_list[i][0]);
                free(data_ptr->dir_wait_list[i][1]);
                data_ptr->dir_wait_list[i][0] = NULL;
                data_ptr->dir_wait_list[i][1] = NULL;
                data_ptr->dir_wait_num--;
                return BURN_SUCCESS;
            }
        }
    }
    
    printf("Dir Wait List No Found Dir\n");
    return BURN_FAILURE;
}

int Get_Dir_Wait_List_Num(DEV_HANDLE hBurnDEV)
{
    BURN_DATA_T *data_ptr;

    printf("In Get_Dir_Wait_List_Num \n");
    data_ptr = get_burn_data(hBurnDEV);

    return data_ptr->dir_wait_num;
}

int	Burn_Ctrl_AddCompletedFile(DEV_HANDLE hBurnDEV, char *file_path)
{
    int i;
    BURN_DATA_T *data_ptr;

    data_ptr = get_burn_data(hBurnDEV);

    for(i = 0; i < DATA_INFO_SIZE; i++)
    {
        if(data_ptr->file_burn_list[i] == NULL)
        {
            data_ptr->file_burn_list[i] = file_path;
            data_ptr->file_burn_num++;
            return BURN_SUCCESS;
        }
    }
    
    printf("File Wait List Is Full\n");
    return BURN_FAILURE;
}

int	Burn_Ctrl_DelCompletedFile(DEV_HANDLE hBurnDEV, char *file_path)
{
    int i;
    BURN_DATA_T *data_ptr;

    data_ptr = get_burn_data(hBurnDEV);

    for(i = 0; i < DATA_INFO_SIZE; i++)
    {
        if(data_ptr->file_burn_list[i] != NULL)
        {
            if(strcmp(data_ptr->file_burn_list[i], file_path) == 0)
            {
                data_ptr->file_burn_list[i] = NULL;
                data_ptr->file_burn_num--;
                return BURN_SUCCESS;
            }
        }
    }
    
    printf("File Wait List No Found File\n");
    return BURN_FAILURE;
}

int Burn_Ctrl_Update_Burning_Project(DEV_HANDLE hBurnDEV)
{
#ifdef WINDOWS
#if 0
    printf("Update Windows Handle\n");
    DestroyBurnHandle(hBurnDEV->hDVD);

    hBurnDEV->hDVD = CreateBurnHandle();
    if (hBurnDEV->hDVD == NULL) 
    {
        printf("Failed to Create Burn Handle\n");
        return BURN_FAILURE;
    }

    ScanDevice(hBurnDEV->hDVD);

    CreateDataProject(hBurnDEV->hDVD);
#endif
#endif

    return BURN_SUCCESS;
}


DEV_TRAY_STAT Burn_Ctrl_Dev_Get_TrayState(DEV_HANDLE hBurnDEV)
{
#ifdef LINUX
	if( Xkd_DVDSDK_GetTrayState(hBurnDEV->hDVD) ) 
	{
		printf(" DVD Tray Is Open, You Must Close It!\n");
        return B_DEV_TRAY_OPEN;
//		Xkd_DVDSDK_Tray(hBurnDEV->hDVD, 0); //关闭光驱
	}
#endif

	return B_DEV_TRAY_CLOSE;	
}

int Burn_Ctrl_Dev_Get_HaveDisc(DEV_HANDLE hBurnDEV)
{
	printf("Is Have Disc?\n");
#ifdef LINUX
	if( Xkd_DVDSDK_HaveDisc(hBurnDEV->hDVD) != 1 )
	{
		printf(" Cdrom Have Not Disc!\n");
//		Xkd_DVDSDK_Tray(hBurnDEV->hDVD, 1);  //打开光驱
        return ERR_NO_HAVE_DISC;
	}
    else
    {
        printf(" CD-Rom Have Disc...\n");
	    printf(" MediaExactType:%d \n", Xkd_DVDSDK_GetMediaExactType(hBurnDEV->hDVD) );
    }
#endif

#ifdef WINDOWS
	if( HaveDisc(hBurnDEV->hDVD, hBurnDEV->dev_id) != BURN_TRUE )
	{
		printf(" Cdrom Have Not Disc!\n");
//		EjectDisc(hBurnDEV->hDVD, hBurnDEV->dev_id);  //打开光驱
        return ERR_NO_HAVE_DISC;
	}
    else
    {
        printf(" CD-Rom Have Disc...\n");
    }
#endif

	return BURN_SUCCESS;	
}

int Burn_Ctrl_Dev_LoadDisc(DEV_HANDLE hBurnDEV)
{
	BURN_DISC_INFO_T *b_disc_info_ptr;

	printf("Start Load Disc ... \n");
#ifdef LINUX	
    if(Xkd_DVDSDK_LoadDisc(hBurnDEV->hDVD) != 0)
	{
		printf("Can't Load Disc ...\n");	
		return ERR_LOAD_DISC_FAILED;
	}
#endif

#ifdef WINDOWS
	LoadDisc(hBurnDEV->hDVD, hBurnDEV->dev_id);
#endif	

	printf("Start Get Disc Info ...\n");

#ifdef LINUX
	printf("Get Disc Info\n");
	LVDVD_DISC_INFO_T pDiscInfo;
	if( Xkd_DVDSDK_GetDiscInfo(hBurnDEV->hDVD, &pDiscInfo) != ERROR_DVD_OK )
	{
		printf("Xkd_DVDSDK_GetDiscInfo is failed!\n");
        return ERR_GET_DISC_INFO_FAILED;
	}
	else
	{
#if 0
		//打印光盘类型
    	printf("Disk Info:\n");
		printf(" Disc Media type [%d] \n", pDiscInfo.ntype );
		printf(" Disc Maxpeed    [%d] \n", pDiscInfo.maxpeed );
		printf(" Disc Discsize   [%d] MB\n", pDiscInfo.discsize );
		printf(" Disc Usedsize   [%d] MB\n", pDiscInfo.usedsize );
		printf(" Disc Freesize   [%d] MB\n", pDiscInfo.freesize );  
#endif
		//判断光盘容量是否获取正确，如果错误反馈失败
		if(pDiscInfo.discsize == 0)
		{
			printf("Disc Size Info Is Error\n");	
			return ERR_GET_DISC_INFO_FAILED;
		}

		b_disc_info_ptr = get_disc_info(hBurnDEV);
		if(b_disc_info_ptr != NULL)
		{
			b_disc_info_ptr->has_disc  = BURN_TRUE;	
			b_disc_info_ptr->write_flag= BURN_TRUE;	
			b_disc_info_ptr->type      = pDiscInfo.ntype;	
			b_disc_info_ptr->maxspeed  = pDiscInfo.maxpeed;	
			b_disc_info_ptr->discsize  = pDiscInfo.discsize;	
			b_disc_info_ptr->usedsize  = 0;	
			b_disc_info_ptr->writecnt  = 0;	
			b_disc_info_ptr->freesize  = pDiscInfo.discsize;	
			strcpy(b_disc_info_ptr->disc_name, "iBurn");

			//如果报警门限没有设置或设置值大于光盘容量，则使用默认配置
			if((b_disc_info_ptr->alarmsize <= 500) || (b_disc_info_ptr->alarmsize >= b_disc_info_ptr->discsize))
			{
  				//为报警设置500M默认配置
				printf("Set Disc Alarm To Default [500] MB\n");
				b_disc_info_ptr->alarmsize = 500;	
			}

            //如果报警预警门限没有设置或设置值大于光盘容量，则使用默认配置
            if((b_disc_info_ptr->alarmwarningsize <= 0) || (b_disc_info_ptr->alarmwarningsize + b_disc_info_ptr->alarmsize >= b_disc_info_ptr->discsize))
            {
                //为报警预警设置0M默认配置
                printf("Set Disc Alarm Waring To Default [0] MB\n");
                b_disc_info_ptr->alarmwarningsize = 0;	
            }
            
		}
	}
#endif
    
#ifdef WIN32
	DISC_INFORMATION discInfo;
	bool ret = GetDiscInfo(hBurnDEV->hDVD, hBurnDEV->dev_id, discInfo);
	if(ret != BURN_TRUE)
	{
		printf("Get Disc Info Failed\n");	
		return ERR_GET_DISC_INFO_FAILED;
	}

		b_disc_info_ptr = get_disc_info(hBurnDEV);
		if(b_disc_info_ptr != NULL)
		{
			b_disc_info_ptr->has_disc  = BURN_TRUE;	
			b_disc_info_ptr->write_flag= BURN_TRUE;	
			b_disc_info_ptr->type      = discInfo.uiDiscType;	
			b_disc_info_ptr->maxspeed  = 16;	
			b_disc_info_ptr->discsize  = discInfo.uiFreeBytes/(1024*1024);	
			b_disc_info_ptr->usedsize  = 0;	
			b_disc_info_ptr->writecnt  = 0;	
			//将Byte转换为MB
			b_disc_info_ptr->freesize  = discInfo.uiFreeBytes/(1024*1024);	
			strcpy(b_disc_info_ptr->disc_name, "iBurn");

			//如果报警门限没有设置或设置值大于光盘容量，则使用默认配置
			if((b_disc_info_ptr->alarmsize <= 500) || (b_disc_info_ptr->alarmsize > b_disc_info_ptr->discsize))
			{
				//为报警设置500M默认配置
				printf("Set Disc Alarm To Default [500] MB\n");
				b_disc_info_ptr->alarmsize = 500;	
			}

            //如果报警预警门限没有设置或设置值大于光盘容量，则使用默认配置
            if((b_disc_info_ptr->alarmwarningsize <= 0) || (b_disc_info_ptr->alarmwarningsize + b_disc_info_ptr->alarmsize >= b_disc_info_ptr->discsize))
            {
                //为报警预警设置0M默认配置
                printf("Set Disc Alarm Waring To Default [0] MB\n");
                b_disc_info_ptr->alarmwarningsize = 0;	
            }
	}
#endif
	
	return BURN_SUCCESS;
}

int Burn_Ctrl_Dev_Get_DiscCanWrite(DEV_HANDLE hBurnDEV)
{
	printf("Is Disc Can Be Write?\n");
#ifdef LINUX	
	if( Xkd_DVDSDK_DiscCanWrite(hBurnDEV->hDVD) != ERROR_DVD_OK )
	{
		printf(" Disc Can not be Writed! \n");
        return ERR_DISC_CAN_NOT_WRITE;
	}
#endif
#ifdef WIN32
	if( DiscBurnable(hBurnDEV->hDVD, hBurnDEV->dev_id) != BURN_TRUE )
	{
		printf(" Cdrom Have Not Disc!\n");
        return ERR_DISC_CAN_NOT_WRITE;
	}
#endif	

	return BURN_SUCCESS;
}

int Burn_Ctrl_Dev_FormatDisc(DEV_HANDLE hBurnDEV)
{
	int ret=0;
	BURN_DISC_INFO_T *b_disc_info_ptr;;
	printf("Start Format Disc ... \n");

	b_disc_info_ptr = get_disc_info(hBurnDEV);
	if(b_disc_info_ptr != NULL)
	{
#ifdef LINUX
		ret = Xkd_DVDSDK_FormatDisc(hBurnDEV->hDVD, b_disc_info_ptr->disc_name);
    	if(ret != 0)
	    {
            printf("Format Disc Failed,retcode: %d, disc: %s\n", ret, b_disc_info_ptr->disc_name);
        	return ERR_DISC_FORMAT_FAILED;
	    }
#endif

#ifdef WINDOWS
//	ret = FormatDisc(hBurnDEV->hDVD, hBurnDEV->dev_id);
	//if(ret != BURN_TURE)
	//{
 //       printf("Format Disc Failed\n");
 //       return ERR_DISC_FORMAT_FAILED;
	//}
#endif
	}
	else
	{
#ifdef LINUX
		ret = Xkd_DVDSDK_FormatDisc(hBurnDEV->hDVD, "iBurn");
    	if(ret != 0)
	    {
            printf("Format Disc Failed,retcode: %d, disc: iBurn\n", ret);
        	return ERR_DISC_FORMAT_FAILED;
	    }
#endif		
	}

	set_burn_state(hBurnDEV, B_ALREAD);

	return BURN_SUCCESS;	
}

int Start_Burning_State_WatchDog(DEV_HANDLE hBurnDEV)
{
#ifdef LINUX
    pthread_t           StateThread;
    pthread_attr_t      attr;	
#endif
#ifdef WINDOWS
	//TODO
#endif

#ifdef LINUX
    if (pthread_create(&StateThread, NULL, StateThrFxn, hBurnDEV)) 
	{
            printf("[MAIN] Failed To Create State Thread\n", __LINE__);
            return BURN_FAILURE;
    }
#endif
#ifdef WINDOWS
	//TODO
#endif
	
    return BURN_SUCCESS;      
}


/* 获取光盘状态 */
int Burn_Ctrl_Dev_Get_DiscInfo(DEV_HANDLE hBurnDEV)
{
	BURN_DISC_INFO_T *b_disc_info_ptr=NULL;
	int ret=0;

	/************* 获取托盘状态 **************/
#if 0
	printf("DVD Tray State:\n");
#ifdef LINUX
	if( Xkd_DVDSDK_GetTrayState(hBurnDEV->hDVD) ) 
	{
		printf(" DVD Tray Is Open, You Must Close It!\n");
        return ERR_TRAY_IS_OPEN;
//		Xkd_DVDSDK_Tray(hBurnDEV->hDVD, 0); //关闭光驱
	}
#endif
#endif
	ret = Burn_Ctrl_Dev_Get_TrayState(hBurnDEV);
	if(ret != B_DEV_TRAY_CLOSE)
		return ret;

    printf(" DVD Tray Is Closed...\n");

	/************* 判断是否有光盘 **************/
#if 0
	printf("Is Have Disc?\n");
#ifdef LINUX
	if( Xkd_DVDSDK_HaveDisc(hBurnDEV->hDVD) != 1 )
	{
		printf(" Cdrom Have Not Disc!\n");
//		Xkd_DVDSDK_Tray(hBurnDEV->hDVD, 1);  //打开光驱
        return ERR_NO_HAVE_DISC;
	}
    else
    {
        printf(" CD-Rom Have Disc...\n");
	    printf(" MediaExactType:%d \n", Xkd_DVDSDK_GetMediaExactType(hBurnDEV->hDVD) );
    }
#endif

#ifdef WINDOWS
	if( HaveDisc(hBurnDEV->hDVD, hBurnDEV->dev_id) != BURN_TRUE )
	{
		printf(" Cdrom Have Not Disc!\n");
//		EjectDisc(hBurnDEV->hDVD, hBurnDEV->dev_id);  //打开光驱
        return ERR_NO_HAVE_DISC;
	}
    else
    {
        printf(" CD-Rom Have Disc...\n");
    }
#endif
#endif
	ret = Burn_Ctrl_Dev_Get_HaveDisc(hBurnDEV);
	if(ret != BURN_SUCCESS)
		return ret;
	
    sleep(1);
	printf("Start Load Disc\n");

	/************* 加载光盘 **************/
#if 0
#ifdef LINUX	
    if(Xkd_DVDSDK_LoadDisc(hBurnDEV->hDVD) != 0)
	{
		printf("Can't Load Disc ...\n");	
		return ERR_LOAD_DISC_FAILED;
	}
#endif

#ifdef WINDOWS
	LoadDisc(hBurnDEV->hDVD, hBurnDEV->dev_id);
#endif

			
	printf("Start Load Disc Success\n");
    sleep(1);

	/************* 获取光盘信息 **************/
#ifdef LINUX
	printf("Get Disc Info\n");
	LVDVD_DISC_INFO_T pDiscInfo;
	if( Xkd_DVDSDK_GetDiscInfo(hBurnDEV->hDVD, &pDiscInfo) != ERROR_DVD_OK )
	{
		printf("Xkd_DVDSDK_GetDiscInfo is failed!\n");
        return ERR_GET_DISC_INFO_FAILED;
	}
	else
	{
#if 0
		//打印光盘类型
    	printf("Disk Info:\n");
		printf(" Disc Media type [%d] \n", pDiscInfo.ntype );
		printf(" Disc Maxpeed    [%d] \n", pDiscInfo.maxpeed );
		printf(" Disc Discsize   [%d] MB\n", pDiscInfo.discsize );
		printf(" Disc Usedsize   [%d] MB\n", pDiscInfo.usedsize );
		printf(" Disc Freesize   [%d] MB\n", pDiscInfo.freesize );  
#endif
		//判断光盘容量是否获取正确，如果错误反馈失败
		if(pDiscInfo.discsize == 0)
		{
			printf("Disc Size Info Is Error\n");	
			return ERR_GET_DISC_INFO_FAILED;
		}

		b_disc_info_ptr = get_disc_info(hBurnDEV);
		if(b_disc_info_ptr != NULL)
		{
			b_disc_info_ptr->has_disc  = BURN_TRUE;	
			b_disc_info_ptr->write_flag= BURN_TRUE;	
			b_disc_info_ptr->type      = pDiscInfo.ntype;	
			b_disc_info_ptr->maxspeed  = pDiscInfo.maxpeed;	
			b_disc_info_ptr->discsize  = pDiscInfo.discsize;	
			b_disc_info_ptr->usedsize  = 0;	
			b_disc_info_ptr->writecnt  = 0;	
			b_disc_info_ptr->freesize  = pDiscInfo.discsize;	

			//如果报警门限没有设置或设置值大于光盘容量，则使用默认配置
			if((b_disc_info_ptr->alarmsize == 0) || (b_disc_info_ptr->alarmsize >= b_disc_info_ptr->discsize))
			{
				//为报警设置100M默认配置
				printf("Set Disc Alarm To Default [100] MB\n");
				b_disc_info_ptr->alarmsize = 1 *1024;	
			}
		}
	}
#endif
    
#ifdef WIN32
	DISC_INFORMATION discInfo;
	bool ret = GetDiscInfo(hBurnDEV->hDVD, hBurnDEV->dev_id, discInfo);
	if(ret != BURN_TRUE)
	{
		printf("Get Disc Info Failed\n");	
		return ERR_GET_DISC_INFO_FAILED;
	}

		b_disc_info_ptr = get_disc_info(hBurnDEV);
		if(b_disc_info_ptr != NULL)
		{
			b_disc_info_ptr->has_disc  = BURN_TRUE;	
			b_disc_info_ptr->write_flag= BURN_TRUE;	
			b_disc_info_ptr->type      = discInfo.uiDiscType;	
			b_disc_info_ptr->maxspeed  = 16;	
			b_disc_info_ptr->discsize  = discInfo.uiFreeBytes/(1024*1024);	
			b_disc_info_ptr->usedsize  = 0;	
			b_disc_info_ptr->writecnt  = 0;	
			//将Byte转换为MB
			b_disc_info_ptr->freesize  = discInfo.uiFreeBytes/(1024*1024);	

			//如果报警门限没有设置或设置值大于光盘容量，则使用默认配置
			if((b_disc_info_ptr->alarmsize = 0) || (b_disc_info_ptr->alarmsize > b_disc_info_ptr->discsize))
			{
				//为报警设置100M默认配置
				printf("Set Disc Alarm To Default [100] MB\n");
				b_disc_info_ptr->alarmsize = 100;	
			}
	}
#endif
#endif
	ret = Burn_Ctrl_Dev_LoadDisc(hBurnDEV);
	if(ret != BURN_SUCCESS)
		return ret;

    sleep(1);

	/************* 判断光盘是否可写入 **************/
	printf("Is Disc Can Be Write?\n");
#if 0
#ifdef LINUX	
	if( Xkd_DVDSDK_DiscCanWrite(hBurnDEV->hDVD) != ERROR_DVD_OK )
	{
		printf(" Disc Can not be Writed! \n");
        return ERR_DISC_CAN_NOT_WRITE;
	}
#endif
#ifdef WIN32
	if( DiscBurnable(hBurnDEV->hDVD, hBurnDEV->dev_id) != BURN_TRUE )
	{
		printf(" Cdrom Have Not Disc!\n");
        return ERR_DISC_CAN_NOT_WRITE;
	}
#endif
#endif
	ret = Burn_Ctrl_Dev_Get_DiscCanWrite(hBurnDEV);
	if(ret != BURN_SUCCESS)
		return ret;
    printf(" Disc Can Be Write ...\n");
		
	//打印光盘信息
/*   	printf("Disk Info:\n");
	printf(" Disc Media type [%d] \n",   b_disc_info_ptr->type );
	printf(" Disc Maxpeed    [%d] \n",   b_disc_info_ptr->maxspeed );
	printf(" Disc Discsize   [%d] MB\n", b_disc_info_ptr->discsize );
	printf(" Disc Usedsize   [%d] MB\n", b_disc_info_ptr->usedsize );
	printf(" Disc Freesize   [%d] MB\n", b_disc_info_ptr->freesize );  
	printf(" Disc Alarmsize  [%d] MB\n", b_disc_info_ptr->alarmsize ); */ 
	
	return BURN_SUCCESS;
}




int Burn_Ctrl_Dev_Get_Ready(DEV_HANDLE hBurnDEV)
{
	BURN_DISC_INFO_T *b_disc_info_ptr=NULL;

#ifdef LINUX
    pthread_t           StateThread;
    pthread_attr_t      attr;	
#endif
#ifdef WINDOWS
	//TODO
#endif
    int ret;

	ret = Burn_Ctrl_Dev_Get_DiscInfo(hBurnDEV);
	if(ret != BURN_SUCCESS)
		return ret;

	/************* 格式化光盘 **************/
#if 0
	printf("Format Disc\n");
#ifdef LINUX
	ret = Xkd_DVDSDK_FormatDisc(hBurnDEV->hDVD, "iBurn");
    if(ret != 0)
    {
        printf("Format Disc Failed\n");
        return ERR_DISC_FORMAT_FAILED;
    }
#endif

#ifdef WINDOWS
//	ret = FormatDisc(hBurnDEV->hDVD, hBurnDEV->dev_id);
	//if(ret != BURN_TURE)
	//{
 //       printf("Format Disc Failed\n");
 //       return ERR_DISC_FORMAT_FAILED;
	//}
#endif

  	printf("Format Disc  is ok! \n");    
#endif
	ret = Burn_Ctrl_Dev_FormatDisc(hBurnDEV);
	if(ret != BURN_SUCCESS)
		return ret;


	ret = Start_Burning_State_WatchDog(hBurnDEV);
	if(ret != BURN_SUCCESS)
		return ret;

#if 0
#ifdef LINUX
    if (pthread_create(&StateThread, NULL, StateThrFxn, hBurnDEV)) 
	{
            printf("[MAIN] Failed To Create State Thread\n", __LINE__);
            return BURN_FAILURE;
    }
#endif
#ifdef WINDOWS
	//TODO
#endif
	#endif
    return BURN_SUCCESS;      
}

int Start_Local_File_Burning(DEV_HANDLE hBurnDEV)
{
	return Burn_Ctrl_StartBurn(hBurnDEV, B_FILE);	
}

int Stop_Local_File_Burning(DEV_HANDLE hBurnDEV)
{
    return Burn_Ctrl_StopBurn(hBurnDEV);	
}

int Set_Local_File_Service_Mode(DEV_HANDLE hBurnDEV)
{
	printf("Set Service Mode Is Local File\n");
	return Burn_Set_Data_Interface(hBurnDEV, B_FILE);	
}

void Burn_Ctrl_SetRecordStatus(DEV_HANDLE hBurnDEV, BOOL bRecordStatus)
{
    printf("Set Recording Status: %d\n", bRecordStatus);
#ifdef LINUX		
    Xkd_DVDSDK_SetRecordStatus(hBurnDEV->hDVD, bRecordStatus);	
#endif
}
