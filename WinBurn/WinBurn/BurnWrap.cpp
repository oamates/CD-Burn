#include <stdafx.h>

#include "LogDlg.h"
#include "ProgressDlg.h"
#include "Settings.h"
#include "Core2Info.h"
#include "ActionManager.h"
#include "Core2Format.h"
#include "ProjectManager.h"

#include "BurnWrap.h"

#include "UUID.h"

DeviceScanCallback::DeviceScanCallback()
{
}

DeviceScanCallback::~DeviceScanCallback()
{
}

void DeviceScanCallback::event_status(ckmmc::DeviceManager::ScanCallback::Status status)
{
}

bool DeviceScanCallback::event_device(ckmmc::Device::Address &addr)
{
    bool bRet=false;

    ckmmc::Device device(addr);
    device.refresh();
    bRet=device.recorder();

    return bRet;
}

//////////////////////////////////////////////////////////////////////////

CBurnWrap::CBurnWrap()
{
    m_projectManager.SetProjectSettings(&m_projectSettings);
    m_actionManager.SetProjectSettings(&m_projectSettings);
    m_core2.SetProjectSettings(&m_projectSettings);

    m_projectManager.SetTreeManager(&m_treeManager);
    

    m_actionManager.SetCore(&m_core);
    m_actionManager.SetCore2(&m_core2);
    m_actionManager.SetBurnImageSettings(&m_burnImageSettings);
    m_actionManager.SetTreeManager(&m_treeManager);
    m_actionManager.SetProjectManager(&m_projectManager);
    m_actionManager.SetDeviceManager(&m_deviceManager);

    m_core.SetCore2(&m_core2);
    m_core.SetBurnImageSettings(&m_burnImageSettings);

    m_treeManager.SetTreeManager(&m_treeManager);

    m_core2.SetLogDlg(&logDlg);
    m_core.SetLogDlg(&logDlg);

    m_actionManager.SetProgressDlg(&progressDlg);
}

CBurnWrap::~CBurnWrap(void)
{
    
}

void CBurnWrap::SetDevice(int nDeviceIndex)
{
    int nDeviceCount=GetDeviceCount();
    if (nDeviceIndex < nDeviceCount)
    {
        m_burnImageSettings.m_pRecorder=m_deviceManager.devices().at(nDeviceIndex);
    }
}

void CBurnWrap::SetBurnImageParameters()
{
    m_burnImageSettings.m_bOnFly=false;
    m_burnImageSettings.m_bVerify=false;
    m_burnImageSettings.m_bEject=true;
    m_burnImageSettings.m_bSimulate=false;
    m_burnImageSettings.m_bBUP=true;
    m_burnImageSettings.m_bPadTracks=true;
    m_burnImageSettings.m_bFixate=true;

    m_burnImageSettings.m_lNumCopies=1;
}

void CBurnWrap::ScanDevice()
{
    m_deviceManager.scan(&m_scanCB);
}

void CBurnWrap::GetDeviceInformation(int nIndex,BURN_DEVICE_INFORMATION &info) const
{
    info.wcszVender[0]=L'\0';
    info.wcszIdentifier[0]=L'\0';
    info.wcszName[0]=L'\0';
    info.wcszRevision[0]=L'\0';

    int nDeviceCount=GetDeviceCount();
    if (nIndex < nDeviceCount)
    {
        ckmmc::Device *pDevice=m_deviceManager.devices().at(nIndex);
        pDevice->refresh();

        const std::vector<ckcore::tuint32> &readSpeeds=pDevice->read_speeds();
        info.vecReadSpeeds=std::vector<ckcore::tuint32>(readSpeeds.begin(),readSpeeds.end());
        wcscpy_s(info.wcszIdentifier,sizeof(info.wcszIdentifier)/sizeof(wchar_t),pDevice->identifier());

        wcscpy_s(info.wcszName,sizeof(info.wcszName)/sizeof(wchar_t),pDevice->address().device_.c_str());
        wcscat_s(info.wcszName+wcslen(info.wcszName),sizeof(info.wcszName)/sizeof(wchar_t)-wcslen(info.wcszName),L" : ");
        wcscat_s(info.wcszName+wcslen(info.wcszName),sizeof(info.wcszName)/sizeof(wchar_t)-wcslen(info.wcszName),pDevice->name());

        wcscpy_s(info.wcszRevision,sizeof(info.wcszRevision)/sizeof(wchar_t),pDevice->revision());
        wcscpy_s(info.wcszVender,sizeof(info.wcszVender)/sizeof(wchar_t),pDevice->vendor());

        const std::vector<ckcore::tuint32> &writeSpeeds=pDevice->write_speeds();
        info.vecWriteSpeeds=std::vector<ckcore::tuint32>(writeSpeeds.begin(),writeSpeeds.end());
    }
}

