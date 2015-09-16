#pragma once

// StartBurnControlPropPage.h : Declaration of the CStartBurnControlPropPage property page class.


// CStartBurnControlPropPage : See StartBurnControlPropPage.cpp for implementation.

class CStartBurnControlPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CStartBurnControlPropPage)
	DECLARE_OLECREATE_EX(CStartBurnControlPropPage)

// Constructor
public:
	CStartBurnControlPropPage();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_STARTBURNCONTROL };

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	DECLARE_MESSAGE_MAP()
};

