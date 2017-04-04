/*********************************************************************************
����:passion
ʱ��:2017-04-02
����:����UDF�ļ�ϵͳMPEG��������¼�ӿ�
**********************************************************************************/
#ifndef __UDF_FSCORE_H__
#define __UDF_FSCORE_H__

#include "LibDVDSDK.h"
#include "drvcomm.h"
#include "libudffs.h"

#define MAX_FILE_OR_DIR   256
#define	PAD(val, granularity)	(((val)+(granularity)-1)&~((granularity)-1))
#define MEM_BUFFER_SIZE		(1024 * 1024 * 3)	// 3M

//�ڵ�����
enum NODETYPE
{
   NODETYPE_FILE = 0 ,  //�ļ�
   NODETYPE_DIR  = 1    //Ŀ¼
};

//udf��¼ģʽ
enum UDFDATAMODE
{
	UDFDATAMODE_VIDEO = 0,  //��Ƶģʽ
	UDFDATAMODE_DATA  = 1,  //����ģʽ
};

//����״̬
enum DVDSTATUS 
{
   DVD_OPEN     = 0 ,   //��
   DVD_CLOSED   = 1 ,   //�ر�
   DVD_READ     = 2 ,   //������
   DVD_WRITE    = 3 ,   //д����
   DVD_RESUME1  = 4 ,	//ԭ�ָ̻�
   DVD_RESUME2  = 5 ,   //���ָ̻�
   DVD_COPY     = 6     //���̸���
};

typedef struct UDFINFO_T udfinfo_t;
typedef struct UDF_CMD_T udfcmd_t;


//�ļ�Ŀ¼�ڵ�
typedef struct tagFileDirNode
{	
    //�ڵ�����
	int NodeType;

	//�ڵ�ID
	int NodeID;
	
	//�ڵ�����
	char Name[256];

	//����,�ļ����Ȼ�Ŀ¼������
	uint64_t FileSize;

	//�ļ�λ��
	uint64_t FileLoca;

	//����ʱ��
	timestamp ts;

	//���ڵ�
	struct tagFileDirNode * Parent;

	//�ֵܽڵ�
	struct tagFileDirNode * Next;
	struct tagFileDirNode * Prior;

	//�ӽڵ�
	struct tagFileDirNode * Child;
	
}FILDIRNODE;


//�ļ�Ŀ¼��
typedef struct tagFileDirTree
{
	//�ڵ�����
	int NodeCount;

	//���ļ���С
	uint64_t TotalFileSize;

	//�׽ڵ�
	FILDIRNODE * FirstNode;

	//β���ڵ�
	FILDIRNODE * LastNode;	
	
}FILEDIRTREE;

//���̽ṹ	
typedef struct{
	// ��������,Ĭ��DVD-R,֧������DVD
	//uint8_t  discType;           

	// �������
	//int trackCount;
	
    // �Ƿ��Ѿ�����
	//int bClosed;     

	// ��������
	//int nDiskType;
	
	// ����������
	uint64_t nDiskCapicity;

	// ʣ������
	//int nDiskRemainCapicity;

	// β��AVDPλ�ã������������������512��λ�ã��˺�512�����ռ����ݱ���ȫ��д��AVDP��Ϊ��������
	//int nDVDFootAVDPAddr;

	// ������ʼ��ַ,Ҳ����FSD��ʼ��ַ
	//int nPDStratAddr;

	// ��������,Ĭ���Ǵ�FSD���������β��257λ��
	//int nPDLength;

	// �����Ѿ���¼������
	//int nPDRecorded;

	//uint8_t			fpacket;			/* fixed/variable packets */
	//uint8_t			packet_size;		/* fixed packet size */
	//uint8_t			link_size;			/* link loss size */
	//uint8_t			write_type;			/* mode1 or mode2 */
	//uint8_t			border;				/* border/session */
	//uint8_t			speed;				/* writing speed */
	
	char            ExtenData[CDROM_BLOCK * PACKET_BLOCK_16];      /* ��չ���� */   //OutUUIDString[32];				/* UUID */OutUUIDString[32];				/* UUID */
	int             ExtenDataLen;       /* ��չ���ݳ��� */
	
	// udfϵͳ���� (0   ~ 63����)
	// VRS = 16, PVDS = 32( IUVD = 33, PD = 34, LVD = 35 ,USD = 36 , TD = 37 ), RVDS = 48( IUVD = 49, PD = 50, LVD = 51 ,USD = 52 , TD = 53)
	CDR_TRACK_T udfsys;             
	
	// udf�ļ����� (288 ~ 351����)
	// (Ϊ�ļ������ļ�ʵ��������),FSD=288,FE_ROOT=289,FID_ROOT=290,FE_VIDEO_TS=291,FID_VIDEO_TS=292,FE_VIDEO_VMGIFO=293,FE_VIDEO_VTSIFO=294
	CDR_TRACK_T udffile;            

	struct udf_disc	udf_disc;
}CDRWDISKINFO;

typedef struct{
	char logicalVolIdent[128];
	char volIdent[32];
	char volSetIdent[128];
	char LVInfoTitle[36];
	char LVInfoDataTime[36];
	char LVInfoEmail[36];
	char fileSetIdent[32];
	char copyrightFileIdent[32];
	char abstractFileIdent[32];
}DISC_VOLID_T;

class CUdfCmd
{
public:
	CUdfCmd(){};
	~CUdfCmd(){};

public:
	// ���Ŀ¼���ļ�
	FILDIRNODE *(*addnode)(void *hMem, FILEDIRTREE *FileDirTree, FILDIRNODE *Parent, char *szName, uint64_t filesize, int NodeType);

