#include <stdafx.h>

#include "WinBurnSDK.h"

#include "BurnWrap.h"

#include "CharsetConvertMFC.h"

BURNHANDLE CreateBurnHandle()
{
    CBurnWrap *p=new CBurnWrap();
    return p;
}

void ScanDevice(BURNHANDLE handle)
{
    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;
        p->ScanDevice();
    }
}

unsigned int GetDeviceCount(BURNHANDLE handle)
{
    if (NULL == handle)
    {
        return 0;
    }

    CBurnWrap *p=(CBurnWrap *)handle;
    return p->GetDeviceCount();
}

void GetDeviceInfo(BURNHANDLE handle,unsigned int nDeviceIndex,BURN_DEVICE_INFORMATION &devInfo)
{
    devInfo.szVender[0]='\0';
    devInfo.szIdentifier[0]='\0';
    devInfo.szName[0]='\0';
    devInfo.szRevision[0]='\0';
    memset(devInfo.readSpeeds,0,sizeof(devInfo.readSpeeds));
    memset(devInfo.writeSpeeds,0,sizeof(devInfo.writeSpeeds));

    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;
        CBurnWrap::BURN_DEVICE_INFORMATION info;
        p->GetDeviceInformation(nDeviceIndex,info);

        CStringA strTemp;

        strTemp=CharsetConvertMFC::UTF16ToUTF8(info.wcszVender);
        strcpy_s(devInfo.szVender,sizeof(devInfo.szVender)/sizeof(char),strTemp.GetBuffer());

        strTemp=CharsetConvertMFC::UTF16ToUTF8(info.wcszIdentifier);
        strcpy_s(devInfo.szIdentifier,sizeof(devInfo.szIdentifier)/sizeof(char),strTemp.GetBuffer());

        strTemp=CharsetConvertMFC::UTF16ToUTF8(info.wcszName);
        strcpy_s(devInfo.szName,sizeof(devInfo.szName)/sizeof(char),strTemp.GetBuffer());

        strTemp=CharsetConvertMFC::UTF16ToUTF8(info.wcszRevision);
        strcpy_s(devInfo.szRevision,sizeof(devInfo.szRevision)/sizeof(char),strTemp.GetBuffer());

        size_t N=info.vecReadSpeeds.size();
        size_t M=sizeof(devInfo.readSpeeds)/sizeof(unsigned long);
        for (size_t i=0;i<N && i<M;++i)
        {
            devInfo.readSpeeds[i]=info.vecReadSpeeds.at(i);
        }

        N=info.vecWriteSpeeds.size();
        M=sizeof(devInfo.writeSpeeds)/sizeof(unsigned long);
        for (size_t i=0;i<N && i<M;++i)
        {
            devInfo.writeSpeeds[i]=info.vecWriteSpeeds.at(i);
        }
    }
}

bool HaveDisc(BURNHANDLE handle,unsigned int nDeviceIndex)
{
    //在刻录完成后，用同一句柄检测不到光盘

    bool bRet=false;

    TRACE("[HaveDisc] Before CreateBurnHandle,%d\r\n",__LINE__);

    BURNHANDLE h=CreateBurnHandle();

    TRACE("[HaveDisc] After CreateBurnHandle,%d\r\n",__LINE__);

    if (h != NULL &&
        handle != NULL)
    {
        TRACE("[HaveDisc] Before ScanDevice,%d\r\n",__LINE__);

        ScanDevice(h);

        TRACE("[HaveDisc] After ScanDevice,%d\r\n",__LINE__);

        CBurnWrap *p=(CBurnWrap *)h;

        TRACE("[HaveDisc] Before HaveDisc,%d\r\n",__LINE__);

        bRet=p->HaveDisc(nDeviceIndex);

        TRACE("[HaveDisc] After HaveDisc,%d\r\n",__LINE__);
    }

    if (h != NULL)
    {
        TRACE("[HaveDisc] Before DestroyBurnHandle,%d\r\n",__LINE__);

        DestroyBurnHandle(h);

        TRACE("[HaveDisc] After DestroyBurnHandle,%d\r\n",__LINE__);
    }

    return bRet;
}

