/*******************************************************************************
* ����    : LibDVDSDK.h 
* ��Ȩ����: xkd
* ��    ��: xkd
* �������: 
* �汾��: v2.0.0
* $Id: 
* SDK����˵��:

  * ������¼�����ӿ�
  * ֧����ͨDVD�������������
    1 ���֧��4������
    2 ֧��ʵʱ��¼
    3 ֧�ֹ��̵����̿���(���̸���)
    4 ֧���ļ������̿���
    5 ֧�ֹ��ָ̻�(ԭ�ָ̻������ָ̻�)
    6 ֧������д����ͣ���ָ�
    7 ֧��ʵʱ״̬��ѯ: ʣ��ռ�
    8 ֧��DVD+R,DVD-R,DVD-RW, BD-R, BD-RW��Ƭ
	9 ֧�ֱ�������(������ļ�ϵͳ�����������,��С:64K), ���ļ�ϵͳ�в��ɼ������ڴ�Ź������к�(UUID)����Ϣ
   10 ֧�ֹ���ʣ��ռ����(�ļ�ϵͳ�в��ɼ���ֻ�����ʣ��������)
	
  * �豸���˵��:
    ʹ�������Զ����, ��1�Ź���Ϊ /dev/sr0, 2�Ź���Ϊ /dev/sr1
  
  * �ӿ�ʹ������
   1: Xkd_DVDSDK_Load("/dev/sr0"); ���ع���
   2: Xkd_DVDSDK_GetDevInfo()      ��ȡ������Ϣ
    .......(�û���ʵʱ��¼���̡��ļ���������)

   3: Xkd_DVDSDK_UnLoad();	ж�ع���
   
  * �û�-ʵʱ��¼����
   0:  Xkd_DVDSDK_GetTrayState();           ��ȡ����״̬����������Ǵ򿪵���Ҫ�رգ���������3ִ��
   1:  Xkd_DVDSDK_Tray(FALSE)				�ر�����
   2:  Xkd_DVDSDK_HaveDisc()				�ж��Ƿ��й���
   3:  Xkd_DVDSDK_LoadDisc();               �й�������ع���
   4:  Xkd_DVDSDK_GetDiscInfo				��ѯ������Ϣ
   5:  Xkd_DVDSDK_SetWriteSpeed				���ù���д����
   6:  Xkd_DVDSDK_LockDoor					����
   7:  Xkd_DVDSDK_FormatDisc				��ʽ������
   8:  Xkd_DVDSDK_CreateDir					����Ŀ¼
   9:  Xkd_DVDSDK_CreateFile				�����ļ�
   10:  Xkd_DVDSDK_fillEmptyDataOnFirst		��ʼ������¼
   11:  Xkd_DVDSDK_SetFileLoca				�趨��¼�ļ�λ��
   12:  Xkd_DVDSDK_WriteData				��¼����
   13:    ......(��¼���̱�������д�����ݣ��������Ա仯)
   14:  Xkd_DVDSDK_CloseFile				�ر��ļ�
   15: Xkd_DVDSDK_GetUUID					���UUID
   16: Xkd_DVDSDK_SetRecvTrackData			д��UUID��������������
   17: Xkd_DVDSDK_fillAllDiscEmptyData		���ʣ����̿ռ䣬����ǰ����
   18: Xkd_DVDSDK_CloseDisc					���̣���պ����ٴ�д��
   19: Xkd_DVDSDK_Tray(TRUE)				��������
      
  * �û�-�ļ����Ƶ���������
   1:  Xkd_DVDSDK_Tray(FALSE)				�ر�����
   2:  Xkd_DVDSDK_HaveDisc()				�ж��Ƿ��й���
   3:  Xkd_DVDSDK_LoadDisc();               �й�������ع���
   4:  Xkd_DVDSDK_Tray(FALSE)				�ر�����
   5:  Xkd_DVDSDK_GetDiscInfo				��ѯ������Ϣ
   6:  Xkd_DVDSDK_GetTotalWriteSize			��ÿ�д��С���ж��Ƿ�С����Ҫд���ļ��Ĵ�С
   7:  Xkd_DVDSDK_SetWriteSpeed				���ù���д����
   8:  Xkd_DVDSDK_LockDoor					����
   9:  Xkd_DVDSDK_FormatDisc				��ʽ������
       while(n) {
	    Xkd_DVDSDK_CopyFile					�����ļ�
	   }
   10:  Xkd_DVDSDK_fillAllDiscEmptyData		���ʣ����̿ռ䣬����ǰ����
   11:  Xkd_DVDSDK_CloseDisc					���̣���պ����ٴ�д��
   12:  Xkd_DVDSDK_Tray(TRUE)				��������
   13:  Xkd_DVDSDK_LockDoor					����
   
* �޸�:
*******************************************************************************/

