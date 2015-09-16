#pragma once
#include "afxcmn.h"

class CHeaderCtrlEx :
    public CHeaderCtrl
{
public:
    CHeaderCtrlEx(void);
    ~CHeaderCtrlEx(void);
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();

public:
    void ClearHead(int nNum);

    void SetBkColor(COLORREF c1,COLORREF c2,COLORREF c3);


public:
    int m_R;
    int m_G;
    int m_B;
    int m_nGradient;	// 画立体背景，渐变系数
    float m_fHeight;  //表头高度，这是倍数,
    int m_nFontHeight; //字体高度
    int m_nFontWith;   //字体宽度
    COLORREF m_color;
    CString m_strFormat; //表示对齐类型的整型数组,0表示左对齐，1表示中间对齐，2表示右对齐
    CStringArray m_arrStrText;

    COLORREF m_colorUp;
    COLORREF m_colorMiddle;
    COLORREF m_colorDown;
};
