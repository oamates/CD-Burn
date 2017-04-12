//cdr_cmd.c
#include <limits.h>
#include  <errno.h>
#include "debug.h"
#include "LibDVDSDK.h"
#include "drvcomm.h"
#include "cdr_cmd.h"
#include <time.h>
#ifdef LINUX
#include <sys/time.h>
#endif

#define WRITE_TRYNUMS	400

#define INIT_CGC()	struct cdrom_generic_command cgc; \
					struct request_sense sense; \
					memset(&cgc, 0, sizeof(struct cdrom_generic_command)); \
					cgc.sense = &sense;

#if !defined(CSLEEP)
	#define CSLEEP(ms) {struct timespec tv = {ms / 1000, (ms % 1000) * 1000000};  nanosleep(&tv, 0);}
#endif//CSLEEP

// 函数声明
static int cdr_closetray(int fd);

// 打印命令调试信息
#if defined(DEBUG)
static void DP_MMCCMD(char *errname, struct cdrom_generic_command *cgc, int line)
{
	int i;

	DP(("[%s] MMC CMD error: line:%d ", errname, line));

	for (i=0; i<12; i++)
	{
		DP(("%02x ", cgc->cmd[i]));
	}
	
	if (cgc->sense)
	{
		DP(("- sense %02x.%02x.%02x\n", cgc->sense->sense_key, cgc->sense->asc, cgc->sense->ascq));
	}
	else
	{
		DP(("\n"));
	}
}
#else
	#define DP_MMCCMD(_errname, _cgc, _line)
#endif//DEBUG

// 发送命令
static int send_cmd(int fd, struct cdrom_generic_command *cgc, uint8_t *buf, int dir, int timeout)
{
	int ret;
	
	cgc->buffer         = buf;
	cgc->data_direction = dir;
	cgc->timeout        = timeout;
	
	cgc->timeout = cgc->timeout <= 0 ? 500 : cgc->timeout;
	
	ret = ioctl(fd, CDROM_SEND_PACKET, cgc);
	
	return ret;
}

//从2字节转换为short整型
static uint16_t from2Byte( uint8_t * Data)
{
    return ( ((Data[0] << 8) & 0xFF00) | (Data[1] & 0xFF) );
}

// 判断光驱是否空闲
static int inline testUnitReady(int fd)
{
	INIT_CGC();
    cgc.cmd[0] = MMCCMD_TEST_UNIT_READY;
	return send_cmd(fd, &cgc, NULL, CGC_DATA_NONE, WAIT_BLANK * 100);
}

// 等待准备好
static BOOL wait_unit_ready(int fd, int secs)
{
	int	iRet;
	while(secs == -1 || secs > 0)
	{
		secs -= secs > 0 ? 1 : 0;
		iRet = testUnitReady(fd);
		if(iRet < 0)
		{
			sleep(1);
			continue;
		}
		else
		{
			return TRUE;
		}
	}
	return FALSE;
}

// 判断是否有光盘
static BOOL testHaveDisc(int fd)
{
	int ret, attempts = 3;

	do{
	    // 获取当前光驱状态
		ret = ioctl(fd, CDROM_DRIVE_STATUS, CDSL_CURRENT);
		if (ret == CDS_TRAY_OPEN)
		{
			cdr_closetray(fd);
			continue;
		}
		else if (ret == CDS_DISC_OK)
		{
			return TRUE;
		}
		else if( ret == CDS_NO_DISC)
		{
			return FALSE;
		}
		else if( ret == CDS_DRIVE_NOT_READY )
		{
			sleep(1);
		}
	}while(--attempts);
	
	return FALSE;
}

/*
static BOOL GetMediaFeature(int fd, int *profile)
{
	INIT_CGC();
    
	unsigned char header[2048];
	memset( header, 0, 2048 );
	
	cgc.cmd[0] = MMCCMD_GET_CONFIGURATION;
	cgc.cmd[1] = 1;
	cgc.cmd[8] = 8;
	cgc.cmd[9] = 0;	  // Necessary to set the proper command length
	cgc.buflen = 8;
		 
	if( !send_cmd(fd, &cgc, header, CGC_DATA_READ, WAIT_BLANK) )
	{
		*profile = from2Byte( &header[6]);
		return TRUE;
	}
	else
	{
		DP_MMCCMD("CONFIGURATION", &cgc, __LINE__);
		return FALSE;
	}
}
*/

//获取设备profile
static int GetDevProfile(int fd,unsigned char * profile)
{
	int len = 8;
	
	INIT_CGC();

	//第一次获取长度
	cgc.cmd[0] = GPCMD_GET_CONFIGURATION;
	cgc.cmd[1] = 1;
	cgc.cmd[8] = len;
	cgc.buflen = len;

	send_cmd(fd, &cgc, profile, CGC_DATA_READ, WAIT_BLANK);

	//第二次获取数据
	len = (profile[3] | (profile[2] << 8) | (profile[1] << 16) | (profile[0] << 24));

	cgc.cmd[0] = GPCMD_GET_CONFIGURATION;
	cgc.cmd[1] = 1;
	cgc.cmd[8] = len;
	cgc.buflen = len;	
	
	send_cmd(fd, &cgc, profile, CGC_DATA_READ, WAIT_BLANK);
	return TRUE;
}

// 获得光盘信息
static BOOL GetDiscInfo(int fd, disc_info_t *di)
{
	int ret;
	INIT_CGC();
	
	memset(di, 0, sizeof(disc_info_t));
	cgc.cmd[0] = GPCMD_READ_DISC_INFO;
	cgc.cmd[8] = cgc.buflen = 2;

	wait_unit_ready(fd, 120);
	
	if ((ret = send_cmd(fd, &cgc, (unsigned char *)di, CGC_DATA_READ, WAIT_BLANK * 100 )) < 0)
	{
		DP_MMCCMD("get disc info", &cgc, __LINE__);
		return FALSE;
	}
	cgc.buflen = be16_to_cpu(di->length) + sizeof(di->length);

	if (cgc.buflen > sizeof(disc_info_t))
		cgc.buflen = sizeof(disc_info_t);

	cgc.cmd[8] = cgc.buflen;
	if ((ret = send_cmd(fd, &cgc, (unsigned char *)di, CGC_DATA_READ, WAIT_BLANK * 100 )) < 0)
	{
		DP_MMCCMD("get disc info", &cgc, __LINE__);
		return FALSE;
	}
	return TRUE;
}

