
// TestBurnControlDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TestBurnControl.h"
#include "TestBurnControlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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


// CTestBurnControlDlg �Ի���




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


// CTestBurnControlDlg ��Ϣ�������

BOOL CTestBurnControlDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

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
    ncxInitNCXEnvironment();

    m_editIP.SetWindowText(_T("10.1.10.55"));
    m_editPort.SetWindowText(_T("7788"));
    m_editTag.SetWindowText(_T("burnControl"));

    return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CTestBurnControlDlg::OnPaint()
{
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
        CDialog::OnPaint();
    }
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
