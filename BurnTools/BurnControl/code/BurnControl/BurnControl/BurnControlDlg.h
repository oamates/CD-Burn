
// BurnControlDlg.h : ͷ�ļ�
//

#pragma once

#include "afxwin.h"

#include "BurnInfo.h"
#include "InterfaceProtocol.h"
#include "PictureListCtrl.h"

#include "InterfaceServer.h"

#include <string>
#include <vector>

#define LOCATIONMAXNUM 2

#define WMUSER_BURNCONTROLDLG_ONRECVPROTOCOL_MSG  _T("WMUSER_BURNCONTROLDLG_ONRECVPROTOCOL-{F2B068FD-DF8B-44a2-A81E-AECC6849FAFC}")
DECLARE_USER_MESSAGE(WMUSER_BURNCONTROLDLG_ONRECVPROTOCOL)

#define WMUSER_BURNCONTROLDLG_BURNBACKUPYES_MSG  _T("WMUSER_BURNCONTROLDLG_BURNBACKUPYES-{77E5311C-C203-4d2c-BAFF-901F082B8C62}")
DECLARE_USER_MESSAGE(WMUSER_BURNCONTROLDLG_BURNBACKUPYES)

#define WMUSER_BURNCONTROLDLG_BURNBACKUPNO_MSG  _T("WMUSER_BURNCONTROLDLG_BURNBACKUPNO-{B8EE4183-4ABB-4584-A00E-AE14BAF23596}")
DECLARE_USER_MESSAGE(WMUSER_BURNCONTROLDLG_BURNBACKUPNO)

#define WMUSER_BURNCONTROLDLG_BURNBACKUPAFTERRESTARTYES_MSG  _T("WMUSER_BURNCONTROLDLG_BURNBACKUPAFTERRESTARTYES-{8E5E4F6C-E50D-4b08-9500-155647CD460C}")
DECLARE_USER_MESSAGE(WMUSER_BURNCONTROLDLG_BURNBACKUPAFTERRESTARTYES)

#define WMUSER_BURNCONTROLDLG_BURNBACKUPAFTERRESTARTNO_MSG  _T("WMUSER_BURNCONTROLDLG_BURNBACKUPAFTERRESTARTNO-{E61220C0-BBE7-41ea-870C-3CA4A524F5B8}")
DECLARE_USER_MESSAGE(WMUSER_BURNCONTROLDLG_BURNBACKUPAFTERRESTARTNO)

#define WMUSER_BURNCONTROLDLG_SENDCMDFAILED_MSG  _T("WMUSER_BURNCONTROLDLG_SENDCMDFAILED-{8B6F6C3F-3175-4164-B0E1-17070E0BA300}")
DECLARE_USER_MESSAGE(WMUSER_BURNCONTROLDLG_SENDCMDFAILED)

#define WMUSER_BURNCONTROLDLG_ASYNCMSGRESULT_MSG  _T("WMUSER_BURNCONTROLDLG_ASYNCMSGRESULT-{9629F1B3-FD47-4a6b-BF23-2644E5B97EBB}")
DECLARE_USER_MESSAGE(WMUSER_BURNCONTROLDLG_ASYNCMSGRESULT)


#define STATE_CONNECT_SERVER_FAILED         "-20"

//ͨ������״̬��ʶ
#define STATE_SINGLE_DEV_ERR_JOB_FAILED     "-3"
#define STATE_SINGLE_DEV_ERR                "-2"
#define STATE_BURN_ERR                      "-1"
#define STATE_DEFAULT                       "0"
#define STATE_BURNING                       "1"
#define STATE_SINGLE_DISC_BURNED            "2"
#define STATE_DOWNLOADING                   "3"
#define STATE_WILL_CLOSE_DISC               "4"
#define STATE_CHANGE_DISC                   "5"
#define STATE_TASK_OVER                     "6"
#define STATE_JOB_OVER                      "7"
#define STATE_JOB_FINISHED                  "8"
#define STATE_RTJOB_PAUSED                  "9"
#define STATE_RTJOB_STOPPED                 "10"
#define STATE_RTTASK_STOPPING               "14"//������¼ҵ����һ��Task����ֹͣ����ʱ����¼���ư�ť������


//�벹���йص�����״̬��ʶ
#define EXTRA_STATE_RTBURN_BACKUP_NONE          "49"
#define EXTRA_STATE_RTBURN_BACKUP_YES           "50"
#define EXTRA_STATE_RTBURN_BACKUP_NO            "51"
#define EXTRA_STATE_RTBURN_BACKUP_RESTART_YES   "52"
#define EXTRA_STATE_RTBURN_BACKUP_RESTART_NO    "53"