//保留轨道,保留blocks个块
static BOOL reserve_track(int fd, int blocks)
{
	INIT_CGC();
	int ret;

	memset(&cgc, 0, sizeof(cgc));
	cgc.cmd[0] = GPCMD_RESERVE_RZONE_TRACK;
	cgc.cmd[5] = ((blocks >> 24) & 0xff);
	cgc.cmd[6] = ((blocks >> 16) & 0xff);
	cgc.cmd[7] = ((blocks >>  8) & 0xff);
	cgc.cmd[8] = (blocks & 0xff);

	wait_unit_ready(fd, 120);
	//while( testUnitReady(fd) != 0 ) sleep(1);
	
	if ((ret = send_cmd(fd, &cgc, NULL, CGC_DATA_NONE, WAIT_BLANK2  )) < 0)
	{
		DP_MMCCMD("reserve_track", &cgc, __LINE__);
		return FALSE;
	}
	return TRUE;
}

//读取轨道信息
static BOOL read_track_info(int fd, int trackno, track_info_t *ti)
{
	INIT_CGC();
	
	memset(ti, 0, sizeof(track_info_t));
	cgc.cmd[0] = GPCMD_READ_TRACK_RZONE_INFO;
	cgc.cmd[1] = 1;
	cgc.cmd[5] = trackno;
	cgc.cmd[8] = cgc.buflen = 28;

	wait_unit_ready(fd, 120);
	
	if ( send_cmd(fd, &cgc, (uint8_t *) ti, CGC_DATA_READ, WAIT_BLANK * 100 ))
	{
		DP_MMCCMD("read_track_info", &cgc, __LINE__);
		return FALSE;
	}
	return TRUE;
}

//关闭轨道
static int close_track(int fd, int track)
{
	INIT_CGC();
	
	cgc.cmd[0] = MMCCMD_CLOSE_TRACK_SESSION;
	cgc.cmd[1] = 0;
	cgc.cmd[2] = 1; /* bit 2 is close session/border */
	cgc.cmd[4] = ((track >> 8) & 0xff);
	cgc.cmd[5] = (track & 0xff);

	wait_unit_ready(fd, 120);

	if (send_cmd(fd, &cgc, NULL, CGC_DATA_NONE, WAIT_BLANK ) )
	{
		DP_MMCCMD("close_track", &cgc, __LINE__);
		return ERROR_DVD_OPTERFAILED;
	}
	return 0;
}

//关闭会话Session,关闭区段，如果关闭最后一个区段，最后一个轨道，则自动封盘，一般轨道数据量要超过800M才可以顺利封闭
//因此如果数据量不够800M最好，填充一些空白数据，超过欧800M，然后再封盘就比较容易了
static int close_session(int fd, int track)
{
	INIT_CGC();
	
	cgc.cmd[0] = MMCCMD_CLOSE_TRACK_SESSION;
	cgc.cmd[1] = 0;
	cgc.cmd[2] = 2;	
	cgc.cmd[4] = ((track >> 8) & 0xff);
	cgc.cmd[5] = (track & 0xff);

	wait_unit_ready(fd, 120);

	if ( send_cmd(fd, &cgc, NULL, CGC_DATA_NONE, WAIT_MAXTIME) )
	{
		DP_MMCCMD("close_session", &cgc, __LINE__);
        DP(("close_session error,fd=%d\n",fd));
		return ERROR_DVD_OPTERFAILED;
	}
	return 0;
}

//确认数据已经同步到刻录机缓冲区
static int sync_cache(int fd)
{
	struct cdrom_generic_command cgc;
	
	memset(&cgc, 0, sizeof(cgc));
	cgc.cmd[0] = 0x35;
	cgc.cmd[1] = 2;
	return send_cmd(fd, &cgc, NULL, CGC_DATA_NONE, WAIT_SYNC);
}

