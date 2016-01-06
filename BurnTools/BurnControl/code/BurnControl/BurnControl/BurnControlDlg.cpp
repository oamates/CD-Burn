
// BurnControlDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"

#include <atlenc.h>

#include "BurnControl.h"
#include "BurnControlDlg.h"

#include "DirectoryUtil.h"
#include "FileUtil.h"

#include "MyPopupDlg.h"

#include "CustomMFCDesktopAlertWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_TIMER_SENDKEEPALIVEMSG           45236
#define ID_TIMER_CHECKISDEVUPDATED          45237

//�����첽��Ϣ���صĴ��������
#define ERR_SEND_STARTBURN_MSG_FAILED             -1
#define ERR_SEND_SAVEBURNINFO_MSG_FAILED          -2
#define ERR_SEND_BURNCMDREQUEST_MSG_FAILED        -3
#define ERR_SEND_BURNCMD_MSG_FAILED               -4

//��ǰ�ܹ�֧����ʾ���ٸ���¼���ݸ�ѡ��
#define LOCATION_MAX_COUNT                  10

//����ÿ�¼ui�ĳ����͵Ŀ�¼��������
#define BURNCONTROL_REQUEST_START   "1"
#define BURNCONTROL_REQUEST_PAUSE   "2"
#define BURNCONTROL_REQUEST_RESUME  "3"
#define BURNCONTROL_REQUEST_STOP    "4"

//����ÿ�¼ui�ĳ����͵Ŀ�¼״̬
#define TO_UPPER_BURN_STATE_BURNING              "1"
#define TO_UPPER_BURN_STATE_PAUSED               "2"
#define TO_UPPER_BURN_STATE_STOPPED              "3"
#define TO_UPPER_BURN_STATE_BURNING_NOT_PERFECT  "4"
#define TO_UPPER_BURN_STATE_FAILED               "5"

//��¼Э����������
#define CMD_SENDRTBURNINFO  "sendRTBurnInfo"
#define CMD_SENDBURNINFO    "sendBurnInfo"
#define CMD_STARTRTBURN     "startRTBurn"
#define CMD_STARTHM3RTBURN  "startHM3RTBurn"
#define CMD_SENDBURNCTRLCMD "sendBurnCtrlCMD"
#define CMD_SENDBURNSTATE   "sendBurnState"
#define CMD_GETJOBSTATE     "getJobState"

//�̲߳����ṹ��
typedef struct
{
    void *pThreadData;//Э������
    void *pUserParam;//ʹ����ָ���Ĳ���

    void *p1;//Ԥ������
    void *p2;
}THREAD_PARAM;

DWORD WINAPI SendJobStateThread(LPVOID lpParameter)
{
    if (lpParameter != NULL)
    {
        THREAD_PARAM *pThreadParam=(THREAD_PARAM *)(lpParameter);

        std::string strUpperIP=std::string((char *)(pThreadParam->p1));
        std::string strUpperPort=std::string((char *)(pThreadParam->p2));
        std::string strProtocolContent=std::string((char *)(pThreadParam->pThreadData));

        //CBurnControlDlg *pBurnControlDlg=(CBurnControlDlg *)(pThreadParam->pUserParam);

        if (strUpperIP != "" &&
            strUpperPort != "" &&
            strProtocolContent != "")
        {
            std::string strResponse;
            strResponse=CInterfaceProtocol::InterfaceCommunicate(strUpperIP,
                atoi(strUpperPort.c_str()),strProtocolContent,BURNCONTROL_TAG_NAME,2);
            if (strResponse.length() > 0)
            {
            }
            else
            {
                LOG_WARNING(("[GetJobStateThread] Send burn state failed,upper : %s,%s\r\n",
                    strUpperIP.c_str(),strUpperPort.c_str()));
            }
        }

        delete [] (char *)(pThreadParam->pThreadData);
        pThreadParam->pThreadData=NULL;

        delete [] (char *)(pThreadParam->p1);
        pThreadParam->p1=NULL;

        delete [] (char *)(pThreadParam->p2);
        pThreadParam->p2=NULL;

        delete pThreadParam;
        pThreadParam=NULL;
    }

    return 0;
}

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

    // �Ի�������
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

    // ʵ��
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

ConfigurableFileEx::ConfigurableFileEx()
{
}
ConfigurableFileEx::~ConfigurableFileEx()
{
}
bool ConfigurableFileEx::GetRect(std::string strSectionName,CRect &rect)
{
    rect.SetRectEmpty();

    std::string left,top,right,bottom;
    left=GetValue(strSectionName,"left").c_str();
    top=GetValue(strSectionName,"top").c_str();
    right=GetValue(strSectionName,"right").c_str();
    bottom=GetValue(strSectionName,"bottom").c_str();

    if (left != "" &&
        top != "" &&
        right != "" &&
        bottom != "")
    {
        rect=CRect( atoi(left.c_str()), atoi(top.c_str()),
            atoi(right.c_str()), atoi(bottom.c_str()) );

        return true;
    }

    return false;
}

// CBurnControlDlg �Ի���
CBurnControlDlg::CBurnControlDlg(CWnd* pParent /*=NULL*/)
: CDialogCustom(CBurnControlDlg::IDD, pParent),
m_eBurnCategory(BURNCATEGORY_UNKNOWN),
m_nJobIndex(0),
m_strProtocol(""),
m_nBurnState(BURNCONTROL_STATE_IDLE),
m_mutexJobVec("JobVec"),
m_nConnectServerFailedCount(0),
m_mutexConnectServerFailedCount("ConnectServerFailedCount"),
m_bDevUpdated(false),
m_bStartServer(false),
m_nItem(-1),
m_nSubItem(-1),
m_bToExit(false),
m_bBackupJob(false),
m_mutexBurnServer("BurnServerVec"),
m_curBurnTypeSel(BURNTYPE_UNKNOWNSEL)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_rectBeforeMinimize.SetRectEmpty();
}

CBurnControlDlg::~CBurnControlDlg()
{
}

void CBurnControlDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogCustom::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_GP_BURNPARAM, m_staticBurnParam);
    DDX_Control(pDX, IDC_STATIC_GP_BURNPASSWORD, m_staticBurnPassword);
    DDX_Control(pDX, IDC_STATIC_GP_FILEINFO, m_staticFileInfo);
    DDX_Control(pDX, IDC_STATIC_GP_BURNSTATE, m_staticBurnState);
    DDX_Control(pDX, IDC_STATIC_GP_CDNAME, m_staticCDName);
    DDX_Control(pDX, IDC_STATIC_GP_JOBLIST, m_staticJobList);
    DDX_Control(pDX, IDC_STATIC_GP_BURNCONTENT, m_staticBurnContent);

    DDX_Control(pDX, IDC_BUTTON_UP, m_btnstJobUp);
    DDX_Control(pDX, IDC_BUTTON_DOWN, m_btnstJobDown);
    DDX_Control(pDX, IDC_BUTTON_BURN, m_btnstStartBurn);
    DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnstClose);
    DDX_Control(pDX, IDC_BUTTON_SAVE, m_btnstSave);
    DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btnstPause);

    DDX_Control(pDX, IDC_LIST_JOBLIST, m_listCtrlJobList);
    DDX_Control(pDX, IDC_STATIC_BURNPIC, m_staticBurnPic);
    DDX_Control(pDX, IDC_EDIT_ITEM, m_editItem);
}

BEGIN_MESSAGE_MAP(CBurnControlDlg, CDialogCustom)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &CBurnControlDlg::OnBnClickedButtonSave)
    ON_REGISTERED_MESSAGE(WMUSER_BURNCONTROLDLG_ONRECVPROTOCOL,&CBurnControlDlg::OnRecvProtocol)
    ON_REGISTERED_MESSAGE(WMUSER_BURNCONTROLDLG_BURNBACKUPYES,&CBurnControlDlg::OnBurnBackupYes)
    ON_REGISTERED_MESSAGE(WMUSER_BURNCONTROLDLG_BURNBACKUPNO,&CBurnControlDlg::OnBurnBackupNo)
    ON_REGISTERED_MESSAGE(WMUSER_BURNCONTROLDLG_BURNBACKUPAFTERRESTARTYES,&CBurnControlDlg::OnBurnBackupAfterRestartYes)
    ON_REGISTERED_MESSAGE(WMUSER_BURNCONTROLDLG_BURNBACKUPAFTERRESTARTNO,&CBurnControlDlg::OnBurnBackupAfterRestartNo)
    ON_REGISTERED_MESSAGE(WMUSER_BURNCONTROLDLG_SENDCMDFAILED,&CBurnControlDlg::OnSendCmdFailed)
    ON_REGISTERED_MESSAGE(WMUSER_BURNCONTROLDLG_ASYNCMSGRESULT,&CBurnControlDlg::OnAsyncMsgResult)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_RADIO_MULTISYN, &CBurnControlDlg::OnBnClickedRadioMultisyn)
    ON_BN_CLICKED(IDC_RADIO_SINGLE, &CBurnControlDlg::OnBnClickedRadioSingle)
    ON_BN_CLICKED(IDC_RADIO_MULTIASYN, &CBurnControlDlg::OnBnClickedRadioMultiasyn)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_SELECTCDROM, &CBurnControlDlg::OnLvnItemchangedListSelectcdrom)
    ON_BN_CLICKED(IDC_CHECK_BURNPASSWORD, &CBurnControlDlg::OnBnClickedCheckBurnpassword)
    ON_BN_CLICKED(IDC_BUTTON_DOWN, &CBurnControlDlg::OnBnClickedButtonDown)
    ON_BN_CLICKED(IDC_BUTTON_UP, &CBurnControlDlg::OnBnClickedButtonUp)
    ON_CBN_SELCHANGE(IDC_COMBO_RECORDER, &CBurnControlDlg::OnCbnSelchangeComboRecorder)
    ON_BN_CLICKED(IDC_BUTTON_BURN, &CBurnControlDlg::OnBnClickedButtonBurn)
    ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CBurnControlDlg::OnBnClickedButtonPause)
    ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CBurnControlDlg::OnBnClickedButtonClose)
    ON_WM_TIMER()
    ON_MESSAGE(WM_SYSTRAY_LBUTTONDBLCLK,OnTrayLButtonDBLClk)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_JOBLIST, &CBurnControlDlg::OnNMDblclkListJoblist)
    ON_EN_KILLFOCUS(IDC_EDIT_ITEM, &CBurnControlDlg::OnEnKillfocusEditItem)
    ON_COMMAND(ID_SUB_QUIT, &CBurnControlDlg::OnSubQuit)
    ON_EN_UPDATE(IDC_EDIT_RECORDERINFO, &CBurnControlDlg::OnEnUpdateEditRecorderinfo)
    ON_WM_CREATE()
    ON_WM_ERASEBKGND()
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_CHECK_L1, &CBurnControlDlg::OnBnClickedCheckL1)
    ON_BN_CLICKED(IDC_CHECK_L2, &CBurnControlDlg::OnBnClickedCheckL2)
    ON_BN_CLICKED(IDC_CHECK_L3, &CBurnControlDlg::OnBnClickedCheckL3)
    ON_BN_CLICKED(IDC_CHECK_L4, &CBurnControlDlg::OnBnClickedCheckL4)
    ON_BN_CLICKED(IDC_CHECK_L5, &CBurnControlDlg::OnBnClickedCheckL5)
    ON_BN_CLICKED(IDC_CHECK_L6, &CBurnControlDlg::OnBnClickedCheckL6)
    ON_BN_CLICKED(IDC_CHECK_L7, &CBurnControlDlg::OnBnClickedCheckL7)
    ON_BN_CLICKED(IDC_CHECK_L8, &CBurnControlDlg::OnBnClickedCheckL8)
    ON_BN_CLICKED(IDC_CHECK_L9, &CBurnControlDlg::OnBnClickedCheckL9)
    ON_BN_CLICKED(IDC_CHECK_L10, &CBurnControlDlg::OnBnClickedCheckL10)
END_MESSAGE_MAP()


// CBurnControlDlg ��Ϣ�������

BOOL CBurnControlDlg::OnInitDialog()
{
    CDialogCustom::OnInitDialog();

    // ��������...���˵�����ӵ�ϵͳ�˵��С�

    // IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
    //  ִ�д˲���
    SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
    SetIcon(m_hIcon, FALSE);		// ����Сͼ��

    // TODO: �ڴ���Ӷ���ĳ�ʼ������
    //�������浯�����ڵķ����һ��ᵼ�½���ˢ�£������ڴ˵���
    CMFCVisualManager::SetDefaultManager (RUNTIME_CLASS (CMFCVisualManagerOfficeXP));

    InitConfig();

    CStringW str;

    //������ͼ��
    str=CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("path","burntray").c_str());
    HICON hIcon=(HICON)::LoadImage(0,str,IMAGE_ICON,0,0,LR_LOADFROMFILE);
    m_trayIcon.Create(NULL,WM_APP+10000,
        CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("uiword","name").c_str()),
        hIcon,IDR_MENU_TRAY);

    SetMaxButtonVisible(FALSE);
    SetTopBorderHeight(35);
    SetBottomBorderHeight(10);

    //UI���Ͻ�ͼ��
    m_staticBurnPic.MoveWindow(15,5,35,30);
    str=CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("path","burnpic").c_str());
    m_staticBurnPic.SetPic(str);

    //���������������������Ƶı༭��
    m_editItem.MoveWindow(0,0,0,0);
    m_editItem.SetLimitText(50);
    m_listCtrlJobList.SetEdit(&m_editItem);

    //����ϴν���ʱ�Ƿ���δ��ɵ�����
    ReadRTBBackupXML(m_RTBurnInfo);
    if (m_RTBurnInfo.GetStateFlag() == STATE_BURNING ||
        m_RTBurnInfo.GetStateFlag() == STATE_DOWNLOADING ||
        m_RTBurnInfo.GetStateFlag() == STATE_WILL_CLOSE_DISC ||
        m_RTBurnInfo.GetStateFlag() == STATE_CHANGE_DISC ||
        m_RTBurnInfo.GetStateFlag() == STATE_RTTASK_STOPPING)
    {
        m_bBackupJob = true;
        LOG_INFO(("[CBurnControlDlg::OnInitDialog] Have running job before last exit!strJobID: %s, jobState: %s\r\n"
            , m_RTBurnInfo.GetStateFlag().c_str(), m_RTBurnInfo.GetStateDescription().c_str()));
    }

    CInterfaceProtocol::SetCallBack(OnRecvProtocol,this);

    m_interfaceServer.SetPort(BURNCONTROL_SERVER_PORT);
    m_interfaceServer.SetMostTagName(BURNCONTROL_TAG_NAME);
    m_bStartServer=(0 == m_interfaceServer.Start());
    if(m_bStartServer)
    {
        LOG_INFO(("[CBurnControlDlg::OnInitDialog] InterfaceServer start success\r\n"));
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::OnInitDialog] InterfaceServer start failed\r\n"));
    }

    //��̨����    
    ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW);

    ::SetWindowPos(this->GetSafeHwnd(), HWND_TOP, 0, 0, 
        0, 0, SWP_HIDEWINDOW);

    return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CBurnControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        if (SC_MINIMIZE == nID)
        {
            if (/*BURNCATEGORY_HM3 == m_eBurnCategory ||
                BURNCATEGORY_RTBURN == m_eBurnCategory*/TRUE)
            {
                GetClientRect(&m_rectBeforeMinimize);

                ClientToScreen(&m_rectBeforeMinimize);

                ShowWindow(SW_HIDE);

                return;
            }
        }
        else if (SC_CLOSE == nID)
        {
            if (!m_bToExit)
            {
                if (m_nBurnState == BURNCONTROL_STATE_RUNNING ||
                    m_nBurnState == BURNCONTROL_STATE_PAUSED)
                {
                    //����������ڱ�ִ�в�����رմ���

                    MessageBox(CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("info","jobrunning").c_str()),
                        CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("info","infomsg").c_str()));

                    return;
                }
                else
                {
                    ShowWindow(SW_HIDE);

                    return;
                }
            }
        }

        CDialogCustom::OnSysCommand(nID, lParam);
    }
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CBurnControlDlg::OnPaint()
{
    //TRACE("[CBurnControlDlg::OnPaint] **** %d,%d ****\r\n",GetTickCount(),__LINE__);

    if (IsIconic())
    {
        CPaintDC dc(this); // ���ڻ��Ƶ��豸������

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // ʹͼ���ڹ����������о���
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // ����ͼ��
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogCustom::OnPaint();
    }
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CBurnControlDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


HBRUSH CBurnControlDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    TRACE("[CBurnControlDlg::OnCtlColor] %d\r\n",__LINE__);

    HBRUSH hbr = CDialogCustom::OnCtlColor(pDC, pWnd, nCtlColor);

    // TODO:  Change any attributes of the DC here
    UINT id=pWnd->GetDlgCtrlID();

    if(CTLCOLOR_STATIC == nCtlColor)
    {
        switch (id)
        {
        case IDC_STATIC_TITLE:
            pDC->SetTextColor(RGB(255,255,255));
            pDC->SetBkMode(TRANSPARENT);
            hbr = (HBRUSH) ::GetStockObject(NULL_BRUSH);
            break;

        case IDC_EDIT_INPUTPASSWORD:
        case IDC_EDIT_CONFIRMPASSWORD:
        case IDC_EDIT_CDNAME:
        case IDC_EDIT_RECORDERINFO:
            break;

        case IDC_CHECK_L1:
        case IDC_CHECK_L2:
        case IDC_CHECK_L3:
        case IDC_CHECK_L4:
        case IDC_CHECK_L5:
        case IDC_CHECK_L6:
        case IDC_CHECK_L7:
        case IDC_CHECK_L8:
        case IDC_CHECK_L9:
        case IDC_CHECK_L10:
        case IDC_CHECK_BURNPASSWORD:
        case IDC_CHECK_CONTENTPASSWORD:
        case IDC_RADIO_SINGLE:
        case IDC_RADIO_MULTISYN:
        case IDC_RADIO_MULTIASYN:
            {
                CRect rc;
                pWnd->GetWindowRect(&rc);
                ScreenToClient(&rc);

                CDC* dc = GetDC();
                pDC->BitBlt(0,0,rc.Width(),rc.Height(),dc,rc.left,rc.top,SRCCOPY);
                ReleaseDC(dc);

                hbr = (HBRUSH) ::GetStockObject(NULL_BRUSH);
                break;
            }

        default:
            pDC->SetBkMode(TRANSPARENT);
            hbr = (HBRUSH) ::GetStockObject(NULL_BRUSH);
        }
    }

    // TODO:  Return a different brush if the default is not desired
    return hbr;
}

void CBurnControlDlg::OnDestroy()
{
    CDialogCustom::OnDestroy();
}

void CBurnControlDlg::OnBnClickedButtonDown()
{
    // TODO: Add your control notification handler code here

    CListCtrl *pListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_JOBLIST));
    if (pListCtrl != NULL &&
        pListCtrl->GetSafeHwnd() != NULL)
    {
        int nIndexLastSel=0x80000000,nIndexFirstSel=0x7fffffff;
        int nSelectedCount=pListCtrl->GetSelectedCount();
        int  nItem=-1;
        if (nSelectedCount > 0)
        {
            for (int i=0;i<nSelectedCount;i++)
            {
                nItem=pListCtrl->GetNextItem(nItem,LVNI_SELECTED);

                if (nItem > nIndexLastSel)
                {
                    nIndexLastSel=nItem;
                }

                if (nItem < nIndexFirstSel)
                {
                    nIndexFirstSel=nItem;
                }
            }

            int nItemCount=pListCtrl->GetItemCount();
            if (nIndexFirstSel >= 0 &&
                nIndexLastSel < nItemCount-1 &&
                (nIndexLastSel-nIndexFirstSel+1) == nSelectedCount &&
                m_normalJob.GetVecNormalBurnJobInfo().size() > static_cast<size_t>(nSelectedCount) &&
                nSelectedCount < pListCtrl->GetItemCount())
            {
                std::vector<CNormalBurnJobInfoEx> &vecNormalBurnJobInfo=m_normalJob.GetVecNormalBurnJobInfo();
                std::vector<CNormalBurnJobInfoEx> vecTemp(vecNormalBurnJobInfo.begin()+nIndexLastSel+1-nSelectedCount,
                    vecNormalBurnJobInfo.begin()+nIndexLastSel+1);//���Ƶ�ǰѡ����
                std::vector<CNormalBurnJobInfoEx>::iterator iter=
                    vecNormalBurnJobInfo.erase(vecNormalBurnJobInfo.begin()+nIndexLastSel+1-nSelectedCount,
                    vecNormalBurnJobInfo.begin()+nIndexLastSel+1);//ɾ����ǰѡ����
                if (iter == vecNormalBurnJobInfo.end())
                {
                    iter=vecNormalBurnJobInfo.begin();
                }
                else
                {
                    ++iter;
                }

                int nNewFirstIndex=-1;//��λ��
                if (iter == vecNormalBurnJobInfo.begin())
                {
                    nNewFirstIndex=0;
                }
                else if (iter == vecNormalBurnJobInfo.end())
                {
                    nNewFirstIndex=pListCtrl->GetItemCount()-nSelectedCount;
                }
                else
                {
                    nNewFirstIndex=iter-vecNormalBurnJobInfo.begin();
                }

                vecNormalBurnJobInfo.insert(iter,vecTemp.begin(),vecTemp.end());

                DisplayJobListUI();
                for (int i=0;i<nSelectedCount;++i)
                {
                    pListCtrl->SetItemState(i+nNewFirstIndex,LVIS_SELECTED,LVIS_SELECTED);
                }

                pListCtrl->SetFocus();
            }
        }//if
    }
}

void CBurnControlDlg::OnBnClickedButtonUp()
{
    // TODO: Add your control notification handler code here

    CListCtrl *pListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_JOBLIST));
    if (pListCtrl != NULL &&
        pListCtrl->GetSafeHwnd() != NULL)
    {
        int nIndexLastSel=0x80000000,nIndexFirstSel=0x7fffffff;
        int nSelectedCount=pListCtrl->GetSelectedCount();
        int  nItem=-1;
        if (nSelectedCount > 0)
        {
            for (int i=0;i<nSelectedCount;i++)
            {
                nItem=pListCtrl->GetNextItem(nItem,LVNI_SELECTED);

                if (nItem > nIndexLastSel)
                {
                    nIndexLastSel=nItem;
                }

                if (nItem < nIndexFirstSel)
                {
                    nIndexFirstSel=nItem;
                }
            }

            int nItemCount=pListCtrl->GetItemCount();
            if (nIndexFirstSel > 0 &&
                nIndexLastSel >= 0 &&
                nIndexLastSel < nItemCount &&
                (nIndexLastSel-nIndexFirstSel+1) == nSelectedCount &&
                m_normalJob.GetVecNormalBurnJobInfo().size() > static_cast<size_t>(nSelectedCount) &&
                nSelectedCount < nItemCount)
            {
                std::vector<CNormalBurnJobInfoEx> &vecNormalBurnJobInfo=m_normalJob.GetVecNormalBurnJobInfo();
                std::vector<CNormalBurnJobInfoEx> vecTemp(vecNormalBurnJobInfo.begin()+nIndexLastSel+1-nSelectedCount,
                    vecNormalBurnJobInfo.begin()+nIndexLastSel+1);
                std::vector<CNormalBurnJobInfoEx>::iterator iter=
                    vecNormalBurnJobInfo.erase(vecNormalBurnJobInfo.begin()+nIndexLastSel+1-nSelectedCount,
                    vecNormalBurnJobInfo.begin()+nIndexLastSel+1);

                if (iter == vecNormalBurnJobInfo.begin())
                {
                    iter=vecNormalBurnJobInfo.end();
                }
                else
                {
                    --iter;
                }

                int nNewFirstIndex=-1;
                if (iter == vecNormalBurnJobInfo.begin())
                {
                    nNewFirstIndex=0;
                }
                else if (iter == vecNormalBurnJobInfo.end())
                {
                    nNewFirstIndex=pListCtrl->GetItemCount()-nSelectedCount;
                }
                else
                {
                    nNewFirstIndex=iter-vecNormalBurnJobInfo.begin();
                }

                vecNormalBurnJobInfo.insert(iter,vecTemp.begin(),vecTemp.end());

                DisplayJobListUI();
                for (int i=0;i<nSelectedCount;++i)
                {
                    pListCtrl->SetItemState(i+nNewFirstIndex,LVIS_SELECTED,LVIS_SELECTED);
                }
                pListCtrl->SetFocus();

            }
        }
    }
}

void CBurnControlDlg::OnCbnSelchangeComboRecorder()
{
    // TODO: Add your control notification handler code here
    UpdateBurnServerUIInfo();
}

void CBurnControlDlg::OnBnClickedRadioSingle()
{
    // TODO: Add your control notification handler code here
    SetCurBurnTypeSel(BURNTYPE_SINGLE);
    DisplayCDROMUI(DEV_UI_FLAG_UPDATE);
}

void CBurnControlDlg::OnBnClickedRadioMultisyn()
{
    // TODO: Add your control notification handler code here
    SetCurBurnTypeSel(BURNTYPE_MULTISYN);
    DisplayCDROMUI(DEV_UI_FLAG_UPDATE);
}

void CBurnControlDlg::OnBnClickedRadioMultiasyn()
{
    // TODO: Add your control notification handler code here
    SetCurBurnTypeSel(BURNTYPE_MULTIASYN);
    DisplayCDROMUI(DEV_UI_FLAG_UPDATE);
}

void CBurnControlDlg::OnLvnItemchangedListSelectcdrom(NMHDR *pNMHDR, LRESULT *pResult)
{
    LOG_INFO(("[CBurnControlDlg::OnLvnItemchangedListSelectcdrom] ......\r\n"));

    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    // TODO: Add your control notification handler code here
    *pResult = 0;

    UINT oldStateUncheck=pNMLV->uOldState & INDEXTOSTATEIMAGEMASK(1);
    UINT oldStateCheck=pNMLV->uOldState & INDEXTOSTATEIMAGEMASK(2);
    UINT newStateUncheck=pNMLV->uNewState & INDEXTOSTATEIMAGEMASK(1);
    UINT newStateCheck=pNMLV->uNewState & INDEXTOSTATEIMAGEMASK(2);
    int nIndex=pNMLV->iItem;
    if ((oldStateCheck && newStateUncheck) ||
        (oldStateUncheck && newStateCheck))
    {
        BURNTYPESEL eCurBurnTypeSel=GetCurBurnTypeSel();
        std::vector<CCDROMDriverInfo> vecCDROMDriverInfo=GetCurCDROMDriverInfoVec();
        if (!vecCDROMDriverInfo.empty() &&
            eCurBurnTypeSel != BURNTYPE_UNKNOWNSEL)
        {
            if (newStateCheck)
            {
                if (BURNTYPE_SINGLE == eCurBurnTypeSel)
                {
                    CListCtrl *pListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_SELECTCDROM));

                    for (size_t i=0;i<vecCDROMDriverInfo.size();++i)
                    {
                        if (i == static_cast<size_t>(nIndex))
                        {
                            vecCDROMDriverInfo[i].SetIsSelected(DEV_SELECTED_YES);
                            continue;
                        }

                        vecCDROMDriverInfo[i].SetIsSelected(DEV_SELECTED_NO);
                        if (pListCtrl != NULL &&
                            pListCtrl->GetSafeHwnd() != NULL)
                        {
                            pListCtrl->SetCheck(i,FALSE);
                        }
                    }
                }

                if (vecCDROMDriverInfo.size() > static_cast<size_t>(nIndex))
                {
                    vecCDROMDriverInfo[nIndex].SetIsSelected(DEV_SELECTED_YES);
                } 
                else
                {
                    LOG_ERROR(("[CBurnControlDlg::OnLvnItemchangedListSelectcdrom] Dev info error,%d\r\n",
                        __LINE__));

                    ASSERT(FALSE);
                }
            } 
            else
            {
                if (vecCDROMDriverInfo.size() > static_cast<size_t>(nIndex))
                {
                    vecCDROMDriverInfo[nIndex].SetIsSelected(DEV_SELECTED_NO);
                } 
                else
                {
                    LOG_ERROR(("[CBurnControlDlg::OnLvnItemchangedListSelectcdrom] Dev info error,%d\r\n",
                        __LINE__));

                    ASSERT(FALSE);
                }
            }

            UpdateCDROMDriverInfo(vecCDROMDriverInfo);
        }
    }
}

void CBurnControlDlg::OnBnClickedCheckBurnpassword()
{
    // TODO: Add your control notification handler code here
    CButton *pButton=(CButton *)GetDlgItem(IDC_CHECK_BURNPASSWORD);
    if (pButton != NULL &&
        pButton->GetSafeHwnd() != NULL)
    {
        bool b=(BST_CHECKED == pButton->GetCheck());

        ((CButton *)GetDlgItem(IDC_CHECK_CONTENTPASSWORD))->EnableWindow(b ? TRUE : FALSE);
        ((CEdit *)GetDlgItem(IDC_EDIT_INPUTPASSWORD))->EnableWindow(b ? TRUE : FALSE);
        ((CEdit *)GetDlgItem(IDC_EDIT_CONFIRMPASSWORD))->EnableWindow(b ? TRUE : FALSE);
    }
}

void CBurnControlDlg::OnBnClickedButtonBurn()
{
#if 0
    ShowWindow(SW_HIDE);

    return;
#endif

    // TODO: Add your control notification handler code here
    if (BURNCONTROL_STATE_RUNNING == m_nBurnState ||
        BURNCONTROL_STATE_PAUSED == m_nBurnState)
    {
        if (BURNCATEGORY_RTBURN == m_eBurnCategory)
        {
            StopRtBurn(false);

            m_eBurnControlCmd=BURNCONTROL_CMD_PAUSE_STOP;
        }
        else if (BURNCATEGORY_HM3 == m_eBurnCategory)
        {
            StopRtBurn(true);

            m_eBurnControlCmd=BURNCONTROL_CMD_PAUSE_STOP;
        }
    }
    else if (BURNCONTROL_STATE_READY == m_nBurnState ||
        BURNCONTROL_STATE_IDLE == m_nBurnState ||
        BURNCONTROL_STATE_ERROR == m_nBurnState)//��һ�������¼��ɣ���״̬BURNCONTROL_STATE_IDLE����ʾ���Խ�����һ�����񣬵���Ҳ�����ظ���¼
    {
        std::string strBurnServerType=GetBurnServerType();

        std::vector<CCDROMDriverInfo> vecDev=GetCurCDROMDriverInfoVec();
        if (vecDev.empty() &&
            BURNSERVER_TYPE_NORMAL == strBurnServerType)
        {
            DESKTOP_ALERT_PARAM param;
            param.strFlag=STATE_BURN_ERR;
            param.strDescription=m_cfgFile.GetValue("info","selectdev");
            param.bAutoClose=TRUE;
            param.nAutoCloseTime=4;
            ShowDesktopAlert(param);

            return;
        }
        else
        {
            //RESET����״̬��Ϣ
            CPictureListCtrl *pJobListCtrl=(CPictureListCtrl *)(GetDlgItem(IDC_LIST_JOBLIST));
            if (pJobListCtrl != NULL &&
                pJobListCtrl->GetSafeHwnd() != NULL &&
                pJobListCtrl->IsWindowVisible())
            {
                int n=pJobListCtrl->GetItemCount();
                for (int i=0;i<n;++i)
                {
                    if (0 == i)
                    {
                        pJobListCtrl->SetItemText(i,1,CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("uiword","burning").c_str()));
                    } 
                    else
                    {
                        pJobListCtrl->SetItemText(i,1,CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("uiword","burnwait").c_str()));
                    }
                }
            }

            //RESET����״̬��Ϣ
            CListCtrl *pDevStateListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_BURNSTATE));
            if (pDevStateListCtrl != NULL &&
                pDevStateListCtrl->GetSafeHwnd() != NULL)
            {
                int n=pDevStateListCtrl->GetItemCount();
                for (int i=1;i<n;i+=2)
                {
                    pDevStateListCtrl->SetItemText(i,1,L"");
                }
            }

            //���û��ѡ���¼��������ô����¼
            std::vector<CCDROMDriverInfo>::iterator iterDev;
            bool bDevSelected=false;
            for (iterDev=vecDev.begin();iterDev!=vecDev.end();++iterDev)
            {
                if (DEV_SELECTED_YES == iterDev->GetIsSelected())
                {
                    bDevSelected=true;

                    break;
                }
            }
            if (!bDevSelected &&
                BURNSERVER_TYPE_NORMAL == strBurnServerType)
            {
                DESKTOP_ALERT_PARAM param;
                param.strFlag=STATE_BURN_ERR;
                param.strDescription=m_cfgFile.GetValue("info","selectdev");
                param.bAutoClose=TRUE;
                param.nAutoCloseTime=4;
                ShowDesktopAlert(param);

                return;
            }

            switch (m_eBurnCategory)
            {
            case BURNCATEGORY_RTBURN:
                m_RTBurnInfo.SetJobFlag(JOB_FLAG_SAVE_AND_BURN);
                if (0 == GetCheckedContent())
                {
                    DESKTOP_ALERT_PARAM param;
                    param.strFlag=STATE_BURN_ERR;
                    param.strDescription=m_cfgFile.GetValue("info","checkcontent");
                    param.bAutoClose=TRUE;
                    param.nAutoCloseTime=4;
                    ShowDesktopAlert(param);

                    return;
                }

                m_eBurnControlCmd=BURNCONTROL_CMD_START_RESUME;

                StartRtBurn(false);

                break;

            case BURNCATEGORY_HM3:
                if (0 == GetCheckedContent())
                {
                    MessageBoxW(CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("info","checkcontent").c_str()),
                        CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("info","infomsg").c_str()),MB_OK);

                    return;
                }

                m_eBurnControlCmd=BURNCONTROL_CMD_START_RESUME;

                StartRtBurn(true);

                break;

            case BURNCATEGORY_COS:
                if (0 == GetCheckedContent())
                {
                    MessageBoxW(CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("info","checkcontent").c_str()),
                        CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("info","infomsg").c_str()),MB_OK);

                    return;
                }
                SaveNoneRTBurnCosCfg();
                CreateAdditionalBurnFile();
                SendBurnJobToBurnServer();
                break;

            case BURNCATEGORY_SINGLE:
                SaveNoneRTBurnSingleCfg();
                CreateAdditionalBurnFile();
                SendBurnJobToBurnServer();
                break;

            case BURNCATEGORY_MULTI:
                {
                    size_t nJobCount=m_normalJob.GetVecNormalBurnJobInfo().size();
                    if (nJobCount > 0)
                    {
                        if (m_nJobIndex >= nJobCount)
                        {
                            m_nJobIndex%=nJobCount;
                        }

                        SaveNoneRTBurnMultiCfg();
                        CreateAdditionalBurnFile();

                        SendBurnJobToBurnServer();
                    }
                    else
                    {
                        LOG_WARNING(("[CBurnControlDlg::OnBnClickedButtonBurn] No job,%d\r\n",__LINE__));

                        assert(false);
                    }
                }
                break;

            case BURNCATEGORY_UNKNOWN:
            default:
                LOG_WARNING(("[CBurnControlDlg::OnBnClickedButtonBurn] Burn type is not corect\r\n"));
                assert(false);
            }
        }
    } 
    else
    {
        LOG_ERROR(("[CBurnControlDlg::OnBnClickedButtonBurn] Burn state error : %d\r\n",m_nBurnState));

        assert(false);
    }
}

void CBurnControlDlg::OnBnClickedButtonPause()
{
    // TODO: Add your control notification handler code here

    if ( (BURNCATEGORY_RTBURN == m_eBurnCategory) ||
        (BURNCATEGORY_HM3 == m_eBurnCategory) )
    {
        if (BURNCONTROL_STATE_RUNNING == m_nBurnState)
        {
            if (BURNCATEGORY_RTBURN == m_eBurnCategory)
            {
                m_eBurnControlCmd=BURNCONTROL_CMD_PAUSE_STOP;

                PauseRtBurn(false);

                ShowBurnControlUIUnable();//��BurnServer�����ͣ��¼�Ĺ����в����������������
            }
            else if (BURNCATEGORY_HM3 == m_eBurnCategory)
            {
                m_eBurnControlCmd=BURNCONTROL_CMD_PAUSE_STOP;

                PauseRtBurn(true);

                ShowBurnControlUIUnable();
            }
        }
        else if (BURNCONTROL_STATE_PAUSED == m_nBurnState)
        {
            if (BURNCATEGORY_RTBURN == m_eBurnCategory)
            {
                m_eBurnControlCmd=BURNCONTROL_CMD_START_RESUME;

                ResumeRtBurn(false);
            }
            else if (BURNCATEGORY_HM3 == m_eBurnCategory)
            {
                m_eBurnControlCmd=BURNCONTROL_CMD_START_RESUME;

                ResumeRtBurn(true);

                m_nBurnState=BURNCONTROL_STATE_RUNNING;

                UpdateBurnControlUI();
            }
        } 
        else
        {
            LOG_ERROR(("[CBurnControlDlg::OnBnClickedButtonPause] Burn state error : %d\r\n",m_nBurnState));

            assert(false);
        }
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::OnBnClickedButtonPause] Not rt burn,can not pause\r\n"));

        assert(false);
    }
}

