//LvDvdRecAPI.c
#include "debug.h"
#include "LibDVDSDK.h"
#include "drvcomm.h"
#include "udffscore.h"
#include <sys/resource.h>
#include "cdr_cmd.h"
#include "UdffsTag.h"
#include "memmalloc.h"
#include "uuid.h"

#define VALID_HANDLE()  if( !pDVD || pDVD->maskid != MASK_ID) return ERROR_DVD_ERRDEVNO;
#define VALID_DRIVER()  if(!pDVD->pUdf) 		return ERROR_DVD_NODEV
#define DVD_ISBUSY()	if(pDVD->isBusy) 		return ERROR_DVD_BUSY
#define DVD_INRECORD()  if(pDVD->bRecording) 	return ERROR_DVD_BUSY

#define HUDF_HANLE()  	pDVD->pUdf
#define HDEV_FD()		pDVD->fd
#define HCDR_CMD()		pDVD->cmd

#define HANDLE_DEF		HDVD_DEV_T *pDVD = (HDVD_DEV_T*)hDVD;

#define DISC_MINFREEBLOCKS	131072		// ʣ���(131072 * 2048 = 256M)
#define MASK_ID				0x12345678	// MASK ID

typedef struct{
	int  maskid;				// maskid
	int  fd;					// �豸���
	int	 isBusy;				// æ��־
	BOOL bRecording;			// ��¼��־
	DVD_DEV_INFO_T stDevInfo;	// �豸��Ϣ
	udfinfo_t  *pUdf;			// UDF �ļ�ϵͳ
	CCDRCmd cmd;				// ��������
}HDVD_DEV_T;

/*******************************************************************************
* ����  : DVDSDK_Load
* ����  : ���ع���,
* ����  :
	szDevName : �������ƣ���: /dev/sr0, /dev/sr1
* ����ֵ: �豸�����NULLΪʧ��
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
DVDDRV_HANDLE DVDSDKInterface::DVDSDK_Load(const char *szDevName)
{
	HDVD_DEV_T *pDVD = NULL;
	int fd;
	fd = open(szDevName, O_RDWR | O_NONBLOCK | O_SYNC);
	if(fd < 0)
		return NULL;
	pDVD = (HDVD_DEV_T*)malloc(sizeof(HDVD_DEV_T));
	if(!pDVD)
	{
		DPERROR(("malloc buffer failed!\n"));
		goto err_exit;
	}
	memset(pDVD, 0, sizeof(HDVD_DEV_T));
	pDVD->maskid = MASK_ID;
	pDVD->fd = fd;
	DVDRec_GetCdrcmd(NULL, &pDVD->cmd);
	pDVD->pUdf = DVDRec_UdfCreate(pDVD->fd, UDFDATAMODE_DATA);
	if(!pDVD->pUdf)
	{
		DPERROR(("crate udf error\n"));
		goto err_exit;
	}
	
	pDVD->pUdf->cdrCmd = pDVD->cmd;
	pDVD->cmd.CDR_UnlockDoor(pDVD->fd);
	return pDVD;

err_exit:
	if(pDVD)
	{
		if(pDVD->pUdf)
			DVDRec_UdfFree(pDVD->pUdf);
		free(pDVD);
	}
	if(fd > 0) close(fd);
	return NULL;
}

/*******************************************************************************
* ����  : DVDSDK_UnLoad
* ����  : ж�ع���
* ����  :
	hDVD : DVDSDK_Load�ķ���ֵ
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_UnLoad(DVDDRV_HANDLE hDVD)
{
	HANDLE_DEF;
	VALID_HANDLE();

	if(pDVD->pUdf)
	{
		//printf("DVDSDK_UnLoad 1\n");
		DVDRec_UdfFree(pDVD->pUdf);
		//printf("DVDSDK_UnLoad 2\n");
	}
	/*if(pDVD->pCmd && pDVD->fd > 0)
	{
		//printf("DVDSDK_UnLoad 3\n");
		pDVD->pCmd->cdr_closetray(pDVD->fd);
		//printf("DVDSDK_UnLoad 4\n");
	}*/


	if(pDVD->fd > 0)
		close(pDVD->fd);

	//printf("DVDSDK_UnLoad 5\n");
	free(pDVD);
	//printf("DVDSDK_UnLoad ok\n");
	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_Tray
* ����  : ��/�ر�����
* ����  :
	hDVD : DVDSDK_Load�ķ���ֵ
	bOpen  : TRUE:������, FALSE:�ر�����
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int	DVDSDKInterface::DVDSDK_Tray(DVDDRV_HANDLE hDVD, int bOpen)
{
	BOOL bRet;

	HANDLE_DEF;
	VALID_HANDLE();

	// ������æ����
	DVD_ISBUSY();

	if(bOpen)
	{
		DVD_INRECORD();
		bRet = HCDR_CMD().CDR_OpenTray(HDEV_FD());
	}
	else
	{
		bRet = HCDR_CMD().CDR_CloseTray(HDEV_FD());
	}
	return bRet ? 0 : ERROR_DVD_OPTERFAILED;
}

