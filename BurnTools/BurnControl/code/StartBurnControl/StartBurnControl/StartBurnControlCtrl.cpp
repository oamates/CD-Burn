// StartBurnControlCtrl.cpp : Implementation of the CStartBurnControlCtrl ActiveX Control class.

#include "stdafx.h"
#include "StartBurnControl.h"
#include "StartBurnControlCtrl.h"
#include "StartBurnControlPropPage.h"

#include "WindowsProcess.h"
#include "Registry.h"
#include "CharsetConvertMFC.h"
#include "NCX.h"

#include <atlenc.h>

#include "SocketUtil.h"
#include "NetUtil.h"

const std::string BURN_PROTOCOL_TAG="burnControl";
const int BURN_PROTOCOL_PORT=7788;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CStartBurnControlCtrl, COleControl)



// Message map

BEGIN_MESSAGE_MAP(CStartBurnControlCtrl, COleControl)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()



// Dispatch map

BEGIN_DISPATCH_MAP(CStartBurnControlCtrl, COleControl)
	DISP_FUNCTION_ID(CStartBurnControlCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION_ID(CStartBurnControlCtrl, "StartBurnControl", dispidStartBurnControl, StartBurnControl, VT_I4, VTS_BSTR)
    DISP_FUNCTION_ID(CStartBurnControlCtrl, "StrBase64", dispidStrBase64, StrBase64, VT_BSTR, VTS_BSTR)
END_DISPATCH_MAP()



// Event map

BEGIN_EVENT_MAP(CStartBurnControlCtrl, COleControl)
END_EVENT_MAP()



// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CStartBurnControlCtrl, 1)
	PROPPAGEID(CStartBurnControlPropPage::guid)
END_PROPPAGEIDS(CStartBurnControlCtrl)



// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CStartBurnControlCtrl, "STARTBURNCONTROL.StartBurnControlCtrl.1",
	0xcdfa0cd4, 0x381b, 0x412d, 0xb7, 0xbd, 0x1d, 0xbf, 0xcd, 0x90, 0x21, 0xaa)



// Type library ID and version

IMPLEMENT_OLETYPELIB(CStartBurnControlCtrl, _tlid, _wVerMajor, _wVerMinor)



// Interface IDs

const IID BASED_CODE IID_DStartBurnControl =
		{ 0x3D243C9C, 0x1F0D, 0x4C7A, { 0x9C, 0x90, 0xCA, 0x4D, 0xFE, 0xE5, 0x60, 0x68 } };
const IID BASED_CODE IID_DStartBurnControlEvents =
		{ 0x74AE4278, 0xFF71, 0x4597, { 0x92, 0x78, 0xF5, 0x35, 0x9, 0xB6, 0xEE, 0xCD } };



// Control type information

static const DWORD BASED_CODE _dwStartBurnControlOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CStartBurnControlCtrl, IDS_STARTBURNCONTROL, _dwStartBurnControlOleMisc)



// CStartBurnControlCtrl::CStartBurnControlCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CStartBurnControlCtrl

BOOL CStartBurnControlCtrl::CStartBurnControlCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_STARTBURNCONTROL,
			IDB_STARTBURNCONTROL,
			afxRegApartmentThreading,
			_dwStartBurnControlOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}



// CStartBurnControlCtrl::CStartBurnControlCtrl - Constructor

CStartBurnControlCtrl::CStartBurnControlCtrl()
{
	InitializeIIDs(&IID_DStartBurnControl, &IID_DStartBurnControlEvents);
	// TODO: Initialize your control's instance data here.
}



// CStartBurnControlCtrl::~CStartBurnControlCtrl - Destructor

CStartBurnControlCtrl::~CStartBurnControlCtrl()
{
	// TODO: Cleanup your control's instance data here.
}



// CStartBurnControlCtrl::OnDraw - Drawing function

void CStartBurnControlCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	if (!pdc)
		return;

	// TODO: Replace the following code with your own drawing code.
	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
	pdc->Ellipse(rcBounds);
}



// CStartBurnControlCtrl::DoPropExchange - Persistence support

void CStartBurnControlCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.
}



// CStartBurnControlCtrl::OnResetState - Reset control to default state

void CStartBurnControlCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}



// CStartBurnControlCtrl::AboutBox - Display an "About" box to the user

void CStartBurnControlCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_STARTBURNCONTROL);
	dlgAbout.DoModal();
}



// CStartBurnControlCtrl message handlers

std::string CStartBurnControlCtrl::GetHostIP()
{
    std::string strHostIP;

    std::string strWirelessIP;

    int nInterfaceCount = NetUtil::GetInterfaceCount();
    for (int i = 0; i < nInterfaceCount; i ++)
    {
        std::string strIP;
        std::string strNetmask;
        std::string strGateway;
        std::string strMacAddr;
        std::string strDescription;

        if( 0 == NetUtil::GetInterfaceParameter(i, strIP, strNetmask, strGateway, strMacAddr, strDescription) )
        {
            if (strDescription.find("VMware") == std::string::npos &&
                strDescription.find("Wireless") == std::string::npos &&
                strDescription.find("WiFi") == std::string::npos)
            {
                strHostIP=strIP;

                break;
            }
            else if (strDescription.find("Wireless") != std::string::npos)
            {
                strWirelessIP=strIP;
            }
        }
    }

    if ("" == strHostIP)
    {
        strHostIP=strWirelessIP;
    }

    return strHostIP;
}