void CBurnControlDlg::OnBnClickedButtonSave()
{
    // TODO: Add your control notification handler code here
    if (BURNCATEGORY_RTBURN == m_eBurnCategory)
    {
        SaveRTBurnCfg();

        m_RTBurnInfo.SetJobFlag(JOB_FLAG_SAVE);

        //���ϲ㷢�Ϳ�¼������Ϣ
        std::string strProtocolContent=CInterfaceProtocol::GenerateProtocolSendRTBurnInfoToUpper(m_RTBurnInfo);
        std::string strIP=m_RTBurnInfo.GetUpServerIP();
        int nPort=-1;
        std::string strPort=m_RTBurnInfo.GetUpServerPort();
        if (strPort != "")
        {
            nPort=atoi(strPort.c_str());
        }

        if ( (strProtocolContent != "") &&
            (strIP != "") &&
            (nPort > 0) )
        {
            AsyncSendNCXMsg(strIP,strPort,strProtocolContent);

            /*std::string strResponse;
            strResponse=CInterfaceProtocol::InterfaceCommunicate(m_RTBurnInfo.GetUpServerIP(),
                nPort,strProtocolContent,BURNCONTROL_TAG_NAME);
            if (strResponse.length() > 0)
            {
                LOG_INFO(("[CBurnControlDlg::OnBnClickedButtonSave] Send protocol sendRTBurnConfig success\r\n"));

                PostMessage(WM_SYSCOMMAND,SC_MINIMIZE,0);
            }
            else
            {
                LOG_ERROR(("[CBurnControlDlg::OnBnClickedButtonSave] Send protocol \"sendRTBurnConfig\" failed,"
                    "remote IP:%s,remote port:%s\r\n",strIP.c_str(),strPort.c_str()));

                MessageBoxW(CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("info","savecfgfailed").c_str()),
                    CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("info","errmsg").c_str()),MB_OK);
            }*/
        }
    }
}

void CBurnControlDlg::OnBnClickedButtonClose()
{
    // TODO: Add your control notification handler code here
    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_RTBURN:
    case BURNCATEGORY_HM3:
        PostMessage(WM_SYSCOMMAND,SC_MINIMIZE,0);
        break;

    case BURNCATEGORY_COS:
    case BURNCATEGORY_SINGLE:
    case BURNCATEGORY_MULTI:
        //�º��¼�����������رա���ť��ֱ���˳�����
        PostMessage(WM_CLOSE,0,0);
        break;

    case BURNCATEGORY_UNKNOWN:
    default:
        LOG_ERROR(("[CBurnControlDlg::OnBnClickedButtonClose] BURNCATEGORY_UNKNOWN\r\n"));
        assert(false);
        break;
    }
}

LRESULT CBurnControlDlg::OnTrayLButtonDBLClk(WPARAM wParam,LPARAM lParam)
{
#if 0
    DESKTOP_ALERT_PARAM param;
    param.strFlag=STATE_CONNECT_SERVER_FAILED;
    param.nTransparency=255;
    param.strDescription=m_cfgFile.GetValue("info","connectserverfailed");
    ShowDesktopAlert(param);
    return NULL;
#endif

    CRect rect;
    rect.SetRect(m_rectBeforeMinimize.left,m_rectBeforeMinimize.top,m_rectBeforeMinimize.right,m_rectBeforeMinimize.bottom);

    LOG_INFO(("[CBurnControlDlg::OnTrayLButtonDBLClk] dbclk, %d,%d,%d ...\r\n",
        m_rectBeforeMinimize.Width(),m_rectBeforeMinimize.Height(),__LINE__));

    if ((m_rectBeforeMinimize.Width() > 0) &&
        (m_rectBeforeMinimize.Height() > 0))
    {
        ShowWindow(SW_SHOW);
        SetForegroundWindow();
    }

    return NULL;
}

void CBurnControlDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: Add your message handler code here and/or call default
    if (ID_TIMER_SENDKEEPALIVEMSG == nIDEvent)
    {
#ifdef BURNCONTROL_TEST
        std::string strIP;
        int nPort;
        GetBurnServerAddress(strIP,nPort);

        std::vector<CCDROMDriverInfo> vecDev;
        if (GetDevInfo(strIP,IntToString(nPort),vecDev))
        {
            UpdateCDROMState(vecDev);
        }
#endif

        SendKeepAliveMsg();

        if (GetConnectServerFailedCount() > 10)
        {
            KillTimer(nIDEvent);

            DESKTOP_ALERT_PARAM param;
            param.strFlag=STATE_CONNECT_SERVER_FAILED;
            param.strDescription=m_cfgFile.GetValue("info","connectserverfailed");
            param.bAutoClose=TRUE;
            param.nAutoCloseTime=30;
            ShowDesktopAlert(param);

            ProcessJobFailed();
        }
    }
    else if (ID_TIMER_CHECKISDEVUPDATED == nIDEvent)
    {
        if (GetDevUpdated())
        {
            KillTimer(nIDEvent);

            DisplayBurnTypeUI(GetCurBurnTypeSel());

            //�����ȡ������Ϣʧ�ܣ������ֲ���ս�����һ�μ����ý׶εĹ�������޸ģ���
            //��ʼʵʱ��¼ʱ������֮ǰѡ��Ĺ�������ʱ���ڴ汣��Ĺ�����Ϣ�ͽ����Ѿ���ʾ�Ĺ�����Ϣ
            // ��������ᵼ�³������
            // �����У����̿�¼ѡ�������������������ѡʱ���¹�������Խ��
            DisplayCDROMUI(DEV_UI_FLAG_UPDATE);
        }
    }

    CDialogCustom::OnTimer(nIDEvent);
}

void CBurnControlDlg::OnNMDblclkListJoblist(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: Add your control notification handler code here

    int nItem=pNMItemActivate->iItem;
    int nSubItem=pNMItemActivate->iSubItem;

    CListCtrl *pListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_JOBLIST));
    if (GetItemType(pListCtrl,nItem,nSubItem) == ITEM_TYPE_EDIT &&
        m_editItem.IsWindowVisible())
    {
        m_nItem=nItem;
        m_nSubItem=nSubItem;

        AttachEdit(pListCtrl,nItem,nSubItem);
    }

    *pResult = 0;
}


void CBurnControlDlg::OnEnKillfocusEditItem()
{
    // TODO: Add your control notification handler code here

    if (m_eBurnCategory != BURNCATEGORY_MULTI)
    {
        return;
    }

    CListCtrl *pListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_JOBLIST));

    CRect rect;
    m_editItem.GetWindowRect(&rect);

    POINT point;
    point.x=rect.left+rect.Width()/2;
    point.y=rect.top+rect.Height()/2;

    ::ScreenToClient(pListCtrl->GetSafeHwnd(),&point);

    CRect clientRect;
    pListCtrl->GetSubItemRect(0,3,LVIR_BOUNDS,clientRect);

    int nItem=-1;
    int nSubItem=-1;

    LVHITTESTINFO info;
    info.pt=point;
    info.flags=LVHT_ONITEM;
    info.iItem=-1;
    info.iSubItem=-1;
    info.iGroup=-1;

    pListCtrl->SubItemHitTest(&info);

    assert((info.iItem >= 0) && (info.iSubItem >= 0));

    nItem=info.iItem;
    nSubItem=info.iSubItem;

    CStringW strText;
    m_editItem.GetWindowText(strText);

    m_editItem.MoveWindow(0,0,0,0);

    pListCtrl->SetItemText(nItem,nSubItem,strText);

    m_normalJob.GetVecNormalBurnJobInfo().at(nItem).GetCommonBurnParam().SetCDName(CharsetConvertMFC::UTF16ToUTF8(strText).GetBuffer());
}

void CBurnControlDlg::OnSubQuit()
{
    // TODO: Add your command handler code here
    LOG_INFO(("[CBurnControlDlg::OnSubQuit] To quit,%d\r\n",
        __LINE__));

    m_bToExit=true;

    PostMessage(WM_CLOSE,0,0);
}

void CBurnControlDlg::OnEnUpdateEditRecorderinfo()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogCustom::OnInitDialog()
    // function to send the EM_SETEVENTMASK message to the control
    // with the ENM_UPDATE flag ORed into the lParam mask.

    // TODO:  Add your control notification handler code here

    CRect rect;
}

int CBurnControlDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialogCustom::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  Add your specialized creation code here
    SetWindowText(L"BurnControl");

    return 0;
}

BOOL CBurnControlDlg::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default

    return CDialogCustom::OnEraseBkgnd(pDC);
}

void CBurnControlDlg::OnClose()
{
    // TODO: Add your message handler code here and/or call default

    if (m_bStartServer)
    {
        m_interfaceServer.Stop();
    }

    CDialogCustom::OnClose();
}

BOOL CBurnControlDlg::PreTranslateMessage(MSG* pMsg)
{
    // TODO: Add your specialized code here and/or call the base class

    BOOL bSkip=FALSE;
    if (WM_KEYDOWN == pMsg->message)
    {
        switch (pMsg->wParam)
        {
        case VK_RETURN:
        case VK_ESCAPE:
            bSkip=TRUE;
            break;

        default:
            bSkip=FALSE;
            break;
        }
    }

    if (bSkip)
    {
        return TRUE;
    }
    else
    {
        return CDialogCustom::PreTranslateMessage(pMsg);
    }
}

BOOL CBurnControlDlg::WcToMb(LPCWSTR pwcszSrc,LPSTR pszDst,int nBufSize)
{
    int dwMinSize;
    dwMinSize=WideCharToMultiByte(CP_OEMCP,NULL,pwcszSrc,-1,NULL,0,NULL,FALSE);
    if(nBufSize < dwMinSize)
    {
        return FALSE;
    }

    WideCharToMultiByte(CP_OEMCP,NULL,pwcszSrc,-1,pszDst,nBufSize,NULL,FALSE);

    return TRUE;
}


void CBurnControlDlg::GetAndDisplayCDROMDriverInfo()
{
    //m_vecBurnServerInfoEx : ��ʱ��ʵʱ��¼ʱ�������ϲ�Ӧ��ָ���Ĺ�����Ϣ��
    //�ڷ�ʵʱ��¼ʱ������¼�������ĵ�ַ��������Ϣ���Ա�ʹ��
    std::vector<CCDROMDriverInfo> vecCDROMDriverInfo;
    if (!m_vecBurnServerInfoEx.empty())
    {
        std::vector<CBurnServerInfoEx>::iterator iter;
        for (iter=m_vecBurnServerInfoEx.begin();iter!=m_vecBurnServerInfoEx.end();++iter)
        {
            //��BurnServer��ȡ������Ϣ�����BurnServer�ж��ֿ�¼���ܣ�������Ҫѯ����ͨ��¼��������Ϣ
            if (BURNSERVER_TYPE_NORMAL == iter->GetBurnServerType())
            {
                std::string strIP = iter->GetIP();
                std::string strPort = iter->GetPort();
                if (GetDevInfo(strIP,strPort,vecCDROMDriverInfo))
                {
                    if (!vecCDROMDriverInfo.empty())
                    {
                        LOG_INFO(("\r\n\r\n------ Burn server : %s,%s -----------------,%d\r\n",
                            strIP.c_str(),strPort.c_str(),__LINE__));
                        std::vector<CCDROMDriverInfo>::iterator iterT;
                        for (iterT=vecCDROMDriverInfo.begin();iterT!=vecCDROMDriverInfo.end();++iterT)
                        {
                            LOG_INFO(("Dev id : %s,is selected : %s\r\n",iterT->GetID().c_str(),iterT->GetIsSelected().c_str()));
                        }
                        LOG_INFO(("---------------------------------------\r\n\r\n"));

                        iter->UpdateCDROMDriverState(vecCDROMDriverInfo);

                        LOG_INFO(("[CBurnControlDlg::GetAndDisplayCDROMDriverInfo] Before DisplayBurnTypeUI,%d\r\n",__LINE__));
                        DisplayBurnTypeUI(GetCurBurnTypeSel());
                        LOG_INFO(("[CBurnControlDlg::GetAndDisplayCDROMDriverInfo] After DisplayBurnTypeUI,%d\r\n",__LINE__));

                        DisplayCDROMUI(DEV_UI_FLAG_CREATE);

                        LOG_INFO(("[CBurnControlDlg::GetAndDisplayCDROMDriverInfo] Update CDROM driver state success\r\n"));
                    }
                    else
                    {
                        LOG_ERROR(("[CBurnControlDlg::GetAndDisplayCDROMDriverInfo] No CDROM driver,%s,%s,%d\r\n",
                            strIP.c_str(),strPort.c_str(),__LINE__));
                    }
                }
                else
                {
                    //�����ȡ������Ϣʧ�ܣ���ô���ϲ���Ĺ�����Ϣ��Ϊ��Ч
                    std::vector<CCDROMDriverInfo> v;
                    iter->SetVecCDROMInfo0(v);
                    iter->SetVecCDROMInfo1(v);
                    iter->SetVecCDROMInfo2(v);
                }
            }
        }
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::GetAndDisplayCDROMDriverInfo] No burn server\r\n"));
    }
    SetDevUpdated(true);
}

void CBurnControlDlg::OnRecvProtocol(NCXSERVERHANDLE hNCXServer,NCXServerCBParam *pCBParam,void *pUsrParam)
{
    char *pszContent=pCBParam->sProtocolContent;
    CBurnControlDlg *pBurnControlDlg=(CBurnControlDlg *)pUsrParam;

    if (pBurnControlDlg != NULL &&
        pBurnControlDlg->GetSafeHwnd() != NULL &&
        pszContent != NULL)
    {
        int nLen=strlen(pszContent);
        char *pszProtocol=new char[nLen+1];

        if (pszProtocol != NULL)
        {
            strcpy(pszProtocol,pszContent);

            LOG_INFO(("[CBurnControlDlg::OnRecvProtocol] Post msg recv protocol,%d\r\n",__LINE__))

            ::PostMessage(pBurnControlDlg->GetSafeHwnd(),WMUSER_BURNCONTROLDLG_ONRECVPROTOCOL,
                0,(LPARAM)pszProtocol);
        }
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::OnRecvProtocol] Param error,%d\r\n",__LINE__));
    }
}

LRESULT CBurnControlDlg::OnRecvProtocol(WPARAM wParam, LPARAM lParam)
{
    char *pszProtocol=(char *)lParam;
    if (NULL == pszProtocol)
    {
        LOG_ERROR(("[CBurnControlDlg::OnRecvProtocol] Protocol content is NULL,%d\r\n",__LINE__));
        return 0;
    }

    std::string strProtocol=std::string(pszProtocol);
    delete [] pszProtocol;
    pszProtocol=NULL;

    LOG_INFO(("[CBurnControlDlg::OnRecvProtocol] ######## Recv msg : %d ######## \r\n%s\r\n",__LINE__,
        strProtocol.c_str()));

    if (m_bBackupJob)
    {
        if (IDYES == MessageBox(_T("�ϴιر�ǰ��δ��ɵ������Ƿ�ָ�������"),_T("����"),MB_YESNO|MB_TOPMOST))
        {
            LOG_INFO(("[CBurnControlDlg::OnInitDialog] Have running job before last exit, user chose resume it!strJobID: %s, jobState: %s\r\n"
                , m_RTBurnInfo.GetStateFlag().c_str(), m_RTBurnInfo.GetStateDescription().c_str()));
            m_eBurnCategory=BURNCATEGORY_RTBURN;
            CBurnServerInfoEx burnServerInfoEx(m_RTBurnInfo.GetBurnServerInfo());
            m_vecBurnServerInfoEx.push_back(burnServerInfoEx);
            ShowBurnControlUI(true);
            m_nBurnState = BURNCONTROL_STATE_RUNNING;
            UpdateBurnControlUI();
            if (m_RTBurnInfo.GetStateFlag() == STATE_RTTASK_STOPPING)
            {
                ShowBurnControlUIUnable();
            }
            ShowWindow(SW_SHOW);
            m_bBackupJob = false;
            DeleteRTBBackupXML();
            return 0;
        }
        else
        {
            m_bBackupJob = false;
            DeleteRTBBackupXML();
            LOG_INFO(("[CBurnControlDlg::OnInitDialog] Have running job before last exit, user chose NOT resume it!strJobID: %s, jobState: %s\r\n"
                , m_RTBurnInfo.GetStateFlag().c_str(), m_RTBurnInfo.GetStateDescription().c_str()));
        }
    }
    std::string strCommandType=CInterfaceProtocol::GetCommandType(strProtocol.c_str());
    if (CMD_SENDRTBURNINFO == strCommandType ||
        CMD_SENDBURNINFO == strCommandType ||
        CMD_STARTRTBURN == strCommandType ||
        CMD_STARTHM3RTBURN == strCommandType)
    {
        if (BURNCONTROL_STATE_RUNNING == m_nBurnState ||
            BURNCONTROL_STATE_PAUSED == m_nBurnState)
        {
            LOG_WARNING(("[CBurnControlDlg::OnRecvProtocol] Is burning,%d\r\n",
                __LINE__));

            //������ڿ�¼����ô��ʾ��¼����
            ShowWindow(SW_SHOW);
            SetForegroundWindow();
            return 0;
        }
        else
        {
            //�����ڲ�����
            ResetBurnInfo();

            m_strProtocol=strProtocol;
            m_nBurnState=BURNCONTROL_STATE_READY;
        }
    }

    if (CMD_SENDRTBURNINFO == strCommandType)//ʵʱ��¼������Ϣ
    {
        m_eBurnCategory=BURNCATEGORY_RTBURN;

        CInterfaceProtocol::AnalyzeProtocolSendRTBurnInfo(strProtocol.c_str(),m_RTBurnInfo);

        //��ʱ��������ϲ�����͵Ŀ�¼��������Ϣ����Щ��Ϣ��һ������Ч��
        CBurnServerInfoEx burnServerInfoEx(m_RTBurnInfo.GetBurnServerInfo());
        m_vecBurnServerInfoEx.push_back(burnServerInfoEx);

        ShowBurnControlUI(true);
    }
    else if (CMD_SENDBURNINFO == strCommandType)//�º��¼Э��
    {
        CInterfaceProtocol::AnalyzeProtocolSendBurnInfo(strProtocol.c_str(),m_normalJob);

        std::string strBurnFlag=m_normalJob.GetBurnFlag();
        if (BURN_FLAG_COS == strBurnFlag)
        {
            m_eBurnCategory=BURNCATEGORY_COS;

            for (size_t i=0;i<m_normalJob.GetVecBurnServerInfo().size();++i)
            {
                CBurnServerInfoEx burnServerInfoEx(m_normalJob.GetVecBurnServerInfo().at(i));
                m_vecBurnServerInfoEx.push_back(burnServerInfoEx);

                LOG_INFO(("-------- Dev info from upper --------,%d\r\n",__LINE__));
                std::vector<CCDROMDriverInfo>::iterator iterT;
                for (iterT=m_vecBurnServerInfoEx.back().GetVecCDROMDriverInfo().begin();
                    iterT!=m_vecBurnServerInfoEx.back().GetVecCDROMDriverInfo().end();++iterT)
                {
                    LOG_INFO(("Dev id : %s,is selected : %s\r\n",iterT->GetID().c_str(),iterT->GetIsSelected().c_str()));
                }
                LOG_INFO(("----------------\r\n"));
            }

            ShowBurnControlUI(true);
        }
        else if(BURN_FLAG_CMS == strBurnFlag)
        {
            int nJobNum=m_normalJob.GetVecNormalBurnJobInfo().size();
            if (nJobNum > 1)//������
            {
                m_eBurnCategory=BURNCATEGORY_MULTI;

                for (size_t i=0;i<m_normalJob.GetVecBurnServerInfo().size();++i)
                {
                    CBurnServerInfoEx burnServerInfoEx(m_normalJob.GetVecBurnServerInfo().at(i));
                    m_vecBurnServerInfoEx.push_back(burnServerInfoEx);
                }

                ShowBurnControlUI(true);
            } 
            else//������
            {
                m_eBurnCategory=BURNCATEGORY_SINGLE;

                for (size_t i=0;i<m_normalJob.GetVecBurnServerInfo().size();++i)
                {
                    CBurnServerInfoEx burnServerInfoEx(m_normalJob.GetVecBurnServerInfo().at(i));
                    m_vecBurnServerInfoEx.push_back(burnServerInfoEx);
                }

                ShowBurnControlUI(true);
            }
        }  
    }
    else if (CMD_STARTRTBURN == strCommandType)//��ʼʵʱ��¼
    {
        m_eBurnCategory=BURNCATEGORY_RTBURN;

        CInterfaceProtocol::AnalyzeProtocolStartRTBurnInfo(strProtocol.c_str(),m_RTBurnInfo);

        CBurnServerInfoEx burnServerInfoEx(m_RTBurnInfo.GetBurnServerInfo());
        m_vecBurnServerInfoEx.push_back(burnServerInfoEx);

        LOG_INFO(("-------- Dev info from upper --------,%d\r\n",__LINE__));
        std::vector<CCDROMDriverInfo>::iterator iterT;
        for (iterT=m_vecBurnServerInfoEx.back().GetVecCDROMDriverInfo().begin();
            iterT!=m_vecBurnServerInfoEx.back().GetVecCDROMDriverInfo().end();++iterT)
        {
            LOG_INFO(("Single Dev id : %s,is selected : %s\r\n",iterT->GetID().c_str(),
                iterT->GetIsSelected().c_str()));
        }
        LOG_INFO(("-------------------------------------\r\n"));

        ShowBurnControlUI(false);

        StartRtBurn(true);
    }
    else if (CMD_STARTHM3RTBURN == strCommandType)//����Ҫ������Ϣ��ʵʱ��¼����HM3
    {
        m_eBurnCategory=BURNCATEGORY_HM3;

        CInterfaceProtocol::AnalyzeProtocolSendRTBurnInfo(strProtocol.c_str(),m_RTBurnInfo);

        CBurnServerInfoEx burnServerInfoEx(m_RTBurnInfo.GetBurnServerInfo());
        m_vecBurnServerInfoEx.push_back(burnServerInfoEx);

        ShowBurnControlUI(true);
    }
    else if (CMD_SENDBURNCTRLCMD == strCommandType)
    {
        strCommandType=CInterfaceProtocol::GetTagValue(strProtocol,"ctrlCMD");
        if ("stopRTBurn" == strCommandType ||
            "pauseRTBurn" == strCommandType)
        {
            m_eBurnControlCmd=BURNCONTROL_CMD_PAUSE_STOP;

            if ("stopRTBurn" == strCommandType)
            {
                ShowBurnControlUIUnable();
            }
        }
        else if ("startRTBurn" == strCommandType ||
            "resumeRTBurn" == strCommandType)
        {
            m_eBurnControlCmd=BURNCONTROL_CMD_START_RESUME;
        }

        LOG_INFO(("[CBurnControlDlg::OnRecvProtocol] Command : %s,%d\r\n",strCommandType.c_str(),__LINE__));

        SendBurnControlCMDToBurnServer(strCommandType);
    }
    else if (CMD_SENDBURNSTATE == strCommandType)//BurnServer���͵ĵ�ǰ��¼״̬
    {
        std::string strJobID=CInterfaceProtocol::GetJobID(strProtocol.c_str());

        //strStateFlag //����¼Э�顷�������ͽӿ�2.7.4
        std::string strJobState=CInterfaceProtocol::GetTagValue(strProtocol,"jobState");

        std::string strJobStateDescription=CInterfaceProtocol::GetTagValue(
            strProtocol,"jobStateDescription");

        LOG_INFO(("[CBurnControlDlg::OnRecvProtocol] Cur state : %s,%s,%d\r\n",
            strJobState.c_str(),strJobStateDescription.c_str(),__LINE__));

        std::vector<CCDROMDriverInfo> vecCDROMInfo;

        if(EXTRA_STATE_RTBURN_BACKUP_YES == strJobState)
        {
            //ʵʱ��¼�������̵���Ϣ
            DealWithBurnStateCallBackMsg(strJobID,strJobState,strJobStateDescription,vecCDROMInfo);
        }
        else if (EXTRA_STATE_RTBURN_BACKUP_RESTART_YES == strJobState)
        {
            //ʵʱ��¼�����豸���̵���Ϣ
            DealWithBurnStateCallBackMsg(strJobID,strJobState,strJobStateDescription,vecCDROMInfo);
        }
        else if (GetCurJobID() == strJobID)
        {
            if (m_nBurnState != BURNCONTROL_STATE_IDLE &&
                m_nBurnState != BURNCONTROL_STATE_ERROR)
            {
                CInterfaceProtocol::AnalyzeProtocolSendBurnState(strProtocol,vecCDROMInfo);

                if (strJobState != m_RTBurnInfo.GetStateFlag()
                    || strJobStateDescription != m_RTBurnInfo.GetStateDescription())
                {
                    m_RTBurnInfo.SetStateFlag(strJobState);
                    m_RTBurnInfo.SetStateDescription(strJobStateDescription);
                    if (m_eBurnCategory == BURNCATEGORY_RTBURN
                        || m_eBurnCategory == BURNCATEGORY_HM3)
                    {//ֻ��ʵʱ��¼��Ҫ�ָ�
                        WriteRTBBackupXML(m_RTBurnInfo);
                    }
                }
                DealWithBurnStateCallBackMsg(strJobID,strJobState,strJobStateDescription,vecCDROMInfo);
            }
            else
            {
                //��ɱ�����ڿ�¼�Ľ��棬���ͺ�ǰһ��������һ��������ʱ�����յ��£����ӣ���������Ϣ
                //��ʱ�������������ȥ����������Ϣ����ִ���Ľ��棬��ȷӦ��ֻ��ʾ��ǰ�������ڱ�ִ��
            }
        } 
        else if (strJobState != "")//�����ǲ���ʱ���͵Ŀ�¼״̬��Ϣ
        {
            LOG_INFO(("[CBurnControlDlg::OnRecvProtocol] Here,%s,%d\r\n",strJobID.c_str(),__LINE__));

            DealWithBurnStateCallBackMsg(strJobID,strJobState,strJobStateDescription,vecCDROMInfo);
        }
        else
        {
            LOG_WARNING(("[CBurnControlDlg::OnRecvProtocol] Received \"sendBurnState\",but job id is wrong\r\n"));
        }
    }
    else if (CMD_GETJOBSTATE == strCommandType)
    {
        std::string strJobID=CInterfaceProtocol::GetJobID(strProtocol.c_str());

        std::string strJobState=TO_UPPER_BURN_STATE_FAILED;

        if (strJobID == GetCurJobID())
        {
            switch (m_nBurnState)
            {
            case BURNCONTROL_STATE_ERROR:
                strJobState=TO_UPPER_BURN_STATE_FAILED;
            	break;

            case BURNCONTROL_STATE_IDLE:
                strJobState=TO_UPPER_BURN_STATE_STOPPED;
                break;

            case BURNCONTROL_STATE_READY:
                strJobState=TO_UPPER_BURN_STATE_STOPPED;
                break;

            case BURNCONTROL_STATE_RUNNING:
                strJobState=TO_UPPER_BURN_STATE_BURNING;
                break;

            case BURNCONTROL_STATE_PAUSED:
                strJobState=TO_UPPER_BURN_STATE_PAUSED;
                break;
            }

            SendCurBurnStateToUpper("","",strJobID,strJobState,"");
        }
    }

    return 0;
}

LRESULT CBurnControlDlg::OnBurnBackupYes(WPARAM wParam, LPARAM lParam)
{
    CBurnServerInfo burnServerInfo;
    if (GetCurBurnServerSel(burnServerInfo))
    {
        std::string strProtocolContent=
            CInterfaceProtocol::GenerateProtocolRTBurnBackupResponse(GetCurJobID(),RTBURN_BACKUP_YES);

        std::string strIP=burnServerInfo.GetIP();
        std::string strPort=burnServerInfo.GetPort();

        AsyncSendNCXMsg(strIP,strPort,strProtocolContent);
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::OnBurnBackupYes] GetCurBurnServerSel failed\r\n"))
    }

    return 0;
}

LRESULT CBurnControlDlg::OnBurnBackupNo(WPARAM wParam, LPARAM lParam)
{
    CBurnServerInfo burnServerInfo;
    if (GetCurBurnServerSel(burnServerInfo))
    {
        std::string strProtocolContent=
            CInterfaceProtocol::GenerateProtocolRTBurnBackupResponse(GetCurJobID(),RTBURN_BACKUP_NO);

        std::string strIP=burnServerInfo.GetIP();
        std::string strPort=burnServerInfo.GetPort();

        AsyncSendNCXMsg(strIP,strPort,strProtocolContent);
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::OnBurnBackupNo] GetCurBurnServerSel failed\r\n"))
    }

    return 0;
}

LRESULT CBurnControlDlg::OnBurnBackupAfterRestartYes(WPARAM wParam, LPARAM lParam)
{
    CBurnServerInfo burnServerInfo;
    if (GetCurBurnServerSel(burnServerInfo))
    {
        std::string strProtocolContent=
            CInterfaceProtocol::GenerateProtocolRTBurnBackupAfterRestartResponse(GetCurJobID(),
            RTBURN_BACKUP_AFTER_RESTART_YES);

        std::string strIP=burnServerInfo.GetIP();
        std::string strPort=burnServerInfo.GetPort();

        AsyncSendNCXMsg(strIP,strPort,strProtocolContent);
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::OnBurnBackupAfterRestartYes] GetCurBurnServerSel failed\r\n"))
    }

    return 0;
}

LRESULT CBurnControlDlg::OnBurnBackupAfterRestartNo(WPARAM wParam, LPARAM lParam)
{
    CBurnServerInfo burnServerInfo;
    if (GetCurBurnServerSel(burnServerInfo))
    {
        std::string strProtocolContent=
            CInterfaceProtocol::GenerateProtocolRTBurnBackupAfterRestartResponse(GetCurJobID(),RTBURN_BACKUP_AFTER_RESTART_NO);

        std::string strIP=burnServerInfo.GetIP();
        std::string strPort=burnServerInfo.GetPort();

        AsyncSendNCXMsg(strIP,strPort,strProtocolContent);
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::OnBurnBackupAfterRestartNo] GetCurBurnServerSel failed\r\n"))
    }

    return 0;
}

LRESULT CBurnControlDlg::OnSendCmdFailed(WPARAM wParam, LPARAM lParam)
{
    DESKTOP_ALERT_PARAM param;
    param.strFlag=STATE_BURN_ERR;

    switch (wParam)
    {
    case ERR_SEND_STARTBURN_MSG_FAILED:
        param.strDescription=m_cfgFile.GetValue("info","sendburncmderr");
    	break;

    case ERR_SEND_SAVEBURNINFO_MSG_FAILED:
        param.strDescription=m_cfgFile.GetValue("info","savecfgfailed");
        break;

    case ERR_SEND_BURNCMDREQUEST_MSG_FAILED:
        param.strDescription=m_cfgFile.GetValue("info","sendrtrequestfailed");
        break;

    case ERR_SEND_BURNCMD_MSG_FAILED:
        param.strDescription=m_cfgFile.GetValue("info","sendburncmderr");
        break;

    default:
        break;
    }

    param.bAutoClose=TRUE;
    param.nAutoCloseTime=4;

    ShowDesktopAlert(param);

    m_nBurnState=BURNCONTROL_STATE_IDLE;

    UpdateBurnControlUI();

    return 0;
}

LRESULT CBurnControlDlg::OnAsyncMsgResult(WPARAM wParam, LPARAM lParam)
{
    E_COMMAND_TYPE eCmdType=static_cast<E_COMMAND_TYPE>(wParam);
    unsigned int nResult=static_cast<unsigned int>(lParam);

    switch (eCmdType)
    {
    case START_RT_BURN:
        {
            if (1 == nResult)
            {
                if (m_nBurnState != BURNCONTROL_STATE_ERROR)
                {
                    m_eBurnControlCmd=BURNCONTROL_CMD_START_RESUME;

                    TRACE("[CBurnControlDlg::OnAsyncMsgResult] %lu,%d\r\n",
                        GetTickCount(),__LINE__);

                    UpdateBurnControlUI();

                    SetConnectServerFailedCount(0);

                    SetTimer(ID_TIMER_SENDKEEPALIVEMSG,6000,NULL);
                }
            }
            else
            {
                LOG_ERROR(("[CBurnControlDlg::OnAsyncMsgResult] Send startRTBurn msg failed,%d\r\n",
                    __LINE__));

                PostMessageW(WMUSER_BURNCONTROLDLG_SENDCMDFAILED,ERR_SEND_STARTBURN_MSG_FAILED,0);
            }
        }
    	break;

    case START_BURN:
        {
            if (1 == nResult)
            {
                if (m_nBurnState != BURNCONTROL_STATE_ERROR)
                {
                    m_nBurnState=BURNCONTROL_STATE_RUNNING;

                    UpdateBurnControlUI();

                    SetConnectServerFailedCount(0);

                    SetTimer(ID_TIMER_SENDKEEPALIVEMSG,6000,NULL);
                }
            }
            else
            {
                LOG_ERROR(("[CBurnControlDlg::OnAsyncMsgResult] Send startRTBurn msg failed,%d\r\n",
                    __LINE__));

                PostMessageW(WMUSER_BURNCONTROLDLG_SENDCMDFAILED,ERR_SEND_STARTBURN_MSG_FAILED,0);
            }
        }
        break;

    case SEND_RT_BURN_CONFIG:
        {
            if (1 == nResult)
            {
                PostMessage(WM_SYSCOMMAND,SC_MINIMIZE,0);
            }
            else
            {
                LOG_ERROR(("[CBurnControlDlg::OnAsyncMsgResult] Send sendRTBurnConfig msg failed,%d\r\n",
                    __LINE__));

                PostMessageW(WMUSER_BURNCONTROLDLG_SENDCMDFAILED,ERR_SEND_SAVEBURNINFO_MSG_FAILED,0);
            }
        }
        break;

    case SEND_RT_BURN_REQUEST:
        {
            if (1 == nResult)
            {
            } 
            else
            {
                LOG_ERROR(("[CBurnControlDlg::OnAsyncMsgResult] Send sendRTBurnRequest msg failed,%d\r\n",
                    __LINE__));

                PostMessageW(WMUSER_BURNCONTROLDLG_SENDCMDFAILED,ERR_SEND_BURNCMDREQUEST_MSG_FAILED,0);
            }
        }
        break;

    case SEND_BUTN_CTRL_CMD:
        {
            if (1 == nResult)
            {
            } 
            else
            {
                LOG_ERROR(("[CBurnControlDlg::OnAsyncMsgResult] Send sendBurnCtrlCMD msg failed,%d\r\n",
                    __LINE__));

                PostMessageW(WMUSER_BURNCONTROLDLG_SENDCMDFAILED,ERR_SEND_BURNCMD_MSG_FAILED,0);
            }
        }
        break;

    default:
        assert(false);
        break;
    }

    return 0;
}

