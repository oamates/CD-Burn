#include "StdAfx.h"
#include "PictureListCtrl.h"

CPictureListCtrl::CPictureListCtrl(void)
{
    m_vectItemPro.clear();
    m_vectpBmp.clear();

    m_pEdit=NULL;
}

CPictureListCtrl::~CPictureListCtrl(void)
{
    m_vectItemPro.clear();
    while(m_vectpBmp.size() > 0)
    {
        (*m_vectpBmp.begin())->DeleteObject();
        delete *m_vectpBmp.begin();
        m_vectpBmp.erase(m_vectpBmp.begin());
    }
}

BEGIN_MESSAGE_MAP(CPictureListCtrl, CListCtrl)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CPictureListCtrl::OnNMCustomdraw)
    ON_WM_HSCROLL()
    ON_WM_VSCROLL()
END_MESSAGE_MAP()


// Gradient - 渐变系数，立体背景用,不用渐变设为0
void CPictureListCtrl::SetHeaderBKColor(int R, int G, int B, int Gradient) //设置表头背景色
{
    m_Header.m_R = R;
    m_Header.m_G = G;
    m_Header.m_B = B;
    m_Header.m_nGradient = Gradient;
}

void CPictureListCtrl::SetHeaderBKColor(COLORREF c1,COLORREF c2,COLORREF c3)
{
    m_Header.SetBkColor(c1,c2,c3);
}

void CPictureListCtrl::SetItemPro(ITEM_PRO itemPro)
{
    std::vector<ITEM_PRO>::iterator iter = FindItemExist(itemPro.m_nItem,itemPro.m_nSubItem);
    if (iter == m_vectItemPro.end())
    {
        m_vectItemPro.push_back(itemPro);
    }
    else
    {
        (*iter) = itemPro;
    }
    CRect rect;
    GetSubItemRect(itemPro.m_nItem,itemPro.m_nSubItem,LVIR_BOUNDS,rect);
    InvalidateRect(rect);
    UpdateWindow();
}

void CPictureListCtrl::SetBitmap(std::vector<CString> vectStrBitmapPath)
{
    while(m_vectpBmp.size() > 0)
    {
        (*m_vectpBmp.begin())->DeleteObject();
        delete *m_vectpBmp.begin();
        m_vectpBmp.erase(m_vectpBmp.begin());
    }

    HBITMAP hBitmap;
    for (size_t i = 0; i < vectStrBitmapPath.size(); i++)
    {
        CBitmap* pBitMap = new CBitmap;
        hBitmap=(HBITMAP)LoadImage(AfxGetInstanceHandle(),vectStrBitmapPath[i],IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
        if(!pBitMap->Attach(hBitmap))
        {
            ASSERT(0);    
        }
        else
        {
            m_vectpBmp.push_back(pBitMap);
        }
    }
}

BOOL CPictureListCtrl::DeleteColumn(_In_ int nCol)
{
    m_Header.ClearHead(nCol);
    return CListCtrl::DeleteColumn(nCol);
}

BOOL CPictureListCtrl::DeleteItem(_In_ int nItem)
{
    std::vector<ITEM_PRO>::iterator iter = m_vectItemPro.begin();

    while(iter != m_vectItemPro.end())
    {
        if ((*iter).m_nItem == nItem)
        {
            iter = m_vectItemPro.erase(iter);
        }
        else if ((*iter).m_nItem > nItem)
        {
            (*iter).m_nItem--;
            iter++;
        }
        else
        {
            iter++;
        }
    }
    return CListCtrl::DeleteItem(nItem);
}

BOOL CPictureListCtrl::DeleteAllColumn()
{
    while (GetHeaderCtrl()->GetItemCount() > 0)
    {
        DeleteColumn(0);
    }
    return TRUE;
}

int CPictureListCtrl::InsertColumn(_In_ int nCol, _In_z_ LPCTSTR lpszColumnHeading, _In_ int nFormat , _In_ int nWidth , _In_ int nSubItem )
{
    m_Header.m_arrStrText.Add(lpszColumnHeading);

    if (nFormat==LVCFMT_LEFT)
    {
        m_Header.m_strFormat = m_Header.m_strFormat + _T("0");
    }
    else if (nFormat==LVCFMT_CENTER)
    {
        m_Header.m_strFormat = m_Header.m_strFormat + _T("1");
    }
    else if (nFormat==LVCFMT_RIGHT)
    {
        m_Header.m_strFormat = m_Header.m_strFormat + _T("2");
    }
    else
    {
        m_Header.m_strFormat = m_Header.m_strFormat + _T("1");
    }

    return CListCtrl::InsertColumn(nCol,lpszColumnHeading,nFormat,nWidth,nSubItem);
}

void CPictureListCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );
    // Take the default processing unless we set this to something else below.    
    *pResult = CDRF_DODEFAULT;
    // First thing - check the draw stage. If it's the control's prepaint    
    // stage, then tell Windows we want messages for every item.     
    if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
    {
        *pResult = CDRF_NOTIFYITEMDRAW;
    }    
    else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )        
    {
        // This is the notification message for an item. We'll request        
        // notifications before each subitem's prepaint stage.         
        *pResult = CDRF_NOTIFYSUBITEMDRAW;        
    }    
    else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )        
    {
        int nItem=static_cast<int>(pLVCD->nmcd.dwItemSpec);
        
        std::vector<ITEM_PRO>::iterator iter = FindItemExist(nItem,pLVCD->iSubItem);
        if (iter == m_vectItemPro.end())
        {
        }
        else
        {
            DrawTextAndButton(CDC::FromHandle(pLVCD->nmcd.hdc),*iter);
            *pResult = CDRF_SKIPDEFAULT; // We've painted everything.
            return;
        }   
        
        // Tell Windows to paint the control itself.        
        *pResult = CDRF_DODEFAULT;        
    }
}


