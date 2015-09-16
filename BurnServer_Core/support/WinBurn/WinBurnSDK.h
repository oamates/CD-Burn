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
    unsigned __int64 uiUsedBytes;//已经使用量
    unsigned __int64 uiFreeBytes;//可用容量
}DISC_INFORMATION;

typedef struct
{
    int nState;//-1：刻录失败；0：初始状态；1：正在刻录；2：刻录完成
    char szBurnStateDescription[1024];
    unsigned char ucBurnedPercentage;
}BURN_STATE_INFORMATION;

extern "C"
{
    //创建刻录句柄
    WBSDLL BURNHANDLE CreateBurnHandle();

    //搜索刻录设备
    WBSDLL void ScanDevice(BURNHANDLE handle);

    //获得刻录设备总数
    WBSDLL unsigned int GetDeviceCount(BURNHANDLE handle);

    //获得刻录光驱信息
    WBSDLL void GetDeviceInfo(BURNHANDLE handle,unsigned int nDeviceIndex,BURN_DEVICE_INFORMATION &devInfo);

    //是否插入光盘
    WBSDLL bool HaveDisc(BURNHANDLE handle,unsigned int nDeviceIndex);

    //光盘是否可刻
    WBSDLL bool DiscBurnable(BURNHANDLE handle,unsigned int nDeviceIndex);

    //加载光盘
    WBSDLL void LoadDisc(BURNHANDLE handle,unsigned int nDeviceIndex);

    //弹出光盘
    WBSDLL void EjectDisc(BURNHANDLE handle,unsigned int nDeviceIndex);

    //获得光盘信息
    WBSDLL bool GetDiscInfo(BURNHANDLE handle,unsigned int nDeviceIndex,DISC_INFORMATION &discInfo);

    //格式化光盘
    WBSDLL bool FormatDisc(BURNHANDLE handle,unsigned int nDeviceIndex);

    //新建刻录项目
    WBSDLL void CreateDataProject(BURNHANDLE handle);

    //添加文件
    WBSDLL bool AddFile(BURNHANDLE handle,BURNDIRECTORY dir,char *pFullFilePath);

    //添加目录
    WBSDLL BURNDIRECTORY AddDirectory(BURNHANDLE handle,BURNDIRECTORY parentDir,char *pFullDirPath);

    //开始刻录
    WBSDLL void StartBurn(BURNHANDLE handle,unsigned int nDeviceIndex);

    //关闭刻录项目
    WBSDLL void CloseDataProject(BURNHANDLE handle);

    //销毁刻录句柄
    WBSDLL void DestroyBurnHandle(BURNHANDLE handle);

    //获得刻录状态信息
    WBSDLL void GetBurnStateInfo(BURNHANDLE handle,BURN_STATE_INFORMATION &burnStateInfo);
};
#endif