void CBurnControlDlg::DealWithBurnStateCallBackMsg(std::string strJobID,
                                                   std::string strStateFlag,std::string strStateDescription,
                                                   const std::vector<CCDROMDriverInfo> &vecDevInfo)
{
    //strStateFlag //����¼Э�顷�������ͽӿ�2.7.4

    //���ϲ㷴���Ŀ�¼״̬
    std::string strBurnStateToUpper;

    std::string strUpperIP;
    std::string strUpperPort;

    std::vector<CCDROMDriverInfo> vecDevTemp=vecDevInfo;

    //ʵʱ��¼����task�������κ�job����Ϊʵʱ��¼���̽���ʹ�õ�һ������
    // ��������ÿ��������״̬ΪstrStateDescription
    if ("" == strJobID)
    {
        vecDevTemp.clear();

        CCDROMDriverInfo devInfo;
        devInfo.SetID("0");
        devInfo.SetStateDescription(strStateDescription);

        vecDevTemp.push_back(devInfo);
    }

    CBurnServerInfo burnServerInfo;
    std::string strBurnServerType;
    if (GetCurBurnServerSel(burnServerInfo))
    {
        strBurnServerType=burnServerInfo.GetBurnServerType();
    }

    if (STATE_SINGLE_DEV_ERR_JOB_FAILED == strStateFlag)
    {
        strBurnStateToUpper=TO_UPPER_BURN_STATE_FAILED;

        UpdateCDROMState(vecDevTemp);

        ProcessJobFailed(strStateFlag,strStateDescription);
    } 
    else if (STATE_SINGLE_DEV_ERR == strStateFlag)
    {
        strBurnStateToUpper=TO_UPPER_BURN_STATE_BURNING_NOT_PERFECT;

        UpdateCDROMState(vecDevTemp);
    } 
    else if(STATE_BURN_ERR == strStateFlag)
    {
        strBurnStateToUpper=TO_UPPER_BURN_STATE_FAILED;

        if (BURNSERVER_TYPE_NORMAL != strBurnServerType)
        {
            UpdateUIJobStateInformation(strStateDescription);
        } 
        else
        {
            std::vector<CCDROMDriverInfo> curVecCDROMDriverInfo=GetCurCDROMDriverInfoVec();

            SetCDROMDriverCommonState(strStateFlag,strStateDescription,curVecCDROMDriverInfo);

            UpdateCDROMState(curVecCDROMDriverInfo);
        }

        ProcessJobFailed(strStateFlag,strStateDescription);

        if (BURNCATEGORY_HM3 == m_eBurnCategory ||
            BURNCATEGORY_RTBURN == m_eBurnCategory)
        {
            DESKTOP_ALERT_PARAM param;
            param.strFlag=STATE_BURN_ERR;
            param.strDescription=strStateDescription;
            ShowDesktopAlert(param);
        }
    }
    else if(STATE_DEFAULT == strStateFlag)
    {
        strBurnStateToUpper=TO_UPPER_BURN_STATE_BURNING;

        if (BURNSERVER_TYPE_NORMAL != strBurnServerType)
        {
            UpdateUIJobStateInformation(strStateDescription);
        } 
        else
        {
            UpdateCDROMState(vecDevTemp);
        }
    }
    else if (STATE_BURNING == strStateFlag)
    {
        strBurnStateToUpper=TO_UPPER_BURN_STATE_BURNING;

        if (BURNCONTROL_CMD_START_RESUME == m_eBurnControlCmd)
        {
            m_nBurnState=BURNCONTROL_STATE_RUNNING;

            UpdateBurnControlUI();
        }

        if (BURNSERVER_TYPE_NORMAL != strBurnServerType)
        {
            UpdateUIJobStateInformation(strStateDescription);
        } 
        else
        {
            UpdateCDROMState(vecDevTemp);
        }

        //�����ʵʱ��¼���̣���ʱ����Ӧ�����º��¼���߼����棬
        //���Բ�Ӧ�ó���ֹͣ����ͣ��¼ѡ��
        if ("" == strJobID)
        {
            ShowBurnControlUIUnable();
        }
    }
    else if (STATE_RTTASK_STOPPING == strStateFlag)
    {//ĳһTaskΪ����ֹͣ��¼��״̬������ť��Ϊ������״̬
        strBurnStateToUpper=TO_UPPER_BURN_STATE_BURNING;
        ShowBurnControlUIUnable();
        UpdateCDROMState(vecDevTemp);
    }
    else if (STATE_SINGLE_DISC_BURNED == strStateFlag)
    {
        strBurnStateToUpper=TO_UPPER_BURN_STATE_BURNING;

        UpdateBurnControlUI();
        UpdateCDROMState(vecDevTemp);
    }
    else if (STATE_DOWNLOADING == strStateFlag)
    {
        strBurnStateToUpper=TO_UPPER_BURN_STATE_BURNING;

        if (BURNSERVER_TYPE_NORMAL != strBurnServerType)
        {
            UpdateUIJobStateInformation(strStateDescription);
        } 
        else
        {
            std::vector<CCDROMDriverInfo> curVecCDROMDriverInfo=GetCurCDROMDriverInfoVec();

            SetCDROMDriverCommonState(strStateFlag,strStateDescription,curVecCDROMDriverInfo);

            UpdateCDROMState(curVecCDROMDriverInfo); 
        }

    }
    else if (STATE_WILL_CLOSE_DISC == strStateFlag)
    {
        strBurnStateToUpper=TO_UPPER_BURN_STATE_BURNING;

        if (BURNSERVER_TYPE_NORMAL != strBurnServerType)
        {
            assert(false);
        } 
        else
        {
            UpdateCDROMState(vecDevTemp);
        }
    }
    else if (STATE_CHANGE_DISC == strStateFlag)
    {
        strBurnStateToUpper=TO_UPPER_BURN_STATE_BURNING;

        if (BURNSERVER_TYPE_NORMAL != strBurnServerType)
        {
        } 
        else
        {
            UpdateCDROMState(vecDevTemp);
        }
    }
    else if (STATE_TASK_OVER == strStateFlag)
    {
        strBurnStateToUpper=TO_UPPER_BURN_STATE_BURNING;

        if (BURNSERVER_TYPE_NORMAL != strBurnServerType)
        {

        } 
        else
        {
            UpdateCDROMState(vecDevTemp);
        }
    }
    else if (STATE_JOB_OVER == strStateFlag)
    {
        strBurnStateToUpper=TO_UPPER_BURN_STATE_STOPPED;

        if (BURNSERVER_TYPE_NORMAL != strBurnServerType)
        {
            UpdateUIJobStateInformation(strStateDescription);
        } 
        else
        {
            std::vector<CCDROMDriverInfo> vecCurDevInfo=GetCurCDROMDriverInfoVec();

            size_t nDevCount=vecCurDevInfo.size();
            for (size_t i=0;i<nDevCount;++i)
            {
                std::string strDevID=vecCurDevInfo.at(i).GetID();

                size_t n=vecDevInfo.size();
                for (size_t j=0;j<n;++j)
                {
                    if (vecDevInfo.at(j).GetID() == strDevID)
                    {
                        vecCurDevInfo.at(i).SetVecLocationInfo(vecDevInfo.at(j).GetVecLocationInfo());

                        break;
                    }
                }
            }

            SetCDROMDriverCommonState(strStateFlag,strStateDescription,vecCurDevInfo);

            UpdateCDROMState(vecCurDevInfo);
        }

        ProcessJobDone();
    }
    else if (STATE_JOB_FINISHED == strStateFlag)
    {
        strBurnStateToUpper=TO_UPPER_BURN_STATE_FAILED;

        if (BURNSERVER_TYPE_NORMAL != strBurnServerType)
        {
            UpdateUIJobStateInformation(strStateDescription);
        } 
        else
        {
            UpdateCDROMState(vecDevTemp); 
        }

        ProcessJobFailed(strStateFlag,strStateDescription);
    }
    else if (STATE_RTJOB_PAUSED == strStateFlag)
    {
        //ʵʱ��¼�Ѿ���ͣ
        strBurnStateToUpper=TO_UPPER_BURN_STATE_PAUSED;

        m_nBurnState=BURNCONTROL_STATE_PAUSED;

        UpdateCDROMState(vecDevTemp);

        UpdateBurnControlUI();
    }
    else if (STATE_RTJOB_STOPPED == strStateFlag)
    {
        //ʵʱ��¼�Ѿ�ֹͣ
        strBurnStateToUpper=TO_UPPER_BURN_STATE_STOPPED;

        m_nBurnState=BURNCONTROL_STATE_IDLE;

        UpdateCDROMState(vecDevTemp);

        UpdateBurnControlUI();
    }
    else if (EXTRA_STATE_RTBURN_BACKUP_YES == strStateFlag)
    {
        DESKTOP_ALERT_PARAM param;
        param.strFlag=strStateFlag;

        std::string str=m_cfgFile.GetValue("info","startburnbackupornot");
        if (!str.empty())
        {
            strStateDescription+=std::string(",")+str;
        }
        param.strDescription=strStateDescription;

        ShowDesktopAlert(param);
    }
    else if (EXTRA_STATE_RTBURN_BACKUP_RESTART_YES == strStateFlag)
    {
        DESKTOP_ALERT_PARAM param;
        param.strFlag=strStateFlag;

        std::string str=m_cfgFile.GetValue("info","startburnbackupafterrestartornot");
        if (!str.empty())
        {
            strStateDescription+=std::string(",")+str;
        }
        param.strDescription=strStateDescription;

        ShowDesktopAlert(param);
    }

    LOG_INFO(("[CBurnControlDlg::DealWithBurnStateCallBackMsg] Flag : %s,%d\r\n",
        strStateFlag.c_str(),__LINE__));

    SendCurBurnStateToUpper("","",strJobID,strBurnStateToUpper,strStateDescription);
}

void CBurnControlDlg::UpdateUIJobStateInformation(std::string strStateDescription)
{
    CListCtrl *pListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_BURNSTATE));
    if (pListCtrl != NULL &&
        pListCtrl->GetSafeHwnd() != NULL)
    {
        int nColumnCount=pListCtrl->GetHeaderCtrl()->GetItemCount();
        if(0 == nColumnCount)
        {
            CRect rect;
            pListCtrl->GetClientRect(&rect);

            pListCtrl->InsertColumn(0,L"",LVCFMT_CENTER,rect.Width());
        }
        else if (nColumnCount > 1)
        {
            while (pListCtrl->GetHeaderCtrl()->GetItemCount() >1)
            {
                pListCtrl->GetHeaderCtrl()->DeleteItem(0);
            }
        }

        int nItemCount=pListCtrl->GetItemCount();
        if(0 == nItemCount)
        {
            pListCtrl->InsertItem(0,L"");
        }
        else if (nItemCount > 1)
        {
            while (pListCtrl->GetItemCount() > 1)
            {
                pListCtrl->DeleteItem(0);
            }
        }

        CStringW str=CharsetConvertMFC::UTF8ToUTF16(strStateDescription.c_str());
        pListCtrl->SetItemText(0,0,str);
    }
}

void CBurnControlDlg::SetCDROMDriverCommonState(std::string strStateFlag,std::string strStateDescription,
                                                std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo)
{
    std::vector<CCDROMDriverInfo>::iterator iter;
    for (iter=vecCDROMDriverInfo.begin();iter!=vecCDROMDriverInfo.end();)
    {
        if ("1" == iter->GetIsSelected())
        {
            iter->SetStateFlag(strStateFlag);
            iter->SetStateDescription(strStateDescription);

            ++iter;
        }
        else
        {
            iter=vecCDROMDriverInfo.erase(iter);
        }
    }
}

void CBurnControlDlg::ProcessJobDone()
{
    CleanJobData();

    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_RTBURN:
    case BURNCATEGORY_HM3:
        m_nBurnState=BURNCONTROL_STATE_IDLE;
        break;

    case BURNCATEGORY_SINGLE:
    case BURNCATEGORY_COS:
        m_nBurnState=BURNCONTROL_STATE_IDLE;
        break;

    case BURNCATEGORY_MULTI:
        {
            std::vector<CNormalBurnJobInfoEx> &vecNormalBurnJobInfoEx=m_normalJob.GetVecNormalBurnJobInfo();

            std::string strJobStateMsg;
            CListCtrl *pListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_JOBLIST));
            if (pListCtrl != NULL)
            {
                if (pListCtrl->GetSafeHwnd() != NULL)
                {
                    int nItemCount=pListCtrl->GetItemCount();
                    if (nItemCount > static_cast<int>(m_nJobIndex))
                    {
                        strJobStateMsg=m_cfgFile.GetValue("uiword","burnsuccess").c_str();
                        if (strJobStateMsg != "")
                        {
                            pListCtrl->SetItemText(m_nJobIndex,1,CharsetConvertMFC::UTF8ToUTF16(strJobStateMsg.c_str()));
                        }

                        strJobStateMsg=m_cfgFile.GetValue("uiword","burning").c_str();
                        if (strJobStateMsg != "")
                        {
                            pListCtrl->SetItemText(m_nJobIndex+1,1,CharsetConvertMFC::UTF8ToUTF16(strJobStateMsg.c_str()));
                        }
                    }
                    else
                    {
                        LOG_ERROR(("[CBurnControlDlg::OnRecvProtocol] Job list num error,item : %d,job index : %d,%d\r\n",
                            nItemCount,m_nJobIndex,__LINE__));

                        assert(false);
                    }
                }
            }

            //��ʶ�µ��������
            m_nJobIndex++;

            if (vecNormalBurnJobInfoEx.size() > m_nJobIndex)//��ʾ���������ڵȴ���ִ��
            {
                CreateAdditionalBurnFile();

                SendBurnJobToBurnServer();
            }
            else
            {
                m_nBurnState=BURNCONTROL_STATE_IDLE;
            }
        }
        break;

    case BURNCATEGORY_UNKNOWN:
    default:
        assert(false);
        LOG_ERROR(("[CBurnControlDlg::OnRecvProtocol] Burn category error\r\n"));
        break;
    }

    UpdateBurnControlUI();
}

void CBurnControlDlg::ProcessJobFailed(std::string strStateFlag,std::string strStateDescription)
{
    KillTimer(ID_TIMER_SENDKEEPALIVEMSG);

    LOG_ERROR(("[CBurnControlDlg::ProcessJobFailed] Detect failed job,%d\r\n",__LINE__));

    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_RTBURN:
    case BURNCATEGORY_HM3:
        m_nBurnState=BURNCONTROL_STATE_ERROR;
        break;

    case BURNCATEGORY_SINGLE:
    case BURNCATEGORY_COS:
        m_nBurnState=BURNCONTROL_STATE_ERROR;
        break;

    case BURNCATEGORY_MULTI:
        {
            CListCtrl *pListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_JOBLIST));
            if (pListCtrl != NULL)
            {
                if (pListCtrl->GetSafeHwnd() != NULL)
                {
                    int nItemCount=pListCtrl->GetItemCount();

                    assert(nItemCount > static_cast<int>(m_nJobIndex));

                    pListCtrl->SetItemText(m_nJobIndex,1,CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("uiword","burnfailed").c_str()));

                    if (static_cast<int>(m_nJobIndex) < nItemCount-1)
                    {
                        pListCtrl->SetItemText(m_nJobIndex+1,1,CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("uiword","burning").c_str()));
                    }
                }
            }

            m_nJobIndex++;

            size_t nJobCount=m_normalJob.GetVecNormalBurnJobInfo().size();
            if (nJobCount > m_nJobIndex)
            {
                CreateAdditionalBurnFile();

                SendBurnJobToBurnServer();
            }
            else
            {
                m_nBurnState=BURNCONTROL_STATE_ERROR;
            }
        }
        break;

    case BURNCATEGORY_UNKNOWN:
    default:
        LOG_ERROR(("[CBurnControlDlg::ProcessJobFailed] Burn category error\r\n"));
        assert(false);
        break;
    }

    UpdateBurnControlUI();
}

void CBurnControlDlg::CleanJobData()
{
    std::string strDataDir=m_cfgFile.GetValue("path","tempdir");

    if (strDataDir != "")
    {
        LOG_INFO(("[CBurnControlDlg::CleanJobData] Data dir path : %s\r\n",strDataDir.c_str()));

        CStringW str=CharsetConvertMFC::UTF8ToUTF16(strDataDir.c_str());

        DirectoryUtil::DeleteDir(CStringA(str));
    }
}

void CBurnControlDlg::InitConfig()
{
    char szCfgFilePath[1024]={0};
    sprintf_s(szCfgFilePath,sizeof(szCfgFilePath)/sizeof(char),"%s",BURNCONTROL_CFG_PATH);
    strcat(szCfgFilePath,"BurnControlCfg.xml");

    if (0 == m_cfgFile.LoadFile(szCfgFilePath))
    {
        LOG_INFO(("[CBurnControlDlg::InitConfig] Load BurnControl config file success\r\n"));
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::InitConfig] Load BurnControl config file failed,%s\r\n",
            szCfgFilePath));
    }
}

void CBurnControlDlg::Layout(ConfigurableFileEx cfg,std::string strSection,CWnd *pWnd)
{
    if (NULL == pWnd ||
        NULL == pWnd->GetSafeHwnd())
    {
        return;
    }

    CRect rect;
    if (cfg.GetRect(strSection,rect))
    {
        pWnd->MoveWindow(&rect);

        pWnd->ShowWindow(SW_SHOW);
        SetForegroundWindow();
    }
    else
    {
        pWnd->ShowWindow(SW_HIDE);
    }
}

void CBurnControlDlg::ClearDevBurnStateInfo()
{
    CListCtrl *pListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_BURNSTATE));
    if (pListCtrl != NULL)
    {
        if (pListCtrl->GetSafeHwnd() != NULL)
        {
            pListCtrl->DeleteAllItems();
        }
    }
}

void CBurnControlDlg::CreateUI(std::string strUICfgFile)
{
    ConfigurableFileEx uiCfgFile;

    if (0 == uiCfgFile.LoadFile(strUICfgFile))
    {
        LOG_INFO(("[CBurnControlDlg::CreateUI] Load UI config file success\r\n"));

        CRect rect;

        //�����С
        if (uiCfgFile.GetRect("burncontrolui",rect))
        {
            CRect rectDesktop;
            ::GetWindowRect(GetDesktopWindow()->GetSafeHwnd(),rectDesktop);

            int dx=(rectDesktop.left+rectDesktop.right)/2-(rect.left+rect.right)/2;
            int dy=(rectDesktop.top+rectDesktop.bottom)/2-(rect.top+rect.bottom)/2;

            rect.left+=dx;
            rect.right+=dx;
            rect.top+=dy;
            rect.bottom+=dy;

            m_rectBeforeMinimize=rect;
        }
        MoveWindow(rect);

        CStringA str;

        //���ϽǱ�������
        str=uiCfgFile.GetValue("titletext","text").c_str();
        SetTitleText(CharsetConvertMFC::UTF8ToUTF16(str));

        //IDC_STATIC_GP_CDNAME
        str=m_cfgFile.GetValue("uiword","gpcdname").c_str();
        GetDlgItem(IDC_STATIC_GP_CDNAME)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"gpcdname",GetDlgItem(IDC_STATIC_GP_CDNAME));

        //IDC_STATIC_CDNAME
        str=m_cfgFile.GetValue("uiword","staticcdname").c_str();
        GetDlgItem(IDC_STATIC_CDNAME)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"staticcdname",GetDlgItem(IDC_STATIC_CDNAME));

        //IDC_EDIT_CDNAME
        Layout(uiCfgFile,"editcdname",GetDlgItem(IDC_EDIT_CDNAME));
        ((CEdit *)(GetDlgItem(IDC_EDIT_CDNAME)))->SetLimitText(50);

        //IDC_STATIC_GP_BURNPARAM
        str=m_cfgFile.GetValue("uiword","gpburnparam").c_str();
        GetDlgItem(IDC_STATIC_GP_BURNPARAM)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"gpburnparam",GetDlgItem(IDC_STATIC_GP_BURNPARAM));

        //IDC_STATIC_RECORDER
        str=m_cfgFile.GetValue("uiword","staticrecorder").c_str();
        GetDlgItem(IDC_STATIC_RECORDER)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"staticrecorder",GetDlgItem(IDC_STATIC_RECORDER));

        //IDC_COMBO_RECORDER
        Layout(uiCfgFile,"comborecorder",GetDlgItem(IDC_COMBO_RECORDER));

        //IDC_STATIC_RECORDERINFO
        str=m_cfgFile.GetValue("uiword","staticrecorderinfo").c_str();
        GetDlgItem(IDC_STATIC_RECORDERINFO)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"staticrecorderinfo",GetDlgItem(IDC_STATIC_RECORDERINFO));

        //IDC_EDIT_RECORDERINFO
        Layout(uiCfgFile,"editrecorderinfo",GetDlgItem(IDC_EDIT_RECORDERINFO));

        //IDC_STATIC_BURNTYPE
        str=m_cfgFile.GetValue("uiword","staticburntype").c_str();
        GetDlgItem(IDC_STATIC_BURNTYPE)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"staticburntype",GetDlgItem(IDC_STATIC_BURNTYPE));

        //IDC_RADIO_SINGLE
        str=m_cfgFile.GetValue("uiword","radiosingle").c_str();
        GetDlgItem(IDC_RADIO_SINGLE)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"radiosingle",GetDlgItem(IDC_RADIO_SINGLE));

        //IDC_RADIO_MULTISYN
        str=m_cfgFile.GetValue("uiword","radiomultisyn").c_str();
        GetDlgItem(IDC_RADIO_MULTISYN)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"radiomultisyn",GetDlgItem(IDC_RADIO_MULTISYN));

        //IDC_RADIO_MULTIASYN
        str=m_cfgFile.GetValue("uiword","radiomultiasyn").c_str();
        GetDlgItem(IDC_RADIO_MULTIASYN)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"radiomultiasyn",GetDlgItem(IDC_RADIO_MULTIASYN));

        //IDC_STATIC_SELECTCDROM
        str=m_cfgFile.GetValue("uiword","staticselectcdrom").c_str();
        GetDlgItem(IDC_STATIC_SELECTCDROM)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"staticselectcdrom",GetDlgItem(IDC_STATIC_SELECTCDROM));

        //IDC_LIST_SELECTCDROM
        Layout(uiCfgFile,"listselectcdrom",GetDlgItem(IDC_LIST_SELECTCDROM));

        //IDC_STATIC_GP_BURNPASSWORD
        str=m_cfgFile.GetValue("uiword","gpburnpassword").c_str();
        GetDlgItem(IDC_STATIC_GP_BURNPASSWORD)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"gpburnpassword",GetDlgItem(IDC_STATIC_GP_BURNPASSWORD));

        //IDC_CHECK_BURNPASSWORD
        str=m_cfgFile.GetValue("uiword","checkburnpassword").c_str();
        GetDlgItem(IDC_CHECK_BURNPASSWORD)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"checkburnpassword",GetDlgItem(IDC_CHECK_BURNPASSWORD));

        //IDC_CHECK_CONTENTPASSWORD
        str=m_cfgFile.GetValue("uiword","checkcontentpassword").c_str();
        GetDlgItem(IDC_CHECK_CONTENTPASSWORD)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"checkcontentpassword",GetDlgItem(IDC_CHECK_CONTENTPASSWORD));

        //IDC_STATIC_INPUTPASSWORD
        str=m_cfgFile.GetValue("uiword","staticinputpassword").c_str();
        GetDlgItem(IDC_STATIC_INPUTPASSWORD)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"staticinputpassword",GetDlgItem(IDC_STATIC_INPUTPASSWORD));

        //IDC_EDIT_INPUTPASSWORD
        Layout(uiCfgFile,"editinputpassword",GetDlgItem(IDC_EDIT_INPUTPASSWORD));

        //IDC_STATIC_CONFIRMPASSWORD
        str=m_cfgFile.GetValue("uiword","staticinputpassword").c_str();
        GetDlgItem(IDC_STATIC_CONFIRMPASSWORD)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"staticconfirmpassword",GetDlgItem(IDC_STATIC_CONFIRMPASSWORD));

        //IDC_EDIT_CONFIRMPASSWORD
        Layout(uiCfgFile,"editconfirmpassword",GetDlgItem(IDC_EDIT_CONFIRMPASSWORD));

        //IDC_STATIC_GP_FILEINFO
        str=m_cfgFile.GetValue("uiword","gpfileinfo").c_str();
        GetDlgItem(IDC_STATIC_GP_FILEINFO)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"gpfileinfo",GetDlgItem(IDC_STATIC_GP_FILEINFO));

        //IDC_LIST_FILEINFO
        Layout(uiCfgFile,"listfileinfo",GetDlgItem(IDC_LIST_FILEINFO));

        //IDC_STATIC_GP_BURNSTATE
        str=m_cfgFile.GetValue("uiword","gpburnstate").c_str();
        GetDlgItem(IDC_STATIC_GP_BURNSTATE)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"gpburnstate",GetDlgItem(IDC_STATIC_GP_BURNSTATE));

        //IDC_LIST_BURNSTATE
        Layout(uiCfgFile,"listburnstate",GetDlgItem(IDC_LIST_BURNSTATE));

        //IDC_BUTTON_BURN
        Layout(uiCfgFile,"buttonburn",GetDlgItem(IDC_BUTTON_BURN));

        //IDC_BUTTON_CLOSE
        Layout(uiCfgFile,"buttonclose",GetDlgItem(IDC_BUTTON_CLOSE));

        //IDC_STATIC_JOBLIST
        str=m_cfgFile.GetValue("uiword","gpjoblist").c_str();
        GetDlgItem(IDC_STATIC_GP_JOBLIST)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"gpjoblist",GetDlgItem(IDC_STATIC_GP_JOBLIST));

        //IDC_LIST_JOBLIST
        Layout(uiCfgFile,"listjoblist",GetDlgItem(IDC_LIST_JOBLIST));

        //IDC_BUTTON_UP
        Layout(uiCfgFile,"buttonjobup",GetDlgItem(IDC_BUTTON_UP));

        //IDC_BUTTON_DOWN
        Layout(uiCfgFile,"buttonjobdown",GetDlgItem(IDC_BUTTON_DOWN));

        //IDC_STATIC_GP_BURNCONTENT
        str=m_cfgFile.GetValue("uiword","gpburncontent").c_str();
        GetDlgItem(IDC_STATIC_GP_BURNCONTENT)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(str));
        Layout(uiCfgFile,"gpburncontent",GetDlgItem(IDC_STATIC_GP_BURNCONTENT));

        //IDC_CHECK_L1
        Layout(uiCfgFile,"check1",GetDlgItem(IDC_CHECK_L1));

        //IDC_CHECK_L2
        Layout(uiCfgFile,"check2",GetDlgItem(IDC_CHECK_L2));

        //IDC_CHECK_L3
        Layout(uiCfgFile,"check3",GetDlgItem(IDC_CHECK_L3));

        //IDC_CHECK_L4
        Layout(uiCfgFile,"check4",GetDlgItem(IDC_CHECK_L4));

        //IDC_CHECK_L5
        Layout(uiCfgFile,"check5",GetDlgItem(IDC_CHECK_L5));

        //IDC_CHECK_L6
        Layout(uiCfgFile,"check6",GetDlgItem(IDC_CHECK_L6));

        //IDC_CHECK_L7
        Layout(uiCfgFile,"check7",GetDlgItem(IDC_CHECK_L7));

        //IDC_CHECK_L8
        Layout(uiCfgFile,"check8",GetDlgItem(IDC_CHECK_L8));

        //IDC_CHECK_L9
        Layout(uiCfgFile,"check9",GetDlgItem(IDC_CHECK_L9));

        //IDC_BUTTON_SAVE
        Layout(uiCfgFile,"buttonsave",GetDlgItem(IDC_BUTTON_SAVE));

        //IDC_BUTTON_PAUSE
        Layout(uiCfgFile,"buttonpause",GetDlgItem(IDC_BUTTON_PAUSE));
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::CreateUI] Load UI config file error : %s\r\n",strUICfgFile.c_str()));
    }
}


CBurnControlDlg::BURNTYPESEL CBurnControlDlg::GetCurBurnTypeSel()
{
//     LOG_INFO(("[CBurnControlDlg::GetCurBurnTypeSel] m_curBurnTypeSel: %d\r\n", m_curBurnTypeSel));
    return m_curBurnTypeSel;
}

void CBurnControlDlg::SetCurBurnTypeSel(BURNTYPESEL burnTypeSel)
{
    m_curBurnTypeSel = burnTypeSel;
    LOG_INFO(("[CBurnControlDlg::SetCurBurnTypeSel] m_curBurnTypeSel: %d\r\n", m_curBurnTypeSel));
}

bool CBurnControlDlg::GetCurBurnServerSel(CBurnServerInfo &burnServerInfo)
{
    ZOSMutexLocker locker(&m_mutexBurnServer);

    if (m_vecBurnServerInfoEx.empty())
    {
        LOG_ERROR(("[CBurnControlDlg::GetCurBurnServerSel] No burn server\r\n"));

        return false;
    }

    bool bRet=false;

    if (BURNCATEGORY_RTBURN == m_eBurnCategory ||
        BURNCATEGORY_HM3 == m_eBurnCategory)
    {
        //ʵʱ��¼��������ʹ��BurnServer����ͨ��¼�豸

        size_t nRecorderCount=m_vecBurnServerInfoEx.size();
        for (size_t i=0;i<nRecorderCount;++i)
        {
            if (BURNSERVER_TYPE_NORMAL == m_vecBurnServerInfoEx.at(i).GetBurnServerType())
            {
                burnServerInfo=m_vecBurnServerInfoEx.at(i);

                std::vector<CCDROMDriverInfo> vecCDROMInfo;
                BURNTYPESEL eCurBurnTypeSel=GetCurBurnTypeSel();
                if (BURNTYPE_SINGLE == eCurBurnTypeSel)
                {
                    vecCDROMInfo=m_vecBurnServerInfoEx.at(i).GetVecCDROMInfo0();

                    bRet=true;
                }
                else if (BURNTYPE_MULTISYN == eCurBurnTypeSel)
                {
                    vecCDROMInfo=m_vecBurnServerInfoEx.at(i).GetVecCDROMInfo1();

                    bRet=true;
                }
                else if (BURNTYPE_MULTIASYN == eCurBurnTypeSel)
                {
                    vecCDROMInfo=m_vecBurnServerInfoEx.at(i).GetVecCDROMInfo2();

                    bRet=true;
                }
                else
                {
                    LOG_ERROR(("[CBurnControlDlg::GetCurBurnServerSel] Unknown burn category\r\n"));

                    bRet=false;
                }

                burnServerInfo.SetVecCDROMInfo(vecCDROMInfo);

                break;
            }
        }
    }
    else
    {
        //�º��¼
        CComboBox *pcbBurnServer=(CComboBox *)GetDlgItem(IDC_COMBO_RECORDER);
        if (pcbBurnServer != NULL &&
            pcbBurnServer->GetSafeHwnd() != NULL)
        {
            int nIndexSel=pcbBurnServer->GetCurSel();
            {
                if (CB_ERR != nIndexSel)
                {
                    if (m_vecBurnServerInfoEx.size() > static_cast<size_t>(nIndexSel))
                    {
                        burnServerInfo=m_vecBurnServerInfoEx.at(nIndexSel);

                        std::vector<CCDROMDriverInfo> vecCDROMInfo;
                        BURNTYPESEL eCurBurnTypeSel=GetCurBurnTypeSel();
                        if (BURNTYPE_SINGLE == eCurBurnTypeSel)
                        {
                            vecCDROMInfo=m_vecBurnServerInfoEx.at(nIndexSel).GetVecCDROMInfo0();
                        }
                        else if (BURNTYPE_MULTISYN == eCurBurnTypeSel)
                        {
                            vecCDROMInfo=m_vecBurnServerInfoEx.at(nIndexSel).GetVecCDROMInfo1();
                        }
                        else if (BURNTYPE_MULTIASYN == eCurBurnTypeSel)
                        {
                            vecCDROMInfo=m_vecBurnServerInfoEx.at(nIndexSel).GetVecCDROMInfo2();
                        }
                        else
                        {
                            LOG_ERROR(("[CBurnControlDlg::GetCurBurnServerSel] Unknown burn type\r\n"));
                            assert(false);
                        }

                        burnServerInfo.SetVecCDROMInfo(vecCDROMInfo);

                        bRet=true;
                    }
                    else
                    {
                        LOG_ERROR(("[CBurnControlDlg::GetCurBurnServerSel] BurnServer count error\r\n"));
                        assert(false);
                    }
                }
            }
        }
    }

    return bRet;
}

std::vector<CCDROMDriverInfo> CBurnControlDlg::GetCurCDROMDriverInfoVec()
{
    std::vector<CCDROMDriverInfo> vecCDROMDriverInfo;

    BURNTYPESEL eCurBurnTypeSel=GetCurBurnTypeSel();

    CBurnServerInfo curBurnServerInfo;
    if (GetCurBurnServerSel(curBurnServerInfo) &&
        eCurBurnTypeSel != BURNTYPE_UNKNOWNSEL)
    {
        int nBurnServerCount=m_vecBurnServerInfoEx.size();
        for (int i=0;i<nBurnServerCount;++i)
        {
            if (m_vecBurnServerInfoEx.at(i).GetIP() == curBurnServerInfo.GetIP() &&
                m_vecBurnServerInfoEx.at(i).GetPort() == curBurnServerInfo.GetPort() &&
                m_vecBurnServerInfoEx.at(i).GetBurnServerType() == curBurnServerInfo.GetBurnServerType())
            {
                CStringA strBurnServerName=
                    CStringA(CharsetConvertMFC::UTF8ToUTF16(m_vecBurnServerInfoEx.at(i).GetBurnServerName().c_str()));
                LOG_INFO(("[CBurnControlDlg::GetCurCDROMDriverInfoVec] Get current cdrom driver success\r\n",
                    strBurnServerName.GetBuffer()));

                if (BURNTYPE_SINGLE == eCurBurnTypeSel)
                {
                    vecCDROMDriverInfo=m_vecBurnServerInfoEx.at(i).GetVecCDROMInfo0();
                    break;
                }
                else if (BURNTYPE_MULTISYN == eCurBurnTypeSel)
                {
                    vecCDROMDriverInfo=m_vecBurnServerInfoEx.at(i).GetVecCDROMInfo1();
                    break;
                }
                else if (BURNTYPE_MULTIASYN == eCurBurnTypeSel)
                {
                    vecCDROMDriverInfo=m_vecBurnServerInfoEx.at(i).GetVecCDROMInfo2();
                    break;
                }
            }
        }
    }

    return vecCDROMDriverInfo;
}

bool CBurnControlDlg::GetDevInfo(std::string strIP,std::string strPort,
                                 std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo)
{
    vecCDROMDriverInfo.clear();

    bool bRet=false;

    std::string strProtocolContent;
    strProtocolContent+="<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n";
    strProtocolContent+="<burnControl>\r\n";
    strProtocolContent+="<commandType>getCDROMInfo</commandType>\r\n";
    strProtocolContent+="</burnControl>\r\n";

    std::string strResponse;
    strResponse=CInterfaceProtocol::InterfaceCommunicate(strIP,atoi(strPort.c_str()),
        strProtocolContent,BURNCONTROL_TAG_NAME,30);

    int nTryCount=1;
    while (true)
    {
        if (strResponse.length() > 0)
        {
            bRet=true;
            CInterfaceProtocol::AnalyzeProtocolCDROMDriverInfo(strResponse,vecCDROMDriverInfo);
            break;
        }
        else
        {
            if (nTryCount > 3)
            {
                break;
            }
            else
            {
                strResponse=CInterfaceProtocol::InterfaceCommunicate(strIP,atoi(strPort.c_str()),
                    strProtocolContent,BURNCONTROL_TAG_NAME,10);

                ++nTryCount;
            }
        }
    }

    if (!bRet)
    {
        LOG_ERROR(("[CBurnControlDlg::GetCDROMDriverInfo] Get CDROM driver information failed\r\n"));
        return false;
    }

    return bRet;
}

