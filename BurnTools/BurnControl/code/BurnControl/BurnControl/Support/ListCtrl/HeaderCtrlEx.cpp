#include "StdAfx.h"
#include "HeaderCtrlEx.h"

CHeaderCtrlEx::CHeaderCtrlEx(void)
: m_R(171)
, m_G(199)
, m_B(235)
, m_nGradient(8)
{
    m_strFormat = "";
    m_fHeight = 1;
    m_nFontHeight = 20;
    m_nFontWith = 0;
    m_color = RGB(0,0,0);

    m_colorUp=COLORREF(-1);
    m_colorMiddle=COLORREF(-1);
    m_colorDown=COLORREF(-1);
}

CHeaderCtrlEx::~CHeaderCtrlEx(void)
{
}
BEGIN_MESSAGE_MAP(CHeaderCtrlEx, CHeaderCtrl)
    ON_WM_PAINT()
END_MESSAGE_MAP()

void CHeaderCtrlEx::ClearHead(int nNum)
{
    m_arrStrText.RemoveAt(nNum);
    SendMessage(WM_PAINT,NULL,NULL);
}

void CHeaderCtrlEx::SetBkColor(COLORREF c1,COLORREF c2,COLORREF c3)
{
    m_colorUp=c1;
    m_colorMiddle=c2;
    m_colorDown=c3;
}

void CHeaderCtrlEx::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // TODO: Add your message handler code here
    // Do not call CHeaderCtrl::OnPaint() for painting messages

    CDrawingManager dm(dc);

    int R = m_R,G = m_G,B = m_B;

    CString strFontName;
    LOGFONT lf;
    SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&lf,0);
    strFontName = lf.lfFaceName;

    CFont nFont ,* nOldFont; 
    dc.SetTextColor(m_color);
    nFont.CreateFont(m_nFontHeight,m_nFontWith,0,0,0,FALSE,FALSE,0,0,0,0,0,0,strFontName);//创建字体 
    nOldFont = dc.SelectObject(&nFont);

    int nItem = GetItemCount();//得到有几个单元 
    for(int i = 0; i < nItem; i++) 
    {
        CRect rect;
        GetItemRect(i,&rect);//得到Item的尺寸
        CRect rectNew(rect);//拷贝尺寸到新的容器中 

        //绘制线条
        {
            CRect rectCutOff = rectNew;

            if (i > 0)
            {
                rectCutOff.DeflateRect(0,0,rectNew.Width()-1,0);
            }

            CBrush _brush; 
            _brush.CreateSolidBrush(RGB(255,255,255));//创建画刷 
            dc.FillRect(&rectNew,&_brush); //填充背景 
            _brush.DeleteObject(); //释放画刷 

        }

        rectNew.DeflateRect(1,0,0,0);

        //绘制立体背景 
        if (m_colorUp != COLORREF(-1) &&
            m_colorMiddle != COLORREF(-1) &&
            m_colorDown != COLORREF(-1))
        {
            CRect rectUp=rectNew;
            rectUp.DeflateRect(0,0,0,rectNew.Height()/2);
            dm.FillGradient(rectUp,m_colorMiddle,m_colorUp);

            CRect rectDown=rectNew;
            rectDown.DeflateRect(0,rectNew.Height()/2,0,0);
            dm.FillGradient(rectDown,m_colorDown,m_colorMiddle); 
        } 
        else
        {
            for(int j = rect.top;j<=rect.bottom;j++) 
            { 
                rectNew.bottom = rectNew.top+1; 

                CBrush _brush; 
                _brush.CreateSolidBrush(RGB(R,G,B));//创建画刷 
                dc.FillRect(&rectNew,&_brush); //填充背景 
                _brush.DeleteObject(); //释放画刷 
                R-=m_nGradient;G-=m_nGradient;B-=m_nGradient;
                if (R<0)R = 0;
                if (G<0)G = 0;
                if (B<0)B= 0;

                rectNew.top = rectNew.bottom; 
            } 
        }

        //设置字体

        dc.SetBkMode(TRANSPARENT);

        

        UINT nFormat = 1;
        if (m_strFormat.GetLength()>=i+1 && m_strFormat[i]=='0')
        {
            nFormat = DT_LEFT;
            rect.left+=3;
        }
        else if (m_strFormat.GetLength()>=i+1 && m_strFormat[i]=='1')
        {
            nFormat = DT_CENTER;
        }
        else if (m_strFormat.GetLength()>=i+1 && m_strFormat[i]=='2')
        {
            nFormat = DT_RIGHT;
            rect.right-=3;
        }

        TEXTMETRIC metric;
        dc.GetTextMetrics(&metric);
        int ofst = 0;
        ofst = rect.Height() - metric.tmHeight;
        rect.OffsetRect(0,ofst/2);
        if (m_arrStrText.GetSize()>=i+1 && !m_arrStrText[i].IsEmpty())
        {
            dc.DrawText(m_arrStrText[i],&rect,nFormat);
        }
        
    } 

    dc.SelectObject(nOldFont); 
    nFont.DeleteObject(); //释放字体 

    //画头部剩余部分
    CRect rtRect;
    CRect clientRect;
    if (nItem > 0)
    {
        GetItemRect(nItem - 1,rtRect);

        GetClientRect(clientRect);
        rtRect.left = rtRect.right;
        rtRect.right = clientRect.right;
    }
    else
    {
        GetClientRect(rtRect);
    }

    CRect nRect(rtRect);

    CRect rectLine(rtRect);

    nRect.DeflateRect(1,0,0,0);
    rectLine.DeflateRect(0,0,rtRect.Width()-1,0);

    CBrush _brush; 
    _brush.CreateSolidBrush(RGB(255,255,255));//创建画刷 
    dc.FillRect(&rectLine,&_brush); //填充背景 
    _brush.DeleteObject(); //释放画刷 

    //绘制立体背景 
    if (m_colorUp != COLORREF(-1) &&
        m_colorMiddle != COLORREF(-1) &&
        m_colorDown != COLORREF(-1))
    {
        CRect rectUp=nRect;
        rectUp.DeflateRect(0,0,0,nRect.Height()/2);
        dm.FillGradient(rectUp,m_colorMiddle,m_colorUp);

        CRect rectDown=nRect;
        rectDown.DeflateRect(0,nRect.Height()/2,0,0);
        dm.FillGradient(rectDown,m_colorDown,m_colorMiddle);
    } 
    else
    {
        for(int j = rtRect.top;j<=rtRect.bottom;j++) 
        { 
            nRect.bottom = nRect.top+1; 
            CBrush _brush; 
            _brush.CreateSolidBrush(RGB(R,G,B));//创建画刷 
            dc.FillRect(&nRect,&_brush); //填充背景 
            _brush.DeleteObject(); //释放画刷 
            R-=m_nGradient;G-=m_nGradient;B-=m_nGradient;
            if (R<0)R = 0;
            if (G<0)G = 0;
            if (B<0)B= 0;
            nRect.top = nRect.bottom; 
        } 
    }
}
