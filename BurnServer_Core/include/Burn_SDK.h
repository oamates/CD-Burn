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

// ��������
typedef enum{
	DVDDRIVER_UNKNOWN = 0,	// δ֪��������
	DVDDRIVER_PRVIDVD,		// ר�ÿ�¼DVD����
	DVDDRIVER_CDR,			// CDֻ������
	DVDDRIVER_CDRW,			// CD��д����
	DVDDRIVER_DVDR,			// DVDֻ������
	DVDDRIVER_DVDRW,		// DVD��д����
	DVDDRIVER_BLUER,		// ����ֻ������
	DVDDRIVER_BLUERW,		// �����д����
}DRIVER_TYPE;

// ��������
typedef enum{
	DISC_UNKNOWN = 0,	// δ֪����
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
    int              dev_id;         // �豸ID
//    int              kernel;         // ��¼��������
    BURN_STAT        burn_state;     // ��¼״̬(�Ƿ��Ѿ���ʼ��¼)
    INTERFACE_TYPE   interface1;      // ��ȡ���ݵĽӿ�����
    char             dev_path[1000]; // �豸·�������ݿ�¼���ĵĲ�ͬ���в�ͬ
#ifdef LINUX
    XKD_DVDDRV       hDVD;           //Linux�ײ�������
#endif
#ifdef WIN32
	BURNHANDLE       hDVD;           //Windows �ײ�������
#endif
    BURN_DATA_T      data;           // �豸��Ӧ�����������
    RingBuffer       hBuf;           // ���ݻ���
	FILE *			 fd;			 //Ӳ�̱��ݾ��
};
typedef struct Dev_Object *DEV_HANDLE;



typedef struct DEV_SYS_INFO
{
	int dev_num;
	char dev1[200];
	char dev2[200];
	char dev3[200];
	char dev4[200];
}DEV_SYS_INFO_T;

/* ������Ϣ*/
typedef struct{
	BURN_BOOL    has_disc;	           // �Ƿ���ڹ���
	BURN_BOOL    write_flag;           // �Ƿ���������д������
	int 	 	 type;			       // ��������	DISC_TYPE
	int			 maxspeed;		       // ����ٶ�(д�ٶ�)
	unsigned int discsize;		       // ��������(MB)
	unsigned int usedsize;		       // ��ʹ�õĴ�С(MB)	
	unsigned int freesize;	           // ���ô�С(MB)
	unsigned int writecnt;	           // ���ô�С(MB)
	unsigned int alarmsize;	           // ��������������С(MB)
    unsigned int alarmwarningsize;     // ��������Ԥ���ռ��С(MB)
	char         disc_name[600];       // ��������
}BURN_DISC_INFO_T;

/* �����豸��Ϣ*/
typedef struct{
	BURN_BOOL has_dev;				   // �Ƿ�����豸
	char szVender[128];				   // ��������
	int  drvtype;					   // ��������			DVDDRIVER_TYPE
	int  discsupts;					   // ����֧������
//	unsigned char disclist[255];       // �����������б���DVDDISC_TYPE
}BURN_DEV_INFO_T;

/* Ӳ���豸��Ϣ*/
typedef struct{
	BURN_BOOL  use_disk;				       // �Ƿ�����豸
	BURN_BOOL  write_flag;			       // �Ƿ������Ӳ��д������
//	unsigned int disksize;	           // Ӳ������(MB)
	unsigned long backupsize;	       // ��������(MB)
	unsigned long usedsize;		       // ��ʹ�ñ��������Ĵ�С(MB)	
	unsigned long freesize;		       // ���ñ���������С(MB)
	unsigned long alarmsize;	       // ��������������С(MB)
    char *backup_path;                 // Ӳ�̱���ȫ·��
}BURN_DISK_INFO_T;

/* ����״̬��Ϣ*/
typedef struct{
	DEV_TRAY_STAT  tray_state;		   // ����״̬ 
	RUNNING_STATE  running_state;      // ����״̬
	RUNNING_INFO   running_info;       // ����״̬��Ҫ����
	char           describe[2048];     // ����״̬��������
}BURN_RUN_STATE_T;

