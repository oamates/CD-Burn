// CustomStatic.cpp : implementation file
//

#include "stdafx.h"
#include "CustomStatic.h"


// CCustomStatic

IMPLEMENT_DYNAMIC(CCustomStatic, CStatic)

CCustomStatic::CCustomStatic():
m_nDeflateX(2),
m_nDeflateY(2),
m_pointRound(4,4),
m_colorFrameLine(RGB(128,128,128))
{
    /*
    typedef struct tagLOGFONT { 
    LONG lfHeight; 
    LONG lfWidth; 
    LONG lfEscapement; 
    LONG lfOrientation; 
    LONG lfWeight; 
    BYTE lfItalic; 
    BYTE lfUnderline; 
    BYTE lfStrikeOut; 
    BYTE lfCharSet; 
    BYTE lfOutPrecision; 
    BYTE lfClipPrecision; 
    BYTE lfQuality; 
    BYTE lfPitchAndFamily; 
    TCHAR lfFaceName[LF_FACESIZE]; 
    } LOGFONT, *PLOGFONT; 
    */
    LOGFONT lf;
    ZeroMemory(&lf,sizeof(lf));

    lf.lfHeight=12;
    lf.lfWidth=6;
    lf.lfEscapement=0;
    lf.lfOrientation=0;
    lf.lfWeight=FW_THIN;
    lf.lfItalic=FALSE;
    lf.lfUnderline=FALSE;
    lf.lfStrikeOut=FALSE;
    lf.lfCharSet=/*GB2312_CHARSET*/DEFAULT_CHARSET;
    lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
    lf.lfQuality=DEFAULT_QUALITY;
    lf.lfPitchAndFamily=DEFAULT_PITCH | FF_DONTCARE;
    _tcscpy_s(lf.lfFaceName,sizeof(lf.lfFaceName)/sizeof(TCHAR),_T("ÐÂËÎÌå"));
    m_fontText.CreateFontIndirect(&lf);
}

CCustomStatic::~CCustomStatic()
{
    m_fontText.DeleteObject();
}


BEGIN_MESSAGE_MAP(CCustomStatic, CStatic)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CCustomStatic message handlers



void CCustomStatic::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // TODO: Add your message handler code here
    // Do not call CStatic::OnPaint() for painting messages



    CRect rect;
    GetClientRect(&rect);

    CString strText;
    GetWindowText(strText);

    dc.SelectObject(&m_fontText);
    CSize size=dc.GetTextExtent(strText);

    CRect rectFrame=rect;
    rectFrame.DeflateRect(m_nDeflateX,size.cy/2,m_nDeflateX,size.cy/2);

    LOGPEN logPen;
    ZeroMemory(&logPen,sizeof(logPen));
    logPen.lopnColor=m_colorFrameLine;
    logPen.lopnStyle=PS_SOLID;
    POINT point;
    point.x=1;
    point.y=0;
    logPen.lopnWidth=point;
    if (m_pen.GetSafeHandle() != NULL)
    {
        m_pen.DeleteObject();
    }
    m_pen.CreatePenIndirect(&logPen);
    CPen *pOldPen=dc.SelectObject(&m_pen);
    //dc.RoundRect(&rectFrame,m_pointRound);

    CRect rectText;
    rectText.left=rectFrame.left+20;
    rectText.top=rectFrame.top-size.cy/2+1;
    rectText.right=rectText.left+size.cx;
    rectText.bottom=rectFrame.top+size.cy/2+1;

#if 1
    //Left
    dc.MoveTo(rectFrame.left,rectFrame.bottom-m_pointRound.x);
    dc.LineTo(rectFrame.left,rectFrame.top+m_pointRound.x-2);

    dc.Arc(rectFrame.left,rectFrame.top,
        rectFrame.left+2*m_pointRound.x,rectFrame.top+2*m_pointRound.x,
        rectFrame.left+m_pointRound.x,rectFrame.top,
        rectFrame.left,rectFrame.top+m_pointRound.x);

    //Top-Left
    dc.MoveTo(rectFrame.left+m_pointRound.x,rectFrame.top);
    dc.LineTo(rectText.left-2,rectFrame.top);

    //Top-Right
    dc.MoveTo(rectText.right+2,rectFrame.top);
    dc.LineTo(rectFrame.right-m_pointRound.x,rectFrame.top);

    dc.Arc(rectFrame.right-2*m_pointRound.x,rectFrame.top,
        rectFrame.right,rectFrame.top+2*m_pointRound.x,
        rectFrame.right,rectFrame.top+m_pointRound.x+1,
        rectFrame.right-m_pointRound.x,rectFrame.top);

    //Right
    dc.MoveTo(rectFrame.right,rectFrame.bottom-m_pointRound.x-1);
    dc.LineTo(rectFrame.right,rectFrame.top+m_pointRound.x);

    dc.Arc(rectFrame.right-2*m_pointRound.x,rectFrame.bottom-2*m_pointRound.x,
        rectFrame.right,rectFrame.bottom,
        rectFrame.right-m_pointRound.x,rectFrame.bottom,
        rectFrame.right,rectFrame.bottom-m_pointRound.x);

    //Bottom
    dc.MoveTo(rectFrame.left+m_pointRound.x+1,rectFrame.bottom);
    dc.LineTo(rectFrame.right-m_pointRound.x-1,rectFrame.bottom);

    dc.Arc(rectFrame.left,rectFrame.bottom-2*m_pointRound.x,
        rectFrame.left+2*m_pointRound.x,rectFrame.bottom,
        rectFrame.left,rectFrame.bottom-m_pointRound.x,
        rectFrame.left+m_pointRound.x+1,rectFrame.bottom);
#endif

    dc.SelectObject(pOldPen);

    pOldPen=dc.SelectObject(&m_pen);
    dc.SetBkMode(TRANSPARENT);
    dc.DrawText(strText,&rectText,DT_LEFT);
    dc.SelectObject(pOldPen);
}

BOOL CCustomStatic::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default
    return TRUE;
    //return CStatic::OnEraseBkgnd(pDC);
}