//static BOOL writetrack_block(int fd, uint8_t *buffer, int lba, int blocks, int retrynums)
BOOL writetrack_block(int fd, uint8_t *buffer, int lba, int blocks, int retrynums)
{
	int iRet;
	int errcode;
    int iRetry = 0;
	INIT_CGC();
	
STARTWRITE:	
	memset(&cgc, 0, sizeof(cgc));
	cgc.sense = &sense;
	cgc.cmd[0] = GPCMD_WRITE_10;
	cgc.cmd[2] = ((lba >> 24) & 0xff);
	cgc.cmd[3] = ((lba >> 16) & 0xff);
	cgc.cmd[4] = ((lba >>  8) & 0xff);
	cgc.cmd[5] = (lba & 0xff);
	cgc.cmd[7] = ((blocks >> 8) & 0xff);
	cgc.cmd[8] = (blocks & 0xff);
	cgc.buflen = blocks * CDROM_BLOCK;
	
    struct timeval	tvStart;
    struct timeval	tvEnd;
    gettimeofday(&tvStart, NULL);
    struct tm 	*tm;
    tm = localtime(&tvStart.tv_sec);
    DP(("writetrack_block, send_cmd fd=%d ptrack->writenext:%d send_cmdtime:   %d:%d:%d %ld us\n", fd, lba, tm->tm_hour, tm->tm_min, tm->tm_sec, tvStart.tv_usec));
	iRet = send_cmd(fd, &cgc, buffer, CGC_DATA_WRITE, WAIT_SYNC * 2000 );
    gettimeofday(&tvEnd, NULL);
    DP(("writetrack_block after send_cmd, fd=%d ptrack->writenext:%d send_cmdlasttime:%ds %ldus \n", fd, lba,(tvEnd.tv_sec - tvStart.tv_sec), tvEnd.tv_usec - tvStart.tv_usec));

	if( iRet != 0 && retrynums > 0)
	{
        DPERROR(("writetrack_block iRet=%d fd=%d cgc.sense->sense_key:0x%X cgc.sense->asc:0x%X cgc.sense->ascq:0x%X \n",iRet ,fd, cgc.sense->sense_key, cgc.sense->asc , cgc.sense->ascq ));
		errcode = ((cgc.sense->sense_key << 16) | (cgc.sense->asc << 8) | cgc.sense->ascq);
		if(  errcode == 0X020408 || errcode == 0X020407 || errcode == 0X020404 )
		{
			if(retrynums < 5)
			{
				CSLEEP(2);
			}
			else if(retrynums < 20)
			{
				CSLEEP(10);
			}
			else if(retrynums < 100)
			{
				CSLEEP(20);
			}
			else
			{
				CSLEEP(50);
			}
			retrynums --;
			goto STARTWRITE;
		}
        else
        {
            if (iRetry < 3)
            {
                iRetry += 1;
                DPERROR(("writetrack_block tryCount = %d iRet=%d fd=%d cgc.sense->sense_key:0x%X cgc.sense->asc:0x%X cgc.sense->ascq:0x%X \n",iRetry,iRet ,fd, cgc.sense->sense_key, cgc.sense->asc , cgc.sense->ascq ));
                CSLEEP(500);
                goto STARTWRITE;
            }
        }
		DPERROR(("writetrack_block:%d errno:%d error:%s errcode:0x%X \n",iRet ,errno, strerror(errno) , errcode ));
	}
	

	return iRet == 0;
/*
	for(i=0; i< retrynums; i++)
	{
		memset(&cgc, 0, sizeof(struct cdrom_generic_command));
		cgc.sense = &sense;
#if 0
		cgc.cmd[0] = GPCMD_WRITE_10;
		//cgc.cmd[1] = (1 << 1) | cgc.cmd[1];
		//cgc.cmd[1] = (1 << 3) | cgc.cmd[1];
		cgc.cmd[2] = ((lba >> 24) & 0xff);
		cgc.cmd[3] = ((lba >> 16) & 0xff);
		cgc.cmd[4] = ((lba >>  8) & 0xff);
		cgc.cmd[5] = (lba & 0xff);
		cgc.cmd[7] = ((blocks >> 8) & 0xff);
		cgc.cmd[8] = (blocks & 0xff);
		cgc.buflen = (blocks << BLOCK_OFFSETBITS); //块与扇区的对齐blocks * 2048 字节
#else
		cgc.cmd[0]  = 0xAA;    //write(12)
		//cgc.cmd[0] = 0x08;    //FUA
		cgc.cmd[2]  = ((lba >> 24) & 0xff);
		cgc.cmd[3]  = ((lba >> 16) & 0xff);
		cgc.cmd[4]  = ((lba >>  8) & 0xff);
		cgc.cmd[5]  = (lba & 0xff);
		cgc.cmd[6]  = ((blocks >> 24) & 0xff);
		cgc.cmd[7]  = ((blocks >> 16) & 0xff);
		cgc.cmd[8]  = ((blocks >> 8) & 0xff);
		cgc.cmd[9]  = (blocks & 0xff);
		cgc.cmd[10] = 0x80;     //streaming
		cgc.buflen  = (blocks << BLOCK_OFFSETBITS); //块与扇区的对齐blocks * 2048 字节
#endif
		iRet = send_cmd(fd, &cgc, buffer, CGC_DATA_WRITE, WAIT_SYNC * 200);
		if(iRet == 0)
		{
			sync_cache(fd);
#if defined(DEBUG)
			if(i > 0)
			{
				DP(("writetrack_block[fd=%d] nums : %d\n", fd, i));
			}
#endif//DEBUG
			return TRUE;
		}
		errcode = ((cgc.sense->sense_key << 16) | (cgc.sense->asc << 8) | cgc.sense->ascq);
		if(  errcode == 0X020408 || errcode == 0X020407 || errcode == 0X020404 )
		{
			if(i < 5)
			{
				CSLEEP(2);
			}
			else if(i < 20)
			{
				CSLEEP(10);
			}
			else if(i < 100)
			{
				CSLEEP(20);
			}
			else
			{
				CSLEEP(50);
			}
			continue;
		}
		break;
	}
	DPERROR(("writetrack_block[fd=%d] nums: %d\n", fd, i));
	DP_MMCCMD("writetrack_block", &cgc, __LINE__);
	*/
	
	//return FALSE;
}

static int FindDiscType(char *pbuf,unsigned char * disclist,int * j)
{
	switch(*pbuf)
	{
		case PROFILE_HD_DVD_RAM:
		{
			disclist[(*j)++] = DISC_HDVD_RAM;
			return DISC_HDVD_RAM;
		}
		case PROFILE_HD_DVD_R:
		{
			disclist[(*j)++] = DISC_HDVD_R;
			return DISC_HDVD_R;
		}
		case PROFILE_HD_DVD_ROM:
		{
			disclist[(*j)++] = DISC_HDVD_ROM;
			return DISC_HDVD_ROM;
		}
		case PROFILE_BD_RE:
		{
			disclist[(*j)++] = DISC_BD_RE;
			return DISC_BD_RE;
		}
		case PROFILE_BD_R2:
		{
			disclist[(*j)++] = DISC_BD_R;
			return DISC_UNKNOWN;
		}
		case PROFILE_BD_R1:
		{				
			disclist[(*j)++] = DISC_BD_R_SEQ;
			//disclist[(*j)++] = DISC_BD_R_SEQ;			//不确定 暂且按此
			return DISC_BD_R;
		}
		case PROFILE_BD_ROM:
		{				
			disclist[(*j)++] = DISC_BD_ROM;
			return DISC_BD_ROM;
		}
		case PROFILE_DVD_R_P_D:	
		{				
			disclist[(*j)++] = DISC_DVD_PLUS_R_DL;
			disclist[(*j)++] = DISC_DVD_PLUS_RW_DL;
			return DISC_DVD_PLUS_R_DL;
		}
		case PROFILE_DVD_R_P:
		{
			disclist[(*j)++] = DISC_DVD_PLUS_R;
			return DISC_DVD_PLUS_R;
		}
		case PROFILE_DVD_RW_P:
		{
			disclist[(*j)++] = DISC_DVD_PLUS_RW;
			return DISC_DVD_PLUS_RW;
		}
		case PROFILE_DVD_D:
		{
			disclist[(*j)++] = DISC_UNKNOWN;		
			return DISC_UNKNOWN;	
		}
		case PROFILE_DVD_R_D:
		{
			disclist[(*j)++] = DISC_DVD_R_DL;
			disclist[(*j)++] = DISC_DVD_R_DL_LJ;	
			return DISC_DVD_R_DL_LJ;			
		}			
		case PROFILE_DVD_R_D1: 
		{
			disclist[(*j)++] = DISC_DVD_R_DL_SEQ;
			return DISC_DVD_R_DL_SEQ;			
		}
		case PROFILE_DVD_RW2:
		{
			disclist[(*j)++] = DISC_DVD_RW;
			disclist[(*j)++] = DISC_DVD_RW_SEQ;
			return DISC_DVD_RW;
		}
		case PROFILE_DVD_RW1:
		{
			disclist[(*j)++] = DISC_DVD_RW_DL;
			return DISC_DVD_RW_DL;
		}
		case PROFILE_DVD_RAM:
		{
			disclist[(*j)++] = DISC_DVD_RAM;
			return DISC_DVD_RAM;
		}
		case PROFILE_DVD_R:
		{
			disclist[(*j)++] = DISC_DVD_R;
			return DISC_DVD_R;
		}
		case PROFILE_DVD_ROM:
		{
			disclist[(*j)++] = DISC_DVD_ROM;
			return DISC_DVD_ROM;
		}
		case PROFILE_CD_RW:
		{
			disclist[(*j)++] = DISC_CD_RW;
			return DISC_CD_RW;
		}
		case PROFILE_CD_R:
		{
			disclist[(*j)++] = DISC_CD_R;
			return DISC_CD_R;
		}
		case PROFILE_CD_ROM:
		{
			disclist[(*j)++] = DISC_CD_ROM;
			return DISC_CD_ROM;
		}
		case PROFILE_REMOVABLE:
		{
			disclist[(*j)++] = DISC_UNKNOWN;
			return DISC_UNKNOWN;
		}
		case PROFILE_NO_CURRENT:
		{
			disclist[(*j)++] = DISC_UNKNOWN;
			return DISC_UNKNOWN;
		}		
		default:
			return 0;
	}

}