/*******************************************************************************
* ����  : DVDSDK_GetTrayState
* ����  : �������״̬(��/�ر�)
* ����  :
	hDVD : DVDSDK_Load�ķ���ֵ
* ����ֵ: 1���򿪣�0���رգ�����Ϊ�������
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int	DVDSDKInterface::DVDSDK_GetTrayState(DVDDRV_HANDLE hDVD)
{
	int iRet;

	HANDLE_DEF;
	VALID_HANDLE();

	iRet = HCDR_CMD().CDR_GetDoorState(HDEV_FD());
	return iRet;
}

/*******************************************************************************
* ����  : DVDSDK_LockDoor
* ����  : ����/����������, ��ֹ�ڿ�¼�����������
* ����  :
	hDVD : DVDSDK_Load�ķ���ֵ
	bLocked: TRUE:����, FALSE:����
* ����ֵ: 0:�ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_LockDoor(DVDDRV_HANDLE hDVD, int bLocked)
{
	BOOL bRet;
	HANDLE_DEF;
	VALID_HANDLE();
	// ������æ����
	DVD_ISBUSY();

	if(bLocked)
		bRet = HCDR_CMD().CDR_LockDoor(HDEV_FD());
	else
		bRet = HCDR_CMD().CDR_UnlockDoor(HDEV_FD());
	return bRet ? 0 : ERROR_DVD_OPTERFAILED;
}

/*******************************************************************************
* ����  : DVDSDK_GetDevInfo
* ����  : ��ù�����Ϣ
* ����  :
	hDVD : DVDSDK_Load�ķ���ֵ
	pDevInfo   : ������Ϣָ��
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_GetDevInfo(DVDDRV_HANDLE hDVD, DVD_DEV_INFO_T *pDevInfo)
{
	HANDLE_DEF;
	VALID_HANDLE();

	memset(&pDVD->stDevInfo, 0, sizeof(DVD_DEV_INFO_T));
	HCDR_CMD().CDR_GetdevInfo(HDEV_FD(), &pDVD->stDevInfo);
	memcpy(pDevInfo, &pDVD->stDevInfo, sizeof(DVD_DEV_INFO_T));
	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_GetDiscInfo
* ����  : ��õ�Ƭ��Ϣ
* ����  :
	nDevNo     : �豸�ţ�0-n, -1Ϊȫ���豸
	pDiscInfo  : ��Ƭ��Ϣ�ṹָ��
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_GetDiscInfo(DVDDRV_HANDLE hDVD, DVD_DISC_INFO_T *pDiscInfo)
{
	int TrickCount,SessionCount,DiskStatus,Speed;
	//unsigned long long Capicity;
	uint64_t Capicity;

	HANDLE_DEF;
	VALID_HANDLE();

	//��ʼ��
	Capicity 		= 0;
	TrickCount 		= 0;
	SessionCount 	= 0;
	DiskStatus 		= 0;
	Speed 			= 0;

	//��ù��������� ��λM
	HCDR_CMD().CDR_GetDiscInfo(HDEV_FD(), &TrickCount, &SessionCount, &Capicity, &DiskStatus);
	DP(("Capicity=%lld\n",Capicity));        //��������(2048)
	pDiscInfo->discsize = (Capicity/512);    //����512��ΪM��λ����

	//��ȡ����ʹ������ ��λM
	HCDR_CMD().CDR_GetDiscUsedSize(HDEV_FD(), (int*)&pDiscInfo->usedsize);
	//ʣ������
	pDiscInfo->freesize = pDiscInfo->discsize - pDiscInfo->usedsize;
    if ( pDiscInfo->freesize < 0 )
    {
        pDiscInfo->freesize = 0;
    }

	//��������
	HCDR_CMD().CDR_DiscExactType(HDEV_FD(), &pDiscInfo->ntype);

	//����ٶ�
	HCDR_CMD().CDR_GetMaxSpeed(HDEV_FD(), &Speed, &pDiscInfo->maxpeed);

	//��ӡ����
	DP(("ntype=%d,maxpeed=%d,discsize=%d,usedsize=%d,freesize=%d\n",pDiscInfo->ntype,
		pDiscInfo->maxpeed,pDiscInfo->discsize,pDiscInfo->usedsize,pDiscInfo->freesize));
	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_HaveDisc
* ����  : �ж��Ƿ��й���
* ����  :
	nDevNo     : �豸�ţ�0-n, -1Ϊȫ���豸
* ����ֵ: TRUE: �У�FALSE����
* ����  : passsion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_HaveDisc(DVDDRV_HANDLE hDVD)
{
	HANDLE_DEF;
	VALID_HANDLE();
	return HCDR_CMD().CDR_HaveDisc(HDEV_FD());
}

/*******************************************************************************
* ����  : DVDSDK_GetMediaExactType
* ����  : ��ȡ��������(��ȷ����)
* ����  :
	nDevNo : �豸�ţ�0-n, -1Ϊȫ���豸
* ����ֵ: ��������
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_GetMediaExactType(DVDDRV_HANDLE hDVD)
{
	int iExactT;

	HANDLE_DEF;
	VALID_HANDLE();

	iExactT = 0;
	HCDR_CMD().CDR_DiscExactType(HDEV_FD(), &iExactT);

	return iExactT;
}

/*******************************************************************************
* ����  : DVDSDK_GetMediaBasicType
* ����  : ��ȡ��������(��������)(�����趨�����ٶȺ͸��ƹ���ʱ)
* ����  :
	nDevNo : �豸�ţ�0-n, -1Ϊȫ���豸
* ����ֵ: ��������
* ����  : passion
* ����  : 2017.4.1
*******************************************************************************/
int DVDSDKInterface::DVDSDK_GetMediaBasicType(DVDDRV_HANDLE hDVD)
{
	int iBasicT,Ret = 0;

	HANDLE_DEF;
	VALID_HANDLE();

	iBasicT = -1;
	Ret = HCDR_CMD().CDR_DiscBasicType(HDEV_FD(), &iBasicT);
	if(( Ret > CD_DISC )||( Ret < B_DVD_DISC ))
		return Ret;
	return iBasicT;
}

/*******************************************************************************
* ����  : DVDSDK_SetWriteSpeed
* ����  : �趨��¼�ٶ�
* ����  :
	nDevNo : �豸�ţ�0-n, -1Ϊȫ���豸
	speed  : �ٶȣ������� 1 2 4 6 8 12
	disctpye  : ��������   	DVD_DISC = 0 ,DVD_DL_DISC = 1 ,CD_DISC = 2
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
* �޸�  : 2017.4.30 Modify by passion add disctype
*******************************************************************************/
int DVDSDKInterface::DVDSDK_SetWriteSpeed(DVDDRV_HANDLE hDVD, int speed, int disctype)
{
	int MaxReadSpeed,MaxWriteSpeed;
	int DiscT = -1;

	HANDLE_DEF;
	VALID_HANDLE();
/*
	if(speed <= 0)
		return ERROR_DVD_OPTERFAILED;

	if((disctype < B_DVD_DISC) || (disctype > CD_DISC))
		return ERROR_DVD_OPTERFAILED;
	MaxReadSpeed = MaxWriteSpeed = 2;   //Ĭ��Ϊ2����
	//��ȡ֧�ֵ�����ٶ�
	HCDR_CMD()->cdr_getmaxspeed(HDEV_FD(), &MaxReadSpeed, &MaxWriteSpeed);

	if((DVD_DISC == disctype)||(DVD_DL_DISC == disctype))
	{
		DiscT = DVD_SPEED_X1;
	}
	else if(B_DVD_DISC == disctype)
	{
		DiscT = BD_SPEED_X1;
	}
	else
		return ERROR_DVD_UNKNOWNDISCTYPE;

	if(speed > (MaxReadSpeed / DiscT))
		MaxReadSpeed = (MaxReadSpeed / DiscT);
	else
		MaxReadSpeed = speed;

	if(speed > (MaxWriteSpeed / DiscT))
		MaxWriteSpeed = (MaxWriteSpeed / DiscT);
	else
		MaxWriteSpeed = speed;
*/
	MaxWriteSpeed = 12;
	//�趨�ٶ�
	return HCDR_CMD().CDR_SetSpeed(HDEV_FD(), MaxReadSpeed, MaxWriteSpeed, disctype);

	//return 0;
}

