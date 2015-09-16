#pragma once


// CCustomStatic

class CPicStatic : public CStatic
{
    DECLARE_DYNAMIC(CPicStatic)

public:
    CPicStatic();
    virtual ~CPicStatic();

    void SetPic(CStringW strPicFilePath);

protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnPaint();

private:
    HBITMAP m_hBmp;
public:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


