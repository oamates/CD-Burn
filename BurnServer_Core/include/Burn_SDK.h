#ifndef __BURN_SDK_H__
#define __BURN_SDK_H__

#include <Burn_State.h>
#include <Database.h>
#include <RingBuffer.h>

#ifdef LINUX
#include <stdbool.h>
#include <LibDVDSDK.h>
#ifdef __cplusplus
extern "C"
{
#endif	
#else
#include <windows.h>
#include <WinBurnSDK.h>
#define  WINDOWS
#define	sleep	Sleep
#define	usleep(n)	Sleep((n/1000))
#define bzero(p,n)	memset(p,0,n)
#endif

#define BURN_DEV_MAX_NUM  4
#define FILE_PATH_MAX  1000
#define URL_PATH_MAX  1000

typedef enum
{
	BURN_LINUX,
	BURN_WINDOWS,
	BURN_EPSON,
	BURN_PRIMERA,
}BURN_KERNEL_TYPE;

typedef enum
{
	BURN_FLASE = 0,
	BURN_TRUE,
}BURN_BOOL;

typedef enum
{
	BURN_SUCCESS = 0,
	BURN_FAILURE,
}BURN_RET;

typedef enum
{
	B_STREAM,
	B_FILE,
}INTERFACE_TYPE;

// 光驱类型
typedef enum{
	DVDDRIVER_UNKNOWN = 0,	// 未知光驱类型
	DVDDRIVER_PRVIDVD,		// 专用刻录DVD光盘
	DVDDRIVER_CDR,			// CD只读光驱
	DVDDRIVER_CDRW,			// CD读写光驱
	DVDDRIVER_DVDR,			// DVD只读光驱
	DVDDRIVER_DVDRW,		// DVD读写光驱
	DVDDRIVER_BLUER,		// 蓝光只读光驱
	DVDDRIVER_BLUERW,		// 蓝光读写光驱
}DRIVER_TYPE;

// 光盘类型
typedef enum{
	DISC_UNKNOWN = 0,	// 未知类型
	DISC_CD_ROM,
	DISC_CD_R,
	DISC_CD_RW,
	DISC_DVD_ROM,
	DISC_DVD_R,          //5
	DISC_DVD_RAM,
	DISC_DVD_RW,
	DISC_DVD_RW_SEQ,
	DISC_DVD_RW_DL,
	DISC_DVD_R_DL_SEQ,    //10
	DISC_DVD_R_DL_LJ,
	DISC_DVD_R_DL,
	DISC_DVD_PLUS_R,
	DISC_DVD_PLUS_RW,
	DISC_DVD_PLUS_RW_DL, //15
	DISC_DVD_PLUS_R_DL,
	DISC_BD_ROM,
	DISC_BD_R_SEQ,
	DISC_BD_R,
	DISC_BD_RE,			//20
	DISC_HDVD_ROM,
	DISC_HDVD_R,
	DISC_HDVD_RAM,
}DISC_TYPE;

struct Dev_Object{
	int              dev_id;         // 设备ID
	//    int              kernel;         // 刻录核心类型
	BURN_STAT        burn_state;     // 刻录状态(是否已经开始刻录)
	INTERFACE_TYPE   interface1;      // 获取数据的接口类型
	char             dev_path[1000]; // 设备路径，根据刻录核心的不同会有不同
#ifdef LINUX
	DVDDRV       hDVD;           //Linux底层操作句柄
#endif
#ifdef WIN32
	BURNHANDLE       hDVD;           //Windows 底层操作句柄
#endif
	BURN_DATA_T      data;           // 设备对应的数据类管理
	RingBuffer       hBuf;           // 数据缓冲
	FILE *			 fd;			 //硬盘备份句柄
};
typedef struct Dev_Object *DEV_HANDLE;



typedef struct DEV_SYS_INFO
{
	int dev_num;
	char dev1[200];
	char dev2[200];
	char dev3[200];
	char dev4[200];
}DEV_SYS_INFO;

/* 光盘信息*/
typedef struct{
	BURN_BOOL    has_disc;	           // 是否存在光盘
	BURN_BOOL    write_flag;           // 是否可以向光盘写入数据
	int 	 	 type;			       // 光盘类型	DISC_TYPE
	int			 maxspeed;		       // 最大速度(写速度)
	unsigned int discsize;		       // 光盘容量(MB)
	unsigned int usedsize;		       // 已使用的大小(MB)	
	unsigned int freesize;	           // 可用大小(MB)
	unsigned int writecnt;	           // 可用大小(MB)
	unsigned int alarmsize;	           // 报警门限容量大小(MB)
	unsigned int alarmwarningsize;     // 报警门限预警空间大小(MB)
	char         disc_name[600];       // 光盘名称
}BURN_DISC_INFO;

/* 光驱设备信息*/
typedef struct{
	BURN_BOOL has_dev;				   // 是否存在设备
	char szVender[128];				   // 厂家名称
	int  drvtype;					   // 光驱类型			DVDDRIVER_TYPE
	int  discsupts;					   // 光盘支持总数
	//	unsigned char disclist[255];       // 光盘里类型列表，见DVDDISC_TYPE
}BURN_DEV_INFO;

/* 硬盘设备信息*/
typedef struct{
	BURN_BOOL  use_disk;				       // 是否存在设备
	BURN_BOOL  write_flag;			       // 是否可以向硬盘写入数据
	//	unsigned int disksize;	           // 硬盘容量(MB)
	unsigned long backupsize;	       // 备份容量(MB)
	unsigned long usedsize;		       // 已使用备份容量的大小(MB)	
	unsigned long freesize;		       // 可用备份容量大小(MB)
	unsigned long alarmsize;	       // 报警门限容量大小(MB)
	char *backup_path;                 // 硬盘备份全路径
}BURN_DISK_INFO;

/* 运行状态信息*/
typedef struct{
	DEV_TRAY_STAT  tray_state;		   // 托盘状态 
	RUNNING_STATE  running_state;      // 运行状态
	RUNNING_INFO   running_info;       // 运行状态简要描述
	char           describe[2048];     // 运行状态具体描述
}BURN_RUN_STATE;

/* 运行状态信息*/
typedef struct{
	//	int   file_size;		           // 当前正在刻录的文件大小
	//  int   burn_size;                   // 正在刻录的文件已刻入的数据大小
	char *burning_file_name;		   // 正在刻录的文件名
	char *completed_file_name;   	   // 正在刻录的文件名
}BURN_FILE;

/*刻录业务参数*/
typedef struct{
	int  cache; 					    // 平台无关层缓冲 
	int  md5_bool;					    // MD5功能开关
	int  burn_num;                      // 光盘刻录份数
	int	 auto_format;                   // 自动格式化
	char passwd[100];                   // 光盘密码
	int  file_path[600];                // 刻录源文件路径
	int  disc_cover[600];	    	    // 刻录封面图片路径
}BURN_PARAM;

/*回调函数*/
typedef	int(*CB_EVENTS)(DEV_HANDLE hBurnDEV, const BURN_RUN_STATE*, void*);

typedef struct
{
	void *val;
	CB_EVENTS important_events;
}CALLBACK_T;

/* 光驱任务相关信息*/
typedef struct{
	int               dev_id;           // 设备ID，每个ID应对一个光驱，对光驱的操作必须提供设备ID
	BURN_DEV_INFO   dev_info;         // 光驱设备信息
	BURN_DISC_INFO  disc_info;        // 光盘信息
	BURN_DISK_INFO  disk_info;        // 硬盘信息
	BURN_RUN_STATE  run_state;        // 运行状态
	BURN_PARAM      burn_param;       // 刻录参数
	BURN_FILE       burn_file;        // 刻录过程中文件相关类
	CALLBACK_T        event;            // 重要事件回调
}BURN_INFO;

#ifdef LINUX
typedef Xkd_DVDSDK_DIR DIR_HANDLE;		        // 目录节点指针
typedef Xkd_DVDSDK_FILE FILE_HANDLE;		    // 文件指针
#else
typedef void* DIR_HANDLE;		        // 目录节点指针
typedef void* FILE_HANDLE;		        // 文件指针
#endif 




/*=============================================================================
 * 光驱设备控制类
 *===========================================================================*/
class CBurnCtrl
{
public:
	CBurnCtrl();
	~CBurnCtrl();

public:
	/*******************************************************************************
	* 名称  : BurnCtrlDevTray
	* 描述  : 打开/关闭托盘
	* 参数  :
	hBurnDEV : 光驱设备句柄
	bOpen  : B_DEV_TRAY_OPEN:打开托盘, B_DEV_TRAY_CLOSE:关闭托盘
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int	BurnCtrlDevTray(DEV_HANDLE hBurnDEV, DEV_TRAY_STAT bOpen);


	/*******************************************************************************
	* 名称  : BurnCtrlLoadDisc
	* 描述  : 加载光盘
	* 参数  :
	hBurnDEV : 光驱设备句柄
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int BurnCtrlLoadDisc(DEV_HANDLE hBurnDEV);


	/*******************************************************************************
	* 名称  : BurnCtrlFormatDisc
	* 描述  : 格式化光盘, 分轨道, 创建UDF文件系统, 准备开始写数据; 目前只支持一个分区，2个轨道
	* 参数  :
	hBurnDEV : 光驱设备句柄
	szDiscName: 光盘名称
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int	BurnCtrlFormatDisc(DEV_HANDLE hBurnDEV, char *szDiscName);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_WriteData
	* 描述  : 向已创建的文件中写入数据
	* 参数  :
	hBurnDEV : 光驱设备句柄
	hFile    : Burn_Ctrl_CreateFile返回的文件句柄
	buf      : 数据缓冲
	size     : 数据大小
	* 返回值: 1：打开，0：关闭，其他为错误代码
	*******************************************************************************/
	static int	Burn_Ctrl_WriteData(DEV_HANDLE hBurnDEV, FILE_HANDLE hBurnFile, char *buf, int size);

#if 0
	/*******************************************************************************
	* 名称  : Burn_Ctrl_CreateFile
	* 描述  : 创建文件，开始写数据
	* 参数  :
	hBurnDEV : 光驱设备句柄
	pDir   : 目录节点指针, NULL为根目录, Burn_Ctrl_CreateDir的返回值
	szFileName: 文件名称
	filesize: 默认0
	* 返回值: 文件节点指针，NULL：创建失败
	*******************************************************************************/
	FILE_HANDLE Burn_Ctrl_CreateFile(DEV_HANDLE hBurnDEV, DIR_HANDLE hDir, char *FileName, uint64_t filesize);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_WriteData
	* 描述  : 向文件中写数据, size = 32 * 1024
	* 参数  :
	hBurnDEV : 光驱设备句柄
	hFile  : 文件节点指针，Burn_Ctrl_CreateFile的返回值
	pBuffer: 数据buffer
	size   : 数据大小，必须是 32*1024 的整数倍
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	int	Burn_Ctrl_WriteData(DEV_HANDLE hBurnDEV, FILE_HANDLE hFile, unsigned char *pBuffer, int size);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_CloseFile
	* 描述  : 关闭文件
	* 参数  :
	hBurnDEV : 光驱设备句柄
	hFile  : 文件节点指针，Burn_Ctrl_CreateFile的返回值
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	int Burn_Ctrl_CloseFile(DEV_HANDLE hBurnDEV, FILE_HANDLE hFile);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_FillAllDiscEmptyData
	* 描述  : 填充光盘所有剩余空间(在刻录停止之后，封盘之前调用)
	* 参数  :
	hBurnDEV : 光驱设备句柄
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	int Burn_Ctrl_FillAllDiscEmptyData(DEV_HANDLE hBurnDEV);
#endif
	/*******************************************************************************
	* 名称  : BurnCtrlStartBurn
	* 描述  : 开始光盘刻录
	* 参数  :
	hBurnDEV : 光驱设备句柄
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int BurnCtrlStartBurn(DEV_HANDLE hBurnDEV, INTERFACE_TYPE type);

	/*******************************************************************************
	* 名称  : BurnCtrlStopBurn
	* 描述  : 封闭刻录轨道; 调用该接口后，光盘将不可写;
	* 参数  :
	hBurnDEV : 光驱设备句柄
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int BurnCtrlStopBurn(DEV_HANDLE hBurnDEV);

	/*******************************************************************************
	* 名称  : BurnCtrlDevGetDiscInfo
	* 描述  : 操作光驱硬件获取光盘信息
	* 参数  :
	hBurnDEV : 光驱设备句柄
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int BurnCtrlDevGetDiscInfo(DEV_HANDLE hBurnDEV);

	/*******************************************************************************
	* 名称  : BurnCtrlCopyDisc
	* 描述  : 光盘复制,直接轨道拷贝，但是保留轨道不复制
	* 参数  :
	hDevSrc  : 拷贝源光驱设备句柄
	hDevDst  : 拷贝目标光驱设备句柄
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int BurnCtrlCopyDisc(DEV_HANDLE hDevSrc, DEV_HANDLE hDevDst);

	/*******************************************************************************
	* 名称  : BurnCtrlResumeDisc
	* 描述  : 光盘恢复,如果在刻录过程中断电，调用该函数，恢复文件系统
	* 参数  :
	hBurnDEV : 光驱设备句柄
	DiscName : 光盘名称
	DirName : 目录名称
	FileName : 文件名称
	FillSize : 填充的数据数
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int BurnCtrlResumeDisc(DEV_HANDLE hBurnDEV, char *DiscName, char *DirName, char *FileName, int FillSize);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_GetReserveData
	* 描述  : 获得保留轨道数据, 对于已经封盘的光盘才有效, 读取时调用
	* 参数  :
	hBurnDEV : 光驱设备句柄
	pBuffer: 返回保留数据指针
	pSize  : 返回Buffer长度
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int BurnCtrlGetReserveData(DEV_HANDLE hBurnDEV, unsigned char **pBuffer, int *pSize);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_GetReserveBuffer
	* 描述  : 获得保留轨道数据指针, 这个函数在保留轨道前调用，直接修改Buffer，在
	封盘时写入该数据,大小 32K， 刻录时调用
	* 参数  :
	hBurnDEV : 光驱设备句柄
	pBuffer: 返回保留数据指针
	pSize  : 返回Buffer长度
	* 返回值: 0: 成功，其他为错误值
	* 作者  : passion
	* 日期  : 2017.3.31
	*******************************************************************************/
	static int BurnCtrlGetReserveBuffer(DEV_HANDLE hBurnDEV, unsigned char **pBuffer, int *pSize);

	static int BurnCtrlCloseDisc(DEV_HANDLE hBurnDEV);

	static int BurnCtrlUpdateBurningProject(DEV_HANDLE hBurnDEV);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_CreateFile
	* 描述  : 在光盘上创建文件
	* 参数  :
	hBurnDEV : 光驱设备句柄
	file_name: 文件名以及刻录全路径
	* 返回值: 文件句柄，NULL为错误
	*******************************************************************************/
	static FILE_HANDLE Burn_Ctrl_CreateFile(DEV_HANDLE hBurnDEV, DIR_HANDLE hBurnDir, char *file_name);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_CreateDir
	* 描述  : 创建目录
	* 参数  :
	hBurnDEV : 光驱设备句柄
	DirName : 目录名称，不能为空
	* 返回值: 目录节点指针, Burn_Ctrl_CreateFile函数会用到, NULL:创建目录失败
	参数:	nDevNo : 设备号，0-n
	DirName : 要创建的目录,方式为"/root/test1dir/test2dir"
	*******************************************************************************/
	static DIR_HANDLE Burn_Ctrl_CreateDir(DEV_HANDLE hBurnDEV, char *DirName);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_CloseFile
	* 描述  : 关闭已创建的文件
	* 参数  :
	hBurnDEV : 光驱设备句柄
	hFile    : Burn_Ctrl_CreateFile返回的文件句柄
	* 返回值: 1：打开，0：关闭，其他为错误代码
	*******************************************************************************/
	static int	Burn_Ctrl_CloseFile(DEV_HANDLE hBurnDEV, FILE_HANDLE hBurnFile);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_AddBurnLocalFile
	* 描述  : 添加本地文件刻录列表，通过调用此函数，刻录SDK可将添加的本地文件直接刻录在
	*         光盘上
	* 参数  :
	hBurnDEV   : 光驱设备句柄
	burn_dir   : 刻入光盘上的目录路径
	file_path  : 本地文件名全路径
	* 返回值: 1：打开，0：关闭，其他为错误代码
	*******************************************************************************/
	static int	Burn_Ctrl_AddBurnLocalFile(DEV_HANDLE hBurnDEV, char *burn_dir, char *file_path);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_DelBurnLocalFile
	* 描述  : 删除本地文件刻录列表中记录的指定刻录文件
	* 参数  :
	hBurnDEV   : 光驱设备句柄
	file_path  : 本地文件名全路径
	* 返回值: 1：打开，0：关闭，其他为错误代码
	*******************************************************************************/
	static int	Burn_Ctrl_DelBurnLocalFile(DEV_HANDLE hBurnDEV, char *file_path);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_AddBurnLocalDir
	* 描述  : 添加本地路径刻录列表，通过调用此函数，刻录SDK可将添加的本地目录下所有文件
	*         直接刻录在光盘上
	* 参数  :
	hBurnDEV   : 光驱设备句柄
	dir_name  : 文件名以及刻录全路径
	* 返回值: 1：打开，0：关闭，其他为错误代码
	*******************************************************************************/
	static int	Burn_Ctrl_AddBurnLocalDir(DEV_HANDLE hBurnDEV, char *burn_dir, char *dir_path);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_DelBurnLocalDir
	* 描述  : 删除本地目录刻录列表中记录的指定刻录目录
	* 参数  :
	hBurnDEV   : 光驱设备句柄
	dir_name  : 文件名以及刻录全路径
	* 返回值: 1：打开，0：关闭，其他为错误代码
	*******************************************************************************/
	static int	Burn_Ctrl_DelBurnLocalDir(DEV_HANDLE hBurnDEV, char *dir_path);

	static int Burn_Ctrl_Dev_Get_Ready(DEV_HANDLE hBurnDEV);

	static DEV_TRAY_STAT Burn_Ctrl_Dev_Get_TrayState(DEV_HANDLE hBurnDEV);

	static int Burn_Ctrl_Dev_Get_HaveDisc(DEV_HANDLE hBurnDEV);

	static int Burn_Ctrl_Dev_LoadDisc(DEV_HANDLE hBurnDEV);

	static int Burn_Ctrl_Dev_Get_DiscCanWrite(DEV_HANDLE hBurnDEV);

	static int Burn_Ctrl_Dev_FormatDisc(DEV_HANDLE hBurnDEV);

	static int Start_Burning_State_WatchDog(DEV_HANDLE hBurnDEV);

	static int Burn_Ctrl_LockDoor(DEV_HANDLE hBurnDEV, int bLocked);

	/*******************************************************************************
	* 名称  : Burn_Ctrl_SetRecordStatus
	* 描述  : 设置光驱刻录状态（用于光盘格式化后刻录失败未封盘，再次格式化前设置刻录状态）

	* 参数  :
	hBurnDEV   : 光驱设备句柄
	* 返回值:
	* 作者  : hexj
	* 日期  : 2014.8.5
	*******************************************************************************/
	static void Burn_Ctrl_SetRecordStatus(DEV_HANDLE hBurnDEV, BOOL bRecordStatus);

	static int Start_Local_File_Burning(DEV_HANDLE hBurnDEV);

	static int Stop_Local_File_Burning(DEV_HANDLE hBurnDEV);

	static int Set_Local_File_Service_Mode(DEV_HANDLE hBurnDEV);


	//private:
	//	DEV_HANDLE		m_handleDev;
	//	char			m_szDiscName[260];
	//	DEV_TRAY_STAT	m_trayStat;
	//	INTERFACE_TYPE	m_interfaceType;
};


/*=============================================================================
 * 光驱设备设置类
 *===========================================================================*/
class CBurnDev
{
public:
	CBurnDev();
	~CBurnDev();

public:
	/*******************************************************************************
	* 名称  : Burn_Get_DeviceNum
	* 描述  : 获得托盘状态(打开/关闭)
	* 参数  :
	dev_sys_info_ptr : 光驱系统设备结构
	* 返回值: 1：打开，0：关闭，其他为错误代码
	*******************************************************************************/
	static int Burn_Get_DeviceNum(DEV_SYS_INFO *dev_sys_info_ptr);

	/*******************************************************************************
	* 名称  : Burn_Dev_Init
	* 描述  : 初始化指定光驱
	* 参数  :
	DevName : 驱动名称：
	Linux核心如: /dev/sr0, /dev/sr1
	Windows核心：NULL
	派美雅     ：配置文件存放路径
	爱普生     ：IP地址
	Kernel  : 指定刻录核心，默认为linux核心
	* 返回值: 光驱设备句柄，NULL为失败
	*******************************************************************************/
	static DEV_HANDLE Burn_Dev_Init(const char *DevName);

	/*******************************************************************************
	* 名称  : Burn_Dev_Release
	* 描述  : 卸载光驱
	* 参数  :
	hBurnDEV : 光驱设备句柄
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int Burn_Dev_Release(DEV_HANDLE hBurnDEV);

	/*******************************************************************************
	* 名称  : Burn_Set_WriteSpeed
	* 描述  : 设定刻录速度, 默认为 8
	* 参数  :
	hBurnDEV : 光驱设备句柄
	speed  : 速度，必须是 1 2 4 6 8 12
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	int Burn_Set_WriteSpeed(DEV_HANDLE hBurnDEV, int speed);

	/*******************************************************************************
	* 名称  : Burn_Set_FileType
	* 描述  : 设定刻录媒体文件类型， 默认为TS
	* 参数  :
	hBurnDEV : 光驱设备句柄
	type     : 文件类型
	* 返回值: 0: 成功，其他为错误值
	int Burn_Set_FileType(DEV_HANDLE hBurnDEV, int type);
	*******************************************************************************/

	/*******************************************************************************
	* 名称  : Burn_Set_DiscAlarmSize
	* 描述  : 设定光盘剩余空间报警门限, 默认为500M
	* 参数  :
	hBurnDEV : 光驱设备句柄
	alarm_size: 报警门限值
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int Burn_Set_DiscAlarmSize(DEV_HANDLE hBurnDEV, int alarm_size);

	/*******************************************************************************
	* 名称  : Burn_Set_DiscAlarmWarningSize
	* 描述  : 设定光盘剩余空间预警门限, 默认为0M
	当光盘剩余空间-报警门限=alarm_warning_size时给出即将到达报警门限的提示
	* 参数  :
	hBurnDEV : 光驱设备句柄
	alarm_warning_size: 预警门限值
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int Burn_Set_DiscAlarmWarningSize(DEV_HANDLE hBurnDEV, int alarm_warning_size);

	/*******************************************************************************
	* 名称  : Burn_Set_AutoFormat
	* 描述  : 设定光盘自动格式化开关，默认为打开
	* 参数  :
	hBurnDEV : 光驱设备句柄
	bopen :  BURN_TURE-打开， BURN_FLASE-关闭;
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	int Burn_Set_AutoFormat(DEV_HANDLE hBurnDEV, DEV_TRAY_STAT bopen);

	/*******************************************************************************
	* 名称  : Burn_Set_BurnCache
	* 描述  : 设定刻录缓冲大小，默认为 3*1024*1024
	* 参数  :
	hBurnDEV : 光驱设备句柄
	cache_size: 缓冲值 [64*1024 - 10*1024*1024]
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	int Burn_Set_BurnCache(DEV_HANDLE hBurnDEV, int cache_size);

	/*******************************************************************************
	* 名称  : Burn_Set_AutoMD5
	* 描述  : 设定是否自动进行MD5并刻录在光盘，默认为开启
	* 参数  :
	hBurnDEV : 光驱设备句柄
	bopen :  BURN_TURE-打开， BURN_FLASE-关闭;
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	int Burn_Set_AutoMD5(DEV_HANDLE hBurnDEV, int bopen);

	/*******************************************************************************
	* 名称  : Burn_Set_AutoMD5
	* 描述  : 设定打开光盘时的密码，如果设置密码则光盘会自动被加密，默认为不加密
	* 参数  :
	hBurnDEV : 光驱设备句柄
	password : 密码;
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	int Burn_Set_PassWord(DEV_HANDLE hBurnDEV, char *password);

	/*******************************************************************************
	* 名称  : Burn_Set_PrintProfile
	* 描述  : 打印profile
	* 参数  :
	hBurnDEV : 光驱设备句柄
	* 返回值: 0: 成功
	*******************************************************************************/
	int Burn_Set_PrintProfile(DEV_HANDLE hBurnDEV);

	/*******************************************************************************
	* 名称  : Burn_Set_BurnNum
	* 描述  : 设置刻录份数
	* 参数  :
	hBurnDEV : 光驱设备句柄
	int      : 刻录份数
	* 返回值: 0: 成功
	*******************************************************************************/
	int Burn_Set_BurnNum(DEV_HANDLE hBurnDEV, int num);

	/*******************************************************************************
	* 名称  : Burn_Set_DiscCover
	* 描述  : 设置刻录份数
	* 参数  :
	hBurnDEV   : 光驱设备句柄
	photo_path : 光盘封面要打印的图片路径
	* 返回值: 0: 成功
	*******************************************************************************/
	int Burn_Set_DiscCover(DEV_HANDLE hBurnDEV, char *photo_path);

	/*******************************************************************************
	* 名称  : Burn_Set_Data_Interface
	* 描述  : 设置刻录获取数据接口
	* 参数  :
	hBurnDEV   : 光驱设备句柄
	interface  : 接口类型
	* 返回值: 0: 成功
	*******************************************************************************/
	static int Burn_Set_Data_Interface(DEV_HANDLE hBurnDEV, INTERFACE_TYPE interface1);

	/*******************************************************************************
	* 名称  : Burn_Set_Event_Callback
	* 描述  : 设置刻录事件回调函数
	* 参数  :
	hBurnDEV   : 光驱设备句柄
	important_events  : 回调函数
	val：               回调函数传回的参数
	* 返回值: 0: 成功
	*******************************************************************************/
	static int Burn_Set_Event_Callback(DEV_HANDLE hBurnDEV, CB_EVENTS important_events, void *val);

	static int Burn_Set_Disk(DEV_HANDLE hBurnDEV, BURN_BOOL use_disk, char *backup_path, unsigned long backupsize, unsigned long alarmsize);

	static int Burn_Set_Disc(DEV_HANDLE hBurnDEV, int type, int maxspeed, unsigned long alarmsize, unsigned long alarmwarningsize);

	static int Set_Burn_Buffer_Size(DEV_HANDLE hBurnDEV, unsigned long buf_size);

	static int Burn_Set_DiscName(DEV_HANDLE hBurnDEV, char *name);
};

/*=============================================================================
* 光驱设备参数获取类
*===========================================================================*/
class CBurnDevInfo
{
public:
	CBurnDevInfo(){};
	~CBurnDevInfo(){};
public:

	/*******************************************************************************
	* 名称  : Burn_Get_TrayState
	* 描述  : 获得托盘状态(打开/关闭)
	* 参数  :
	hBurnDEV : 光驱设备句柄
	* 返回值: 1：打开，0：关闭，其他为错误代码
	*******************************************************************************/
	static int	Burn_Get_TrayState(DEV_HANDLE hBurnDEV);

	/*******************************************************************************
	* 名称  : Burn_Get_DevInfo
	* 描述  : 获得光驱信息
	* 参数  :
	hBurnDEV : 光驱设备句柄
	pDevInfo   : 光驱信息指针
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int Burn_Get_DevInfo(DEV_HANDLE hBurnDEV, BURN_DEV_INFO *pDevInfo);

	/*******************************************************************************
	* 名称  : Burn_Get_DiscInfo
	* 描述  : 获得碟片信息
	* 参数  :
	hBurnDEV : 光驱设备句柄
	pDiscInfo  : 碟片信息结构指针
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int Burn_Get_DiscInfo(DEV_HANDLE hBurnDEV, BURN_DISC_INFO *pDiscInfo);

	//获取指定光驱设备运行的所有信息
	static int Get_Burn_Info(DEV_HANDLE hBurnDEV, BURN_INFO *burn_info);

	//获取光驱目前状态-就位/运行中/停止
	static BURN_STAT Get_Running_Burn_State(DEV_HANDLE hBurnDEV);

	//获取光驱运行过程中的状态-正常/异常
	static RUNNING_STATE Get_Running_State(DEV_HANDLE hBurnDEV);

	//获取光驱运行过程中的具体状态信息
	static RUNNING_INFO Get_Running_info(DEV_HANDLE hBurnDEV);

	//获取光驱目前操作的文件状态
	static int Get_Burn_File_State(DEV_HANDLE hBurnDEV, BURN_FILE  *burn_file_info);
	/*******************************************************************************
	* 名称  : Burn_Get_DiskInfo
	* 描述  : 获得碟片信息
	* 参数  :
	hBurnDEV : 硬盘设备句柄
	pDiscInfo  : 硬盘信息结构指针
	* 返回值: 0: 成功，其他为错误值
	*******************************************************************************/
	static int Burn_Get_DiskInfo(DEV_HANDLE hBurnDEV, BURN_DISK_INFO *pDiscInfo);

	/*******************************************************************************
	* 名称  : Burn_Get_DiscCanWrite
	* 描述  : 判断光盘是否可写
	* 参数  :
	hBurnDEV : 光驱设备句柄
	* 返回值: 0: 可写，其他为错误值
	*******************************************************************************/
	static int Burn_Get_DiscCanWrite(DEV_HANDLE hBurnDEV);

	static DEV_TRAY_STAT Get_Running_Tray_State(DEV_HANDLE hBurnDEV);

	static char *Get_Burning_File_Name(DEV_HANDLE hBurnDEV);

	static char *Get_Completed_File_Name(DEV_HANDLE hBurnDEV);
};


#if 0
/*******************************************************************************
* 名称  : Burn_Get_DiscExactType
* 描述  : 获取光盘类型(精确类型)
* 参数  : 
hBurnDEV : 光驱设备句柄
* 返回值: 光盘类型
*******************************************************************************/
int Burn_Get_DiscExactType(DEV_HANDLE hBurnDEV);


/*******************************************************************************
* 名称  : Burn_Get_DiscBasicType
* 描述  : 获取光盘类型(基本类型)(用于设定光驱速度和复制光盘时)
* 参数  : 
hBurnDEV : 光驱设备句柄
* 返回值: 光盘类型
*******************************************************************************/
int Burn_Get_DiscBasicType(DEV_HANDLE hBurnDEV);
#endif


/*=============================================================================
 * 光驱刻录数据类
 *===========================================================================*/
class CBurnData
{
public:
	CBurnData(){};
	~CBurnData(){};
public:
	static int Create_Burn_Environment(DEV_HANDLE hBurnDEV);
	static int Burn_Set_RT_File_Name(char *filename);
	// add interface 
	static void Burn_Get_RT_File_Name(char *filename);
	static int Set_RT_File_Name_Update();

};

class CBurnFileOpr
{
public:
	CBurnFileOpr(){};
	~CBurnFileOpr(){};
public:
	/*******************************************************************************
	* 名称  : Burn_File_Form_Local_File
	* 描述  : 刻录本地文件
	* 参数  :
	hBurnDEV   : 光驱设备句柄
	burn_dir   : 目录路径，刻录后该目录路径会被创建在光盘根目录，文件会直接被刻录
	*                在此目录下，如果不需要目录则传入参数NULL
	file_path  : 本地文件名全路径
	* 返回值: 1：打开，0：关闭，其他为错误代码
	*******************************************************************************/
	static int Burn_File_Form_Local_File(DEV_HANDLE hBurnDEV, char *burn_dir, char *file_path);

	/*******************************************************************************
	* 名称  : Burn_Dir_Form_Local_Dir
	* 描述  : 刻录本地目录
	* 参数  :
	hBurnDEV   : 光驱设备句柄
	burn_dir   : 目录路径，刻录后该目录路径会被创建在光盘根目录，文件会直接被刻录
	*                在此目录下，如果不需要目录则传入参数NULL
	burn_path  : 本地目录名全路径
	* 返回值: 1：打开，0：关闭，其他为错误代码
	*******************************************************************************/
	static int Burn_Dir_Form_Local_Dir(DEV_HANDLE hBurnDEV, char *burn_dir, char *burn_path);

	int Burn_Specific_Dir_To_Disc(DEV_HANDLE hBurnDEV, char *dir_path);

	static int Write_Dev_Buf_Data_To_File(DEV_HANDLE hBurnDEV, FILE_HANDLE hFile);

};

/*******************************************************************************
* 名称  : Burn_Get_LocalFileBurnSchedule
* 描述  : 获取指定的本地文件刻录进度。
* 参数  :
hBurnDEV   : 光驱设备句柄
file_name  : 文件名以及刻录全路径
* 返回值: 1：打开，0：关闭，其他为错误代码
*******************************************************************************/
int	Burn_Get_LocalFileBurnSchedule(DEV_HANDLE hBurnDEV, char *file_name);


/*******************************************************************************
* 名称  : Burn_Get_LocalFileBurnSchedule
* 描述  : 获取指定的本地文件刻录进度。
* 参数  :
hBurnDEV   : 光驱设备句柄
file_name  : 文件名以及刻录全路径
* 返回值: 进度值 0 -100, 100表示全部完成.
*******************************************************************************/
int	Burn_Get_AllLocalFileBurnSchedule(DEV_HANDLE hBurnDEV);

//int Burn_Set_Data_Interface(DEV_HANDLE hBurnDEV, INTERFACE_TYPE interface);


/*******************************************************************************
* 名称  : Burn_Get_TotalWriteSize
* 描述  : 获得光盘整个可写空间
* 参数  :
hBurnDEV : 光驱设备句柄
pTotalSize: 返回整个可写空间
* 返回值: 0: 成功，其他为错误值
*******************************************************************************/
int Burn_Get_TotalWriteSize(DEV_HANDLE hBurnDEV, unsigned long long *pTotalSize);

/*******************************************************************************
* 名称  : Burn_Get_FreeWriteSize
* 描述  : 获得剩余可写空间
* 参数  :
nDevNo   : 设备号，0-n
pFreeSize: 返回剩余可写空间
* 返回值: 0: 成功，其他为错误值
*******************************************************************************/
int Burn_Get_FreeWriteSize(DEV_HANDLE hBurnDEV, unsigned long long *pFreeSize);

class CDiskOpr{
public:
	CDiskOpr(){};
	~CDiskOpr(){};

public:
	static int Set_Disk_Param(DEV_HANDLE hBurnDEV, BURN_BOOL use_dev, int backup_size, int alarm_size, char *disk_path);
	static int Disk_Create_BackUpFile(DEV_HANDLE hBurnDEV, const char *file_name);
	static int Disk_Close_BackUpFile(DEV_HANDLE hBurnDEV);
	static int Disk_Read_BackUpFile(DEV_HANDLE hBurnDEV, char *buf, int buf_size);
	static int Start_Disk_BackUp(DEV_HANDLE hBurnDEV);
	static int Stop_Disk_BackUp(DEV_HANDLE hBurnDEV);
};

int Fill_Data_To_Dev_Buf(DEV_HANDLE hBurnDEV, char *buf_ptr, int len, int block);
void Burn_Set_Pause_Mode(BURN_BOOL val);


#ifdef LINUX
#ifdef __cplusplus
}
#endif
#endif
#endif