/* ����״̬��Ϣ*/
typedef struct{
//	int   file_size;		           // ��ǰ���ڿ�¼���ļ���С
//  int   burn_size;                   // ���ڿ�¼���ļ��ѿ�������ݴ�С
	char *burning_file_name;		   // ���ڿ�¼���ļ���
	char *completed_file_name;   	   // ���ڿ�¼���ļ���
}BURN_FILE_T;

/*��¼ҵ�����*/
typedef struct{
	int  cache; 					    // ƽ̨�޹ز㻺�� 
	int  md5_bool;					    // MD5���ܿ���
    int  burn_num;                      // ���̿�¼����
	int	 auto_format;                   // �Զ���ʽ��
    char passwd[100];                   // ��������
    int  file_path[600];                // ��¼Դ�ļ�·��
    int  disc_cover[600];	    	    // ��¼����ͼƬ·��
}BURN_PARAM_T;

/*�ص�����*/
typedef	int (* CB_EVENTS)(DEV_HANDLE hBurnDEV, const BURN_RUN_STATE_T*, void*);	

typedef struct
{
	void *val;		
	CB_EVENTS important_events;
}CALLBACK_T;

/* �������������Ϣ*/
typedef struct{
	int               dev_id;           // �豸ID��ÿ��IDӦ��һ���������Թ����Ĳ��������ṩ�豸ID
	BURN_DEV_INFO_T   dev_info;         // �����豸��Ϣ
    BURN_DISC_INFO_T  disc_info;        // ������Ϣ
    BURN_DISK_INFO_T  disk_info;        // Ӳ����Ϣ
    BURN_RUN_STATE_T  run_state;        // ����״̬
    BURN_PARAM_T      burn_param;       // ��¼����
    BURN_FILE_T       burn_file;        // ��¼�������ļ������
	CALLBACK_T        event;            // ��Ҫ�¼��ص�
}BURN_INFO_T;

#ifdef LINUX
	typedef Xkd_DVDSDK_DIR DIR_HANDLE;		        // Ŀ¼�ڵ�ָ��
	typedef Xkd_DVDSDK_FILE FILE_HANDLE;		    // �ļ�ָ��
#else
	typedef void* DIR_HANDLE;		        // Ŀ¼�ڵ�ָ��
	typedef void* FILE_HANDLE;		        // �ļ�ָ��
#endif 




/*=============================================================================
 * �����豸������
 *===========================================================================*/


/*******************************************************************************
* ����  : Burn_Ctrl_DevTray
* ����  : ��/�ر�����
* ����  : 
	hBurnDEV : �����豸���
	bOpen  : B_DEV_TRAY_OPEN:������, B_DEV_TRAY_CLOSE:�ر�����
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int	Burn_Ctrl_DevTray(DEV_HANDLE hBurnDEV, DEV_TRAY_STAT bOpen);


/*******************************************************************************
* ����  : Burn_Ctrl_LoadDisc
* ����  : ���ع���
* ����  : 
	hBurnDEV : �����豸���
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Ctrl_LoadDisc(DEV_HANDLE hBurnDEV);


/*******************************************************************************
* ����  : Burn_Ctrl_FormatDisc
* ����  : ��ʽ������, �ֹ��, ����UDF�ļ�ϵͳ, ׼����ʼд����; Ŀǰֻ֧��һ��������2�����
* ����  : 
	hBurnDEV : �����豸���
	szDiscName: ��������
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int	Burn_Ctrl_FormatDisc(DEV_HANDLE hBurnDEV, char *szDiscName);

#if 0
/*******************************************************************************
* ����  : Burn_Ctrl_CreateFile
* ����  : �����ļ�����ʼд����
* ����  : 
	hBurnDEV : �����豸���
	pDir   : Ŀ¼�ڵ�ָ��, NULLΪ��Ŀ¼, Burn_Ctrl_CreateDir�ķ���ֵ
	szFileName: �ļ�����
	filesize: Ĭ��0
* ����ֵ: �ļ��ڵ�ָ�룬NULL������ʧ��
*******************************************************************************/
FILE_HANDLE Burn_Ctrl_CreateFile(DEV_HANDLE hBurnDEV, DIR_HANDLE hDir, char *FileName, uint64_t filesize);

