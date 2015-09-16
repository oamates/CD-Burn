#pragma once
#include "../../resource.h"
#include <ckcore/log.hh>
#include "Log/Log.h"
#include "afxcmn.h"
using namespace ckcore;

// CLogDlg dialog

class CLogDlg : public ckcore::Log
{
public:
	CLogDlg();
	virtual ~CLogDlg();

public:
	virtual void print(const tchar *format,...);
	virtual void print_line(const tchar *format,...);
};