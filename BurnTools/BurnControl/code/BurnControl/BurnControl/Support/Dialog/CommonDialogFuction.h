#pragma once

#ifndef __AFXWIN_H__
#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif


//�������ܣ�����������ʾ�Ի���
//���룺strAlertText����ʾ���е����ݣ�strDlgCaption����ʾ�������ʾ���ƣ�bSmallCaption����ʾ���������С����
//      nAnimation����ʾ�򵯳�������nAnimationSpeed����ʾ�򶯻��ٶ�(2....200)��
//      nAutoCloseTime����ʾ���Զ��ر�ʱ��(0Ϊ���Զ��ر�)��nVisualMngr����ʾ����(0...8)��
void DesktopAlertPopup(CString strAlertText,CString strDlgCaption = _T("Message"),BOOL bSmallCaption = FALSE,
					   int nAnimation = 2,int nAnimationSpeed = 100,int nAutoCloseTime =0,int nVisualMngr = 3);
