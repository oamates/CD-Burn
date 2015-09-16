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
    int m_nGradient;	// �����屳��������ϵ��
    float m_fHeight;  //��ͷ�߶ȣ����Ǳ���,
    int m_nFontHeight; //����߶�
    int m_nFontWith;   //������
    COLORREF m_color;
    CString m_strFormat; //��ʾ�������͵���������,0��ʾ����룬1��ʾ�м���룬2��ʾ�Ҷ���
    CStringArray m_arrStrText;

    COLORREF m_colorUp;
    COLORREF m_colorMiddle;
    COLORREF m_colorDown;
};