void CBurnControlDlg::UpdateCDROMDriverInfo(const std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo)
{
    if (vecCDROMDriverInfo.empty())
    {
        LOG_ERROR(("[CBurnControlDlg::UpdateCDROMDriverInfo] No CDROM driver\r\n"));
        assert(false);
        return;
    }

    int nIndexSel=CB_ERR;//��ǰѡ��Ŀ�¼������
    if (BURNCATEGORY_RTBURN == m_eBurnCategory ||
        BURNCATEGORY_HM3 == m_eBurnCategory)
    {
        nIndexSel=0;
    }
    else
    {
        CComboBox *pcbBurnServer=(CComboBox *)GetDlgItem(IDC_COMBO_RECORDER);
        if (pcbBurnServer != NULL &&
            pcbBurnServer->GetSafeHwnd() != NULL)
        {
            nIndexSel=pcbBurnServer->GetCurSel();
        }
    }

    if (CB_ERR != nIndexSel &&
        m_vecBurnServerInfoEx.size() > static_cast<size_t>(nIndexSel))
    {
        BURNTYPESEL burnType=GetCurBurnTypeSel();
        if (BURNTYPE_SINGLE == burnType)
        {
            m_vecBurnServerInfoEx.at(nIndexSel).SetVecCDROMInfo0(vecCDROMDriverInfo);
        }
        else if (BURNTYPE_MULTISYN == burnType)
        {
            m_vecBurnServerInfoEx.at(nIndexSel).SetVecCDROMInfo1(vecCDROMDriverInfo);
        }
        else if (BURNTYPE_MULTIASYN == burnType)
        {
            m_vecBurnServerInfoEx.at(nIndexSel).SetVecCDROMInfo2(vecCDROMDriverInfo);
        }
        else
        {
            LOG_ERROR(("[CBurnControlDlg::UpdateCDROMDriverInfo] Unknown burn type\r\n"));
            assert(false);
        }
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::UpdateCDROMDriverInfo] Current BurnServer selection error\r\n"));
        assert(false);
    }

    /*std::vector<CCDROMDriverInfo>::const_iterator iter;
    for (iter=vecCDROMDriverInfo.begin();iter!=vecCDROMDriverInfo.end();++iter)
    {
    CStringA strCDROMDriverName=
    CStringA(CharsetConvertMFC::UTF8ToUTF16(iter->GetID().c_str()));
    if (iter->GetIsSelected() == "2")
    {
    LOG_INFO(("[CBurnControlDlg::GetCurCDROMDriverInfoVec] CDROMDriver %s is not selected\r\n",
    strCDROMDriverName.GetBuffer()));
    }
    else if (iter->GetIsSelected() == "1")
    {
    LOG_INFO(("[CBurnControlDlg::GetCurCDROMDriverInfoVec] CDROMDriver %s is selected\r\n",
    strCDROMDriverName.GetBuffer()));
    } 
    else
    {
    assert(false);
    }
    }*/
}

std::vector<CLocationInfo> CBurnControlDlg::GetVecLocationInfo()
{
    std::vector<CLocationInfo> vecLocationInfo;

    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_RTBURN:
    case BURNCATEGORY_HM3:
        {
            vecLocationInfo=m_RTBurnInfo.GetVecVideoLocationInfo();
        }
        break;

    case BURNCATEGORY_COS:
    case BURNCATEGORY_SINGLE:
        if (1 == m_normalJob.GetVecNormalBurnJobInfo().size())
        {
            vecLocationInfo=m_normalJob.GetVecNormalBurnJobInfo().at(0).GetVecLocationInfo();
        }
        else
        {
            LOG_ERROR(("[CBurnControlDlg::GetVecLocationInfo] Job count error\r\n"));
            assert(false);
        }
        break;

    case BURNCATEGORY_MULTI:
        {

            if (!m_normalJob.GetVecNormalBurnJobInfo().empty())
            {
                vecLocationInfo=m_normalJob.GetVecNormalBurnJobInfo().at(0).GetVecLocationInfo();
            }
            else
            {
                LOG_ERROR(("[CBurnControlDlg::GetVecLocationInfo] No job\r\n"));
                assert(false);
            }
        }
        break;

    case BURNCATEGORY_UNKNOWN:
    default:
        {
            LOG_ERROR(("[CBurnControlDlg::GetVecLocationInfo] Burn category error\r\n"));
            assert(false);
        }
        break;
    }

    return vecLocationInfo;
}

std::string CBurnControlDlg::GetCurJobID()
{
    std::string strJobID;

    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_RTBURN:
    case BURNCATEGORY_HM3:
        strJobID=m_RTBurnInfo.GetJobID();
        break;

    case BURNCATEGORY_COS:
    case BURNCATEGORY_SINGLE:
        if (1 == m_normalJob.GetVecNormalBurnJobInfo().size())
        {
            strJobID=m_normalJob.GetVecNormalBurnJobInfo().at(0).GetJobID();
        } 
        else
        {
            LOG_ERROR(("[CBurnControlDlg::GetCurJobID] Job count error\r\n"));
            assert(false);
        }
        break;

    case BURNCATEGORY_MULTI:
        {
            size_t nJobCount=m_normalJob.GetVecNormalBurnJobInfo().size();
            if (nJobCount > m_nJobIndex)
            {
                strJobID=m_normalJob.GetVecNormalBurnJobInfo().at(m_nJobIndex).GetJobID();
            }
        }
        break;

    case BURNCATEGORY_UNKNOWN:
    default:
        LOG_ERROR(("[CBurnControlDlg::GetCurJobID] UNKNOWNCATEGORY\r\n"));
        break;
    }

    return strJobID;
}

void CBurnControlDlg::GetBurnServerAddress(std::string &strIP,int &nPort)
{
    std::string strProtocolContentToLower;
    std::string strBurnServerPort;
    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_COS:
    case BURNCATEGORY_SINGLE:
        if (1 == m_normalJob.GetVecNormalBurnJobInfo().size())
        {
            strIP=m_normalJob.GetVecNormalBurnJobInfo().at(0).GetBurnServerInfo().GetIP();
            strBurnServerPort=m_normalJob.GetVecNormalBurnJobInfo().at(0).GetBurnServerInfo().GetPort();
        } 
        else
        {
            LOG_ERROR(("[CBurnControlDlg::GetBurnServerAddress] Job count error\r\n"));
            assert(false);
        }
        break;

    case BURNCATEGORY_MULTI:
        {
            //��������Ҫ���⴦��ÿ�η���һ�����񣬵���һ��������ɺ󣬲ŷ����µ�����
            if (!m_normalJob.GetVecNormalBurnJobInfo().empty())
            {
                strIP=m_normalJob.GetVecNormalBurnJobInfo().at(m_nJobIndex).GetBurnServerInfo().GetIP();
                strBurnServerPort=m_normalJob.GetVecNormalBurnJobInfo().at(m_nJobIndex).GetBurnServerInfo().GetPort();
            }
            else
            {
                LOG_ERROR(("[CBurnControlDlg::GetBurnServerAddress] No job\r\n"));
                assert(false);
            }
        }
        break;

    case BURNCATEGORY_RTBURN:
    case BURNCATEGORY_HM3:
        strIP=m_RTBurnInfo.GetBurnServerInfo().GetIP();
        strBurnServerPort=m_RTBurnInfo.GetBurnServerInfo().GetPort();
        break;

    default:
        LOG_WARNING(("[CBurnControlDlg::GetBurnServerAddress] Unknown burn category\r\n"));
    }

    if (strBurnServerPort != "")
    {
        nPort=atoi(strBurnServerPort.c_str());
    }
}

void CBurnControlDlg::HideJobList()
{
    if (GetDlgItem(IDC_STATIC_GP_JOBLIST) != NULL &&
        GetDlgItem(IDC_STATIC_GP_JOBLIST)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_STATIC_GP_JOBLIST)->ShowWindow(SW_HIDE);
    }

    if (GetDlgItem(IDC_LIST_JOBLIST) != NULL &&
        GetDlgItem(IDC_LIST_JOBLIST)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_LIST_JOBLIST)->ShowWindow(SW_HIDE);
    }

    if (GetDlgItem(IDC_BUTTON_UP) != NULL &&
        GetDlgItem(IDC_BUTTON_UP)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_BUTTON_UP)->ShowWindow(SW_HIDE);
    }

    if (GetDlgItem(IDC_BUTTON_DOWN) != NULL &&
        GetDlgItem(IDC_BUTTON_DOWN)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_BUTTON_DOWN)->ShowWindow(SW_HIDE);
    }
}

void CBurnControlDlg::HideCDName()
{
    CWnd *pWnd=NULL;

    pWnd=GetDlgItem(IDC_STATIC_GP_CDNAME);
    if (pWnd != NULL
        && pWnd->GetSafeHwnd() != NULL)
    {
        pWnd->ShowWindow(SW_HIDE);
    }

    pWnd=GetDlgItem(IDC_STATIC_CDNAME);
    if (pWnd != NULL
        && pWnd->GetSafeHwnd() != NULL)
    {
        pWnd->ShowWindow(SW_HIDE);
    }

    pWnd=GetDlgItem(IDC_EDIT_CDNAME);
    if (pWnd != NULL
        && pWnd->GetSafeHwnd() != NULL)
    {
        pWnd->ShowWindow(SW_HIDE);
    }
}

void CBurnControlDlg::HideBurnContent()
{
    if (GetDlgItem(IDC_STATIC_GP_BURNCONTENT) != NULL &&
        GetDlgItem(IDC_STATIC_GP_BURNCONTENT)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_STATIC_GP_BURNCONTENT)->ShowWindow(SW_HIDE);
    }

    if (GetDlgItem(IDC_CHECK_L1) != NULL &&
        GetDlgItem(IDC_CHECK_L1)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_CHECK_L1)->ShowWindow(SW_HIDE);
    }

    if (GetDlgItem(IDC_CHECK_L2) != NULL &&
        GetDlgItem(IDC_CHECK_L2)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_CHECK_L2)->ShowWindow(SW_HIDE);
    }

    if (GetDlgItem(IDC_CHECK_L3) != NULL &&
        GetDlgItem(IDC_CHECK_L3)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_CHECK_L3)->ShowWindow(SW_HIDE);
    }

    if (GetDlgItem(IDC_CHECK_L4) != NULL &&
        GetDlgItem(IDC_CHECK_L4)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_CHECK_L4)->ShowWindow(SW_HIDE);
    }

    if (GetDlgItem(IDC_CHECK_L5) != NULL &&
        GetDlgItem(IDC_CHECK_L5)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_CHECK_L5)->ShowWindow(SW_HIDE);
    }

    if (GetDlgItem(IDC_CHECK_L6) != NULL &&
        GetDlgItem(IDC_CHECK_L6)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_CHECK_L6)->ShowWindow(SW_HIDE);
    }

    if (GetDlgItem(IDC_CHECK_L7) != NULL &&
        GetDlgItem(IDC_CHECK_L7)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_CHECK_L7)->ShowWindow(SW_HIDE);
    }

    if (GetDlgItem(IDC_CHECK_L8) != NULL &&
        GetDlgItem(IDC_CHECK_L8)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_CHECK_L8)->ShowWindow(SW_HIDE);
    }

    if (GetDlgItem(IDC_CHECK_L9) != NULL &&
        GetDlgItem(IDC_CHECK_L9)->GetSafeHwnd() != NULL)
    {
        GetDlgItem(IDC_CHECK_L9)->ShowWindow(SW_HIDE);
    }
}

void CBurnControlDlg::HideRecorder()
{
    CWnd *pWnd=NULL;

    pWnd=GetDlgItem(IDC_STATIC_RECORDER);
    if (pWnd != NULL &&
        pWnd->GetSafeHwnd() != NULL)
    {
        pWnd->ShowWindow(SW_HIDE);
    }

    pWnd=GetDlgItem(IDC_COMBO_RECORDER);
    if (pWnd != NULL &&
        pWnd->GetSafeHwnd() != NULL)
    {
        pWnd->ShowWindow(SW_HIDE);
    }

    pWnd=GetDlgItem(IDC_STATIC_RECORDERINFO);
    if (pWnd != NULL &&
        pWnd->GetSafeHwnd() != NULL)
    {
        pWnd->ShowWindow(SW_HIDE);
    }

    pWnd=GetDlgItem(IDC_EDIT_RECORDERINFO);
    if (pWnd != NULL &&
        pWnd->GetSafeHwnd() != NULL)
    {
        pWnd->ShowWindow(SW_HIDE);
    }
}

void CBurnControlDlg::HideFileInfo()
{
    CWnd *pWnd=NULL;

    pWnd=GetDlgItem(IDC_STATIC_GP_FILEINFO);
    if (pWnd != NULL &&
        pWnd->GetSafeHwnd()!=NULL)
    {
        pWnd->ShowWindow(SW_HIDE);
    }

    pWnd=GetDlgItem(IDC_STATIC_GP_FILEINFO);
    if (pWnd != NULL &&
        pWnd->GetSafeHwnd()!=NULL)
    {
        pWnd->ShowWindow(SW_HIDE);
    }
}

void CBurnControlDlg::DisplayBurnContentUI()
{
    //�������п�¼���ݸ�ѡ��
    for (UINT nLocationIndex=IDC_CHECK_L1;nLocationIndex<IDC_CHECK_L1+LOCATION_MAX_COUNT;++nLocationIndex)
    {
        CWnd *p=GetDlgItem(nLocationIndex);
        if (p != NULL)
        {
            if (p->GetSafeHwnd() != NULL)
            {
                ((CButton*)p)->SetCheck(BST_UNCHECKED);
                p->ShowWindow(SW_HIDE);
            }
        }
    }

    std::vector<CLocationInfo> vecLocationInfo=GetVecLocationInfo();
    size_t nLocationCount=vecLocationInfo.size();
    size_t nLocationIndex=0;
    int nLocationSelectedNum = 0;
    for (nLocationIndex=0;nLocationIndex<nLocationCount;++nLocationIndex)
    {
        if ("" == vecLocationInfo.at(nLocationIndex).GetDescription())
        {
            LOG_ERROR(("[CBurnControlDlg::DisplayBurnContentUI] Location description is empty\r\n"));

            assert(false);

            continue;
        }

        CStringW strLocationDescription;
        strLocationDescription=CharsetConvertMFC::UTF8ToUTF16(vecLocationInfo.at(nLocationIndex).GetDescription().c_str());
        BOOL bIsSelected=( BURN_YES == vecLocationInfo.at(nLocationIndex).GetBurnOrNot() );

        if (bIsSelected == TRUE)
        {
            nLocationSelectedNum++;
        }
        if (nLocationSelectedNum > LOCATIONMAXNUM)
        {
            bIsSelected = FALSE;
            LOG_WARNING(("[CBurnControlDlg::DisplayBurnContentUI] Selected location num more than %d, nLocationIndex: %d, strLocationDescription: %s \r\n"
                , LOCATIONMAXNUM, nLocationIndex, vecLocationInfo.at(nLocationIndex).GetDescription().c_str()));
        }
        int nCheckButtonID=IDC_CHECK_L1+nLocationIndex;
        CWnd *p=GetDlgItem(nCheckButtonID);
        if (p != NULL)
        {
            if (p->GetSafeHwnd() != NULL)
            {
                p->ShowWindow(SW_SHOW);
                p->SetForegroundWindow();
                p->SetWindowText(strLocationDescription);
                ((CButton *)p)->SetCheck(bIsSelected);
                if (nLocationSelectedNum > LOCATIONMAXNUM)
                {
                    p->EnableWindow(FALSE);
                    LOG_INFO(("[CBurnControlDlg::DisplayBurnContentUI] Selected location num is %d, UnableWindow! nLocationIndex: %d, strLocationDescription: %s \r\n"
                        , LOCATIONMAXNUM, nLocationIndex, vecLocationInfo.at(nLocationIndex).GetDescription().c_str()));
                }
            }
        }
    }

    //COS�º��¼��ͥ���¼����ѡ��
    if (BURNCATEGORY_COS == m_eBurnCategory)
    {
        if (1 == m_normalJob.GetVecNormalBurnJobInfo().size())
        {
            CNormalBurnJobInfoEx info=m_normalJob.GetVecNormalBurnJobInfo().at(0);
            if ((info.GetNoteFileRemoteIP() != "") &&
                (info.GetNoteFileRemotePort() != "") &&
                (info.GetNoteRelativePath() != "") &&
                (info.GetNoteFileName() != ""))
            {
                UINT nCheckButtonIDForNote=IDC_CHECK_L1+nLocationIndex;
                CStringW strLocationDescription=CharsetConvertMFC::UTF8ToUTF16(
                    m_cfgFile.GetValue("uiword","notecheckboxname").c_str());

                CWnd *p=GetDlgItem(nCheckButtonIDForNote);
                if (p != NULL)
                {
                    if (p->GetSafeHwnd() != NULL)
                    {
                        p->ShowWindow(SW_SHOW);
                        p->SetForegroundWindow();
                        p->SetWindowText(strLocationDescription);
                        ((CButton *)p)->SetCheck(TRUE);
                    }
                }
            }
        }
        else
        {
            LOG_ERROR(("[CBurnControlDlg::DisplayBurnContentUI] Job count is wrong\r\n"));
            assert(false);
        }
    }
}

void CBurnControlDlg::DisplayBurnServer(const std::vector<CBurnServerInfoEx> &vecBurnServerInfoEx)
{
    CComboBox *pcbBurnServer=(CComboBox *)GetDlgItem(IDC_COMBO_RECORDER);
    if (pcbBurnServer != NULL &&
        pcbBurnServer->GetSafeHwnd() != NULL &&
        !vecBurnServerInfoEx.empty())
    {
        while (pcbBurnServer->GetCount() > 0)
        {
            pcbBurnServer->DeleteString(0);
        }

        int nBurnServerIndex=0,nComboCurSel=-1,nBurnServerCurSel=0;
        std::vector<CBurnServerInfoEx>::const_iterator iter;
        for (iter=vecBurnServerInfoEx.begin();iter!=vecBurnServerInfoEx.end();++iter,++nBurnServerIndex)
        {
            std::string strBurnServerName=iter->GetBurnServerName();

            //�����ϸ�˳�����
            pcbBurnServer->InsertString(-1,CharsetConvertMFC::UTF8ToUTF16(strBurnServerName.c_str()));

            //Ĭ����ʾ��һ�������Ӧ�Ŀ�¼������
            if (!m_normalJob.GetVecNormalBurnJobInfo().empty())
            {
                const CNormalBurnJobInfoEx &normalBurnJobInfoEx=m_normalJob.GetVecNormalBurnJobInfo().at(0);
                if (iter->GetIP() == normalBurnJobInfoEx.GetBurnServerInfo().GetIP() &&
                    iter->GetPort() == normalBurnJobInfoEx.GetBurnServerInfo().GetPort())
                {
                    if (nComboCurSel == -1)
                    {
                        nComboCurSel = nBurnServerIndex;
                    }
                }
                if (nComboCurSel == -1 && iter->GetIP() == NetUtil::GetHostIP()
                    && iter->GetBurnServerType() == "1")
                {//û��BurnServer����Ϣ������ʾ������ͨ��¼��
                    LOG_INFO(("[CBurnControlDlg::DisplayBurnServer] Cannot find job Burnserver info, display local Burnserver info!\r\n"));
                    nComboCurSel = nBurnServerIndex;
                }
                if (nComboCurSel == -1)
                {
                    LOG_INFO(("[CBurnControlDlg::DisplayBurnServer] Cannot find job Burnserver info, and no local Burnserver info!\r\n"));
                }
            }
            else
            {
                LOG_ERROR(("[CBurnControlDlg::DisplayBurnServer] No job\r\n"));
                assert(false);
            }
        }

        if (pcbBurnServer->GetCount() > 0)
        {
            if (nComboCurSel < 0)
            {
                nComboCurSel=0;
            }

            pcbBurnServer->SetCurSel(nComboCurSel);

            UpdateBurnServerUIInfo();
        }
    }
}

void CBurnControlDlg::SetInitialBurnTypeUI(BURNTYPESEL eBurnType)
{
    SetCurBurnTypeSel(eBurnType);
    CButton *p1=((CButton *)GetDlgItem(IDC_RADIO_SINGLE));
    CButton *p2=((CButton *)GetDlgItem(IDC_RADIO_MULTISYN));
    CButton *p3=((CButton *)GetDlgItem(IDC_RADIO_MULTIASYN));

    if (NULL == p1 ||
        NULL == p2 ||
        NULL == p3 ||
        NULL == p1->GetSafeHwnd() ||
        NULL == p2->GetSafeHwnd() ||
        NULL == p3->GetSafeHwnd())
    {
        return;
    }

    if (BURNTYPE_MULTISYN == eBurnType)
    {
        p1->SetCheck(BST_UNCHECKED);
        p2->SetCheck(BST_CHECKED);
        p3->SetCheck(BST_UNCHECKED);
    }
    else if (eBurnType == BURNTYPE_MULTIASYN)
    {
        p1->SetCheck(BST_UNCHECKED);
        p2->SetCheck(BST_UNCHECKED);
        p3->SetCheck(BST_CHECKED);
    }
    else
    {
        p1->SetCheck(BST_CHECKED);
        p2->SetCheck(BST_UNCHECKED);
        p3->SetCheck(BST_UNCHECKED);
    }
}

std::string CBurnControlDlg::GetBurnServerType()
{
    std::string str=BURNSERVER_TYPE_NORMAL;

    CBurnServerInfo burnServerInfo;
    if (GetCurBurnServerSel(burnServerInfo))
    {
        str=burnServerInfo.GetBurnServerType();
    }

    return str;
}

void CBurnControlDlg::DisplayBurnTypeUI(BURNTYPESEL eBurnType)
{
    LOG_INFO(("[CBurnControlDlg::DisplayBurnTypeUI] Enter~~~~~ eBurnType : %d\r\n",eBurnType));

    CButton *p1=((CButton *)GetDlgItem(IDC_RADIO_SINGLE));
    CButton *p2=((CButton *)GetDlgItem(IDC_RADIO_MULTISYN));
    CButton *p3=((CButton *)GetDlgItem(IDC_RADIO_MULTIASYN));

    size_t nDevNum=GetCurCDROMDriverInfoVec().size();

    LOG_INFO(("[CBurnControlDlg::DisplayBurnTypeUI] Dev num : %d\r\n",nDevNum));

    if (nDevNum <= 0 &&
        BURNSERVER_TYPE_NORMAL == GetBurnServerType())
    {
        //��ʾû�п��õĿ�¼������
        DESKTOP_ALERT_PARAM param;
        param.strFlag=STATE_BURN_ERR;
        param.strDescription=m_cfgFile.GetValue("info","noburnserver");
        param.bAutoClose=TRUE;
        param.nAutoCloseTime=30;
        ShowDesktopAlert(param);

        return;
    }

    BOOL bEnable=GetDlgItem(IDC_LIST_SELECTCDROM)->IsWindowEnabled();//��ʶUI�Ƿ��ڲ��ɲ���״̬
 
    if (1 == nDevNum)
    {
        if (p1 != NULL)
        {
            if (p1->GetSafeHwnd() != NULL)
            {
                p1->SetCheck(BST_CHECKED);

                p1->EnableWindow(bEnable);

                SetCurBurnTypeSel(BURNTYPE_SINGLE);
            }
        }

        if (p2 != NULL)
        {
            if (p2->GetSafeHwnd() != NULL)
            {
                p2->SetCheck(BST_UNCHECKED);

                if (p2->IsWindowEnabled())
                {
                    p2->EnableWindow(FALSE);
                }
            }
        }

        if (p3 != NULL)
        {
            if (p3->GetSafeHwnd() != NULL)
            {
                p3->SetCheck(BST_UNCHECKED);
            }

            if (p3->IsWindowEnabled())
            {
                p3->EnableWindow(FALSE);
            }
        }
    }
    else
    {
        //
        if (BURNTYPE_MULTISYN == eBurnType)
        {
            if (p1 != NULL)
            {
                if (p1->GetSafeHwnd() != NULL)
                {
                    p1->SetCheck(BST_UNCHECKED);

                    p1->EnableWindow(bEnable);
                }
            }

            if (p2 != NULL)
            {
                if (p2->GetSafeHwnd() != NULL)
                {
                    p2->SetCheck(BST_CHECKED);

                    p2->EnableWindow(bEnable);
                }
            }

            if (p3 != NULL)
            {
                if (p3->GetSafeHwnd() != NULL)
                {
                    p3->SetCheck(BST_UNCHECKED);

                    p3->EnableWindow(bEnable);
                }
            }
        }
        else if (eBurnType == BURNTYPE_MULTIASYN)
        {
            if (p1 != NULL)
            {
                if (p1->GetSafeHwnd() != NULL)
                {
                    p1->SetCheck(BST_UNCHECKED);

                    p1->EnableWindow(bEnable);
                }
            }

            if (p2 != NULL)
            {
                if (p2->GetSafeHwnd() != NULL)
                {
                    p2->SetCheck(BST_UNCHECKED);

                    p2->EnableWindow(bEnable);
                }
            }

            if (p3 != NULL)
            {
                if (p3->GetSafeHwnd() != NULL)
                {
                    p3->SetCheck(BST_CHECKED);

                    p3->EnableWindow(bEnable);
                }
            }
        }
        else
        {
            if (p1 != NULL)
            {
                if (p1->GetSafeHwnd() != NULL)
                {
                    p1->SetCheck(BST_CHECKED);

                    p1->EnableWindow(bEnable);
                }
            }

            if (p2 != NULL)
            {
                if (p2->GetSafeHwnd() != NULL)
                {
                    p2->SetCheck(BST_UNCHECKED);

                    p2->EnableWindow(bEnable);
                }
            }

            if (p3 != NULL)
            {
                if (p3->GetSafeHwnd() != NULL)
                {
                    p3->SetCheck(BST_UNCHECKED);

                    p3->EnableWindow(bEnable);
                }
            }

        }

    }
}

void CBurnControlDlg::DisplayCDROMUI(int nFlag)
{
    CListCtrl *pListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_SELECTCDROM));
    if (pListCtrl != NULL &&
        pListCtrl->GetSafeHwnd() != NULL)
    {
        std::vector<CCDROMDriverInfo> vecCDROMInfo=GetCurCDROMDriverInfoVec();
        if (!vecCDROMInfo.empty())
        {
            if (0 == nFlag)//���������б�
            {
                pListCtrl->SetExtendedStyle(pListCtrl->GetExtendedStyle() | LVS_EX_CHECKBOXES);

                pListCtrl->DeleteAllItems();

                while (pListCtrl->GetHeaderCtrl()->GetItemCount() > 0)
                {
                    pListCtrl->DeleteColumn(0);
                }

                CRect rect;
                pListCtrl->GetClientRect(&rect);
                pListCtrl->InsertColumn(0,L"",LVCFMT_LEFT,rect.Width()*3/4);
            }

            std::vector<CCDROMDriverInfo>::const_iterator iter;
            int nIndex=0;
            for (iter=vecCDROMInfo.begin();iter!=vecCDROMInfo.end();++iter,++nIndex)
            {
                CStringW strDevName=CharsetConvertMFC::UTF8ToUTF16(
                    m_cfgFile.GetValue("uiword","cdromdrivername").c_str())+
                    CStringW(L"")+CharsetConvertMFC::UTF8ToUTF16(iter->GetID().c_str());

                CStringW strDevDescription=CharsetConvertMFC::UTF8ToUTF16(iter->GetDescription().c_str());

                if (0 == nFlag)//���²��������Ϣ
                {
                    if ( (iter->GetDescription() != "") ||
                        ( (DEV_STATE_BUSY == iter->GetStateFlag()) && (iter->GetJobID() != GetCurJobID()) ) )
                    {
                        strDevName+=L"(";

                        if (strDevDescription != L"")
                        {
                            strDevName+=strDevDescription;

                            strDevName+=L",";
                        }

                        //��ʼʵʱ��¼���ܳ����������������¼�����Ѿ����͸�BurnServer��Ȼ���ȡ������Ϣ���߳�
                        //�ſ�ʼ��BurnServer��ȡ������Ϣ����ʱ��¼�������У����Ƿ��صĹ�����Ϣ��ʾ�������ڱ�ʹ�ã������Ǳ���ǰ����ʹ��
                        if ((iter->GetJobID() != GetCurJobID()) &&
                            (iter->GetJobID() != ""))
                        {
                            LOG_INFO(("[CBurnControlDlg::DisplayCDROMUI] Dev job id : %s,cur job id : %s\r\n",
                                iter->GetJobID().c_str(),GetCurJobID().c_str()));

                            strDevName+=
                                CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("uiword","devbusy").c_str());

                        }

                        strDevName+=L")";
                    }
                    else if (DEV_STATE_UNAVAILABLE == iter->GetStateFlag())
                    {
                        strDevName+=L"(";

                        CStringW strCDROMDriverDescription=
                            CharsetConvertMFC::UTF8ToUTF16(iter->GetDescription().c_str());

                        if (strCDROMDriverDescription != L"")
                        {
                            strDevName+=strCDROMDriverDescription;

                            strDevName+=L",";
                        }

                        strDevName+=CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("uiword","devunavailable").c_str());

                        strDevName+=L")";
                    }

                    //����˳���������������Ԫ��˳���ϸ�һ��
                    pListCtrl->InsertItem(nIndex,strDevName);
                }

                if (DEV_SELECTED_YES == iter->GetIsSelected())
                {
                    LOG_INFO(("[CBurnControlDlg::DisplayCDROMUI] Dev %d is selected,%d\r\n",nIndex,__LINE__));

                    pListCtrl->SetCheck(nIndex,TRUE);
                }
                else
                {
                    LOG_INFO(("[CBurnControlDlg::DisplayCDROMUI] Dev %d is not selected\r\n",nIndex,__LINE__));

                    pListCtrl->SetCheck(nIndex,FALSE);
                }

                LOG_INFO(("[CBurnControlDlg::DisplayCDROMUI] Before DisplayBurnTypeUI,%d\r\n",__LINE__));
                DisplayBurnTypeUI(GetCurBurnTypeSel());
                LOG_INFO(("[CBurnControlDlg::DisplayCDROMUI] After DisplayBurnTypeUI,%d\r\n",__LINE__));
            }

        }//if (!vecCDROMInfo.empty())
        else
        {
            //�����Ƿ���ͨ��¼������û�й�����Ϣ
            LOG_WARNING(("[CBurnControlDlg::DisplayCDROMUI] No CDROM driver\r\n"));
        }
    }
}

void CBurnControlDlg::DisplayPasswordUI(BOOL bBurnPassword,BOOL bContentPassword)
{
    ((CButton *)GetDlgItem(IDC_CHECK_BURNPASSWORD))->EnableWindow(TRUE);
    ((CButton *)GetDlgItem(IDC_CHECK_BURNPASSWORD))->SetCheck(bBurnPassword ? BST_CHECKED : BST_UNCHECKED);

    ((CButton *)GetDlgItem(IDC_CHECK_CONTENTPASSWORD))->EnableWindow(bBurnPassword ? TRUE : FALSE);
    ((CButton *)GetDlgItem(IDC_CHECK_CONTENTPASSWORD))->SetCheck( (bBurnPassword && bContentPassword) ? BST_CHECKED : BST_UNCHECKED);

    ((CEdit *)GetDlgItem(IDC_EDIT_INPUTPASSWORD))->EnableWindow(bBurnPassword ? TRUE : FALSE);
    ((CEdit *)GetDlgItem(IDC_EDIT_INPUTPASSWORD))->SetWindowText(bBurnPassword ? L"******" : L"");

    ((CEdit *)GetDlgItem(IDC_EDIT_CONFIRMPASSWORD))->EnableWindow(bBurnPassword ? TRUE : FALSE);
    ((CEdit *)GetDlgItem(IDC_EDIT_CONFIRMPASSWORD))->SetWindowText(bBurnPassword ? L"******" : L"");
}

void CBurnControlDlg::DisplayJobListUI()
{
    if (m_eBurnCategory == BURNCATEGORY_MULTI)
    {
        CPictureListCtrl *pListCtrl=(CPictureListCtrl *)(GetDlgItem(IDC_LIST_JOBLIST));
        if (pListCtrl != NULL &&
            pListCtrl->GetSafeHwnd() != NULL)
        {
            if (pListCtrl->GetHeaderCtrl()->GetItemCount() == 0)
            {
                pListCtrl->SetExtendedStyle(pListCtrl->GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
                m_listCtrlJobList.SetHeaderBKColor(RGB(229,246,255),RGB(180,226,253),RGB(231,246,254));

                CRect rect;
                pListCtrl->GetClientRect(&rect);
                pListCtrl->InsertColumn(0,CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("uiword","cljobname").c_str()),LVCFMT_CENTER,rect.Width()/3);
                pListCtrl->InsertColumn(1,CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("uiword","cljobstate").c_str()),LVCFMT_CENTER,rect.Width()/6);
                pListCtrl->InsertColumn(2,CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("uiword","cljobsize").c_str()),LVCFMT_CENTER,rect.Width()/6);
                pListCtrl->InsertColumn(3,CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("uiword","clcdname").c_str()),LVCFMT_CENTER,rect.Width()/3);
            }

            pListCtrl->DeleteAllItems();

            if (m_normalJob.GetVecNormalBurnJobInfo().empty())
            {
                LOG_WARNING(("[CBurnControlDlg::DisplayJobListUI] No job,%d\r\n",__LINE__));
            }

            std::vector<CNormalBurnJobInfoEx>::iterator iter;
            int nIndex=-1;
            std::string strJobName;
            std::string strCDName;
            std::string strWaitMsg;
            for (iter=m_normalJob.GetVecNormalBurnJobInfo().begin();
                iter!=m_normalJob.GetVecNormalBurnJobInfo().end();++iter)
            {
                strJobName=iter->GetJobName();
                if (strJobName != "")
                {
                    ++nIndex;

                    pListCtrl->InsertItem(nIndex,CharsetConvertMFC::UTF8ToUTF16(strJobName.c_str()));
                }
                else
                {
                    LOG_WARNING(("[CBurnControlDlg::DisplayJobListUI()] Job name empty\r\n"));

                    continue;
                }

                strWaitMsg=m_cfgFile.GetValue("uiword","burnwait");
                if (strWaitMsg != "")
                {
                    pListCtrl->SetItemText(nIndex,1,CharsetConvertMFC::UTF8ToUTF16(strWaitMsg.c_str()));
                }

                double dJobSize;
                dJobSize=iter->GetJobSize()/1024;
                CStringW strJobSize;
                strJobSize.Format(L"%.2lf M",dJobSize);
                if (L"" == strJobSize)
                {
                    strJobSize=L"0";
                }
                pListCtrl->SetItemText(nIndex,2,strJobSize);

                strCDName=iter->GetCommonBurnParam().GetCDName();
                if (strCDName != "")
                {
                    pListCtrl->SetItemText(nIndex,3,CharsetConvertMFC::UTF8ToUTF16(strCDName.c_str()));
                }
            }
        }
    }
    else
    {
        LOG_WARNING(("[CBurnControlDlg::DisplayJobListUI()] Job type error\r\n"));

        assert(false);
    }
}

DWORD WINAPI ShowBurnDeviceThreadProc(LPVOID lpParameter)
{
    CBurnControlDlg *pBurnControlDlg=(CBurnControlDlg *)lpParameter;
    if (pBurnControlDlg != NULL &&
        pBurnControlDlg->GetSafeHwnd() != NULL)
    {
        //���¿�¼��������Ϣ
        pBurnControlDlg->UpdateBurnServerInfo();

        //���¹�����Ϣ,��Ϊ�ײ��ȡ������Ϣ���ܱȽ��������Եȴ��ظ���ʱ��ϳ�
        pBurnControlDlg->GetAndDisplayCDROMDriverInfo();
    }

    return 0;
}

DWORD WINAPI GetBurnServerInfoThreadProc(LPVOID lpParameter)
{
    if (lpParameter != NULL)
    {
        THREAD_PARAM *pThreadParam=(THREAD_PARAM *)(lpParameter);

        std::string strRemoteIP=std::string((char *)(pThreadParam->p1));
        std::string strRemotePort=std::string((char *)(pThreadParam->p2));
        std::string strProtocolContent=std::string((char *)(pThreadParam->pThreadData));

        CBurnControlDlg *pBurnControlDlg=(CBurnControlDlg *)(pThreadParam->pUserParam);

        LOG_INFO(("[GetBurnServerInfoThreadProc] ip : %s,port : %s,%d\r\n",
            strRemoteIP.c_str(),strRemotePort.c_str(),__LINE__));

        if (strRemoteIP != "" &&
            strRemotePort != "" &&
            strProtocolContent != "")
        {
            std::string strResponse;
            strResponse=CInterfaceProtocol::InterfaceCommunicate(strRemoteIP,
                atoi(strRemotePort.c_str()),strProtocolContent,BURNCONTROL_TAG_NAME,2);
            if (strResponse.length() > 0)
            {
                std::vector<CBurnServerInfoEx> vecBurnServerInfoEx;
                CInterfaceProtocol::AnalyzeProtocolBurnServerInfo(strResponse,vecBurnServerInfoEx);

                size_t nBurnServerCount=vecBurnServerInfoEx.size();
                for (size_t i=0;i<nBurnServerCount;++i)
                {
                    vecBurnServerInfoEx.at(i).SetIP(strRemoteIP);
                    vecBurnServerInfoEx.at(i).SetPort(strRemotePort);

                    //��¼�����������Ʋ���Ϊ�գ����һ̨�������ж������͵Ŀ�¼������
                    //��ô�Կ�¼������������Ϣ��Ϊ��¼�����������֣�������һ���Ϊ��
                    vecBurnServerInfoEx.at(i).SetBurnServerName(vecBurnServerInfoEx.at(i).GetDescription());
                }

                pBurnControlDlg->AddBurnServer(vecBurnServerInfoEx);

                //LOG_INFO(("[GetJobStateThread] Send burn state success,upper : %s,%s\r\n",
                //    strUpperIP.c_str(),strUpperPort.c_str()));
            }
            else
            {
            }
        }

        delete [] (char *)(pThreadParam->pThreadData);
        pThreadParam->pThreadData=NULL;

        delete [] (char *)(pThreadParam->p1);
        pThreadParam->p1=NULL;

        delete [] (char *)(pThreadParam->p2);
        pThreadParam->p2=NULL;

        delete pThreadParam;
        pThreadParam=NULL;
    }

    LOG_INFO(("[GetBurnServerInfoThreadProc] %d\r\n",__LINE__));

    return 0;
}

