//cdr_cmd.h
#ifndef __CDR_CMD_H__
#define __CDR_CMD_H__

#include "LibDVDSDK.h"
#include "drvcomm.h"

//int DVDRec_GetCdrcmd(char *pCdrName, struct CDR_CMD_T **pCmd);

class CCDRCmd;

int DVDRec_GetCdrcmd(char *pCdrName, CCDRCmd* pCmd);

class CCDRCmd
{
public:
	CCDRCmd(){};
	~CCDRCmd(){};
public:
	// 锁定托盘
	BOOL CDR_LockDoor(int nFd);		
	// 解锁托盘
	BOOL CDR_UnlockDoor(int nFd);
	// 打开托盘
	BOOL CDR_OpenTray(int nFd);
	// 关闭托盘
	BOOL CDR_CloseTray(int nFd);	
	// 判断是否有光盘
	BOOL CDR_HaveDisc(int nFd);		
	// 格式化光盘
	int CDR_FormatDisc(int nFd, int ReserveBlocks);									
	// 获得光驱Buffer能力
	int CDR_Buffcap(int nFd, uint32_t *buffersize, uint32_t *bufferfree);			
	// 获得光盘基本类型
	int CDR_DiscBasicType(int nFd, int *type);
	// 获得光盘精确类型
	int CDR_DiscExactType(int nFd, int *type);										
	int CDR_GetDiscInfo(int nFd, int *TrickCount, int *SessionCount, uint64_t *Capicity, int *DiskStatus);
	// 获得光驱能力
	int CDR_DevcapAblity(int nFd, int *capablity);
	// 加载媒体
	int CDR_LoadMedia(int nFd);
	// 读轨道数据
	int CDR_ReadTrack(int nFd, int start, uint8_t *pbuffer, int size);
	// 写轨道数据, size可以为任意大小
	int CDR_WriteTrack(int nFd, stCDRTrack *ptrack, uint8_t *pbuffer, int size);
	// 刷新全部数据到轨道
	int CDR_FlushTrack(int nFd, stCDRTrack *ptrack);
	// 暂停数据写入
	int CDR_PauseWrite(int nFd);
	// 恢复数据写入
	int CDR_ResumeWrite(int nFd);
	// 获得轨道信息
	int CDR_GetTrackinfo(int nFd, int trackid, stCDRTrack *ptrack);
	// 关闭轨道
	int CDR_CloseTrack(int nFd, stCDRTrack *ptrack);
	// 关闭session
	int CDR_CloseSession(int nFd, stCDRTrack *ptrack);
	// 打印光驱profile
	int CDR_DpdevProfile(int nFd);
	// 获取光驱厂商
	int CDR_GetdevInfo(int nFd, DVD_DEV_INFO_T * pDevInfo);
	// 获取光盘最大速度 
	int CDR_GetMaxSpeed(int nFd, int *MaxReadSpeed, int *MaxWriteSpeed);
	// 设定最大速度 
	int CDR_SetSpeed(int nFd, int ReadSpeed, int WriteSpeed, int DiscType);
	// 获取光盘使用容量
	int CDR_GetDiscUsedSize(int nFd, int * usedsize);
	//获取仓门状态
	int CDR_GetDoorState(int nFd);
};

#endif//__CDR_CMD_H__