/*******************************************************************************
* ����  : Burn_Ctrl_WriteData
* ����  : ���ļ���д����, size = 32 * 1024
* ����  : 
	hBurnDEV : �����豸���
	hFile  : �ļ��ڵ�ָ�룬Burn_Ctrl_CreateFile�ķ���ֵ
	pBuffer: ����buffer
	size   : ���ݴ�С�������� 32*1024 ��������
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int	Burn_Ctrl_WriteData(DEV_HANDLE hBurnDEV, FILE_HANDLE hFile, unsigned char *pBuffer, int size);

/*******************************************************************************
* ����  : Burn_Ctrl_CloseFile
* ����  : �ر��ļ�
* ����  : 
	hBurnDEV : �����豸���
	hFile  : �ļ��ڵ�ָ�룬Burn_Ctrl_CreateFile�ķ���ֵ
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Ctrl_CloseFile(DEV_HANDLE hBurnDEV, FILE_HANDLE hFile);

/*******************************************************************************
* ����  : Burn_Ctrl_FillAllDiscEmptyData
* ����  : ����������ʣ��ռ�(�ڿ�¼ֹ֮ͣ�󣬷���֮ǰ����)
* ����  : 
	hBurnDEV : �����豸���
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Ctrl_FillAllDiscEmptyData(DEV_HANDLE hBurnDEV);
#endif

/*******************************************************************************
* ����  : Burn_Ctrl_StartBurn
* ����  : ��ʼ���̿�¼
* ����  : 
	hBurnDEV : �����豸���
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Ctrl_StartBurn(DEV_HANDLE hBurnDEV, INTERFACE_TYPE type);

/*******************************************************************************
* ����  : Burn_Ctrl_StopBurn
* ����  : ��տ�¼���; ���øýӿں󣬹��̽�����д;
* ����  : 
	hBurnDEV : �����豸���
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Ctrl_StopBurn(DEV_HANDLE hBurnDEV);

/*******************************************************************************
* ����  : Burn_Ctrl_Dev_Get_DiscInfo
* ����  : ��������Ӳ����ȡ������Ϣ
* ����  : 
	hBurnDEV : �����豸���
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Ctrl_Dev_Get_DiscInfo(DEV_HANDLE hBurnDEV);

/*******************************************************************************
* ����  : Burn_Ctrl_CopyDisc
* ����  : ���̸���,ֱ�ӹ�����������Ǳ������������
* ����  : 
	hDevSrc  : ����Դ�����豸���
	hDevDst  : ����Ŀ������豸���
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Ctrl_CopyDisc(DEV_HANDLE hDevSrc, DEV_HANDLE hDevDst);

/*******************************************************************************
* ����  : Burn_Ctrl_ResumeDisc
* ����  : ���ָ̻�,����ڿ�¼�����жϵ磬���øú������ָ��ļ�ϵͳ
* ����  : 
	hBurnDEV : �����豸���
	DiscName : ��������
	DirName : Ŀ¼����
	FileName : �ļ�����
	FillSize : ����������
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Ctrl_ResumeDisc(DEV_HANDLE hBurnDEV, char *DiscName, char *DirName, char *FileName, int FillSize);

/*******************************************************************************
* ����  : Burn_Ctrl_GetReserveData
* ����  : ��ñ����������, �����Ѿ����̵Ĺ��̲���Ч, ��ȡʱ����
* ����  : 
	hBurnDEV : �����豸���
	pBuffer: ���ر�������ָ��
	pSize  : ����Buffer����
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Ctrl_GetReserveData(DEV_HANDLE hBurnDEV, unsigned char **pBuffer, int *pSize);

/*******************************************************************************
* ����  : Burn_Ctrl_GetReserveBuffer
* ����  : ��ñ����������ָ��, ��������ڱ������ǰ���ã�ֱ���޸�Buffer����
		  ����ʱд�������,��С 32K�� ��¼ʱ����
* ����  : 
	hBurnDEV : �����豸���
	pBuffer: ���ر�������ָ��
	pSize  : ����Buffer����
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Burn_Ctrl_GetReserveBuffer(DEV_HANDLE hBurnDEV, unsigned char **pBuffer, int *pSize);


int Burn_Ctrl_CloseDisc(DEV_HANDLE hBurnDEV);

int Burn_Ctrl_Update_Burning_Project(DEV_HANDLE hBurnDEV);
/*=============================================================================
 * �����豸������
 *===========================================================================*/
