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
#define HCDR_CMD()		pDVD->pCmd

#define HANDLE_DEF		HDVD_DEV_T *pDVD = (HDVD_DEV_T*)hDVD;

#define DISC_MINFREEBLOCKS	131072		// 剩余块(131072 * 2048 = 256M)
#define MASK_ID				0x12345678	// MASK ID

typedef struct{
	int  maskid;				// maskid
	int  fd;					// 设备句柄
	int	 isBusy;				// 忙标志
	BOOL bRecording;			// 刻录标志
	LVDVD_DEV_INFO_T stDevInfo;	// 设备信息
	udfinfo_t  *pUdf;			// UDF 文件系统
	struct CDR_CMD_T *pCmd;  	// 光驱命令
}HDVD_DEV_T;

/*******************************************************************************
* 名称  : Xkd_DVDSDK_Load
* 描述  : 加载光驱,
* 参数  :
	szDevName : 驱动名称，如: /dev/sr0, /dev/sr1
* 返回值: 设备句柄，NULL为失败
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
XKD_DVDDRV Xkd_DVDSDK_Load(const char *szDevName)
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
	LvDVDRec_GetCdrcmd(NULL, &pDVD->pCmd);
	pDVD->pUdf = LvDVDRec_UdfCreate(pDVD->fd, UDFDATAMODE_DATA);
	if(!pDVD->pUdf)
	{
		DPERROR(("crate udf error\n"));
		goto err_exit;
	}
	pDVD->pUdf->cdr_cmd = pDVD->pCmd;
	pDVD->pCmd->cdr_unlockdoor(pDVD->fd);

	return pDVD;

err_exit:
	if(pDVD)
	{
		if(pDVD->pUdf)
			LvDVDRec_UdfFree(pDVD->pUdf);
		free(pDVD);
	}
	if(fd > 0) close(fd);
	return NULL;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_UnLoad
* 描述  : 卸载光驱
* 参数  :
	hDVD : Xkd_DVDSDK_Load的返回值
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_UnLoad(XKD_DVDDRV hDVD)
{
	HANDLE_DEF;
	VALID_HANDLE();

	if(pDVD->pUdf)
	{
		//printf("Xkd_DVDSDK_UnLoad 1\n");
		LvDVDRec_UdfFree(pDVD->pUdf);
		//printf("Xkd_DVDSDK_UnLoad 2\n");
	}
	/*if(pDVD->pCmd && pDVD->fd > 0)
	{
		//printf("Xkd_DVDSDK_UnLoad 3\n");
		pDVD->pCmd->cdr_closetray(pDVD->fd);
		//printf("Xkd_DVDSDK_UnLoad 4\n");
	}*/


	if(pDVD->fd > 0)
		close(pDVD->fd);

	//printf("Xkd_DVDSDK_UnLoad 5\n");
	free(pDVD);
	//printf("Xkd_DVDSDK_UnLoad ok\n");
	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_Tray