void CPictureListCtrl::PreSubclassWindow()
{
    // TODO: Add your specialized code here and/or call the base class
    if(GetHeaderCtrl()) 
    {   
        m_Header.SubclassWindow(GetHeaderCtrl()->m_hWnd); 
    }

    CListCtrl::PreSubclassWindow();
}

void CPictureListCtrl::DrawTextAndButton(CDC *pDC,ITEM_PRO itemPro)
{
    //CDrawingManager dm(*pDC);
    CRect rect;
    GetSubItemRect(itemPro.m_nItem, itemPro.m_nSubItem, LVIR_BOUNDS, rect);

    pDC->FillSolidRect(&rect,itemPro.m_crBkColor);
    
    CString strText = GetItemText(itemPro.m_nItem,itemPro.m_nSubItem);
    CRect rectText=rect;
    CRect rectButton=rect;
    if (itemPro.m_btnPos == BTN_RIGHT)
    {
        rectText.DeflateRect(0,0,itemPro.m_nBtnWidth,0);
        rectButton.DeflateRect(rect.Width()-itemPro.m_nBtnWidth,1,1,1);
    }
    else if (itemPro.m_btnPos == BTN_LEFT)
    {
        rectText.DeflateRect(itemPro.m_nBtnWidth,0,0,0);
        rectButton.DeflateRect(1,1,rect.Width()-itemPro.m_nBtnWidth,1);
    }
    else if (itemPro.m_btnPos == BTN_ON)
    {
        rectButton.DeflateRect((rect.Width()-itemPro.m_nBtnWidth)/2,1,(rect.Width()-itemPro.m_nBtnWidth)/2,1);
        rectText = rectButton;
    }

    COLORREF oldColor = pDC->SetTextColor(itemPro.m_crTextColor);
    UINT nFormat = DT_VCENTER | DT_SINGLELINE | DT_CENTER;
    pDC->DrawText(strText,&rectText,nFormat); 
    pDC->SetTextColor(oldColor);

    CBrush *pOldBrush = NULL;
    if (itemPro.m_nBMPSerial != -1 && itemPro.m_nBMPSerial < (int)m_vectpBmp.size())
    {
//         CBrush brushBtn(&(*m_vectpBmp[itemPro.m_nBMPSerial]));
//         pOldBrush=pDC->SelectObject(&brushBtn);
//         pDC->FillRect(&rectButton,&brushBtn);
//         pDC->SelectObject(pOldBrush); 

        BITMAP bitmap;
        m_vectpBmp[itemPro.m_nBMPSerial]->GetObject(sizeof(bitmap),&bitmap);
        CDC dcBitmap;
        dcBitmap.CreateCompatibleDC(pDC);
        CBitmap *pOldBmp=dcBitmap.SelectObject(&(*m_vectpBmp[itemPro.m_nBMPSerial]));

        pDC->StretchBlt(rectButton.left,rectButton.top,rectButton.Width(),rectButton.Height(),
            &dcBitmap,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY);

        dcBitmap.SelectObject(pOldBrush); 
    }
    pDC->SelectObject(pOldBrush);
    pDC->DrawText(itemPro.m_strBtnText,&rectButton,nFormat);
}

std::vector<ITEM_PRO>::iterator CPictureListCtrl::FindItemExist(int nItem, int nSubItem)
{
    std::vector<ITEM_PRO>::iterator iter = m_vectItemPro.begin();
    while(iter != m_vectItemPro.end())
    {
        if ((nItem == (*iter).m_nItem) && (nSubItem == (*iter).m_nSubItem))
        {
            break;
        }
        else
        {
            iter++;
        }
    }
    return iter;
}

void CPictureListCtrl::SetEdit(CEdit *pEdit)
{
    m_pEdit=pEdit;
}

void CPictureListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: Add your message handler code here and/or call default

    if (m_pEdit != NULL)
    {
        if (m_pEdit->GetSafeHwnd() != NULL)
        {
            SetFocus();
        }
    }

    CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPictureListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: Add your message handler code here and/or call default

    if (m_pEdit != NULL)
    {
        if (m_pEdit->GetSafeHwnd() != NULL)
        {
            SetFocus();
        }
    }

    CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}
