// TransParentCheckBox.cpp : implementation file
//

#include "stdafx.h"
#include "TransparentCheckBox.h"


// CTransParentCheckBox

IMPLEMENT_DYNAMIC(CTransparentCheckBox, CButton)

CTransparentCheckBox::CTransparentCheckBox()
{

}

CTransparentCheckBox::~CTransparentCheckBox()
{
}


BEGIN_MESSAGE_MAP(CTransparentCheckBox, CButton)
    ON_WM_PAINT()
END_MESSAGE_MAP()



// CTransParentCheckBox message handlers



void CTransparentCheckBox::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    CRect rect;
    GetClientRect(&rect);

    CRect rectBox;
    rectBox = rect;
    rectBox.right = rectBox.left + 15;
    dc.DrawFrameControl(rectBox, DFC_BUTTON, DFCS_BUTTONCHECK|GetCheck()?DFCS_CHECKED:0);

    CFont *pOldFont = NULL;
    if (GetParent() != NULL)
    {
        pOldFont = dc.SelectObject(GetParent()->GetFont());
    }
    dc.SetBkMode(TRANSPARENT);

    rect.OffsetRect(20,0);
    CString strText;
    GetWindowText(strText);
    dc.DrawText(strText, rect, DT_LEFT|DT_VCENTER|DT_SINGLELINE);   
    if (pOldFont != NULL)
    {
        dc.SelectObject(pOldFont);
    }
}