/*******************************************************************************
* ����  : DVDSDK_SetCopySpeed
* ����  : �趨���̸��Ƶ��ٶ�
* ����  :
	nSrcDevno : Դ�豸�ţ�0-n
	nDstDevno : Ŀ���豸�ţ�0-n
	srctype  : ��������:B_DVD_DISC = 0,DVD_DL_DISC = 1,DVD_DISC = 2,CD_DISC = 3
	dsttype  : ��������:B_DVD_DISC = 0,DVD_DL_DISC = 1,DVD_DISC = 2,CD_DISC = 3
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.4.14
*******************************************************************************/
int DVDSDKInterface::DVDSDK_SetCopySpeed(DVDDRV_HANDLE HDVDSrc, DVDDRV_HANDLE HDVDDst, int srctype, int dsttype)
{

	int SrcMaxReadSpeed, SrcMaxWriteSpeed;
	int DstMaxReadSpeed, DstMaxWriteSpeed;
	int SrcTypeS = -1;
	int DstTypeS = -1;

	HDVD_DEV_T *pDVDSrc = (HDVD_DEV_T*)HDVDSrc;
	HDVD_DEV_T *pDVDDst = (HDVD_DEV_T*)HDVDDst;

	if(!pDVDSrc || pDVDSrc->maskid != MASK_ID)
		return ERROR_DVD_ERRDEVNO;
	if(!pDVDDst || pDVDDst->maskid != MASK_ID)
		return ERROR_DVD_ERRDEVNO;

	if((srctype < B_DVD_DISC) || (srctype > CD_DISC))
		return ERROR_DVD_OPTERFAILED;

	if((dsttype < B_DVD_DISC) || (dsttype > CD_DISC))
		return ERROR_DVD_OPTERFAILED;

	SrcMaxReadSpeed = SrcMaxWriteSpeed = 2;   //Ĭ��Ϊ2����
	DstMaxReadSpeed = DstMaxWriteSpeed = 2;   //Ĭ��Ϊ2����

	//��ȡ֧�ֵ�����ٶ�
	pDVDSrc->cmd.CDR_GetMaxSpeed(pDVDSrc->fd, &SrcMaxReadSpeed, &SrcMaxWriteSpeed);
	pDVDDst->cmd.CDR_GetMaxSpeed(pDVDDst->fd, &DstMaxReadSpeed, &DstMaxWriteSpeed);

	if( srctype == dsttype )
	{
		if(SrcMaxReadSpeed > DstMaxWriteSpeed)
			SrcMaxReadSpeed = DstMaxWriteSpeed;
		else
			DstMaxWriteSpeed = SrcMaxReadSpeed;
	}

	if((DVD_DISC == srctype)||(DVD_DL_DISC == srctype))
	{
		SrcTypeS = DVD_SPEED_X1;
	}
	else if(B_DVD_DISC == srctype)
	{
		SrcTypeS = BD_SPEED_X1;
	}
	else
		return ERROR_DVD_UNKNOWNDISCTYPE;

	if( srctype != dsttype )
	{
		if((DVD_DISC == dsttype)||(DVD_DL_DISC == dsttype))
		{
			DstTypeS = DVD_SPEED_X1;
		}
		else if(B_DVD_DISC == dsttype)
		{
			DstTypeS = BD_SPEED_X1;
		}
		else
			return ERROR_DVD_UNKNOWNDISCTYPE;
	}
	else
		DstTypeS = SrcTypeS;

	//�趨Դ���ٶ�
	SrcMaxReadSpeed = (SrcMaxReadSpeed/SrcTypeS);

	SrcMaxWriteSpeed = (SrcMaxWriteSpeed/SrcTypeS);

	//�趨Ŀ�����ٶ�
	DstMaxReadSpeed = (DstMaxReadSpeed/DstTypeS);

	DstMaxWriteSpeed = (DstMaxWriteSpeed/DstTypeS);

	//�趨�ٶ�
	pDVDSrc->cmd.CDR_SetSpeed(pDVDSrc->fd, SrcMaxReadSpeed, SrcMaxWriteSpeed, srctype);
	pDVDDst->cmd.CDR_SetSpeed(pDVDDst->fd, DstMaxReadSpeed, DstMaxWriteSpeed, dsttype);

	return 0;
}

static int set_core_dump(int enable, int size)
{
    struct rlimit lim;
    struct rlimit newlim;
    if (enable)
    {
        if (getrlimit(RLIMIT_CORE, &lim)<0)
        {
            fprintf(stderr, "cannot get the maximum core size: %s\n",strerror(errno));
            goto ERROR;
        }
        if (lim.rlim_cur<size)
        {
            /* first try max limits */
            newlim.rlim_max=RLIM_INFINITY;
            newlim.rlim_cur=newlim.rlim_max;
            if (setrlimit(RLIMIT_CORE, &newlim)==0) goto DONE;
            /* now try with size */
            if (lim.rlim_max<size)
            {
                newlim.rlim_max=size;
            }
            newlim.rlim_cur=newlim.rlim_max;
            if (setrlimit(RLIMIT_CORE, &newlim)==0)
                goto DONE;
            /* if this failed too, try rlim_max, better than nothing */
            newlim.rlim_max=lim.rlim_max;
            newlim.rlim_cur=newlim.rlim_max;
            if (setrlimit(RLIMIT_CORE, &newlim)<0)
            {
                fprintf(stderr, "could increase core limits at all: %s\n",strerror (errno));
            }
            else
            {
                fprintf(stderr, "core limits increased only to %lu\n",(unsigned long)lim.rlim_max);
            }
            goto ERROR; /* it's an error we haven't got the size we wanted*/
        }
        goto DONE; /*nothing to do */
    }
    else
    {
        /* disable */
        newlim.rlim_cur=0;
        newlim.rlim_max=0;
        if (setrlimit(RLIMIT_CORE, &newlim)<0)
        {
            fprintf(stderr, "failed to disable core dumps: %s\n",strerror(errno));
            goto ERROR;
        }
    }
DONE:
    fprintf(stderr,"core dump limits set to %lu\n", (unsigned long)newlim.rlim_cur);
    return 0;
ERROR:
    return -1;
}