void CBurnControlDlg::AddBurnServer(const std::vector<CBurnServerInfoEx> &vecBurnServer)
{
    ZOSMutexLocker  locker(&m_mutexBurnServer);

    std::vector<CBurnServerInfoEx>::const_iterator iter;

    if (vecBurnServer.empty())
    {
    }
    else
    {
        if (m_eBurnCategory != BURNCATEGORY_RTBURN &&
            m_eBurnCategory != BURNCATEGORY_HM3)
        {
            //ֻ�з�ʵʱ��¼����Ҫ���¶���Ŀ�¼��������Ϣ�������Ƿ�֧�ֵ�������¼��
            
            for (iter=vecBurnServer.begin();iter!=vecBurnServer.end();++iter)
            {
                std::string strBurnServerName=iter->GetBurnServerName();
                std::string strIsAvailable=iter->GetIsAvailable();

                if (strBurnServerName != "" &&
                    BURNSERVER_AVAILABLE_YES == strIsAvailable)
                {
                    m_vecBurnServerInfoEx.push_back(*iter);
                }
            }
        }
    }
}

void CBurnControlDlg::UpdateBurnServerInfo()
{
    //ֻ�з�ʵʱ��¼����Ҫ���¶���Ŀ�¼��������Ϣ�������Ƿ�֧�ֵ�������¼��
    if ( (BURNCATEGORY_RTBURN == m_eBurnCategory) ||
        (BURNCATEGORY_HM3 == m_eBurnCategory) )
    {
        return;
    }

    std::vector<CBurnServerInfoEx>::iterator iter;

    if (WindowsProcess::FindProcess(L"BurnServer.exe") &&
        (m_eBurnCategory != BURNCATEGORY_HM3) &&
        (m_eBurnCategory != BURNCATEGORY_RTBURN))
    {
        std::string strHostIP=NetUtil::GetHostIP();

        bool bLocalBurnServer=false;//Э�����Ƿ����������¼������
        for (iter=m_vecBurnServerInfoEx.begin();iter!=m_vecBurnServerInfoEx.end();iter++)
        {
            if (iter->GetIP() == strHostIP)
            {
                bLocalBurnServer=true;

                break;
            }
        }

        if (!bLocalBurnServer)
        {
            CBurnServerInfo hostBurnServerInfo;
            hostBurnServerInfo.SetIP(strHostIP);
            hostBurnServerInfo.SetPort(IntToString(BURNSERVER_PORT));

            m_vecBurnServerInfoEx.push_back(CBurnServerInfoEx(hostBurnServerInfo));

            LOG_INFO(("[CBurnControlDlg::UpdateBurnServerInfo()] Add local BurnServer,%d\r\n",__LINE__));
        }
    }

    std::vector<CBurnServerInfoEx> vecTemp;
    vecTemp.swap(m_vecBurnServerInfoEx);

    // Log
    LOG_INFO(("[CBurnControlDlg::UpdateBurnServerInfo()] Initial BurnServer list : \r\n"));
    LOG_INFO(("--------------\r\n"));
    for (iter=vecTemp.begin();iter!=vecTemp.end();iter++)
    {
        LOG_INFO(("Server ip : %s,Server port : %s\r\n",iter->GetIP().c_str(),iter->GetPort().c_str()));
    }
    LOG_INFO(("--------------\r\n"));

    LOG_INFO(("[CBurnControlDlg::UpdateBurnServerInfo()] xxxx Begin,%d xxxx\r\n",__LINE__));

    std::vector<HANDLE> vecThreadHandle;

    //������ͬһ��BurnServer���Ͷ�λ�ȡ��ϢЭ��
    std::map<std::string,std::string> mapTemp;

    for (iter=vecTemp.begin();iter!=vecTemp.end();iter++)
    {
        std::string strIP=iter->GetIP();
        std::string strPort=iter->GetPort();

        if (!mapTemp.empty() &&
            mapTemp.find(strIP+strPort) != mapTemp.end())
        {
            continue;
        }

        mapTemp[strIP+strPort]="";

        THREAD_PARAM *pThreadParam=new THREAD_PARAM;
        if (pThreadParam != NULL)
        {
            std::string strProtocolContent;
            strProtocolContent+="<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n";
            strProtocolContent+="<burnControl>\r\n";
            strProtocolContent+="<commandType>getBurnServerInfo</commandType>\r\n";
            strProtocolContent+="</burnControl>\r\n";

            size_t nContentLen=strProtocolContent.length();
            pThreadParam->pThreadData=new char[nContentLen+1];
            if (pThreadParam->pThreadData != NULL)
            {
                strcpy((char *)(pThreadParam->pThreadData),strProtocolContent.c_str());

                pThreadParam->pUserParam=this;

                pThreadParam->p1=new char[128];
                if (pThreadParam->p1 != NULL)
                {
                    strcpy((char *)(pThreadParam->p1),strIP.c_str());

                    pThreadParam->p2=new char[128];
                    if (pThreadParam->p2 != NULL)
                    {
                        strcpy((char *)(pThreadParam->p2),strPort.c_str());

                        HANDLE hThread=CreateThread(NULL,0,GetBurnServerInfoThreadProc,pThreadParam,0,NULL);
                        if (hThread != NULL)
                        {
                            vecThreadHandle.push_back(hThread);

                            continue;
                        }

                        delete [] (char *)(pThreadParam->p2);
                        pThreadParam->p2=NULL;
                    }

                    delete [] (char *)(pThreadParam->p1);
                    pThreadParam->p1=NULL;
                }

                delete [] (char *)(pThreadParam->pThreadData);
                pThreadParam->pThreadData=NULL;
            }

            delete pThreadParam;
            pThreadParam=NULL;
        }
    }//for

    while(!vecThreadHandle.empty())
    {
        WaitForSingleObject(vecThreadHandle.back(),INFINITE);

        CloseHandle(vecThreadHandle.back());

        vecThreadHandle.pop_back();
    }

    LOG_INFO(("[CBurnControlDlg::UpdateBurnServerInfo()] xxxx End,%d xxxx\r\n",__LINE__));

    if (m_eBurnCategory != BURNCATEGORY_RTBURN &&
        m_eBurnCategory != BURNCATEGORY_HM3)
    {
        DisplayBurnServer(m_vecBurnServerInfoEx);
    }
}

CStringW CBurnControlDlg::GetAlertMsg() const 
{
    return m_strAlertMsg;
}
void CBurnControlDlg::SetAlertMsg(CStringW val) 
{
    m_strAlertMsg = val; 
}

void CBurnControlDlg::ShowBurnInfoUI(bool bUpdateServerAndCDROM)
{
    SetTimer(ID_TIMER_CHECKISDEVUPDATED,2000,NULL);

    if (bUpdateServerAndCDROM)
    {
        DWORD nThreadID;
        HANDLE hThread=CreateThread(NULL,0,ShowBurnDeviceThreadProc,this,0,&nThreadID);
        if (hThread != NULL)
        {
            CloseHandle(hThread);
        }
    }

    std::string strResPath;
    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_SINGLE://�������º��¼
        {
            assert(1 == m_normalJob.GetVecNormalBurnJobInfo().size());

            if (m_normalJob.GetVecNormalBurnJobInfo().size() > 0)
            {
                CNormalBurnJobInfoEx &normalJob=m_normalJob.GetVecNormalBurnJobInfo().at(0);

                std::string strCDName=normalJob.GetCommonBurnParam().GetCDName();
                GetDlgItem(IDC_EDIT_CDNAME)->SetWindowText(CharsetConvertMFC::UTF8ToUTF16(strCDName.c_str()));

                SetInitialBurnTypeUI(static_cast<BURNTYPESEL>(StringToInt(normalJob.GetCommonBurnParam().GetBurnType())));

                std::string strBurnPassword=normalJob.GetCommonBurnParam().GetBurnPassword();
                std::string strContentPassword=normalJob.GetCommonBurnParam().GetContentPassword();
                DisplayPasswordUI((strBurnPassword != ""),(strBurnPassword == strContentPassword));

                strResPath=m_cfgFile.GetValue("path","startburn");
                if (strResPath != "")
                {
                    m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstStartBurn.DrawBorder(FALSE);
                }

                strResPath=m_cfgFile.GetValue("path","close");
                if (strResPath != "")
                {
                    m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstClose.DrawBorder(FALSE);
                }
            }
        }
        break;

    case BURNCATEGORY_MULTI://�º��¼������
        {
            if (m_normalJob.GetVecNormalBurnJobInfo().empty())
            {
                LOG_ERROR(("[CBurnControlDlg::ShowBurnInfo()] No job,%d\r\n",__LINE__));

                assert(false);
            }
            else
            {
                //��ʾ�����б�
                DisplayJobListUI();

                strResPath=m_cfgFile.GetValue("path","jobup");
                if (strResPath != "")
                {
                    m_btnstJobUp.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstJobUp.DrawBorder(FALSE);
                }

                strResPath=m_cfgFile.GetValue("path","jobdown");
                if (strResPath != "")
                {
                    m_btnstJobDown.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstJobDown.DrawBorder(FALSE);
                }

                SetInitialBurnTypeUI(static_cast<BURNTYPESEL>(StringToInt(m_normalJob.GetVecNormalBurnJobInfo().at(m_nJobIndex).GetCommonBurnParam().GetBurnType())));

                std::string strBurnPassword=m_normalJob.GetVecNormalBurnJobInfo().at(m_nJobIndex).GetCommonBurnParam().GetBurnPassword();
                std::string strContentPassword=m_normalJob.GetVecNormalBurnJobInfo().at(m_nJobIndex).GetCommonBurnParam().GetContentPassword();
                DisplayPasswordUI((strBurnPassword != ""),(strBurnPassword == strContentPassword));

                strResPath=m_cfgFile.GetValue("path","startburn");
                if (strResPath != "")
                {
                    m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstStartBurn.DrawBorder(FALSE);
                }

                strResPath=m_cfgFile.GetValue("path","close");
                if (strResPath != "")
                {
                    m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstClose.DrawBorder(FALSE);
                }
            }
        }
        break;

    case BURNCATEGORY_COS://cos�º��¼
        {
            if (1 != m_normalJob.GetVecNormalBurnJobInfo().size())
            {
                LOG_ERROR(("[CBurnControlDlg::ShowBurnInfo()] Job count error\r\n"));
                assert(false);
            }
            else
            {
                CNormalBurnJobInfoEx &normalBurnJobInfoEx=
                    m_normalJob.GetVecNormalBurnJobInfo().at(0);

                std::string strCDName=normalBurnJobInfoEx.GetCommonBurnParam().GetCDName();
                CStringW strCDNameW=CharsetConvertMFC::UTF8ToUTF16(strCDName.c_str());
                GetDlgItem(IDC_EDIT_CDNAME)->SetWindowText(strCDNameW);

                DisplayBurnContentUI();

                SetInitialBurnTypeUI(static_cast<BURNTYPESEL>(StringToInt(normalBurnJobInfoEx.GetCommonBurnParam().GetBurnType())));

                std::string strBurnPassword=normalBurnJobInfoEx.GetCommonBurnParam().GetBurnPassword();
                std::string strContentPassword=normalBurnJobInfoEx.GetCommonBurnParam().GetContentPassword();
                DisplayPasswordUI((strBurnPassword != ""),(strBurnPassword == strContentPassword));

                strResPath=m_cfgFile.GetValue("path","startburn");
                if (strResPath != "")
                {
                    m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstStartBurn.DrawBorder(FALSE);
                }

                strResPath=m_cfgFile.GetValue("path","close");
                if (strResPath != "")
                {
                    m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstClose.DrawBorder(FALSE);
                }
            }
        }
        break;

    case BURNCATEGORY_HM3:
        {
            DisplayBurnContentUI();

            SetInitialBurnTypeUI(static_cast<BURNTYPESEL>(StringToInt(m_RTBurnInfo.GetCommonBurnParam().GetBurnType())));

            std::string strBurnPassword=m_RTBurnInfo.GetCommonBurnParam().GetBurnPassword();
            std::string strContentPassword=m_RTBurnInfo.GetCommonBurnParam().GetContentPassword();
            DisplayPasswordUI((strBurnPassword != ""),(strBurnPassword == strContentPassword));

            strResPath=m_cfgFile.GetValue("path","startburn");
            if (strResPath != "")
            {
                m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstStartBurn.DrawBorder(FALSE);
            }

            strResPath=m_cfgFile.GetValue("path","pause2");
            if (strResPath != "")
            {
                m_btnstPause.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstPause.DrawBorder(FALSE);
            }

            strResPath=m_cfgFile.GetValue("path","save2");
            if (strResPath != "")
            {
                m_btnstSave.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstSave.DrawBorder(FALSE);
            }

            strResPath=m_cfgFile.GetValue("path","close");
            if (strResPath != "")
            {
                m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstClose.DrawBorder(FALSE);
            }
        }
        break;

    case BURNCATEGORY_RTBURN://ʵʱ��¼
        {
            DisplayBurnContentUI();

            SetInitialBurnTypeUI(static_cast<BURNTYPESEL>(StringToInt(m_RTBurnInfo.GetCommonBurnParam().GetBurnType())));

            std::string strBurnPassword=m_RTBurnInfo.GetCommonBurnParam().GetBurnPassword();
            std::string strContentPassword=m_RTBurnInfo.GetCommonBurnParam().GetContentPassword();
            DisplayPasswordUI((strBurnPassword != ""),(strBurnPassword == strContentPassword));

            if ("2" == m_RTBurnInfo.GetJobType())
            {
                //����ͥǰ׼���Ļ���
                strResPath=m_cfgFile.GetValue("path","startburn2");
                if (strResPath != "")
                {
                    m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstStartBurn.DrawBorder(FALSE);
                }

                strResPath=m_cfgFile.GetValue("path","pause2");
                if (strResPath != "")
                {
                    m_btnstPause.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstPause.DrawBorder(FALSE);
                }

                strResPath=m_cfgFile.GetValue("path","save");
                if (strResPath != "")
                {
                    m_btnstSave.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstSave.DrawBorder(FALSE);
                }

                strResPath=m_cfgFile.GetValue("path","close");
                if (strResPath != "")
                {
                    m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstClose.DrawBorder(FALSE);
                }
            }
            else if ("1" == m_RTBurnInfo.GetJobType())
            {
                //����ͥ����̿���ʵʱ��¼�Ļ���

                strResPath=m_cfgFile.GetValue("path","startburn");
                if (strResPath != "")
                {
                    m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstStartBurn.DrawBorder(FALSE);
                }

                strResPath=m_cfgFile.GetValue("path","pause2");
                if (strResPath != "")
                {
                    m_btnstPause.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstPause.DrawBorder(FALSE);
                }

                strResPath=m_cfgFile.GetValue("path","save");
                if (strResPath != "")
                {
                    m_btnstSave.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstSave.DrawBorder(FALSE);
                }

                strResPath=m_cfgFile.GetValue("path","close");
                if (strResPath != "")
                {
                    m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstClose.DrawBorder(FALSE);
                }
            }

        }
        break;

    case BURNCATEGORY_UNKNOWN:
    default:
        LOG_ERROR(("[CBurnControlDlg::ShowBurnInfo] Burn type is not correct\r\n"));
        assert(false);
    }

    ClearDevBurnStateInfo();
}

void CBurnControlDlg::ShowBurnControlUI(bool bShowUI)
{
    ShowWindow(SW_HIDE);

    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_SINGLE:
        {
            HideJobList();
            HideBurnContent();
            CreateUI(std::string(BURNCONTROL_CFG_PATH)+"BurnControlUINoneRTSingle.xml");
        }
        break;

    case BURNCATEGORY_MULTI:
        {
            HideCDName();
            HideBurnContent();
            CreateUI(std::string(BURNCONTROL_CFG_PATH)+"BurnControlUINoneRTMulti.xml");
        }
        break;

    case BURNCATEGORY_COS:
        {
            HideJobList();
            CreateUI(std::string(BURNCONTROL_CFG_PATH)+"BurnControlUINoneRTCOS.xml");
        }
        break;

    case BURNCATEGORY_HM3:
    case BURNCATEGORY_RTBURN:
        {
            HideCDName();
            HideRecorder();//���ؿ�¼��
            HideFileInfo();//���ؿ�¼�ļ���Ϣ
            CreateUI(std::string(BURNCONTROL_CFG_PATH)+"BurnControlUIRT.xml");
        }
        break;

    case BURNCATEGORY_UNKNOWN:
    default:
        LOG_WARNING(("[CBurnControlDlg::ShowBurnUI] Unknown burn type\r\n"));
    }

    ShowBurnInfoUI(bShowUI);

    UpdateBurnControlUI();

    if (bShowUI)
    {
        ShowWindow(SW_SHOW);
        SetForegroundWindow();
    }
}

int CBurnControlDlg::GetCheckedContent()
{
    int nRet=0;

    std::vector<CLocationInfo> vecLocationInfo=GetVecLocationInfo();
    if (!vecLocationInfo.empty())
    {
        size_t nLocationCount=vecLocationInfo.size();
        for (size_t i=0;i<nLocationCount;++i)
        {
            CWnd *p=GetDlgItem(i+IDC_CHECK_L1);
            if (p != NULL)
            {
                if (p->GetSafeHwnd() != NULL)
                {
                    if (p->IsWindowVisible())
                    {
                        if (((CButton *)p)->GetCheck() == BST_CHECKED)
                        {
                            ++nRet;
                        }
                    }
                }
            }
        }
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::SaveBurnContentInfo] Can not get video location information\r\n"));
        assert(false);
    }

    return nRet;
}

void CBurnControlDlg::SaveBurnContentInfo()
{
    std::vector<CLocationInfo> vecLocationInfo=GetVecLocationInfo();
    if (!vecLocationInfo.empty())
    {
        size_t nLocationCount=vecLocationInfo.size();
        for (size_t i=0;i<nLocationCount;++i)
        {
            CWnd *p=GetDlgItem(i+IDC_CHECK_L1);
            if (p != NULL)
            {
                if (p->GetSafeHwnd() != NULL)
                {
                    if (p->IsWindowVisible())
                    {
                        vecLocationInfo.at(i).SetBurnOrNot( (((CButton *)p)->GetCheck() == BST_CHECKED) ? BURN_YES : BURN_NO );
                    }
                }
            }
        }

        // Just for log
        //std::vector<CLocationInfo>::const_iterator iter;
        //for (iter=vecLocationInfo.begin();iter!=vecLocationInfo.end();++iter)
        //{
        //    CStringA str=CStringA(CharsetConvertMFC::UTF8ToUTF16(iter->GetDescription().c_str()));
        //    if ("2" == iter->GetBurnOrNot())
        //    {
        //        LOG_INFO(("[[CBurnControlDlg::SaveBurnContentInfo]] %s is not selected\r\n",str.GetBuffer()));
        //    }
        //    else if("1" == iter->GetBurnOrNot())
        //    {
        //        LOG_INFO(("[[CBurnControlDlg::SaveBurnContentInfo]] %s is selected\r\n",str.GetBuffer()));
        //    }
        //    else
        //    {
        //        LOG_ERROR(("[CBurnControlDlg::SaveBurnContentInfo] Unknown burn selection\r\n"));
        //        assert(false);
        //    }
        //}
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::SaveBurnContentInfo] Can not get video location information\r\n"));
        assert(false);
    }

    //COS�º��¼��ͥ���¼��
    if (BURNCATEGORY_COS == m_eBurnCategory)
    {
        assert(1 == m_normalJob.GetVecNormalBurnJobInfo().size());

        const CNormalBurnJobInfoEx &info=m_normalJob.GetVecNormalBurnJobInfo().at(0);

        bool bIsSelected=false;
        if (info.GetNoteFileRemoteIP() != "" &&
            info.GetNoteFileRemotePort() != "" &&
            info.GetNoteRelativePath() != "" &&
            info.GetNoteFileName() != "")
        {
            //ȷ��ÿ��check box��ID��������
            UINT unNoteCheckBoxID=IDC_CHECK_L1+vecLocationInfo.size();

            if (GetDlgItem(unNoteCheckBoxID) != NULL &&
                GetDlgItem(unNoteCheckBoxID)->GetSafeHwnd() != NULL)
            {
                bIsSelected=((CButton *)GetDlgItem(unNoteCheckBoxID))->GetCheck() == BST_CHECKED;
            }

            m_normalJob.GetVecNormalBurnJobInfo().at(0).SetBurnNoteFile(bIsSelected);
        }
    }

    //�����¼����
    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_RTBURN:
    case BURNCATEGORY_HM3:
        m_RTBurnInfo.SetVecVideoLocationInfo(vecLocationInfo);
        break;

    case BURNCATEGORY_COS:
        {
            assert(1 == m_normalJob.GetVecNormalBurnJobInfo().size());

            m_normalJob.GetVecNormalBurnJobInfo().at(0).SetVecVideoLocationInfo(vecLocationInfo);
        }
        break;

    default:
        assert(false);
    }
}

BOOL CBurnControlDlg::SavePassword()
{
    CString strBurnPassword;
    CString strConfirmPassword;
    if (((CButton *)GetDlgItem(IDC_CHECK_BURNPASSWORD))->GetCheck() == BST_CHECKED)
    {
        GetDlgItem(IDC_EDIT_INPUTPASSWORD)->GetWindowText(strBurnPassword);
        GetDlgItem(IDC_EDIT_CONFIRMPASSWORD)->GetWindowText(strConfirmPassword);

        if (strBurnPassword == strConfirmPassword &&
            strBurnPassword != "")
        {
            switch (m_eBurnCategory)
            {
            case BURNCATEGORY_RTBURN:
            case BURNCATEGORY_HM3:
                m_RTBurnInfo.GetCommonBurnParam().SetBurnPassword(CharsetConvertMFC::UTF16ToUTF8StdString(strBurnPassword));
                if (((CButton *)GetDlgItem(IDC_CHECK_CONTENTPASSWORD))->GetCheck() == BST_CHECKED)
                {
                    m_RTBurnInfo.GetCommonBurnParam().SetContentPassword(CharsetConvertMFC::UTF16ToUTF8StdString(strBurnPassword));
                }
                break;

            case BURNCATEGORY_COS:
                if (1 == m_normalJob.GetVecNormalBurnJobInfo().size())
                {
                    m_normalJob.GetVecNormalBurnJobInfo().at(0).GetCommonBurnParam().SetBurnPassword(
                        CharsetConvertMFC::UTF16ToUTF8StdString(strBurnPassword));
                    if (((CButton *)GetDlgItem(IDC_CHECK_CONTENTPASSWORD))->GetCheck() == BST_CHECKED)
                    {
                        m_normalJob.GetVecNormalBurnJobInfo().at(0).GetCommonBurnParam().SetContentPassword(
                            CharsetConvertMFC::UTF16ToUTF8StdString(strBurnPassword));
                    }
                }
                else
                {
                    LOG_ERROR(("[CBurnControlDlg::SavePassword] Job count error\r\n"));
                    assert(false);
                }
                break;

            case BURNCATEGORY_SINGLE:
                if (1 == m_normalJob.GetVecNormalBurnJobInfo().size())
                {
                    m_normalJob.GetVecNormalBurnJobInfo().at(0).GetCommonBurnParam().SetBurnPassword(
                        CharsetConvertMFC::UTF16ToUTF8StdString(strBurnPassword));
                    if (((CButton *)GetDlgItem(IDC_CHECK_CONTENTPASSWORD))->GetCheck() == BST_CHECKED)
                    {
                        m_normalJob.GetVecNormalBurnJobInfo().at(0).GetCommonBurnParam().SetContentPassword(
                            CharsetConvertMFC::UTF16ToUTF8StdString(strBurnPassword));
                    }
                }
                else
                {
                    LOG_ERROR(("[CBurnControlDlg::SavePassword] Job count error\r\n"));
                    assert(false);
                }
                break;

            case BURNCATEGORY_MULTI:
                {
                    size_t nJobCount=m_normalJob.GetVecNormalBurnJobInfo().size();
                    if (nJobCount <= 0)
                    {
                        LOG_ERROR(("[CBurnControlDlg::SavePassword] No job\r\n"));
                        assert(false);
                    }
                    for (size_t i=0;i<nJobCount;++i)
                    {
                        m_normalJob.GetVecNormalBurnJobInfo().at(i).GetCommonBurnParam().SetBurnPassword(
                            CharsetConvertMFC::UTF16ToUTF8StdString(strBurnPassword));
                        if (((CButton *)GetDlgItem(IDC_CHECK_CONTENTPASSWORD))->GetCheck() == BST_CHECKED)
                        {
                            m_normalJob.GetVecNormalBurnJobInfo().at(i).GetCommonBurnParam().SetContentPassword(
                                CharsetConvertMFC::UTF16ToUTF8StdString(strBurnPassword));
                        }
                    }
                }
                break;

            case BURNCATEGORY_UNKNOWN:
            default:
                assert(false);
                break;
            }
        }
        else
        {
            //��ʾ����Ϊ�ջ����벻һ��
            return FALSE;
        }
    }

    return TRUE;
}

void CBurnControlDlg::SaveCDName()
{
    switch(m_eBurnCategory)
    {
    case BURNCATEGORY_COS:
    case BURNCATEGORY_SINGLE:
        {
            CEdit *peditCDName=(CEdit *)(GetDlgItem(IDC_EDIT_CDNAME));
            CString strCDName;
            if (peditCDName != NULL &&
                peditCDName->GetSafeHwnd() != NULL)
            {
                peditCDName->GetWindowText(strCDName);

                assert(1 == m_normalJob.GetVecNormalBurnJobInfo().size());

                if (strCDName != L"")
                {
                    m_normalJob.GetVecNormalBurnJobInfo().at(0).GetCommonBurnParam().SetCDName(
                        CharsetConvertMFC::UTF16ToUTF8(strCDName).GetBuffer());
                }
                else
                {
                    m_normalJob.GetVecNormalBurnJobInfo().at(0).GetCommonBurnParam().SetCDName("");
                }
            }
        }
        break;

    case BURNCATEGORY_MULTI:
        {

            CListCtrl *plistctrlJobList=(CListCtrl *)(GetDlgItem(IDC_LIST_JOBLIST));
            if (plistctrlJobList != NULL &&
                plistctrlJobList->GetSafeHwnd() != NULL)
            {
                int nItemCount=plistctrlJobList->GetItemCount();
                if (m_normalJob.GetVecNormalBurnJobInfo().size() == static_cast<size_t>(nItemCount))
                {
                    for (int i=0;i<nItemCount;++i)
                    {
                        wchar_t szCDName[128]={0};
                        plistctrlJobList->GetItemText(i,3,szCDName,128);
                        if (wcslen(szCDName) > 0)
                        {
                            m_normalJob.GetVecNormalBurnJobInfo().at(i).GetCommonBurnParam().SetCDName(
                                CharsetConvertMFC::UTF16ToUTF8(szCDName).GetBuffer());
                        }
                        else
                        {
                            m_normalJob.GetVecNormalBurnJobInfo().at(i).GetCommonBurnParam().SetCDName("");
                        }
                    }
                }
                else
                {
                    LOG_WARNING(("[CBurnControlDlg::SaveCDName] Job list item count error\r\n"));
                    assert(false);
                }
            }
        }
        break;

    default:
        break;
    }
}

void CBurnControlDlg::SaveBurnServerInfo()
{
    BURNTYPESEL eBurnTypeSel=GetCurBurnTypeSel();
    std::string strBurnTypeSel;
    switch(eBurnTypeSel)
    {
    case BURNTYPE_SINGLE:
        strBurnTypeSel="1";
        break;
    case BURNTYPE_MULTISYN:
        strBurnTypeSel="2";
        break;
    case BURNTYPE_MULTIASYN:
        strBurnTypeSel="3";
        break;
    default:
        break;
    }

    CBurnServerInfo burnServerInfo;
    bool bRet=GetCurBurnServerSel(burnServerInfo);
    if (eBurnTypeSel != BURNTYPE_UNKNOWNSEL &&
        bRet)
    {
        switch (m_eBurnCategory)
        {
        case BURNCATEGORY_HM3:
            m_RTBurnInfo.SetBurnServerInfo(burnServerInfo);
            m_RTBurnInfo.GetCommonBurnParam().SetBurnType(strBurnTypeSel);
            break;

        case BURNCATEGORY_RTBURN:
            m_RTBurnInfo.SetBurnServerInfo(burnServerInfo);
            m_RTBurnInfo.GetCommonBurnParam().SetBurnType(strBurnTypeSel);
            break;

        case BURNCATEGORY_COS:
        case BURNCATEGORY_SINGLE:
            if (!m_normalJob.GetVecNormalBurnJobInfo().empty())
            {
                m_normalJob.GetVecNormalBurnJobInfo().at(0).SetBurnServerInfo(burnServerInfo);
                m_normalJob.GetVecNormalBurnJobInfo().at(0).GetCommonBurnParam().SetBurnType(strBurnTypeSel);
            }
            else
            {
                assert(false);
            }
            break;

        case BURNCATEGORY_MULTI:
            {
                //��ǰ����ԭ����������������Ӧ��ͬ���Ŀ�¼����
                size_t nJobCount=m_normalJob.GetVecNormalBurnJobInfo().size();
                assert(nJobCount > 0);
                for (size_t i=0;i<nJobCount;++i)
                {
                    m_normalJob.GetVecNormalBurnJobInfo().at(i).SetBurnServerInfo(burnServerInfo);
                    m_normalJob.GetVecNormalBurnJobInfo().at(i).GetCommonBurnParam().SetBurnType(strBurnTypeSel);
                }
            }
            break;

        default:
            assert(false);
            break;
        }
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::SaveBurnServerInfo] Unknown burn type or GetCurBurnServerSel failed\r\n"));
    }
}

//����Ƕ������̣���ʱҲ��ÿһ�������Ŀ�¼��������Ϊһ���ģ���ҪBurnServer��������
void CBurnControlDlg::SaveCDROMDriverBurnContent()
{
    std::vector<CLocationInfo> vecLocationInfo=GetVecLocationInfo();

    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_RTBURN:
    case BURNCATEGORY_HM3:
        {
            if (!vecLocationInfo.empty())
            {
                std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo=m_RTBurnInfo.GetBurnServerInfo().GetVecCDROMDriverInfo();
                std::vector<CCDROMDriverInfo>::iterator iter;
                for (iter=vecCDROMDriverInfo.begin();iter!=vecCDROMDriverInfo.end();++iter)
                {
                    if (iter->GetIsSelected() == DEV_SELECTED_YES)
                    {
                        iter->SetVecLocationInfo(vecLocationInfo);
                    }
                }
            }
        }
        break;

    case BURNCATEGORY_COS:
    case BURNCATEGORY_SINGLE:
        {
            if (!vecLocationInfo.empty() &&
                1 == m_normalJob.GetVecNormalBurnJobInfo().size())
            {
                std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo=
                    m_normalJob.GetVecNormalBurnJobInfo().at(0).GetBurnServerInfo().GetVecCDROMDriverInfo();
                std::vector<CCDROMDriverInfo>::iterator iter;
                for (iter=vecCDROMDriverInfo.begin();iter!=vecCDROMDriverInfo.end();++iter)
                {
                    if (iter->GetIsSelected() == "1")
                    {
                        iter->SetVecLocationInfo(vecLocationInfo);
                    }
                }
            }
        }
        break;

    case BURNCATEGORY_MULTI:
        {
            if (!vecLocationInfo.empty())
            {
                std::vector<CNormalBurnJobInfoEx> &vecNormalBurnJobInfo=m_normalJob.GetVecNormalBurnJobInfo();
                std::vector<CNormalBurnJobInfoEx>::iterator iterJob;
                for (iterJob=vecNormalBurnJobInfo.begin();iterJob!=vecNormalBurnJobInfo.end();++iterJob)
                {
                    std::vector<CCDROMDriverInfo> &vecCDROMDriverInfo=iterJob->GetBurnServerInfo().GetVecCDROMDriverInfo();
                    std::vector<CCDROMDriverInfo>::iterator iterCDROMDriver;
                    for (iterCDROMDriver=vecCDROMDriverInfo.begin();iterCDROMDriver!=vecCDROMDriverInfo.end();++iterCDROMDriver)
                    {
                        if (iterCDROMDriver->GetIsSelected() == "1")
                        {
                            iterCDROMDriver->SetVecLocationInfo(vecLocationInfo);
                        }
                    }
                }
            }
        }
        break;

    case BURNCATEGORY_UNKNOWN:
    default:
        LOG_ERROR(("[CBurnControlDlg::SaveCDROMDriverBurnContent] Unknown burn category\r\n"));

        assert(false);

        break;
    }
}

void CBurnControlDlg::SaveRTBurnCfg()
{
    SaveBurnContentInfo();

    //����ѡ������Щ����
    SaveBurnServerInfo();

    SavePassword();
    SaveCDROMDriverBurnContent();
}

void CBurnControlDlg::SaveNoneRTBurnCosCfg()
{
    SaveCDName();
    SaveBurnContentInfo();
    SaveBurnServerInfo();
    SavePassword();
    SaveCDROMDriverBurnContent();
}

void CBurnControlDlg::SaveNoneRTBurnSingleCfg()
{
    SaveCDName();
    SaveBurnServerInfo();
    SavePassword();
    SaveCDROMDriverBurnContent();
}

void CBurnControlDlg::SaveNoneRTBurnMultiCfg()
{
    SaveCDName();
    SaveBurnServerInfo();
    SavePassword();
    SaveCDROMDriverBurnContent();
}

void CBurnControlDlg::CreateAdditionalBurnFile()
{
    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_RTBURN:
    case BURNCATEGORY_HM3:
        {
            CreatePlayListFile(m_RTBurnInfo);
            CreateAutorunFile(m_RTBurnInfo);
        }
        break;

    case BURNCATEGORY_COS:
    case BURNCATEGORY_SINGLE:
        {
            assert(1 == m_normalJob.GetVecNormalBurnJobInfo().size());

            if (1 == m_normalJob.GetVecNormalBurnJobInfo().size())
            {
                CreatePlayListFile(m_normalJob.GetVecNormalBurnJobInfo().at(0));

                CreateAutorunFile(m_normalJob.GetVecNormalBurnJobInfo().at(0));
            }
            else
            {
                LOG_INFO(("[CBurnControlDlg::CreateAdditionalBurnFile] Job num error : %d\r\n",
                    m_normalJob.GetVecNormalBurnJobInfo().size()));
            }
        }
        break;

    case BURNCATEGORY_MULTI:
        {
            size_t nJobCount=m_normalJob.GetVecNormalBurnJobInfo().size();

            if (m_nJobIndex < nJobCount)
            {
                CreatePlayListFile(m_normalJob.GetVecNormalBurnJobInfo().at(m_nJobIndex));

                CreateAutorunFile(m_normalJob.GetVecNormalBurnJobInfo().at(m_nJobIndex));
            }
            else
            {
                assert(false);
            }
        }
        break;

    case BURNCATEGORY_UNKNOWN:
    default:
        assert(false);
    }
}

