
// BurnControlDlg.h : 头文件
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

//通用任务状态标识
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
#define STATE_RTTASK_STOPPING               "14"//整个刻录业务有一个Task处于停止过程时，刻录控制按钮不可用


//与补刻有关的任务状态标识
#define EXTRA_STATE_RTBURN_BACKUP_NONE          "49"
#define EXTRA_STATE_RTBURN_BACKUP_YES           "50"
#define EXTRA_STATE_RTBURN_BACKUP_NO            "51"
#define EXTRA_STATE_RTBURN_BACKUP_RESTART_YES   "52"
#define EXTRA_STATE_RTBURN_BACKUP_RESTART_NO    "53"

//任务状态属性
#define STATE_PROPERTY_NORMAL       "1"
#define STATE_PROPERTY_PRIMERA      "2"
#define STATE_PROPERTY_EPSON        "3"
#define STATE_PROPERTY_RT           "4"
#define STATE_PROPERTY_RTBACKUP     "5"

//通过ncx发送的异步消息的名称
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

// CBurnControlDlg 对话框
class CBurnControlDlg : public CDialogCustom
{
    // 构造
public:
    CBurnControlDlg(CWnd* pParent = NULL);	// 标准构造函数
    ~CBurnControlDlg();

    // 对话框数据
    enum { IDD = IDD_BURNCONTROL_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


    // 实现
protected:
    HICON m_hIcon;

    // 生成的消息映射函数
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

    //获取并且显示光驱信息，仅仅在程序启动时执行
    void GetAndDisplayCDROMDriverInfo();

    void AddBurnServer(const std::vector<CBurnServerInfoEx> &vecBurnServer);

    //询问BurnServer，获取上层给的BurnServer是否可用，同时获取BurnServer下挂载的刻录机信息，
    // 如果挂载了多种类型的刻录机（如普通刻录光驱+派美雅刻录机+爱普生刻录机），那么将原来的
    // 刻录服务器分解为多种类型的多个刻录服务器
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

    //有些状态可以被用来描述所有光驱
    void SetCDROMDriverCommonState(std::string strStateFlag,std::string strStateDescription,
        std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo);

    void ProcessJobDone();
    void ProcessJobFailed(std::string strStateFlag="",std::string strStateDescription="");

    void CleanJobData();

    //加载配置文件
    void InitConfig();

    void Layout(ConfigurableFileEx cfg,std::string strSection,CWnd *pWnd);

    void ClearDevBurnStateInfo();//清楚界面刻录状态信息

    void CreateUI(std::string strUICfgFile);

    typedef enum _BURNTYPESEL_
    {
        BURNTYPE_SINGLE=1,
        BURNTYPE_MULTISYN,
        BURNTYPE_MULTIASYN,
        BURNTYPE_UNKNOWNSEL
    }BURNTYPESEL;
    BURNTYPESEL GetCurBurnTypeSel();//获取UI当前选择的刻录方式
    void SetCurBurnTypeSel(BURNTYPESEL burnTypeSel);
    
    bool GetCurBurnServerSel(CBurnServerInfo &burnServerInfo);//获取当前选择的刻录服务器信息
    
    std::vector<CCDROMDriverInfo> GetCurCDROMDriverInfoVec();

    //获取BurnServer控制的光驱信息
    bool GetDevInfo(std::string strIP,std::string strPort,std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo);

    void UpdateCDROMDriverInfo(const std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo);
    
    std::vector<CLocationInfo> GetVecLocationInfo();//获取刻录内容信息，返回的是原始的协议里面的数据
    
    std::string GetCurJobID();
    
    void GetBurnServerAddress(std::string &strIP,int &nPort);

    void HideJobList();
    void HideCDName();
    void HideBurnContent();
    void HideRecorder();
    void HideFileInfo();

    void DisplayBurnContentUI();//显示刻录画面的信息
    void DisplayBurnServer(const std::vector<CBurnServerInfoEx> &vecBurnServerInfoEx);//显示刻录服务器列表

    //在显示刻录方式时，如果只有一个光驱，那么只能选择单盘刻录，但是需要有一个正确的初始值
    void SetInitialBurnTypeUI(BURNTYPESEL eBurnType);

    std::string GetBurnServerType();

    void DisplayBurnTypeUI(BURNTYPESEL eBurnType);

#define DEV_UI_FLAG_CREATE  0
#define DEV_UI_FLAG_UPDATE  1
    void DisplayCDROMUI(int nFlag);//显示光驱列表，nFlag 0：重新生成，1：仅更新

    void DisplayPasswordUI(BOOL bBurnPassword,BOOL bContentPassword);//bBurnPassword：是否勾选设置密码，bContentPassword：是否勾选内容加密

    void DisplayJobListUI();

    void ShowBurnInfoUI(bool bUpdateServerAndCDROM = true);//在UI显示协议中包含的内容
    void ShowBurnControlUI(bool bShowUI);

    int GetCheckedContent();//获取勾选的画面个数，用于如果没有勾选画面，则不允许刻录