/*******************************************************************************
* ����  : Burn_Dev_Init
* ����  : ��ʼ��ָ������
* ����  : 
	DevName : �������ƣ�
              Linux������: /dev/sr0, /dev/sr1
              Windows���ģ�NULL
              ������     �������ļ����·��
              ������     ��IP��ַ
    Kernel  : ָ����¼���ģ�Ĭ��Ϊlinux����
* ����ֵ: �����豸�����NULLΪʧ��
*******************************************************************************/
DEV_HANDLE Burn_Dev_Init(const char *DevName);

/*******************************************************************************
* ����  : Burn_Dev_Release
* ����  : ж�ع���
* ����  : 
	hBurnDEV : �����豸��� 
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Dev_Release(DEV_HANDLE hBurnDEV);


/*******************************************************************************
* ����  : Burn_Set_WriteSpeed
* ����  : �趨��¼�ٶ�, Ĭ��Ϊ 8
* ����  : 
	hBurnDEV : �����豸���
	speed  : �ٶȣ������� 1 2 4 6 8 12
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Set_WriteSpeed(DEV_HANDLE hBurnDEV, int speed);

/*******************************************************************************
* ����  : Burn_Set_FileType
* ����  : �趨��¼ý���ļ����ͣ� Ĭ��ΪTS
* ����  : 
	hBurnDEV : �����豸���
	type     : �ļ�����
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
int Burn_Set_FileType(DEV_HANDLE hBurnDEV, int type);
*******************************************************************************/

/*******************************************************************************
* ����  : Burn_Set_DiscAlarmSize
* ����  : �趨����ʣ��ռ䱨������, Ĭ��Ϊ500M
* ����  : 
	hBurnDEV : �����豸���
	alarm_size: ��������ֵ
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Set_DiscAlarmSize(DEV_HANDLE hBurnDEV, int alarm_size);

/*******************************************************************************
* ����  : Burn_Set_DiscAlarmWarningSize
* ����  : �趨����ʣ��ռ�Ԥ������, Ĭ��Ϊ0M
          ������ʣ��ռ�-��������=alarm_warning_sizeʱ�����������ﱨ�����޵���ʾ
* ����  : 
	hBurnDEV : �����豸���
	alarm_warning_size: Ԥ������ֵ
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Set_DiscAlarmWarningSize(DEV_HANDLE hBurnDEV, int alarm_warning_size);

/*******************************************************************************
* ����  : Burn_Set_AutoFormat
* ����  : �趨�����Զ���ʽ�����أ�Ĭ��Ϊ��
* ����  : 
	hBurnDEV : �����豸���
	bopen :  BURN_TURE-�򿪣� BURN_FLASE-�ر�;
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Set_AutoFormat(DEV_HANDLE hBurnDEV, DEV_TRAY_STAT bopen);

/*******************************************************************************
* ����  : Burn_Set_BurnCache
* ����  : �趨��¼�����С��Ĭ��Ϊ 3*1024*1024
* ����  : 
	hBurnDEV : �����豸���
	cache_size: ����ֵ [64*1024 - 10*1024*1024]
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Set_BurnCache(DEV_HANDLE hBurnDEV, int cache_size);

/*******************************************************************************
* ����  : Burn_Set_AutoMD5
* ����  : �趨�Ƿ��Զ�����MD5����¼�ڹ��̣�Ĭ��Ϊ����
* ����  : 
	hBurnDEV : �����豸���
	bopen :  BURN_TURE-�򿪣� BURN_FLASE-�ر�;
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Set_AutoMD5(DEV_HANDLE hBurnDEV, int bopen);

/*******************************************************************************
* ����  : Burn_Set_AutoMD5
* ����  : �趨�򿪹���ʱ�����룬���������������̻��Զ������ܣ�Ĭ��Ϊ������
* ����  : 
	hBurnDEV : �����豸���
	password : ����;
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Set_PassWord(DEV_HANDLE hBurnDEV, char *password);

/*******************************************************************************
* ����  : Burn_Set_PrintProfile
* ����  : ��ӡprofile
* ����  : 
	hBurnDEV : �����豸���
* ����ֵ: 0: �ɹ�
*******************************************************************************/
int Burn_Set_PrintProfile(DEV_HANDLE hBurnDEV);

