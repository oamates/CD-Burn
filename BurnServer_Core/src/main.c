#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef LINUX
	#include <pthread.h>
#endif

#ifdef WIN32
#include <windows.h>
#include	<process.h>
#endif

#include <Burn_SDK.h>
#include <Burn_Stream.h>

static int test_flag = 1;
static int proc_thread_cnt = 0;
static int proc_thread_return_cnt = 0;
static char      *file_path;
static BURN_MODE  burn_mode;
static int start_burn_flag = 1;
static int close_burn_flag = 0;

//将本句注释掉可实现多光驱刻录
#define ONLY_ONE_DEVICE

//将本句打开可进入测试模式， 不会向光驱中写入数据
//#define RTSP_TEST

//回调函数
int important_events(DEV_HANDLE hBurnDEV, const BURN_RUN_STATE_T *run_state, void *val)
{
	int ret;

	printf("=================================================\n");
	printf("dev_id = %d\n", hBurnDEV->dev_id);
	printf("tray_state = %d\n", run_state->tray_state);
	printf("running_state = %d\n", run_state->running_state);
	printf("running_info = %d\n", run_state->running_info);
	printf("val = %s\n", (char*) val);
	printf("=================================================\n");

    ret = Stop_Stream_Burning(hBurnDEV);
    if(ret == BURN_FAILURE)
    {
        printf("Stop Burnning Disc Failed\n");
    }
		
	return 0;
}

static int program_exit_flag = 0;
void *ctrlThrFxn(void *args)
{
    int ret;
	char cc;
    DEV_HANDLE hBurnDEV;
    hBurnDEV = (DEV_HANDLE)args;

	if(hBurnDEV == NULL)
		return NULL;

    printf("Dev[%d] Ctrl Thread Start ...\n", hBurnDEV->dev_id);

	while(1)
	{	
		cc = getchar();
    	if(cc == 'Q')
		{
            printf("====== Start To Quit ======\n");
			program_exit_flag = 1;

			break;
		}
    	else if(cc == 'S')
		{
            printf("====== Start To Writting  ======\n");

			ret = Start_Burning_State_WatchDog(hBurnDEV);
			if(ret != BURN_SUCCESS)
			{
                printf("Stop Burnning Disc Failed\n");
			}
			printf("State_WatchDog start, start_burn_flag = 1\n");
            start_burn_flag = 1;
//            Burn_Set_Pause_Mode(BUiRN_FLASE);
			continue;
		}
    	else if(cc == 'P')
		{
            printf("====== Start To Pause ======\n");
            ret = Stop_Stream_Burning(hBurnDEV);
            if(ret == BURN_FAILURE)
            {
                printf("Stop Burnning Disc Failed\n");
            }

            Set_RT_File_Name_Update();
            start_burn_flag = 0;
			continue;
		}
		else if(cc == 'C')
			break;

		sleep(1);
	}
    start_burn_flag = 0;
    close_burn_flag = 1;

	printf("QQQQQQQQQQQQQQ Dev = [%d] QQQQQQQQQQQQQQQQ\n", hBurnDEV->dev_id);
    ret = Stop_Stream_Burning(hBurnDEV);
    if(ret == BURN_FAILURE)
    {
        printf("Stop Burnning Disc Failed\n");
    }

    printf("Dev[%d] Stop Burnning Disc Success\n", hBurnDEV->dev_id);

    test_flag = 0;    

	return NULL;
}

