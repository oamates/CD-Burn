#pragma once


// CCustomStatic

class CCustomStatic : public CStatic
{
	DECLARE_DYNAMIC(CCustomStatic)

public:
	CCustomStatic();
	virtual ~CCustomStatic();

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnPaint();

private:
    int m_nDeflateX;
    int m_nDeflateY;
    CPoint m_pointRound;

    CPen m_pen;
    COLORREF m_colorFrameLine;

    CFont m_fontText;
public:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