    void SaveBurnContentInfo();//保存勾选了哪些画面
    BOOL SavePassword();//返回值用于判定用户勾选了设置密码但是没有输入密码，或者密码不一致的情形
    void SaveCDName();
    void SaveBurnServerInfo();
    void SaveCDROMDriverBurnContent();//设置每个光驱要刻的内容，此函数的功能是处理当前原型界面的操作，如果以后扩展为不同的光驱同时刻录不同的内容，那么也要相应修改函数代码

    void SaveRTBurnCfg();//将配置参数保存到数据结构里
    void SaveNoneRTBurnCosCfg();
    void SaveNoneRTBurnSingleCfg();
    void SaveNoneRTBurnMultiCfg();

    void CreateAdditionalBurnFile();

    //生成播放列表文件，实时刻录和事后刻录生成规则不同
    void CreatePlayListFile(CRTBurnInfo &RTBurnInfo);
    void CreatePlayListFile(CNormalBurnJobInfoEx &jobInfo);

    //生成自动运行文件，实时刻录和事后刻录生成规则不同
    void CreateAutorunFile(CRTBurnInfo &RTBurnInfo);
    void CreateAutorunFile(CNormalBurnJobInfoEx &jobInfo);


    void SendBurnJobToBurnServer();

    void SendBurnControlCMDToBurnServer(std::string strCMD);

    void SendGetJobStateMessage(std::string strIP,std::string strPort,std::string strJobID);

    void SendCurBurnStateToUpper(std::string strIP,std::string strPort,std::string strJobID,
        std::string strBurnState,std::string strBurnStateDescription);

    bool GetBurnFilesState(const std::vector<CCDROMDriverInfo> &vecDevInfo,
        std::map< int,std::vector<CSingleFileInfo> > &mapBurnFileInfo);//从光驱列表提取当前任务各文件的刻录状态信息

    void UpdateCDROMState(const std::vector<CCDROMDriverInfo> &vecCDRONInfo);//更新每个光驱的刻录状态

    void UpdateBurnFilesState(const std::vector<CCDROMDriverInfo> &vecCDRONInfo);//更新刻录文件信息

    void EnableBurnContent(bool b);

    void EnableBurnParam(bool b);

    void EnableBurnPassword(bool b);

    void EnableBurnCDName(bool b);

    void EnableBurnJobList(bool b);

    void UpdateBurnControlUI();//根据当前刻录状态，按照相应业务逻辑显示界面

    void ShowBurnControlUIIdle();//等待接收任务状态
    void ShowBurnControlUIBurning();//刻录状态
    void ShowBurnControlUIPause();//刻录暂停状态

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

    //桌面弹出消息参数结构体
    typedef struct _DESKTOP_ALERT_PARAM_ 
    {
        int nAnimation;
        int nAnimationSpeed;
        int nTransparency;
        BOOL bSmallCaption;
        BOOL bAutoClose;
        int nAutoCloseTime;

        std::string strFlag;//事件标识
        std::string strDescription;//事件描述

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

    BURNCATEGORY                        m_eBurnCategory;//不同业务类型的刻录
    CRTBurnInfo                         m_RTBurnInfo;//保存当前任务的实时刻录的信息
    CNormalBurnInfo                     m_normalJob;//保存当前任务的事后刻录信息
    size_t                              m_nJobIndex;//多任务时，当前任务索引
    std::string                         m_strProtocol;//当前任务协议，可用于异常恢复

#define BURNCONTROL_STATE_ERROR     -2
#define BURNCONTROL_STATE_IDLE      -1
#define BURNCONTROL_STATE_READY     0
#define BURNCONTROL_STATE_RUNNING   1
#define BURNCONTROL_STATE_PAUSED    2
#define BURNCONTROL_STATE_PAUSING   3
#define BURNCONTROL_STATE_STOPPING  4
    int                                 m_nBurnState;//刻录状态，-1：等待接收任务，0：准备，1：正在刻录，2：暂停

    typedef enum
    {
        BURNCONTROL_CMD_START_RESUME=0,
        BURNCONTROL_CMD_PAUSE_STOP
    }BURNCONTROL_CMD;
    BURNCONTROL_CMD m_eBurnControlCmd;//记录实时刻录上一次的刻录控制命令

    std::vector<CBurnServerInfoEx>      m_vecBurnServerInfoEx;//记录每一个BurnServer每一种刻录方式的光驱选择信息
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

    bool            m_bDevUpdated;//判断是否已经从BurnServer获取的光驱信息

    CStringW        m_strAlertMsg;//桌面弹出窗口的消息内容
    std::string     m_strAlertFlag;//弹出的消息类型标识

    InterfaceServer m_interfaceServer;
    bool            m_bStartServer;
    CEdit m_editItem;
    int m_nItem;//编辑框当前关联的项
    int m_nSubItem;

    bool m_bToExit;//收到关闭的消息时是否真正退出程序

    bool m_bBackupJob;
    BURNTYPESEL m_curBurnTypeSel;
};