void CBurnControlDlg::CreatePlayListFile(CRTBurnInfo &RtJob)
{
    std::string strProtocolContent;
    strProtocolContent+="<?xml version=\"1.0\" encoding=\"gb2312\" ?>\r\n";

    strProtocolContent+="<config ";

    strProtocolContent+="playerrendermode=";
    strProtocolContent+="\"";
    strProtocolContent+=CharsetConvertMFC::UTF16ToGB18030(L"0");
    strProtocolContent+="\" ";

    strProtocolContent+="splitmode=";
    strProtocolContent+="\"";
    strProtocolContent+=CharsetConvertMFC::UTF16ToGB18030(L"1");
    strProtocolContent+="\" ";

    strProtocolContent+="playerlayout=";
    strProtocolContent+="\"";
    strProtocolContent+=CharsetConvertMFC::UTF16ToGB18030(L"1");
    strProtocolContent+="\" ";

    strProtocolContent+="playtype=";
    strProtocolContent+="\"";
    strProtocolContent+=CharsetConvertMFC::UTF16ToGB18030(L"���ز���");
    strProtocolContent+="\" ";

    strProtocolContent+=">";

    std::string strNoteFileWholePath=RtJob.GetNoteFileDownloadURL();
    size_t nIndex=strNoteFileWholePath.rfind("\\");
    std::string strNoteFileName;
    if (nIndex != std::string::npos)
    {
        strNoteFileName=std::string(strNoteFileWholePath.begin()+nIndex+1,strNoteFileWholePath.end());
    }
    strProtocolContent+="<notepath>";
    if (strNoteFileName != "")
    {
        //ʵʱ��¼ȷ����¼�����ĵ�,"note_1"������BurnControl��BurnServer����һ��
        std::string strNoteFileNameNoExtent=FileUtil::GetFileNameNoExtent(strNoteFileName);
        std::string strNewNoteFileName=strNoteFileName;
        strNewNoteFileName.replace(0,strNoteFileNameNoExtent.length(),"note_1");
        
        strProtocolContent+=strNewNoteFileName;
    }
    strProtocolContent+="</notepath>\r\n";

    strProtocolContent+="<playlistroot>";
    if (RtJob.GetJobID() != "")
    {
        strProtocolContent+=CharsetConvertMFC::UTF8ToGB18030(RtJob.GetJobID().c_str());
    }
    strProtocolContent+="</playlistroot>\r\n";

    std::string strDisplay=RtJob.GetPlayListContent();//��������ʾ��Ϣ

    char szTemp[1024]={0};
    int nTemp=1024;
    ATL::Base64Decode(strDisplay.c_str(),strDisplay.length(),(BYTE *)szTemp,&nTemp);

    strProtocolContent+=CharsetConvertMFC::UTF8ToGB18030(szTemp).GetBuffer();//��������ʾ��Ϣ

    std::string strBurnType=RtJob.GetCommonBurnParam().GetBurnType();
    if ("1" == strBurnType ||
        "2" == strBurnType ||
        "3" == strBurnType)//��ʾÿ��������¼������һ�������Կ���ʹ��һ��playlist�ļ�
    {
        const std::vector<CLocationInfo> &vecVideoLocationInfo = RtJob.GetVecVideoLocationInfo();
        std::vector<CLocationInfo>::const_iterator iterVideoLocationInfo = vecVideoLocationInfo.begin();
        int nStreamIndex = 0;
        for(;iterVideoLocationInfo != vecVideoLocationInfo.end(); iterVideoLocationInfo++)
        {
            if (iterVideoLocationInfo->GetBurnOrNot() == "1")
            {
                strProtocolContent+="<view ";

                //strProtocolContent+="location=";
                //strProtocolContent+="\"";
                //strProtocolContent+="1";
                //strProtocolContent+="\" ";

                strProtocolContent+="locationname=";
                strProtocolContent+="\"";
                strProtocolContent+=CharsetConvertMFC::UTF8ToGB18030(iterVideoLocationInfo->GetDescription().c_str());
                strProtocolContent+="\" ";

                strProtocolContent+="kerneltype=";
                strProtocolContent+="\"";
                strProtocolContent+="chnsys";
                strProtocolContent+="\" ";

                strProtocolContent+="streamindex=";
                strProtocolContent+="\"";
                strProtocolContent+=IntToString(nStreamIndex);
                strProtocolContent+="\" ";

                strProtocolContent+=">\r\n";

                //ʵʱ��¼��������һ��ts�ļ�
                strProtocolContent+="<url>";
                if (RtJob.GetNewFileName() != "")
                {
                    strProtocolContent+=CharsetConvertMFC::UTF8ToGB18030(RtJob.GetNewFileName().c_str()).GetBuffer();
                }
                else
                {
                    strProtocolContent+="trial.ts";//Ĭ������
                }
                strProtocolContent+="</url>\r\n";

                strProtocolContent+="</view>\r\n";
                nStreamIndex++;
            }
        }
    }

    strProtocolContent+="</config>";

    TiXmlDocument xmlDocument;
    xmlDocument.Parse(strProtocolContent.c_str(),NULL,TIXML_ENCODING_LEGACY);

    CStringW strTempDir=CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("path","tempdir").c_str());

    _wmkdir(strTempDir.GetBuffer());

    CStringW strWholePath=strTempDir+CStringW(L"\\")+CharsetConvertMFC::UTF8ToUTF16(RtJob.GetJobID().c_str())+CStringW(L".playlist");

    char szWholePath[1024]={0};
    WcToMb(strWholePath.GetBuffer(),szWholePath,sizeof(szWholePath)/sizeof(char));

    if (xmlDocument.SaveFile(szWholePath))
    {
        LOG_INFO(("[CInterfaceProtocol::CreatePlayListFile] Create play list file success\r\n"));

        std::string strLocalIP=NetUtil::GetHostIP(); 

        CPlaylistFileInfo playlistFileInfo;
        playlistFileInfo.SetRemoteIP(strLocalIP);
        char szPort[8]={0};
        sprintf(szPort,"%d",FILEANYWHERE_PORT);
        playlistFileInfo.SetRemotePort(szPort);
        playlistFileInfo.SetDownloadURL(CharsetConvertMFC::UTF16ToUTF8(strWholePath).GetBuffer());
        RtJob.SetPlaylistFileInfo(playlistFileInfo);
    }
    else
    {
        LOG_ERROR(("[CInterfaceProtocol::CreatePlayListFile] Create play list file failed\r\n"));
    }
}
void CBurnControlDlg::CreatePlayListFile(CNormalBurnJobInfoEx &jobInfo)
{
    std::string strProtocolContent;
    strProtocolContent+="<?xml version=\"1.0\" encoding=\"gb2312\" ?>\r\n";

    strProtocolContent+="<config ";

    strProtocolContent+="playerrendermode=";
    strProtocolContent+="\"";
    strProtocolContent+=CharsetConvertMFC::UTF16ToGB18030(L"0");
    strProtocolContent+="\" ";

    strProtocolContent+="splitmode=";
    strProtocolContent+="\"";
    strProtocolContent+=CharsetConvertMFC::UTF16ToGB18030(L"1");
    strProtocolContent+="\" ";

    strProtocolContent+="playerlayout=";
    strProtocolContent+="\"";
    strProtocolContent+=CharsetConvertMFC::UTF16ToGB18030(L"1");
    strProtocolContent+="\" ";

    strProtocolContent+="playtype=";
    strProtocolContent+="\"";
    strProtocolContent+=CharsetConvertMFC::UTF16ToGB18030(L"���ز���");
    strProtocolContent+="\" ";

    strProtocolContent+=">";

    strProtocolContent+="<playlistroot>";
    if (jobInfo.GetJobID() != "")
    {
        strProtocolContent+=CharsetConvertMFC::UTF8ToGB18030(jobInfo.GetJobID().c_str());
    }
    strProtocolContent+="</playlistroot>\r\n";

    strProtocolContent+="<notepath>";
    if (jobInfo.GetNoteFileName() != "")
    {
        strProtocolContent+=CharsetConvertMFC::UTF8ToGB18030(jobInfo.GetNoteFileName().c_str()).GetBuffer();
    }
    strProtocolContent+="</notepath>\r\n";

    std::string strDisplay=jobInfo.GetPlayListContent();

    char szTemp[1024]={0};
    int nTemp=1024;
    ATL::Base64Decode(strDisplay.c_str(),strDisplay.length(),(BYTE *)szTemp,&nTemp);

    CStringW strTest;
    strTest=CharsetConvertMFC::UTF8ToUTF16(szTemp);

    LOG_INFO(("\r\n%s\r\n",szTemp))

        if (strlen(szTemp) > 0)
        {
            strProtocolContent+=CharsetConvertMFC::UTF8ToGB18030(std::string(szTemp).c_str()).GetBuffer();//��������ʾ��Ϣ
        }

        std::string strBurnType=jobInfo.GetCommonBurnParam().GetBurnType();
        if ("1" == strBurnType ||
            "2" == strBurnType ||
            "3" == strBurnType)//��ʾÿ��������¼������һ�������Կ���ʹ��һ��playlist�ļ�
        {
            const std::vector<CLocationInfo> &vecLocationInfo=jobInfo.GetVecLocationInfo();
            std::vector<CLocationInfo>::const_iterator iterLocationInfo;
            for (iterLocationInfo=vecLocationInfo.begin();iterLocationInfo!=vecLocationInfo.end();
                ++iterLocationInfo)
            {
                strProtocolContent+="<view ";

                //strProtocolContent+="location=";
                //strProtocolContent+="\"";
                //if (iterLocationInfo->GetID() != "")
                //{
                //    strProtocolContent+=CharsetConvertMFC::UTF8ToGB18030(iterLocationInfo->GetID().c_str()).GetBuffer();
                //}
                //strProtocolContent+="\" ";

                strProtocolContent+="locationname=";
                strProtocolContent+="\"";
                if (iterLocationInfo->GetDescription() != "")
                {
                    strProtocolContent+=CharsetConvertMFC::UTF8ToGB18030(iterLocationInfo->GetDescription().c_str()).GetBuffer();
                }
                strProtocolContent+="\" ";

                strProtocolContent+="kerneltype=";
                strProtocolContent+="\"";
                if (iterLocationInfo->GetStreamType() != "")
                {
                    strProtocolContent+=CharsetConvertMFC::UTF8ToGB18030(iterLocationInfo->GetStreamType().c_str()).GetBuffer();
                }
                strProtocolContent+="\" ";

                strProtocolContent+=">\r\n";

                const std::vector<CSingleFileInfo> &vecSingleFileInfo=iterLocationInfo->GetVecFileInfo();
                std::vector<CSingleFileInfo>::const_iterator iterSingleFileInfo;
                for (iterSingleFileInfo=vecSingleFileInfo.begin();iterSingleFileInfo!=vecSingleFileInfo.end();
                    ++iterSingleFileInfo)
                {
                    strProtocolContent+="<url>";
                    if (iterSingleFileInfo->GetFileName() != "")
                    {
                        strProtocolContent+=CharsetConvertMFC::UTF8ToGB18030(iterSingleFileInfo->GetFileName().c_str()).GetBuffer();
                    }
                    strProtocolContent+="</url>\r\n";
                }

                strProtocolContent+="</view>\r\n";
            }
        }

        strProtocolContent+="</config>";

        TiXmlDocument xmlDocument;
        const char *p=xmlDocument.Parse(strProtocolContent.c_str(),NULL,TIXML_ENCODING_LEGACY);


        CStringW strTempDir=CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("path","tempdir").c_str());

        _wmkdir(strTempDir.GetBuffer());

        CStringW strWholePath=strTempDir+CStringW(L"\\")+CharsetConvertMFC::UTF8ToUTF16(jobInfo.GetJobID().c_str())+CStringW(L".playlist");

        char szWholePath[1024]={0};
        WcToMb(strWholePath.GetBuffer(),szWholePath,sizeof(szWholePath)/sizeof(char));

        if (xmlDocument.SaveFile(szWholePath))
        {
            LOG_INFO(("[CInterfaceProtocol::CreatePlayListFile] Create play list file success\r\n"));

            std::string strLocalIP=NetUtil::GetHostIP();

            LOG_INFO(("[CBurnControlDlg::CreatePlayListFile] Host ip : %s\r\n",strLocalIP.c_str()));

            CPlaylistFileInfo playlistFileInfo;
            playlistFileInfo.SetRemoteIP(strLocalIP);
            char szPort[8]={0};
            sprintf(szPort,"%d",FILEANYWHERE_PORT);
            playlistFileInfo.SetRemotePort(szPort);
            playlistFileInfo.SetDownloadURL(CharsetConvertMFC::UTF16ToUTF8(strWholePath).GetBuffer());
            jobInfo.SetPlaylistFileInfo(playlistFileInfo);
        }
        else
        {
            LOG_ERROR(("[CInterfaceProtocol::CreatePlayListFile] Create play list file failed\r\n"));
        }
}

void CBurnControlDlg::CreateAutorunFile(CRTBurnInfo &RTBurnInfo)
{
    CString strJobID=CharsetConvertMFC::UTF8ToUTF16(RTBurnInfo.GetJobID().c_str());

    CStringW strTempDir=CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("path","tempdir").c_str());

    _wmkdir(strTempDir.GetBuffer());

    CStringW strWholePath=strTempDir+CStringW(L"\\")+CStringW(L"AutoRun.ini");

    CString strCMD=L"Auth.exe";

    CStringW  strMd5;
    if (L"" == strMd5)
    {
        strMd5=L"d41d8cd98f00b204e9800998ecf8427e";
    } 
    strCMD+=L" ";
    strCMD+=strMd5;
    CString strIcon=L"PlayBack.exe";
//     CString strPlayBack=L"PlayBack.exe";
//     CString strPlayList=strJobID+L".playlist";

    CIni ini;
    ini.SetPathName(strWholePath);
    ini.WriteString(L"AutoRun",L"Icon",strIcon);
    ini.WriteString(L"AutoRun",L"open",strCMD);
//     ini.WriteString(L"AutoRun",L"playlistpath",strPlayList);
//     ini.WriteString(L"AutoRun",L"playbackpath",strPlayBack);

    CString strNewWholePath=strTempDir+CStringW(L"\\")+CStringW(L"AutoRun.inf");

    DeleteFileW(strNewWholePath);
    if (0 == MoveFile(strWholePath,strNewWholePath))
    {
        LOG_ERROR(("[CInterfaceProtocol::CreateAutorunFile] Ceate AutoRun.inf error\r\n"));
    }
    else
    {
        std::string strLocalIP=NetUtil::GetHostIP(); 

        RTBurnInfo.SetAutorunFileRemoteIP(strLocalIP);

        char szPort[8]={0};
        sprintf(szPort,"%d",FILEANYWHERE_PORT);
        RTBurnInfo.SetAutorunFileRemotePort(szPort);

        RTBurnInfo.SetAutorunFilePath(CharsetConvertMFC::UTF16ToUTF8(strNewWholePath).GetBuffer());
    }
}

void CBurnControlDlg::CreateAutorunFile(CNormalBurnJobInfoEx &jobInfo)
{
    CString strJobID=CharsetConvertMFC::UTF8ToUTF16(jobInfo.GetJobID().c_str());

    CStringW strTempDir=CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("path","tempdir").c_str());

    _wmkdir(strTempDir.GetBuffer());

    CStringW strWholePath=strTempDir+CStringW(L"\\")+CStringW(L"AutoRun.ini");
    CStringW strWholePathLinux=strTempDir+CStringW(L"\\")+CStringW(L"AutoRunLinux.ini");

    CString strCMD=L"Auth.exe";

    CStringW  strMd5;
    if (L"" == strMd5)
    {
        strMd5=L"d41d8cd98f00b204e9800998ecf8427e";
    } 
    strCMD+=L" ";
    strCMD+=strMd5;
    CString strIcon=L"PlayBack\\PlayBack.exe";
    CString strIconLinux=L"PlayBack.exe";
//     CString strPlayBack=L"PlayBack\\PlayBack.exe";
//     CString strPlayList=strJobID+"\\"+strJobID+L".playlist";

    CIni ini;
    ini.SetPathName(strWholePath);
    ini.WriteString(L"AutoRun",L"Icon",strIcon);
    ini.WriteString(L"AutoRun",L"open",strCMD);
//     ini.WriteString(L"AutoRun",L"playlistpath",strPlayList);
//     ini.WriteString(L"AutoRun",L"playbackpath",strPlayBack);

    CIni iniLinux;
    iniLinux.SetPathName(strWholePathLinux);
    iniLinux.WriteString(L"AutoRun",L"Icon",strIconLinux);
    iniLinux.WriteString(L"AutoRun",L"open",strCMD);

    CString strNewWholePath=strTempDir+CStringW(L"\\")+CStringW(L"AutoRun.inf");
    CString strNewWholePathLinux=strTempDir+CStringW(L"\\")+CStringW(L"AutoRunLinux.inf");

    DeleteFileW(strNewWholePath);
    DeleteFileW(strNewWholePathLinux);
    if (0 == MoveFile(strWholePath,strNewWholePath))
    {
        LOG_ERROR(("[CInterfaceProtocol::CreateAutorunFile] Ceate AutoRun.inf error\r\n"));
    }
    else
    {
        std::string strLocalIP=NetUtil::GetHostIP(); 

        jobInfo.SetAutorunFileRemoteIP(strLocalIP);

        char szPort[8]={0};
        sprintf(szPort,"%d",FILEANYWHERE_PORT);
        jobInfo.SetAutorunFileRemotePort(szPort);

        jobInfo.SetAutorunFilePath(CharsetConvertMFC::UTF16ToUTF8(strNewWholePath).GetBuffer());
        if (0 == MoveFile(strWholePathLinux,strNewWholePathLinux))
        {
            LOG_ERROR(("[CInterfaceProtocol::CreateAutorunFile] Ceate AutoRunLinux.inf error\r\n"));
        }
        else
        {
            jobInfo.SetAutorunFilePath(CharsetConvertMFC::UTF16ToUTF8(strNewWholePathLinux).GetBuffer());
        }
    }
}

void CBurnControlDlg::SendBurnJobToBurnServer()
{
    std::string strProtocolContentToBurnServer;

    std::string strBurnServerIP;
    std::string strBurnServerPort;

    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_COS:
        if (1 == m_normalJob.GetVecNormalBurnJobInfo().size())
        {
            const CNormalBurnJobInfoEx &jobInfo=m_normalJob.GetVecNormalBurnJobInfo().at(0);

            strProtocolContentToBurnServer=CInterfaceProtocol::GenerateProtocolStartBurnToBurnServer(jobInfo);
            strBurnServerIP=jobInfo.GetBurnServerInfo().GetIP();
            strBurnServerPort=jobInfo.GetBurnServerInfo().GetPort();
        }
        else
        {
            LOG_ERROR(("[CBurnControlDlg::SendBurnJobToBurnServer] Job count error\r\n"));
            assert(false);
        }
        break;

    case BURNCATEGORY_SINGLE:
        if (1 == m_normalJob.GetVecNormalBurnJobInfo().size())
        {
            const CNormalBurnJobInfoEx &jobInfo=m_normalJob.GetVecNormalBurnJobInfo().at(0);

            strProtocolContentToBurnServer=CInterfaceProtocol::GenerateProtocolStartBurnToBurnServer(jobInfo);
            strBurnServerIP=jobInfo.GetBurnServerInfo().GetIP();
            strBurnServerPort=jobInfo.GetBurnServerInfo().GetPort();
        }
        else
        {
            LOG_ERROR(("[CBurnControlDlg::SendBurnJobToBurnServer] Job count error\r\n"));
            assert(false);
        }
        break;

    case BURNCATEGORY_MULTI:
        //��������Ҫ���⴦��ÿ�η���һ�����񣬵���һ��������ɺ󣬲ŷ����µ�����
        {
            size_t nJobCount=m_normalJob.GetVecNormalBurnJobInfo().size();

            if (nJobCount > m_nJobIndex)
            {
                strProtocolContentToBurnServer=CInterfaceProtocol::GenerateProtocolStartBurnToBurnServer(m_normalJob.GetVecNormalBurnJobInfo().at(m_nJobIndex));
                strBurnServerIP=m_normalJob.GetVecNormalBurnJobInfo().at(m_nJobIndex).GetBurnServerInfo().GetIP();
                strBurnServerPort=m_normalJob.GetVecNormalBurnJobInfo().at(m_nJobIndex).GetBurnServerInfo().GetPort();
            }
            else
            {
                LOG_ERROR(("[CBurnControlDlg::SendBurnJobToBurnServer] Job count error\r\n"));
                assert(false);
            }
        }
        break;

    case BURNCATEGORY_RTBURN:
    case BURNCATEGORY_HM3:
        strProtocolContentToBurnServer=CInterfaceProtocol::GenerateProtocolStartRTBurnToBurnServer(m_RTBurnInfo);
        strBurnServerIP=m_RTBurnInfo.GetBurnServerInfo().GetIP();
        strBurnServerPort=m_RTBurnInfo.GetBurnServerInfo().GetPort();
        break;

    case BURNCATEGORY_UNKNOWN:
    default:
        assert(false);
    }

    if ( (strProtocolContentToBurnServer != "") &&
        (strBurnServerIP != "") &&
        (strBurnServerPort != "") )
    {
        //��BurnServer���Ϳ�¼Э��
        // 
        // 
        // 
        m_nBurnState=BURNCONTROL_STATE_READY;

        AsyncSendNCXMsg(strBurnServerIP,strBurnServerPort,strProtocolContentToBurnServer);

        ShowBurnControlUIUnable();

        /*std::string strResponse;
        strResponse=CInterfaceProtocol::InterfaceCommunicate(strBurnServerIP,atoi(strBurnServerPort.c_str()),
        strProtocolContentToBurnServer,BURNCONTROL_TAG_NAME);
        if (strResponse.length() > 0)
        {
        LOG_INFO(("[CBurnControlDlg::SendBurnJobToBurnServer] Send protocol success\r\n"));

        m_nBurnState=BURNCONTROL_STATE_RUNNING;

        UpdateBurnControlUI();

        SetTimer(ID_TIMER_SENDKEEPALIVEMSG,6000,NULL);
        }
        else
        {
        LOG_ERROR(("[CBurnControlDlg::SendBurnJobToBurnServer] Send protocol failed\r\n"));

        m_nBurnState=BURNCONTROL_STATE_IDLE;

        UpdateBurnControlUI();
        }*/
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::SendBurnJobToBurnServer] Protocol content is null or server address is not correct��\
                   send protocol failed\r\n"));
    }
}

void CBurnControlDlg::SendBurnControlCMDToBurnServer(std::string strCMD)
{
    std::string strCurJobID=GetCurJobID();
    std::string strBurnServerIP;
    int nBurnServerPort=0;
    GetBurnServerAddress(strBurnServerIP,nBurnServerPort);
    if ( (strCurJobID != "") &&
        (strBurnServerIP != "") &&
        (nBurnServerPort > 0) )
    {
        std::string strProtocol=CInterfaceProtocol::GenerateProtocolSendBurnCtrolCMDToBurnServer(
            strCurJobID,strCMD);

        AsyncSendNCXMsg(strBurnServerIP,IntToString(nBurnServerPort),strProtocol);

        //std::string strResponse;
        //strResponse=CInterfaceProtocol::InterfaceCommunicate(strBurnServerIP,nBurnServerPort,
        //    strProtocol,BURNCONTROL_TAG_NAME);

        //if(strResponse.length() > 0)
        //{

        //}
        //else
        //{

        //}
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::SendBurnControlCMDToBurnServer] Param error,job id %s,burn server ip %s,burn server port %d\r\n",
            strCurJobID.c_str(),strBurnServerIP.c_str(),nBurnServerPort));
    }
}

void CBurnControlDlg::SendCurBurnStateToUpper(std::string strIP,std::string strPort,std::string strJobID,std::string strBurnState,std::string strBurnStateDescription)
{
    if (("" == strIP) ||
        ("" == strPort))
    {
        if (strJobID == m_RTBurnInfo.GetJobID())
        {
            strIP=m_RTBurnInfo.GetUpServerIP();
            strPort=m_RTBurnInfo.GetUpServerPort();
        }
        else
        {
            size_t N=m_normalJob.GetVecNormalBurnJobInfo().size();
            for (size_t i=0;i<N;++i)
            {
                if (m_normalJob.GetVecNormalBurnJobInfo().at(i).GetJobID() == strJobID)
                {
                    strIP=m_normalJob.GetVecNormalBurnJobInfo().at(i).GetUpServerIP();
                    strPort=m_normalJob.GetVecNormalBurnJobInfo().at(i).GetUpServerPort();

                    break;
                }
            }
        }
    }

    if ("" == strJobID ||
        "" == strIP ||
        "" == strPort ||
        "" == strBurnState)
    {
        return;
    }

    std::string strProtocolContent;
    strProtocolContent+="<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n";
    strProtocolContent+="<burnControl>\r\n";

    strProtocolContent+="<stateFlag>";
    strProtocolContent+=strBurnState.c_str();
    strProtocolContent+="</stateFlag>\r\n";

    strProtocolContent+="<stateDescription>";
    strProtocolContent+=strBurnStateDescription.c_str();
    strProtocolContent+="</stateDescription>\r\n";

    strProtocolContent+="<jobID>";
    strProtocolContent+=strJobID.c_str();
    strProtocolContent+="</jobID>\r\n";

    strProtocolContent+="<commandType>sendRTBurnState</commandType>\r\n";

    strProtocolContent+="</burnControl>\r\n";

    THREAD_PARAM *pThreadParam=new THREAD_PARAM;
    if (pThreadParam != NULL)
    {
        int nContentLen=strProtocolContent.length();
        pThreadParam->pThreadData=new char[nContentLen+1];
        if (pThreadParam->pThreadData != NULL)
        {
            strcpy((char *)(pThreadParam->pThreadData),strProtocolContent.c_str());

            pThreadParam->pUserParam=this;

            pThreadParam->p1=new char[128];
            if (pThreadParam->p1 != NULL)
            {
                strcpy((char *)(pThreadParam->p1),strIP.c_str());

                pThreadParam->p2=new char[128];
                if (pThreadParam->p2 != NULL)
                {
                    strcpy((char *)(pThreadParam->p2),strPort.c_str());

                    HANDLE hThread=CreateThread(NULL,0,SendJobStateThread,pThreadParam,0,NULL);
                    if (hThread != NULL)
                    {
                        CloseHandle(hThread);

                        return;
                    }

                    delete [] (char *)(pThreadParam->p2);
                    pThreadParam->p2=NULL;
                }

                delete [] (char *)(pThreadParam->p1);
                pThreadParam->p1=NULL;
            }

            delete [] (char *)(pThreadParam->pThreadData);
            pThreadParam->pThreadData=NULL;
        }

        delete pThreadParam;
        pThreadParam=NULL;
    }
}


//������mapBurnFileInfo��keyΪ����id��valueΪ������Ҫ��¼���ļ��б�
bool CBurnControlDlg::GetBurnFilesState(const std::vector<CCDROMDriverInfo> &vecDevInfo,
                                       std::map< int,std::vector<CSingleFileInfo> > &mapBurnFileInfo)
{
    mapBurnFileInfo.clear();

    std::vector<CCDROMDriverInfo>::const_iterator iterDevInfo;
    for (iterDevInfo=vecDevInfo.begin();iterDevInfo!=vecDevInfo.end();++iterDevInfo)
    {
        std::string strDevID=iterDevInfo->GetID();
        std::vector<CSingleFileInfo> vecSingleFileInfo;

        const std::vector<CLocationInfo> &vecLocationInfo=iterDevInfo->GetVecLocationInfo();
        std::vector<CLocationInfo>::const_iterator iterLocationInfo;
        for (iterLocationInfo=vecLocationInfo.begin();iterLocationInfo!=vecLocationInfo.end();
            ++iterLocationInfo)
        {
            const std::vector<CSingleFileInfo> &vecFileInfo=iterLocationInfo->GetVecFileInfo();
            std::vector<CSingleFileInfo>::const_iterator iterFileInfo;
            for (iterFileInfo=vecFileInfo.begin();iterFileInfo!=vecFileInfo.end();++iterFileInfo)
            {
                CSingleFileInfo fileInfo;
                fileInfo.SetFileName(iterFileInfo->GetFileName());
                fileInfo.SetSize(iterFileInfo->GetSize());
                fileInfo.SetBurnState(iterFileInfo->GetBurnState());
                fileInfo.SetPercent(iterFileInfo->GetPercent());

                vecSingleFileInfo.push_back(fileInfo);
            }
        }

        int nDevID=-1;
        if (strDevID != "")
        {
            nDevID=atoi(strDevID.c_str());
        }

        if(nDevID >= 0)
        {
            mapBurnFileInfo[nDevID]=vecSingleFileInfo;

        }
    }

    return !mapBurnFileInfo.empty();
}

void CBurnControlDlg::UpdateCDROMState(const std::vector<CCDROMDriverInfo> &vecDevInfo)
{
    CListCtrl *pListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_BURNSTATE));
    if (pListCtrl != NULL &&
        pListCtrl->GetSafeHwnd() != NULL &&
        !vecDevInfo.empty())
    {
        CStringW strName=CharsetConvertMFC::UTF8ToUTF16(m_cfgFile.GetValue("uiword","cdromdrivername").c_str());

        int nItemCount=pListCtrl->GetItemCount();
        int nColumnCount=pListCtrl->GetHeaderCtrl()->GetItemCount();

        if (2*vecDevInfo.size() == static_cast<size_t>(nItemCount) &&
            2 == nColumnCount)
        {
            std::vector<CCDROMDriverInfo>::const_iterator iter;
            int nIndex=0;
            for (iter=vecDevInfo.begin();iter!=vecDevInfo.end();++iter,++nIndex)
            {
                pListCtrl->SetItemText(2*nIndex,0,
                    strName+CStringW(L" ")+CharsetConvertMFC::UTF8ToUTF16(CStringA(iter->GetID().c_str())));

                CStringW str=CharsetConvertMFC::UTF8ToUTF16(iter->GetStateDescription().c_str());
                if (str != L"")
                {
                    pListCtrl->SetItemText(2*nIndex+1,1,str);
                }
            }
        }
        else
        {
            //��������״̬�б�
            pListCtrl->SetExtendedStyle(pListCtrl->GetExtendedStyle() & (~LVS_EX_GRIDLINES ));
            pListCtrl->SetExtendedStyle(pListCtrl->GetExtendedStyle() | LVS_EX_FULLROWSELECT );

            pListCtrl->DeleteAllItems();

            while(pListCtrl->GetHeaderCtrl()->GetItemCount() > 0)
            {
                pListCtrl->DeleteColumn(0);
            }

            CRect rect;
            pListCtrl->GetClientRect(&rect);
            pListCtrl->InsertColumn(0,L"",LVCFMT_LEFT,rect.Width()/10);
            pListCtrl->InsertColumn(1,L"",LVCFMT_LEFT,rect.Width()*9/10-20);

            std::vector<CCDROMDriverInfo>::const_iterator iter;
            int nIndex=0;
            for (iter=vecDevInfo.begin();iter!=vecDevInfo.end();++iter,++nIndex)
            {
                pListCtrl->InsertItem(2*nIndex,strName+CString(L" ")+CharsetConvertMFC::UTF8ToUTF16(iter->GetID().c_str()));

                pListCtrl->InsertItem(2*nIndex+1,L"");

                CStringW str=CharsetConvertMFC::UTF8ToUTF16(CStringA(iter->GetStateDescription().c_str()));
                if (str != L"")
                {
                    pListCtrl->SetItemText(2*nIndex+1,1,str);
                }
            }
        }
    }

    UpdateBurnFilesState(vecDevInfo);
}

void CBurnControlDlg::UpdateBurnFilesState(const std::vector<CCDROMDriverInfo> &vecDevInfo)
{
    std::map< int,std::vector<CSingleFileInfo> > mapBurnFileInfo;
    if (!GetBurnFilesState(vecDevInfo,mapBurnFileInfo))
    {
        return;
    }

    if (mapBurnFileInfo.empty())
    {
        LOG_ERROR(("[CBurnControlDlg::UpdateBurnFilesState] Find no buring file,%d\r\n",__LINE__));

        return;
    }

    static int n=0;
    static std::map< int,std::vector<CSingleFileInfo> > mapLastBurnFileInfo=mapBurnFileInfo;

    bool bChanged=false;
    if (mapLastBurnFileInfo.size() == mapBurnFileInfo.size())
    {
        std::map< int,std::vector<CSingleFileInfo> >::const_iterator iterLast,iterCur;
        for (iterCur=mapBurnFileInfo.begin(),iterLast=mapLastBurnFileInfo.begin();
            iterCur!=mapBurnFileInfo.end() && iterLast!=mapLastBurnFileInfo.end();
            ++iterCur,++iterLast)
        {
            if (iterCur->first != iterLast->first)
            {
                break;
            }
            else
            {
                const std::vector<CSingleFileInfo> &vecFileInfoCur=iterCur->second;
                const std::vector<CSingleFileInfo> &vecFileInfoLast=iterLast->second;
                if (vecFileInfoCur.size() != vecFileInfoLast.size())
                {
                    break;
                }
                else
                {
                    std::string strFileName;
                    std::string strFileSize;

                    size_t nFileCount=vecFileInfoCur.size();
                    for (size_t i=0;i<nFileCount;++i)
                    {
                        strFileName=vecFileInfoCur.at(i).GetFileName();
                        strFileSize=vecFileInfoCur.at(i).GetSize();

                        if (vecFileInfoLast.at(i).GetFileName() != strFileName ||
                            vecFileInfoLast.at(i).GetSize() != strFileSize)
                        {
                            bChanged=true;

                            break;
                        }
                    }
                }
            }
        }

        if (iterCur != mapBurnFileInfo.end() ||
            iterLast != mapLastBurnFileInfo.end())
        {
            bChanged=true;
        }
    }
    else
    {
        bChanged=true;
    }


    CListCtrl *pListCtrl=(CListCtrl *)(GetDlgItem(IDC_LIST_FILEINFO));
    if (pListCtrl != NULL &&
        pListCtrl->GetSafeHwnd() != NULL)
    {
        if (bChanged || 0 == n)//��Ҫ���²�������
        {
            pListCtrl->SetExtendedStyle(pListCtrl->GetExtendedStyle() & (~LVS_EX_GRIDLINES ));
            pListCtrl->SetExtendedStyle(pListCtrl->GetExtendedStyle() | LVS_EX_FULLROWSELECT );

            pListCtrl->DeleteAllItems();

            while(pListCtrl->GetHeaderCtrl()->GetItemCount() > 0)
            {
                pListCtrl->DeleteColumn(0);
            }


            CRect rect;
            pListCtrl->GetClientRect(rect);
            pListCtrl->InsertColumn(0,_T(""),LVCFMT_CENTER,rect.Width()/2);
            pListCtrl->InsertColumn(1,_T(""),LVCFMT_CENTER,rect.Width()/4);
            pListCtrl->InsertColumn(2,_T(""),LVCFMT_CENTER,rect.Width()/4-20);

            int nIndex=0;
            std::map< int,std::vector<CSingleFileInfo> >::iterator iter;
            for (iter=mapBurnFileInfo.begin();iter!=mapBurnFileInfo.end();++iter)
            {
                CStringW strDevID;
                strDevID.Format(L"%d",iter->first);

                int nDevItemIndex=nIndex;

                CStringW strDevName=CharsetConvertMFC::UTF8ToUTF16(
                    m_cfgFile.GetValue("uiword","cdromdrivername").c_str())+CStringW(L" ")+strDevID;
                pListCtrl->InsertItem(nDevItemIndex,strDevName);

                ++nIndex;

                double dTotalSize=0;
                std::vector<CSingleFileInfo> &vecCSingleFileInfo=iter->second;
                std::vector<CSingleFileInfo>::iterator iterFileInfo;
                for (iterFileInfo=vecCSingleFileInfo.begin();iterFileInfo!=vecCSingleFileInfo.end();++iterFileInfo)
                {
                    std::string strFileName=iterFileInfo->GetFileName();
                    std::string strFileSize=iterFileInfo->GetSize();
                    std::string strState=iterFileInfo->GetBurnState();

                    pListCtrl->InsertItem(nIndex,CharsetConvertMFC::UTF8ToUTF16(strFileName.c_str()));

                    double dFileSize=StringToInt64(strFileSize)/1024.0/1024.0;
                    CStringW str;
                    str.Format(L"%.2lf M",dFileSize);
                    pListCtrl->SetItemText(nIndex,1,str);

                    LOG_INFO(("[CBurnControlDlg::UpdateBurnFilesState] **** Item index : %d,file size : %.2lf,%d\r\n",
                        nIndex,dFileSize,__LINE__));

                    ++nIndex;

                    dTotalSize+=dFileSize;
                }

                CStringW str;
                str.Format(L"%.2lf M",dTotalSize);
                pListCtrl->SetItemText(nDevItemIndex,1,str);

                LOG_INFO(("[CBurnControlDlg::UpdateBurnFilesState] Total size : %.2lf\r\n",
                    dTotalSize));

                pListCtrl->InsertItem(nIndex,_T(""));

                ++nIndex;//ÿ���������ļ�֮���໥����
            }
        }
        else
        {
            int nIndex=0;

            std::map< int,std::vector<CSingleFileInfo> >::iterator iter;
            for (iter=mapBurnFileInfo.begin();iter!=mapBurnFileInfo.end();++iter)
            {
                int nDevItemIndex=nIndex;

                ++nIndex;

                std::vector<CSingleFileInfo> &vecCSingleFileInfo=iter->second;
                std::vector<CSingleFileInfo>::iterator iterFileInfo;
                for (iterFileInfo=vecCSingleFileInfo.begin();iterFileInfo!=vecCSingleFileInfo.end();++iterFileInfo)
                {
                    std::string strState=iterFileInfo->GetBurnState();

                    if ("0" == strState)
                    {
                        //��ʾ�ȴ���¼��ͼ��
                        //pListCtrl->SetItemText(nIndex,2,CString(_T("�ȴ���¼")));
                    }
                    else if ("1" == strState)
                    {
                        //��ʾ���ڿ�¼��ͼ��
                        //pListCtrl->SetItemText(nIndex,2,CString(_T("���ڿ�¼")));
                    }
                    else if ("2" == strState)
                    {
                        //��ʾ��¼��ɵ�ͼ��
                        //pListCtrl->SetItemText(nIndex,2,CString(_T("��¼���")));
                    }

                    ++nIndex;
                }

                ++nIndex;
            }
        }
    }

    mapLastBurnFileInfo=mapBurnFileInfo;
    ++n;
}

