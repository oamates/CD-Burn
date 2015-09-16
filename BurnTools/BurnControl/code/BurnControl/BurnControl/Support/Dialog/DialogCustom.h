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
    //类似于MoveWindow(CRect rect),只是移动结束后居中。类似于调用MoveWindow(CRect rect)，CenterWindow()
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
    //初始背景色，用于透明化对话框
    COLORREF        m_rgbInitialBk;
    //边框颜色
    COLORREF        m_rgbFrame;
    //边框渐变色另一种颜色
    COLORREF        m_rgbFrameOther;
    //内部底色
    COLORREF        m_rgbBk;
    //内部底色渐变色另一种颜色
    COLORREF        m_rgbBkOther;
    //X坐标表示绘制外边框圆角的椭圆宽度，Y坐标表示绘制外边框圆角的椭圆高度
    POINT           m_pointFrameAngle;
    //X坐标表示绘制内部底色框圆角的椭圆宽度，Y坐标表示绘制内部底色框框圆角的椭圆高度
    POINT           m_pointBkAngle;
    //内部底色框左上角的Y坐标，即外边框的上边框高度
    int             m_nBkTop;
    //内部底色框左上角的X坐标，即外边框的左右边框宽度
    int             m_nBkLeft;
    //内部底色框右下角的Y坐标距离客户区下边界的大小，即外边框下边框的高度
    int             m_nBkBottom;
    //渐变区域的左上角的X坐标
    int             m_nGradientLeft;
	//渐变区域的左上角的Y坐标
	int             m_nGradientTop;
    //渐变区域的右下角的Y坐标
    int             m_nGradientBottom;
    //内部底色框边界缩小量
    int             m_nBkReduce;
};
