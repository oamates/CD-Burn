#ifndef _PRIMERA_H_
#define _PRIMERA_H_

#include <afx.h>

#include <string>
#include <vector>

#include "../../BurnInfo.h"

//����Ψһ��id
std::string GeneratePrimeraBurnJob(const CNormalBurnJobInfoEx &jobInfo,std::vector<std::string> vecData);


CStringW GetFieldText(CNormalBurnJobInfoEx jobInformation,std::string strFieldID);

bool IsPrimeraOK();

int GetPrimeraJobStatus(CStringW strJobID,CStringW &strErr);//����ֵ��-1 ��ȡʧ�ܣ�0 ����û�п�ʼ��1 ��������У�2 ����ɹ���ɣ�3 ����ִ��ʧ��

#endif
