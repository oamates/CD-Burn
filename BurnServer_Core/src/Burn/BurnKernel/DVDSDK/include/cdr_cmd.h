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
	int CDR_FormatDisc(int nFd, int ReserveBlocks);									
	// ��ù���Buffer����
	int CDR_Buffcap(int nFd, uint32_t *buffersize, uint32_t *bufferfree);			
	// ��ù��̻�������
	int CDR_DiscBasicType(int nFd, int *type);
	// ��ù��̾�ȷ����
	int CDR_DiscExactType(int nFd, int *type);										
	int CDR_GetDiscInfo(int nFd, int *TrickCount, int *SessionCount, uint64_t *Capicity, int *DiskStatus);
	// ��ù�������
	int CDR_DevcapAblity(int nFd, int *capablity);
	// ����ý��
	int CDR_LoadMedia(int nFd);
	// ���������
	int CDR_ReadTrack(int nFd, int start, uint8_t *pbuffer, int size);
	// д�������, size����Ϊ�����С
	int CDR_WriteTrack(int nFd, stCDRTrack *ptrack, uint8_t *pbuffer, int size);
	// ˢ��ȫ�����ݵ����
	int CDR_FlushTrack(int nFd, stCDRTrack *ptrack);
	// ��ͣ����д��
	int CDR_PauseWrite(int nFd);
	// �ָ�����д��
	int CDR_ResumeWrite(int nFd);
	// ��ù����Ϣ
	int CDR_GetTrackinfo(int nFd, int trackid, stCDRTrack *ptrack);
	// �رչ��
	int CDR_CloseTrack(int nFd, stCDRTrack *ptrack);
	// �ر�session
	int CDR_CloseSession(int nFd, stCDRTrack *ptrack);
	// ��ӡ����profile
	int CDR_DpdevProfile(int nFd);
	// ��ȡ��������
	int CDR_GetdevInfo(int nFd, DVD_DEV_INFO_T * pDevInfo);
	// ��ȡ��������ٶ� 
	int CDR_GetMaxSpeed(int nFd, int *MaxReadSpeed, int *MaxWriteSpeed);
	// �趨����ٶ� 
	int CDR_SetSpeed(int nFd, int ReadSpeed, int WriteSpeed, int DiscType);
	// ��ȡ����ʹ������
	int CDR_GetDiscUsedSize(int nFd, int * usedsize);
	//��ȡ����״̬
	int CDR_GetDoorState(int nFd);
};

#endif//__CDR_CMD_H__