LONG CStartBurnControlCtrl::StartBurnControl(LPCTSTR strStartInfo)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    // TODO: Add your dispatch handler code here
    LONG nRet=-1;

    if (WindowsProcess::FindProcess(L"BurnControl.exe"))
    {
        nRet=0;
    }
    else
    {
        CStringW strExeWholePath;

        CRegistry reg;

        reg.SetRootKey(HKEY_CURRENT_USER);
        CStringW strKey=L"Software\\CS\\BurnControl1.0";
        if (reg.SetKey(strKey,FALSE))
        {
            strExeWholePath=reg.ReadString(L"BurnControl.exe",L"");
        }

        if (L"" == strExeWholePath)
        {
            strExeWholePath=L"C:\\Program Files\\BurnControl1.0\\BurnControl.exe";
        }

        if (WindowsProcess::StartProcess(strExeWholePath.GetBuffer()))
        {
            nRet=0;
        }
        else
        {
            nRet=-1;
        }
    }

    if (0 == nRet)
    {
        bool bBurnControlConnectable=false;
        int nCheckCount=0;

        while (true)
        {
            TRACE(L"Try %d ...\r\n",nCheckCount);

            if (nCheckCount > 100)
            {
                nRet=-1;

                break;
            }

//             if (SocketUtil::CheckTcpPortConnectable(GetHostIP().c_str(),BURN_PROTOCOL_PORT,100))
//             {
//                 break;
//             }
			if (_T("127.0.0.1"),BURN_PROTOCOL_PORT,100)
			{
				break;
			}
            else
            {
                nCheckCount++;
            }
        }
    }

    int n;

    if (0 == nRet)
    {
        //上层应用通过参数发送刻录协议
        if (strStartInfo != L"")
        {
            std::string strProtocol=std::string(CharsetConvertMFC::UTF16ToGB18030(strStartInfo).GetBuffer());

            std::string strDstIP=GetTagValue(strProtocol,"dstIP");
            std::string strDstPort=GetTagValue(strProtocol,"dstPort");
            std::string strData=GetTagValue(strProtocol,"content");


            if ((strDstIP != "") &&
                (strDstPort != "") &
                (strData != ""))
            {
                size_t nContentLen=strData.length();
                char *pszContent=new char[2*nContentLen+1];

                if (pszContent != NULL)
                {
                    n=2*nContentLen+1;
                    if (ATL::Base64Decode(strData.c_str(),strData.length(),(BYTE *)pszContent,&n))
                    {
                        pszContent[n]='\0';

                        CStringA str=CharsetConvertMFC::GB18030ToUTF8(pszContent);

                        std::string strResponse;
                        strResponse=InterfaceCommunicate(strDstIP,atoi(strDstPort.c_str()),
                            str.GetBuffer(),BURN_PROTOCOL_TAG);
                        if (strResponse.length() > 0)
                        {
                        }
                    }

                    delete pszContent;
                }
            }
        }
        else
        {
            //上层应用通过socket发送刻录协议
        }
    }

    return nRet;
}

std::string CStartBurnControlCtrl::GetTagValue(std::string strContent, std::string strTagName)
{
    std::string	strFind;
    size_t nBegin = 0;
    size_t nEnd = 0;
    std::string strReturn;

    strFind = "<";
    strFind += strTagName;
    strFind += ">";

    nBegin = strContent.find(strFind);
    if (nBegin != std::string::npos)
    {
        strFind = "</";
        strFind += strTagName;
        strFind += ">";

        nBegin += strTagName.length()+2;
        nEnd = strContent.find(strFind, nBegin);
        if (nEnd != std::string::npos && nEnd >= nBegin)
        {
            strReturn = strContent.substr(nBegin, nEnd-nBegin);
        }
    }

    return strReturn;
}

std::string CStartBurnControlCtrl::InterfaceCommunicate(std::string strIP, int nPort,const std::string &strContent,std::string strTag)
{
    NCXCLIENTHANDLE     hNCXClient;
    std::string         strResponse;

    hNCXClient=ncxCreateNCXClient();

    if (hNCXClient != NULL &&
        strIP != "" &&
        nPort > 0 &&
        strContent != "" &&
        strTag != "")
    {
        ncxSetNCXClientOuterMostTagName(hNCXClient,const_cast<char *>(strTag.c_str()));

        if (ncxConnectServer(hNCXClient,strIP.c_str(),nPort,7))
        {
            int nRet=ncxCommunicateWithServer(hNCXClient, strContent.c_str(),
                strContent.length()+1,7);

            if (0 == nRet)
            {
                strResponse=ncxGetRespondProtocol(hNCXClient);
            }
        }
    }

    if (hNCXClient != NULL)
    {
        ncxDestroyNCXClient(hNCXClient);
    }

    return strResponse;
}



BSTR CStartBurnControlCtrl::StrBase64(LPCTSTR strSrc)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString strResult;

    // TODO: Add your dispatch handler code here
    std::string str=CharsetConvertMFC::UTF16ToGB18030(strSrc);

    int nLen=str.length();

    char *pszTemp=new char[2*nLen+1];
    int n=2*nLen+1;
    if (ATL::Base64Encode((const BYTE *)(str.c_str()),str.length(),pszTemp,&n))
    {
        pszTemp[n]='\0';

        strResult=CharsetConvertMFC::GB18030ToUTF16(pszTemp);
    }

    delete [] pszTemp;

    return strResult.AllocSysString();
}
