#ifndef _PRIMERA_H_
#define _PRIMERA_H_

#include <afx.h>

#include <string>
#include <vector>

#include "../../BurnInfo.h"

//返回唯一的id
std::string GeneratePrimeraBurnJob(const CNormalBurnJobInfoEx &jobInfo,std::vector<std::string> vecData);


CStringW GetFieldText(CNormalBurnJobInfoEx jobInformation,std::string strFieldID);

bool IsPrimeraOK();

int GetPrimeraJobStatus(CStringW strJobID,CStringW &strErr);//返回值：-1 获取失败；0 任务没有开始；1 任务进行中；2 任务成功完成；3 任务执行失败

#endif
