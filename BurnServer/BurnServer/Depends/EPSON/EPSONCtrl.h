#ifndef _EPSONCTRL_H_
#define _EPSONCTRL_H_

#include <afx.h>

#include <vector>
#include <map>

#include "PP100API.h"

class CEPSONCtrl
{
public:
    static int InitEPSONEnv();
    static void DestroyEPSONEnv();

    CEPSONCtrl();
    virtual ~CEPSONCtrl();

public:
    int InitParameter();

    virtual int CreateJob(CStringW strJobID,CStringW strVolumeLabel,std::vector<std::string> vecData);
    virtual void SetReplaceField();

    void AddFile(wchar_t *pFileName,wchar_t *pDstFileName);
    void AddDirectory(wchar_t *pSourceDirectory,wchar_t *pDestinationDirectory);

    int SubmitJob();
    void CancelJob();

    int GetEPSONJobStatus();//返回值：-1 获取失败；0 任务没有开始；1 任务进行中；2 任务成功完成；3 任务执行失败

    std::map<CStringW,CStringW> GetMapReplaceField() const;
    void SetMapReplaceField(const std::map<CStringW,CStringW> &val);

private:
    static int LoadPP100API();
    static int ConnectServer();
    static void DisconnectServer();

    static std::vector<CString> GetStringElements(CString str);
    static CStringW GetExeDirectory();

private:
    static PP100_INITIALIZE                 m_pPP100_Initialize;
    static PP100_DESTROY                    m_pPP100_Destroy;
    static PP100_CONNECTSERVER              m_pPP100_ConnectServer;
    static PP100_DISCONNECTSERVER           m_pPP100_DisconnectServer;
    static PP100_ENUMPUBLISHERS             m_pPP100_EnumPublishers;
    static PP100_GETPUBLISHERSTATUS         m_pPP100_GetPublisherStatus;
    static PP100_GETJOBSTATUS               m_pPP100_GetJobStatus;
    static PP100_CREATEJOB                  m_pPP100_CreateJob;
    static PP100_COPYJOB                    m_pPP100_CopyJob;
    static PP100_SETDISCTYPE                m_pPP100_SetDiscType;
    static PP100_SETLABEL                   m_pPP100_SetLabel;
    static PP100_GETDISCTYPE                m_pPP100_GetDiscType;
    static PP100_SETDATA                    m_pPP100_SetData;
    static PP100_GETDATA                    m_pPP100_GetData;
    static PP100_DELETEDATA                 m_pPP100_DeleteData;
    static PP100_ADDDATA                    m_pPP100_AddData;
    static PP100_SUBMITJOB                  m_pPP100_SubmitJob;
    static PP100_CANCELJOB                  m_pPP100_CancelJob;
    static PP100_SETCOPIES                  m_pPP100_SetCopies;
    static PP100_SETFORMAT                  m_pPP100_SetFormat;
    static PP100_CONVERTERRORCODETOSTRING   m_pPP100_ConvertErrorCodeToString;
    static PP100_ADDREPLACEFIELDTEXT        m_pPP100_AddReplaceFieldText;
    static PP100_SETCLOSEDISC               m_pPP100_SetCloseDisc;
    static PP100_SETVOLUMELABEL             m_pPP100_SetVolumeLabel;

    static unsigned long                    m_hServer;
    static wchar_t                          m_wcszHost[64];//本机ip地址
    static wchar_t                          m_wcszOrderFolder[256];

    wchar_t                                 m_wcszJobID[41];

    unsigned long                           m_ulDiscType;
    unsigned long                           m_ulDiscFormat;
    unsigned long                           m_ulCopies;
    wchar_t                                 m_wcszLabelFile[MAX_PATH];
    std::map<CStringW,CStringW>             m_mapReplaceField;
};
#endif
