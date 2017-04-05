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
	BOOL CDR_LockDoor(int fd);		// ��������
	BOOL CDR_UnlockDoor(int fd);		// ��������
	BOOL CDR_OpenTray(int fd);		// ������
	BOOL CDR_CloseTray(int fd);		// �ر�����
	BOOL CDR_HaveDisc(int fd);		// �ж��Ƿ��й���
	int CDR_FormatDisc(int fd, int ReserveBlocks);									// ��ʽ������
	int CDR_Buffcap(int fd, uint32_t *buffsize, uint32_t *bufffree);				// ��ù���Buffer����
	int CDR_DiscBasicType(int fd, int *type);										// ��ù��̻�������
	int CDR_DiscExactType(int fd, int *type);										// ��ù��̾�ȷ����
	int CDR_GetDiscInfo(int fd, int *TrickCount, int *SessionCount, uint64_t *Capicity, int *DiskStatus);
	int CDR_DevcapAblity(int fd, int *capablity);									// ��ù�������
	int CDR_LoadMedia(int fd);														// ����ý��
	int CDR_ReadTrack(int fd, int start, uint8_t *pbuffer, int size);				// ���������
	int CDR_WriteTrack(int fd, CDR_TRACK_T *ptrack, uint8_t *pbuffer, int size);	// д�������, size����Ϊ�����С
	int CDR_FlushTrack(int fd, CDR_TRACK_T *ptrack);								// ˢ��ȫ�����ݵ����
	int CDR_PauseWrite(int fd);														// ��ͣ����д��
	int CDR_ResumeWrite(int fd);													// �ָ�����д��
	int CDR_GetTrackinfo(int fd, int trackid, CDR_TRACK_T *ptrack);					// ��ù����Ϣ
	int CDR_CloseTrack(int fd, CDR_TRACK_T *ptrack);								// �رչ��
	int CDR_CloseSession(int fd, CDR_TRACK_T *ptrack);								// �ر�session
	int CDR_DpdevProfile(int fd);													// ��ӡ����profile
	int CDR_GetdevInfo(int fd, DVD_DEV_INFO_T * pDevInfo);	                    // ��ȡ��������
	int CDR_GetMaxSpeed(int fd, int *MaxReadSpeed, int *MaxWriteSpeed);               // ��ȡ��������ٶ� 
	int CDR_SetSpeed(int fd, int ReadSpeed, int WriteSpeed, int DiscType);             // �趨����ٶ� 
	int CDR_GetDiscUsedSize(int fd, int * usedsize);                                 // ��ȡ����ʹ������
	int CDR_GetDoorState(int fd);                                                   //��ȡ����״̬
};


#endif//__CDR_CMD_H__