int CBurnWrap::GetDeviceCount() const
{
    return m_deviceManager.devices().size();
}

void CBurnWrap::LoadDisc(int nDeviceIndex)
{
    if (m_deviceManager.devices().size() > static_cast<size_t>(nDeviceIndex))
    {
        ckmmc::Device *pDevice = m_deviceManager.devices().at(nDeviceIndex);

        m_core.LoadDisc(*pDevice,true);
    }
}

bool CBurnWrap::HaveDisc(int nDeviceIndex)
{
    bool bSupportedProfile = true;

    int nDeviceCount=m_deviceManager.devices().size();
    if (nDeviceIndex < nDeviceCount)
    {
        ckmmc::Device *pDevice=m_deviceManager.devices().at(nDeviceIndex);
        pDevice->refresh();
        ckmmc::Device::Profile profile = pDevice->profile();

        if (profile == ckmmc::Device::ckPROFILE_NONE)
        {
            return false;
        }
    }

    return bSupportedProfile;
}

bool CBurnWrap::DiscBurnable(int nDeviceIndex)
{
    bool bRet=false;

    ckmmc::Device *pDevice=m_deviceManager.devices().at(nDeviceIndex);

    CCore2Info info(&logDlg);
    CCore2DiscInfo discInfo;

    if (info.ReadDiscInformation(*pDevice,&discInfo))
    {
        if (discInfo.m_ucFlags & CCore2DiscInfo::FLAG_ERASABLE)
        {
            bRet=true;
        }
        else
        {
            if (CCore2DiscInfo::DS_EMTPY == discInfo.m_ucDiscStatus)
            {
                bRet=true;
            }
        }
    }

    return bRet;
}

bool CBurnWrap::GetDiscInformation(int nDeviceIndex,DISC_INFORMATION &stDiscInformation) const
{
    bool bRet=true;

    if (m_deviceManager.devices().size() > static_cast<size_t>(nDeviceIndex))
    {
        ckmmc::Device *pDevice=m_deviceManager.devices().at(nDeviceIndex);
        pDevice->refresh();

        CCore2Info info(const_cast<CLogDlg *>(&logDlg));

        CCore2DiscInfo discInfo;
        if (info.ReadDiscInformation(*pDevice,&discInfo))
        {
            stDiscInformation.uiDiscType=discInfo.m_ucDiscType;
            stDiscInformation.uiDiscStatus=discInfo.m_ucDiscStatus;
            stDiscInformation.uiDiscFlags=discInfo.m_ucFlags;
        }
        else
        {
            bRet=false;
        }

        unsigned __int64 uiUsedSize=0;
        unsigned __int64 uiFreeSize=0;
        if (info.GetTotalDiscCapacity(*pDevice,uiUsedSize,uiFreeSize))
        {
            stDiscInformation.uiUsedBytes=uiUsedSize;
            stDiscInformation.uiFreeBytes=uiFreeSize;
        }
        else
        {
            if (stDiscInformation.uiDiscFlags & CCore2DiscInfo::FLAG_ERASABLE)
            {
                stDiscInformation.uiUsedBytes=uiUsedSize;
                stDiscInformation.uiFreeBytes=4706074624L;
            }
            else
            {
                bRet=false;
            }
        }
    }

    return bRet;
}

