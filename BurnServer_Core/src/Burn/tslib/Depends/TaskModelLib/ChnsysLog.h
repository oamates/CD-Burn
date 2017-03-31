#ifndef _CHNSYSLOG_H_
#define _CHNSYSLOG_H_

#include "ChnsysTypes.h"

class   ZLog;
extern  ZLog*   pLog;

CHNSYS_INT OS_LOG_Init();
CHNSYS_INT OS_LOG_UnInit();

#define OS_LOG_SET_PATH(l)		if(pLog)			{pLog->SetLogPath(l);}
#define OS_LOG_SET_LEVEL(l)	if(pLog)			{pLog->SetLogLevel(l);}
#define OS_LOG_ERROR(s)		if(pLog)			{pLog->LOGError		s;}
#define OS_LOG_WARNING(s)		if(pLog)			{pLog->LOGWarning	s;}
#define OS_LOG_INFO(s)			if(pLog)			{pLog->LOGInfo		s;}
#define OS_LOG_DEBUG(s)		if(pLog)			{pLog->LOGDebug		s;}

#endif //_CHNSYSLOG_H_