#ifndef __Xkd_DVDSDK_H__
#define __Xkd_DVDSDK_H__

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

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
}DVDDRIVER_TYPE;

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
}DVDDISC_TYPE;

//������
#define _ERRC(X)   (0x1000 + X)

//DVD_SDK���ش�����
#define ERROR_DVD_OK				0				//�ɹ�
#define ERROR_DVD_NODEV				_ERRC(300)		// �豸������
#define ERROR_DVD_ERRDEVNO			_ERRC(301)		// ������豸��
#define ERROR_DVD_BUSY				_ERRC(302)		// ������æ
#define ERROR_DVD_OPTERFAILED		_ERRC(303)		// ��������ʧ��
#define ERROR_DVD_NODISC			_ERRC(304)		// ������CD
#define ERROR_DVD_UNKNOWNDISCTYPE	_ERRC(305)		// δ֪��CD����(��֧�ָù�������)
#define ERROR_DVD_UNBLANKDISC		_ERRC(306)		// ���ǿհ���
#define ERROR_DVD_RESERVETRACKERR	_ERRC(307)		// �����������
#define ERROR_DVD_CDNOFILES			_ERRC(308)		// û���ҵ��ļ�
#define ERROR_DVD_FORMATFAILED		_ERRC(309)		// ��ʽ������ʧ��
#define ERROR_DVD_OPENFILEERROR		_ERRC(310)		// ���ļ�ʧ��
#define ERROR_DVD_WRITEERROR		_ERRC(311)		// д�̷�������
#define ERROR_DVD_DISCNOFREESIZE	_ERRC(312)		// �޿��ÿռ�
#define ERROR_DVD_LOADDISCFAILED	_ERRC(313)		// ����ʧ��
#define ERROR_DVD_NAMEEMPTY			_ERRC(314)		// ����Ϊ��
#define ERROR_DVD_NAMEEXIST			_ERRC(315)		// �����Ѿ�����
#define ERROR_DVD_DISCDIFFTYPE		_ERRC(316)		// �������Ͳ�ͬ
#define ERROR_DVD_CANTRESUMEBLANK	_ERRC(317)		// ����Ϊ����
#define ERROR_DVD_CANTRESUMEDISC    _ERRC(318)		// ���ָܻ��Ĺ���
#define ERROR_DVD_CANTCOPYDISC      _ERRC(319)		// ���ܹ���������ͬ���ܸ���
#define ERROR_DVD_SRCBLANKDISC		_ERRC(320)		// Դ��Ϊ����


typedef void *Xkd_DVDSDK_DIR;		// Ŀ¼�ڵ�ָ��
typedef void *Xkd_DVDSDK_FILE;		// �ļ�ָ��
typedef void *XKD_DVDDRV;			// DVD�������

// ������Ϣ
typedef struct{
	int 	 	 ntype;			// ��������	DISC_TYPE
	int			 maxpeed;		// ����ٶ�(д�ٶ�)
	unsigned int discsize;		// ��������(MB)
	unsigned int usedsize;		// ��ʹ�õĴ�С(MB)	
	unsigned int freesize;		// ���ô�С(MB)
}LVDVD_DISC_INFO_T;

// �����豸��Ϣ
typedef struct{
	char szVender[128];					// ��������
	int  drvtype;						// ��������			DVDDRIVER_TYPE
	int  discsupts;						// ����֧������
	unsigned char disclist[255];		// �����������б���DVDDISC_TYPE
}LVDVD_DEV_INFO_T;

#include <stdint.h>

