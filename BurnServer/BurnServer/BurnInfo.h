#ifndef _BURN_INFO_H_
#define _BURN_INFO_H_

#include <vector>
#include <string>
#include <map>

#include "FileUtil.h"

//////////////////////////////////////////////////////////////////////////
class CSingleFileInfo
{
public:
    CSingleFileInfo();
    ~CSingleFileInfo();

    std::string GetFileName() const;
    void SetFileName(std::string val);

    std::string GetNewFileName() const;
    void SetNewFileName(std::string val);

    std::string GetBurnState() const;
    void SetBurnState(std::string val);

    std::string GetSize() const;
    void SetSize(std::string val);

    std::string GetDownloadPercent() const;
    void SetDownloadPercent(std::string val);
private:
    std::string     m_strFileName;//ȫ·��
    std::string     m_strNewFileName;//����������
    std::string     m_strBurnState;//��¼״̬��0���ȴ���¼��1�����ڿ�¼��2����¼���
    std::string     m_strSize;
    std::string     m_strDownloadPercent;//���ؽ���
};

//////////////////////////////////////////////////////////////////////////
class CLocationInfo
{
public:
    CLocationInfo(void);
    ~CLocationInfo(void);
    CLocationInfo(const CLocationInfo &videoLocationInfo);
    CLocationInfo & operator=(const CLocationInfo &videoLocationInfo);

    std::string GetID() const;
    void SetID(std::string val);

    std::string GetDescription() const;
    void SetDescription(std::string val);

    std::string GetBurnOrNot() const;
    void SetBurnOrNot(std::string val);

    std::string GetStreamType() const;
    void SetStreamType(std::string val);

    std::string GetURL() const;
    void SetURL(std::string val);

    std::vector<CSingleFileInfo>  & GetVecFileInfo();
    const std::vector<CSingleFileInfo>  & GetVecFileInfo() const;
    void SetVecFileInfo(std::vector<CSingleFileInfo> val);
private:
    std::string                 m_strID;
    std::string                 m_strDescription;
    std::string                 m_strBurnOrNot;//�Ƿ�Ĭ��Ҫ��¼��0����1����
    std::string                 m_strStreamType;//��Ƶ�����ͣ�����ֵ��ȷ��????
    std::string                 m_strURL;//����ʵʱ��¼
    std::vector<CSingleFileInfo> m_vecFileInfo;//�����º��¼
};


//////////////////////////////////////////////////////////////////////////
//�涨��¼���������������б��ļ��ϲ���һ��CLocationInfo����������λ�ã�m_strID=-1
class CCDROMDriverInfo
{
public:
    CCDROMDriverInfo(void);
    ~CCDROMDriverInfo(void);

    CCDROMDriverInfo(const CCDROMDriverInfo &CDROMDriverInfo);
    CCDROMDriverInfo & operator=(const CCDROMDriverInfo &CDROMDriverInfo);

    std::string GetID() const;
    void SetID(std::string val);

    std::string GetDescription() const;
    void SetDescription(std::string val);

    std::string GetIsSelected() const;
    void SetIsSelected(std::string val);

    std::vector<CLocationInfo>  & GetVecLocationInfo();
    const std::vector<CLocationInfo> & GetVecLocationInfo() const;
    void SetVecLocationInfo(std::vector<CLocationInfo> val);

    std::string GetStateFlag() const;
    void SetStateFlag(std::string val);

    std::string GetStateDescription() const;
    void SetStateDescription(std::string val);

    std::string GetJobID() const;
    void SetJobID(std::string val);

    std::string GetHasCD() const ;
    void SetHasCD(std::string val);

    std::string GetCDLeftCapacity() const;
    void SetCDLeftCapacity(std::string val);

    std::string GetCDTotalCapacity() const;
    void SetCDTotalCapacity(std::string val);
private:
    std::string                     m_strID;
    std::string                     m_strDescription;
    std::string                     m_strIsSelected;//���ڽ����߼�����2δѡ��1����ѡ��
    std::vector<CLocationInfo>      m_vecVideoLocationInfo;//��¼����Ƶ�����б�
    std::string                     m_strStateFlag;//����¼Э�顷�������ͽӿ�2.7.4
    std::string                     m_strStateDescription;

    std::string                     m_strJobID;//�������ڱ���һ������ʹ��

#define DEV_HAS_CD_YES  "1"
#define DEV_HAS_CD_NO   "0"
    std::string                     m_strHasCD;//�Ƿ������̣�1���ǣ���1����

    std::string                     m_strCDLeftCapacity;//����ʣ������
    std::string                     m_strCDTotalCapacity;//����������
};

//////////////////////////////////////////////////////////////////////////
class CPlaylistFileInfo
{
public:
    CPlaylistFileInfo();
    ~CPlaylistFileInfo();

    std::string GetDownloadURL() const;
    void SetDownloadURL(std::string val);

    std::string GetRemoteIP() const;
    void SetRemoteIP(std::string val);

    std::string GetRemotePort() const;
    void SetRemotePort(std::string val);
private:
    std::string m_strDownloadURL;
    std::string m_strRemoteIP;
    std::string m_strRemotePort;
};

//////////////////////////////////////////////////////////////////////////
class CPlayerInfo
{
public:
    CPlayerInfo(void);
    ~CPlayerInfo(void);

    std::string GetVersion() const;
    void SetVersion(std::string val);

    std::string GetDownloadURL() const;
    void SetDownloadURL(std::string val);

    std::string GetRemoteIP() const;
    void SetRemoteIP(std::string val);

    std::string GetRemotePort() const;
    void SetRemotePort(std::string val);
private:
    std::string m_strVersion;
    std::string m_strDownloadURL;
    std::string m_strRemoteIP;
    std::string m_strRemotePort;
};

//////////////////////////////////////////////////////////////////////////
class CPrintPictureInfo
{
public:
    CPrintPictureInfo();
    ~CPrintPictureInfo();

    std::string GetDownloadURL() const;
    void SetDownloadURL(std::string val);

    std::string GetRemoteIP() const;
    void SetRemoteIP(std::string val);

    std::string GetRemotePort() const;
    void SetRemotePort(std::string val);
private:
    std::string m_strDownloadURL;
    std::string m_strRemoteIP;
    std::string m_strRemotePort;
};

//////////////////////////////////////////////////////////////////////////
#define BURNSERVER_TYPE_NORMAL      "1"
#define BURNSERVER_TYPE_PRIMERA     "2"
#define BURNSERVER_TYPE_EPSON       "4"
class CBurnServerInfo
{
public:
    CBurnServerInfo();
    ~CBurnServerInfo();

    CBurnServerInfo(const CBurnServerInfo &burnServerInfo);
    CBurnServerInfo & operator=(const CBurnServerInfo &burnServerInfo);

    std::string GetIP() const;
    void SetIP(std::string val);

    std::string GetPort() const;
    void SetPort(std::string val);

    std::string GetBurnServerName() const;
    void SetBurnServerName(std::string val);

    std::string GetDescription() const;
    void SetDescription(std::string val);

    const std::vector<CCDROMDriverInfo> & GetVecCDROMDriverInfo() const;
    std::vector<CCDROMDriverInfo> & GetVecCDROMDriverInfo();
    void SetVecCDROMInfo(std::vector<CCDROMDriverInfo> val);

    std::string GetBurnServerType() const;
    void SetBurnServerType(std::string val);

    std::string GetBurnServerTypeDescription() const;
    void SetBurnServerTypeDescription(std::string val);

    std::string GetIsDefault() const;
    void SetIsDefault(std::string val);

    std::string GetIsAvailable() const;
    void SetIsAvailable(std::string val);

protected:
    std::string             m_strIP;
    std::string             m_strPort;
    std::string             m_strBurnServerName;
    std::string             m_strDescription;
    std::vector<CCDROMDriverInfo> m_vecCDROMInfo;
    std::string             m_strBurnServerType;//��¼���������ͣ������Ƿ�֧��������¼ ��1����ͨ��¼��������Ĭ�ϵģ���2�������ſ�¼����������4����������¼������
    std::string             m_strBurnServerTypeDescription;//��¼����������������Ϣ
    std::string             m_strIsDefault;//�Ƿ���ΪĬ�ϵĿ�¼�����������ж����¼����������ĳһ������û��ָ����¼�������������ʹ�ã�0����1����
    std::string             m_strIsAvailable;//�Ƿ���ã�����չʾ֮ǰ���������¼����������ͨ��,��0���񣬡�1����
};

//////////////////////////////////////////////////////////////////////////
class CCommonBurnParam
{
public:
    CCommonBurnParam();
    ~CCommonBurnParam();

    std::string GetBurnType() const;
    void SetBurnType(std::string val);

