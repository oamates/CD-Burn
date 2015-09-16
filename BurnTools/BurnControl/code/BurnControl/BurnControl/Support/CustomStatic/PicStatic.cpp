// CustomStatic.cpp : implementation file
//

#include "stdafx.h"
#include "PicStatic.h"


// CPicStatic

IMPLEMENT_DYNAMIC(CPicStatic, CStatic)

CPicStatic::CPicStatic():
m_hBmp(NULL)
{ 
}

CPicStatic::~CPicStatic()
{
}

void CPicStatic::SetPic(CStringW strPicFilePath)
{
    m_hBmp=(HBITMAP)(::LoadImageW(NULL,strPicFilePath,IMAGE_BITMAP,0,0,LR_LOADFROMFILE));
}


BEGIN_MESSAGE_MAP(CPicStatic, CStatic)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CPicStatic message handlers



void CPicStatic::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // TODO: Add your message handler code here
    // Do not call CStatic::OnPaint() for painting messages

}

BOOL CPicStatic::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default

    CRect rect;
    GetClientRect(&rect);

    if (m_hBmp != NULL)
    {
        BITMAP bmp;
        CBitmap::FromHandle(m_hBmp)->GetBitmap(&bmp);

        int nWidth=bmp.bmWidth;
        int nHeight=bmp.bmHeight;
        if (rect.Width() < nWidth)
        {
            nWidth=rect.Width();
        }
        if (rect.Height() < nHeight)
        {
            nHeight=rect.Height();
        }

        CDC memDC;
        memDC.CreateCompatibleDC(pDC);

        CBitmap *pOldBmp=memDC.SelectObject(CBitmap::FromHandle(m_hBmp));

        pDC->TransparentBlt(0,0,nWidth,nHeight,&memDC,0,0,nWidth,nHeight,RGB(255,255,255));

        memDC.SelectObject(pOldBmp);

        DeleteDC(memDC.GetSafeHdc());
    }

    return TRUE;
    //return CStatic::OnEraseBkgnd(pDC);
}