static CCDRCmd cdr_dvd;

int DVDRec_GetCdrcmd(char *pCdrName, CCDRCmd *pCmd)
{
	pCmd = &cdr_dvd;
	return 0;
}

/*************************** class CDRCmd ***************************************/
// 锁定托盘
BOOL CCDRCmd::CDR_LockDoor(int fd)
{
	int ret;

	if ((ret = ioctl(fd, CDROM_CLEAR_OPTIONS, CDO_LOCK)) < 0)
		return ret;

	ret = ioctl(fd, CDROM_LOCKDOOR, 1);
#if defined(DEBUG)
	if (ret < 0)
	{
		DPERROR(("can't lock door\n"));
	}
#endif//DEBUG
	return ret < 0 ? FALSE : TRUE;
}

// 解锁托盘
BOOL CCDRCmd::CDR_UnlockDoor(int fd)
{
	int ret = ioctl(fd, CDROM_LOCKDOOR, 0);
#if defined(DEBUG)
	if (ret < 0)
	{
		DPERROR(("can't unlock door\n"));
	}
#endif//DEBUG

	ioctl(fd, CDROM_SET_OPTIONS, CDO_LOCK);

	return ret < 0 ? FALSE : TRUE;
}

// 打开托盘
BOOL CCDRCmd::CDR_OpenTray(int fd)
{
#if 0	
	int ret;

	ret = ioctl(fd, CDROMEJECT, 0);
#if defined(DEBUG)
	if (ret)
	{
		DPERROR(("cdr_opentray error:%d\n", ret));
	}
#endif//DEBUG
	return ret ? FALSE : TRUE;
#endif
	INIT_CGC();

	cgc.cmd[0] = MMCCMD_PREVENT_ALLOW_MEDIUM_REMOVAL;
	cgc.cmd[5] = 0; 		// Necessary to set the proper command length
	send_cmd(fd, &cgc, NULL, CGC_DATA_NONE, WAIT_BLANK);

	cgc.cmd[0] = MMCCMD_START_STOP_UNIT;
	cgc.cmd[1] = 1;
	cgc.cmd[5] = 0; 		// Necessary to set the proper command length
	cgc.cmd[4] = 0x2;      // eject medium LoEj = 1, Start = 0
	return send_cmd(fd, &cgc, NULL, CGC_DATA_NONE, WAIT_BLANK);

}

// 关闭托盘
BOOL CCDRCmd::CDR_CloseTray(int fd)
{
#if 0		
	int ret;

	ret = ioctl(fd, CDROMCLOSETRAY);
#if defined(DEBUG)
	if (ret)
	{
		DPERROR(("cdr_closetray error:%d\n", ret));
	}
#endif//DEBUG
	return ret ? FALSE : TRUE;
#endif
	INIT_CGC();

	cgc.cmd[0] = MMCCMD_PREVENT_ALLOW_MEDIUM_REMOVAL;
	cgc.cmd[5] = 0; 		// Necessary to set the proper command length
	send_cmd(fd, &cgc, NULL, CGC_DATA_NONE, WAIT_BLANK);

	//INIT_CGC();
	cgc.cmd[0] = MMCCMD_START_STOP_UNIT;
	cgc.cmd[1] = 1;
	cgc.cmd[5] = 0; 		// Necessary to set the proper command length
	cgc.cmd[4] = 0x3;	   // eject medium LoEj = 1, Start = 0
	return send_cmd(fd, &cgc, NULL, CGC_DATA_NONE, WAIT_BLANK);

}

BOOL CCDRCmd::CDR_HaveDisc(int fd)
{
	return testHaveDisc(fd);
}

// 获得轨道信息
int CCDRCmd::CDR_GetTrackinfo(int fd, int nTrackID , stCDRTrack *pTrack)
{
	track_info_t trick;

	if (!read_track_info(fd, nTrackID , &trick))
	{
		DPERROR(("read_track_info error\n"));
		return ERROR_DVD_OPTERFAILED;
	}
	pTrack->sessionid = (trick.session_number_m << 8) | trick.session_number_l;
	pTrack->trackmode = trick.track_mode;  //轨道模式
	pTrack->datamode = trick.data_mode;   //数据模式		
	pTrack->trackid = nTrackID ;

	pTrack->tracksize = be32_to_cpu(trick.track_size);
	pTrack->freeblocks = be32_to_cpu(trick.free_blocks);
	pTrack->writestart = be32_to_cpu(trick.track_start);
	pTrack->writenext = be32_to_cpu(trick.next_writable);
	pTrack->writedsize = pTrack->writenext - pTrack->writestart;

	pTrack->bclosed = (pTrack->freeblocks == 0 && pTrack->writenext == 0) ? TRUE : FALSE;

	return 0;
}