    std::string GetBurnPassword() const;
    void SetBurnPassword(std::string val);

    std::string GetContentPassword() const;
    void SetContentPassword(std::string val);

    std::string GetCDName() const;
    void SetCDName(std::string val);

private:
#define BURN_TYPE_SINGLE        "1"
#define BURN_TYPE_MULTI_SYN     "2"
#define BURN_TYPE_MULTI_ASYN    "3"
    std::string     m_strBurnType;

    std::string     m_strBurnPassword;
    std::string     m_strContentPassword;
    std::string     m_strCDName;
};

//////////////////////////////////////////////////////////////////////////
class CRTBurnInfo
{
public:
    CRTBurnInfo(void);
    ~CRTBurnInfo(void);

    CRTBurnInfo(const CRTBurnInfo &RTBurnInfo);
    CRTBurnInfo & operator=(const CRTBurnInfo &RTBurnInfo);

    std::string GetJobID() const;
    void SetJobID(std::string val);

    std::string GetJobType() const;
    void SetJobType(std::string val);

    const CBurnServerInfo & GetBurnServerInfo() const;
    CBurnServerInfo  & GetBurnServerInfo();
    void SetBurnServerInfo(CBurnServerInfo val);

    const CCommonBurnParam & GetCommonBurnParam() const;
    CCommonBurnParam & GetCommonBurnParam();
    void SetCommonBurnParam(CCommonBurnParam val) ;

    const std::vector<CLocationInfo> & GetVecVideoLocationInfo() const;
    std::vector<CLocationInfo> & GetVecVideoLocationInfo();
    void SetVecVideoLocationInfo(std::vector<CLocationInfo> val);

    std::string GetUpServerIP() const ;
    void SetUpServerIP(std::string val) ;

    std::string GetUpServerPort() const ;
    void SetUpServerPort(std::string val);

    std::string GetNewFileName() const;
    void SetNewFileName(std::string val);

    std::string GetJobFlag() const;
    void SetJobFlag(std::string val);

    std::string GetStateFlag() const;
    void SetStateFlag(std::string val);

    std::string GetStateDescription() const;
    void SetStateDescription(std::string val);

    const CPlayerInfo & GetPlayerInfo() const;
    CPlayerInfo & GetPlayerInfo();
    void SetPlayerInfo(CPlayerInfo val);

    std::string GetPlayListContent() const;
    void SetPlayListContent(std::string val);

    std::string GetNoteFileRemoteIP() const;
    void SetNoteFileRemoteIP(std::string val);

    std::string GetNoteFileRemotePort() const;
    void SetNoteFileRemotePort(std::string val);

    std::string GetNoteFileDownloadURL() const;
    void SetNoteFileDownloadURL(std::string val);

    const CPlaylistFileInfo & GetPlaylistFileInfo() const;
    CPlaylistFileInfo & GetPlaylistFileInfo();
    void SetPlaylistFileInfo(CPlaylistFileInfo val);

    std::string GetViewCount() const;
    void SetViewCount(std::string val);

    std::string GetAutorunFileRemoteIP() const;
    void SetAutorunFileRemoteIP(std::string val);

    std::string GetAutorunFileRemotePort() const;
    void SetAutorunFileRemotePort(std::string val);

    std::string GetAutorunFilePath() const;
    void SetAutorunFilePath(std::string val);
protected:
    std::string                     m_strJobID;
    std::string                     m_strJobType;//���ʹ�Э�����Ϊ������ʵʱ��¼����"2"���������ÿ�¼����Ȼ�����������ʼ��¼"1"
    CBurnServerInfo                 m_BurnServerInfo;
    CCommonBurnParam                m_CommonBurnParam;//��¼��ʽ����¼�������á��������ƣ�ʵʱ��¼Э������õڶ���
    std::vector<CLocationInfo>      m_vecLocationInfo;
    std::string                     m_strUpServerIP;
    std::string                     m_strUpServerPort;
    std::string                     m_strNewFileName;//TS�ļ�������
    std::string                     m_strJobFlag;//��Ҫ�������ã����������¼
    std::string                     m_strStateFlag;
    std::string                     m_strStateDescription;
    CPlayerInfo                     m_PlayerInfo;
    std::string                     m_strPlayListContent;
    std::string                     m_strNoteFileRemoteIP;
    std::string                     m_strNoteFileRemotePort;
    std::string                     m_strNoteFilePath;
    std::string                     m_strAutorunFileRemoteIP;//��BurnServer���ݵ�autorun.inf�ļ���Ϣ
    std::string                     m_strAutorunFileRemotePort;
    std::string                     m_strAutorunFilePath;
    CPlaylistFileInfo               m_PlaylistFileInfo;
    std::string                     m_strViewCount;
};

