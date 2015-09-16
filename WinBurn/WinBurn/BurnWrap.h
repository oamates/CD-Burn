#pragma once

#include "ckmmc/devicemanager.hh"
#include "ckmmc/device.hh"

#include <map>

#include "Core.h"
#include "TreeManager.h"
#include "ActionManager.h"

#include "ProgressDlg.h"
#include "LogDlg.h"

class DeviceScanCallback : public ckmmc::DeviceManager::ScanCallback
{
public:
    DeviceScanCallback();
    ~DeviceScanCallback();

    virtual void event_status(ckmmc::DeviceManager::ScanCallback::Status status);
    virtual bool event_device(ckmmc::Device::Address &addr);
};

class CBurnWrap
{
public:
    //光驱信息
    typedef struct
    {
        wchar_t wcszVender[1024];
        wchar_t wcszIdentifier[1024];
        wchar_t wcszName[1024];
        wchar_t wcszRevision[1024];
        std::vector<unsigned long> vecReadSpeeds;
        std::vector<unsigned long> vecWriteSpeeds;
    }BURN_DEVICE_INFORMATION;

    //光盘信息
    typedef struct
    {
        unsigned int uiDiscType;
        unsigned int uiDiscStatus;
        unsigned int uiDiscFlags;
        unsigned __int64 uiUsedBytes;
        unsigned __int64 uiFreeBytes;
    }DISC_INFORMATION;

    //刻录状态信息
    typedef struct
    {
        int nState;
        wchar_t wcszBurnStateDescription[1024];
        unsigned char ucBurnedPercentage;
    }BURN_STATE_INFORMATION;

    CBurnWrap(void);
    ~CBurnWrap(void);

    void ScanDevice();

    void GetDeviceInformation(int nIndex,BURN_DEVICE_INFORMATION &info) const;

    int GetDeviceCount() const;

    void LoadDisc(int nDeviceIndex);

    bool HaveDisc(int nDeviceIndex);

    bool DiscBurnable(int nDeviceIndex);

    bool GetDiscInformation(int nDeviceIndex,DISC_INFORMATION &stDiscInformation) const;

    bool FormatDisc(int nDeviceIndex);

    void SetDiscVolumeLabel(CStringW strVolumeLabel);

    void CreateDataProject();

    bool AddFile(CProjectNode *pDirectory,CStringW strFilePath);

    CProjectNode * AddDirectory(CProjectNode *pParentDirectory,CStringW strDirectoryPath);

    void StartBurn(int nDeviceIndex);

    void CloseDataProject();

    void EjectDisc(int nDeviceIndex);

    void GetBurnStateInformation(BURN_STATE_INFORMATION &burnStateInfo) const;

private:

    void SetDevice(int nDeviceIndex);

    void SetBurnImageParameters();

    CProjectManager m_projectManager;
    CTreeManager m_treeManager;
    ckmmc::DeviceManager m_deviceManager;
    CCore m_core;
    CActionManager m_actionManager;
    CProgressDlg progressDlg;
    CLogDlg logDlg;
    CCore2 m_core2;

    CProjectSettings m_projectSettings;
    CBurnImageSettings m_burnImageSettings;

    DeviceScanCallback m_scanCB;
};