/*******************************************************************************
* ����  : Xkd_DVDSDK_Load
* ����  : ���ع���,
* ����  : 
	szDevName : �������ƣ���: /dev/sr0, /dev/sr1
* ����ֵ: �豸�����NULLΪʧ��
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
XKD_DVDDRV Xkd_DVDSDK_Load(const char *szDevName);

/*******************************************************************************
* ����  : Xkd_DVDSDK_UnLoad
* ����  : ж�ع���
* ����  : 
	hDVD : Xkd_DVDSDK_Load�ķ���ֵ
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_UnLoad(XKD_DVDDRV hDVD);

/*******************************************************************************
* ����  : Xkd_DVDSDK_Tray
* ����  : ��/�ر�����
* ����  : 
	hDVD : Xkd_DVDSDK_Load�ķ���ֵ
	bOpen  : TRUE:������, FALSE:�ر�����
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int	Xkd_DVDSDK_Tray(XKD_DVDDRV hDVD, int bOpen);

/*******************************************************************************
* ����  : Xkd_DVDSDK_GetTrayState
* ����  : �������״̬(��/�ر�)
* ����  : 
	hDVD : Xkd_DVDSDK_Load�ķ���ֵ
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int	Xkd_DVDSDK_GetTrayState(XKD_DVDDRV hDVD);

/*******************************************************************************
* ����  : Xkd_DVDSDK_LockDoor
* ����  : ����/����������, ��ֹ�ڿ�¼�����������
* ����  : 
	hDVD : Xkd_DVDSDK_Load�ķ���ֵ
	bLocked: TRUE:����, FALSE:����
* ����ֵ: 0:�ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_LockDoor(XKD_DVDDRV hDVD, int bLocked);

/*******************************************************************************
* ����  : Xkd_DVDSDK_GetDevInfo
* ����  : ��ù�����Ϣ
* ����  : 
	hDVD : Xkd_DVDSDK_Load�ķ���ֵ
	pDevInfo   : ������Ϣָ��
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_GetDevInfo(XKD_DVDDRV hDVD, LVDVD_DEV_INFO_T *pDevInfo);

/*******************************************************************************
* ����  : Xkd_DVDSDK_GetDiscInfo
* ����  : ��õ�Ƭ��Ϣ
* ����  : 
	nDevNo     : �豸�ţ�0-n, -1Ϊȫ���豸
	pDiscInfo  : ��Ƭ��Ϣ�ṹָ��
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_GetDiscInfo(XKD_DVDDRV hDVD, LVDVD_DISC_INFO_T *pDiscInfo);

/*******************************************************************************
* ����  : Xkd_DVDSDK_HaveDisc
* ����  : �ж��Ƿ��й���
* ����  : 
	nDevNo     : �豸�ţ�0-n, -1Ϊȫ���豸
* ����ֵ: TRUE: �У�FALSE����
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_HaveDisc(XKD_DVDDRV hDVD);

/*******************************************************************************
* ����  : Xkd_DVDSDK_GetMediaExactType
* ����  : ��ȡ��������(��ȷ����)
* ����  : 
	nDevNo : �豸�ţ�0-n, -1Ϊȫ���豸
* ����ֵ: ��������
* ����  : xkd
* ����  : 2010.1.12
*******************************************************************************/
int Xkd_DVDSDK_GetMediaExactType(XKD_DVDDRV hDVD);

/*******************************************************************************
* ����  : Xkd_DVDSDK_GetMediaBasicType
* ����  : ��ȡ��������(��������)(�����趨�����ٶȺ͸��ƹ���ʱ)
* ����  : 
	nDevNo : �豸�ţ�0-n, -1Ϊȫ���豸
* ����ֵ: ��������
* ����  : xkd
* ����  : 2010.12.29
*******************************************************************************/
int Xkd_DVDSDK_GetMediaBasicType(XKD_DVDDRV hDVD);

/*******************************************************************************
* ����  : Xkd_DVDSDK_SetWriteSpeed
* ����  : �趨��¼�ٶ�
* ����  : 
	nDevNo : �豸�ţ�0-n, -1Ϊȫ���豸
	speed  : �ٶȣ������� 1 2 4 6 8 12
	disctpye  : ��������   	DVD_DISC = 0 ,DVD_DL_DISC = 1 ,CD_DISC = 2  
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
* �޸�  : 2010.12.30 Modify by xkd add disctype
*******************************************************************************/
int Xkd_DVDSDK_SetWriteSpeed(XKD_DVDDRV hDVD, int speed,int disctype);

/*******************************************************************************
* ����  : Xkd_DVDSDK_SetCopySpeed
* ����  : �趨���̸��Ƶ��ٶ�
* ����  : 
	nSrcDevno : Դ�豸�ţ�0-n
	nDstDevno : Ŀ���豸�ţ�0-n
	srctype  : ��������:B_DVD_DISC = 0,DVD_DL_DISC = 1,DVD_DISC = 2,CD_DISC = 3
	dsttype  : ��������:B_DVD_DISC = 0,DVD_DL_DISC = 1,DVD_DISC = 2,CD_DISC = 3
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.1.14
*******************************************************************************/
int Xkd_DVDSDK_SetCopySpeed(XKD_DVDDRV HDVDSrc, XKD_DVDDRV HDVDDst, int srctype, int dsttype);