//////////////////////////////////////////////////////////////////////////
class CNormalBurnJobInfo
{
public:
    CNormalBurnJobInfo();
    ~CNormalBurnJobInfo();

    CNormalBurnJobInfo(const CNormalBurnJobInfo &normalBurnJobInfo);
    CNormalBurnJobInfo & operator=(const CNormalBurnJobInfo &normalBurnJobInfo);

    std::string GetJobID() const;
    void SetJobID(std::string val);

    std::string GetJobName() const;
    void SetJobName(std::string val);

    std::string GetJobDescription() const;
    void SetJobDescription(std::string val);

    const CBurnServerInfo & GetBurnServerInfo() const;
    CBurnServerInfo & GetBurnServerInfo();
    void SetBurnServerInfo(CBurnServerInfo val);

    const CCommonBurnParam & GetCommonBurnParam() const;
    CCommonBurnParam & GetCommonBurnParam();
    void SetCommonBurnParam(CCommonBurnParam val);

    const CPlayerInfo & GetPlayerInfo() const;
    CPlayerInfo & GetPlayerInfo();
    void SetPlayerInfo(CPlayerInfo val);

    std::string GetNoteFileRemoteIP() const;
    void SetNoteFileRemoteIP(std::string val);

    std::string GetNoteFileRemotePort() const;
    void SetNoteFileRemotePort(std::string val);

    std::string GetNoteRelativePath() const;
    void SetNoteRelativePath(std::string val);

    std::string GetNoteFileName() const;
    void SetNoteFileName(std::string val);

    std::string GetAutorunFileRemoteIP() const;
    void SetAutorunFileRemoteIP(std::string val);

    std::string GetAutorunFileRemotePort() const;
    void SetAutorunFileRemotePort(std::string val);

    std::string GetAutorunFilePath() const;
    void SetAutorunFilePath(std::string val);

    std::string GetVideoFileRemoteIP() const;
    void SetVideoFileRemoteIP(std::string val);

    std::string GetVideoFileRemotePort() const;
    void SetVideoFileRemotePort(std::string val);

    std::string GetVideoFileRelativePath() const;
    void SetVideoFileRelativePath(std::string val);

    const std::vector<CLocationInfo> & GetVecLocationInfo() const;
    std::vector<CLocationInfo> & GetVecLocationInfo();
    void SetVecVideoLocationInfo(std::vector<CLocationInfo> val);

    std::string GetPlayListContent() const;
    void SetPlayListContent(std::string val);

    std::string GetViewCount() const;
    void SetViewCount(std::string val);

    const std::map<std::string,std::string> & GetMapLabelFieldText() const;
    std::map<std::string,std::string> & GetMapLabelFieldText();
    void SetMapLabelFieldText(std::map<std::string,std::string> val);

    const CPlaylistFileInfo & GetPlaylistFileInfo() const;
    CPlaylistFileInfo & GetPlaylistFileInfo();
    void SetPlaylistFileInfo(CPlaylistFileInfo val);

    std::string GetUpServerIP() const;
    void SetUpServerIP(std::string val);

    std::string GetUpServerPort() const;
    void SetUpServerPort(std::string val);

    std::string GetStateFlag() const;
    void SetStateFlag(std::string val);

    std::string GetStateDescription() const;
    void SetStateDescription(std::string val);
protected:
    std::string                         m_strJobID;
    std::string                         m_strJobName;
    std::string                         m_strJobDescription;
    CBurnServerInfo                     m_BurnServerInfo;
    CCommonBurnParam                    m_CommonBurnParam;
    CPlayerInfo                         m_PlayerInfo;
    std::string                         m_strNoteFileRemoteIP;//��ű�¼�ļ��ķ�������ַ
    std::string                         m_strNoteFileRemotePort;
    std::string                         m_strNoteRelativePath;
    std::string                         m_strNoteFileName;
    std::string                         m_strAutorunFileRemoteIP;//��BurnServer���ݵ�autorun.inf�ļ���Ϣ
    std::string                         m_strAutorunFileRemotePort;
    std::string                         m_strAutorunFilePath;
    std::string                         m_strVideoFileRemoteIP;//�����Ƶ�ļ��ķ�������ַ
    std::string                         m_strVideoFileRemotePort;
    std::string                         m_strVideoFileRelativePath;//��Ƶ�ļ����·��
    std::vector<CLocationInfo>          m_vecVideoLocationInfo;
    std::string                         m_strPlayListContent;//��������playlist�ļ�����Ϣ
    std::string                         m_strViewCount;
    std::map<std::string,std::string>   m_mapLabelFieldText;//�����ӡ�ֶ�
    CPlaylistFileInfo                   m_PlaylistFileInfo;//����BurnServer��playlist�ļ���Ϣ
    std::string                         m_strUpServerIP;//����Ҫ��������Ϣ�������ϲ�ʱʹ�õĵ�ַ
    std::string                         m_strUpServerPort;

