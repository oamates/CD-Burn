#ifndef __BURN_STREAM__H__
#define __BURN_STREAM__H__

#ifdef LINUX
#ifdef __cplusplus
extern "C"
{
#endif
#endif
#include <Burn_SDK.h>
#include <rtspts.h>

// 刻录服务模式
typedef enum{
	MEDIAFILE_BURN_STREAM   = 1,	
	MEDIAFILE_BURN_LOCAL_FILE,	
}BURN_MODE;

// 刻录文件类型
typedef enum{
	MEDIAFILE_TS   = 1,	
	MEDIAFILE_MP4,	
	MEDIAFILE_AVI,	
	MEDIAFILE_H264,	
}MEDIAFILE_TYPE;

// 流协议类型
typedef enum{
	RTSP   = 1,	
	H323,	
    FINDANYWHERE,
    FTP,
}PROTOCL_TYPE;

/* 数据流设置参数*/
typedef struct{
	int          stream_id;	    // 数据流ID
    int          reconn_time;   // 数据流获取的重连时间间隔
	PROTOCL_TYPE protocl_type;  // 获取数据流的方案
	int          ts_channel;    // 该数据流混入TS的哪一个通道
	char		 url[1000] ;	// 数据源URL地址
}STREAM_INFO_T;


//一个TS最多只能混4路流
#define STREAM_INFO_MAX_NUM   4
/* 数据流设置参数*/
typedef struct{
    DEV_HANDLE      hBurnDEV;   // 该路流绑定的光驱句柄
    RTSPTS_HANDLE	hStream;    // RTSP所提供的句柄
	STREAM_INFO_T  *stream_info[STREAM_INFO_MAX_NUM]; //数据流信息
    BURN_MODE       burn_mode;  // 刻录模式(实时刻录/事后刻录)
	char            file_name[500]; //刻录流文件名称
    int             ts_mark;    // TS通道记录
}BURN_STREAM_T;

#if 0
/* 数据流相关状态信息*/
typedef struct{
	int          stream_id;	    // 数据流ID
	int          ts_channel;    // 该数据流混入TS的哪一个通道
    int          get_stat;      // 获取数据流的运行状态
    int          proc_stat;     // 数据流再处理的运行状态(比如TS混流)
	PROTOCL_TYPE protocl_type;  // 获取数据流的方案
    DEV_HANDLE   hBurnDEV;      // 该路流绑定的光驱句柄
	char		 url[100] ;		// 数据源URL地址
}STREAM_INFO_T;
#endif

/*******************************************************************************
* 名称  : Download_File_To_Burn
* 描述  : 下载文件并直接进行刻录
* 参数  : 
	file_info : 下载的文件XML列表，具体定义参见<<FileAnywhere对外接口协议>>
	hBurnDEV : 刻入的光驱设备句柄
* 返回值: 0: 成功，其他为错误值
*******************************************************************************/
int Download_File_To_Burn(char* file_info, DEV_HANDLE hBurnDEV);

/*******************************************************************************
* 名称  : Download_File
* 描述  : 下载文件并直接进行刻录
* 参数  : 
	file_info : 下载的文件XML列表，具体定义参见<<FileAnywhere对外接口协议>>
* 返回值: 0: 成功，其他为错误值
*******************************************************************************/
int Download_File(char* file_info);

/*******************************************************************************
* 名称  : Get_Remote_File_Info
* 描述  : 下载文件并直接进行刻录
* 参数  : 
	file_info : 下载的文件XML列表，具体定义参见<<FileAnywhere对外接口协议>>
	hBurnDEV : 刻入的光驱设备句柄
* 返回值: 0: 成功，其他为错误值
*******************************************************************************/
int Get_Remote_File_Info(char* file_info);

/*******************************************************************************
* 名称  : Add_Stream_To_Burn
* 描述  : 将指定的数据流加入刻录任务中
* 参数  : 
	stream   : 添加的数据流信息 
	hBurnDEV : 刻入的光驱设备句柄
* 返回值: 0: 成功，其他为错误值
*******************************************************************************/
int Add_Stream_To_Burn(DEV_HANDLE hBurnDEV, STREAM_INFO_T *stream);


/*******************************************************************************
* 名称  : Clear_Burn_Stream_List
* 描述  : 清空刻录流列表
* 参数  : 
	hBurnDEV : 刻入的光驱设备句柄
* 返回值: 0: 成功，其他为错误值
*******************************************************************************/
int Clear_Burn_Stream_List(DEV_HANDLE hBurnDEV);

/*******************************************************************************
* 名称  : Get_Stream_Info
* 描述  : 获取数据流状态信息
* 参数  : 
	stream_id : 数据流ID
* 返回值: 数据流状态结构体 
*******************************************************************************/
STREAM_INFO_T Get_Stream_Info(int stream_id);


/*******************************************************************************
* 名称  : Set_Stream_Proc_Type
* 描述  : 设置数据流刻录前的处理方案
* 参数  : 
	type : 处理方案
* 返回值: 0: 成功，其他为错误值 
*******************************************************************************/
int Start_Stream_Burning(DEV_HANDLE hBurnDEV);

int Stop_Stream_Burning(DEV_HANDLE hBurnDEV);

BURN_MODE Get_Service_Mode(DEV_HANDLE hBurnDEV);

//int Set_Service_Mode(DEV_HANDLE hBurnDEV, BURN_MODE mode);

int Set_Stream_Proc_Type(MEDIAFILE_TYPE type);

int Init_Stream_Subsystem(DEV_HANDLE hBurnDEV);

int Init_Task_Module(const char* log_path);
int Unint_Task_Module(void);

int Set_Stream_Service_Mode(DEV_HANDLE hBurnDEV);

void Print_Stream_List(DEV_HANDLE hBurnDEV);
#ifdef LINUX
#ifdef __cplusplus
}
#endif
#endif
#endif