/*******************************************************************************
* ����  : Xkd_DVDSDK_LoadDisc
* ����  : ���ع���
* ����  : 
	nDevNo : �豸�ţ�0-n
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_LoadDisc(XKD_DVDDRV hDVD);

/*******************************************************************************
* ����  : Xkd_DVDSDK_DiscCanWrite
* ����  : �жϹ����Ƿ��д
* ����  : 
	nDevNo : �豸�ţ�0-n
* ����ֵ: 0: ��д������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_DiscCanWrite(XKD_DVDDRV hDVD);

/*******************************************************************************
* ����  : Xkd_DVDSDK_FormatDisc
* ����  : ��ʽ������, �ֹ��, ����UDF�ļ�ϵͳ, ׼����ʼд����; Ŀǰֻ֧��һ��������2�����
* ����  : 
	nDevNo : �豸�ţ�0-n
	szDiscName: ��������
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int	Xkd_DVDSDK_FormatDisc(XKD_DVDDRV hDVD, char *szDiscName);

/*******************************************************************************
* ����  : Xkd_DVDSDK_SetFileLoca
* ����  : �趨�ļ�λ�� (д�ļ�ǰ����Ҫ�����������)
* ����  : 
	nDevNo : �豸�ţ�0-n
	FileNode : �ļ��ڵ�
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd 
* ����  : 2011.1.17
*******************************************************************************/
int Xkd_DVDSDK_SetFileLoca(XKD_DVDDRV hDVD, Xkd_DVDSDK_FILE FileNode);

/*******************************************************************************
* ����  : Xkd_DVDSDK_fillEmptyDataOnFirst
* ����  : �������ݣ��ڸ�ʽ��֮�󣬿�ʼ��¼֮ǰ���ã����⿪ʼ��¼ʱд������ݵ�ͣ�ͣ�
          �����Ƶ��֡  (��һ���ļ�ǰ���ã�֮����ļ�����Xkd_DVDSDK_SetFileLoca)
* ����  : 
	nDevNo : �豸�ţ�0-n
	fillsize: ����С��0Ϊ�Զ���������С
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_fillEmptyDataOnFirst(XKD_DVDDRV hDVD, unsigned int fillsize);

/*******************************************************************************
* ����  : Xkd_DVDSDK_CreateDir
* ����  : ����Ŀ¼
* ����  : 
	nDevNo : �豸�ţ�0-n
	szDirName : Ŀ¼���ƣ�����Ϊ��
* ����ֵ: Ŀ¼�ڵ�ָ��, Xkd_DVDSDK_CreateFile�������õ�, NULL:����Ŀ¼ʧ��
* ����  : xkd
* ����  : 2010.12.11
* �޸�  : 2010.12.21 xkd 
          ����:	nDevNo : �豸�ţ�0-n
				szDirName : Ҫ������Ŀ¼,��ʽΪ"/root/test1dir/test2dir"
*******************************************************************************/
Xkd_DVDSDK_DIR Xkd_DVDSDK_CreateDir(XKD_DVDDRV hDVD, char *szDirName);

/*******************************************************************************
* ����  : Xkd_DVDSDK_CreateFile
* ����  : �����ļ�����ʼд����
* ����  : 
	nDevNo : �豸�ţ�0-n
	pDir   : Ŀ¼�ڵ�ָ��, NULLΪ��Ŀ¼, Xkd_DVDSDK_CreateDir�ķ���ֵ
	szFileName: �ļ�����
	filesize: Ĭ��0
* ����ֵ: �ļ��ڵ�ָ�룬NULL������ʧ��
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
Xkd_DVDSDK_FILE Xkd_DVDSDK_CreateFile(XKD_DVDDRV hDVD, Xkd_DVDSDK_DIR pDir, char *szFileName, uint64_t filesize);

/*******************************************************************************
* ����  : Xkd_DVDSDK_WriteData
* ����  : ���ļ���д����, size = 32 * 1024
* ����  : 
	nDevNo : �豸�ţ�0-n
	pFile  : �ļ��ڵ�ָ�룬Xkd_DVDSDK_CreateFile�ķ���ֵ
	pBuffer: ����buffer
	size   : ���ݴ�С�������� 32*1024 ��������
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int	Xkd_DVDSDK_WriteData(XKD_DVDDRV hDVD, Xkd_DVDSDK_FILE pFile, unsigned char *pBuffer, int size);

/*******************************************************************************
* ����  : Xkd_DVDSDK_CloseFile
* ����  : �ر��ļ�
* ����  : 
	nDevNo : �豸�ţ�0-n
	pFile  : �ļ��ڵ�ָ�룬Xkd_DVDSDK_CreateFile�ķ���ֵ
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_CloseFile(XKD_DVDDRV hDVD, Xkd_DVDSDK_FILE pFile);

/*******************************************************************************
* ����  : Xkd_DVDSDK_fillAllDiscEmptyData
* ����  : ����������ʣ��ռ�(�ڿ�¼ֹ֮ͣ�󣬷���֮ǰ����)
* ����  : 
	nDevNo : �豸�ţ�0-n
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
* �޸�  : 20111.1.17 modify by xkd for FUNC
*******************************************************************************/
int Xkd_DVDSDK_fillAllDiscEmptyData(XKD_DVDDRV hDVD);

