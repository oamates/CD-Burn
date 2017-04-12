//cdr_cmd.h
#ifndef __CDR_CMD_H__
#define __CDR_CMD_H__

#include "LibDVDSDK.h"
#include "drvcomm.h"

class CCDRCmd;

int DVDRec_GetCdrcmd(char *pCdrName, CCDRCmd* pCmd);

class CCDRCmd
{
public:
	CCDRCmd(){};
	~CCDRCmd(){};
public:
	// ��������
	BOOL CDR_LockDoor(int nFd);		
	// ��������
	BOOL CDR_UnlockDoor(int nFd);
	// ������
	BOOL CDR_OpenTray(int nFd);
	// �ر�����
	BOOL CDR_CloseTray(int nFd);	
	// �ж��Ƿ��й���
	BOOL CDR_HaveDisc(int nFd);		
	// ��ʽ������
	int CDR_FormatDisc(int nFd, int nReserveBlocks);									
	// ��ù���Buffer����
	int CDR_Buffcap(int nFd, uint32_t *pBuffersize, uint32_t *pBufferfree);			
	// ��ù��̻�������
	int CDR_DiscBasicType(int nFd, int *pType);
	// ��ù��̾�ȷ����
	int CDR_DiscExactType(int nFd, int *pType);										
	int CDR_GetDiscInfo(int nFd, int *pTrickCount, int *pSessionCount, uint64_t *pCapicity, int *pDiskStatus);
	// ��ù�������
	int CDR_DevcapAblity(int nFd, int *pCapablity);
	// ����ý��
	int CDR_LoadMedia(int nFd);
	// ���������
	int CDR_ReadTrack(int nFd, int nStart, uint8_t *pBuffer, int nSize);
	// д�������, size����Ϊ�����С
	int CDR_WriteTrack(int nFd, stCDRTrack *pTrack, uint8_t *pBuffer, int nSize);
	// ˢ��ȫ�����ݵ����
	int CDR_FlushTrack(int nFd, stCDRTrack *pTrack);
	// ��ͣ����д��
	int CDR_PauseWrite(int nFd);
	// �ָ�����д��
	int CDR_ResumeWrite(int nFd);
	// ��ù����Ϣ
	int CDR_GetTrackinfo(int nFd, int pTrackid, stCDRTrack *pTrack);
	// �رչ��
	int CDR_CloseTrack(int nFd, stCDRTrack *pTrack);
	// �ر�session
	int CDR_CloseSession(int nFd, stCDRTrack *pTrack);
	// ��ӡ����profile
	int CDR_DpdevProfile(int nFd);
	// ��ȡ��������
	int CDR_GetdevInfo(int nFd, DVD_DEV_INFO_T * pDevInfo);
	// ��ȡ��������ٶ� 
	int CDR_GetMaxSpeed(int nFd, int *pMaxReadSpeed, int *pMaxWriteSpeed);
	// �趨����ٶ� 
	int CDR_SetSpeed(int nFd, int nReadSpeed, int nWriteSpeed, int nDiscType);
	// ��ȡ����ʹ������
	int CDR_GetDiscUsedSize(int nFd, int *pUsedsize);
	//��ȡ����״̬
	int CDR_GetDoorState(int nFd);
};

#endif//__CDR_CMD_H__