bool CBurnWrap::FormatDisc(int nDeviceIndex)
{
    bool bRet=false;

    if (m_deviceManager.devices().size() > static_cast<size_t>(nDeviceIndex))
    {
        ckmmc::Device *pDevice=m_deviceManager.devices().at(nDeviceIndex);

        CCore2Info info(&logDlg);
        CCore2DiscInfo discInfo;

        if (info.ReadDiscInformation(*pDevice,&discInfo))
        {
            if (discInfo.m_ucFlags & CCore2DiscInfo::FLAG_ERASABLE ||
                CCore2DiscInfo::DS_EMTPY == discInfo.m_ucDiscStatus)
            {
                CCore2Format discFormat(&logDlg);
                bRet=discFormat.FormatUnit(*pDevice,m_actionManager.GetProgressDlg(),true);
            }
        }
    }

    return bRet;
}

void CBurnWrap::SetDiscVolumeLabel(CStringW strVolumeLabel)
{
    m_projectSettings.SetDiscVolumeLabel(strVolumeLabel.GetBuffer());
}

void CBurnWrap::CreateDataProject()
{
    m_projectManager.NewDataProject();
}


bool CBurnWrap::AddFile(CProjectNode *pDirectory,CStringW strFilePath)
{
    bool bRet=false;

    CProjectManager::CFileTransaction fileTransaction(&m_treeManager);
    bRet=fileTransaction.AddFile(strFilePath.GetBuffer(),pDirectory);

    return bRet;
}

CProjectNode * CBurnWrap::AddDirectory(CProjectNode *pParentDirectory,CStringW strDirectoryPath)
{
    CProjectNode *pNode=NULL;
    CProjectManager::CFileTransaction fileTransaction(&m_treeManager);

    if (NULL == pParentDirectory)
    {
        pNode=fileTransaction.AddFolder(pParentDirectory,strDirectoryPath.GetBuffer());
    }
    else
    {
        pNode=pParentDirectory;
    }

    std::vector<CProjectNode *> FolderStack;
    fileTransaction.AddFilesInFolder(pNode,FolderStack);

    CProjectNode *pTargetNode=NULL;
    wchar_t wcszRealParentPath[1024]={0};
    while (!FolderStack.empty())
    { 
        pTargetNode=FolderStack.back();
        FolderStack.pop_back();

        lstrcpy(wcszRealParentPath,pTargetNode->pItemData->szFullPath);
        AddDirectory(pTargetNode,wcszRealParentPath);
    }

    return pNode;
}

void CBurnWrap::StartBurn(int nDeviceIndex)
{
    //¶àÅÌÐø¿ÌÊ±£¬ÖØÖÃ¿ÌÂ¼×´Ì¬
    progressDlg.SetBurnState(0);

    progressDlg.SetBurnStateDescription(L"");
    m_core.SetBurnedPercentage(0);

    SetDevice(nDeviceIndex);

    SetBurnImageParameters();

    printf("[CBurnWrap::StartBurn] **** Start burn ****\r\n");

    m_actionManager.BurnCompilation(NULL);
}

void CBurnWrap::CloseDataProject()
{
}

void CBurnWrap::EjectDisc(int nDeviceIndex)
{
    if (m_deviceManager.devices().size() > static_cast<size_t>(nDeviceIndex))
    {
        ckmmc::Device *pDevice = m_deviceManager.devices().at(nDeviceIndex);

        m_core.EjectDisc(*pDevice,true);
    }
}

void CBurnWrap::GetBurnStateInformation(BURN_STATE_INFORMATION &burnStateInfo) const
{
    burnStateInfo.nState=progressDlg.GetBurnState();
    wcscpy_s(burnStateInfo.wcszBurnStateDescription,sizeof(burnStateInfo.wcszBurnStateDescription)/sizeof(wchar_t),
        progressDlg.GetBurnStateDescription());
    burnStateInfo.ucBurnedPercentage=m_core.GetBurnedPercentage();
}