/*******************************************************************************
* ����  : DVDSDK_LoadDisc
* ����  : ���ع���
* ����  :
	nDevNo : �豸�ţ�0-n
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_LoadDisc(DVDDRV_HANDLE hDVD)
{
	int iRet;

	HANDLE_DEF;
	VALID_HANDLE();

	// ������æ����
	DVD_ISBUSY();

#ifdef DEBUG
	set_core_dump(1, 256*1024*1024);
#endif

	iRet = HCDR_CMD().CDR_LoadMedia(HDEV_FD());

	return iRet;
}

/*******************************************************************************
* ����  : DVDSDK_DiscCanWrite
* ����  : �жϹ����Ƿ��д
* ����  :
	nDevNo : �豸�ţ�0-n
* ����ֵ: 0: ��д������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_DiscCanWrite(DVDDRV_HANDLE hDVD)
{
	int iRet;
	uint64_t Capicity;
	int TrickCount, SessionCount, DiskStatus;

	HANDLE_DEF;
	VALID_HANDLE();
	// ������æ����
	iRet = HCDR_CMD().CDR_LoadMedia(HDEV_FD());
	if(iRet)
		return iRet;

	if ((iRet = HCDR_CMD().CDR_GetDiscInfo(HDEV_FD(), &TrickCount, &SessionCount, &Capicity, &DiskStatus)))
	{
		DPERROR(("get disc info error:%d\n", iRet));
		return iRet;
	}
	if(DiskStatus != DISCSTE_BLANK)
	{
		return ERROR_DVD_UNBLANKDISC;
	}
	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_FormatDisc
* ����  : ��ʽ������, �ֹ��, ����UDF�ļ�ϵͳ, ׼����ʼд����; Ŀǰֻ֧��һ��������2�����
* ����  :
	nDevNo : �豸�ţ�0-n
	szDiscName: ��������
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int	DVDSDKInterface::DVDSDK_FormatDisc(DVDDRV_HANDLE hDVD, char *szDiscName)
{
	int iRet;
	uint64_t Capicity;
	DISC_VOLID_T disc_volid;
	//��ȡ������Ϣ��״̬
	int TrickCount, SessionCount, DiskStatus;

	HANDLE_DEF;
	VALID_HANDLE();

	// ������æ����
	DVD_ISBUSY();
	DVD_INRECORD();

	memset(&disc_volid, 0, sizeof(DISC_VOLID_T));
	strcpy(disc_volid.logicalVolIdent, szDiscName);
	strcpy(disc_volid.volIdent,        szDiscName);
	strcpy(disc_volid.volSetIdent,     szDiscName);
	strcpy(disc_volid.LVInfoTitle, 	      "avs_DVD_1.0");
	strcpy(disc_volid.LVInfoDataTime,     "2017-4-17");
	strcpy(disc_volid.LVInfoEmail,        "avs_work");
	strcpy(disc_volid.fileSetIdent,       "avs_DVD_1.0");
	strcpy(disc_volid.copyrightFileIdent, "avs_DVD_1.0");
	strcpy(disc_volid.abstractFileIdent,  "avs_DVD_1.0");

	iRet = HCDR_CMD().CDR_LoadMedia(HDEV_FD());
	if(iRet)
	{
		// û�й���
		return ERROR_DVD_NODISC;
	}

	if ((iRet = HCDR_CMD().CDR_GetDiscInfo(HDEV_FD(), &TrickCount, &SessionCount, &Capicity, &DiskStatus)))
	{
		DPERROR(("get disc info error:%d\n", iRet));
		return iRet;
	}
	DP(("diskBlocks = %lld, sessioncount:%d, trackcount:%d\n",Capicity, SessionCount, TrickCount));

	if(DiskStatus != DISCSTE_BLANK)
	{
		DPERROR(("Disc is not blank!\n"));
		return ERROR_DVD_UNBLANKDISC;
	}
	//HCDR_CMD()->cdr_lockdoor(HDEV_FD());

	DVDRec_UdfTreeFree(HUDF_HANLE());

	MEMMOCLINE;
	//������Ŀ¼
	HUDF_HANLE()->udfCmd.AddNode(HUDF_HANLE()->m_hMem, HUDF_HANLE()->m_FileDirTree, NULL, "", 0, NODETYPE_DIR);

	sleep(1);

	if( TrickCount == 1 )
	{
		DP(("start to format disc[%d]\n", pDVD->fd));
		iRet = HCDR_CMD().CDR_FormatDisc(HDEV_FD(), UDF_FS_LENGTH);
		if(iRet)
		{
			DPERROR(("format disc error:%d\n", iRet));
			HCDR_CMD().CDR_UnlockDoor(HDEV_FD());
			return ERROR_DVD_FORMATFAILED;
		}
	}

	HUDF_HANLE()->m_CdRwDiskinfo->nDiskCapicity = Capicity;

	iRet = HUDF_HANLE()->udfCmd.InitUdfFs(HUDF_HANLE(), &disc_volid);
	if(iRet)
	{
		DPERROR(("init udf fs error:%d\n", iRet));
		HCDR_CMD().CDR_UnlockDoor(HDEV_FD());
		return ERROR_DVD_FORMATFAILED;
	}

	//DP(("trackID1=%d\n", HUDF_HANLE()->m_CdRwDiskinfo->udfsys.trackid));
	//DP(("trackID2=%d\n", HUDF_HANLE()->m_CdRwDiskinfo->udffile.trackid));

	HCDR_CMD().CDR_GetTrackinfo(HDEV_FD(),
		HUDF_HANLE()->m_CdRwDiskinfo->udfsys.trackid,  &(HUDF_HANLE()->m_CdRwDiskinfo->udfsys));

	HCDR_CMD().CDR_GetTrackinfo(HDEV_FD(), 
		HUDF_HANLE()->m_CdRwDiskinfo->udffile.trackid, &(HUDF_HANLE()->m_CdRwDiskinfo->udffile));

	pDVD->bRecording = TRUE;

	DP(("format disc[%d] ...........complete ok\n", pDVD->fd));
	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_SetFileLoca
* ����  : �趨�ļ�λ�� (д�ļ�ǰ����Ҫ�����������)
* ����  :
	nDevNo : �豸�ţ�0-n
	FileNode : �ļ��ڵ�
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.4.17
*******************************************************************************/
int DVDSDKInterface::DVDSDK_SetFileLoca(DVDDRV_HANDLE hDVD, DVDSDK_FILE FileNode)
{
	HANDLE_DEF;
	VALID_HANDLE();

	((FILDIRNODE *)(FileNode))->FileLoca = HUDF_HANLE()->m_CdRwDiskinfo->udffile.writenext - UDF_SYS_LEN;
    DP(("+++++++++[DVDSDK_SetFileLoca] File name = %s,File  location = %ld++++++++++\n",((FILDIRNODE *)(FileNode))->Name,((FILDIRNODE *)(FileNode))->FileLoca));

	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_FillEmptyDataOnFirst
* ����  : �������ݣ��ڸ�ʽ��֮�󣬿�ʼ��¼֮ǰ���ã����⿪ʼ��¼ʱд������ݵ�ͣ�ͣ�
          �����Ƶ��֡  (��һ���ļ�ǰ���ã�֮����ļ�����DVDSDK_SetFileLoca)
* ����  :
	nDevNo : �豸�ţ�0-n
	fullsize: ����С��0Ϊ�Զ���������С
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_FillEmptyDataOnFirst(DVDDRV_HANDLE hDVD, unsigned int fullsize)
{
	unsigned int nEmptySize;

	HANDLE_DEF;
	VALID_HANDLE();

	// ������æ����
	DVD_ISBUSY();

	nEmptySize = fullsize ? fullsize : (1024 * 1024 * 64);
	nEmptySize &= ~(PACKET32_SIZE - 1);	// 64K����

	HUDF_HANLE()->udfCmd.WriteFileTrackEmdpy(HUDF_HANLE(), nEmptySize);

	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_Analysisdirectory
* ����  : ���Ŀ¼ʱ��Ŀ¼�ַ����н���
* ����  :
	szDirName : Ŀ¼��
	len : Ŀ¼����
	szDirRemain: ����ʣ����ַ�������:szDirNameΪ"/test/test1/test2",
	                ����ֵΪ:"/test1/test2"
    szRemainName:�������ַ�
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.4.21
* ˵��  : Ŀ¼��һ���ַ�����Ϊ"/"
*******************************************************************************/
static int DVDSDK_Analysisdirectory(char * szDirName, int len, char ** szDirRemain, char * szAnalysisName)
{
	int i,j,start;
	char tmpname[128];

	memset(tmpname,0,128);
	j = 0;
	start = 0;
	if(len == 0)
		return -1;
	for(i = 0;i < len;i++)
	{
		if((szDirName[i] == '/')&&(i == 0))
			start = 1;
		else if((szDirName[i] == '/')&&(i != 0))
		{
			start = 0;
			break;
		}
		if(start == 1)
			tmpname[j++] = szDirName[i+1];
		else
		{
			if(i == 0)
			{
				DPERROR(("Dir name:%s error\n", szDirName));
				return -1;
			}
		}
	}
	tmpname[i-1] = 0;
	*szDirRemain = szDirName+i;
	if( szAnalysisName )
	{
		memcpy(szAnalysisName,tmpname,j);
	}
	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_CreateDir
* ����  : ����Ŀ¼
* ����  :
	nDevNo : �豸�ţ�0-n
	pParentDir: ��Ŀ¼��Ϊ��Ϊ��Ŀ¼�´���
	szDirName : Ŀ¼���ƣ�����Ϊ��
* ����ֵ: Ŀ¼�ڵ�ָ��, DVDSDK_CreateFile�������õ�, NULL:����Ŀ¼ʧ��
* ����  : passion
* ����  : 2017.3.31
* �޸�  : 2017.4.21 passion
          ����:	nDevNo : �豸�ţ�0-n
				szDirName : Ҫ������Ŀ¼,��ʽΪ"/test/test1/test2"
*******************************************************************************/
DVDSDK_DIR DVDSDKInterface::DVDSDK_CreateDir(DVDDRV_HANDLE hDVD, char *szDirName)
{
	FILDIRNODE *DirNode;
	FILDIRNODE *tmpNode;
	int iRet;
	char *szDirRemain;
	char szAnalysisName[128];

	HDVD_DEV_T *pDVD = (HDVD_DEV_T*)hDVD;
	if( !pDVD || pDVD->maskid != MASK_ID) return NULL;

	if(NULL == szDirName || !strlen(szDirName))
		return NULL;

	szDirRemain = szDirName;
	DirNode = NULL;
	iRet    = 0;
	while((iRet == 0))
	{
		iRet = DVDSDK_Analysisdirectory(szDirRemain, strlen(szDirRemain), &szDirRemain, szAnalysisName);

		if(iRet == -1)
			break;
		if(DirNode == NULL)
			DirNode = HUDF_HANLE()->udfCmd.FindNodeByID(HUDF_HANLE()->m_FileDirTree, 0);

		if (DirNode == NULL)
		{
			DirNode = HUDF_HANLE()->udfCmd.AddNode(HUDF_HANLE()->m_hMem, HUDF_HANLE()->m_FileDirTree,
									NULL, "", 0, NODETYPE_DIR);//û��Ŀ¼ �򴴽���Ŀ¼
		}
		tmpNode = HUDF_HANLE()->udfCmd.GetNodeInDir(DirNode, szAnalysisName);
		if(tmpNode)
		{
			DP(("Dir name:%s exist\n", szDirName));
			DirNode = tmpNode;
			continue;//return NULL;
		}
		MEMMOCLINE;
		DirNode = HUDF_HANLE()->udfCmd.AddNode(HUDF_HANLE()->m_hMem, HUDF_HANLE()->m_FileDirTree,
						(FILDIRNODE*)DirNode, szAnalysisName, 0, NODETYPE_DIR);
	}
	return DirNode;
}

/*******************************************************************************
* ����  : DVDSDK_CreateFile
* ����  : �����ļ�����ʼд����
* ����  :
	nDevNo : �豸�ţ�0-n
	pDir   : Ŀ¼�ڵ�ָ��, NULLΪ��Ŀ¼, DVDSDK_CreateDir�ķ���ֵ
	szFileName: �ļ�����
* ����ֵ: �ļ��ڵ�ָ�룬NULL������ʧ��
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
DVDSDK_FILE DVDSDKInterface::DVDSDK_CreateFile(DVDDRV_HANDLE hDVD, DVDSDK_DIR pDir, char *szFileName, uint64_t filesize)
{
	FILDIRNODE *DirNode;
	FILDIRNODE *pParent;

	HDVD_DEV_T *pDVD = (HDVD_DEV_T*)hDVD;
	if( !pDVD || pDVD->maskid != MASK_ID) return NULL;

	if(NULL == szFileName || !strlen(szFileName))
		return NULL;

	if(pDir)
		pParent = (FILDIRNODE*)pDir;
	else
		pParent = HUDF_HANLE()->udfCmd.FindNodeByID(HUDF_HANLE()->m_FileDirTree, 0);

	MEMMOCLINE;
	DirNode = HUDF_HANLE()->udfCmd.AddNode(HUDF_HANLE()->m_hMem, HUDF_HANLE()->m_FileDirTree,
				pParent, szFileName, filesize, NODETYPE_FILE);
	if(!DirNode)
	{
		DPERROR(("create file failed!\n"));
	}
	return DirNode;
}

/*******************************************************************************
* ����  : DVDSDK_WriteData
* ����  : ���ļ���д����, size = 32 * 1024
* ����  :
	nDevNo : �豸�ţ�0-n
	pFile  : �ļ��ڵ�ָ�룬DVDSDK_CreateFile�ķ���ֵ
	pBuffer: ����buffer
	size   : ���ݴ�С�������� 32*1024 ��������
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int	DVDSDKInterface::DVDSDK_WriteData(DVDDRV_HANDLE hDVD, DVDSDK_FILE pFile, unsigned char *pBuffer, int size)
{
	int iRet;
	HANDLE_DEF;
	VALID_HANDLE();

	if(!pDVD->bRecording)
	{
		DPERROR(("write data before format disc,fd=%d\n",HDEV_FD()));
		return ERROR_DVD_WRITEERROR;
	}
	if(HUDF_HANLE()->m_CdRwDiskinfo->udffile.freeblocks <= DISC_MINFREEBLOCKS)
	{
		DPERROR(("disc not free block %d,fd=%d\n", HUDF_HANLE()->m_CdRwDiskinfo->udffile.freeblocks,HDEV_FD()));
		return ERROR_DVD_DISCNOFREESIZE;
	}
	// �趨�ļ�λ��
	//HUDF_HANLE(nDevNo)->m_CurrentFileLocation = HUDF_HANLE(nDevNo)->m_CdRwDiskinfo->udffile.writestart - UDF_SYS_LEN;

	iRet = HUDF_HANLE()->udfCmd.WriteStream(HUDF_HANLE(), static_cast<FILDIRNODE*>(pFile), pBuffer, size);
	if(iRet)
	{
		DPERROR(("WriteStream error:%d,fd=%d\n", iRet,HDEV_FD()));
		return ERROR_DVD_WRITEERROR;
	}
	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_CloseFile
* ����  : �ر��ļ�
* ����  :
	nDevNo : �豸�ţ�0-n
	pFile  : �ļ��ڵ�ָ�룬DVDSDK_CreateFile�ķ���ֵ
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_CloseFile(DVDDRV_HANDLE hDVD, DVDSDK_FILE pFile)
{
	HANDLE_DEF;
	VALID_HANDLE();

	// Flush�������ݵ��ļ����
	HCDR_CMD().CDR_FlushTrack(HDEV_FD(), &HUDF_HANLE()->m_CdRwDiskinfo->udffile);

	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_FillAllDiscEmptyData
* ����  : ����������ʣ��ռ�(�ڿ�¼ֹ֮ͣ�󣬷���֮ǰ����)
* ����  :
	nDevNo : �豸�ţ�0-n
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
* �޸�  : 2017.4.17 modify by passion for FUNC
*******************************************************************************/
int DVDSDKInterface::DVDSDK_FillAllDiscEmptyData(DVDDRV_HANDLE hDVD)
{
	char FillBuf[PACKET32_SIZE];
	int FreeSize,MinSize;
	int DiscType;

	HANDLE_DEF;
	VALID_HANDLE();

	memset(FillBuf,0,PACKET32_SIZE);
	if(!pDVD->bRecording)
	{
		DPERROR(("write data before format disc\n"));
		return ERROR_DVD_WRITEERROR;
	}

	DiscType = DVDSDK_GetMediaBasicType(hDVD);
	if(( DiscType > CD_DISC )||( DiscType < B_DVD_DISC ))
		return DiscType; //���ǹ������;ͷ��������ֵ

	//�趨����ٶ� (һ��DVD����ٶ�16����ǰ��С��16���򰴵�ǰ�̵�������趨)
	DVDSDK_SetWriteSpeed(hDVD, 16, DiscType);

	//��λΪM
	MinSize = ((HUDF_HANLE()->m_CdRwDiskinfo->udffile.tracksize / 512)/100) * 5;//ʣ������ �����������5/100������
	while(1)
	{
		FreeSize = (HUDF_HANLE()->m_CdRwDiskinfo->udffile.freeblocks / 512);
		//DP(("FreeSize=%d,MinSize=%d\n",FreeSize,MinSize));
		if(FreeSize < MinSize)
			break;
		HUDF_HANLE()->udfCmd.WriteEmptyStream(HUDF_HANLE(), (uint8_t*)FillBuf, PACKET32_SIZE);
	}

	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_CloseDisc
* ����  : д��UDF�ļ�ϵͳ���ͷ�UDF�ļ�ϵͳ, �رչ��; ���øýӿں󣬹��̽�����д;
* ����  :
	nDevNo : �豸�ţ�0-n
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_CloseDisc(DVDDRV_HANDLE hDVD)
{
	int iRet;
	HANDLE_DEF;
	VALID_HANDLE();

	if(!pDVD->bRecording)
	{
		return 0;
	}

	pDVD->bRecording = FALSE;

	//д��udf�ļ�ϵͳ
	iRet = HUDF_HANLE()->udfCmd.CloseDisc(HUDF_HANLE());
	if(iRet)
	{
		DPERROR(("close disc error:%d\n", iRet));
		DVDRec_UdfTreeFree(HUDF_HANLE());
		HCDR_CMD().CDR_UnlockDoor(HDEV_FD());
		return iRet;
	}
    else
    {
        DP(("udf_cmd->CloseDisc success! fd = %d \r\n", HDEV_FD()));
    }

	iRet = HCDR_CMD().CDR_CloseTrack(HDEV_FD(), &HUDF_HANLE()->m_CdRwDiskinfo->udffile);
	if(iRet)
	{
		DPERROR(("close file track error:%d, fd=%d\n", iRet,HDEV_FD()));
	}

	iRet = HCDR_CMD().CDR_CloseSession(HDEV_FD(), &HUDF_HANLE()->m_CdRwDiskinfo->udffile);
	if(iRet)
	{
		DPERROR(("close file session error:%d, fd=%d\n", iRet,HDEV_FD()));
	}

	// �ͷ�UDFĿ¼��
	DVDRec_UdfTreeFree(HUDF_HANLE());
	HCDR_CMD().CDR_UnlockDoor(HDEV_FD());

	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_CopyDisc
* ����  : ���̸���,ֱ�ӹ�����������Ǳ������������
* ����  :
	nSrcDevno : Դ�豸�ţ�0-n
	nDstDevno : Ŀ���豸�ţ�0-n
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_CopyDisc(DVDDRV_HANDLE HDVDSrc, DVDDRV_HANDLE HDVDDst)
{
	int iRet,idisc1,idisc2;
	int i = 0;
    //unsigned char TempUUID[8];
	uint64_t Capicity;
	stCDRTrack TrackInfo;
	//char Buffer[32];
	//char TempStr[3];
	int TrickCount, SessionCount, DiskStatus;
	DVD_DISC_INFO_T SrcDiscInfo,DstDiscInfo;
	//struct partitionDesc pd;
	uint8_t buf[PACKET32_SIZE];
	HDVD_DEV_T *pDVDSrc = (HDVD_DEV_T*)HDVDSrc;
	HDVD_DEV_T *pDVDDst = (HDVD_DEV_T*)HDVDDst;

	if(!pDVDSrc || pDVDSrc->maskid != MASK_ID)
		return ERROR_DVD_ERRDEVNO;
	if(!pDVDDst || pDVDDst->maskid != MASK_ID)
		return ERROR_DVD_ERRDEVNO;
	if(pDVDSrc->bRecording || pDVDDst->bRecording)
		return ERROR_DVD_BUSY;

	memset(buf,0,PACKET32_SIZE);
	memset(&TrackInfo,0,sizeof(stCDRTrack));

	//�жϹ��������Ƿ���ͬ
	idisc1 = DVDSDK_GetMediaBasicType(pDVDSrc);
	idisc2 = DVDSDK_GetMediaBasicType(pDVDDst);

	if(( idisc1 > CD_DISC )||( idisc1 < B_DVD_DISC ))
	{
		DPERROR(("srcdisc type unsupport or no disc\n"));
		return idisc1; //���ǹ������;ͷ��������ֵ
	}

	if(( idisc2 > CD_DISC )||( idisc2 < B_DVD_DISC ))
	{
		DPERROR(("dstdisc type unsupport or no disc\n"));
		return idisc2; //���ǹ������;ͷ��������ֵ
	}

	DP(("idisc1=%d,idisc2=%d\n", idisc1, idisc2));
	//if(idisc1 != idisc2)
	//	return ERROR_DVD_DISCDIFFTYPE;

    //�ж�ԭ���Ƿ�Ϊ����
	if ((iRet = pDVDDst->cmd.CDR_GetDiscInfo(pDVDSrc->fd, &TrickCount, &SessionCount, &Capicity, &DiskStatus)))
	{
		DPERROR(("get disc info error:%d\n", iRet));
		return iRet;
	}
	DP(("diskBlocks = %lld, sessioncount:%d, trackcount:%d\n", Capicity, SessionCount, TrickCount));

	if(DiskStatus == DISCSTE_BLANK)
	{
		DPERROR(("SrcDisc is blank!\n"));
		return ERROR_DVD_SRCBLANKDISC;
	}

	//�ж�Ŀ�����Ƿ�Ϊ����
	if ((iRet = pDVDDst->cmd.CDR_GetDiscInfo(pDVDDst->fd, &TrickCount, &SessionCount, &Capicity, &DiskStatus)))
	{
		DPERROR(("get disc info error:%d\n", iRet));
		return iRet;
	}
	DP(("diskBlocks = %lld, sessioncount:%d, trackcount:%d\n", Capicity, SessionCount, TrickCount));

	if(DiskStatus != DISCSTE_BLANK)
	{
		DPERROR(("DstDisc is not blank!\n"));
		return ERROR_DVD_UNBLANKDISC;
	}

	//�����ж�ԭ�̴���Ŀ�������ܸ���
	if( idisc2 > idisc1 )
	{
		DPERROR(("DstDisc is small!\n"));
		return ERROR_DVD_CANTCOPYDISC;
	}

	//��������25g��50g��Ҫ�ж�����
	DVDSDK_GetDiscInfo(pDVDSrc, &SrcDiscInfo);
	DVDSDK_GetDiscInfo(pDVDDst, &DstDiscInfo);
	DP(("SrcDiscInfo.discsize=%d,DstDiscInfo.discsize=%d\n",SrcDiscInfo.discsize,DstDiscInfo.discsize));
	if(SrcDiscInfo.discsize > DstDiscInfo.discsize)
	{
		DPERROR(("DstDisc size is small!\n"));
		return ERROR_DVD_CANTCOPYDISC;
	}

	DVDSDK_SetCopySpeed(pDVDSrc, pDVDDst, idisc1, idisc2);

	/*//��ȡuuid
	uuid_generate(TempUUID);
	for(i=0;i<16;i++)
	{
		sprintf(TempStr,"%02X",TempUUID[i]);
		memcpy(&Buffer[i*2],TempStr,2);
	}*/

	//��ȡ�ڶ����̵Ĺ����Ϣ
	i = 0;
	pDVDDst->cmd.CDR_GetTrackinfo(pDVDDst->fd, 1, &TrackInfo);
	TrackInfo.pbuffer = (uint8_t*)malloc(PACKET32_SIZE * 4);
	//TrackInfo.pbuffer = (uint8_t*)malloc(PACKET32_SIZE);
	TrackInfo.buffsize = 0;
	while (!pDVDSrc->cmd.CDR_ReadTrack(pDVDSrc->fd, i, buf, PACKET32_SIZE))
	{
		if( UDF_UUID_ADDR == i )
		{
			memset( buf, 0, PACKET32_SIZE);
			//memcpy( buf, Buffer, 32);
		}
		pDVDDst->cmd.CDR_WriteTrack(pDVDDst->fd, &TrackInfo, buf, PACKET32_SIZE);
		memset(buf, 0, PACKET32_SIZE);
		i += PACKET_BLOCK_32;
	}
	//�����������ʱ ˵�����ݶ��� ������ѭ�� ��������
	DP(("data is over!\n"));
	iRet = pDVDDst->cmd.CDR_CloseTrack(pDVDDst->fd, &TrackInfo);
	if(iRet)
	{
		DPERROR(("close file track error:%d\n", iRet));
	}

	iRet = pDVDDst->cmd.CDR_CloseSession(pDVDDst->fd, &TrackInfo);
	if(iRet)
	{
		DPERROR(("close file session error:%d\n", iRet));
	}

	pDVDSrc->cmd.CDR_UnlockDoor(pDVDSrc->fd);
	pDVDDst->cmd.CDR_UnlockDoor(pDVDDst->fd);

	free(TrackInfo.pbuffer);
	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_ResumeDisc
* ����  : ���ָ̻�,����ڿ�¼�����жϵ磬���øú������ָ��ļ�ϵͳ
* ����  :
	nDevNo : �豸�ţ�0-n
	DiscName : ��������
	DirName : Ŀ¼����
	FileName : �ļ�����
	FillSize : ����������(Ĭ��Ϊ64M)
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
* �޸�  : 2017.4.30 modify by passion for Complete Func
*******************************************************************************/
int DVDSDKInterface::DVDSDK_ResumeDisc(DVDDRV_HANDLE hDVD, char *DiscName, char *DirName, char *FileName, int FillSize)
{
	int iRet;
	uint64_t Capicity;
	//unsigned char TempUUID[8];
	unsigned int nEmptySize = 0;
	int TrickCount, SessionCount, DiskStatus;
	DISC_VOLID_T disc_volid;
	DVDSDK_DIR  pDir;
	DVDSDK_FILE pNode;
	stCDRTrack stTrack;

	HANDLE_DEF;
	VALID_HANDLE();
	DVD_INRECORD();

	//���ж�Ŀ�����Ƿ�Ϊ����
	if ((iRet = HCDR_CMD().CDR_GetDiscInfo(HDEV_FD(), &TrickCount, &SessionCount, &Capicity, &DiskStatus)))
	{
		DPERROR(("get disc info error:%d\n", iRet));
		return iRet;
	}
	DP(("diskBlocks = %lld, sessioncount:%d, trackcount:%d\n",Capicity, SessionCount, TrickCount));

	if(DiskStatus == DISCSTE_BLANK)
	{
		DPERROR(("Disc is blank!\n"));
		return ERROR_DVD_CANTRESUMEBLANK;
	}

	//һ����� �޷��ָ�
	if(2 != TrickCount)
	{
		DPERROR(("Disc can't resume!\n"));
		return ERROR_DVD_CANTRESUMEDISC;
	}

	//�жϹ��1�Ƿ��ѱ�д������
	HUDF_HANLE()->cdrCmd.CDR_GetTrackinfo(HDEV_FD(), 1, &stTrack);
	DP(("1stTrack.freeblocks=%d\n",stTrack.freeblocks));
	if( 1024 != stTrack.freeblocks )
	{
		DPERROR(("Disc can't resume!\n"));
		return ERROR_DVD_CANTRESUMEDISC;
	}

	//�жϹ��2�Ƿ���
	HUDF_HANLE()->cdrCmd.CDR_GetTrackinfo(HDEV_FD(), 2, &stTrack);
	DP(("2stTrack.freeblocks=%d,stTrack.writedsize=%d\n",stTrack.freeblocks,stTrack.writedsize));
	if(( 0 == stTrack.freeblocks )||( 0 == stTrack.writedsize )||( 0 == stTrack.writenext ))
	{
		DPERROR(("Disc can't resume!\n"));
		return ERROR_DVD_CANTRESUMEDISC;
	}

	memset(&disc_volid, 0, sizeof(DISC_VOLID_T));
	strcpy(disc_volid.logicalVolIdent, DiscName);
	strcpy(disc_volid.volIdent,        DiscName);
	strcpy(disc_volid.volSetIdent,     DiscName);
// 	strcpy(disc_volid.LVInfoTitle, 	      "COMM_DVD_1.0");
// 	strcpy(disc_volid.LVInfoDataTime,     "2017-4-17");
// 	strcpy(disc_volid.LVInfoEmail,        "avs_work");
// 	strcpy(disc_volid.fileSetIdent,       "avs_DVD_1.0");
// 	strcpy(disc_volid.copyrightFileIdent, "avs_DVD_1.0");
// 	strcpy(disc_volid.abstractFileIdent,  "avs_DVD_1.0");
	strcpy(disc_volid.LVInfoTitle, 	      "DVDRECORD");
	strcpy(disc_volid.LVInfoDataTime,     "2017-4-17");
	strcpy(disc_volid.LVInfoEmail,        "DVDRECORD");
	strcpy(disc_volid.fileSetIdent,       "DVDRECORD");
	strcpy(disc_volid.copyrightFileIdent, "DVDRECORD");
	strcpy(disc_volid.abstractFileIdent,  "DVDRECORD");


	// ����UUID
	//uuid_generate_8bytes(TempUUID);
	memset(HUDF_HANLE()->m_CdRwDiskinfo->ExtenData, 0, PACKET16_SIZE);
	//memcpy(HUDF_HANLE()->m_CdRwDiskinfo->ExtenData, TempUUID, 8);

	HUDF_HANLE()->m_CdRwDiskinfo->nDiskCapicity = Capicity;

	//������Ŀ¼
	HUDF_HANLE()->udfCmd.AddNode(HUDF_HANLE()->m_hMem, HUDF_HANLE()->m_FileDirTree, NULL, "", 0, NODETYPE_DIR);

	//��ʼ���ļ�ϵͳ
	iRet = HUDF_HANLE()->udfCmd.InitUdfFs(HUDF_HANLE(), &disc_volid);
	if(iRet)
	{
		DPERROR(("init udf fs error:%d\n", iRet));
		HCDR_CMD().CDR_UnlockDoor(HDEV_FD());
		return ERROR_DVD_FORMATFAILED;
	}

	DP(("trackID1=%d\n", HUDF_HANLE()->m_CdRwDiskinfo->udfsys.trackid));
	DP(("trackID2=%d\n", HUDF_HANLE()->m_CdRwDiskinfo->udffile.trackid));

	//��ȡ�������
	HCDR_CMD().CDR_GetTrackinfo(HDEV_FD(),
			HUDF_HANLE()->m_CdRwDiskinfo->udfsys.trackid, &(HUDF_HANLE()->m_CdRwDiskinfo->udfsys));

	HCDR_CMD().CDR_GetTrackinfo(HDEV_FD(),
			HUDF_HANLE()->m_CdRwDiskinfo->udffile.trackid, &(HUDF_HANLE()->m_CdRwDiskinfo->udffile));

	//����Ŀ¼���ļ�
	pDir  = DVDSDK_CreateDir(hDVD, DirName);
	pNode = DVDSDK_CreateFile(hDVD, pDir, FileName, 0);

	if( NULL == pNode )
	{
		DVDRec_UdfTreeFree(HUDF_HANLE());
		DPERROR(("filename is exist!\n"));
		return ERROR_DVD_NAMEEXIST;
	}

	//�趨�ļ���ʼ (Ĭ�����Ϊ1024 * 1024 * 64)
	nEmptySize = FillSize ? FillSize : (1024 * 1024 * 64);
	nEmptySize &= ~(PACKET32_SIZE - 1);	// 64K����

	((FILDIRNODE *)(pNode))->FileLoca = HUDF_HANLE()->m_CdRwDiskinfo->udffile.writestart
		                                        - UDF_SYS_LEN + (nEmptySize/CDROM_BLOCK);
	((FILDIRNODE *)(pNode))->FileSize = (HUDF_HANLE()->m_CdRwDiskinfo->udffile.writedsize - 1040 - PACKET_BLOCK_32)*CDROM_BLOCK
		               					 - nEmptySize;           //1040�ڶ������ʼ
	((HDVD_DEV_T*)hDVD)->bRecording = TRUE;
	//д���ļ�ϵͳ �رչ������
	DVDSDK_CloseDisc((HDVD_DEV_T*)hDVD);

	return 0;
}


/*******************************************************************************
* ����  : DVDSDK_GetReserveData
* ����  : ��ñ����������, �����Ѿ����̵Ĺ��̲���Ч, ��ȡʱ����
* ����  :
	nDevNo : �豸�ţ�0-n
	pBuffer: ���ر�������ָ��
	pSize  : ����Buffer����
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_GetReserveData(DVDDRV_HANDLE hDVD, unsigned char **pBuffer, int *pSize)
{
	HANDLE_DEF;
	VALID_HANDLE();
	DVD_INRECORD();

	HCDR_CMD().CDR_ReadTrack(HDEV_FD(), UDF_UUID_ADDR,
		     (uint8_t*)(HUDF_HANLE()->m_CdRwDiskinfo->ExtenData),PACKET16_SIZE);

	*pBuffer = (unsigned char*)HUDF_HANLE()->m_CdRwDiskinfo->ExtenData;
	*pSize   = PACKET16_SIZE;
	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_GetReserveBuffer
* ����  : ��ñ����������ָ��, ��������ڱ������ǰ���ã�ֱ���޸�Buffer����
		  ����ʱд�������,��С 32K�� ��¼ʱ����
* ����  :
	nDevNo : �豸�ţ�0-n
	pBuffer: ���ر�������ָ��
	pSize  : ����Buffer����
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_GetReserveBuffer(DVDDRV_HANDLE hDVD, unsigned char **pBuffer, int *pSize)
{
	HANDLE_DEF;
	VALID_HANDLE();

	*pBuffer = (unsigned char*)HUDF_HANLE()->m_CdRwDiskinfo->ExtenData;
	*pSize   = PACKET16_SIZE;
	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_GetTotalWriteSize
* ����  : ��ù���������д�ռ�
* ����  :
	nDevNo    : �豸�ţ�0-n
	pTotalSize: ����������д�ռ�
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_GetTotalWriteSize(DVDDRV_HANDLE hDVD, unsigned long long *pTotalSize)
{
	int iRet = 0;
	HANDLE_DEF;
	VALID_HANDLE();

	if(pDVD->bRecording)
	{
		uint64_t tmp;
		tmp = HUDF_HANLE()->m_CdRwDiskinfo->udffile.freeblocks;
		*pTotalSize = HUDF_HANLE()->m_CdRwDiskinfo->nDiskCapicity;
	}
	else
	{
		int TrickCount, SessionCount, DiskStatus;
		uint64_t uTotalSize;
		iRet = HCDR_CMD().CDR_GetDiscInfo(HDEV_FD(), &TrickCount, &SessionCount, &uTotalSize, &DiskStatus);
		*pTotalSize = (uTotalSize<<11);
	}
	return iRet;
}

/*******************************************************************************
* ����  : DVDSDK_GetFreeWriteSize
* ����  : ���ʣ���д�ռ�
* ����  :
	nDevNo   : �豸�ţ�0-n
	pFreeSize: ����ʣ���д�ռ�
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int DVDSDKInterface::DVDSDK_GetFreeWriteSize(DVDDRV_HANDLE hDVD, unsigned long long *pFreeSize)
{
	int iRet = 0;
	HANDLE_DEF;
	VALID_HANDLE();

	if(pDVD->bRecording)
	{
		uint64_t tmp;
		tmp = HUDF_HANLE()->m_CdRwDiskinfo->udffile.freeblocks;
		*pFreeSize = (tmp << 11);
	}
	else
	{
		int TrickCount, SessionCount, DiskStatus;
		uint64_t uTotalSize;
		iRet = HCDR_CMD().CDR_GetDiscInfo(HDEV_FD(), &TrickCount, &SessionCount, &uTotalSize, &DiskStatus);
		*pFreeSize = (uTotalSize<<11);
	}
	return iRet;
}

/*******************************************************************************
* ����  : DVDSDK_PrintProfile
* ����  : ��ӡprofile
* ����  :
	nDevNo   : �豸�ţ�0-n
* ����ֵ: 0: �ɹ�
* ����  : passion
* ����  : 2017.1.14
*******************************************************************************/
int DVDSDKInterface::DVDSDK_PrintProfile(DVDDRV_HANDLE hDVD)
{
	HANDLE_DEF;
	VALID_HANDLE();
	HCDR_CMD().CDR_DpdevProfile(HDEV_FD());

	return 0;
}

/*******************************************************************************
* ����  : DVDSDK_SetRecordStatus
* ����  : ���ù�����¼״̬�����ڹ��̸�ʽ�����¼ʧ��δ���̣��ٴθ�ʽ��ǰ���ÿ�¼״̬��

* ����  :
	hDVD   : DVD�������
* ����ֵ: 
* ����  : passion
* ����  : 2017.4.5
*******************************************************************************/
void DVDSDKInterface::DVDSDK_SetRecordStatus(DVDDRV_HANDLE hDVD, BOOL bRecordStatus)
{
	HANDLE_DEF;
    pDVD->bRecording = bRecordStatus;
}

/*******************************************************************************
* ����  : LvDVDRec_udffstest
* ����  : UDF������
* ����  :
* ����ֵ: 0: �ɹ�������Ϊ����ֵ
* ����  : passion
* ����  : 2017.3.31
*******************************************************************************/
int LvDVDRec_udffstest(void)
{
	DP(("LvDVDRec_udffstest ...start!\n"));
	udfinfo_t  *pUdf =  DVDRec_UdfCreate(-1, UDFDATAMODE_DATA);
	if(!pUdf)
	{
		DPERROR(("LvDVDRec_UdfCreate error\n"));
		return -1;
	}
	DP(("LvDVDRec_udffstest ...start1!\n"));
	DVDRec_GetCdrcmd(NULL, &pUdf->cdrCmd);
	DP(("LvDVDRec_udffstest ...start2!\n"));
	pUdf->udfCmd.UdfFsTest(pUdf);
	DP(("LvDVDRec_udffstest ...start3!\n"));
	DVDRec_UdfFree(pUdf);
	DP(("LvDVDRec_udffstest ...end!\n"));
	return 0;
}