// 格式化光盘
int CCDRCmd::CDR_FormatDisc(int fd, int nReserveBlocks)
{
	disc_info_t stdisc_info;
	stCDRTrack stTrack;
	int firstTrack, lastTrack, TrickCount;

	// 判断是否有光盘
	if (!testHaveDisc(fd))
	{
		return ERROR_DVD_NODISC;
	}
	// 加锁仓门
	CDR_LockDoor(fd);

	if (!GetDiscInfo(fd, &stdisc_info))
	{
		DPERROR(("GetDiscInfo is error!\n"));
		return ERROR_DVD_OPTERFAILED;
	}

	firstTrack = ((stdisc_info.first_track_m << 8) | stdisc_info.first_track_l);
	lastTrack = ((stdisc_info.last_track_m << 8) | stdisc_info.last_track_l);
	TrickCount = lastTrack;
	if (TrickCount >= 2 || stdisc_info.status == DISCSTE_INCOMPLETE)
	{
		DPERROR(("this disc is not blank disc, can be writed!\n")); // 不是空白光盘
		return ERROR_DVD_UNBLANKDISC;
	}

	if (stdisc_info.status != DISCSTE_BLANK)
	{
		DPERROR(("disc is not blank disc : %d\n", stdisc_info.status));
		return ERROR_DVD_UNBLANKDISC;
	}

	wait_unit_ready(fd, 120);

	if (!reserve_track(fd, nReserveBlocks))
	{
		DPERROR(("reserve_track failed!\n"));
		return ERROR_DVD_RESERVETRACKERR;
	}

	if (CDR_GetTrackinfo(fd, 1, &stTrack) != 0)
	{
		DPERROR(("gettrackinfo 1 error!\n"));
		return ERROR_DVD_OPTERFAILED;
	}
	else
	{
		DP(("ID=%d isClose=%d TrackMode=%d DataMode=%d start=%d Next=%d free=%d Recorded=%d Size=%d\n",
			1, stTrack.bclosed, stTrack.trackmode, stTrack.datamode, stTrack.writestart, stTrack.writenext,
			stTrack.freeblocks, stTrack.writedsize, stTrack.tracksize));
	}

	if (CDR_GetTrackinfo(fd, 2, &stTrack) != 0)
	{
		DPERROR(("gettrackinfo 2 error!\n"));
		return ERROR_DVD_OPTERFAILED;
	}
	else
	{
		DP(("ID=%d isClose=%d TrackMode=%d DataMode=%d start=%d Next=%d free=%d Recorded=%d Size=%d\n",
			2, stTrack.bclosed, stTrack.trackmode, stTrack.datamode, stTrack.writestart, stTrack.writenext,
			stTrack.freeblocks, stTrack.writedsize, stTrack.tracksize));
	}
	return 0;
}

// 获得光驱Buffer能力
int CCDRCmd::CDR_Buffcap(int fd, uint32_t *pBuffsize, uint32_t *pBufferFree)
{
	struct{
		unsigned int pad;
		unsigned int buffer_size;
		unsigned int buffer_free;
	} __attribute((packed))bufcap;

	INIT_CGC();

	int ret;

	memset(&bufcap, 0, sizeof(bufcap));
	cgc.cmd[0] = MMCCMD_READ_BUFFER_CAPACITY;
	cgc.cmd[8] = cgc.buflen = 12;
	cgc.cmd[9] = 0;

	wait_unit_ready(fd, 120);

	if ((ret = send_cmd(fd, &cgc, (uint8_t *)&bufcap, CGC_DATA_READ, WAIT_BLANK)))
	{
		DP_MMCCMD("get bufcap", &cgc, __LINE__);
		return ERROR_DVD_OPTERFAILED;
	}
	*pBuffsize = be32_to_cpu(bufcap.buffer_size);
	*pBufferFree = be32_to_cpu(bufcap.buffer_free);
	return 0;
}

int CCDRCmd::CDR_DiscExactType(int fd, int *pType)
{
	unsigned char Profile[512];
	unsigned char disclist;
	int j = 0;

	// 判断是否有光盘
	if (!testHaveDisc(fd))
	{
		return ERROR_DVD_NODISC;
	}

	wait_unit_ready(fd, 120);

	memset(Profile, 0, sizeof(Profile));
	if (!GetDevProfile(fd, Profile))
	{
		DPERROR(("GetDevProfile error\n"));
		return ERROR_DVD_OPTERFAILED;
	}
	//tmp = from2Byte( &Profile[6] );
	*pType = FindDiscType((char*)(&Profile[7]), (unsigned char*)&disclist, &j);
	return *pType;
}

// 获得光盘基本类型
int CCDRCmd::CDR_DiscBasicType(int fd, int *pType)
{
	unsigned char Profile[512];
	int tmp;

	// 判断是否有光盘
	if (!testHaveDisc(fd))
	{
		return ERROR_DVD_NODISC;
	}

	wait_unit_ready(fd, 120);

	memset(Profile, 0, sizeof(Profile));
	if (!GetDevProfile(fd, Profile))
	{
		DPERROR(("GetDevProfile error\n"));
		return ERROR_DVD_OPTERFAILED;
	}
	tmp = from2Byte(&Profile[6]);

	if ((tmp >= 0x08) && (tmp <= 0x0a))
	{
		*pType = CD_DISC;
		return CD_DISC;
	}
	else if (((tmp >= 0x10) && (tmp <= 0x14)) || (0x1A == tmp) || (0x1B == tmp))
	{
		*pType = DVD_DISC;
		return DVD_DISC;
	}
	else if (((tmp >= 0x15) && (tmp <= 0x16)) || (0x2B == tmp))
	{
		*pType = DVD_DL_DISC;
		return DVD_DL_DISC;
	}
	else if ((tmp >= 0x40) && (tmp <= 0x42))
	{
		*pType = B_DVD_DISC;
		return B_DVD_DISC;
	}
	DP(("Profile is %04x!\n", tmp));
	return ERROR_DVD_UNKNOWNDISCTYPE;
}

int CCDRCmd::CDR_GetDiscInfo(int fd, int *TrickCount, int *SessionCount, uint64_t *Capicity, int *DiskStatus)
{
	disc_info_t di;
	int CapicitySize = 0;
	int i, firstTrack, lastTrack;
	track_info_t trick;

	firstTrack = 0;
	lastTrack = 0;

	// 判断是否有光盘
	if (!testHaveDisc(fd))
	{
		return ERROR_DVD_NODISC;
	}

	if (!GetDiscInfo(fd, &di))
		return ERROR_DVD_OPTERFAILED;

	*DiskStatus = di.status;

	*SessionCount = (di.n_sessions_m << 8) | di.n_sessions_l;
	*Capicity = 0LL;

	firstTrack = (di.first_track_m << 8) | di.first_track_l;
	lastTrack = (di.last_track_m << 8) | di.last_track_l;
	*TrickCount = lastTrack;
	//读取每个轨道容量，累加出光盘总容量
	//for (i = firstTrack; i <= lastTrack; i++) 
	for (i = 1; i <= lastTrack; i++)
	{
		if (read_track_info(fd, i, &trick))
		{
			CapicitySize += be32_to_cpu(trick.track_size);
		}
	}
	*Capicity = (CapicitySize + PACKET_BLOCK_16 * (lastTrack - firstTrack)); //光盘总容量，每二个轨道之间会间隔16扇区轨道信息
	return 0;
}