/*******************************************************************************
* ����  : Xkd_DVDSDK_CloseDisc
* ����  : ��տ�¼���; ���øýӿں󣬹��̽�����д;
* ����  : 
	nDevNo : �豸�ţ�0-n
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_CloseDisc(XKD_DVDDRV hDVD);

/*******************************************************************************
* ����  : Xkd_DVDSDK_CopyDisc
* ����  : ���̸���,ֱ�ӹ�����������Ǳ������������
* ����  : 
	nSrcDevno : Դ�豸�ţ�0-n
	nDstDevno : Ŀ���豸�ţ�0-n
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_CopyDisc(XKD_DVDDRV HDVDSrc, XKD_DVDDRV HDVDDst);

/*******************************************************************************
* ����  : Xkd_DVDSDK_ResumeDisc
* ����  : ���ָ̻�,����ڿ�¼�����жϵ磬���øú������ָ��ļ�ϵͳ
* ����  : 
	nDevNo : �豸�ţ�0-n
	DiscName : ��������
	DirName : Ŀ¼����
	FileName : �ļ�����
	FillSize : ����������
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
* �޸�  : 2010.12.30 modify by xkd for Complete Func
*******************************************************************************/
int Xkd_DVDSDK_ResumeDisc(XKD_DVDDRV hDVD, char *DiscName, char *DirName, char *FileName, int FillSize);

/*******************************************************************************
* ����  : Xkd_DVDSDK_GetReserveData
* ����  : ��ñ����������, �����Ѿ����̵Ĺ��̲���Ч, ��ȡʱ����
* ����  : 
	nDevNo : �豸�ţ�0-n
	pBuffer: ���ر�������ָ��
	pSize  : ����Buffer����
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_GetReserveData(XKD_DVDDRV hDVD, unsigned char **pBuffer, int *pSize);

/*******************************************************************************
* ����  : Xkd_DVDSDK_GetReserveBuffer
* ����  : ��ñ����������ָ��, ��������ڱ������ǰ���ã�ֱ���޸�Buffer����
		  ����ʱд�������,��С 32K�� ��¼ʱ����
* ����  : 
	nDevNo : �豸�ţ�0-n
	pBuffer: ���ر�������ָ��
	pSize  : ����Buffer����
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_GetReserveBuffer(XKD_DVDDRV hDVD, unsigned char **pBuffer, int *pSize);

/*******************************************************************************
* ����  : Xkd_DVDSDK_GetTotalWriteSize
* ����  : ��ù���������д�ռ�
* ����  : 
	nDevNo    : �豸�ţ�0-n
	pTotalSize: ����������д�ռ�
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_GetTotalWriteSize(XKD_DVDDRV hDVD, unsigned long long *pTotalSize);

/*******************************************************************************
* ����  : Xkd_DVDSDK_GetFreeWriteSize
* ����  : ���ʣ���д�ռ�
* ����  : 
	nDevNo   : �豸�ţ�0-n
	pFreeSize: ����ʣ���д�ռ�
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : xkd
* ����  : 2010.12.11
*******************************************************************************/
int Xkd_DVDSDK_GetFreeWriteSize(XKD_DVDDRV hDVD, unsigned long long *pFreeSize);

/*******************************************************************************
* ����  : Xkd_DVDSDK_PrintProfile
* ����  : ��ӡprofile
* ����  : 
	nDevNo   : �豸�ţ�0-n
* ����ֵ: 0: �ɹ�
* ����  : xkd
* ����  : 2011.1.14
*******************************************************************************/
int Xkd_DVDSDK_PrintProfile(XKD_DVDDRV hDVD);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif//__Xkd_DVDSDK_H__
