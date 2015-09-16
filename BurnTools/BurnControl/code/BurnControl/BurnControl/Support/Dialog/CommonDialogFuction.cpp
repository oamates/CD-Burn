#include "stdafx.h"
#include "CommonDialogFuction.h"

void DesktopAlertPopup(CString strAlertText,CString strDlgCaption,BOOL bSmallCaption,int nAnimation,int nAnimationSpeed,int nAutoCloseTime,int nVisualMngr)
{
	switch (nVisualMngr)
	{
	case 0:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case 1:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case 2:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		break;

	case 3:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		break;

	case 4:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case 5:
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		break;

	case 6:
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		break;

	case 7:
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		break;

	case 8:
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		break;

	default:
		break;
	}

	CMFCDesktopAlertWnd* pPopup = new CMFCDesktopAlertWnd;
	if (pPopup != NULL)
	{
		pPopup->SetAnimationType((CMFCPopupMenu::ANIMATION_TYPE)nAnimation);
		pPopup->SetAnimationSpeed((BYTE)nAnimationSpeed);
		pPopup->SetSmallCaption(bSmallCaption);
		pPopup->SetAutoCloseTime(nAutoCloseTime);

		CMFCDesktopAlertWndInfo params;
		params.m_strText = strAlertText;

		pPopup->Create(NULL,params,NULL,CPoint(-1,-1));
		pPopup->SetWindowText(strDlgCaption);
	}	
}