* 描述  : 打开/关闭托盘
* 参数  :
	hDVD : Xkd_DVDSDK_Load的返回值
	bOpen  : TRUE:打开托盘, FALSE:关闭托盘
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int	Xkd_DVDSDK_Tray(XKD_DVDDRV hDVD, int bOpen)
{
	BOOL bRet;

	HANDLE_DEF;
	VALID_HANDLE();

	// 光驱正忙返回
	DVD_ISBUSY();

	if(bOpen)
	{
		DVD_INRECORD();
		bRet = HCDR_CMD()->cdr_opentray(HDEV_FD());
	}
	else
	{
		bRet = HCDR_CMD()->cdr_closetray(HDEV_FD());
	}
	return bRet ? 0 : ERROR_DVD_OPTERFAILED;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_GetTrayState
* 描述  : 获得托盘状态(打开/关闭)
* 参数  :
	hDVD : Xkd_DVDSDK_Load的返回值
* 返回值: 1：打开，0：关闭，其他为错误代码
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int	Xkd_DVDSDK_GetTrayState(XKD_DVDDRV hDVD)
{
	int iRet;

	HANDLE_DEF;
	VALID_HANDLE();

	iRet = HCDR_CMD()->cdr_getdoorstate(HDEV_FD());
	return iRet;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_LockDoor
* 描述  : 锁定/解锁光驱门, 防止在刻录过程中意外打开
* 参数  :
	hDVD : Xkd_DVDSDK_Load的返回值
	bLocked: TRUE:锁定, FALSE:解锁
* 返回值: 0:成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_LockDoor(XKD_DVDDRV hDVD, int bLocked)
{
	BOOL bRet;
	HANDLE_DEF;
	VALID_HANDLE();
	// 光驱正忙返回
	DVD_ISBUSY();

	if(bLocked)
		bRet = HCDR_CMD()->cdr_lockdoor(HDEV_FD());
	else
		bRet = HCDR_CMD()->cdr_unlockdoor(HDEV_FD());
	return bRet ? 0 : ERROR_DVD_OPTERFAILED;
}


/*******************************************************************************
* 名称  : Xkd_DVDSDK_GetDevInfo
* 描述  : 获得光驱信息
* 参数  :
	hDVD : Xkd_DVDSDK_Load的返回值
	pDevInfo   : 光驱信息指针
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_GetDevInfo(XKD_DVDDRV hDVD, LVDVD_DEV_INFO_T *pDevInfo)
{
	HANDLE_DEF;
	VALID_HANDLE();

	memset(&pDVD->stDevInfo, 0, sizeof(LVDVD_DEV_INFO_T));
	HCDR_CMD()->cdr_getdevinfo(HDEV_FD(), &pDVD->stDevInfo);
	memcpy(pDevInfo, &pDVD->stDevInfo, sizeof(LVDVD_DEV_INFO_T));
	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_GetDiscInfo
* 描述  : 获得碟片信息
* 参数  :
	nDevNo     : 设备号，0-n, -1为全部设备
	pDiscInfo  : 碟片信息结构指针
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_GetDiscInfo(XKD_DVDDRV hDVD, LVDVD_DISC_INFO_T *pDiscInfo)
{
	int TrickCount,SessionCount,DiskStatus,Speed;
	unsigned long long Capicity;

	HANDLE_DEF;
	VALID_HANDLE();

	//初始化
	Capicity 		= 0;
	TrickCount 		= 0;
	SessionCount 	= 0;
	DiskStatus 		= 0;
	Speed 			= 0;

	//获得光盘总容量 单位M
	HCDR_CMD()->cdr_getdiscinfo(HDEV_FD(),&TrickCount,&SessionCount,&Capicity,&DiskStatus);
	DP(("Capicity=%lld\n",Capicity));        //光驱区数(2048)
	pDiscInfo->discsize = (Capicity/512);    //除以512及为M单位容量

	//获取光盘使用容量 单位M
	HCDR_CMD()->cdr_getdiscusedsize(HDEV_FD(),&pDiscInfo->usedsize);

	//剩余容量
	pDiscInfo->freesize = pDiscInfo->discsize - pDiscInfo->usedsize;
    if ( pDiscInfo->freesize < 0 )
    {
        pDiscInfo->freesize = 0;
    }

	//光盘类型
	HCDR_CMD()->cdr_discexacttype(HDEV_FD(), &pDiscInfo->ntype);

	//最大速度
	HCDR_CMD()->cdr_getmaxspeed(HDEV_FD(), &Speed, &pDiscInfo->maxpeed);

	//打印数据
	DP(("ntype=%d,maxpeed=%d,discsize=%d,usedsize=%d,freesize=%d\n",pDiscInfo->ntype,
		pDiscInfo->maxpeed,pDiscInfo->discsize,pDiscInfo->usedsize,pDiscInfo->freesize));
	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_HaveDisc
* 描述  : 判断是否有光盘
* 参数  :
	nDevNo     : 设备号，0-n, -1为全部设备
* 返回值: TRUE: 有，FALSE：无
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_HaveDisc(XKD_DVDDRV hDVD)
{
	HANDLE_DEF;
	VALID_HANDLE();
	return HCDR_CMD()->cdr_havedisc(HDEV_FD());
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_GetMediaExactType
* 描述  : 获取光盘类型(精确类型)
* 参数  :
	nDevNo : 设备号，0-n, -1为全部设备
* 返回值: 光盘类型
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_GetMediaExactType(XKD_DVDDRV hDVD)
{
	int iExactT;

	HANDLE_DEF;
	VALID_HANDLE();

	iExactT = 0;
	HCDR_CMD()->cdr_discexacttype(HDEV_FD(), &iExactT);

	return iExactT;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_GetMediaBasicType
* 描述  : 获取光盘类型(基本类型)(用于设定光驱速度和复制光盘时)
* 参数  :
	nDevNo : 设备号，0-n, -1为全部设备
* 返回值: 光盘类型
* 作者  : xkd
* 日期  : 2010.12.29
*******************************************************************************/
int Xkd_DVDSDK_GetMediaBasicType(XKD_DVDDRV hDVD)
{
	int iBasicT,Ret = 0;

	HANDLE_DEF;
	VALID_HANDLE();

	iBasicT = -1;
	Ret = HCDR_CMD()->cdr_discbasictype(HDEV_FD(),&iBasicT);
	if(( Ret > CD_DISC )||( Ret < B_DVD_DISC ))
		return Ret;
	return iBasicT;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_SetWriteSpeed
* 描述  : 设定刻录速度
* 参数  :
	nDevNo : 设备号，0-n, -1为全部设备
	speed  : 速度，必须是 1 2 4 6 8 12
	disctpye  : 光盘类型   	DVD_DISC = 0 ,DVD_DL_DISC = 1 ,CD_DISC = 2
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
* 修改  : 2010.12.30 Modify by yanming add disctype
*******************************************************************************/
int Xkd_DVDSDK_SetWriteSpeed(XKD_DVDDRV hDVD, int speed, int disctype)
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
	MaxReadSpeed = MaxWriteSpeed = 2;   //默认为2倍速
	//获取支持的最大速度
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
	//设定速度
	return HCDR_CMD()->cdr_setspeed(HDEV_FD(), MaxReadSpeed, MaxWriteSpeed, disctype);

	//return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_SetCopySpeed
* 描述  : 设定光盘复制的速度
* 参数  :
	nSrcDevno : 源设备号，0-n
	nDstDevno : 目的设备号，0-n
	srctype  : 光盘类型:B_DVD_DISC = 0,DVD_DL_DISC = 1,DVD_DISC = 2,CD_DISC = 3
	dsttype  : 光盘类型:B_DVD_DISC = 0,DVD_DL_DISC = 1,DVD_DISC = 2,CD_DISC = 3
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2010.1.14
*******************************************************************************/
int Xkd_DVDSDK_SetCopySpeed(XKD_DVDDRV HDVDSrc, XKD_DVDDRV HDVDDst, int srctype, int dsttype)
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

	SrcMaxReadSpeed = SrcMaxWriteSpeed = 2;   //默认为2倍速
	DstMaxReadSpeed = DstMaxWriteSpeed = 2;   //默认为2倍速

	//获取支持的最大速度
	pDVDSrc->pCmd->cdr_getmaxspeed(pDVDSrc->fd, &SrcMaxReadSpeed, &SrcMaxWriteSpeed);
	pDVDDst->pCmd->cdr_getmaxspeed(pDVDDst->fd, &DstMaxReadSpeed, &DstMaxWriteSpeed);

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

	//设定源盘速度
	SrcMaxReadSpeed = (SrcMaxReadSpeed/SrcTypeS);

	SrcMaxWriteSpeed = (SrcMaxWriteSpeed/SrcTypeS);

	//设定目标盘速度
	DstMaxReadSpeed = (DstMaxReadSpeed/DstTypeS);

	DstMaxWriteSpeed = (DstMaxWriteSpeed/DstTypeS);

	//设定速度
	pDVDSrc->pCmd->cdr_setspeed(pDVDSrc->fd, SrcMaxReadSpeed, SrcMaxWriteSpeed, srctype);
	pDVDDst->pCmd->cdr_setspeed(pDVDDst->fd, DstMaxReadSpeed, DstMaxWriteSpeed, dsttype);

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
* 名称  : Xkd_DVDSDK_LoadDisc
* 描述  : 加载光盘
* 参数  :
	nDevNo : 设备号，0-n
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_LoadDisc(XKD_DVDDRV hDVD)
{
	int iRet;

	HANDLE_DEF;
	VALID_HANDLE();

	// 光驱正忙返回
	DVD_ISBUSY();

#ifdef DEBUG
	set_core_dump(1, 256*1024*1024);
#endif

	iRet = HCDR_CMD()->cdr_loadmedia(HDEV_FD());

	return iRet;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_DiscCanWrite
* 描述  : 判断光盘是否可写
* 参数  :
	nDevNo : 设备号，0-n
* 返回值: 0: 可写，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_DiscCanWrite(XKD_DVDDRV hDVD)
{
	int iRet;
	uint64_t Capicity;
	int TrickCount, SessionCount, DiskStatus;

	HANDLE_DEF;
	VALID_HANDLE();
	// 光驱正忙返回
	iRet = HCDR_CMD()->cdr_loadmedia(HDEV_FD());
	if(iRet)
		return iRet;

	if( (iRet = HCDR_CMD()->cdr_getdiscinfo(HDEV_FD(), &TrickCount, &SessionCount, &Capicity, &DiskStatus) ) )
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
* 名称  : Xkd_DVDSDK_FormatDisc
* 描述  : 格式化光盘, 分轨道, 创建UDF文件系统, 准备开始写数据; 目前只支持一个分区，2个轨道
* 参数  :
	nDevNo : 设备号，0-n
	szDiscName: 光盘名称
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int	Xkd_DVDSDK_FormatDisc(XKD_DVDDRV hDVD, char *szDiscName)
{
	int iRet;
	uint64_t Capicity;
	DISC_VOLID_T disc_volid;
	//获取光盘信息和状态
	int TrickCount, SessionCount, DiskStatus;

	HANDLE_DEF;
	VALID_HANDLE();

	// 光驱正忙返回
	DVD_ISBUSY();
	DVD_INRECORD();

	memset(&disc_volid, 0, sizeof(DISC_VOLID_T));
	strcpy(disc_volid.logicalVolIdent, szDiscName);
	strcpy(disc_volid.volIdent,        szDiscName);
	strcpy(disc_volid.volSetIdent,     szDiscName);
	strcpy(disc_volid.LVInfoTitle, 	      "xkd_DVD_1.0");
	strcpy(disc_volid.LVInfoDataTime,     "2010-12-17");
	strcpy(disc_volid.LVInfoEmail,        "xkd_work");
	strcpy(disc_volid.fileSetIdent,       "xkd_DVD_1.0");
	strcpy(disc_volid.copyrightFileIdent, "xkd_DVD_1.0");
	strcpy(disc_volid.abstractFileIdent,  "xkd_DVD_1.0");

	iRet = HCDR_CMD()->cdr_loadmedia(HDEV_FD());
	if(iRet)
	{
		// 没有光盘
		return ERROR_DVD_NODISC;
	}

	if( (iRet = HCDR_CMD()->cdr_getdiscinfo(HDEV_FD(), &TrickCount, &SessionCount, &Capicity, &DiskStatus) ) )
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

	LvDVDRec_UdfTreeFree(HUDF_HANLE());

	MEMMOCLINE;
	//创建根目录
	HUDF_HANLE()->udf_cmd->addnode(HUDF_HANLE()->m_hMem, HUDF_HANLE()->m_FileDirTree, NULL, "", 0, NODETYPE_DIR);

	sleep(1);

	if( TrickCount == 1 )
	{
		DP(("start to format disc[%d]\n", pDVD->fd));
		iRet = HCDR_CMD()->cdr_formatdisc(HDEV_FD(), UDF_FS_LENGTH);
		if(iRet)
		{
			DPERROR(("format disc error:%d\n", iRet));
			HCDR_CMD()->cdr_unlockdoor(HDEV_FD());
			return ERROR_DVD_FORMATFAILED;
		}
	}

	HUDF_HANLE()->m_CdRwDiskinfo->nDiskCapicity = Capicity;

	iRet = HUDF_HANLE()->udf_cmd->InitUdfFs(HUDF_HANLE(), &disc_volid);
	if(iRet)
	{
		DPERROR(("init udf fs error:%d\n", iRet));
		HCDR_CMD()->cdr_unlockdoor(HDEV_FD());
		return ERROR_DVD_FORMATFAILED;
	}

	//DP(("trackID1=%d\n", HUDF_HANLE()->m_CdRwDiskinfo->udfsys.trackid));
	//DP(("trackID2=%d\n", HUDF_HANLE()->m_CdRwDiskinfo->udffile.trackid));

	HCDR_CMD()->cdr_gettrackinfo(HDEV_FD(),
			HUDF_HANLE()->m_CdRwDiskinfo->udfsys.trackid,  &(HUDF_HANLE()->m_CdRwDiskinfo->udfsys));

	HCDR_CMD()->cdr_gettrackinfo(HDEV_FD(),
			HUDF_HANLE()->m_CdRwDiskinfo->udffile.trackid, &(HUDF_HANLE()->m_CdRwDiskinfo->udffile));

	pDVD->bRecording = TRUE;

	DP(("format disc[%d] ...........complete ok\n", pDVD->fd));
	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_SetFileLoca
* 描述  : 设定文件位置 (写文件前都需要调用这个函数)
* 参数  :
	nDevNo : 设备号，0-n
	FileNode : 文件节点
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2011.1.17
*******************************************************************************/
int Xkd_DVDSDK_SetFileLoca(XKD_DVDDRV hDVD, Xkd_DVDSDK_FILE FileNode)
{
	HANDLE_DEF;
	VALID_HANDLE();

	((FILDIRNODE *)(FileNode))->FileLoca = HUDF_HANLE()->m_CdRwDiskinfo->udffile.writenext - UDF_SYS_LEN;
    DP(("+++++++++[Xkd_DVDSDK_SetFileLoca] File name = %s,File  location = %ld++++++++++\n",((FILDIRNODE *)(FileNode))->Name,((FILDIRNODE *)(FileNode))->FileLoca));

	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_FillEmptyDataOnFirst
* 描述  : 填充空数据，在格式化之后，开始刻录之前调用，避免开始刻录时写轨道数据的停滞，
          造成视频丢帧  (第一个文件前调用，之后的文件调用Xkd_DVDSDK_SetFileLoca)
* 参数  :
	nDevNo : 设备号，0-n
	fullsize: 填充大小，0为自动计算填充大小
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_FillEmptyDataOnFirst(XKD_DVDDRV hDVD, unsigned int fullsize)
{
	unsigned int nEmptySize;

	HANDLE_DEF;
	VALID_HANDLE();

	// 光驱正忙返回
	DVD_ISBUSY();

	nEmptySize = fullsize ? fullsize : (1024 * 1024 * 64);
	nEmptySize &= ~(PACKET32_SIZE - 1);	// 64K对其

	HUDF_HANLE()->udf_cmd->WriteFileTrackEmdpy(HUDF_HANLE(), nEmptySize);

	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_Analysisdirectory
* 描述  : 添加目录是对目录字符进行解析
* 参数  :
	szDirName : 目录名
	len : 目录长度
	szDirRemain: 解析剩余的字符串例如:szDirName为"/test/test1/test2",
	                返回值为:"/test1/test2"
    szRemainName:解析的字符
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2010.12.21
* 说明  : 目录第一个字符必须为"/"
*******************************************************************************/
static int Xkd_DVDSDK_Analysisdirectory(char * szDirName, int len, char ** szDirRemain, char * szAnalysisName)
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
* 名称  : Xkd_DVDSDK_CreateDir
* 描述  : 创建目录
* 参数  :
	nDevNo : 设备号，0-n
	pParentDir: 父目录，为空为跟目录下创建
	szDirName : 目录名称，不能为空
* 返回值: 目录节点指针, Xkd_DVDSDK_CreateFile函数会用到, NULL:创建目录失败
* 作者  : xkd
* 日期  : 2017.3.31
* 修改  : 2010.12.21 xkd
          参数:	nDevNo : 设备号，0-n
				szDirName : 要创建的目录,方式为"/test/test1/test2"
*******************************************************************************/
Xkd_DVDSDK_DIR Xkd_DVDSDK_CreateDir(XKD_DVDDRV hDVD, char *szDirName)
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
		iRet = Xkd_DVDSDK_Analysisdirectory(szDirRemain, strlen(szDirRemain), &szDirRemain, szAnalysisName);

		if(iRet == -1)
			break;
		if(DirNode == NULL)
			DirNode = HUDF_HANLE()->udf_cmd->findnodebyid(HUDF_HANLE()->m_FileDirTree, 0);

		if (DirNode == NULL)
		{
			DirNode = HUDF_HANLE()->udf_cmd->addnode(HUDF_HANLE()->m_hMem, HUDF_HANLE()->m_FileDirTree,
									NULL, "", 0, NODETYPE_DIR);//没根目录 则创建根目录
		}
		tmpNode = HUDF_HANLE()->udf_cmd->GetNodeInDir(DirNode, szAnalysisName);
		if(tmpNode)
		{
			DP(("Dir name:%s exist\n", szDirName));
			DirNode = tmpNode;
			continue;//return NULL;
		}
		MEMMOCLINE;
		DirNode = HUDF_HANLE()->udf_cmd->addnode(HUDF_HANLE()->m_hMem, HUDF_HANLE()->m_FileDirTree,
						(FILDIRNODE*)DirNode, szAnalysisName, 0, NODETYPE_DIR);
	}
	return DirNode;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_CreateFile
* 描述  : 创建文件，开始写数据
* 参数  :
	nDevNo : 设备号，0-n
	pDir   : 目录节点指针, NULL为根目录, Xkd_DVDSDK_CreateDir的返回值
	szFileName: 文件名称
* 返回值: 文件节点指针，NULL：创建失败
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
Xkd_DVDSDK_FILE Xkd_DVDSDK_CreateFile(XKD_DVDDRV hDVD, Xkd_DVDSDK_DIR pDir, char *szFileName, uint64_t filesize)
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
		pParent = HUDF_HANLE()->udf_cmd->findnodebyid(HUDF_HANLE()->m_FileDirTree, 0);

	MEMMOCLINE;
	DirNode = HUDF_HANLE()->udf_cmd->addnode(HUDF_HANLE()->m_hMem, HUDF_HANLE()->m_FileDirTree,
				pParent, szFileName, filesize, NODETYPE_FILE);
	if(!DirNode)
	{
		DPERROR(("create file failed!\n"));
	}
	return DirNode;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_WriteData
* 描述  : 向文件中写数据, size = 32 * 1024
* 参数  :
	nDevNo : 设备号，0-n
	pFile  : 文件节点指针，Xkd_DVDSDK_CreateFile的返回值
	pBuffer: 数据buffer
	size   : 数据大小，必须是 32*1024 的整数倍
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int	Xkd_DVDSDK_WriteData(XKD_DVDDRV hDVD, Xkd_DVDSDK_FILE pFile, unsigned char *pBuffer, int size)
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
	// 设定文件位置
	//HUDF_HANLE(nDevNo)->m_CurrentFileLocation = HUDF_HANLE(nDevNo)->m_CdRwDiskinfo->udffile.writestart - UDF_SYS_LEN;

	iRet = HUDF_HANLE()->udf_cmd->WriteStream(HUDF_HANLE(), pFile, pBuffer, size);
	if(iRet)
	{
		DPERROR(("WriteStream error:%d,fd=%d\n", iRet,HDEV_FD()));
		return ERROR_DVD_WRITEERROR;
	}
	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_CloseFile
* 描述  : 关闭文件
* 参数  :
	nDevNo : 设备号，0-n
	pFile  : 文件节点指针，Xkd_DVDSDK_CreateFile的返回值
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_CloseFile(XKD_DVDDRV hDVD, Xkd_DVDSDK_FILE pFile)
{
	HANDLE_DEF;
	VALID_HANDLE();

	// Flush所有数据到文件轨道
	HCDR_CMD()->cdr_flushtrack(HDEV_FD(), &HUDF_HANLE()->m_CdRwDiskinfo->udffile);

	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_FillAllDiscEmptyData
* 描述  : 填充光盘所有剩余空间(在刻录停止之后，封盘之前调用)
* 参数  :
	nDevNo : 设备号，0-n
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
* 修改  : 20111.1.17 modify by yanming for FUNC
*******************************************************************************/
int Xkd_DVDSDK_FillAllDiscEmptyData(XKD_DVDDRV hDVD)
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

	DiscType = Xkd_DVDSDK_GetMediaBasicType(hDVD);
	if(( DiscType > CD_DISC )||( DiscType < B_DVD_DISC ))
		return DiscType; //不是光盘类型就返回其错误值

	//设定最大速度 (一般DVD最大速度16，当前盘小于16，则按当前盘的最大速设定)
	Xkd_DVDSDK_SetWriteSpeed(hDVD, 16, DiscType);

	//单位为M
	MinSize = ((HUDF_HANLE()->m_CdRwDiskinfo->udffile.tracksize / 512)/100) * 5;//剩余容量 按轨道容量的5/100来计算
	while(1)
	{
		FreeSize = (HUDF_HANLE()->m_CdRwDiskinfo->udffile.freeblocks / 512);
		//DP(("FreeSize=%d,MinSize=%d\n",FreeSize,MinSize));
		if(FreeSize < MinSize)
			break;
		HUDF_HANLE()->udf_cmd->WriteEmptyStream(HUDF_HANLE(), FillBuf, PACKET32_SIZE);
	}

	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_CloseDisc
* 描述  : 写入UDF文件系统，释放UDF文件系统, 关闭轨道; 调用该接口后，光盘将不可写;
* 参数  :
	nDevNo : 设备号，0-n
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_CloseDisc(XKD_DVDDRV hDVD)
{
	int iRet;
	HANDLE_DEF;
	VALID_HANDLE();

	if(!pDVD->bRecording)
	{
		return 0;
	}

	pDVD->bRecording = FALSE;

	//写入udf文件系统
	iRet = HUDF_HANLE()->udf_cmd->CloseDisc(HUDF_HANLE());
	if(iRet)
	{
		DPERROR(("close disc error:%d\n", iRet));
		LvDVDRec_UdfTreeFree(HUDF_HANLE());
		HCDR_CMD()->cdr_unlockdoor(HDEV_FD());
		return iRet;
	}
    else
    {
        DP(("udf_cmd->CloseDisc success! fd = %d \r\n", HDEV_FD()));
    }

	iRet = HCDR_CMD()->cdr_closetrack(HDEV_FD(), &HUDF_HANLE()->m_CdRwDiskinfo->udffile);
	if(iRet)
	{
		DPERROR(("close file track error:%d, fd=%d\n", iRet,HDEV_FD()));
	}

	iRet = HCDR_CMD()->cdr_closesession(HDEV_FD(), &HUDF_HANLE()->m_CdRwDiskinfo->udffile);
	if(iRet)
	{
		DPERROR(("close file session error:%d, fd=%d\n", iRet,HDEV_FD()));
	}

	// 释放UDF目录树
	LvDVDRec_UdfTreeFree(HUDF_HANLE());
	HCDR_CMD()->cdr_unlockdoor(HDEV_FD());

	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_CopyDisc
* 描述  : 光盘复制,直接轨道拷贝，但是保留轨道不复制
* 参数  :
	nSrcDevno : 源设备号，0-n
	nDstDevno : 目的设备号，0-n
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_CopyDisc(XKD_DVDDRV HDVDSrc, XKD_DVDDRV HDVDDst)
{
	int iRet,idisc1,idisc2;
	int i = 0;
    //unsigned char TempUUID[8];
	uint64_t Capicity;
	CDR_TRACK_T TrackInfo;
	//char Buffer[32];
	//char TempStr[3];
	int TrickCount, SessionCount, DiskStatus;
	LVDVD_DISC_INFO_T SrcDiscInfo,DstDiscInfo;
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
	memset(&TrackInfo,0,sizeof(CDR_TRACK_T));

	//判断光盘类型是否相同
	idisc1 = Xkd_DVDSDK_GetMediaBasicType(pDVDSrc);
	idisc2 = Xkd_DVDSDK_GetMediaBasicType(pDVDDst);

	if(( idisc1 > CD_DISC )||( idisc1 < B_DVD_DISC ))
	{
		DPERROR(("srcdisc type unsupport or no disc\n"));
		return idisc1; //不是光盘类型就返回其错误值
	}

	if(( idisc2 > CD_DISC )||( idisc2 < B_DVD_DISC ))
	{
		DPERROR(("dstdisc type unsupport or no disc\n"));
		return idisc2; //不是光盘类型就返回其错误值
	}

	DP(("idisc1=%d,idisc2=%d\n", idisc1, idisc2));
	//if(idisc1 != idisc2)
	//	return ERROR_DVD_DISCDIFFTYPE;

    //判断原盘是否为空盘
	if( (iRet = pDVDDst->pCmd->cdr_getdiscinfo(pDVDSrc->fd, &TrickCount, &SessionCount, &Capicity, &DiskStatus) ) )
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

	//判断目标盘是否为空盘
	if( (iRet = pDVDDst->pCmd->cdr_getdiscinfo(pDVDDst->fd, &TrickCount, &SessionCount, &Capicity, &DiskStatus) ) )
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

	//容量判断原盘大于目标盘则不能复制
	if( idisc2 > idisc1 )
	{
		DPERROR(("DstDisc is small!\n"));
		return ERROR_DVD_CANTCOPYDISC;
	}

	//蓝光盘有25g和50g的要判断容量
	Xkd_DVDSDK_GetDiscInfo(pDVDSrc, &SrcDiscInfo);
	Xkd_DVDSDK_GetDiscInfo(pDVDDst, &DstDiscInfo);
	DP(("SrcDiscInfo.discsize=%d,DstDiscInfo.discsize=%d\n",SrcDiscInfo.discsize,DstDiscInfo.discsize));
	if(SrcDiscInfo.discsize > DstDiscInfo.discsize)
	{
		DPERROR(("DstDisc size is small!\n"));
		return ERROR_DVD_CANTCOPYDISC;
	}

	Xkd_DVDSDK_SetCopySpeed(pDVDSrc, pDVDDst, idisc1, idisc2);

	/*//获取uuid
	uuid_generate(TempUUID);
	for(i=0;i<16;i++)
	{
		sprintf(TempStr,"%02X",TempUUID[i]);
		memcpy(&Buffer[i*2],TempStr,2);
	}*/

	//获取第二光盘的轨道信息
	i = 0;
	pDVDDst->pCmd->cdr_gettrackinfo(pDVDDst->fd, 1, &TrackInfo);
	TrackInfo.pbuffer = (uint8_t*)malloc(PACKET32_SIZE * 4);
	//TrackInfo.pbuffer = (uint8_t*)malloc(PACKET32_SIZE);
	TrackInfo.buffsize = 0;
	while(!pDVDSrc->pCmd->cdr_readtrack(pDVDSrc->fd, i, buf, PACKET32_SIZE))
	{
		if( UDF_UUID_ADDR == i )
		{
			memset( buf, 0, PACKET32_SIZE);
			//memcpy( buf, Buffer, 32);
		}
		pDVDDst->pCmd->cdr_writetrack(pDVDDst->fd, &TrackInfo, buf, PACKET32_SIZE);
		memset(buf, 0, PACKET32_SIZE);
		i += PACKET_BLOCK_32;
	}
	//当读轨道出错时 说明数据读完 会跳出循环 封轨道扇区
	DP(("data is over!\n"));
	iRet = pDVDDst->pCmd->cdr_closetrack(pDVDDst->fd, &TrackInfo);
	if(iRet)
	{
		DPERROR(("close file track error:%d\n", iRet));
	}

	iRet = pDVDDst->pCmd->cdr_closesession(pDVDDst->fd, &TrackInfo);
	if(iRet)
	{
		DPERROR(("close file session error:%d\n", iRet));
	}

	pDVDSrc->pCmd->cdr_unlockdoor(pDVDSrc->fd);
	pDVDDst->pCmd->cdr_unlockdoor(pDVDDst->fd);

	free(TrackInfo.pbuffer);
	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_ResumeDisc
* 描述  : 光盘恢复,如果在刻录过程中断电，调用该函数，恢复文件系统
* 参数  :
	nDevNo : 设备号，0-n
	DiscName : 光盘名称
	DirName : 目录名称
	FileName : 文件名称
	FillSize : 填充的数据数(默认为64M)
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
* 修改  : 2010.12.30 modify by yanming for Complete Func
*******************************************************************************/
int Xkd_DVDSDK_ResumeDisc(XKD_DVDDRV hDVD, char *DiscName, char *DirName, char *FileName, int FillSize)
{
	int iRet;
	uint64_t Capicity;
	//unsigned char TempUUID[8];
	unsigned int nEmptySize = 0;
	int TrickCount, SessionCount, DiskStatus;
	DISC_VOLID_T disc_volid;
	Xkd_DVDSDK_DIR  pDir;
	Xkd_DVDSDK_FILE pNode;
	CDR_TRACK_T stTrack;

	HANDLE_DEF;
	VALID_HANDLE();
	DVD_INRECORD();

	//先判断目标盘是否为空盘
	if( (iRet = HCDR_CMD()->cdr_getdiscinfo(HDEV_FD(), &TrickCount, &SessionCount, &Capicity, &DiskStatus) ) )
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

	//一个轨道 无法恢复
	if(2 != TrickCount)
	{
		DPERROR(("Disc can't resume!\n"));
		return ERROR_DVD_CANTRESUMEDISC;
	}

	//判断轨道1是否已被写入数据
	HUDF_HANLE()->cdr_cmd->cdr_gettrackinfo(HDEV_FD(), 1, &stTrack);
	DP(("1stTrack.freeblocks=%d\n",stTrack.freeblocks));
	if( 1024 != stTrack.freeblocks )
	{
		DPERROR(("Disc can't resume!\n"));
		return ERROR_DVD_CANTRESUMEDISC;
	}

	//判断轨道2是否封闭
	HUDF_HANLE()->cdr_cmd->cdr_gettrackinfo(HDEV_FD(), 2, &stTrack);
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
// 	strcpy(disc_volid.LVInfoDataTime,     "2010-12-17");
// 	strcpy(disc_volid.LVInfoEmail,        "xkd_work");
// 	strcpy(disc_volid.fileSetIdent,       "xkd_DVD_1.0");
// 	strcpy(disc_volid.copyrightFileIdent, "xkd_DVD_1.0");
// 	strcpy(disc_volid.abstractFileIdent,  "xkd_DVD_1.0");
	strcpy(disc_volid.LVInfoTitle, 	      "DVDRECORD");
	strcpy(disc_volid.LVInfoDataTime,     "2011-12-17");
	strcpy(disc_volid.LVInfoEmail,        "DVDRECORD");
	strcpy(disc_volid.fileSetIdent,       "DVDRECORD");
	strcpy(disc_volid.copyrightFileIdent, "DVDRECORD");
	strcpy(disc_volid.abstractFileIdent,  "DVDRECORD");


	// 产生UUID
	//uuid_generate_8bytes(TempUUID);
	memset(HUDF_HANLE()->m_CdRwDiskinfo->ExtenData, 0, PACKET16_SIZE);
	//memcpy(HUDF_HANLE()->m_CdRwDiskinfo->ExtenData, TempUUID, 8);

	HUDF_HANLE()->m_CdRwDiskinfo->nDiskCapicity = Capicity;

	//创建根目录
	HUDF_HANLE()->udf_cmd->addnode(HUDF_HANLE()->m_hMem, HUDF_HANLE()->m_FileDirTree, NULL, "", 0, NODETYPE_DIR);

	//初始化文件系统
	iRet = HUDF_HANLE()->udf_cmd->InitUdfFs(HUDF_HANLE(), &disc_volid);
	if(iRet)
	{
		DPERROR(("init udf fs error:%d\n", iRet));
		HCDR_CMD()->cdr_unlockdoor(HDEV_FD());
		return ERROR_DVD_FORMATFAILED;
	}

	DP(("trackID1=%d\n", HUDF_HANLE()->m_CdRwDiskinfo->udfsys.trackid));
	DP(("trackID2=%d\n", HUDF_HANLE()->m_CdRwDiskinfo->udffile.trackid));

	//获取轨道参数
	HCDR_CMD()->cdr_gettrackinfo(HDEV_FD(),
			HUDF_HANLE()->m_CdRwDiskinfo->udfsys.trackid, &(HUDF_HANLE()->m_CdRwDiskinfo->udfsys));

	HCDR_CMD()->cdr_gettrackinfo(HDEV_FD(),
			HUDF_HANLE()->m_CdRwDiskinfo->udffile.trackid, &(HUDF_HANLE()->m_CdRwDiskinfo->udffile));

	//创建目录和文件
	pDir  = Xkd_DVDSDK_CreateDir(hDVD, DirName);
	pNode = Xkd_DVDSDK_CreateFile(hDVD, pDir, FileName, 0);

	if( NULL == pNode )
	{
		LvDVDRec_UdfTreeFree(HUDF_HANLE());
		DPERROR(("filename is exist!\n"));
		return ERROR_DVD_NAMEEXIST;
	}

	//设定文件起始 (默认填充为1024 * 1024 * 64)
	nEmptySize = FillSize ? FillSize : (1024 * 1024 * 64);
	nEmptySize &= ~(PACKET32_SIZE - 1);	// 64K对其

	((FILDIRNODE *)(pNode))->FileLoca = HUDF_HANLE()->m_CdRwDiskinfo->udffile.writestart
		                                        - UDF_SYS_LEN + (nEmptySize/CDROM_BLOCK);
	((FILDIRNODE *)(pNode))->FileSize = (HUDF_HANLE()->m_CdRwDiskinfo->udffile.writedsize - 1040 - PACKET_BLOCK_32)*CDROM_BLOCK
		               					 - nEmptySize;           //1040第二轨道起始
	((HDVD_DEV_T*)hDVD)->bRecording = TRUE;
	//写入文件系统 关闭轨道扇区
	Xkd_DVDSDK_CloseDisc((HDVD_DEV_T*)hDVD);

	return 0;
}


/*******************************************************************************
* 名称  : Xkd_DVDSDK_GetReserveData
* 描述  : 获得保留轨道数据, 对于已经封盘的光盘才有效, 读取时调用
* 参数  :
	nDevNo : 设备号，0-n
	pBuffer: 返回保留数据指针
	pSize  : 返回Buffer长度
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_GetReserveData(XKD_DVDDRV hDVD, unsigned char **pBuffer, int *pSize)
{
	HANDLE_DEF;
	VALID_HANDLE();
	DVD_INRECORD();

	HCDR_CMD()->cdr_readtrack(HDEV_FD(),UDF_UUID_ADDR,
		     HUDF_HANLE()->m_CdRwDiskinfo->ExtenData,PACKET16_SIZE);

	*pBuffer = HUDF_HANLE()->m_CdRwDiskinfo->ExtenData;
	*pSize   = PACKET16_SIZE;
	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_GetReserveBuffer
* 描述  : 获得保留轨道数据指针, 这个函数在保留轨道前调用，直接修改Buffer，在
		  封盘时写入该数据,大小 32K， 刻录时调用
* 参数  :
	nDevNo : 设备号，0-n
	pBuffer: 返回保留数据指针
	pSize  : 返回Buffer长度
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_GetReserveBuffer(XKD_DVDDRV hDVD, unsigned char **pBuffer, int *pSize)
{
	HANDLE_DEF;
	VALID_HANDLE();

	*pBuffer = HUDF_HANLE()->m_CdRwDiskinfo->ExtenData;
	*pSize   = PACKET16_SIZE;
	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_GetTotalWriteSize
* 描述  : 获得光盘整个可写空间
* 参数  :
	nDevNo    : 设备号，0-n
	pTotalSize: 返回整个可写空间
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_GetTotalWriteSize(XKD_DVDDRV hDVD, unsigned long long *pTotalSize)
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
		iRet = HCDR_CMD()->cdr_getdiscinfo(HDEV_FD(), &TrickCount, &SessionCount, &uTotalSize, &DiskStatus);
		*pTotalSize = (uTotalSize<<11);
	}
	return iRet;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_GetFreeWriteSize
* 描述  : 获得剩余可写空间
* 参数  :
	nDevNo   : 设备号，0-n
	pFreeSize: 返回剩余可写空间
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int Xkd_DVDSDK_GetFreeWriteSize(XKD_DVDDRV hDVD, unsigned long long *pFreeSize)
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
		iRet = HCDR_CMD()->cdr_getdiscinfo(HDEV_FD(), &TrickCount, &SessionCount, &uTotalSize, &DiskStatus);
		*pFreeSize = (uTotalSize<<11);
	}
	return iRet;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_PrintProfile
* 描述  : 打印profile
* 参数  :
	nDevNo   : 设备号，0-n
* 返回值: 0: 成功
* 作者  : xkd
* 日期  : 2011.1.14
*******************************************************************************/
int Xkd_DVDSDK_PrintProfile(XKD_DVDDRV hDVD)
{
	HANDLE_DEF;
	VALID_HANDLE();
	HCDR_CMD()->cdr_dpdevprofile(HDEV_FD());

	return 0;
}

/*******************************************************************************
* 名称  : Xkd_DVDSDK_SetRecordStatus
* 描述  : 设置光驱刻录状态（用于光盘格式化后刻录失败未封盘，再次格式化前设置刻录状态）

* 参数  :
	hDVD   : DVD驱动句柄
* 返回值: 
* 作者  : hexj
* 日期  : 2014.8.5
*******************************************************************************/
void Xkd_DVDSDK_SetRecordStatus(XKD_DVDDRV hDVD, BOOL bRecordStatus)
{
	HANDLE_DEF;
    pDVD->bRecording = bRecordStatus;
}
#if 1
/*******************************************************************************
* 名称  : LvDVDRec_udffstest
* 描述  : UDF测试用
* 参数  :
* 返回值: 0: 成功，其他为错误值
* 作者  : xkd
* 日期  : 2017.3.31
*******************************************************************************/
int LvDVDRec_udffstest(void)
{
	DP(("LvDVDRec_udffstest ...start!\n"));
	udfinfo_t  *pUdf = LvDVDRec_UdfCreate(-1, UDFDATAMODE_DATA);
	if(!pUdf)
	{
		DPERROR(("LvDVDRec_UdfCreate error\n"));
		return -1;
	}
	DP(("LvDVDRec_udffstest ...start1!\n"));
	LvDVDRec_GetCdrcmd(NULL, &pUdf->cdr_cmd);
	DP(("LvDVDRec_udffstest ...start2!\n"));
	pUdf->udf_cmd->udffstest(pUdf);
	DP(("LvDVDRec_udffstest ...start3!\n"));
	LvDVDRec_UdfFree(pUdf);
	DP(("LvDVDRec_udffstest ...end!\n"));
	return 0;
}


#endif