	// ͨ�����Ʋ��ҽڵ�
	FILDIRNODE *(*findnodebyname)(FILEDIRTREE *FileDirTree, char *szName);

	// ͨ���ڵ�ID
	FILDIRNODE *(*findnodebyid)(FILEDIRTREE *FileDirTree, int nodeid);

	// �ж�Ŀ¼�������Ƿ��ظ�
	FILDIRNODE *(*GetNodeInDir)(FILDIRNODE *pDirNode, char *szName);

	// ��ȡĿ¼����
	int(*getdircount)(FILEDIRTREE *FileDirTree);

	// ��ȡ�ļ�����
	int(*getfilecount)(FILEDIRTREE *FileDirTree);

	// ��ʼ���ļ�ϵͳ,��ʼ������UDF�ļ�ϵͳ������
	int(*InitUdfFs)(udfinfo_t *pUdfInfo, DISC_VOLID_T *pDiscVol);

	// д�����ݵ������ļ�, length����Ϊ�����С
	int(*WriteStream)(udfinfo_t *pUdfInfo, FILDIRNODE *FileNode, uint8_t *Data, int length);

	// �������ݵ��������, length����Ϊ�����С
	int(*WriteEmptyStream)(udfinfo_t *pUdfInfo, uint8_t *Data, int length);

	// ���̹���
	int(*CloseDisc)(udfinfo_t *pUdfInfo);

	// ���ļ����д������ݣ����ڸ�ʽ��֮�����, ��ֹ����¼��ʼʱд�����ͣ�ͣ������Ƶ��֡
	int(*WriteFileTrackEmdpy)(udfinfo_t *pUdfInfo, uint32_t emptydize);

	// UDF�ļ�ϵͳ����
	int(*udffstest)(udfinfo_t *pUdfInfo);
};

// UDF�ļ�ϵͳ�ṹ
struct UDFINFO_T{
	// �豸���
	int fd;
	
	//����ģʽ
	int m_DataMode;

	// ��ǰ�ļ�λ��
	//uint32_t m_CurrentFileLocation;	
	
	void *m_hMem;
	uint8_t *m_pMemBuffer;

	// ��ʱʹ�õ�Buffer : 32 * 2048
	uint8_t	pPackBuff[PACKET32_SIZE];

	// ��������Buffer : 32 * 2048, ���ڴ��UUID����Ϣ
	//uint8_t	pUUIDBuff[PACKET32_SIZE];

	//������Ϣ�ṹ
	CDRWDISKINFO *m_CdRwDiskinfo;
	
	//�ļ�Ŀ¼��
	FILEDIRTREE *m_FileDirTree;
	
	// UDF cmd
	udfcmd_t *udf_cmd;
	
	// mmc cmd
	struct CDR_CMD_T *cdr_cmd;

	//
	CUdfCmd	udfCmd;
	//class CCdrcmd
	CCDRCmd cdrCmd;
};

//struct UDF_CMD_T{
//	// ���Ŀ¼���ļ�
//	FILDIRNODE *(*addnode)(void *hMem, FILEDIRTREE *FileDirTree, FILDIRNODE *Parent, char *szName, uint64_t filesize, int NodeType);
//
//	// ͨ�����Ʋ��ҽڵ�
//	FILDIRNODE *(*findnodebyname)(FILEDIRTREE *FileDirTree, char *szName);
//
//	// ͨ���ڵ�ID
//	FILDIRNODE *(*findnodebyid)(FILEDIRTREE *FileDirTree, int nodeid);
//
//	// �ж�Ŀ¼�������Ƿ��ظ�
//	FILDIRNODE *(*GetNodeInDir)(FILDIRNODE *pDirNode,  char *szName);
//
//	// ��ȡĿ¼����
//	int (*getdircount)(FILEDIRTREE *FileDirTree);
//
//	// ��ȡ�ļ�����
//	int (*getfilecount)(FILEDIRTREE *FileDirTree);
//
//	// ��ʼ���ļ�ϵͳ,��ʼ������UDF�ļ�ϵͳ������
//	int (*InitUdfFs)(udfinfo_t *pUdfInfo, DISC_VOLID_T *pDiscVol);
//
//	// д�����ݵ������ļ�, length����Ϊ�����С
//	int (*WriteStream)(udfinfo_t *pUdfInfo, FILDIRNODE *FileNode, uint8_t *Data, int length);
//
//	// �������ݵ��������, length����Ϊ�����С
//	int (*WriteEmptyStream)(udfinfo_t *pUdfInfo, uint8_t *Data, int length);
//
//	// ���̹���
//	int (*CloseDisc)(udfinfo_t *pUdfInfo);
//
//	// ���ļ����д������ݣ����ڸ�ʽ��֮�����, ��ֹ����¼��ʼʱд�����ͣ�ͣ������Ƶ��֡
//	int (*WriteFileTrackEmdpy)(udfinfo_t *pUdfInfo, uint32_t emptydize);
//
//	// UDF�ļ�ϵͳ����
//	int (*udffstest)(udfinfo_t *pUdfInfo);
//};

// ����UDF
udfinfo_t * DVDRec_UdfCreate(int fd, uint16_t DataMode);

// ����UDF
int DVDRec_UdfFree(udfinfo_t *pUdfInfo);

// �ͷ�UDF�ļ�ϵͳĿ¼
int DVDRec_UdfTreeFree(udfinfo_t *pUdfInfo);

#endif//__UDF_FSCORE_H__

