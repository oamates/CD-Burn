#pragma once
#include "HeaderCtrlEx.h"
#include <vector>

enum BUTTON_POS
{
    BTN_LEFT,
    BTN_RIGHT,
    BTN_ON
};

class ITEM_PRO
{
public:
    
    ITEM_PRO()
    {
        m_nBMPSerial = -1;
        m_nItem = -1;
        m_nSubItem = -1;
        m_btnPos = BTN_RIGHT;
        m_nBtnWidth = 22;
        m_strBtnText = _T("");
        m_crBkColor = GetSysColor(COLOR_WINDOW);
        m_crTextColor = RGB(0,0,0);
    }

    ~ITEM_PRO()
    {
    }

    ITEM_PRO(const ITEM_PRO &itemPro)
    {
        m_nItem         = itemPro.m_nItem;
        m_nSubItem      = itemPro.m_nSubItem;
        m_nBMPSerial    = itemPro.m_nBMPSerial;
        m_btnPos        = itemPro.m_btnPos;
        m_nBtnWidth     = itemPro.m_nBtnWidth;
        m_strBtnText    = itemPro.m_strBtnText;
        m_crTextColor   = itemPro.m_crTextColor;
        m_crBkColor     = itemPro.m_crBkColor;
    }
    const ITEM_PRO & operator = (const ITEM_PRO &itemPro)
    {
        m_nItem         = itemPro.m_nItem;
        m_nSubItem      = itemPro.m_nSubItem;
        m_nBMPSerial    = itemPro.m_nBMPSerial;
        m_btnPos        = itemPro.m_btnPos;
        m_nBtnWidth     = itemPro.m_nBtnWidth;
        m_strBtnText    = itemPro.m_strBtnText;
        m_crTextColor   = itemPro.m_crTextColor;
        m_crBkColor     = itemPro.m_crBkColor;
        return *this;
    }
    
    int         m_nItem;        //行号
    int         m_nSubItem;     //列号
    int         m_nBMPSerial;   //图标序列号
    BUTTON_POS  m_btnPos;       //按钮所在位置
    int         m_nBtnWidth;    //按钮宽度
    CString     m_strBtnText;   //按钮上的文字
    COLORREF    m_crTextColor;  //字体颜色
    COLORREF    m_crBkColor;    //对应选项背景色
};

class CPictureListCtrl :
    public CListCtrl
{
public:
    void SetHeaderBKColor(int R, int G, int B, int Gradient);

    void SetHeaderBKColor(COLORREF c1,COLORREF c2,COLORREF c3);

    void SetItemPro(ITEM_PRO itemPro);
    void SetBitmap(std::vector<CString> vectStrBitmapPath);
    BOOL DeleteItem(_In_ int nItem);
    BOOL DeleteColumn(_In_ int nCol);
    BOOL DeleteAllColumn();
    int  InsertColumn(_In_ int nCol, _In_z_ LPCTSTR lpszColumnHeading, _In_ int nFormat  = LVCFMT_CENTER , _In_ int nWidth  = -1 , _In_ int nSubItem  = -1 );
public:
    CPictureListCtrl(void);
    ~CPictureListCtrl(void);
    DECLARE_MESSAGE_MAP()
    afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
    virtual void PreSubclassWindow();
public:
    void SetEdit(CEdit *pEdit);
private:
     void DrawTextAndButton(CDC *pDC, ITEM_PRO itemPro);
     std::vector<ITEM_PRO>::iterator FindItemExist(int nItem, int nSubItem);
     
private:
    std::vector<ITEM_PRO>   m_vectItemPro;     //某一项的属性
    std::vector<CBitmap*>    m_vectpBmp;          //图标集合
    CHeaderCtrlEx           m_Header;           //表头

    CEdit                   *m_pEdit;
public:
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