/*******************************************************************************
* ����  : Burn_Set_BurnNum
* ����  : ���ÿ�¼����
* ����  : 
	hBurnDEV : �����豸���
    int      : ��¼����
* ����ֵ: 0: �ɹ�
*******************************************************************************/
int Burn_Set_BurnNum(DEV_HANDLE hBurnDEV, int num);

/*******************************************************************************
* ����  : Burn_Set_DiscCover
* ����  : ���ÿ�¼����
* ����  : 
	hBurnDEV   : �����豸���
    photo_path : ���̷���Ҫ��ӡ��ͼƬ·��
* ����ֵ: 0: �ɹ�
*******************************************************************************/
int Burn_Set_DiscCover(DEV_HANDLE hBurnDEV, char *photo_path);

/*******************************************************************************
* ����  : Burn_Set_Data_Interface
* ����  : ���ÿ�¼��ȡ���ݽӿ� 
* ����  : 
	hBurnDEV   : �����豸���
    interface  : �ӿ�����
* ����ֵ: 0: �ɹ�
*******************************************************************************/
int Burn_Set_Data_Interface(DEV_HANDLE hBurnDEV, INTERFACE_TYPE interface1);

/*******************************************************************************
* ����  : Burn_Set_Event_Callback
* ����  : ���ÿ�¼�¼��ص����� 
* ����  : 
	hBurnDEV   : �����豸���
    important_events  : �ص�����
	val��               �ص��������صĲ���
* ����ֵ: 0: �ɹ�
*******************************************************************************/
int Burn_Set_Event_Callback(DEV_HANDLE hBurnDEV, CB_EVENTS important_events, void *val);
/*=============================================================================
 * �����豸������ȡ��
 *===========================================================================*/

/*******************************************************************************
* ����  : Burn_Get_DeviceNum
* ����  : �������״̬(��/�ر�)
* ����  : 
	dev_sys_info_ptr : ����ϵͳ�豸�ṹ
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
*******************************************************************************/
 int Burn_Get_DeviceNum(DEV_SYS_INFO_T *dev_sys_info_ptr);


/*******************************************************************************
* ����  : Burn_Get_TrayState
* ����  : �������״̬(��/�ر�)
* ����  : 
	hBurnDEV : �����豸���
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
*******************************************************************************/
int	Burn_Get_TrayState(DEV_HANDLE hBurnDEV);


/*******************************************************************************
* ����  : Burn_Get_DevInfo
* ����  : ��ù�����Ϣ
* ����  : 
	hBurnDEV : �����豸���
	pDevInfo   : ������Ϣָ��
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Get_DevInfo(DEV_HANDLE hBurnDEV, BURN_DEV_INFO_T *pDevInfo);


/*******************************************************************************
* ����  : Burn_Get_DiscInfo
* ����  : ��õ�Ƭ��Ϣ
* ����  : 
	hBurnDEV : �����豸���
	pDiscInfo  : ��Ƭ��Ϣ�ṹָ��
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Get_DiscInfo(DEV_HANDLE hBurnDEV, BURN_DISC_INFO_T *pDiscInfo);

/*******************************************************************************
* ����  : Burn_Get_DiskInfo
* ����  : ��õ�Ƭ��Ϣ
* ����  : 
	hBurnDEV : Ӳ���豸���
	pDiscInfo  : Ӳ����Ϣ�ṹָ��
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Get_DiskInfo(DEV_HANDLE hBurnDEV, BURN_DISK_INFO_T *pDiscInfo);

#if 0
/*******************************************************************************
* ����  : Burn_Get_DiscExactType
* ����  : ��ȡ��������(��ȷ����)
* ����  : 
	hBurnDEV : �����豸���
* ����ֵ: ��������
*******************************************************************************/
int Burn_Get_DiscExactType(DEV_HANDLE hBurnDEV);