//����״̬����
#define STATE_PROPERTY_NORMAL       "1"
#define STATE_PROPERTY_PRIMERA      "2"
#define STATE_PROPERTY_EPSON        "3"
#define STATE_PROPERTY_RT           "4"
#define STATE_PROPERTY_RTBACKUP     "5"

//ͨ��ncx���͵��첽��Ϣ������
typedef enum
{
    START_RT_BURN=1,
    START_BURN,
    SEND_RT_BURN_CONFIG,
    SEND_RT_BURN_REQUEST,
    SEND_BUTN_CTRL_CMD
}E_COMMAND_TYPE;

//#define BURNCONTROL_TEST

class ConfigurableFileEx : public ConfigurableFile 
{
public:
    ConfigurableFileEx();
    ~ConfigurableFileEx();

    bool GetRect(std::string sSectionName,CRect &rect);
protected:
private:
};

// CBurnControlDlg �Ի���
class CBurnControlDlg : public CDialogCustom
{
    // ����
public:
    CBurnControlDlg(CWnd* pParent = NULL);	// ��׼���캯��
    ~CBurnControlDlg();

    // �Ի�������
    enum { IDD = IDD_BURNCONTROL_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


    // ʵ��
protected:
    HICON m_hIcon;

    // ���ɵ���Ϣӳ�亯��
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

    afx_msg void OnDestroy( );

    afx_msg void OnBnClickedButtonDown();
    afx_msg void OnBnClickedButtonUp();

    afx_msg void OnCbnSelchangeComboRecorder();

    afx_msg void OnBnClickedRadioSingle();
    afx_msg void OnBnClickedRadioMultisyn();
    afx_msg void OnBnClickedRadioMultiasyn();

    afx_msg void OnLvnItemchangedListSelectcdrom(NMHDR *pNMHDR, LRESULT *pResult);

    afx_msg void OnBnClickedCheckBurnpassword();

    afx_msg void OnBnClickedButtonBurn();
    afx_msg void OnBnClickedButtonPause();
    afx_msg void OnBnClickedButtonSave();
    afx_msg void OnBnClickedButtonClose();

    afx_msg LRESULT OnTrayLButtonDBLClk(WPARAM wParam,LPARAM lParam);

    afx_msg void OnTimer(UINT_PTR nIDEvent);

    afx_msg void OnNMDblclkListJoblist(NMHDR *pNMHDR, LRESULT *pResult);

    afx_msg void OnEnKillfocusEditItem();

    afx_msg void OnSubQuit();

    afx_msg void OnEnUpdateEditRecorderinfo();

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

    afx_msg BOOL OnEraseBkgnd(CDC* pDC);

    afx_msg void OnClose();

