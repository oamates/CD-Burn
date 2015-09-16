
// WinBurnDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TestWinBurn.h"
#include "TestWinBurnDlg.h"

#include "CharsetConvertMFC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_TIMER_UPDATEBURNSTATE 7777
#define ID_TIMER_CHECKDISC  6598

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


// CWinBurnDlg �Ի���




CWinBurnDlg::CWinBurnDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWinBurnDlg::IDD, pParent),
    m_hBurnHandle(NULL),
    m_nCurDevSel(-1)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWinBurnDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_RECORDER, m_listctrlRecorder);
    DDX_Control(pDX, IDC_EDIT_BURNSYN, m_editBurnSYN);
}

BEGIN_MESSAGE_MAP(CWinBurnDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTON_SEARCH_RECORDER, &CWinBurnDlg::OnBnClickedButtonSearchRecorder)
    ON_BN_CLICKED(IDC_BUTTON_EJECTDISC, &CWinBurnDlg::OnBnClickedButtonEjectdisc)
    ON_BN_CLICKED(IDC_BUTTON_HAVEDISC, &CWinBurnDlg::OnBnClickedButtonHavedisc)
    ON_BN_CLICKED(IDC_BUTTON_LOADDISC, &CWinBurnDlg::OnBnClickedButtonLoaddisc)
    ON_BN_CLICKED(IDC_BUTTON_BURNABLE, &CWinBurnDlg::OnBnClickedButtonBurnable)
    ON_BN_CLICKED(IDC_BUTTON_BLANKDISC, &CWinBurnDlg::OnBnClickedButtonBlankdisc)
    ON_BN_CLICKED(IDC_BUTTON_DISCINFO, &CWinBurnDlg::OnBnClickedButtonDiscinfo)
    ON_BN_CLICKED(IDC_BUTTON_BURN, &CWinBurnDlg::OnBnClickedButtonBurn)
    ON_BN_CLICKED(IDC_BUTTON_NEWPROJECT, &CWinBurnDlg::OnBnClickedButtonNewproject)
    ON_BN_CLICKED(IDC_BUTTON_ADDFILE, &CWinBurnDlg::OnBnClickedButtonAddfile)
    ON_BN_CLICKED(IDC_BUTTON_ADDDIR, &CWinBurnDlg::OnBnClickedButtonAdddir)
    ON_WM_DESTROY()
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CWinBurnDlg ��Ϣ�������

