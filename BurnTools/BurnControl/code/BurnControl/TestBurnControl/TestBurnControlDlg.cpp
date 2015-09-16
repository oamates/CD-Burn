
// TestBurnControlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TestBurnControl.h"
#include "TestBurnControlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

    // 对话框数据
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    // 实现
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


// CTestBurnControlDlg 对话框




CTestBurnControlDlg::CTestBurnControlDlg(CWnd* pParent /*=NULL*/)
: CDialog(CTestBurnControlDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestBurnControlDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
    DDX_Control(pDX, IDC_EDIT_IP, m_editIP);
    DDX_Control(pDX, IDC_EDIT_PORT, m_editPort);
    DDX_Control(pDX, IDC_EDIT_TAG, m_editTag);
}

BEGIN_MESSAGE_MAP(CTestBurnControlDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTON_SENDBURNXML, &CTestBurnControlDlg::OnBnClickedSendBurnXml)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CTestBurnControlDlg 消息处理程序

BOOL CTestBurnControlDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // 将“关于...”菜单项添加到系统菜单中。

    // IDM_ABOUTBOX 必须在系统命令范围内。
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

    // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标

    // TODO: 在此添加额外的初始化代码
    ncxInitNCXEnvironment();

    m_editIP.SetWindowText(_T("10.1.10.55"));
    m_editPort.SetWindowText(_T("7788"));
    m_editTag.SetWindowText(_T("burnControl"));

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTestBurnControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTestBurnControlDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTestBurnControlDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CTestBurnControlDlg::OnBnClickedSendBurnXml()
{
    // TODO: Add your control notification handler code here
    CString strLastBurnXmlPath = theApp.GetProfileString(L"Settings",L"BurnXmlDir");
    if (strLastBurnXmlPath == L"")
    {
        wchar_t szCurDirectory[MAX_PATH]={0};
        GetCurrentDirectory(MAX_PATH, szCurDirectory);
        strLastBurnXmlPath=szCurDirectory;
    }

    wchar_t szFilters[]= L"(*.*)|*.*||";
    CFileDialog FileDlg(TRUE, L"",L"",OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,szFilters);
    FileDlg.m_ofn.lpstrInitialDir=strLastBurnXmlPath;
    if (FileDlg.DoModal() == IDOK)
    {
        std::string strProtocolContent;
        TiXmlDocument *pDocument=new TiXmlDocument;

        if (pDocument != NULL)
        {
            strLastBurnXmlPath+=FileDlg.GetFileName();
            CStringA str(strLastBurnXmlPath);
            if (pDocument->LoadFile(str.GetBuffer()))
            {
                TiXmlPrinter xmlPrinter;
                pDocument->Accept(&xmlPrinter);
                strProtocolContent=std::string(xmlPrinter.CStr());
            }

            delete pDocument;
        }

        if (strProtocolContent != "")
        {
            std::string strIP="127.0.0.1";
            std::string strPort="8888";
            std::string strTag="burnControl";

            CString str;

            m_editIP.GetWindowText(str);
            if (str.GetLength() > 0)
            {
                strIP=CStringA(str).GetBuffer();
            }

            m_editPort.GetWindowText(str);
            if (str.GetLength() > 0)
            {
                strPort=CStringA(str).GetBuffer();
            }

            m_editTag.GetWindowText(str);
            if (str.GetLength() > 0)
            {
                strTag=CStringA(str).GetBuffer();
            }

            std::string strResponse;
            strResponse=CInterfaceProtocol::InterfaceCommunicate(strIP,
                atoi(strPort.c_str()),strProtocolContent,strTag, 4);
            if (strResponse.length() > 0)
            {
                MessageBox(CString(CStringA(strResponse.c_str())));
            }
        }

        CString strFileDir=FileDlg.GetPathName().Left(FileDlg.GetPathName().ReverseFind(L'\\')+1);
        theApp.WriteProfileString(_T("Settings"), _T("BurnXmlDir"), strFileDir);
    }
}

void CTestBurnControlDlg::OnDestroy()
{
    CDialog::OnDestroy();

    // TODO: Add your message handler code here
    ncxUnInitNCXEnvironment();
}
