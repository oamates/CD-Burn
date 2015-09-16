#pragma once


// CTransParentCheckBox

class CTransparentCheckBox : public CButton
{
	DECLARE_DYNAMIC(CTransparentCheckBox)

public:
	CTransparentCheckBox();
	virtual ~CTransparentCheckBox();

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnPaint();
};


