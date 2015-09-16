// StartBurnControl.cpp : Implementation of CStartBurnControlApp and DLL registration.

#include "stdafx.h"
#include "StartBurnControl.h"

#include "comcat.h"
#include "strsafe.h"
#include "objsafe.h"

#include "SafeOCX.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CStartBurnControlApp theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0x2F5B963B, 0xC18C, 0x4E87, { 0xBA, 0x8F, 0xEE, 0x1, 0x41, 0xD, 0xB8, 0x30 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;

// CLSID_SafeItem - Necessary for safe ActiveX control
// Id taken from IMPLEMENT_OLECREATE_EX function in xxxCtrl.cpp
const CATID CLSID_SafeItem =
{ 0xcdfa0cd4, 0x381b, 0x412d,{ 0xb7, 0xbd, 0x1d, 0xbf, 0xcd, 0x90, 0x21, 0xaa}};


// CStartBurnControlApp::InitInstance - DLL initialization

BOOL CStartBurnControlApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		// TODO: Add your own module initialization code here.
	}

	return bInit;
}



// CStartBurnControlApp::ExitInstance - DLL termination

int CStartBurnControlApp::ExitInstance()
{
	// TODO: Add your own module termination code here.

	return COleControlModule::ExitInstance();
}



// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    HRESULT hr;    // HResult used by Safety Functions

    AFX_MANAGE_STATE(_afxModuleAddrThis);

    if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
        return ResultFromScode(SELFREG_E_TYPELIB);

    if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
        return ResultFromScode(SELFREG_E_CLASS);

    // Mark the control as safe for initializing.

    hr = CreateComponentCategory(CATID_SafeForInitializing, 
        L"Controls safely initializable from persistent data!");
    if (FAILED(hr))
        return hr;

    hr = RegisterCLSIDInCategory(CLSID_SafeItem, 
        CATID_SafeForInitializing);
    if (FAILED(hr))
        return hr;

    // Mark the control as safe for scripting.

    hr = CreateComponentCategory(CATID_SafeForScripting, 
        L"Controls safely  scriptable!");
    if (FAILED(hr))
        return hr;

    hr = RegisterCLSIDInCategory(CLSID_SafeItem, 
        CATID_SafeForScripting);
    if (FAILED(hr))
        return hr;

    return NOERROR;
}



// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    AFX_MANAGE_STATE(_afxModuleAddrThis);  

    // 删除控件初始化安全入口.   
    HRESULT hr=UnRegisterCLSIDInCategory(CLSID_SafeItem, CATID_SafeForInitializing);  

    if (FAILED(hr))  
        return hr;  

    // 删除控件脚本安全入口   
    hr=UnRegisterCLSIDInCategory(CLSID_SafeItem, CATID_SafeForScripting);  

    if (FAILED(hr))  
        return hr;  

    if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))  
        return ResultFromScode(SELFREG_E_TYPELIB);  

    if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))  
        return ResultFromScode(SELFREG_E_CLASS);  

    return NOERROR;
}

