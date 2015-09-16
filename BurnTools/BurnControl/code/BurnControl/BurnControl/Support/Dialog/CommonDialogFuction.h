#pragma once

#ifndef __AFXWIN_H__
#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif


//函数功能：弹出桌面提示对话框
//输入：strAlertText，提示框中的内容；strDlgCaption，提示框标题显示名称；bSmallCaption，提示框标题栏最小化；
//      nAnimation，提示框弹出动画；nAnimationSpeed，提示框动画速度(2....200)；
//      nAutoCloseTime，提示框自动关闭时间(0为不自动关闭)；nVisualMngr，提示框风格(0...8)；
void DesktopAlertPopup(CString strAlertText,CString strDlgCaption = _T("Message"),BOOL bSmallCaption = FALSE,
					   int nAnimation = 2,int nAnimationSpeed = 100,int nAutoCloseTime =0,int nVisualMngr = 3);