/********************* 刻录实时数据流 *************************/
void *BurnStreamThrFxn(void *args)
{
    int        ret;
	
    DEV_HANDLE hBurnDEV;
#ifdef	LINUX
    pthread_t           ctrlThread;
#else
#endif	//

    hBurnDEV = (DEV_HANDLE)args;
	if(hBurnDEV == NULL)
        	goto cleanup;

    printf("BurnStreamThrFxn Thread ........\n");

TEST:
#ifndef RTSP_TEST
	//配置光盘信息
	while(1)
	{
		ret = Burn_Ctrl_Dev_Get_DiscInfo(hBurnDEV);
    	if(ret != BURN_SUCCESS)
	    {
			switch(ret)
			{
				case ERR_TRAY_IS_OPEN:
					printf("The Tray Is Open ...\n");	
					break;

				case ERR_NO_HAVE_DISC:
					printf("Can't Find Disc ...\n");	
					break;

				case ERR_LOAD_DISC_FAILED:
					printf("Can Not Load Disc ...\n");	
					break;

				case ERR_GET_DISC_INFO_FAILED:
					printf("Can Not Get Disc Info ...\n");	
					break;

				case ERR_DISC_CAN_NOT_WRITE:
					printf("Disc Can Not To Be Write ...\n");	
					break;

				case ERR_DISC_FORMAT_FAILED:
					printf("Format Disc Failed ...\n");	
					break;
			}
    	}
		else
			break;

		Burn_Ctrl_DevTray(hBurnDEV, B_DEV_TRAY_OPEN);
		printf("[%d]Please Change Disc, Push Any Key To Continue ...\n", ret);
		getchar();
	}
#endif
#if 1
    //初始化流子系统
    ret = Init_Stream_Subsystem(hBurnDEV);
    if(ret != BURN_SUCCESS)
    {
        printf("Init Stream Subsystem Failed\n");
        	goto cleanup;
    }

    printf("Init Stream Subsystem Success\n");
#endif

#ifdef LINUX
    if (pthread_create(&ctrlThread, NULL, ctrlThrFxn, hBurnDEV)) {
            printf("[MAIN] Failed to create ctrl thread\n");
        	goto cleanup;
        }
#endif

/****************** 进行参数设置 *********************/

	//设置回调函数
	Burn_Set_Event_Callback(hBurnDEV, important_events, "chnsys");


    //设置刻录模式(实时刻录)
    ret = Set_Stream_Service_Mode(hBurnDEV);
    if(ret != BURN_SUCCESS)
    {
        printf("Set Service Mode Failed\n");
        goto cleanup;
    }

    printf("Set Service Mode Success\n");

	//设置缓冲大小
	ret = Set_Burn_Buffer_Size(hBurnDEV, 10*1024*1024);
    if(ret != BURN_SUCCESS)
    {
        printf("Set Burn Buffer Size Failed\n");
        goto cleanup;
    }

    printf("Set Burn Buffer Size Success\n");

	//设置光盘报警门限(单位为MB)
	Burn_Set_DiscAlarmSize(hBurnDEV, 500);

    Burn_Set_RT_File_Name("wangwh.ts");

	Burn_Set_DiscName(hBurnDEV, "BurnServer Test");
/*******************************************************/
#if 1
    //创建烧写环境
    ret = Create_Burn_Environment(hBurnDEV);
    if(ret != BURN_SUCCESS)
    {
        printf("Create Burn Environment Failed\n");
        goto cleanup;
    }

    printf("Create Burn Environment Success\n");
#endif


sleep(3);
#ifndef RTSP_TEST
    //控制光驱进行准备
	while(1)
	{
		ret = Burn_Ctrl_Dev_Get_Ready(hBurnDEV);
    	if(ret != BURN_SUCCESS)
	    {
			switch(ret)
			{
				case ERR_TRAY_IS_OPEN:
					printf("The Tray Is Open ...\n");	
					break;

				case ERR_NO_HAVE_DISC:
					printf("Can't Find Disc ...\n");	
					break;

				case ERR_LOAD_DISC_FAILED:
					printf("Can Not Load Disc ...\n");	
					break;

				case ERR_GET_DISC_INFO_FAILED:
					printf("Can Not Get Disc Info ...\n");	
					break;

				case ERR_DISC_CAN_NOT_WRITE:
					printf("Disc Can Not To Be Write ...\n");	
					break;

				case ERR_DISC_FORMAT_FAILED:
					printf("Format Disc Failed ...\n");	
					break;
			}
    	}
		else
			break;
		

		Burn_Ctrl_DevTray(hBurnDEV, B_DEV_TRAY_OPEN);
		printf("Please Change Disc, Push Any Key To Continue ...\n");
		getchar();
	}

    printf("Ctrl Dev Get Ready Success\n");
#endif

	//实时刻录处理
	{
		if (hBurnDEV->dev_id == 0)
		{
			//设置第一路流
			STREAM_INFO_T stream0;
			stream0.stream_id    = 0;
			stream0.reconn_time  = 30;
			stream0.protocl_type = RTSP;
			//混入TS的第一个通道
			stream0.ts_channel   = 0;
			strcpy(stream0.url, "rtsp://10.14.1.58/1");
			//将流添加入烧写队列
			ret = Add_Stream_To_Burn(hBurnDEV, &stream0);
			if(ret != BURN_SUCCESS)
			{
				printf("Add Stream0 To Burn Failed\n");
			}
		}
		else
		{
			//设置第一路流
			STREAM_INFO_T stream0;
			stream0.stream_id    = 0;
			stream0.reconn_time  = 30;
			stream0.protocl_type = RTSP;
			//混入TS的第一个通道
			stream0.ts_channel   = 0;
			strcpy(stream0.url, "rtsp://10.1.2.225/m1a1");
			//将流添加入烧写队列
			ret = Add_Stream_To_Burn(hBurnDEV, &stream0);
			if(ret != BURN_SUCCESS)
			{
				printf("Add Stream0 To Burn Failed\n");
			}
		}

#if 0
		STREAM_INFO_T stream1;
		stream1.stream_id    = 1;
		stream1.reconn_time  = 30;
		stream1.protocl_type = RTSP;
		stream1.ts_channel   = 1;
		strcpy(stream1.url, "rtsp://10.1.2.249/1");
		ret = Add_Stream_To_Burn(hBurnDEV, &stream1);
		if(ret != BURN_SUCCESS)
		{
			printf("Add Stream1 To Burn Failed\n");
		}		

		STREAM_INFO_T stream2;
		stream2.stream_id    = 2;
		stream2.reconn_time  = 30;
		stream2.protocl_type = RTSP;
		stream2.ts_channel   = 2;
		strcpy(stream2.url, "rtsp://127.0.0.1/0");
		ret = Add_Stream_To_Burn(hBurnDEV, &stream2);
		if(ret != BURN_SUCCESS)
		{
			printf("Add Stream2 To Burn Failed\n");
		}		

		STREAM_INFO_T stream3;
		stream3.stream_id    = 3;
		stream3.reconn_time  = 30;
		stream3.protocl_type = RTSP;
		stream3.ts_channel   = 3;
		strcpy(stream3.url, "rtsp://127.0.0.1/1");
		ret = Add_Stream_To_Burn(hBurnDEV, &stream3);
		if(ret != BURN_SUCCESS)
		{
			printf("Add Stream3 To Burn Failed\n");
		}		
#endif
	}

	//    Print_Stream_List(hBurnDEV);

	while(1)
	{
		if(start_burn_flag == 1)
		{
#if 1
			//开启刻录
			ret = Start_Stream_Burning(hBurnDEV);
			if(ret != BURN_SUCCESS)
			{
				printf("Burn File Failed\n");
				goto cleanup;
			}

			printf("======== Dev[%d] Burn File Success ======= \n", hBurnDEV->dev_id);
#endif
		}
		else if(close_burn_flag == 1)
		{
			break;
		}

		usleep(200);
	}


#if 0
	ret = Burn_Dir_Form_Local_Dir(hBurnDEV, NULL, "/root/Test/Player");
	if(ret != BURN_SUCCESS)
	{
        printf("Burn Player Failed\n");
        return -1;
    }

    printf("======== Burn Player Success ======= \n");
#endif
	
#if 0
	ret = Burn_Dir_Form_Local_Dir(hBurnDEV, NULL, "/root/Test/Player");
    if(ret != BURN_SUCCESS)
    {
        printf("Burn Player Failed\n");
    }

    printf("======== Burn Player Success ======= \n");
#endif
		
#ifndef RTSP_TEST	
	Disk_Close_BackUpFile(hBurnDEV);
			
#if 1
	
	Burn_File_Form_Local_File(hBurnDEV, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED.playlist");
//	Burn_File_Form_Local_File(hBurnDEV, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/abc");
//	Burn_File_Form_Local_File(hBurnDEV, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/abc1");
//	Burn_File_Form_Local_File(hBurnDEV, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/abc2");
//	Burn_File_Form_Local_File(hBurnDEV, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/abc3");
	Burn_File_Form_Local_File(hBurnDEV, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/note_1.doc");
	Burn_File_Form_Local_File(hBurnDEV, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/AutoRun.inf");
	Burn_File_Form_Local_File(hBurnDEV, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/AuthChnsys.exe");

	Burn_Dir_Form_Local_Dir(hBurnDEV, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/PlayBack" );
//	Burn_File_Form_Local_File(hBurnDEV, NULL, "/mnt/HD0/BurnDownload/EBFB6F5D_0DBE_7DDF_86B8_36C4921DF0ED/Note.doc");



sleep(1);
    printf("Dev[%d] begin to Close Disc 11111111111111\n", hBurnDEV->dev_id);
    //封盘指令
    ret = Burn_Ctrl_CloseDisc(hBurnDEV);
    if(ret != BURN_SUCCESS)
    {
        printf("Close Disc Failed\n");
        goto cleanup;
    }

    printf("Dev[%d] Close Disc Success 22222222222222\n", hBurnDEV->dev_id);
#endif

#if 1
    sleep(2);
    //弹出光驱
    ret = Burn_Ctrl_DevTray(hBurnDEV, B_DEV_TRAY_OPEN);
#endif
    printf("Burn Ctrl Dev Tray Success\n");
#endif

	ret = Clear_Burn_Stream_List(hBurnDEV);
    if(ret != BURN_SUCCESS)
    {
        printf("Clear Burn Stream List Failed\n");
    }

	if(program_exit_flag == 0)
		goto TEST;

cleanup:
	proc_thread_return_cnt++;

	return 0;
}


/********************* 刻录本地数据 *************************/
#ifdef WIN32
unsigned int __stdcall BurnLocalFileThrFxn(void *args)
#endif
#ifdef LINUX
void *BurnLocalFileThrFxn(void *args)
#endif
{
    int        ret;
	
    DEV_HANDLE hBurnDEV;
//    pthread_t           ctrlThread;
//    pthread_attr_t      attr;	

    printf("BurnLocalFileThrFxn Thread ........\n");
    hBurnDEV = (DEV_HANDLE)args;

	if(hBurnDEV == NULL)
	{
		printf("hBurnDEV == NULL\n");
        	goto cleanup;
	}

    printf("BurnLocalFileThrFxn Thread ........\n");

	//配置光盘信息
	while(1)
	{
		ret = Burn_Ctrl_Dev_Get_DiscInfo(hBurnDEV);
    	if(ret != BURN_SUCCESS)
	    {
			switch(ret)
			{
				case ERR_TRAY_IS_OPEN:
					printf("The Tray Is Open ...\n");	
					break;

				case ERR_NO_HAVE_DISC:
					printf("Can't Find Disc ...\n");	
					break;

				case ERR_LOAD_DISC_FAILED:
					printf("Can Not Load Disc ...\n");	
					break;

				case ERR_GET_DISC_INFO_FAILED:
					printf("Can Not Get Disc Info ...\n");	
					break;

				case ERR_DISC_CAN_NOT_WRITE:
					printf("Disc Can Not To Be Write ...\n");	
					break;

				case ERR_DISC_FORMAT_FAILED:
					printf("Format Disc Failed ...\n");	
					break;
			}
    	}
		else
			break;

		Burn_Ctrl_DevTray(hBurnDEV, B_DEV_TRAY_OPEN);
		printf("[%d]Please Change Disc, Push Any Key To Continue ...\n", ret);
		getchar();
	}

#if 0
    //初始化流子系统
    ret = Init_Stream_Subsystem(hBurnDEV);
    if(ret != BURN_SUCCESS)
    {
        printf("Init Stream Subsystem Failed\n");
        	goto cleanup;
    }

    printf("Init Stream Subsystem Success\n");
#endif

#if 0
    if (pthread_create(&ctrlThread, NULL, ctrlThrFxn, hBurnDEV)) {
            printf("[MAIN] Failed to create ctrl thread\n", __LINE__);
            return BURN_FAILURE;
        }
#endif

/****************** 进行参数设置 *********************/
    //设置刻录模式(事后刻录)
    ret = Set_Local_File_Service_Mode(hBurnDEV);
    if(ret != BURN_SUCCESS)
    {
        printf("Set Service Mode Failed\n");
        	goto cleanup;
    }

    printf("Set Service Mode Success\n");

//    Burn_Set_AbnormalCallback(hBurnDEV, abnormal);

	//设置缓冲大小
	ret = Set_Burn_Buffer_Size(hBurnDEV, 10*1024*1024);
    if(ret != BURN_SUCCESS)
    {
        printf("Set Burn Buffer Size Failed\n");
        	goto cleanup;
    }

    printf("Set Burn Buffer Size Success\n");

	//设置光盘报警门限(单位为MB)
	Burn_Set_DiscAlarmSize(hBurnDEV, 700);

	//设置回调函数
	Burn_Set_Event_Callback(hBurnDEV, important_events, "chnsys");

/*******************************************************/
#if 1
    //创建烧写环境
    ret = Create_Burn_Environment(hBurnDEV);
    if(ret != BURN_SUCCESS)
    {
        printf("Create Burn Environment Failed\n");
        	goto cleanup;
    }

    printf("Create Burn Environment Success\n");
#endif


sleep(3);

    //控制光驱进行准备
	while(1)
	{
		ret = Burn_Ctrl_Dev_Get_Ready(hBurnDEV);
    	if(ret != BURN_SUCCESS)
	    {
			switch(ret)
			{
				case ERR_TRAY_IS_OPEN:
					printf("The Tray Is Open ...\n");	
					break;

				case ERR_NO_HAVE_DISC:
					printf("Can't Find Disc ...\n");	
					break;

				case ERR_LOAD_DISC_FAILED:
					printf("Can Not Load Disc ...\n");	
					break;

				case ERR_GET_DISC_INFO_FAILED:
					printf("Can Not Get Disc Info ...\n");	
					break;

				case ERR_DISC_CAN_NOT_WRITE:
					printf("Disc Can Not To Be Write ...\n");	
					break;

				case ERR_DISC_FORMAT_FAILED:
					printf("Format Disc Failed ...\n");	
					break;
			}
    	}
		else
			break;
		

		Burn_Ctrl_DevTray(hBurnDEV, B_DEV_TRAY_OPEN);
		printf("Please Change Disc, Push Any Key To Continue ...\n");
		getchar();
	}

    printf("Ctrl Dev Get Ready Success\n");


    //事后刻录处理
    {
        //指定本地的刻录目录
        ret = Burn_Specific_Dir_To_Disc(hBurnDEV, file_path);
//        ret = Burn_Ctrl_AddBurnLocalDir(hBurnDEV, NULL, file_path);
        if(ret != BURN_SUCCESS)
        {
            printf("Add Burn File Failed\n");
        	goto cleanup;
        }

#if 0
        //指定本地的文件
        ret = Burn_Ctrl_AddBurnLocalFile(hBurnDEV, NULL, "/home/wwh/mywork/BurnServer/release/bin/core.6128");
        if(ret != BURN_SUCCESS)
        {
            printf("Add File Failed\n");
            return -1;
        }
        printf("Add Burn File Success\n");
#endif
    }

#if 1
    //开启刻录
    ret = Start_Local_File_Burning(hBurnDEV);
    if(ret != BURN_SUCCESS)
    {
        printf("Burn File Failed\n");
        	goto cleanup;
    }

    printf("======== Dev[%d] Burn File Success ======= \n", hBurnDEV->dev_id);
#endif
    

#if 0
	ret = Burn_Dir_Form_Local_Dir(hBurnDEV, NULL, "./PlayBack");
    if(ret != BURN_SUCCESS)
    {
        printf("Burn Player Failed\n");
        return -1;
    }

    printf("======== Burn Player Success ======= \n");
#endif
	
#if 1
sleep(1);
    //封盘指令
    ret = Burn_Ctrl_CloseDisc(hBurnDEV);
    if(ret != BURN_SUCCESS)
    {
        printf("Close Disc Failed\n");
        	goto cleanup;
    }

    printf("Dev[%d] Close Disc Success\n", hBurnDEV->dev_id);
#endif

#if 1
    sleep(2);
    //弹出光驱
    ret = Burn_Ctrl_DevTray(hBurnDEV, B_DEV_TRAY_OPEN);
#endif
    printf("Burn Ctrl Dev Tray Success\n");

cleanup:
	proc_thread_return_cnt++;

	return 0;
}

void print_used(char *proc_name)
{
    printf("[%s] [BurnMode] [File_Path/BackUp]\n", proc_name);
    printf("BurnMode : 0-Burn Local File, 1-Burn Stream To File\n");
    printf("FilePath : When BurnMode Is 0, Give The Local File Path\n");
    printf("BackUp   : When BurnMode Is 1, Set 1 To Use Disk BackUp\n");
}

int main(int argc, char *argv[])
{
    DEV_HANDLE hBurnDEV  = NULL;
    DEV_HANDLE hBurnDEV2 = NULL;
    DEV_HANDLE hBurnDEV3 = NULL;
    DEV_HANDLE hBurnDEV4 = NULL;
        
    int        backup;
    int        ret = 0;
//    char      *file_path;

	DEV_SYS_INFO_T      dev_sys_info;

    if(argc < 3)
    {
        print_used(argv[0]);
        return -1;
    }

    if(atoi(argv[1]) == 0)
    {
        burn_mode = MEDIAFILE_BURN_LOCAL_FILE;
        file_path = argv[2];
        printf("Burn Local File, File Path Is %s\n", file_path);
    }
    else if(atoi(argv[1]) == 1)
    {
        burn_mode = MEDIAFILE_BURN_STREAM;
        backup = atoi(argv[2]);
        printf("Burn Stream File, Backup Set Is %d\n", backup);
    }
    else
    {
        print_used(argv[0]);
        return -1;
    }

	Init_Task_Module("./");

	//获取系统设备信息
	if(Burn_Get_DeviceNum(&dev_sys_info) != 0)
	{
		printf("Get DeviceNum Failed\n");	
		return -1;
	}

    int i = 0;

	hBurnDEV = Burn_Dev_Init(dev_sys_info.dev1);
    if (hBurnDEV != NULL)
    {
        printf("hBurnDEV :%p\n",(void *)hBurnDEV);	
        i++;
    }
    
	hBurnDEV2 = Burn_Dev_Init(dev_sys_info.dev2);
    if (hBurnDEV2 != NULL)
    {
        printf("hBurnDEV2 :%p\n",(void *)hBurnDEV2);	
        i++;
    }

    printf("Get DeviceNum :%d\n",i);	

#ifndef ONLY_ONE_DEVICE 
	hBurnDEV3 = Burn_Dev_Init(dev_sys_info.dev3);
	hBurnDEV4 = Burn_Dev_Init(dev_sys_info.dev4);
#endif

	if(hBurnDEV != NULL)
	{
#ifdef LINUX
	    pthread_t           dev1Thread;

#if 1
	//开启硬盘备份
	ret = Set_Disk_Param(hBurnDEV, BURN_TRUE, 1024*1024, 100, "/mnt/HD0");
    if(ret != BURN_SUCCESS)
    {
        printf("Set Disk Param Failed\n");
		return BURN_FAILURE;
    }

	//创建备份文件
	ret = Disk_Create_BackUpFile(hBurnDEV, "chnsys1.ts");
    if(ret != BURN_SUCCESS)
    {
        printf("Create Disk BackUp File Failed\n");
		return BURN_FAILURE;
    }
	//开始硬盘备份
	ret = Start_Disk_BackUp(hBurnDEV);	
    if(ret != BURN_SUCCESS)
    {
        printf("Start Disk BackUp Failed\n");
		return BURN_FAILURE;
    }
#endif
#if 1
	printf("Create DEV1 Proc Thread ... \n");
	if (pthread_create(&dev1Thread, NULL, BurnStreamThrFxn, hBurnDEV)) {
		printf("[MAIN][%d] Failed to create dev1 thread\n", __LINE__);
		return BURN_FAILURE;
	}
#endif
#if 0
    if (hBurnDEV2 != NULL)
    {
        printf("Create DEV2 Proc Thread ... \n");
        if (pthread_create(&dev1Thread, NULL, BurnStreamThrFxn, hBurnDEV2)) {
            printf("[MAIN][%d] Failed to create dev2 thread\n", __LINE__);
            return BURN_FAILURE;
        }
    }
    
#endif
#endif
#ifdef WIN32
		_beginthreadex(NULL,1024000,BurnLocalFileThrFxn,(void*)hBurnDEV,0,NULL);

		//TODO
#endif
		proc_thread_cnt++;
	}

#ifndef ONLY_ONE_DEVICE 
	if(hBurnDEV2 != NULL)
	{
#ifdef LINUX

#if 0
	//开启硬盘备份
	ret = Set_Disk_Param(hBurnDEV2, BURN_TURE, 1024*1024, 100, "/mnt/HD0");
    if(ret != BURN_SUCCESS)
    {
        printf("Set Disk Param Failed\n");
		return BURN_FAILURE;
    }

	//创建备份文件
	ret = Disk_Create_BackUpFile(hBurnDEV2, "chnsys2.ts");
    if(ret != BURN_SUCCESS)
    {
        printf("Create Disk BackUp File Failed\n");
		return BURN_FAILURE;
    }
	//开始硬盘备份
	ret = Start_Disk_BackUp(hBurnDEV2);	
    if(ret != BURN_SUCCESS)
    {
        printf("Start Disk BackUp Failed\n");
		return BURN_FAILURE;
    }
#endif
		
	    pthread_t           dev2Thread;

   		if (pthread_create(&dev2Thread, NULL, BurnStreamThrFxn, hBurnDEV2)) {
        	    printf("[MAIN][%d] Failed to create dev2 thread\n", __LINE__);
            	return BURN_FAILURE;
        }
#endif
#ifdef WINDOWS
		//TODO
#endif

		proc_thread_cnt++;
	}
	
	if(hBurnDEV3 != NULL)
	{
#ifdef LINUX
	    pthread_t           dev3Thread;

   		if (pthread_create(&dev3Thread, NULL, BurnLocalFileThrFxn, hBurnDEV3)) {
        	    printf("[MAIN][%d] Failed to create dev3 thread\n", __LINE__);
            	return BURN_FAILURE;
        }
#endif
#ifdef WINDOWS
		//TODO
#endif

		proc_thread_cnt++;
	}
	
	if(hBurnDEV4 != NULL)
	{
#ifdef LINUX
	    pthread_t           dev4Thread;

   		if (pthread_create(&dev4Thread, NULL, BurnStreamThrFxn, hBurnDEV4)) {
        	    printf("[MAIN][%d] Failed to create dev4 thread\n", __LINE__);
            	return BURN_FAILURE;
        }
#endif
#ifdef WINDOWS
		//TODO
#endif

		proc_thread_cnt++;
	}			
#endif

	while(1)
	{
		if(proc_thread_return_cnt == proc_thread_cnt)
			break;

		sleep(1);	
	}

	printf("Burn Server Return ...\n");

	Unint_Task_Module();

    return 0;
}
