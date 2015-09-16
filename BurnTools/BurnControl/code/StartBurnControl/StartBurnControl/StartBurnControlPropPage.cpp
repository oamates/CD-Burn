// StartBurnControlPropPage.cpp : Implementation of the CStartBurnControlPropPage property page class.

#include "stdafx.h"
#include "StartBurnControl.h"
#include "StartBurnControlPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CStartBurnControlPropPage, COlePropertyPage)



// Message map

BEGIN_MESSAGE_MAP(CStartBurnControlPropPage, COlePropertyPage)
END_MESSAGE_MAP()



// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CStartBurnControlPropPage, "STARTBURNCONTR.StartBurnContrPropPage.1",
	0x69206d5c, 0xa30e, 0x47ec, 0x83, 0xfc, 0x6f, 0x2e, 0xa1, 0xb6, 0xdd, 0x1c)



// CStartBurnControlPropPage::CStartBurnControlPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CStartBurnControlPropPage

BOOL CStartBurnControlPropPage::CStartBurnControlPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_STARTBURNCONTROL_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}



// CStartBurnControlPropPage::CStartBurnControlPropPage - Constructor

CStartBurnControlPropPage::CStartBurnControlPropPage() :
	COlePropertyPage(IDD, IDS_STARTBURNCONTROL_PPG_CAPTION)
{
}



// CStartBurnControlPropPage::DoDataExchange - Moves data between page and properties

void CStartBurnControlPropPage::DoDataExchange(CDataExchange* pDX)
{
	DDP_PostProcessing(pDX);
}



// CStartBurnControlPropPage message handlers