    std::string                         m_strStateFlag;
    std::string                         m_strStateDescription;

};

//
//��¼ÿһ���ļ�������״̬��first��ʾ��������id,second���FILE_DOWNLOAD_INFO
typedef struct _FILE_DOWNLOAD_INFO
{
    std::string strJobID;

#define TYPE_NOTE_FILE  "0"
#define TYPE_PLAYER     "1"
#define TYPE_PLAY_LIST  "2"
#define TYPE_VIDEO      "3"
#define TYPE_AUTORUN    "4"
#define TYPE_AUTH       "5"
    std::string strType;//0����¼��1����������2��playlist��3����Ƶ�ļ���4��autorun�ļ���5��auth�ļ�

    std::string strFileName;//
    std::string strOldPercentage;
    std::string strPercentage;//���Ϊ"-1",��ʾ�����쳣
    std::string strCDROMDriverID;//����id�����Ϊ"ALL"����ʾ�������й���
    INT64 nFileTotalSize;
}FILE_DOWNLOAD_INFO;

//////////////////////////////////////////////////////////////////////////
class CNormalBurnJobInfoEx : public CNormalBurnJobInfo//����BurnServer���º��¼���Ӳ���
{
public:
    CNormalBurnJobInfoEx();
    ~CNormalBurnJobInfoEx();

    CNormalBurnJobInfoEx(const CNormalBurnJobInfoEx &normalBurnJobInfoEx);
    CNormalBurnJobInfoEx & operator=(const CNormalBurnJobInfoEx &normalBurnJobInfoEx);

    INT64 GetFileTotalSize() const;
    void SetFileTotalSize(INT64 val);

    INT64 GetFileDownloadSize() const;
    void SetFileDownloadSize(INT64 val);

    int GetTotalCount() const;
    void SetTotalCount(int val);

    int GetDownloadCount() const;
    void SetDownloadCount(int val);

    std::string GetPrimeraJobID() const;
    void SetPrimeraJobID(std::string val);

    const std::map<std::string,FILE_DOWNLOAD_INFO> & GetMapFileDownloadState() const;
    std::map<std::string,FILE_DOWNLOAD_INFO> & GetMapFileDownloadState();
    void SetMapFileDownloadState(const std::map<std::string,FILE_DOWNLOAD_INFO> &val);

    const std::map<std::string,std::string> & GetMapBurnTask() const;
    std::map<std::string,std::string> & GetMapBurnTask();
    void SetMapBurnTask(const std::map<std::string,std::string> &val);
private:
    std::string m_strCopies;
    std::string m_strDiscType;
    std::string m_strSpeed;
    std::string m_strCloseDisc;
    std::string m_strDeleteFile;

    INT64 m_nFileTotalSize;//��ǰ������Ҫ���ص��ļ��ܴ�С
    INT64 m_nFileDownloadSize;//�Ѿ����ش�С

    int m_nTotalCount;
    int m_nDownloadCount;

    std::map<std::string,FILE_DOWNLOAD_INFO> m_mapFileDownloadState;

    std::map<std::string,std::string> m_mapBurnTask;//һ����¼������ܷ�Ϊ���task��ͨ�����������Ա��������¼�������������״̬

    //����������id�������Ż�ȡ����״̬ʱ������⣬���ظ�ִ��ͬһ������ʱ��
    //��ȡ�Ŀ�������һ��ִ�е�״̬
    std::string m_strPrimeraJobID;

};

//////////////////////////////////////////////////////////////////////////
class CNormalBurnInfo
{
public:
    CNormalBurnInfo();
    ~CNormalBurnInfo();