void CBurnControlDlg::EnableBurnContent(bool b)
{
    for (UINT i=IDC_CHECK_L1;i<IDC_CHECK_L1+LOCATION_MAX_COUNT;++i)
    {
        CWnd *p=GetDlgItem(i);
        if (p != NULL &&
            p->GetSafeHwnd() != NULL)
        {
            if (b && ((CButton*)p)->GetCheck())
            {
                p->EnableWindow(b);
            }
            else if (!b)
            {
                p->EnableWindow(b);
            }
        }
    }
}

void CBurnControlDlg::EnableBurnParam(bool bEnable)
{
    CWnd *p=GetDlgItem(IDC_COMBO_RECORDER);
    if (p != NULL)
    {
        if (p->GetSafeHwnd() != NULL)
        {
            p->EnableWindow(bEnable);
        }
    }

    p=GetDlgItem(IDC_EDIT_RECORDERINFO);
    if (p != NULL)
    {
        if (p->GetSafeHwnd() != NULL)
        {
            p->EnableWindow(bEnable);
        }
    }

    CBurnServerInfo curBurnServerInfo;
    if (GetCurBurnServerSel(curBurnServerInfo))
    {
        TRACE("[CBurnControlDlg::EnableBurnParam] %lu,%d\r\n",
            GetTickCount(),__LINE__);

        if (curBurnServerInfo.GetBurnServerType() != BURNSERVER_TYPE_NORMAL)
        {
            TRACE("[CBurnControlDlg::EnableBurnParam] %lu,%d\r\n",
                GetTickCount(),__LINE__);
        }
        else
        {
            TRACE("[CBurnControlDlg::EnableBurnParam] %lu,%d\r\n",
                GetTickCount(),__LINE__);

            GetDlgItem(IDC_RADIO_SINGLE)->EnableWindow(bEnable);
            GetDlgItem(IDC_RADIO_MULTISYN)->EnableWindow(bEnable);
            GetDlgItem(IDC_RADIO_MULTIASYN)->EnableWindow(bEnable);

            if (GetDevUpdated())
            {
                TRACE("[CBurnControlDlg::EnableBurnParam] %lu,%d\r\n",
                    GetTickCount(),__LINE__);

                if (bEnable)
                {
                    if (1 == GetCurCDROMDriverInfoVec().size())
                    {
                        LOG_INFO(("[CBurnControlDlg::EnableBurnParam] %d\r\n",__LINE__));

                        GetDlgItem(IDC_RADIO_MULTISYN)->EnableWindow(FALSE);
                        GetDlgItem(IDC_RADIO_MULTIASYN)->EnableWindow(FALSE);
                    }
                }
            }

            TRACE("[CBurnControlDlg::EnableBurnParam] %lu,%d\r\n",
                GetTickCount(),__LINE__);

            GetDlgItem(IDC_LIST_SELECTCDROM)->EnableWindow(bEnable);

            DWORD nColor;
            if (bEnable)
            {
                nColor=GetSysColor(COLOR_WINDOWTEXT);
            }
            else
            {
                nColor=GetSysColor(COLOR_GRAYTEXT);
            }
            ((CListCtrl *)GetDlgItem(IDC_LIST_SELECTCDROM))->SetTextColor(nColor);

            TRACE("[CBurnControlDlg::EnableBurnParam] %lu,%d\r\n",
                GetTickCount(),__LINE__);
        }
    }

    TRACE("[CBurnControlDlg::EnableBurnParam] %lu,%d\r\n",
        GetTickCount(),__LINE__);
}

void CBurnControlDlg::EnableBurnPassword(bool b)
{
    if (b)
    {
        BOOL bBurnPassword=(BST_CHECKED == ((CButton *)GetDlgItem(IDC_CHECK_BURNPASSWORD))->GetCheck());
        BOOL bContentPassword=(BST_CHECKED == ((CButton *)GetDlgItem(IDC_CHECK_CONTENTPASSWORD))->GetCheck());
        DisplayPasswordUI(bBurnPassword,bContentPassword);
    } 
    else
    {
        ((CButton *)GetDlgItem(IDC_CHECK_BURNPASSWORD))->EnableWindow(FALSE);
        ((CButton *)GetDlgItem(IDC_CHECK_CONTENTPASSWORD))->EnableWindow(FALSE);
        ((CEdit *)GetDlgItem(IDC_EDIT_INPUTPASSWORD))->EnableWindow(FALSE);
        ((CEdit *)GetDlgItem(IDC_EDIT_CONFIRMPASSWORD))->EnableWindow(FALSE);
    }
}

void CBurnControlDlg::EnableBurnCDName(bool b)
{
    CWnd *p=GetDlgItem(IDC_EDIT_CDNAME);
    if (p != NULL)
    {
        if (p->GetSafeHwnd() != NULL)
        {
            if (p->IsWindowVisible())
            {
                p->EnableWindow(b);
            }
        }
    }
}

void CBurnControlDlg::EnableBurnJobList(bool b)
{
    if (m_btnstJobUp.IsWindowVisible())
    {
        m_btnstJobUp.EnableWindow(b);
    }

    if (m_btnstJobDown.IsWindowVisible())
    {
        m_btnstJobDown.EnableWindow(b);
    }

    if (b)
    {
        m_editItem.ShowWindow(SW_SHOW);
    }
    else
    {
        m_editItem.ShowWindow(SW_HIDE);
    }
}

void CBurnControlDlg::UpdateBurnControlUI()
{
    switch (m_nBurnState)
    {
    case BURNCONTROL_STATE_ERROR:
    case BURNCONTROL_STATE_IDLE:
    case BURNCONTROL_STATE_READY:
        ShowBurnControlUIIdle();
        break;

    case BURNCONTROL_STATE_RUNNING:
        ShowBurnControlUIBurning();
        break;

    case BURNCONTROL_STATE_PAUSED:
        ShowBurnControlUIPause();
        break;

    default:
        break;
    }
}

void CBurnControlDlg::ShowBurnControlUIIdle()
{
    TRACE("[CBurnControlDlg::ShowBurnControlUIIdle] Begin,%lu,%d\r\n",
        GetTickCount(),__LINE__);

    std::string strResPath;

    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_HM3:
        {
//             EnableBurnContent(true);

            EnableBurnParam(true);

            EnableBurnPassword(true);

            strResPath=m_cfgFile.GetValue("path","save2");
            if (strResPath != "")
            {
                m_btnstSave.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstSave.DrawBorder(FALSE);
            }
            m_btnstSave.EnableWindow(FALSE);

            strResPath=m_cfgFile.GetValue("path","startburn");
            if (strResPath != "")
            {
                m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstStartBurn.DrawBorder(FALSE);
            }
            m_btnstStartBurn.EnableWindow(TRUE);


            strResPath=m_cfgFile.GetValue("path","pause2");
            if (strResPath != "")
            {
                m_btnstPause.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstPause.DrawBorder(FALSE);
            }
            m_btnstPause.EnableWindow(FALSE);

            strResPath=m_cfgFile.GetValue("path","close");
            if (strResPath != "")
            {
                m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstClose.DrawBorder(FALSE);
            }
            m_btnstClose.EnableWindow(TRUE);
        }
        break;

    case BURNCATEGORY_RTBURN:
        {
            EnableBurnContent(true);

            EnableBurnParam(true);

            EnableBurnPassword(true);


            std::string strJobType=m_RTBurnInfo.GetJobType();
            if ("2" == strJobType)
            {
                strResPath=m_cfgFile.GetValue("path","save");
                if (strResPath != "")
                {
                    m_btnstSave.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstSave.DrawBorder(FALSE);
                }
                m_btnstSave.EnableWindow(TRUE);

                strResPath=m_cfgFile.GetValue("path","startburn2");
                if (strResPath != "")
                {
                    m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstStartBurn.DrawBorder(FALSE);
                }
                m_btnstStartBurn.EnableWindow(FALSE);

                strResPath=m_cfgFile.GetValue("path","pause2");
                if (strResPath != "")
                {
                    m_btnstPause.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstPause.DrawBorder(FALSE);
                }
                m_btnstPause.EnableWindow(FALSE);

                strResPath=m_cfgFile.GetValue("path","close");
                if (strResPath != "")
                {
                    m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstClose.DrawBorder(FALSE);
                }
                m_btnstClose.EnableWindow(TRUE);
            }
            else
            {
                if (strJobType != "1")
                {
                    LOG_WARNING(("[CBurnControlDlg::ShowBurnControlUIIdle] Job type : %s\r\n",strJobType.c_str()));
                }

                strResPath=m_cfgFile.GetValue("path","save2");
                if (strResPath != "")
                {
                    m_btnstSave.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstSave.DrawBorder(FALSE);
                }
                m_btnstSave.EnableWindow(FALSE);

                strResPath=m_cfgFile.GetValue("path","startburn");
                if (strResPath != "")
                {
                    m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstStartBurn.DrawBorder(FALSE);
                }
                m_btnstStartBurn.EnableWindow(TRUE);

                strResPath=m_cfgFile.GetValue("path","pause2");
                if (strResPath != "")
                {
                    m_btnstPause.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstPause.DrawBorder(FALSE);
                }
                m_btnstPause.EnableWindow(FALSE);

                strResPath=m_cfgFile.GetValue("path","close");
                if (strResPath != "")
                {
                    m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                    m_btnstClose.DrawBorder(FALSE);
                }
                m_btnstClose.EnableWindow(TRUE);
            }
        }

        break;

    case BURNCATEGORY_COS:
        {
            EnableBurnCDName(true);

            EnableBurnContent(true);

            EnableBurnParam(true);

            EnableBurnPassword(true);

            strResPath=m_cfgFile.GetValue("path","startburn");
            if (strResPath != "")
            {
                m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstStartBurn.DrawBorder(FALSE);
            }
            m_btnstStartBurn.EnableWindow(TRUE);

            strResPath=m_cfgFile.GetValue("path","close");
            if (strResPath != "")
            {
                m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstClose.DrawBorder(FALSE);
            }
            m_btnstClose.EnableWindow(TRUE);
        }
        break;

    case BURNCATEGORY_SINGLE:
        {
            EnableBurnCDName(true);

            EnableBurnParam(true);

            EnableBurnPassword(true);

            strResPath=m_cfgFile.GetValue("path","startburn");
            if (strResPath != "")
            {
                m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstStartBurn.DrawBorder(FALSE);
            }
            m_btnstStartBurn.EnableWindow(TRUE);

            strResPath=m_cfgFile.GetValue("path","close");
            if (strResPath != "")
            {
                m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstClose.DrawBorder(FALSE);
            }
            m_btnstClose.EnableWindow(TRUE);
        }
        break;

    case BURNCATEGORY_MULTI:
        {
            EnableBurnJobList(true);

            EnableBurnParam(true);

            EnableBurnPassword(true);

            strResPath=m_cfgFile.GetValue("path","startburn");
            if (strResPath != "")
            {
                m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstStartBurn.DrawBorder(FALSE);
            }
            m_btnstStartBurn.EnableWindow(TRUE);

            strResPath=m_cfgFile.GetValue("path","close");
            if (strResPath != "")
            {
                m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstClose.DrawBorder(FALSE);
            }
            m_btnstClose.EnableWindow(TRUE);
        }
        break;
    }

    TRACE("[CBurnControlDlg::ShowBurnControlUIIdle] End,%lu,%d\r\n",
        GetTickCount(),__LINE__);
}

void CBurnControlDlg::ShowBurnControlUIBurning()
{
    TRACE("[CBurnControlDlg::ShowBurnControlUIBurning] Begin,%lu,%d\r\n",
        GetTickCount(),__LINE__);

    std::string strResPath;

    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_HM3:
        {
            EnableBurnContent(false);

            TRACE("------------------,%lu,%d\r\n",GetTickCount(),__LINE__);

            EnableBurnParam(false);

            TRACE("------------------,%lu,%d\r\n",GetTickCount(),__LINE__);

            EnableBurnPassword(false);

            TRACE("------------------,%lu,%d\r\n",GetTickCount(),__LINE__);

            strResPath=m_cfgFile.GetValue("path","save2");
            if (strResPath != "")
            {
                m_btnstSave.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstSave.DrawBorder(FALSE);
            }
            m_btnstSave.EnableWindow(FALSE);

            TRACE("------------------,%lu,%d\r\n",GetTickCount(),__LINE__);

            strResPath=m_cfgFile.GetValue("path","stopburn");
            if (strResPath != "")
            {
                m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstStartBurn.DrawBorder(FALSE);
            }
            m_btnstStartBurn.EnableWindow(TRUE);

            TRACE("------------------,%lu,%d\r\n",GetTickCount(),__LINE__);

            strResPath=m_cfgFile.GetValue("path","pause");
            if (strResPath != "")
            {
                m_btnstPause.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstPause.DrawBorder(FALSE);
            }
            m_btnstPause.EnableWindow(TRUE);

            TRACE("------------------,%lu,%d\r\n",GetTickCount(),__LINE__);

            strResPath=m_cfgFile.GetValue("path","close2");
            if (strResPath != "")
            {
                m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstClose.DrawBorder(FALSE);
            }
            m_btnstClose.EnableWindow(FALSE);
        }
        break;

    case BURNCATEGORY_RTBURN:
        {
            EnableBurnContent(false);

            EnableBurnParam(false);

            EnableBurnPassword(false);

            strResPath=m_cfgFile.GetValue("path","save2");
            if (strResPath != "")
            {
                m_btnstSave.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstSave.DrawBorder(FALSE);
            }
            m_btnstSave.EnableWindow(FALSE);

            strResPath=m_cfgFile.GetValue("path","stopburn");
            if (strResPath != "")
            {
                m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstStartBurn.DrawBorder(FALSE);
            }
            m_btnstStartBurn.EnableWindow(TRUE);


            strResPath=m_cfgFile.GetValue("path","pause");
            if (strResPath != "")
            {
                m_btnstPause.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstPause.DrawBorder(FALSE);
            }
            m_btnstPause.EnableWindow(TRUE);

            strResPath=m_cfgFile.GetValue("path","close2");
            if (strResPath != "")
            {
                m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstClose.DrawBorder(FALSE);
            }
            m_btnstClose.EnableWindow(FALSE);
        }

        break;

    case BURNCATEGORY_COS:
        {
            EnableBurnCDName(false);

            EnableBurnContent(false);

            EnableBurnParam(false);

            EnableBurnPassword(false);

            strResPath=m_cfgFile.GetValue("path","startburn2");
            if (strResPath != "")
            {
                m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstStartBurn.DrawBorder(FALSE);
            }
            m_btnstStartBurn.EnableWindow(FALSE);

            strResPath=m_cfgFile.GetValue("path","close2");
            if (strResPath != "")
            {
                m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstClose.DrawBorder(FALSE);
            }
            m_btnstClose.EnableWindow(FALSE);
        }
        break;

    case BURNCATEGORY_SINGLE:
        {
            EnableBurnCDName(false);

            EnableBurnParam(false);

            EnableBurnPassword(false);

            strResPath=m_cfgFile.GetValue("path","startburn2");
            if (strResPath != "")
            {
                m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstStartBurn.DrawBorder(FALSE);
            }
            m_btnstStartBurn.EnableWindow(FALSE);

            strResPath=m_cfgFile.GetValue("path","close2");
            if (strResPath != "")
            {
                m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstClose.DrawBorder(FALSE);
            }
            m_btnstClose.EnableWindow(FALSE);
        }
        break;

    case BURNCATEGORY_MULTI:
        {
            EnableBurnJobList(false);

            EnableBurnParam(false);

            EnableBurnPassword(false);

            strResPath=m_cfgFile.GetValue("path","startburn2");
            if (strResPath != "")
            {
                m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstStartBurn.DrawBorder(FALSE);
            }
            m_btnstStartBurn.EnableWindow(FALSE);

            strResPath=m_cfgFile.GetValue("path","close2");
            if (strResPath != "")
            {
                m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstClose.DrawBorder(FALSE);
            }
            m_btnstClose.EnableWindow(FALSE);
        }
        break;
    }

    TRACE("[CBurnControlDlg::ShowBurnControlUIBurning] End,%lu,%d\r\n",
        GetTickCount(),__LINE__);
}

void CBurnControlDlg::ShowBurnControlUIPause()
{
    std::string strResPath;

    switch (m_eBurnCategory)
    {
    case BURNCATEGORY_HM3:
    case BURNCATEGORY_RTBURN:
        {
            EnableBurnContent(false);

            EnableBurnParam(false);

            EnableBurnPassword(false);

            strResPath=m_cfgFile.GetValue("path","save2");
            if (strResPath != "")
            {
                m_btnstSave.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstSave.DrawBorder(FALSE);
            }
            m_btnstSave.EnableWindow(FALSE);

            strResPath=m_cfgFile.GetValue("path","stopburn");
            if (strResPath != "")
            {
                m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstStartBurn.DrawBorder(FALSE);
            }
            m_btnstStartBurn.EnableWindow(TRUE);


            strResPath=m_cfgFile.GetValue("path","resume");
            if (strResPath != "")
            {
                m_btnstPause.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstPause.DrawBorder(FALSE);
            }
            m_btnstPause.EnableWindow(TRUE);

            strResPath=m_cfgFile.GetValue("path","close2");
            if (strResPath != "")
            {
                m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
                m_btnstClose.DrawBorder(FALSE);
            }
            m_btnstClose.EnableWindow(FALSE);
        }
        break;

    case BURNCATEGORY_COS:
        assert(false);
        break;

    case BURNCATEGORY_SINGLE:
        assert(false);
        break;

    case BURNCATEGORY_MULTI:
        assert(false);
        break;
    }
}

void CBurnControlDlg::ShowBurnControlUIUnable()
{
    std::string strResPath;

    strResPath=m_cfgFile.GetValue("path","save2");
    if (strResPath != "")
    {
        m_btnstSave.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
        m_btnstSave.DrawBorder(FALSE);
    }
    m_btnstSave.EnableWindow(FALSE);

    strResPath=m_cfgFile.GetValue("path","stopburn2");
    if (strResPath != "")
    {
        m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
        m_btnstStartBurn.DrawBorder(FALSE);
    }
    m_btnstStartBurn.EnableWindow(FALSE);

    if ( (BURNCATEGORY_COS == m_eBurnCategory) ||
        (BURNCATEGORY_SINGLE == m_eBurnCategory) ||
        (BURNCATEGORY_MULTI == m_eBurnCategory) )
    {
        strResPath=m_cfgFile.GetValue("path","startburn2");
        if (strResPath != "")
        {
            m_btnstStartBurn.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
            m_btnstStartBurn.DrawBorder(FALSE);
        }
        m_btnstStartBurn.EnableWindow(FALSE);
    }


    strResPath=m_cfgFile.GetValue("path","pause2");
    if (strResPath != "")
    {
        m_btnstPause.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
        m_btnstPause.DrawBorder(FALSE);
    }
    m_btnstPause.EnableWindow(FALSE);

    strResPath=m_cfgFile.GetValue("path","close2");
    if (strResPath != "")
    {
        m_btnstClose.SetBitmaps(CharsetConvertMFC::UTF8ToUTF16(strResPath.c_str()),RGB(255,255,255));
        m_btnstClose.DrawBorder(FALSE);
    }
    m_btnstClose.EnableWindow(FALSE);
}

void CBurnControlDlg::SetTopBorderHeight(int nHeight)
{
    m_nBkTop=nHeight;
}

void CBurnControlDlg::SetBottomBorderHeight(int nHeight)
{
    m_nBkBottom=nHeight;
}

void CBurnControlDlg::SetTitleText(CString strTitleText)
{
    CSize size=GetDC()->GetTextExtent(strTitleText);

    CRect rect;
    m_staticBurnPic.GetWindowRect(&rect);

    ScreenToClient(&rect);

    GetDlgItem(IDC_STATIC_TITLE)->MoveWindow(rect.right+4,
        (m_nBkTop-size.cy)*2/3,size.cx,size.cy);

    GetDlgItem(IDC_STATIC_TITLE)->SetWindowText(strTitleText);
}

void CBurnControlDlg::StartRtBurn(bool bToServer)
{
    if (BURNCATEGORY_RTBURN == m_eBurnCategory)
    {
        if (bToServer)
        {
            int nCount=0;
            while (true)
            {
                SaveBurnServerInfo();

                int nDevCount=m_RTBurnInfo.GetBurnServerInfo().GetVecCDROMDriverInfo().size();
                if (nDevCount > 0)
                {
                    break;
                }
                else
                {
                    if (nCount > 4)
                    {
                        return;
                    }

                    ++nCount;

                    Sleep(2000);
                }
            }

            CreateAdditionalBurnFile();
            SendBurnJobToBurnServer();
        }
        else
        {
            //ʵʱ��¼�����¼��ťʱ�����ϲ㷢�͵�ǰ��¼������Ϣ���Լ�׼���ÿ�¼�ı��

            SaveRTBurnCfg();

            std::string strProtocolContentToUpper=CInterfaceProtocol::GenerateProtocolSendRTBurnInfoToUpper(
                m_RTBurnInfo);

            std::string strUpServerIP=m_RTBurnInfo.GetUpServerIP();
            int nUpServerPort=0;
            if (m_RTBurnInfo.GetUpServerPort() != "")
            {
                nUpServerPort=atoi(m_RTBurnInfo.GetUpServerPort().c_str());
            }

            if (strUpServerIP != "" &&
                nUpServerPort != 0)
            {
                std::string strResponse=CInterfaceProtocol::InterfaceCommunicate(
                    strUpServerIP,nUpServerPort,strProtocolContentToUpper,BURNCONTROL_TAG_NAME,2);
                if ("" == strResponse)
                {
                    DESKTOP_ALERT_PARAM param;
                    param.strFlag=STATE_BURN_ERR;
                    param.strDescription=m_cfgFile.GetValue("info","sendrtrequestfailed");
                    param.bAutoClose=TRUE;
                    param.nAutoCloseTime=4;
                    ShowDesktopAlert(param);

                    LOG_ERROR(("[CBurnControlDlg::StartRtBurn] Send start burn request protocol to upper failed\r\n"));
                }
                else
                {
                    LOG_INFO(("[CBurnControlDlg::StartRtBurn] Send start burn request protocol to upper success\r\n"));
                }
            }
        }
    }
    else if(BURNCATEGORY_HM3 == m_eBurnCategory)
    {
        SaveRTBurnCfg();
        CreateAdditionalBurnFile();
        SendBurnJobToBurnServer();
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::StartRtBurn] Burn category error,%d\r\n",__LINE__));

        assert(false);
    }
}

void CBurnControlDlg::StopRtBurn(bool bToServer)
{
    std::string strProtocolContent;
    std::string strServerIP;
    std::string strServerPort;

    if ((BURNCATEGORY_RTBURN == m_eBurnCategory) &&
        (!bToServer))
    {
        strProtocolContent=CInterfaceProtocol::GenerateProtocolSendRTBurnRequestToUpper(
            m_RTBurnInfo.GetJobID(),BURNCONTROL_REQUEST_STOP);

        strServerIP=m_RTBurnInfo.GetUpServerIP();
        strServerPort=m_RTBurnInfo.GetUpServerPort();

        if ( (strProtocolContent != "") &&
            (strServerIP != "") &&
            (strServerPort != "") )
        {
            AsyncSendNCXMsg(strServerIP,strServerPort,strProtocolContent);
            ShowBurnControlUIUnable();
            //std::string strResponse;
            //strResponse=CInterfaceProtocol::InterfaceCommunicate(strServerIP,atoi(strServerPort.c_str()),
            //    strProtocolContent,BURNCONTROL_TAG_NAME);
            //if (strResponse.length() > 0)
            //{
            //    LOG_INFO(("[CBurnControlDlg::StopRtBurn] Send stop protocol to upper success\r\n"));
            //}
            //else
            //{
            //    LOG_ERROR(("[CBurnControlDlg::StopRtBurn] Send stop protocol to upper failed\r\n"));
            //}
        }
        else
        {
            LOG_ERROR(("[CBurnControlDlg::StopRtBurn] Parameter error,%d\r\n",__LINE__));
            m_nBurnState = BURNCONTROL_STATE_ERROR;
            ShowBurnControlUIIdle();
            DeleteRTBBackupXML();
        }
    }
    else if((BURNCATEGORY_HM3 == m_eBurnCategory) ||
        ((BURNCATEGORY_RTBURN == m_eBurnCategory) && bToServer))
    {
        SendBurnControlCMDToBurnServer("stopRTBurn");
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::StopRtBurn] Burn category error\r\n"));

        assert(false);
    }
}

void CBurnControlDlg::PauseRtBurn(bool bToServer)
{
    std::string strProtocolContent;
    std::string strServerIP;
    std::string strServerPort;

    if ((BURNCATEGORY_RTBURN == m_eBurnCategory) &&
        (!bToServer))
    {
        strProtocolContent=CInterfaceProtocol::GenerateProtocolSendRTBurnRequestToUpper(
            m_RTBurnInfo.GetJobID(),BURNCONTROL_REQUEST_PAUSE);

        strServerIP=m_RTBurnInfo.GetUpServerIP();
        strServerPort=m_RTBurnInfo.GetUpServerPort();

        if ( (strProtocolContent != "") &&
            (strServerIP != "") &&
            (strServerPort != "") )
        {
            AsyncSendNCXMsg(strServerIP,strServerPort,strProtocolContent);

            //std::string strResponse;
            //strResponse=CInterfaceProtocol::InterfaceCommunicate(strServerIP,atoi(strServerPort.c_str()),
            //    strProtocolContent,BURNCONTROL_TAG_NAME);
            //if (strResponse.length() > 0)
            //{
            //    LOG_INFO(("[CBurnControlDlg::PauseRtBurn] Send pause protocol to upper success\r\n"));
            //}
            //else
            //{
            //    LOG_ERROR(("[CBurnControlDlg::PauseRtBurn] Send pause protocol to upper failed\r\n"));
            //}
        }
        else
        {
            LOG_ERROR(("[CBurnControlDlg::PauseRtBurn] Upper address empty\r\n"));
        }
    } 
    else if((BURNCATEGORY_HM3 == m_eBurnCategory) ||
        ((BURNCATEGORY_RTBURN == m_eBurnCategory) && bToServer))
    {
        SendBurnControlCMDToBurnServer("pauseRTBurn");
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::PauseRtBurn] Burn category error\r\n"));

        assert(false);
    }
}

void CBurnControlDlg::ResumeRtBurn(bool bToServer)
{
    std::string strProtocolContent;
    std::string strServerIP;
    std::string strServerPort;

    if ((BURNCATEGORY_RTBURN == m_eBurnCategory) && (!bToServer))
    {
        strProtocolContent=CInterfaceProtocol::GenerateProtocolSendRTBurnRequestToUpper(
            m_RTBurnInfo.GetJobID(),BURNCONTROL_REQUEST_RESUME);

        strServerIP=m_RTBurnInfo.GetUpServerIP();
        strServerPort=m_RTBurnInfo.GetUpServerPort();

        if (strProtocolContent != "" &&
            strServerIP != "" &&
            strServerPort != "")
        {
            std::string strResponse;
            strResponse=CInterfaceProtocol::InterfaceCommunicate(strServerIP,atoi(strServerPort.c_str()),
                strProtocolContent,BURNCONTROL_TAG_NAME,2);
            if (strResponse.length() > 0)
            {
                LOG_INFO(("[CBurnControlDlg::ResumeRtBurn] Send resume protocol to upper success\r\n"));
            }
            else
            {
                LOG_ERROR(("[CBurnControlDlg::ResumeRtBurn] Send resume protocol to upper failed\r\n"));
            }
        }
        else
        {
            LOG_ERROR(("[CBurnControlDlg::ResumeRtBurn] Upper address empty\r\n"));
        }
    } 
    else if((BURNCATEGORY_HM3 == m_eBurnCategory) ||
        ((BURNCATEGORY_RTBURN == m_eBurnCategory) && bToServer))
    {
        SendBurnControlCMDToBurnServer("resumeRTBurn");
    }
    else
    {
        LOG_ERROR(("[CBurnControlDlg::ResumeRtBurn] Burn category error\r\n"));

        assert(false);
    }
}

void CBurnControlDlg::ResetBurnInfo()
{
    m_eBurnCategory=BURNCATEGORY_UNKNOWN;
    m_RTBurnInfo=CRTBurnInfo();
    m_normalJob=CNormalBurnInfo();
    m_nJobIndex=0;
    m_strProtocol="";
    m_nBurnState=BURNCONTROL_STATE_IDLE;
    m_vecBurnServerInfoEx.clear();

    SetConnectServerFailedCount(0);
    SetDevUpdated(false);
}

void CBurnControlDlg::UpdateBurnServerUIInfo()
{
    CComboBox *pComboBox=(CComboBox *)GetDlgItem(IDC_COMBO_RECORDER);
    if (pComboBox != NULL &&
        pComboBox->GetSafeHwnd() != NULL)
    {
        int nIndex=pComboBox->GetCurSel();
        if (nIndex >= 0)
        {
            //���¿�¼�����Ϳ�¼����Ϣ����ʾ����
            CBurnServerInfo curBurnServerInfo;
            if (GetCurBurnServerSel(curBurnServerInfo))
            {
                CStringW strBurnServerDescriptionW;
                CEdit *pBurnServerDescription=(CEdit *)(GetDlgItem(IDC_EDIT_RECORDERINFO));
                if (pBurnServerDescription != NULL &&
                    pBurnServerDescription->GetSafeHwnd() != NULL &&
                    curBurnServerInfo.GetDescription() != "")
                {
                    strBurnServerDescriptionW=CharsetConvertMFC::UTF8ToUTF16(
                        curBurnServerInfo.GetDescription().c_str());

                    pBurnServerDescription->SetWindowText(CString(strBurnServerDescriptionW));
                }

                if (BURNSERVER_TYPE_NORMAL != curBurnServerInfo.GetBurnServerType())
                {
                    GetDlgItem(IDC_RADIO_SINGLE)->EnableWindow(FALSE);
                    GetDlgItem(IDC_RADIO_MULTISYN)->EnableWindow(FALSE);
                    GetDlgItem(IDC_RADIO_MULTIASYN)->EnableWindow(FALSE);

                    ((CListCtrl *)GetDlgItem(IDC_LIST_SELECTCDROM))->DeleteAllItems();
                    GetDlgItem(IDC_LIST_SELECTCDROM)->EnableWindow(FALSE);

                    DWORD nColor=GetSysColor(COLOR_GRAYTEXT);
                    ((CListCtrl *)GetDlgItem(IDC_LIST_SELECTCDROM))->SetTextColor(nColor);

                    DisplayCDROMUI(DEV_UI_FLAG_CREATE);
                } 
                else
                {
                    GetDlgItem(IDC_RADIO_SINGLE)->EnableWindow(TRUE);
                    GetDlgItem(IDC_RADIO_MULTISYN)->EnableWindow(TRUE);
                    GetDlgItem(IDC_RADIO_MULTIASYN)->EnableWindow(TRUE);

                    GetDlgItem(IDC_LIST_SELECTCDROM)->EnableWindow(TRUE);
                    DWORD nColor=GetSysColor(COLOR_WINDOWTEXT);
                    ((CListCtrl *)GetDlgItem(IDC_LIST_SELECTCDROM))->SetTextColor(nColor);

                    DisplayCDROMUI(DEV_UI_FLAG_CREATE);
                }
            }
        }
    }
}

CBurnControlDlg::ITEM_TYPE CBurnControlDlg::GetItemType(CListCtrl *pListCtrl,int nItem,int nSubItem)
{
    ITEM_TYPE eItemType=ITEM_TYPE_UNKNOWN;

    if ((void *)pListCtrl == (void *)(GetDlgItem(IDC_LIST_JOBLIST)))
    {
        if (3 == nSubItem)
        {
            return ITEM_TYPE_EDIT;
        }
    }

    return eItemType;
}

void CBurnControlDlg::AttachEdit(CListCtrl *pListCtrl,int nItem,int nSubItem)
{
    assert((pListCtrl != NULL) && (pListCtrl->GetSafeHwnd() != NULL));

    CStringW strText=pListCtrl->GetItemText(nItem,nSubItem);

    CRect rect;
    pListCtrl->GetSubItemRect(nItem,nSubItem,LVIR_BOUNDS,rect);

    m_editItem.SetParent(pListCtrl);
    m_editItem.SetWindowText(strText);
    m_editItem.MoveWindow(rect);

    m_editItem.SetFocus();
}

void CBurnControlDlg::ShowDesktopAlert(const DESKTOP_ALERT_PARAM &param)
{
    CStringW strMsg=CharsetConvertMFC::UTF8ToUTF16(param.strDescription.c_str());
    SetAlertMsg(strMsg);
    SetAlertFlag(param.strFlag);

    assert(strMsg != L"");

    CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);

    CCustomMFCDesktopAlertWnd* pCustomMFCDesktopAlertWnd=new CCustomMFCDesktopAlertWnd;
    if (pCustomMFCDesktopAlertWnd != NULL)
    {
        CPopupDlg::Parent(this);

        pCustomMFCDesktopAlertWnd->SetAnimationType((CMFCPopupMenu::ANIMATION_TYPE) param.nAnimation);
        pCustomMFCDesktopAlertWnd->SetAnimationSpeed(param.nAnimationSpeed);
        pCustomMFCDesktopAlertWnd->SetTransparency((BYTE)param.nTransparency);
        pCustomMFCDesktopAlertWnd->SetSmallCaption(param.bSmallCaption);
        pCustomMFCDesktopAlertWnd->SetAutoCloseTime(param.bAutoClose ? param.nAutoCloseTime * 1000 : 0);

        CPoint ptPopup=CPoint(-1,-1);

        pCustomMFCDesktopAlertWnd->Create(NULL,IDD_DIALOG_POPUP,
            NULL,ptPopup,RUNTIME_CLASS(CPopupDlg));
        pCustomMFCDesktopAlertWnd->SetParent(NULL);

        HICON hIcon=(HICON)::LoadImage(::AfxGetResourceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME),
            IMAGE_ICON,::GetSystemMetrics(SM_CXSMICON),::GetSystemMetrics(SM_CYSMICON),0);

        pCustomMFCDesktopAlertWnd->SetIcon(hIcon,FALSE);
    }
}

