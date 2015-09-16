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
#include "AdvancedProgress.h"

#include "LogDlg.h"
#include "Core2.h"

class CCore2Format
{
private:
	bool WaitBkgndFormat(ckmmc::Device &Device,CAdvancedProgress *pProgress);

public:
	CCore2Format();
    CCore2Format(CLogDlg *pLogDlg);
	~CCore2Format();

    CLogDlg * GetLogDlg() const;
    void SetLogDlg(CLogDlg * val);
    CLogDlg *m_pLogDlg;

    CCore2 * GetCore2() const;
    void SetCore2(CCore2 * val);
    CCore2 *m_pCore2;

	bool FormatUnit(ckmmc::Device &Device,CAdvancedProgress *pProgress,bool bFull);
};