bool DiscBurnable(BURNHANDLE handle,unsigned int nDeviceIndex)
{
    bool bRet=false;

    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;
        bRet=p->DiscBurnable(nDeviceIndex);
    }

    return bRet;
}

void LoadDisc(BURNHANDLE handle,unsigned int nDeviceIndex)
{
    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;
        p->LoadDisc(nDeviceIndex);
    }
}

void EjectDisc(BURNHANDLE handle,unsigned int nDeviceIndex)
{
    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;
        p->EjectDisc(nDeviceIndex);
    }
}

bool GetDiscInfo(BURNHANDLE handle,unsigned int nDeviceIndex,DISC_INFORMATION &discInfo)
{
    bool bRet=false;

    memset(&discInfo,0,sizeof(discInfo));

    BURNHANDLE h=CreateBurnHandle();
    ScanDevice(h);

    if (handle != NULL &&
        h != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)h;
        CBurnWrap::DISC_INFORMATION info;
        bRet=p->GetDiscInformation(nDeviceIndex,info);

        if (bRet)
        {
            discInfo.uiDiscFlags=info.uiDiscFlags;
            discInfo.uiDiscStatus=info.uiDiscStatus;
            discInfo.uiDiscType=info.uiDiscType;
            discInfo.uiFreeBytes=info.uiFreeBytes;
            discInfo.uiUsedBytes=info.uiUsedBytes;
        }
    }

    if (h != NULL)
    {
        DestroyBurnHandle(h);
    }

    return bRet;
}

bool FormatDisc(BURNHANDLE handle,unsigned int nDeviceIndex)
{
    bool bRet=false;

    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;
        bRet=p->FormatDisc(nDeviceIndex);
    }

    return bRet;
}

void SetDiscVolumeLabel(BURNHANDLE handle,char *pVolumeLabel)
{
    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;
        p->SetDiscVolumeLabel(CharsetConvertMFC::UTF8ToUTF16(pVolumeLabel));
    }
}

void CreateDataProject(BURNHANDLE handle)
{
    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;
        p->CreateDataProject();
    }
}

bool AddFile(BURNHANDLE handle,BURNDIRECTORY dir,char *pFullFilePath)
{
    bool bRet=false;

    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;

        CStringW strFullFilePath=CharsetConvertMFC::UTF8ToUTF16(pFullFilePath);
        bRet=p->AddFile((CProjectNode *)dir,strFullFilePath.GetBuffer());

        wprintf_s(L"[AddFile] File path : %s\r\n",strFullFilePath.GetBuffer());
    }

    return bRet;
}

BURNDIRECTORY AddDirectory(BURNHANDLE handle,BURNDIRECTORY parentDir,char *pFullDirPath)
{
    BURNDIRECTORY curDir=NULL;

    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;

        CStringW strFullDirPath=CharsetConvertMFC::UTF8ToUTF16(pFullDirPath);
        curDir=p->AddDirectory((CProjectNode *)parentDir,strFullDirPath.GetBuffer());

        wprintf_s(L"[AddDirectory] Dir path : %s\r\n",strFullDirPath.GetBuffer());
    }

    return curDir;
}

void StartBurn(BURNHANDLE handle,unsigned int nDeviceIndex)
{
    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;
        p->StartBurn(nDeviceIndex);
    }
}

void CloseDataProject(BURNHANDLE handle)
{
    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;
        p->CloseDataProject();
    }
}

void DestroyBurnHandle(BURNHANDLE handle)
{
    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;
        delete p;
    }
}

void GetBurnStateInfo(BURNHANDLE handle,BURN_STATE_INFORMATION &burnStateInfo)
{
    if (handle != NULL)
    {
        CBurnWrap *p=(CBurnWrap *)handle;
        CBurnWrap::BURN_STATE_INFORMATION info;
        p->GetBurnStateInformation(info);

        burnStateInfo.nState=info.nState;
        burnStateInfo.ucBurnedPercentage=info.ucBurnedPercentage;

        CStringA strBurnStateDescription=CharsetConvertMFC::UTF16ToUTF8(info.wcszBurnStateDescription);

        strcpy_s(burnStateInfo.szBurnStateDescription,
            sizeof(burnStateInfo.szBurnStateDescription)/sizeof(char),
            strBurnStateDescription.GetBuffer());
    }
}
