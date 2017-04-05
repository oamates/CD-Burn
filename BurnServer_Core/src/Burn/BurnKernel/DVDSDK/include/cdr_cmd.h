//cdr_cmd.h
#ifndef __CDR_CMD_H__
#define __CDR_CMD_H__

#include "LibDVDSDK.h"
#include "drvcomm.h"

int DVDRec_GetCdrcmd(char *pCdrName, struct CDR_CMD_T **pCmd);

class CCDRCmd
{
public:
	CCDRCmd();
	~CCDRCmd();
public:
	BOOL CDR_LockDoor(int fd);		// 锁定托盘
	BOOL CDR_UnlockDoor(int fd);		// 解锁托盘
	BOOL CDR_OpenTray(int fd);		// 打开托盘
	BOOL CDR_CloseTray(int fd);		// 关闭托盘
	BOOL CDR_HaveDisc(int fd);		// 判断是否有光盘
	int CDR_FormatDisc(int fd, int ReserveBlocks);									// 格式化光盘
	int CDR_Buffcap(int fd, uint32_t *buffsize, uint32_t *bufffree);				// 获得光驱Buffer能力
	int CDR_DiscBasicType(int fd, int *type);										// 获得光盘基本类型
	int CDR_DiscExactType(int fd, int *type);										// 获得光盘精确类型
	int CDR_GetDiscInfo(int fd, int *TrickCount, int *SessionCount, uint64_t *Capicity, int *DiskStatus);
	int CDR_DevcapAblity(int fd, int *capablity);									// 获得光驱能力
	int CDR_LoadMedia(int fd);														// 加载媒体
	int CDR_ReadTrack(int fd, int start, uint8_t *pbuffer, int size);				// 读轨道数据
	int CDR_WriteTrack(int fd, CDR_TRACK_T *ptrack, uint8_t *pbuffer, int size);	// 写轨道数据, size可以为任意大小
	int CDR_FlushTrack(int fd, CDR_TRACK_T *ptrack);								// 刷新全部数据到轨道
	int CDR_PauseWrite(int fd);														// 暂停数据写入
	int CDR_ResumeWrite(int fd);													// 恢复数据写入
	int CDR_GetTrackinfo(int fd, int trackid, CDR_TRACK_T *ptrack);					// 获得轨道信息
	int CDR_CloseTrack(int fd, CDR_TRACK_T *ptrack);								// 关闭轨道
	int CDR_CloseSession(int fd, CDR_TRACK_T *ptrack);								// 关闭session
	int CDR_DpdevProfile(int fd);													// 打印光驱profile
	int CDR_GetdevInfo(int fd, DVD_DEV_INFO_T * pDevInfo);	                    // 获取光驱厂商
	int CDR_GetMaxSpeed(int fd, int *MaxReadSpeed, int *MaxWriteSpeed);               // 获取光盘最大速度 
	int CDR_SetSpeed(int fd, int ReadSpeed, int WriteSpeed, int DiscType);             // 设定最大速度 
	int CDR_GetDiscUsedSize(int fd, int * usedsize);                                 // 获取光盘使用容量
	int CDR_GetDoorState(int fd);                                                   //获取仓门状态
};


#endif//__CDR_CMD_H__
