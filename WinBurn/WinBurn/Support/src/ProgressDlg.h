#pragma once
#include "../../resource.h"
#include "AdvancedProgress.h"
#include "afxwin.h"
#include "afxcmn.h"
#include <ckmmc/device.hh>
#include "DeviceUtil.h"
using namespace ckcore;
// CProgressDlg dialog

class CProgressDlg : public CAdvancedProgress
{

public:
	CProgressDlg();   // standard constructor
	virtual ~CProgressDlg();

private:
	unsigned char m_ucPercent;

public:
	/**
	* Notifies change in progress.
	* @param [in] percent The progress in percent.
	*/
	virtual void set_progress(unsigned char percent);

	/**
	* Enables or disables marquee progress reporting
	* @param [in] enable Set to true to enable marquee mode or set to false to
	*			   disable the marquee mode.
	*/
	virtual void set_marquee(bool enable) {};

	/**
	* Sets the status message describing the current operation.
	* @param [in] format The status message format.
	*/
	void set_status(const TCHAR *szStatus,...);

	/**
	* Transmits a message to the interface implementor. This message is
	* intended to be displayed to the end user.
	* @param [in] type The type of message.
	* @param [in] format The message format.
	*/
	virtual void notify(ckcore::Progress::MessageType Type,const TCHAR *szMessage,...);

	/**
	* Checks wether the operation has been cancelled or not.
	* @return If the process has been cancelled true is returned, if not
	*		   cancelled false is returned.
	*/
	virtual bool cancelled();

	// Called when the operation is complteted.
	virtual void NotifyCompleted();

	// Should be set to true when a real writing process is started.
	virtual void SetRealMode(bool bRealMode);

	// Not forced to be implemented by inheritor.
	/*virtual void SetBuffer(int iPercent);*/

	virtual void AllowReload();
	virtual void AllowCancel(bool bAllow);

	virtual bool RequestNextDisc();

	// Starts the smoke effect.
	virtual void StartSmoke();

private:	
	CStatic m_TotalStatic;
	CStatic m_StatusStatic;

    TCHAR m_szStringBuffer[1024];
	
//	CString			m_sProgressMsg;
//	CRichEditCtrl m_re2ProgressMsg;

	bool			m_bCancelled;

public:
	void SetDevice(const TCHAR *szDevice);
	void SetDevice(ckmmc::Device &Device);

public:
	afx_msg void OnNMRClickListProgressmsg(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBurnclearlog();

    int GetBurnState() const;
    void SetBurnState(int val);
    int m_nBurnState;//-1£º¿ÌÂ¼Ê§°Ü£»0£º³õÊ¼×´Ì¬£»1£ºÕýÔÚ¿ÌÂ¼£»2£º¿ÌÂ¼Íê³É

    const wchar_t * GetBurnStateDescription() const;

    void SetBurnStateDescription(const wchar_t *s);

};

/*extern CProgressDlg * g_pProgressDlg;*/