BOOL CWinBurnDlg::OnInitDialog()
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
    CRect rect;
    m_listctrlRecorder.GetClientRect(rect);
    int nWidth=rect.Width();

    m_listctrlRecorder.InsertColumn(0,_T("��������"),LVCFMT_LEFT,nWidth/4-1);
    m_listctrlRecorder.InsertColumn(1,_T("��¼״̬"),LVCFMT_LEFT,nWidth/4-1);
    m_listctrlRecorder.InsertColumn(2,_T("������Ϣ"),LVCFMT_LEFT,nWidth/4-1);
    m_listctrlRecorder.InsertColumn(3,_T("��¼����"),LVCFMT_LEFT,nWidth/4-1);

    m_listctrlRecorder.SetExtendedStyle(m_listctrlRecorder.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

    m_editBurnSYN.SetWindowText(L"0");

    SetTimer(ID_TIMER_CHECKDISC,6000,NULL);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CWinBurnDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWinBurnDlg::OnPaint()
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
HCURSOR CWinBurnDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWinBurnDlg::OnBnClickedButtonSearchRecorder()
{
    // TODO: Add your control notification handler code here
    if (ToBurnSYN())
    {
        if (!m_vecBurnHandle.empty())
        {
            if (m_vecBurnHandle.back() != NULL)
            {
                DestroyBurnHandle(m_vecBurnHandle.back());
                m_vecBurnHandle.pop_back();
            }
        }

        m_listctrlRecorder.DeleteAllItems();

        BURNHANDLE h=CreateBurnHandle();
        ScanDevice(h);
        int nDevCount=GetDeviceCount(h);
        for (int i=0;i<nDevCount;++i)
        {
            BURN_DEVICE_INFORMATION info;
            GetDeviceInfo(h,i,info);

            m_listctrlRecorder.InsertItem(i,CharsetConvertMFC::UTF8ToUTF16(info.szName).GetBuffer());
        }

        if (nDevCount > 0)
        {
            m_vecBurnHandle.push_back(h);

            int i;
            for (i=0;i<nDevCount-1;++i)
            {
                h=CreateBurnHandle();
                ScanDevice(h);
                m_vecBurnHandle.push_back(h);
            }
        }
    }
    else
    {
        if (m_hBurnHandle != NULL)
        {
            DestroyBurnHandle(m_hBurnHandle);
        }

        m_hBurnHandle=CreateBurnHandle();

        ScanDevice(m_hBurnHandle);

        int n=GetDeviceCount(m_hBurnHandle);

        m_listctrlRecorder.DeleteAllItems();
        for (int i=0;i<n;++i)
        {
            BURN_DEVICE_INFORMATION info;
            GetDeviceInfo(m_hBurnHandle,i,info);

            m_listctrlRecorder.InsertItem(i,CharsetConvertMFC::UTF8ToUTF16(info.szName).GetBuffer());
        }
    }
}

void CWinBurnDlg::OnBnClickedButtonEjectdisc()
{
    // TODO: Add your control notification handler code here
    m_listctrlRecorder.SetFocus();

    int n=m_listctrlRecorder.GetItemCount();
    for (int i=0;i<n;++i)
    {
        if (m_listctrlRecorder.GetItemState(i,LVIS_SELECTED))
        {
           if (m_hBurnHandle != NULL)
           {
               EjectDisc(m_hBurnHandle,i);
           }

           break;
        }
    }
}

void CWinBurnDlg::OnBnClickedButtonHavedisc()
{
    // TODO: Add your control notification handler code here
    m_listctrlRecorder.SetFocus();

    int n=m_listctrlRecorder.GetItemCount();
    for (int i=0;i<n;++i)
    {
        if (m_listctrlRecorder.GetItemState(i,LVIS_SELECTED))
        {
            if (HaveDisc(m_hBurnHandle,i))
            {
                TRACE("YES,%d\r\n",GetTickCount());

                //MessageBoxW(L"Yes",L"Information",MB_OK);
            }
            else
            {
                TRACE("NO,%d\r\n",GetTickCount());

                //MessageBoxW(L"No",L"Information",MB_OK);
            }

            break;
        }
    }
}

void CWinBurnDlg::OnBnClickedButtonLoaddisc()
{
    // TODO: Add your control notification handler code here
    m_listctrlRecorder.SetFocus();

    int n=m_listctrlRecorder.GetItemCount();
    for (int i=0;i<n;++i)
    {
        if (m_listctrlRecorder.GetItemState(i,LVIS_SELECTED))
        {
            if (m_hBurnHandle != NULL)
            {
                LoadDisc(m_hBurnHandle,i);
            }

            break;
        }
    }
}

void CWinBurnDlg::OnBnClickedButtonBurnable()
{
    // TODO: Add your control notification handler code here
    m_listctrlRecorder.SetFocus();

    int n=m_listctrlRecorder.GetItemCount();
    for (int i=0;i<n;++i)
    {
        if (m_listctrlRecorder.GetItemState(i,LVIS_SELECTED))
        {
            if (DiscBurnable(m_hBurnHandle,i))
            {
                MessageBoxW(L"Yes",L"Information",MB_OK);
            }
            else
            {
                MessageBoxW(L"No",L"Information",MB_OK);
            }

            break;
        }
    }
}

void CWinBurnDlg::OnBnClickedButtonBlankdisc()
{
    // TODO: Add your control notification handler code here
    m_listctrlRecorder.SetFocus();

    int n=m_listctrlRecorder.GetItemCount();
    for (int i=0;i<n;++i)
    {
        if (m_listctrlRecorder.GetItemState(i,LVIS_SELECTED))
        {
            if (FormatDisc(m_hBurnHandle,i))
            {
                MessageBoxW(L"Yes,format success",L"Information",MB_OK);
            }
            else
            {
                MessageBoxW(L"No,format failed",L"Information",MB_OK);
            }

            break;
        }
    }
}

void CWinBurnDlg::OnBnClickedButtonDiscinfo()
{
    // TODO: Add your control notification handler code here
    m_listctrlRecorder.SetFocus();

    int n=m_listctrlRecorder.GetItemCount();
    for (int i=0;i<n;++i)
    {
        if (m_listctrlRecorder.GetItemState(i,LVIS_SELECTED))
        {
            if (m_hBurnHandle != NULL)
            {
                DISC_INFORMATION info;
                if (GetDiscInfo(m_hBurnHandle,i,info))
                {
                    CStringW strDiscInfo;
                    strDiscInfo.Format(L"type:%d\n status:%d\n flags:%d\n used bytes:%llu MB\n free bytes:%llu MB",
                        info.uiDiscType,info.uiDiscStatus,info.uiDiscFlags,info.uiUsedBytes/1024/1024,info.uiFreeBytes/1024/1024);
                    MessageBoxW(strDiscInfo,L"Information",MB_OK);
                }
                else
                {
                    MessageBoxW(L"Error",L"Information",MB_OK);
                }
            }

            break;
        }
    }
}

void CWinBurnDlg::OnBnClickedButtonBurn()
{
    // TODO: Add your control notification handler code here
    CStringW strBurnSYN;
    m_editBurnSYN.GetWindowText(strBurnSYN);

    if (ToBurnSYN())
    {
        std::vector<BURNHANDLE>::iterator iter;
        int nIndex=0;
        for (iter=m_vecBurnHandle.begin();iter!=m_vecBurnHandle.end();++iter,++nIndex)
        {
            StartBurn(*iter,nIndex);
        }

        SetTimer(ID_TIMER_UPDATEBURNSTATE,1000,NULL);
    }
    else
    {
        m_listctrlRecorder.SetFocus();

        int n=m_listctrlRecorder.GetItemCount();
        int i;
        for (i=0;i<n;++i)
        {
            if (m_listctrlRecorder.GetItemState(i,LVIS_SELECTED))
            {
                if (m_hBurnHandle != NULL)
                {
                    // For test
                    while (!HaveDisc(m_hBurnHandle,0))
                    {
                        MessageBoxW(L"Please insert disc",L"Warning",MB_OK);
                    }

                    StartBurn(m_hBurnHandle,i);

                    m_nCurDevSel=i;

                    SetTimer(ID_TIMER_UPDATEBURNSTATE,2000,NULL);
                }

                break;
            }
        }

        if (i == n)
        {
            MessageBoxW(L"Please select a device",L"Warning",MB_OK);
        }
    }
}

void CWinBurnDlg::OnBnClickedButtonNewproject()
{
    // TODO: Add your control notification handler code here
    if (ToBurnSYN())
    {
        std::vector<BURNHANDLE>::iterator iter;
        for (iter=m_vecBurnHandle.begin();iter!=m_vecBurnHandle.end();++iter)
        {
            CreateDataProject(*iter);
        }
    }
    else
    {
        CreateDataProject(m_hBurnHandle);

        CStringW strVolumeLabel=L"���Թ�����������";

        SetDiscVolumeLabel(m_hBurnHandle,CharsetConvertMFC::UTF16ToUTF8(strVolumeLabel).GetBuffer());
    }
}

void CWinBurnDlg::OnBnClickedButtonAddfile()
{
    // TODO: Add your control notification handler code here
    CStringW strLastFilePath = theApp.GetProfileString(L"Settings",L"AddFile");
    if (strLastFilePath == L"")
    {
        wchar_t szCurDirectory[MAX_PATH]={0};
        GetCurrentDirectory(MAX_PATH, szCurDirectory);
        strLastFilePath=szCurDirectory;
    }

    wchar_t szFilters[]= L"(*.*)|*.*||";
    CFileDialog FileDlg(TRUE, L"",L"",OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,szFilters);
    FileDlg.m_ofn.lpstrInitialDir=strLastFilePath;
    if (FileDlg.DoModal() == IDOK)
    {
        strLastFilePath=FileDlg.GetPathName();

        if (ToBurnSYN())
        {
            std::vector<BURNHANDLE>::iterator iter;
            for (iter=m_vecBurnHandle.begin();iter!=m_vecBurnHandle.end();++iter)
            {
                AddFile(*iter,NULL,CharsetConvertMFC::UTF16ToUTF8(strLastFilePath).GetBuffer());
            }
        } 
        else
        {
            AddFile(m_hBurnHandle,NULL,CharsetConvertMFC::UTF16ToUTF8(strLastFilePath).GetBuffer());
        }
        
        CString strFileDir=FileDlg.GetPathName().Left(FileDlg.GetPathName().ReverseFind(L'\\')+1);
        theApp.WriteProfileString(_T("Settings"), _T("AddFile"), strFileDir);
    }
}

void CWinBurnDlg::OnBnClickedButtonAdddir()
{
    // TODO: Add your control notification handler code here
    CStringW strFolderPath=SelectDir();

    if (strFolderPath != "")
    {
        if (ToBurnSYN())
        {
            std::vector<BURNHANDLE>::iterator iter;
            for (iter=m_vecBurnHandle.begin();iter!=m_vecBurnHandle.end();++iter)
            {
                AddDirectory(*iter,NULL,CharsetConvertMFC::UTF16ToUTF8(strFolderPath).GetBuffer());
            }
        } 
        else
        {
            AddDirectory(m_hBurnHandle,NULL,CharsetConvertMFC::UTF16ToUTF8(strFolderPath).GetBuffer());
        }
    }
}

CStringW CWinBurnDlg:: SelectDir()
{	
    BROWSEINFO bi;	
    ZeroMemory(&bi,sizeof(BROWSEINFO));	
    bi.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
    wchar_t wcszName[MAX_PATH] = L"Test WIn Burn";
    bi.pszDisplayName = wcszName;	
    bi.lpszTitle = L"ѡ���ļ����ļ���";	
    bi.ulFlags = BIF_RETURNFSANCESTORS;	
    LPITEMIDLIST idl = SHBrowseForFolder(&bi);	
    if(idl == NULL)		
    {
        return L"";
    }	

    CStringW strDirectoryPath;	
    SHGetPathFromIDList(idl, strDirectoryPath.GetBuffer(MAX_PATH));	
    strDirectoryPath.ReleaseBuffer();

    if(strDirectoryPath.IsEmpty())		
    {
        return L"";	
    }

    return strDirectoryPath;
}
void CWinBurnDlg::OnDestroy()
{
    CDialog::OnDestroy();

    // TODO: Add your message handler code here
    if (m_hBurnHandle != NULL)
    {
        DestroyBurnHandle(m_hBurnHandle);
    }

    if (!m_vecBurnHandle.empty())
    {
        if (m_vecBurnHandle.back() != NULL)
        {
            DestroyBurnHandle(m_vecBurnHandle.back());
            m_vecBurnHandle.pop_back();
        }
    }
}

void CWinBurnDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: Add your message handler code here and/or call default
    if (ID_TIMER_UPDATEBURNSTATE == nIDEvent)
    {
        if (ToBurnSYN())
        {
            bool bKillTimer=true;

            std::vector<BURNHANDLE>::iterator iter;
            int nIndex=0;
            for (iter=m_vecBurnHandle.begin();iter!=m_vecBurnHandle.end();++iter,++nIndex)
            {
                BURN_STATE_INFORMATION burnStateInfo;
                GetBurnStateInfo(*iter,burnStateInfo);

                if (burnStateInfo.nState != 2)
                {
                    bKillTimer=false;
                }

                CStringW strState=L"";
                switch (burnStateInfo.nState)
                {
                case -1:
                    strState=L"��¼ʧ��";
                    break;

                case 0:
                    break;

                case 1:
                    strState=L"���ڿ�¼";
                    if (100 == burnStateInfo.ucBurnedPercentage)
                    {
                        strState=L"���ڷ��̣������ĵȴ�...";
                    }
                    break;

                case 2:
                    strState=L"��¼���";
                    break;
                }
                m_listctrlRecorder.SetItemText(nIndex,1,strState);

                m_listctrlRecorder.SetItemText(nIndex,2,CharsetConvertMFC::UTF8ToUTF16(burnStateInfo.szBurnStateDescription).GetBuffer());

                CStringW strBurnedPercentage=L"0";
                strBurnedPercentage.Format(L"%d",burnStateInfo.ucBurnedPercentage);
                m_listctrlRecorder.SetItemText(nIndex,3,strBurnedPercentage);
            }

            if (bKillTimer)
            {
                KillTimer(nIDEvent);
            }
        }
        else
        {
            if (m_hBurnHandle != NULL)
            {
                BURN_STATE_INFORMATION burnStateInfo;
                GetBurnStateInfo(m_hBurnHandle,burnStateInfo);

                if (2 == burnStateInfo.nState)
                {
                    KillTimer(nIDEvent);
                }

                CStringW strState=L"";
                switch (burnStateInfo.nState)
                {
                case -1:
                    strState=L"��¼ʧ��";
                    break;

                case 0:
                    break;

                case 1:
                    strState=L"���ڿ�¼";
                    if (100 == burnStateInfo.ucBurnedPercentage)
                    {
                        strState=L"���ڷ��̣������ĵȴ�...";
                    }
                    break;

                case 2:
                    strState=L"��¼���";
                    break;
                }
                m_listctrlRecorder.SetItemText(m_nCurDevSel,1,strState);

                m_listctrlRecorder.SetItemText(m_nCurDevSel,2,CharsetConvertMFC::UTF8ToUTF16(burnStateInfo.szBurnStateDescription).GetBuffer());

                CStringW strBurnedPercentage=L"0";
                strBurnedPercentage.Format(L"%d",burnStateInfo.ucBurnedPercentage);
                m_listctrlRecorder.SetItemText(m_nCurDevSel,3,strBurnedPercentage);
            }
            else
            {
                KillTimer(nIDEvent);
            }
        }
    }
    else if (ID_TIMER_CHECKDISC == nIDEvent)
    {
        OnBnClickedButtonHavedisc();
    }

    CDialog::OnTimer(nIDEvent);
}

bool CWinBurnDlg::ToBurnSYN() const
{
    CStringW strBurnSYN;
    m_editBurnSYN.GetWindowText(strBurnSYN);

    return (L"1" == strBurnSYN);
}