    virtual BOOL PreTranslateMessage(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

    afx_msg void OnBnClickedCheckL1();
    afx_msg void OnBnClickedCheckL2();
    afx_msg void OnBnClickedCheckL3();
    afx_msg void OnBnClickedCheckL4();
    afx_msg void OnBnClickedCheckL5();
    afx_msg void OnBnClickedCheckL6();
    afx_msg void OnBnClickedCheckL7();
    afx_msg void OnBnClickedCheckL8();
    afx_msg void OnBnClickedCheckL9();
    afx_msg void OnBnClickedCheckL10();

public:

    static BOOL WcToMb(LPCWSTR pwcszSrc,LPSTR pszDst,int nBufSize);

    //��ȡ������ʾ������Ϣ�������ڳ�������ʱִ��
    void GetAndDisplayCDROMDriverInfo();

    void AddBurnServer(const std::vector<CBurnServerInfoEx> &vecBurnServer);

    //ѯ��BurnServer����ȡ�ϲ����BurnServer�Ƿ���ã�ͬʱ��ȡBurnServer�¹��صĿ�¼����Ϣ��
    // ��������˶������͵Ŀ�¼��������ͨ��¼����+�����ſ�¼��+��������¼��������ô��ԭ����
    // ��¼�������ֽ�Ϊ�������͵Ķ����¼������
    void UpdateBurnServerInfo();

    CStringW GetAlertMsg() const;
    void SetAlertMsg(CStringW val);

    std::string GetAlertFlag() const;
    void SetAlertFlag(std::string val);

    void SetConnectServerFailedCount(int n);

    int GetConnectServerFailedCount();

    bool GetDevUpdated() const;
    void SetDevUpdated(bool val);

private:
    static void OnRecvProtocol(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam,void *pUsrParam);
    LRESULT OnRecvProtocol(WPARAM wParam, LPARAM lParam);

#define RTBURN_BACKUP_YES   "1"
#define RTBURN_BACKUP_NO    "2"

    LRESULT OnBurnBackupYes(WPARAM wParam, LPARAM lParam);
    LRESULT OnBurnBackupNo(WPARAM wParam, LPARAM lParam);

#define RTBURN_BACKUP_AFTER_RESTART_YES   "3"
#define RTBURN_BACKUP_AFTER_RESTART_NO    "4"
    LRESULT OnBurnBackupAfterRestartYes(WPARAM wParam, LPARAM lParam);
    LRESULT OnBurnBackupAfterRestartNo(WPARAM wParam, LPARAM lParam);

    LRESULT OnSendCmdFailed(WPARAM wParam, LPARAM lParam);
    LRESULT OnAsyncMsgResult(WPARAM wParam, LPARAM lParam);

    void DealWithBurnStateCallBackMsg(std::string strJobID,std::string strStateFlag,
        std::string strStateDescription,const std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo);

    void UpdateUIJobStateInformation(std::string strStateDescription);

    //��Щ״̬���Ա������������й���
    void SetCDROMDriverCommonState(std::string strStateFlag,std::string strStateDescription,
        std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo);

    void ProcessJobDone();
    void ProcessJobFailed(std::string strStateFlag="",std::string strStateDescription="");

    void CleanJobData();

    //���������ļ�
    void InitConfig();

    void Layout(ConfigurableFileEx cfg,std::string strSection,CWnd *pWnd);

    void ClearDevBurnStateInfo();//��������¼״̬��Ϣ

    void CreateUI(std::string strUICfgFile);

    typedef enum _BURNTYPESEL_
    {
        BURNTYPE_SINGLE=1,
        BURNTYPE_MULTISYN,
        BURNTYPE_MULTIASYN,
        BURNTYPE_UNKNOWNSEL
    }BURNTYPESEL;
    BURNTYPESEL GetCurBurnTypeSel();//��ȡUI��ǰѡ��Ŀ�¼��ʽ
    void SetCurBurnTypeSel(BURNTYPESEL burnTypeSel);
    
    bool GetCurBurnServerSel(CBurnServerInfo &burnServerInfo);//��ȡ��ǰѡ��Ŀ�¼��������Ϣ
    
    std::vector<CCDROMDriverInfo> GetCurCDROMDriverInfoVec();

    //��ȡBurnServer���ƵĹ�����Ϣ
    bool GetDevInfo(std::string strIP,std::string strPort,std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo);

    void UpdateCDROMDriverInfo(const std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo);
    
    std::vector<CLocationInfo> GetVecLocationInfo();//��ȡ��¼������Ϣ�����ص���ԭʼ��Э�����������
    
    std::string GetCurJobID();
    
    void GetBurnServerAddress(std::string &strIP,int &nPort);

    void HideJobList();
    void HideCDName();
    void HideBurnContent();
    void HideRecorder();
    void HideFileInfo();

    void DisplayBurnContentUI();//��ʾ��¼�������Ϣ
    void DisplayBurnServer(const std::vector<CBurnServerInfoEx> &vecBurnServerInfoEx);//��ʾ��¼�������б�

    //����ʾ��¼��ʽʱ�����ֻ��һ����������ôֻ��ѡ���̿�¼��������Ҫ��һ����ȷ�ĳ�ʼֵ
    void SetInitialBurnTypeUI(BURNTYPESEL eBurnType);

    std::string GetBurnServerType();

    void DisplayBurnTypeUI(BURNTYPESEL eBurnType);

#define DEV_UI_FLAG_CREATE  0
#define DEV_UI_FLAG_UPDATE  1
    void DisplayCDROMUI(int nFlag);//��ʾ�����б�nFlag 0���������ɣ�1��������

    void DisplayPasswordUI(BOOL bBurnPassword,BOOL bContentPassword);//bBurnPassword���Ƿ�ѡ�������룬bContentPassword���Ƿ�ѡ���ݼ���

    void DisplayJobListUI();

    void ShowBurnInfoUI(bool bUpdateServerAndCDROM = true);//��UI��ʾЭ���а���������
    void ShowBurnControlUI(bool bShowUI);

    int GetCheckedContent();//��ȡ��ѡ�Ļ���������������û�й�ѡ���棬�������¼

    void SaveBurnContentInfo();//���湴ѡ����Щ����
    BOOL SavePassword();//����ֵ�����ж��û���ѡ���������뵫��û���������룬�������벻һ�µ�����
    void SaveCDName();
    void SaveBurnServerInfo();
    void SaveCDROMDriverBurnContent();//����ÿ������Ҫ�̵����ݣ��˺����Ĺ����Ǵ���ǰԭ�ͽ���Ĳ���������Ժ���չΪ��ͬ�Ĺ���ͬʱ��¼��ͬ�����ݣ���ôҲҪ��Ӧ�޸ĺ�������

    void SaveRTBurnCfg();//�����ò������浽���ݽṹ��
    void SaveNoneRTBurnCosCfg();
    void SaveNoneRTBurnSingleCfg();
    void SaveNoneRTBurnMultiCfg();

    void CreateAdditionalBurnFile();

    //���ɲ����б��ļ���ʵʱ��¼���º��¼���ɹ���ͬ
    void CreatePlayListFile(CRTBurnInfo &RTBurnInfo);
    void CreatePlayListFile(CNormalBurnJobInfoEx &jobInfo);

    //�����Զ������ļ���ʵʱ��¼���º��¼���ɹ���ͬ
    void CreateAutorunFile(CRTBurnInfo &RTBurnInfo);
    void CreateAutorunFile(CNormalBurnJobInfoEx &jobInfo);


    void SendBurnJobToBurnServer();

    void SendBurnControlCMDToBurnServer(std::string strCMD);

    void SendGetJobStateMessage(std::string strIP,std::string strPort,std::string strJobID);

    void SendCurBurnStateToUpper(std::string strIP,std::string strPort,std::string strJobID,
        std::string strBurnState,std::string strBurnStateDescription);

    bool GetBurnFilesState(const std::vector<CCDROMDriverInfo> &vecDevInfo,
        std::map< int,std::vector<CSingleFileInfo> > &mapBurnFileInfo);//�ӹ����б���ȡ��ǰ������ļ��Ŀ�¼״̬��Ϣ

    void UpdateCDROMState(const std::vector<CCDROMDriverInfo> &vecCDRONInfo);//����ÿ�������Ŀ�¼״̬

    void UpdateBurnFilesState(const std::vector<CCDROMDriverInfo> &vecCDRONInfo);//���¿�¼�ļ���Ϣ

    void EnableBurnContent(bool b);

    void EnableBurnParam(bool b);

    void EnableBurnPassword(bool b);

    void EnableBurnCDName(bool b);

    void EnableBurnJobList(bool b);

    void UpdateBurnControlUI();//���ݵ�ǰ��¼״̬��������Ӧҵ���߼���ʾ����

    void ShowBurnControlUIIdle();//�ȴ���������״̬
    void ShowBurnControlUIBurning();//��¼״̬
    void ShowBurnControlUIPause();//��¼��ͣ״̬

    void ShowBurnControlUIUnable();

    void SetTopBorderHeight(int nHeight);
    void SetBottomBorderHeight(int nHeight);
    void SetTitleText(CString strTitleText);

    void StartRtBurn(bool bToServer);
    void StopRtBurn(bool bToServer);
    void PauseRtBurn(bool bToServer);
    void ResumeRtBurn(bool bToServer);

    void ResetBurnInfo();

    void UpdateBurnServerUIInfo();

    typedef enum _ITEM_TYPE
    {
        ITEM_TYPE_EDIT=0,
        ITEM_TYPE_UNKNOWN
    }ITEM_TYPE;

    ITEM_TYPE GetItemType(CListCtrl *pListCtrl,int nItem,int nSubItem);

    void AttachEdit(CListCtrl *pListCtrl,int nItem,int nSubItem);

    //���浯����Ϣ�����ṹ��
    typedef struct _DESKTOP_ALERT_PARAM_ 
    {
        int nAnimation;
        int nAnimationSpeed;
        int nTransparency;
        BOOL bSmallCaption;
        BOOL bAutoClose;
        int nAutoCloseTime;

        std::string strFlag;//�¼���ʶ
        std::string strDescription;//�¼�����

        _DESKTOP_ALERT_PARAM_()
        {
            nAnimation=3;
            nAnimationSpeed=30;
            nTransparency=255;
            bSmallCaption=FALSE;
            bAutoClose=FALSE;
            nAutoCloseTime=0;

            strFlag="";
            strDescription="";
        }
    }DESKTOP_ALERT_PARAM;
    void ShowDesktopAlert(const DESKTOP_ALERT_PARAM &param);

    void SendKeepAliveMsg();

    void AsyncSendNCXMsg(std::string strIP,std::string strPort,std::string strContent);

    int GetBurnLocationCheckedNum();

    void EnableBurnLocationUnChecked(BOOL bEnable);

    void UpdateBurnContentShowStatus();

    void ReadRTBBackupXML(CRTBurnInfo &rtBurnInfo);

    void WriteRTBBackupXML(CRTBurnInfo rtBurnInfo);

    bool DeleteRTBBackupXML();
private:
    typedef enum _BURNCATEGORY_
    {
        BURNCATEGORY_SINGLE=0,
        BURNCATEGORY_MULTI,
        BURNCATEGORY_COS,
        BURNCATEGORY_RTBURN,
        BURNCATEGORY_HM3,
        BURNCATEGORY_UNKNOWN
    }BURNCATEGORY;

    BURNCATEGORY                        m_eBurnCategory;//��ͬҵ�����͵Ŀ�¼
    CRTBurnInfo                         m_RTBurnInfo;//���浱ǰ�����ʵʱ��¼����Ϣ
    CNormalBurnInfo                     m_normalJob;//���浱ǰ������º��¼��Ϣ
    size_t                              m_nJobIndex;//������ʱ����ǰ��������
    std::string                         m_strProtocol;//��ǰ����Э�飬�������쳣�ָ�

#define BURNCONTROL_STATE_ERROR     -2
#define BURNCONTROL_STATE_IDLE      -1
#define BURNCONTROL_STATE_READY     0
#define BURNCONTROL_STATE_RUNNING   1
#define BURNCONTROL_STATE_PAUSED    2
#define BURNCONTROL_STATE_PAUSING   3
#define BURNCONTROL_STATE_STOPPING  4
    int                                 m_nBurnState;//��¼״̬��-1���ȴ���������0��׼����1�����ڿ�¼��2����ͣ

    typedef enum
    {
        BURNCONTROL_CMD_START_RESUME=0,
        BURNCONTROL_CMD_PAUSE_STOP
    }BURNCONTROL_CMD;
    BURNCONTROL_CMD m_eBurnControlCmd;//��¼ʵʱ��¼��һ�εĿ�¼��������

    std::vector<CBurnServerInfoEx>      m_vecBurnServerInfoEx;//��¼ÿһ��BurnServerÿһ�ֿ�¼��ʽ�Ĺ���ѡ����Ϣ
    ZOSMutex    m_mutexBurnServer;

    CCustomStatic   m_staticBurnParam;
    CCustomStatic   m_staticBurnPassword;
    CCustomStatic   m_staticFileInfo;
    CCustomStatic   m_staticBurnState;
    CCustomStatic   m_staticCDName;
    CCustomStatic   m_staticJobList;
    CCustomStatic   m_staticBurnContent;

    CButtonST       m_btnstJobUp;
    CButtonST       m_btnstJobDown;

    CButtonST       m_btnstStartBurn;
    CButtonST       m_btnstClose;
    CButtonST       m_btnstSave;
    CButtonST       m_btnstPause;

    CBitmap         m_bmpBurnPic;

    CSystemTray     m_trayIcon;

    ZOSMutex        m_mutexJobVec;


    CPictureListCtrl m_listCtrlJobList;

    ConfigurableFileEx m_cfgFile;

    CRect           m_rectBeforeMinimize;

    CPicStatic      m_staticBurnPic;

    int             m_nConnectServerFailedCount;
    ZOSMutex        m_mutexConnectServerFailedCount;

    bool            m_bDevUpdated;//�ж��Ƿ��Ѿ���BurnServer��ȡ�Ĺ�����Ϣ

    CStringW        m_strAlertMsg;//���浯�����ڵ���Ϣ����
    std::string     m_strAlertFlag;//��������Ϣ���ͱ�ʶ

    InterfaceServer m_interfaceServer;
    bool            m_bStartServer;
    CEdit m_editItem;
    int m_nItem;//�༭��ǰ��������
    int m_nSubItem;

    bool m_bToExit;//�յ��رյ���Ϣʱ�Ƿ������˳�����

    bool m_bBackupJob;
    BURNTYPESEL m_curBurnTypeSel;
};
