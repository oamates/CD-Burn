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

// ��¼����ģʽ
typedef enum{
	MEDIAFILE_BURN_STREAM   = 1,	
	MEDIAFILE_BURN_LOCAL_FILE,	
}BURN_MODE;

// ��¼�ļ�����
typedef enum{
	MEDIAFILE_TS   = 1,	
	MEDIAFILE_MP4,	
	MEDIAFILE_AVI,	
	MEDIAFILE_H264,	
}MEDIAFILE_TYPE;

// ��Э������
typedef enum{
	RTSP   = 1,	
	H323,	
    FINDANYWHERE,
    FTP,
}PROTOCL_TYPE;

/* ���������ò���*/
typedef struct{
	int          stream_id;	    // ������ID
    int          reconn_time;   // ��������ȡ������ʱ����
	PROTOCL_TYPE protocl_type;  // ��ȡ�������ķ���
	int          ts_channel;    // ������������TS����һ��ͨ��
	char		 url[1000] ;	// ����ԴURL��ַ
}STREAM_INFO_T;


//һ��TS���ֻ�ܻ�4·��
#define STREAM_INFO_MAX_NUM   4
/* ���������ò���*/
typedef struct{
    DEV_HANDLE      hBurnDEV;   // ��·���󶨵Ĺ������
    RTSPTS_HANDLE	hStream;    // RTSP���ṩ�ľ��
	STREAM_INFO_T  *stream_info[STREAM_INFO_MAX_NUM]; //��������Ϣ
    BURN_MODE       burn_mode;  // ��¼ģʽ(ʵʱ��¼/�º��¼)
	char            file_name[500]; //��¼���ļ�����
    int             ts_mark;    // TSͨ����¼
}BURN_STREAM_T;

#if 0
/* ���������״̬��Ϣ*/
typedef struct{
	int          stream_id;	    // ������ID
	int          ts_channel;    // ������������TS����һ��ͨ��
    int          get_stat;      // ��ȡ������������״̬
    int          proc_stat;     // �������ٴ��������״̬(����TS����)
	PROTOCL_TYPE protocl_type;  // ��ȡ�������ķ���
    DEV_HANDLE   hBurnDEV;      // ��·���󶨵Ĺ������
	char		 url[100] ;		// ����ԴURL��ַ
}STREAM_INFO_T;
#endif

/*******************************************************************************
* ����  : Download_File_To_Burn
* ����  : �����ļ���ֱ�ӽ��п�¼
* ����  : 
	file_info : ���ص��ļ�XML�б����嶨��μ�<<FileAnywhere����ӿ�Э��>>
	hBurnDEV : ����Ĺ����豸���
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Download_File_To_Burn(char* file_info, DEV_HANDLE hBurnDEV);

/*******************************************************************************
* ����  : Download_File
* ����  : �����ļ���ֱ�ӽ��п�¼
* ����  : 
	file_info : ���ص��ļ�XML�б����嶨��μ�<<FileAnywhere����ӿ�Э��>>
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Download_File(char* file_info);

/*******************************************************************************
* ����  : Get_Remote_File_Info
* ����  : �����ļ���ֱ�ӽ��п�¼
* ����  : 
	file_info : ���ص��ļ�XML�б����嶨��μ�<<FileAnywhere����ӿ�Э��>>
	hBurnDEV : ����Ĺ����豸���
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Get_Remote_File_Info(char* file_info);

/*******************************************************************************
* ����  : Add_Stream_To_Burn
* ����  : ��ָ���������������¼������
* ����  : 
	stream   : ��ӵ���������Ϣ 
	hBurnDEV : ����Ĺ����豸���
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Add_Stream_To_Burn(DEV_HANDLE hBurnDEV, STREAM_INFO_T *stream);


/*******************************************************************************
* ����  : Clear_Burn_Stream_List
* ����  : ��տ�¼���б�
* ����  : 
	hBurnDEV : ����Ĺ����豸���
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
*******************************************************************************/
int Clear_Burn_Stream_List(DEV_HANDLE hBurnDEV);

/*******************************************************************************
* ����  : Get_Stream_Info
* ����  : ��ȡ������״̬��Ϣ
* ����  : 
	stream_id : ������ID
* ����ֵ: ������״̬�ṹ�� 
*******************************************************************************/
STREAM_INFO_T Get_Stream_Info(int stream_id);


/*******************************************************************************
* ����  : Set_Stream_Proc_Type
* ����  : ������������¼ǰ�Ĵ�����
* ����  : 
	type : ������
* ����ֵ: 0: �ɹ�������Ϊ����ֵ 
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
