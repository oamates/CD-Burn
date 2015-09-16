#ifndef _BURNSERVEREPSONCTRL_H_
#define _BURNSERVEREPSONCTRL_H_

#include <afx.h>

#include "CharsetConvertMFC.h"
#include "../../BurnInfo.h"

#include "EPSONCtrl.h"

class CBurnServerEPSONCtrl : public CEPSONCtrl
{
public:
    CBurnServerEPSONCtrl(void);
    ~CBurnServerEPSONCtrl(void);

    void SetBurnJobInformation(const CNormalBurnJobInfoEx &burnJobInfo);

    virtual int CreateJob(CStringW strJobID,std::vector<std::string> vecData);
    virtual void SetReplaceField();

private:
    CNormalBurnJobInfoEx m_jobInformation;
};
#endif