/*******************************************************************************
* ����  : Burn_Get_DiscBasicType
* ����  : ��ȡ��������(��������)(�����趨�����ٶȺ͸��ƹ���ʱ)
* ����  : 
	hBurnDEV : �����豸���
* ����ֵ: ��������
*******************************************************************************/
int Burn_Get_DiscBasicType(DEV_HANDLE hBurnDEV);
#endif

/*******************************************************************************
* ����  : Burn_Get_DiscCanWrite
* ����  : �жϹ����Ƿ��д
* ����  : 
	hBurnDEV : �����豸���
* ����ֵ: 0: ��д������Ϊ����ֵ
*******************************************************************************/
int Burn_Get_DiscCanWrite(DEV_HANDLE hBurnDEV);


/*******************************************************************************
* ����  : Burn_Get_TotalWriteSize
* ����  : ��ù���������д�ռ�
* ����  : 
	hBurnDEV : �����豸���
	pTotalSize: ����������д�ռ�
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Get_TotalWriteSize(DEV_HANDLE hBurnDEV, unsigned long long *pTotalSize);


/*******************************************************************************
* ����  : Burn_Get_FreeWriteSize
* ����  : ���ʣ���д�ռ�
* ����  : 
	nDevNo   : �豸�ţ�0-n
	pFreeSize: ����ʣ���д�ռ�
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Burn_Get_FreeWriteSize(DEV_HANDLE hBurnDEV, unsigned long long *pFreeSize);


/*=============================================================================
 * ������¼������
 *===========================================================================*/
/*******************************************************************************
* ����  : Burn_Ctrl_CreateDir
* ����  : ����Ŀ¼
* ����  : 
	hBurnDEV : �����豸���
	DirName : Ŀ¼���ƣ�����Ϊ��
* ����ֵ: Ŀ¼�ڵ�ָ��, Burn_Ctrl_CreateFile�������õ�, NULL:����Ŀ¼ʧ��
          ����:	nDevNo : �豸�ţ�0-n
				DirName : Ҫ������Ŀ¼,��ʽΪ"/root/test1dir/test2dir"
*******************************************************************************/
DIR_HANDLE Burn_Ctrl_CreateDir(DEV_HANDLE hBurnDEV, char *DirName);

/*******************************************************************************
* ����  : Burn_Ctrl_CreateFile
* ����  : �ڹ����ϴ����ļ� 
* ����  : 
	hBurnDEV : �����豸���
	file_name: �ļ����Լ���¼ȫ·��
* ����ֵ: �ļ������NULLΪ����
*******************************************************************************/
FILE_HANDLE Burn_Ctrl_CreateFile(DEV_HANDLE hBurnDEV, DIR_HANDLE hBurnDir, char *file_name);


/*******************************************************************************
* ����  : Burn_Ctrl_WriteData
* ����  : ���Ѵ������ļ���д������
* ����  : 
	hBurnDEV : �����豸���
	hFile    : Burn_Ctrl_CreateFile���ص��ļ����
	buf      : ���ݻ���
	size     : ���ݴ�С
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
*******************************************************************************/
int	Burn_Ctrl_WriteData(DEV_HANDLE hBurnDEV, FILE_HANDLE hBurnFile, char *buf, int size);


/*******************************************************************************
* ����  : Burn_Ctrl_CloseFile
* ����  : �ر��Ѵ������ļ� 
* ����  : 
	hBurnDEV : �����豸���
	hFile    : Burn_Ctrl_CreateFile���ص��ļ����
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
*******************************************************************************/
int	Burn_Ctrl_CloseFile(DEV_HANDLE hBurnDEV, FILE_HANDLE hBurnFile);


/*******************************************************************************
* ����  : Burn_Ctrl_AddBurnLocalFile
* ����  : ��ӱ����ļ���¼�б�ͨ�����ô˺�������¼SDK�ɽ���ӵı����ļ�ֱ�ӿ�¼��
*         ������
* ����  : 
	hBurnDEV   : �����豸���
	burn_dir   : ��������ϵ�Ŀ¼·�� 
	file_path  : �����ļ���ȫ·��
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
*******************************************************************************/
int	Burn_Ctrl_AddBurnLocalFile(DEV_HANDLE hBurnDEV, char *burn_dir, char *file_path);