// 获得光驱能力
int CCDRCmd::CDR_DevcapAblity(int fd, int *capablity)
{
	return 0;
}

int CCDRCmd::CDR_LoadMedia(int fd)
{
	INIT_CGC();

	// 判断是否有光盘
	if (!testHaveDisc(fd))
	{
		return ERROR_DVD_NODISC;
	}

	wait_unit_ready(fd, 120);

	cgc.cmd[0] = MMCCMD_START_STOP_UNIT;
	cgc.cmd[5] = 0; // Necessary to set the proper command length
	cgc.cmd[4] = 0x3;	   // eject medium LoEj = 1, Start = 0
	send_cmd(fd, &cgc, NULL, CGC_DATA_NONE, WAIT_BLANK);
	//	cdr_lockdoor(fd);
	return 0;
}

int CCDRCmd::CDR_GetdevInfo(int fd, DVD_DEV_INFO_T *pstDevInfo)
{
	unsigned char header[512];
	unsigned char *pbuf;
	int datalen, j, i, profilenum;

	INIT_CGC();
	profilenum = 0;
	memset(header, 0x00, sizeof(header));

	cgc.cmd[0] = GPCMD_INQUIRY;
	//cgc.cmd[1] = 1;
	cgc.cmd[2] = 0x00;
	cgc.cmd[3] = 0x00020 >> 8;
	cgc.cmd[4] = 0x00020 & 0xff;
	cgc.cmd[5] = 0;
	//cgc.cmd[6] = 0;
	//cgc.cmd[7] = 0x00ff >> 8;
	//cgc.cmd[8] = 0x00ff & 0xff;
	cgc.buflen = 0x020;

	send_cmd(fd, &cgc, header, CGC_DATA_READ, WAIT_BLANK);
	memcpy(pstDevInfo->szVender, &header[8], 28);

	memset(header, 0, sizeof(header));
	//获取profile
	GetDevProfile(fd, header);

	datalen = header[11];       //获取profile 数据
	profilenum = (datalen / 4); //profile 数量 数量可以判定是何种设备

	if (profilenum > 15)
	{
		pstDevInfo->drvtype = DVDDRIVER_BLUERW;
	}
	else if (profilenum == 15)
	{
		pstDevInfo->drvtype = DVDDRIVER_BLUER;
	}
	else if (profilenum > 4)
	{
		pstDevInfo->drvtype = DVDDRIVER_DVDRW;

	}
	else if (profilenum == 4)
	{
		pstDevInfo->drvtype = DVDDRIVER_DVDR;
	}
	else if (profilenum > 1)
	{
		pstDevInfo->drvtype = DVDDRIVER_CDRW;
	}
	else if (profilenum == 1)
	{
		pstDevInfo->drvtype = DVDDRIVER_CDR;
	}
	else
	{
		pstDevInfo->drvtype = DVDDRIVER_UNKNOWN;
	}

	j = 0;
	pbuf = header + 13;   //13开始为profile数据区
	for (i = 0; i < (datalen / 4); i++)
	{
		FindDiscType((char*)pbuf, pstDevInfo->disclist, &j);
		pbuf += 4;
	}

	pstDevInfo->discsupts = j;

	return 0;
}

// 读轨道数据
int CCDRCmd::CDR_ReadTrack(int fd, int start, uint8_t *pbuffer, int size)
{
	INIT_CGC();
	int blocks = size / CDROM_BLOCK;
	wait_unit_ready(fd, 120);

	cgc.cmd[0] = GPCMD_READ_10;
	cgc.cmd[2] = ((start >> 24) & 0xff);
	cgc.cmd[3] = ((start >> 16) & 0xff);
	cgc.cmd[4] = ((start >> 8) & 0xff);
	cgc.cmd[5] = (start & 0xff);

	cgc.cmd[7] = ((blocks >> 8) & 0xff);
	cgc.cmd[8] = (blocks & 0xff);

	cgc.buflen = size; 	// 块与扇区的对齐blocks * 2048 字节

	if (send_cmd(fd, &cgc, (uint8_t *)pbuffer, CGC_DATA_READ, WAIT_BLANK))
	{
		DP_MMCCMD("readtrack", &cgc, __LINE__);
		return ERROR_DVD_OPTERFAILED;
	}
	return 0;
}

