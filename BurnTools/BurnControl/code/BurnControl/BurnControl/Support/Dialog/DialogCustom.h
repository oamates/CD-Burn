#pragma once

#include "CDialogSK.h"
#include "BtnST.h"

class CDialogCustom :
    public CDialogSK
{
public:
    void SetMaxButtonVisible(BOOL bVisible);
    void SetMinButtonVisible(BOOL bVisible);
    void SetCloseButtonVisible(BOOL bVisible);
    void SetIgnoreESC(BOOL bIgnore = TRUE);
    void SetIgnoreReturn(BOOL bIgnore = TRUE);
    void ShowMaximize();
    //������MoveWindow(CRect rect),ֻ���ƶ���������С������ڵ���MoveWindow(CRect rect)��CenterWindow()
    void MoveCenterWindow(CRect rect);

protected:
    void InitParameter();
    int CalculateRect();
    void Relayout();
    BOOL OwnerDrawBackground(CDC* pDC);
public:
    DECLARE_MESSAGE_MAP()
    virtual BOOL OnInitDialog();

    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedButtonMin();
    afx_msg void OnBnClickedButtonMax();
    afx_msg void OnBnClickedButtonClose();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
    CDialogCustom(CWnd* pParent = NULL);
    CDialogCustom(UINT uResourceID, CWnd* pParent = NULL);
    CDialogCustom(LPCTSTR pszResourceID, CWnd* pParent = NULL);
    virtual  ~CDialogCustom();
protected:
    CButtonST       m_btnMin;
    CRect           m_rectBtnMin;

    CButtonST       m_btnMax;
    CRect           m_rectBtnMax;

    CButtonST       m_btnClose;
    CRect           m_rectBtnClose;

    BOOL            m_bShowButtonMin;
    BOOL            m_bShowButtonMax;
    BOOL            m_bShowButtonClose;

    BOOL            m_bIgnoreESC;
    BOOL            m_bIgnoreReturn;
protected:
    HICON           m_hIcon;
	HICON           m_hIconBig;
    //��ʼ����ɫ������͸�����Ի���
    COLORREF        m_rgbInitialBk;
    //�߿���ɫ
    COLORREF        m_rgbFrame;
    //�߿򽥱�ɫ��һ����ɫ
    COLORREF        m_rgbFrameOther;
    //�ڲ���ɫ
    COLORREF        m_rgbBk;
    //�ڲ���ɫ����ɫ��һ����ɫ
    COLORREF        m_rgbBkOther;
    //X�����ʾ������߿�Բ�ǵ���Բ��ȣ�Y�����ʾ������߿�Բ�ǵ���Բ�߶�
    POINT           m_pointFrameAngle;
    //X�����ʾ�����ڲ���ɫ��Բ�ǵ���Բ��ȣ�Y�����ʾ�����ڲ���ɫ���Բ�ǵ���Բ�߶�
    POINT           m_pointBkAngle;
    //�ڲ���ɫ�����Ͻǵ�Y���꣬����߿���ϱ߿�߶�
    int             m_nBkTop;
    //�ڲ���ɫ�����Ͻǵ�X���꣬����߿�����ұ߿���
    int             m_nBkLeft;
    //�ڲ���ɫ�����½ǵ�Y�������ͻ����±߽�Ĵ�С������߿��±߿�ĸ߶�
    int             m_nBkBottom;
    //������������Ͻǵ�X����
    int             m_nGradientLeft;
	//������������Ͻǵ�Y����
	int             m_nGradientTop;
    //������������½ǵ�Y����
    int             m_nGradientBottom;
    //�ڲ���ɫ��߽���С��
    int             m_nBkReduce;
};