/*******************************************************************************
* ����  : Burn_Ctrl_DelBurnLocalFile
* ����  : ɾ�������ļ���¼�б��м�¼��ָ����¼�ļ�
* ����  : 
	hBurnDEV   : �����豸���
	file_path  : �����ļ���ȫ·��
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
*******************************************************************************/
int	Burn_Ctrl_DelBurnLocalFile(DEV_HANDLE hBurnDEV, char *file_path);


/*******************************************************************************
* ����  : Burn_File_Form_Local_File
* ����  : ��¼�����ļ� 
* ����  : 
	hBurnDEV   : �����豸���
	burn_dir   : Ŀ¼·������¼���Ŀ¼·���ᱻ�����ڹ��̸�Ŀ¼���ļ���ֱ�ӱ���¼
*                �ڴ�Ŀ¼�£��������ҪĿ¼�������NULL
	file_path  : �����ļ���ȫ·��
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
*******************************************************************************/
int Burn_File_Form_Local_File(DEV_HANDLE hBurnDEV, char *burn_dir, char *file_path);

/*******************************************************************************
* ����  : Burn_Dir_Form_Local_Dir
* ����  : ��¼����Ŀ¼
* ����  : 
	hBurnDEV   : �����豸���
	burn_dir   : Ŀ¼·������¼���Ŀ¼·���ᱻ�����ڹ��̸�Ŀ¼���ļ���ֱ�ӱ���¼
*                �ڴ�Ŀ¼�£��������ҪĿ¼�������NULL
	burn_path  : ����Ŀ¼��ȫ·��
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
*******************************************************************************/
int Burn_Dir_Form_Local_Dir(DEV_HANDLE hBurnDEV, char *burn_dir, char *burn_path);

/*******************************************************************************
* ����  : Burn_Ctrl_AddBurnLocalDir
* ����  : ��ӱ���·����¼�б�ͨ�����ô˺�������¼SDK�ɽ���ӵı���Ŀ¼�������ļ�
*         ֱ�ӿ�¼�ڹ�����
* ����  : 
	hBurnDEV   : �����豸���
	dir_name  : �ļ����Լ���¼ȫ·��
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
*******************************************************************************/
int	Burn_Ctrl_AddBurnLocalDir(DEV_HANDLE hBurnDEV, char *burn_dir, char *dir_path);


/*******************************************************************************
* ����  : Burn_Ctrl_DelBurnLocalDir
* ����  : ɾ������Ŀ¼��¼�б��м�¼��ָ����¼Ŀ¼
* ����  : 
	hBurnDEV   : �����豸���
	dir_name  : �ļ����Լ���¼ȫ·��
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
*******************************************************************************/
int	Burn_Ctrl_DelBurnLocalDir(DEV_HANDLE hBurnDEV, char *dir_path);

/*******************************************************************************
* ����  : Burn_Get_LocalFileBurnSchedule
* ����  : ��ȡָ���ı����ļ���¼���ȡ�
* ����  : 
	hBurnDEV   : �����豸���
	file_name  : �ļ����Լ���¼ȫ·��
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
*******************************************************************************/
int	Burn_Get_LocalFileBurnSchedule(DEV_HANDLE hBurnDEV, char *file_name);


/*******************************************************************************
* ����  : Burn_Get_LocalFileBurnSchedule
* ����  : ��ȡָ���ı����ļ���¼���ȡ�
* ����  : 
	hBurnDEV   : �����豸���
	file_name  : �ļ����Լ���¼ȫ·��
* ����ֵ: ����ֵ 0 -100, 100��ʾȫ�����.
*******************************************************************************/
int	Burn_Get_AllLocalFileBurnSchedule(DEV_HANDLE hBurnDEV);

//int Burn_Set_Data_Interface(DEV_HANDLE hBurnDEV, INTERFACE_TYPE interface);

//��ȡָ�������豸���е�������Ϣ
int Get_Burn_Info(DEV_HANDLE hBurnDEV, BURN_INFO_T *burn_info);

//��ȡ����Ŀǰ״̬-��λ/������/ֹͣ
BURN_STAT Get_Running_Burn_State(DEV_HANDLE hBurnDEV);

