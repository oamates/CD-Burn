#ifndef __PLATFORM_H__
#define __PLATFORM_H__
#include <Burn_SDK.h>
#include <Database.h>

BURN_INFO_T* get_burn_database(DEV_HANDLE hBurnDEV);

BURN_DEV_INFO_T* get_dev_info(DEV_HANDLE hBurnDEV);

BURN_DISC_INFO_T* get_disc_info(DEV_HANDLE hBurnDEV);

BURN_DISK_INFO_T* get_disk_info(DEV_HANDLE hBurnDEV);

BURN_RUN_STATE_T* get_run_state(DEV_HANDLE hBurnDEV);

BURN_PARAM_T* get_burn_param(DEV_HANDLE hBurnDEV);

BURN_FILE_T* get_burn_file_state(DEV_HANDLE hBurnDEV);

BURN_DATA_T* get_burn_data(DEV_HANDLE hBurnDEV);



#endif