// 写轨道数据, size可以为任意大小
int CCDRCmd::CDR_WriteTrack(int fd, stCDRTrack *ptrack, uint8_t *pbuffer, int size)
{
	int copysize;
	uint8_t * pStart = NULL;
	if (ptrack->bclosed)
	{
		DP(("track closed, can't write\n"));
		return ERROR_DVD_UNBLANKDISC;
	}

	if (!ptrack->pbuffer)
	{
		DPERROR(("track buffer is null!\n"));
		return -1;
	}

	// #if defined(DEBUG)
	// 	if( (size & (PACKET32_SIZE - 1)) )
	// 	{
	// 		DPERROR(("no track buffer to save data, size:%d \n",size));
	// 		return ERROR_DVD_WRITEERROR;
	// 	}
	// 
	// #endif//DEBUG

	/*
	while(size)
	{
	if(ptrack->buffsize)
	{
	copysize = PACKET32_SIZE - ptrack->buffsize;
	copysize = copysize > size ? size : copysize;
	memcpy(ptrack->pbuffer + ptrack->buffsize, pbuffer, copysize);
	pbuffer += copysize;
	size    -= copysize;
	ptrack->buffsize += copysize;
	if(ptrack->buffsize == PACKET32_SIZE)
	{
	DP(("writetrack_block1, fd=%d ptrack->writenext:%d  \n", fd, ptrack->writenext));
	if(!writetrack_block(fd, ptrack->pbuffer, ptrack->writenext, PACKET_BLOCK_32, WRITE_TRYNUMS))
	{
	DPERROR(("writetrack_block error\n"));
	return ERROR_DVD_WRITEERROR;
	}
	ptrack->buffsize = 0;
	ptrack->writenext	+= PACKET_BLOCK_32;
	ptrack->writedsize	+= PACKET_BLOCK_32;
	ptrack->freeblocks	-= PACKET_BLOCK_32;
	}
	}
	else
	{
	if(size >= PACKET32_SIZE)
	{
	DP(("writetrack_block2, fd=%d ptrack->writenext:%d  \n", fd, ptrack->writenext));
	if(!writetrack_block(fd, pbuffer, ptrack->writenext, PACKET_BLOCK_32, WRITE_TRYNUMS))
	{
	DPERROR(("writetrack_block error\n"));
	return ERROR_DVD_WRITEERROR;
	}
	ptrack->writenext	+= PACKET_BLOCK_32;
	ptrack->writedsize	+= PACKET_BLOCK_32;
	ptrack->freeblocks	-= PACKET_BLOCK_32;
	pbuffer += PACKET32_SIZE;
	size    -= PACKET32_SIZE;
	}
	else
	{
	ptrack->buffsize = 0;
	copysize = PACKET32_SIZE - ptrack->buffsize;
	copysize = copysize > size ? size : copysize;
	memcpy(ptrack->pbuffer + ptrack->buffsize, pbuffer, copysize);
	pbuffer += copysize;
	size    -= copysize;
	ptrack->buffsize += copysize;
	}
	}
	}
	*/

	pStart = ptrack->pbuffer;
	if (size)
	{
		memcpy(ptrack->pbuffer + ptrack->buffsize, pbuffer, size);
		ptrack->buffsize += size;
	}

	DP(("cdr_writetrack, fd=%d ptrack->writestart:%d ptrack->writenext:%d ptrack->buffsize:%d \n", fd, ptrack->writestart, ptrack->writenext, ptrack->buffsize));
	while (ptrack->buffsize >= PACKET32_SIZE)
	{
		struct timeval	tvStart;
		struct timeval	tvEnd;
		gettimeofday(&tvStart, NULL);
		struct tm 	*tm;
		tm = localtime(&tvStart.tv_sec);
		DP(("cdr_writetrack writetrack_block, fd=%d ptrack->writestart:%d ptrack->writenext:%d writetrack_blocktime:   %d:%d:%d %ld us\n", fd, ptrack->writestart, ptrack->writenext, tm->tm_hour, tm->tm_min, tm->tm_sec, tvStart.tv_usec));
		if (!writetrack_block(fd, pStart, ptrack->writenext, PACKET_BLOCK_32, WRITE_TRYNUMS))
		{
			DPERROR(("writetrack_block error, fd=%d ptrack->writestart:%d ptrack->writenext:%d  \n", fd, ptrack->writestart, ptrack->writenext));
			return ERROR_DVD_WRITEERROR;
		}
		pStart += PACKET32_SIZE;
		ptrack->buffsize -= PACKET32_SIZE;
		ptrack->writenext += PACKET_BLOCK_32;
		ptrack->writedsize += PACKET_BLOCK_32;
		ptrack->freeblocks -= PACKET_BLOCK_32;
		gettimeofday(&tvEnd, NULL);
		DP(("cdr_writetrack after writetrack_block, fd=%d ptrack->writestart:%d ptrack->writenext:%d ptrack->buffsize:%d writetrack_blocklasttime:%ds %ldus \n", fd, ptrack->writestart, ptrack->writenext, ptrack->buffsize, (tvEnd.tv_sec - tvStart.tv_sec), tvEnd.tv_usec - tvStart.tv_usec));
	}

	if (ptrack->buffsize > 0 && pStart != ptrack->pbuffer)
	{
		memmove(ptrack->pbuffer, pStart, ptrack->buffsize);
	}

	if (ptrack->writedsize >= ptrack->tracksize || ptrack->freeblocks <= 0)
	{
		ptrack->bclosed = TRUE;
	}
	DP(("end cdr_writetrack, fd=%d ptrack->writestart:%d ptrack->writenext:%d ptrack->buffsize:%d ptrack->bclosed:%d\n", fd, ptrack->writestart, ptrack->writenext, ptrack->buffsize, ptrack->bclosed));
	return 0;
}

// 刷新剩余数据到轨道
int CCDRCmd::CDR_FlushTrack(int fd, stCDRTrack *ptrack)
{
	if (ptrack->buffsize > 0)
	{
		if (ptrack->buffsize < PACKET32_SIZE)
		{
			memset(ptrack->pbuffer + ptrack->buffsize, 0, (PACKET32_SIZE - ptrack->buffsize));
		}
		else
		{
			fprintf(stderr, "*******cdr_flushtrack******** bufsize %d\n", ptrack->buffsize);
		}
		if (!writetrack_block(fd, ptrack->pbuffer, ptrack->writenext, PACKET_BLOCK_32, WRITE_TRYNUMS))
		{
			DPERROR(("flushtrack data error\n"));
			return ERROR_DVD_WRITEERROR;
		}
		ptrack->buffsize = 0;
		ptrack->writenext += PACKET_BLOCK_32;
		ptrack->writedsize += PACKET_BLOCK_32;
		ptrack->freeblocks -= PACKET_BLOCK_32;
	}
	return 0;
}

// 暂停数据写入
int CCDRCmd::CDR_PauseWrite(int fd)
{
	return 0;
}

// 恢复数据写入
int CCDRCmd::CDR_ResumeWrite(int fd)
{
	return 0;
}

// 关闭轨道
int CCDRCmd::CDR_CloseTrack(int fd, stCDRTrack *ptrack)
{
	int i, iRet;
	if (ptrack->bclosed)
		return 0;

	for (i = 0; i<100; i++)
	{
		iRet = close_track(fd, (int)ptrack->trackid);
		if (!iRet)
			return 0;
		CSLEEP(20);
	}
	return ERROR_DVD_OPTERFAILED;
}

// 关闭session
int CCDRCmd::CDR_CloseSession(int fd, stCDRTrack *ptrack)
{
	int i, iRet;
	if (ptrack->bclosed)
		return 0;

	for (i = 0; i<100; i++)
	{
		iRet = close_session(fd, (int)ptrack->sessionid);
		if (!iRet)
		{
			ptrack->bclosed = TRUE;
			return 0;
		}
		CSLEEP(20);
	}
	return ERROR_DVD_OPTERFAILED;
}