//��ȡ�������й����е�״̬-����/�쳣
RUNNING_STATE Get_Running_State(DEV_HANDLE hBurnDEV);

//��ȡ�������й����еľ���״̬��Ϣ
RUNNING_INFO Get_Running_info(DEV_HANDLE hBurnDEV);

//��ȡ����Ŀǰ�������ļ�״̬
int Get_Burn_File_State(DEV_HANDLE hBurnDEV, BURN_FILE_T  *burn_file_info);

int Create_Burn_Environment(DEV_HANDLE hBurnDEV);
int Burn_Ctrl_Dev_Get_Ready(DEV_HANDLE hBurnDEV);

int Start_Local_File_Burning(DEV_HANDLE hBurnDEV);
int Stop_Local_File_Burning(DEV_HANDLE hBurnDEV);

int Set_Local_File_Service_Mode(DEV_HANDLE hBurnDEV);

DEV_TRAY_STAT Burn_Ctrl_Dev_Get_TrayState(DEV_HANDLE hBurnDEV);
int Burn_Ctrl_Dev_Get_HaveDisc(DEV_HANDLE hBurnDEV);
int Burn_Ctrl_Dev_LoadDisc(DEV_HANDLE hBurnDEV);
int Burn_Ctrl_Dev_Get_DiscCanWrite(DEV_HANDLE hBurnDEV);
int Burn_Ctrl_Dev_FormatDisc(DEV_HANDLE hBurnDEV);
int Start_Burning_State_WatchDog(DEV_HANDLE hBurnDEV);
int Burn_Ctrl_LockDoor(DEV_HANDLE hBurnDEV, int bLocked);

DEV_TRAY_STAT Get_Running_Tray_State(DEV_HANDLE hBurnDEV);
char *Get_Burning_File_Name(DEV_HANDLE hBurnDEV);
char *Get_Completed_File_Name(DEV_HANDLE hBurnDEV);
int Burn_Set_Disk(DEV_HANDLE hBurnDEV, BURN_BOOL use_disk, char *backup_path, unsigned long backupsize, unsigned long alarmsize);
int Burn_Set_Disc(DEV_HANDLE hBurnDEV, int type, int maxspeed, unsigned long alarmsize, unsigned long alarmwarningsize);
int Set_Burn_Buffer_Size(DEV_HANDLE hBurnDEV, unsigned long buf_size);

int Burn_Specific_Dir_To_Disc(DEV_HANDLE hBurnDEV, char *dir_path);
int Set_Disk_Param(DEV_HANDLE hBurnDEV, BURN_BOOL use_dev, int backup_size, int alarm_size, char *disk_path);
int Disk_Create_BackUpFile(DEV_HANDLE hBurnDEV, const char *file_name);
int Disk_Close_BackUpFile(DEV_HANDLE hBurnDEV);
int Disk_Read_BackUpFile(DEV_HANDLE hBurnDEV, char *buf, int buf_size);
int Start_Disk_BackUp(DEV_HANDLE hBurnDEV);
int Stop_Disk_BackUp(DEV_HANDLE hBurnDEV);

int Fill_Data_To_Dev_Buf(DEV_HANDLE hBurnDEV, char *buf_ptr, int len, int block);
int Write_Dev_Buf_Data_To_File(DEV_HANDLE hBurnDEV, FILE_HANDLE hFile);
int Burn_Set_DiscName(DEV_HANDLE hBurnDEV, char *name);
void Burn_Set_Pause_Mode(BURN_BOOL val);
int Burn_Set_RT_File_Name(char *filename);
/*******************************************************************************
* ����  : Burn_Ctrl_SetRecordStatus
* ����  : ���ù�����¼״̬�����ڹ��̸�ʽ�����¼ʧ��δ���̣��ٴθ�ʽ��ǰ���ÿ�¼״̬��

* ����  :
	hBurnDEV   : �����豸���
* ����ֵ: 
* ����  : hexj
* ����  : 2014.8.5
*******************************************************************************/
void Burn_Ctrl_SetRecordStatus(DEV_HANDLE hBurnDEV, BOOL bRecordStatus);

#ifdef LINUX
#ifdef __cplusplus
}
#endif
#endif
#endif
