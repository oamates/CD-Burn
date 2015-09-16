/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <ckmmc/device.hh>
#include <ckmmc/devicemanager.hh>

#include "Core.h"
#include "ProgressDlg.h"
#include "Core2.h"
#include "Settings.h"
#include "ProjectManager.h"

class CActionManager
{
private:
	class CEraseParam
	{
	public:
		bool m_bNotifyCompleted;

        void *m_pCustom;

		CEraseParam(bool bNotifyCompleted) : m_bNotifyCompleted(bNotifyCompleted),m_pCustom(NULL) {}
	};

	static DWORD WINAPI BurnCompilationThread(LPVOID lpThreadParameter);
//	static DWORD WINAPI CreateImageThread(LPVOID lpThreadParameter);
//	static DWORD WINAPI CopyDiscOnFlyThread(LPVOID lpThreadParameter);
//	static DWORD WINAPI CopyDiscThread(LPVOID lpThreadParameter);
	static DWORD WINAPI EraseThread(LPVOID lpThreadParameter);

	void QuickErase(ckmmc::Device &Device);
	bool QuickEraseQuery(ckmmc::Device &Device,HWND hWndParent);

public:
	CActionManager();
	~CActionManager();

	INT_PTR BurnCompilation(HWND hWndParent);
//	INT_PTR CreateImage(HWND hWndParent,bool bAppMode);
//	INT_PTR BurnImage(HWND hWndParent,bool bAppMode);
//	INT_PTR BurnImageEx(HWND hWndParent,bool bAppMode,const TCHAR *szFilePath);
//	INT_PTR CopyDisc(HWND hWndParent,bool bAppMode);
//	INT_PTR CopyImage(HWND hWndParent,bool bAppMode);
//	INT_PTR ManageTracks(bool bAppMode);
//	INT_PTR Erase(HWND hWndParent,bool bAppMode);
//	INT_PTR Fixate(HWND hWndParent,bool bAppMode);

	// For testing purposes only.
	//int VerifyCompilation(HWND hWndParent);
	// 
	
    CCore * GetCore();
    void SetCore(CCore *pCore);
    CCore *m_pCore;

    CCore2 * GetCore2() const;
    void SetCore2(CCore2 * val);
    CCore2 *m_pCore2;

    CProgressDlg * GetProgressDlg();
    void SetProgressDlg(CProgressDlg *pProgressDlg);
    CProgressDlg * m_pProgressDlg;

    CBurnImageSettings * GetBurnImageSettings() const;
    void SetBurnImageSettings(CBurnImageSettings * val);
    CBurnImageSettings *m_pBurnImageSettings;

    CEraseSettings & GetEraseSettings();
    void SetEraseSettings(CEraseSettings val);
    CEraseSettings m_eraseSettings;

    CProjectManager * GetProjectManager() const;
    void SetProjectManager(CProjectManager * val);
    CProjectManager *m_pProjectManager;

    CTreeManager * GetTreeManager() const;
    void SetTreeManager(CTreeManager * val);
    CTreeManager *m_pTreeManager;

    CProjectSettings * GetProjectSettings() const;
    void SetProjectSettings(CProjectSettings * val);
    CProjectSettings *m_pProjectSettings;

    ckmmc::DeviceManager * GetDeviceManager() const;
    void SetDeviceManager(ckmmc::DeviceManager * val);
    ckmmc::DeviceManager *m_pDeviceManager;

};

//extern CActionManager g_ActionManager;
