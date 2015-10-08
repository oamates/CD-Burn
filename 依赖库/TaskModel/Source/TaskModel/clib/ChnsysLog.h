#ifndef _CHNSYSLOG_H_
#define _CHNSYSLOG_H_

#include "ChnsysTypes.h"

class   ZLog;
extern  ZLog*   g_pLog;

CHNSYS_INT OS_LOG_Init();
CHNSYS_INT OS_LOG_UnInit();

#define OS_LOG_SET_PATH(l)		if(g_pLog)			{g_pLog->SetLogPath(l);}
#define OS_LOG_SET_LEVEL(l)	if(g_pLog)			{g_pLog->SetLogLevel(l);}
#define OS_LOG_ERROR(s)		if(g_pLog)			{g_pLog->LOGError		s;}
#define OS_LOG_WARNING(s)		if(g_pLog)			{g_pLog->LOGWarning	s;}
#define OS_LOG_INFO(s)			if(g_pLog)			{g_pLog->LOGInfo		s;}
#define OS_LOG_DEBUG(s)		if(g_pLog)			{g_pLog->LOGDebug		s;}

#endif //_CHNSYSLOG_H_