DWORD WINAPI SendKeepAliveMsgThread(LPVOID lpParameter)
{
    if (lpParameter != NULL)
    {
        THREAD_PARAM *pThreadParam=(THREAD_PARAM *)(lpParameter);

        std::string strUpperIP;
        std::string strUpperPort;
        std::string strProtocolContent;
        CBurnControlDlg *pBurnControlDlg=(CBurnControlDlg *)(pThreadParam->pUserParam);

        if (pThreadParam->p1 != NULL)
        {
            strUpperIP=std::string((char *)(pThreadParam->p1));

            delete [] (char *)(pThreadParam->p1);
            pThreadParam->p1=NULL;
        }

        if (pThreadParam->p2 != NULL)
        {
            strUpperPort=std::string((char *)(pThreadParam->p2));

            delete [] (char *)(pThreadParam->p2);
            pThreadParam->p2=NULL;
        }

        if (pThreadParam->pThreadData != NULL)
        {
            strProtocolContent=std::string((char *)(pThreadParam->pThreadData));

            delete [] (char *)(pThreadParam->pThreadData);
            pThreadParam->pThreadData=NULL;
        }

        if (strUpperIP          != "" &&
            strUpperPort        != "" &&
            strProtocolContent  != "" &&
            pBurnControlDlg     != NULL)
        {
            std::string strResponse;
            strResponse=CInterfaceProtocol::InterfaceCommunicate(strUpperIP,
                atoi(strUpperPort.c_str()),strProtocolContent,BURNCONTROL_TAG_NAME,1);
            if (strResponse.length() > 0)
            {   
                pBurnControlDlg->SetConnectServerFailedCount(0);
            }
            else
            {
                pBurnControlDlg->SetConnectServerFailedCount(pBurnControlDlg->GetConnectServerFailedCount()+1);
            }
        }

        delete pThreadParam;
        pThreadParam=NULL;
    }

    return 0;
}

DWORD WINAPI SendAsyncNCXMsgThread(LPVOID lpParameter)
{
    if (lpParameter != NULL)
    {
        THREAD_PARAM *pThreadParam=(THREAD_PARAM *)(lpParameter);

        std::string strUpperIP;
        std::string strUpperPort;
        std::string strProtocolContent;
        CBurnControlDlg *pBurnControlDlg=(CBurnControlDlg *)(pThreadParam->pUserParam);

        if (pThreadParam->p1 != NULL)
        {
            strUpperIP=std::string((char *)(pThreadParam->p1));

            delete [] (char *)(pThreadParam->p1);
            pThreadParam->p1=NULL;
        }

        if (pThreadParam->p2 != NULL)
        {
            strUpperPort=std::string((char *)(pThreadParam->p2));

            delete [] (char *)(pThreadParam->p2);
            pThreadParam->p2=NULL;
        }

        if (pThreadParam->pThreadData != NULL)
        {
            strProtocolContent=std::string((char *)(pThreadParam->pThreadData));

            delete [] (char *)(pThreadParam->pThreadData);
            pThreadParam->pThreadData=NULL;
        }

        if (strUpperIP != "" &&
            strUpperPort != "" &&
            strProtocolContent != "" &&
            pBurnControlDlg != NULL)
        {
            std::string strResponse;
            strResponse=CInterfaceProtocol::InterfaceCommunicate(strUpperIP,
                atoi(strUpperPort.c_str()),strProtocolContent,BURNCONTROL_TAG_NAME,2);

            std::string strCmdType=CInterfaceProtocol::GetCommandType(strProtocolContent.c_str());
            bool bResult=( strResponse.length() > 0 );

            E_COMMAND_TYPE eCmdType;
            unsigned int nResult;

            if (bResult)
            {
                nResult=1;
            }
            else
            {
                nResult=0;
            }

            if ("startRTBurn" == strCmdType)
            {
                eCmdType=START_RT_BURN;
            } 
            else if("startBurn" == strCmdType)
            {
                eCmdType=START_BURN;
            }
            else if ("sendRTBurnConfig" == strCmdType)
            {
                eCmdType=SEND_RT_BURN_CONFIG;
            }
            else if ("sendRTBurnRequest" == strCmdType)
            {
                eCmdType=SEND_RT_BURN_REQUEST;
            }
            else if ("sendBurnCtrlCMD" == strCmdType)
            {
                eCmdType=SEND_BUTN_CTRL_CMD;
            }

            ::PostMessageW(pBurnControlDlg->GetSafeHwnd(),WMUSER_BURNCONTROLDLG_ASYNCMSGRESULT,static_cast<WPARAM>(eCmdType),
                static_cast<LPARAM>(nResult));
        }

        delete pThreadParam;
        pThreadParam=NULL;
    }

    return 0;
}

void CBurnControlDlg::SendKeepAliveMsg()
{
    CBurnServerInfo burnServerInfo;
    if (GetCurBurnServerSel(burnServerInfo))
    {
        std::string strKeepAliveTestContent;
        strKeepAliveTestContent="<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
        strKeepAliveTestContent+="<burnControl>";
        strKeepAliveTestContent+="<commandType>";
        strKeepAliveTestContent+="keepAlive";
        strKeepAliveTestContent+="</commandType>";
        strKeepAliveTestContent+="</burnControl>";

        THREAD_PARAM *pThreadParam=new THREAD_PARAM;
        if (pThreadParam != NULL)
        {
            int nContentLen=strKeepAliveTestContent.length();

            pThreadParam->pThreadData=new char[nContentLen+1];
            if (pThreadParam->pThreadData != NULL)
            {
                strcpy((char *)(pThreadParam->pThreadData),strKeepAliveTestContent.c_str());
                pThreadParam->pUserParam=this;

                pThreadParam->p1=new char[128];
                if (pThreadParam->p1 != NULL)
                {
                    strcpy((char *)(pThreadParam->p1),burnServerInfo.GetIP().c_str());

                    pThreadParam->p2=new char[128];
                    if (pThreadParam->p2 != NULL)
                    {
                        strcpy((char *)(pThreadParam->p2),burnServerInfo.GetPort().c_str());

                        HANDLE hThread=CreateThread(NULL,0,SendKeepAliveMsgThread,pThreadParam,0,NULL);
                        if (hThread != NULL)
                        {
                            CloseHandle(hThread);

                            return;
                        }
                    }

                    delete [] (char *)(pThreadParam->p1);
                    pThreadParam->p1=NULL;
                }

                delete [] (char *)(pThreadParam->pThreadData);
                pThreadParam->pThreadData=NULL;
            }

            delete pThreadParam;
            pThreadParam=NULL;
        }
    }
}

void CBurnControlDlg::AsyncSendNCXMsg(std::string strIP,std::string strPort,std::string strContent)
{
    THREAD_PARAM *pThreadParam=new THREAD_PARAM;
    if (pThreadParam != NULL)
    {
        int nContentLen=strContent.length();

        pThreadParam->pThreadData=new char[nContentLen+1];
        if (pThreadParam->pThreadData != NULL)
        {
            strcpy((char *)(pThreadParam->pThreadData),strContent.c_str());
            pThreadParam->pUserParam=this;

            pThreadParam->p1=new char[128];
            if (pThreadParam->p1 != NULL)
            {
                strcpy((char *)(pThreadParam->p1),strIP.c_str());

                pThreadParam->p2=new char[128];
                if (pThreadParam->p2 != NULL)
                {
                    strcpy((char *)(pThreadParam->p2),strPort.c_str());

                    HANDLE hThread=CreateThread(NULL,0,SendAsyncNCXMsgThread,pThreadParam,0,NULL);
                    if (hThread != NULL)
                    {
                        CloseHandle(hThread);

                        return;
                    }
                }

                delete [] (char *)(pThreadParam->p1);
                pThreadParam->p1=NULL;
            }

            delete [] (char *)(pThreadParam->pThreadData);
            pThreadParam->pThreadData=NULL;
        }

        delete pThreadParam;
        pThreadParam=NULL;
    }
}

std::string CBurnControlDlg::GetAlertFlag() const 
{ 
    return m_strAlertFlag;
}

void CBurnControlDlg::SetAlertFlag(std::string val) 
{
    m_strAlertFlag = val;
}

void CBurnControlDlg::SetConnectServerFailedCount(int n)
{
    ZOSMutexLocker  locker(&m_mutexConnectServerFailedCount);

    m_nConnectServerFailedCount=n;
}

int CBurnControlDlg::GetConnectServerFailedCount()
{
    ZOSMutexLocker  locker(&m_mutexConnectServerFailedCount);

    return m_nConnectServerFailedCount;
}

bool CBurnControlDlg::GetDevUpdated() const
{
    return m_bDevUpdated;
}

void CBurnControlDlg::SetDevUpdated(bool val) 
{ 
    m_bDevUpdated = val; 
}

void CBurnControlDlg::OnBnClickedCheckL1()
{
    // TODO: Add your control notification handler code here
    UpdateBurnContentShowStatus();
}

void CBurnControlDlg::OnBnClickedCheckL2()
{
    // TODO: Add your control notification handler code here
    UpdateBurnContentShowStatus();
}

void CBurnControlDlg::OnBnClickedCheckL3()
{
    // TODO: Add your control notification handler code here
    UpdateBurnContentShowStatus();
}

void CBurnControlDlg::OnBnClickedCheckL4()
{
    // TODO: Add your control notification handler code here
    UpdateBurnContentShowStatus();
}

void CBurnControlDlg::OnBnClickedCheckL5()
{
    // TODO: Add your control notification handler code here
    UpdateBurnContentShowStatus();
}

void CBurnControlDlg::OnBnClickedCheckL6()
{
    // TODO: Add your control notification handler code here
    UpdateBurnContentShowStatus();
}

void CBurnControlDlg::OnBnClickedCheckL7()
{
    // TODO: Add your control notification handler code here
    UpdateBurnContentShowStatus();
}

void CBurnControlDlg::OnBnClickedCheckL8()
{
    // TODO: Add your control notification handler code here
    UpdateBurnContentShowStatus();
}

void CBurnControlDlg::OnBnClickedCheckL9()
{
    // TODO: Add your control notification handler code here
    UpdateBurnContentShowStatus();
}

void CBurnControlDlg::OnBnClickedCheckL10()
{
    // TODO: Add your control notification handler code here
    UpdateBurnContentShowStatus();
}

int CBurnControlDlg::GetBurnLocationCheckedNum()
{
    int nRet = 0;
    for (UINT nLocationIndex=IDC_CHECK_L1;nLocationIndex<IDC_CHECK_L1+LOCATION_MAX_COUNT;++nLocationIndex)
    {
        CWnd *p=GetDlgItem(nLocationIndex);
        if (p != NULL)
        {
            if (p->GetSafeHwnd() != NULL)
            {
                CStringW strText;
                p->GetWindowText(strText);
                if (strText != CharsetConvertMFC::UTF8ToUTF16(
                    m_cfgFile.GetValue("uiword","notecheckboxname").c_str()))
                {
                    if (((CButton*)p)->GetCheck() == BST_CHECKED)
                    {
                        nRet++;
                    }
                }
            }
        }
    }
    return nRet;
}

void CBurnControlDlg::EnableBurnLocationUnChecked(BOOL bEnable)
{
    for (UINT nLocationIndex=IDC_CHECK_L1;nLocationIndex<IDC_CHECK_L1+LOCATION_MAX_COUNT;++nLocationIndex)
    {
        CWnd *p=GetDlgItem(nLocationIndex);
        if (p != NULL)
        {
            if (p->GetSafeHwnd() != NULL)
            {
                CStringW strText;
                p->GetWindowText(strText);
                if (strText != CharsetConvertMFC::UTF8ToUTF16(
                    m_cfgFile.GetValue("uiword","notecheckboxname").c_str()))
                {
                    if (((CButton*)p)->GetCheck() == BST_UNCHECKED
                        && p->IsWindowVisible())
                    {
                        p->EnableWindow(bEnable);
                    }
                }
            }
        }
    }
}

void CBurnControlDlg::UpdateBurnContentShowStatus()
{
    if (m_eBurnCategory == BURNCATEGORY_HM3
        || m_eBurnCategory == BURNCATEGORY_RTBURN)
    {
        if (GetBurnLocationCheckedNum() >= LOCATIONMAXNUM)
        {
            EnableBurnLocationUnChecked(FALSE);
        }
        else
        {
            EnableBurnLocationUnChecked(TRUE);
        }
    }
}

void CBurnControlDlg::ReadRTBBackupXML(CRTBurnInfo &rtBurnInfo)
{
    ConfigurableFile rtbBackupXML;
    rtbBackupXML.LoadFile(XML_RTBBACKUP_PATH);
    TiXmlNode* pNodeRTBInfo = rtbBackupXML.GetKeyNode("rtbInfo");
    if (pNodeRTBInfo != NULL)
    {
        for (TiXmlNode* pNodeRTBInfoOption = pNodeRTBInfo->FirstChild(); pNodeRTBInfoOption != NULL; pNodeRTBInfoOption = pNodeRTBInfoOption->NextSibling())
        {
            if (pNodeRTBInfoOption->ValueStr() == "jobID")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetJobID(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "jobType")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetJobType(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "burnServerInfo")
            {
                for (TiXmlNode* pNodeBurnServerOption = pNodeRTBInfoOption->FirstChild(); pNodeBurnServerOption != NULL; pNodeBurnServerOption = pNodeBurnServerOption->NextSibling())
                {
                    if (pNodeBurnServerOption->ValueStr() == "burnServerIP")
                    {
                        if (pNodeBurnServerOption->FirstChild() != NULL)
                        {
                            rtBurnInfo.GetBurnServerInfo().SetIP(pNodeBurnServerOption->FirstChild()->ValueStr());
                        }
                    }
                    else if (pNodeBurnServerOption->ValueStr() == "burnServerPort")
                    {
                        if (pNodeBurnServerOption->FirstChild() != NULL)
                        {
                            rtBurnInfo.GetBurnServerInfo().SetPort(pNodeBurnServerOption->FirstChild()->ValueStr());
                        }
                    }
                    else if (pNodeBurnServerOption->ValueStr() == "burnServerName")
                    {
                        if (pNodeBurnServerOption->FirstChild() != NULL)
                        {
                            rtBurnInfo.GetBurnServerInfo().SetBurnServerName(pNodeBurnServerOption->FirstChild()->ValueStr());
                        }
                    }
                    else if (pNodeBurnServerOption->ValueStr() == "burnServerDescription")
                    {
                        if (pNodeBurnServerOption->FirstChild() != NULL)
                        {
                            rtBurnInfo.GetBurnServerInfo().SetDescription(pNodeBurnServerOption->FirstChild()->ValueStr());
                        }
                    }
                    else if (pNodeBurnServerOption->ValueStr() == "vectCDROMInfo")
                    {
                        std::vector<CCDROMDriverInfo> vectCDROMInfo;
                        for (TiXmlNode* pNodeCDROMInfo = pNodeBurnServerOption->FirstChild(); pNodeCDROMInfo != NULL; pNodeCDROMInfo = pNodeCDROMInfo->NextSibling())
                        {
                            CCDROMDriverInfo cdromInfo;
                            for (TiXmlNode* pNodeCDROMOption = pNodeCDROMInfo->FirstChild(); pNodeCDROMOption != NULL; pNodeCDROMOption = pNodeCDROMOption->NextSibling())
                            {
                                if (pNodeCDROMOption->ValueStr() == "cdromID")
                                {
                                    if (pNodeCDROMOption->FirstChild() != NULL)
                                    {
                                        cdromInfo.SetID(pNodeCDROMOption->FirstChild()->ValueStr());
                                    }
                                }
                                else if (pNodeCDROMOption->ValueStr() == "cdromDescription")
                                {
                                    if (pNodeCDROMOption->FirstChild() != NULL)
                                    {
                                        cdromInfo.SetDescription(pNodeCDROMOption->FirstChild()->ValueStr());
                                    }
                                }
                                else if (pNodeCDROMOption->ValueStr() == "isSelected")
                                {
                                    if (pNodeCDROMOption->FirstChild() != NULL)
                                    {
                                        cdromInfo.SetIsSelected(pNodeCDROMOption->FirstChild()->ValueStr());
                                    }
                                }
                            }
                            vectCDROMInfo.push_back(cdromInfo);
                        }
                        rtBurnInfo.GetBurnServerInfo().SetVecCDROMInfo(vectCDROMInfo);
                    }
                    else if (pNodeBurnServerOption->ValueStr() == "defaultBurnServerFlag")
                    {
                        if (pNodeBurnServerOption->FirstChild() != NULL)
                        {
                            rtBurnInfo.GetBurnServerInfo().SetIsDefault(pNodeBurnServerOption->FirstChild()->ValueStr());
                        }
                    }

                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "burnType")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.GetCommonBurnParam().SetBurnType(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "burnPassword")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.GetCommonBurnParam().SetBurnPassword(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "contentPassword")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.GetCommonBurnParam().SetContentPassword(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "cdName")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.GetCommonBurnParam().SetCDName(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "vectVideoLocationInfo")
            {
                std::vector<CLocationInfo> vectVideoLocation;
                for (TiXmlNode* pNodeVideoLocationInfo = pNodeRTBInfoOption->FirstChild(); pNodeVideoLocationInfo != NULL; pNodeVideoLocationInfo = pNodeVideoLocationInfo->NextSibling())
                {
                    CLocationInfo videoLocationInfo;
                    for (TiXmlNode* pNodeVideoLocationOption = pNodeVideoLocationInfo->FirstChild(); pNodeVideoLocationOption != NULL; pNodeVideoLocationOption = pNodeVideoLocationOption->NextSibling())
                    {
                        if (pNodeVideoLocationOption->ValueStr() == "locationID")
                        {
                            if (pNodeVideoLocationOption->FirstChild() != NULL)
                            {
                                videoLocationInfo.SetID(pNodeVideoLocationOption->FirstChild()->ValueStr());
                            }
                        }
                        else if (pNodeVideoLocationOption->ValueStr() == "locationDescription")
                        {
                            if (pNodeVideoLocationOption->FirstChild() != NULL)
                            {
                                videoLocationInfo.SetDescription(pNodeVideoLocationOption->FirstChild()->ValueStr());
                            }
                        }
                        else if (pNodeVideoLocationOption->ValueStr() == "locationBurnOrNot")
                        {
                            if (pNodeVideoLocationOption->FirstChild() != NULL)
                            {
                                videoLocationInfo.SetBurnOrNot(pNodeVideoLocationOption->FirstChild()->ValueStr());
                            }
                        }
                        else if (pNodeVideoLocationOption->ValueStr() == "locationURL")
                        {
                            if (pNodeVideoLocationOption->FirstChild() != NULL)
                            {
                                videoLocationInfo.SetURL(pNodeVideoLocationOption->FirstChild()->ValueStr());
                            }
                        }
                        else if (pNodeVideoLocationOption->ValueStr() == "streamType")
                        {
                            if (pNodeVideoLocationOption->FirstChild() != NULL)
                            {
                                videoLocationInfo.SetStreamType(pNodeVideoLocationOption->FirstChild()->ValueStr());
                            }
                        }
                    }
                    vectVideoLocation.push_back(videoLocationInfo);
                }
                rtBurnInfo.SetVecVideoLocationInfo(vectVideoLocation);
            }
            else if (pNodeRTBInfoOption->ValueStr() == "newFileName")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetNewFileName(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "upServerIP")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetUpServerIP(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "upServerPort")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetUpServerPort(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "stateFlag")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetStateFlag(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "stateDescription")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetStateDescription(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "playInfo")
            {
                for (TiXmlNode* pNodePlayerOption = pNodeRTBInfoOption->FirstChild(); pNodePlayerOption != NULL; pNodePlayerOption = pNodePlayerOption->NextSibling())
                {
                    if (pNodePlayerOption->ValueStr() == "playerVersion")
                    {
                        if (pNodePlayerOption->FirstChild() != NULL)
                        {
                            rtBurnInfo.GetPlayerInfo().SetVersion(pNodePlayerOption->FirstChild()->ValueStr());
                        }
                    }
                    else if (pNodePlayerOption->ValueStr() == "playerDownloadURL")
                    {
                        if (pNodePlayerOption->FirstChild() != NULL)
                        {
                            rtBurnInfo.GetPlayerInfo().SetDownloadURL(pNodePlayerOption->FirstChild()->ValueStr());
                        }
                    }
                    else if (pNodePlayerOption->ValueStr() == "playerRemoteIP")
                    {
                        if (pNodePlayerOption->FirstChild() != NULL)
                        {
                            rtBurnInfo.GetPlayerInfo().SetRemoteIP(pNodePlayerOption->FirstChild()->ValueStr());
                        }
                    }
                    else if (pNodePlayerOption->ValueStr() == "playerRemotePort")
                    {
                        if (pNodePlayerOption->FirstChild() != NULL)
                        {
                            rtBurnInfo.GetPlayerInfo().SetRemotePort(pNodePlayerOption->FirstChild()->ValueStr());
                        }
                    }
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "playListContent")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetPlayListContent(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "noteFileRemoteIP")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetNoteFileRemoteIP(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "noteFileRemotePort")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetNoteFileRemotePort(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "noteFileDownloadURL")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetNoteFileDownloadURL(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "autorunFileRemoteIP")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetAutorunFileRemoteIP(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "autorunFileRemotePort")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetAutorunFileRemotePort(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "autorunFilePath")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetAutorunFilePath(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "playlistRemoteIP")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.GetPlaylistFileInfo().SetRemoteIP(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "playlistRemotePort")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.GetPlaylistFileInfo().SetRemotePort(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "playlistDownloadURL")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.GetPlaylistFileInfo().SetDownloadURL(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "cdAlarmLimit")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetCDAlarmLimit(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "burnBufferSize")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetBurnBufferSize(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
            else if (pNodeRTBInfoOption->ValueStr() == "md5")
            {
                if (pNodeRTBInfoOption->FirstChild() != NULL)
                {
                    rtBurnInfo.SetMd5(pNodeRTBInfoOption->FirstChild()->ValueStr());
                }
            }
        }
    }
}

void CBurnControlDlg::WriteRTBBackupXML(CRTBurnInfo rtBurnInfo)
{
    TiXmlDocument doc;
    TiXmlElement rtbInfoElement("rtbInfo");

    //����ID
    TiXmlElement jobIDElement("jobID");
    TiXmlText jobIDText(rtBurnInfo.GetJobID());
    jobIDElement.InsertEndChild(jobIDText);
    rtbInfoElement.InsertEndChild(jobIDElement);

    //����״̬
    TiXmlElement jobTypeElement("jobType");
    TiXmlText jobTypeText(rtBurnInfo.GetJobType());
    jobTypeElement.InsertEndChild(jobTypeText);
    rtbInfoElement.InsertEndChild(jobTypeElement);

    //BurnServer��Ϣ������������Ϣ��
    TiXmlElement burnServerInfoElement("burnServerInfo");

    TiXmlElement burnServerIPElement("burnServerIP");
    TiXmlText burnServerIPText(rtBurnInfo.GetBurnServerInfo().GetIP());
    burnServerIPElement.InsertEndChild(burnServerIPText);
    burnServerInfoElement.InsertEndChild(burnServerIPElement);

    TiXmlElement burnServerPortElement("burnServerPort");
    TiXmlText burnServerPortText(rtBurnInfo.GetBurnServerInfo().GetPort());
    burnServerPortElement.InsertEndChild(burnServerPortText);
    burnServerInfoElement.InsertEndChild(burnServerPortElement);

    TiXmlElement burnServerNameElement("burnServerName");
    TiXmlText burnServerNameText(rtBurnInfo.GetBurnServerInfo().GetBurnServerName());
    burnServerNameElement.InsertEndChild(burnServerNameText);
    burnServerInfoElement.InsertEndChild(burnServerNameElement);

    TiXmlElement burnServerDescriptionElement("burnServerDescription");
    TiXmlText burnServerDescriptionText(rtBurnInfo.GetBurnServerInfo().GetBurnServerTypeDescription());
    burnServerDescriptionElement.InsertEndChild(burnServerDescriptionText);
    burnServerInfoElement.InsertEndChild(burnServerDescriptionElement);

    TiXmlElement vectCDROMInfoElement("vectCDROMInfo");
    std::vector<CCDROMDriverInfo> vectBurnServerCDROMInfo;
    vectBurnServerCDROMInfo = rtBurnInfo.GetBurnServerInfo().GetVecCDROMDriverInfo();

    for (int i = 0; i < (int)vectBurnServerCDROMInfo.size(); i++)
    {
        TiXmlElement cdromInfoElement("cdromInfo_" + IntToString(i + 1));

        TiXmlElement cdromIDElement("cdromID");
        TiXmlText cdromIDText(vectBurnServerCDROMInfo.at(i).GetID());
        cdromIDElement.InsertEndChild(cdromIDText);
        cdromInfoElement.InsertEndChild(cdromIDElement);

        TiXmlElement cdromDescriptionElement("cdromDescription");
        TiXmlText cdromDescriptionText(vectBurnServerCDROMInfo.at(i).GetDescription());
        cdromDescriptionElement.InsertEndChild(cdromDescriptionText);
        cdromInfoElement.InsertEndChild(cdromDescriptionElement);

        TiXmlElement isSelectedElement("isSelected");
        TiXmlText isSelectedText(vectBurnServerCDROMInfo.at(i).GetIsSelected());
        isSelectedElement.InsertEndChild(isSelectedText);
        cdromInfoElement.InsertEndChild(isSelectedElement);

        vectCDROMInfoElement.InsertEndChild(cdromInfoElement);
    }
    burnServerInfoElement.InsertEndChild(vectCDROMInfoElement);


    TiXmlElement defaultBurnServerFlagElement("defaultBurnServerFlag");
    TiXmlText defaultBurnServerFlagText(rtBurnInfo.GetBurnServerInfo().GetIsDefault());
    defaultBurnServerFlagElement.InsertEndChild(defaultBurnServerFlagText);
    burnServerInfoElement.InsertEndChild(defaultBurnServerFlagElement);

    rtbInfoElement.InsertEndChild(burnServerInfoElement);

    //��¼��ʽ
    TiXmlElement burnTypeElement("burnType");
    TiXmlText burnTypeText(rtBurnInfo.GetCommonBurnParam().GetBurnType());
    burnTypeElement.InsertEndChild(burnTypeText);
    rtbInfoElement.InsertEndChild(burnTypeElement);

    //���ʼ�������
    TiXmlElement burnPasswordElement("burnPassword");
    TiXmlText burnPasswordText(rtBurnInfo.GetCommonBurnParam().GetBurnPassword());
    burnPasswordElement.InsertEndChild(burnPasswordText);
    rtbInfoElement.InsertEndChild(burnPasswordElement);

    //���ݼ�������
    TiXmlElement contentPasswordElement("contentPassword");
    TiXmlText contentPasswordText(rtBurnInfo.GetCommonBurnParam().GetContentPassword());
    contentPasswordElement.InsertEndChild(contentPasswordText);
    rtbInfoElement.InsertEndChild(contentPasswordElement);

    //��������
    TiXmlElement cdNameElement("cdName");
    TiXmlText cdNameText(rtBurnInfo.GetCommonBurnParam().GetCDName());
    cdNameElement.InsertEndChild(cdNameText);
    rtbInfoElement.InsertEndChild(cdNameElement);

    //��¼������Ϣ
    TiXmlElement vectBurnVideoElement("vectVideoLocationInfo");
    std::vector<CLocationInfo> vectVideoLocationInfo;
    vectVideoLocationInfo = rtBurnInfo.GetVecVideoLocationInfo();

    for (int i = 0; i < (int)vectVideoLocationInfo.size(); i++)
    {
        TiXmlElement rtbVideoInfoElement("rtbVideoInfo_" + IntToString(i + 1));

        TiXmlElement locationIDElement("locationID");
        TiXmlText locationIDText(vectVideoLocationInfo.at(i).GetID());
        locationIDElement.InsertEndChild(locationIDText);
        rtbVideoInfoElement.InsertEndChild(locationIDElement);

        TiXmlElement locationDescriptionElement("locationDescription");
        TiXmlText locationDescriptionText(vectVideoLocationInfo.at(i).GetDescription());
        locationDescriptionElement.InsertEndChild(locationDescriptionText);
        rtbVideoInfoElement.InsertEndChild(locationDescriptionElement);

        TiXmlElement locationBurnOrNotElement("locationBurnOrNot");
        TiXmlText locationBurnOrNotText(vectVideoLocationInfo.at(i).GetBurnOrNot());
        locationBurnOrNotElement.InsertEndChild(locationBurnOrNotText);
        rtbVideoInfoElement.InsertEndChild(locationBurnOrNotElement);

        TiXmlElement locationURLElement("locationURL");
        TiXmlText locationURLText(vectVideoLocationInfo.at(i).GetURL());
        locationURLElement.InsertEndChild(locationURLText);
        rtbVideoInfoElement.InsertEndChild(locationURLElement);

        TiXmlElement streamTypeElement("streamType");
        TiXmlText streamTypeText(vectVideoLocationInfo.at(i).GetStreamType());
        streamTypeElement.InsertEndChild(streamTypeText);
        rtbVideoInfoElement.InsertEndChild(streamTypeElement);

        vectBurnVideoElement.InsertEndChild(rtbVideoInfoElement);
    }
    rtbInfoElement.InsertEndChild(vectBurnVideoElement);

    //�ϲ�Ӧ�÷���IP
    TiXmlElement upServerIPElement("upServerIP");
    TiXmlText upServerIPText(rtBurnInfo.GetUpServerIP());
    upServerIPElement.InsertEndChild(upServerIPText);
    rtbInfoElement.InsertEndChild(upServerIPElement);

    //�ϲ�Ӧ�÷���Port
    TiXmlElement upServerPortElement("upServerPort");
    TiXmlText upServerPortText(rtBurnInfo.GetUpServerPort());
    upServerPortElement.InsertEndChild(upServerPortText);
    rtbInfoElement.InsertEndChild(upServerPortElement);

    //ts�ļ�����
    TiXmlElement newFileNameElement("newFileName");
    TiXmlText newFileNameText(rtBurnInfo.GetNewFileName());
    newFileNameElement.InsertEndChild(newFileNameText);
    rtbInfoElement.InsertEndChild(newFileNameElement);

    //��¼״̬����
    TiXmlElement stateFlagElement("stateFlag");
    TiXmlText stateFlagText(rtBurnInfo.GetStateFlag());
    stateFlagElement.InsertEndChild(stateFlagText);
    rtbInfoElement.InsertEndChild(stateFlagElement);

    //��¼״̬����
    TiXmlElement stateDescriptionElement("stateDescription");
    TiXmlText stateDescriptionText(rtBurnInfo.GetStateDescription());
    stateDescriptionElement.InsertEndChild(stateDescriptionText);
    rtbInfoElement.InsertEndChild(stateDescriptionElement);

    //��������Ϣ
    TiXmlElement playInfoElement("playInfo");

    TiXmlElement playerVersionElement("playerVersion");
    TiXmlText playerVersionText(rtBurnInfo.GetPlayerInfo().GetVersion());
    playerVersionElement.InsertEndChild(playerVersionText);
    playInfoElement.InsertEndChild(playerVersionElement);

    TiXmlElement playerDownloadURLElement("playerDownloadURL");
    TiXmlText playerDownloadURLText(rtBurnInfo.GetPlayerInfo().GetDownloadURL());
    playerDownloadURLElement.InsertEndChild(playerDownloadURLText);
    playInfoElement.InsertEndChild(playerDownloadURLElement);

    TiXmlElement playerRemoteIPElement("playerRemoteIP");
    TiXmlText playerRemoteIPText(rtBurnInfo.GetPlayerInfo().GetRemoteIP());
    playerRemoteIPElement.InsertEndChild(playerRemoteIPText);
    playInfoElement.InsertEndChild(playerRemoteIPElement);

    TiXmlElement playerRemotePortElement("playerRemotePort");
    TiXmlText playerRemotePortText(rtBurnInfo.GetPlayerInfo().GetRemotePort());
    playerRemotePortElement.InsertEndChild(playerRemotePortText);
    playInfoElement.InsertEndChild(playerRemotePortElement);

    rtbInfoElement.InsertEndChild(playInfoElement);

    //playlist����
    TiXmlElement playListContentElement("playListContent");
    TiXmlText playListContentText(rtBurnInfo.GetPlayListContent());
    playListContentElement.InsertEndChild(playListContentText);
    rtbInfoElement.InsertEndChild(playListContentElement);

    //��¼����IP
    TiXmlElement noteFileRemoteIPElement("noteFileRemoteIP");
    TiXmlText noteFileRemoteIPText(rtBurnInfo.GetNoteFileRemoteIP());
    noteFileRemoteIPElement.InsertEndChild(noteFileRemoteIPText);
    rtbInfoElement.InsertEndChild(noteFileRemoteIPElement);

    //��¼���ض˿�
    TiXmlElement noteFileRemotePortElement("noteFileRemotePort");
    TiXmlText noteFileRemotePortText(rtBurnInfo.GetNoteFileRemotePort());
    noteFileRemotePortElement.InsertEndChild(noteFileRemotePortText);
    rtbInfoElement.InsertEndChild(noteFileRemotePortElement);

    //��¼����·��
    TiXmlElement noteFileDownloadURLElement("noteFileDownloadURL");
    TiXmlText noteFileDownloadURLText(rtBurnInfo.GetNoteFileDownloadURL());
    noteFileDownloadURLElement.InsertEndChild(noteFileDownloadURLText);
    rtbInfoElement.InsertEndChild(noteFileDownloadURLElement);

    //�Զ������ļ�����IP
    TiXmlElement autorunFileRemoteIPElement("autorunFileRemoteIP");
    TiXmlText autorunFileRemoteIPText(rtBurnInfo.GetAutorunFileRemoteIP());
    autorunFileRemoteIPElement.InsertEndChild(autorunFileRemoteIPText);
    rtbInfoElement.InsertEndChild(autorunFileRemoteIPElement);

    //�Զ������ļ����ض˿�
    TiXmlElement autorunFileRemotePortElement("autorunFileRemotePort");
    TiXmlText autorunFileRemotePortText(rtBurnInfo.GetAutorunFileRemotePort());
    autorunFileRemotePortElement.InsertEndChild(autorunFileRemotePortText);
    rtbInfoElement.InsertEndChild(autorunFileRemotePortElement);

    //�Զ������ļ�����·��
    TiXmlElement autorunFilePathElement("autorunFilePath");
    TiXmlText autorunFilePathText(rtBurnInfo.GetAutorunFilePath());
    autorunFilePathElement.InsertEndChild(autorunFilePathText);
    rtbInfoElement.InsertEndChild(autorunFilePathElement);

    //playlist����IP
    TiXmlElement playlistRemoteIPElement("playlistRemoteIP");
    TiXmlText playlistRemoteIPText(rtBurnInfo.GetPlaylistFileInfo().GetRemoteIP());
    playlistRemoteIPElement.InsertEndChild(playlistRemoteIPText);
    rtbInfoElement.InsertEndChild(playlistRemoteIPElement);

    //playlist���ض˿�
    TiXmlElement playlistRemotePortElement("playlistRemotePort");
    TiXmlText playlistRemotePortText(rtBurnInfo.GetPlaylistFileInfo().GetRemotePort());
    playlistRemotePortElement.InsertEndChild(playlistRemotePortText);
    rtbInfoElement.InsertEndChild(playlistRemotePortElement);

    //playlist����·��
    TiXmlElement playlistDownloadURLElement("playlistDownloadURL");
    TiXmlText playlistDownloadURLText(rtBurnInfo.GetPlaylistFileInfo().GetDownloadURL());
    playlistDownloadURLElement.InsertEndChild(playlistDownloadURLText);
    rtbInfoElement.InsertEndChild(playlistDownloadURLElement);

    //���̱�������
    TiXmlElement cdAlarmLimitElement("cdAlarmLimit");
    TiXmlText cdAlarmLimitText(rtBurnInfo.GetCDAlarmLimit());
    cdAlarmLimitElement.InsertEndChild(cdAlarmLimitText);
    rtbInfoElement.InsertEndChild(cdAlarmLimitElement);

    //��¼Buffer��С
    TiXmlElement burnBufferSizeElement("burnBufferSize");
    TiXmlText burnBufferSizeText(rtBurnInfo.GetBurnBufferSize());
    burnBufferSizeElement.InsertEndChild(burnBufferSizeText);
    rtbInfoElement.InsertEndChild(burnBufferSizeElement);

    //MD5
    TiXmlElement md5Element("md5");
    TiXmlText md5Text(rtBurnInfo.GetMd5());
    md5Element.InsertEndChild(md5Text);
    rtbInfoElement.InsertEndChild(md5Element);

    TiXmlElement rootElement("rtbBackupConfig");
    rootElement.InsertEndChild(rtbInfoElement);

    TiXmlDeclaration declaration("1.0","utf-8","");
    doc.InsertEndChild(declaration);
    doc.InsertEndChild(rootElement);

    doc.SaveFile(XML_RTBBACKUP_PATH);
}

bool CBurnControlDlg::DeleteRTBBackupXML()
{
    return FileUtil::DelFile(XML_RTBBACKUP_PATH);
}