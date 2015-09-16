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
    
    int         m_nItem;        //�к�
    int         m_nSubItem;     //�к�
    int         m_nBMPSerial;   //ͼ�����к�
    BUTTON_POS  m_btnPos;       //��ť����λ��
    int         m_nBtnWidth;    //��ť���
    CString     m_strBtnText;   //��ť�ϵ�����
    COLORREF    m_crTextColor;  //������ɫ
    COLORREF    m_crBkColor;    //��Ӧѡ���ɫ
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
    std::vector<ITEM_PRO>   m_vectItemPro;     //ĳһ�������
    std::vector<CBitmap*>    m_vectpBmp;          //ͼ�꼯��
    CHeaderCtrlEx           m_Header;           //��ͷ

    CEdit                   *m_pEdit;
public:
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
