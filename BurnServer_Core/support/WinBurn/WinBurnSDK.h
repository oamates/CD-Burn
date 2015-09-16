#ifndef _WIN_BURN_SDK_H_
#define _WIN_BURN_SDK_H_

#include <vector>
#include <string>

#ifdef WIN_BURN_DLL_EXPORT
#define WBSDLL __declspec(dllexport)
#else
#define WBSDLL __declspec(dllimport)
#endif

typedef void *BURNHANDLE;
typedef void *BURNDIRECTORY;

typedef struct
{
    char szVender[512];
    char szIdentifier[512];
    char szName[512];
    char szRevision[512];
    unsigned long readSpeeds[32];
    unsigned long writeSpeeds[32];
}BURN_DEVICE_INFORMATION;

typedef struct
{
    unsigned int uiDiscType;
    unsigned int uiDiscStatus;
    unsigned int uiDiscFlags;
    unsigned __int64 uiUsedBytes;//�Ѿ�ʹ����
    unsigned __int64 uiFreeBytes;//��������
}DISC_INFORMATION;

typedef struct
{
    int nState;//-1����¼ʧ�ܣ�0����ʼ״̬��1�����ڿ�¼��2����¼���
    char szBurnStateDescription[1024];
    unsigned char ucBurnedPercentage;
}BURN_STATE_INFORMATION;

extern "C"
{
    //������¼���
    WBSDLL BURNHANDLE CreateBurnHandle();

    //������¼�豸
    WBSDLL void ScanDevice(BURNHANDLE handle);

    //��ÿ�¼�豸����
    WBSDLL unsigned int GetDeviceCount(BURNHANDLE handle);

    //��ÿ�¼������Ϣ
    WBSDLL void GetDeviceInfo(BURNHANDLE handle,unsigned int nDeviceIndex,BURN_DEVICE_INFORMATION &devInfo);

    //�Ƿ�������
    WBSDLL bool HaveDisc(BURNHANDLE handle,unsigned int nDeviceIndex);

    //�����Ƿ�ɿ�
    WBSDLL bool DiscBurnable(BURNHANDLE handle,unsigned int nDeviceIndex);

    //���ع���
    WBSDLL void LoadDisc(BURNHANDLE handle,unsigned int nDeviceIndex);

    //��������
    WBSDLL void EjectDisc(BURNHANDLE handle,unsigned int nDeviceIndex);

    //��ù�����Ϣ
    WBSDLL bool GetDiscInfo(BURNHANDLE handle,unsigned int nDeviceIndex,DISC_INFORMATION &discInfo);

    //��ʽ������
    WBSDLL bool FormatDisc(BURNHANDLE handle,unsigned int nDeviceIndex);

    //�½���¼��Ŀ
    WBSDLL void CreateDataProject(BURNHANDLE handle);

    //����ļ�
    WBSDLL bool AddFile(BURNHANDLE handle,BURNDIRECTORY dir,char *pFullFilePath);

    //���Ŀ¼
    WBSDLL BURNDIRECTORY AddDirectory(BURNHANDLE handle,BURNDIRECTORY parentDir,char *pFullDirPath);

    //��ʼ��¼
    WBSDLL void StartBurn(BURNHANDLE handle,unsigned int nDeviceIndex);

    //�رտ�¼��Ŀ
    WBSDLL void CloseDataProject(BURNHANDLE handle);

    //���ٿ�¼���
    WBSDLL void DestroyBurnHandle(BURNHANDLE handle);

    //��ÿ�¼״̬��Ϣ
    WBSDLL void GetBurnStateInfo(BURNHANDLE handle,BURN_STATE_INFORMATION &burnStateInfo);
};
#endif
