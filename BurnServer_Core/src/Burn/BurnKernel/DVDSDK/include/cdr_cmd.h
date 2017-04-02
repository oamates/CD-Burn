//cdr_cmd.h
#ifndef __CDR_CMD_H__
#define __CDR_CMD_H__

#include "LibDVDSDK.h"
#include "drvcomm.h"

int DVDRec_GetCdrcmd(char *pCdrName, struct CDR_CMD_T **pCmd);

#endif//__CDR_CMD_H__
