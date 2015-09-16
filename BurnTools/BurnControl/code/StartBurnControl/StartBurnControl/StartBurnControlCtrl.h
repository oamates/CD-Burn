#pragma once

#include <string>

// StartBurnControlCtrl.h : Declaration of the CStartBurnControlCtrl ActiveX Control class.


// CStartBurnControlCtrl : See StartBurnControlCtrl.cpp for implementation.

class CStartBurnControlCtrl : public COleControl
{
	DECLARE_DYNCREATE(CStartBurnControlCtrl)

// Constructor
public:
	CStartBurnControlCtrl();

// Overrides
public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();

// Implementation
protected:
	~CStartBurnControlCtrl();

	DECLARE_OLECREATE_EX(CStartBurnControlCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CStartBurnControlCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CStartBurnControlCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CStartBurnControlCtrl)		// Type name and misc status

// Message maps
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
    LONG StartBurnControl(LPCTSTR strStartInfo);

    enum 
    {
        dispidStrBase64 = 2L,
        dispidStartBurnControl = 1L
    };

private:
    std::string GetTagValue(std::string strContent, std::string strTagName);
    std::string InterfaceCommunicate(std::string strIP, int nPort,const std::string &strContent,std::string strTag);

    std::string GetHostIP();
protected:
    BSTR StrBase64(LPCTSTR strSrc);
};