    CNormalBurnInfo(const CNormalBurnInfo &normalBurnInfo);
    CNormalBurnInfo & operator=(const CNormalBurnInfo &normalBurnInfo);

    const std::vector<CBurnServerInfo> & GetVecBurnServerInfo() const;
    std::vector<CBurnServerInfo> & GetVecBurnServerInfo();
    void SetVecBurnServerInfo(std::vector<CBurnServerInfo> val);

    std::string  GetBurnFlag() const;
    void SetBurnFlag(std::string val);

    const CCommonBurnParam &  GetCommonBurnParam() const;
    CCommonBurnParam &  GetCommonBurnParam();
    void SetCommonBurnParam(CCommonBurnParam val);

    const CPlayerInfo &  GetPlayerInfo() const;
    CPlayerInfo &  GetPlayerInfo();
    void SetPlayerInfo(CPlayerInfo val);

    const std::vector<CNormalBurnJobInfoEx> & GetVecNormalBurnJobInfo() const;
    std::vector<CNormalBurnJobInfoEx> & GetVecNormalBurnJobInfo();
    void SetVecNormalBurnJobInfo(std::vector<CNormalBurnJobInfoEx> val);

    bool GetDefaultBurnServer(CBurnServerInfo &burnServerInfo);

private:
    std::vector<CBurnServerInfo> m_vecBurnServerInfo;//��¼�������б�
    std::string m_strBurnFlag;//��¼��ǣ��������ֵ��û�����0��CMS�º��¼��1��COS�º��¼
    CCommonBurnParam m_CommonBurnParam;//������ͨ�ò����������ʹ�ÿ�¼��ʽ����¼����
    CPlayerInfo m_PlayerInfo;
    std::vector<CNormalBurnJobInfoEx> m_vecNormalBurnJobInfoEx;//�����б�����ÿһ��Ԫ�ذ���һ�������������Ϣ
};

//����BurnServer��ʵʱ��¼���Ӳ���
#define RT_CONTROL_STATE_PAUSE      "9"
#define RT_CONTROL_STATE_STOP       "10"
#define RT_CONTROL_STATE_START      "11"
#define RT_CONTROL_STATE_UNKNOWN    "0"
class CRTBurnInfoEx : public CRTBurnInfo
{
public:
    CRTBurnInfoEx();
    ~CRTBurnInfoEx();

    std::string GetHardDiskBackup() const;
    void SetHardDiskBackup(std::string val);

    std::string GetSpeed() const;
    void SetSpeed(std::string val);

    std::string GetCDAlarmLimit() const;
    void SetCDAlarmLimit(std::string val);

    std::string GetCDAutoFormat() const;
    void SetCDAutoFormat(std::string val);

    std::string GetBurnBufferSize() const;
    void SetBurnBufferSize(std::string val);

    std::string GetHardDiskBakAlarmLimit() const;
    void SetHardDiskBakAlarmLimit(std::string val);

    std::string GetStreamType() const;
    void SetStreamType(std::string val);

    std::string GetMd5() const;
    void SetMd5(std::string val);

    const std::map<std::string,std::string> & GetMapBurnTask() const;
    std::map<std::string,std::string> & GetMapBurnTask();
    void SetMapBurnTask(const std::map<std::string,std::string> &val);

    const std::map<std::string,FILE_DOWNLOAD_INFO> & GetMapFileDownloadState() const;
    std::map<std::string,FILE_DOWNLOAD_INFO> & GetMapFileDownloadState();
    void SetMapFileDownloadState(const std::map<std::string,FILE_DOWNLOAD_INFO> &val);

    int GetTotalCount() const;
    void SetTotalCount(int val);

    int GetDownloadCount() const;
    void SetDownloadCount(int val);

    std::string GetControlState() const;
    void SetControlState(std::string val);
private:
    std::string m_strHardDiskBackup;
    std::string m_strSpeed;
    std::string m_strCDAlarmLimit;
    std::string m_strCDAutoFormat;
    std::string m_strBurnBufferSize;
    std::string m_strHardDiskBakAlarmLimit;
    std::string m_strStreamType;
    std::string m_strMd5;

    std::map<std::string,std::string> m_mapBurnTask;

    std::map<std::string,FILE_DOWNLOAD_INFO> m_mapFileDownloadState;

    int m_nTotalCount;
    int m_nDownloadCount;

    std::string m_strControlState;//���ڱ��ʵʱ��¼����״̬
};
#endif