// 打印光驱profile
int CCDRCmd::CDR_DpdevProfile(int fd)
{

	unsigned char buf[1024];
	unsigned char disclist[256];
	unsigned char *pbuf;
	int datalen = 0;
	int profilenum = 0;
	int i, j;

	memset(buf, 0, sizeof(buf));
	GetDevProfile(fd, buf);

	datalen = buf[11];       //profile 数据
	profilenum = (datalen / 4); //profile 数量 数量可以判定是何种设备
	DP(("profilenum=%d\n", profilenum));

	j = 0;
	pbuf = buf + 13;   //13开始为profile数据区
	for (i = 0; i < profilenum; i++)
	{
		FindDiscType((char*)pbuf, disclist, &j);
		pbuf += 4;
	}

	//打印输出
	DP(("Device Support media num is %d!\n", j));

	for (i = 0; i < j; i++)
	{
		DP(("Media no.%d is %d!\n", i, disclist[i]));
	}

	//Current Profile
	DP(("Current Profile %x!\n", buf[7]));

	pbuf = buf + 13;
	for (i = 0; i < profilenum; i++)
	{
		DP(("profile no.%d is %x!\n", i, *pbuf));
		pbuf += 4;
	}

	return 0;
}

//获取光盘支持的最大速度
int CCDRCmd::CDR_GetMaxSpeed(int fd, int *MaxReadSpeed, int *MaxWriteSpeed)
{
	unsigned char header[100];
	unsigned char * buffer;
	int offset;
	int len = 8;

	INIT_CGC();

	//第一次读取写参数到header中
	memset(&cgc, 0, sizeof(cgc));
	cgc.cmd[0] = GPCMD_MODE_SENSE_10;
	cgc.cmd[2] = GPMODE_CAPABILITIES_PAGE | 0 << 6;
	cgc.cmd[7] = cgc.buflen >> 8;
	cgc.cmd[8] = cgc.buflen = len;

	send_cmd(fd, &cgc, header, CGC_DATA_READ, WAIT_BLANK);

	len = 2 + (((header[0] & 0xff) << 8) | (header[1] & 0xff));
	offset = 8 + (((header[6] & 0xff) << 8) | (header[7] & 0xff));
	buffer = (unsigned char*)calloc(len, sizeof(uint8_t));

	//第二次读取当前设备写参数，到buffer中
	memset(&cgc, 0, sizeof(cgc));
	cgc.cmd[0] = GPCMD_MODE_SENSE_10;
	cgc.cmd[2] = GPMODE_CAPABILITIES_PAGE | 0 << 6;
	cgc.cmd[7] = cgc.buflen >> 8;
	cgc.cmd[8] = cgc.buflen = len;

	send_cmd(fd, &cgc, buffer, CGC_DATA_READ, WAIT_BLANK);

	struct mm_cap_page_2A* mm = (struct mm_cap_page_2A*)&buffer[8];

	*MaxReadSpeed = (int)from2Byte(mm->max_read_speed);
	*MaxWriteSpeed = (int)from2Byte(mm->max_write_speed);
	DP(("*MaxReadSpeed=%d,*MaxWriteSpeed =%d\n", *MaxReadSpeed, *MaxWriteSpeed));

	free(buffer);
	return 0;
}

//默认12倍速,建议默认情况下不要设置速度，当前的光驱都会自动调整以最佳方式工作
int CCDRCmd::CDR_SetSpeed(int fd, int ReadSpeed, int WriteSpeed, int DiscType)
{
	int Speed;
	/*  //嵌入式系统下，可以成功，x86下略有差别

	Speed = DVD_SPEED_X1;        //默认为dvd
	struct cdrom_generic_command cgc;

	DP(("setting write speed to %d\n", WriteSpeed));
	DP(("setting read speed to %d\n", ReadSpeed));

	if(B_DVD_DISC == DiscType)
	Speed = BD_SPEED_X1;
	else
	Speed = DVD_SPEED_X1;

	memset(&cgc, 0, sizeof(cgc));
	cgc.cmd[0] = GPCMD_SET_SPEED;

	cgc.cmd[2] = ((Speed * ReadSpeed) >> 8) & 0xff;//0xff;
	cgc.cmd[3] = (Speed * ReadSpeed) & 0xff;//0xff;

	cgc.cmd[4] = ((Speed * WriteSpeed) >> 8) & 0xff;
	cgc.cmd[5] = (Speed * WriteSpeed) & 0xff;
	*/

	struct cdrom_generic_command cgc;

	DP(("setting speed to ReadSpeed:%d WriteSpeed:%d \n", ReadSpeed, WriteSpeed));

	memset(&cgc, 0, sizeof(cgc));
	cgc.cmd[0] = 0xbb;

	/* read speed */
	cgc.cmd[2] = 0xff;
	cgc.cmd[3] = 0xff;

	/* write speed */
	cgc.cmd[4] = ((0xb0 * WriteSpeed) >> 8) & 0xff;
	cgc.cmd[5] = (0xb0 * WriteSpeed) & 0xff;

	return send_cmd(fd, &cgc, NULL, CGC_DATA_NONE, WAIT_BLANK);
}

int CCDRCmd::CDR_GetDiscUsedSize(int fd, int * usedsize)
{
	unsigned char header[10];

	INIT_CGC();

	//获取光盘使用容量
	cgc.cmd[0] = GPCMD_READ_CDVD_CAPACITY;
	cgc.cmd[1] = 1;
	cgc.buflen = 10;

	send_cmd(fd, &cgc, header, CGC_DATA_READ, WAIT_BLANK);
	*usedsize = (header[0] << 24) | (header[1] << 16)
		| (header[2] << 8) | header[3];
	*usedsize = *usedsize / 512;                         //单位为M
	DP(("usedsize=%d\n", *usedsize));
	return 0;
}

//获取仓门状态
int CCDRCmd::CDR_GetDoorState(int fd)
{
	unsigned char buf[200];
	int Ret = 0;

	INIT_CGC();

	memset(buf, 0, 200);
	cgc.cmd[0] = GPCMD_MECHANISM_STATUS;//GPCMD_GET_EVENT_STATUS_NOTIFICATION;
	//cgc.cmd[4] = 0x10;	   
	//cgc.cmd[7] = 
	cgc.cmd[8] = 100 >> 8;
	cgc.cmd[9] = 100;
	cgc.buflen = 100;
	send_cmd(fd, &cgc, buf, CGC_DATA_READ, WAIT_BLANK);
	Ret = buf[1] >> 4;

	return Ret;
}