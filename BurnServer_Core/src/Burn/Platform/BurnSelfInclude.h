#ifndef __BURN_SELF_INCLUDE_H__
#define __BURN_SELF_INCLUDE_H__

/*Database.c*/
BURN_DATA_T* get_burn_data(DEV_HANDLE hBurnDEV);
int create_dev_database(DEV_HANDLE hBurnDEV);
int remove_dev_database(DEV_HANDLE hBurnDEV);
BURN_INFO* get_burn_database(DEV_HANDLE hBurnDEV);
BURN_DEV_INFO* get_dev_info(DEV_HANDLE hBurnDEV);
BURN_DISC_INFO* get_disc_info(DEV_HANDLE hBurnDEV);
BURN_DISK_INFO* get_disk_info(DEV_HANDLE hBurnDEV);
BURN_RUN_STATE* get_run_state(DEV_HANDLE hBurnDEV);
BURN_PARAM* get_burn_param(DEV_HANDLE hBurnDEV);
BURN_FILE* get_burn_file_state(DEV_HANDLE hBurnDEV);
CALLBACK_T* get_callback_info(DEV_HANDLE hBurnDEV);
int set_disc_used_add(DEV_HANDLE hBurnDEV, int byte);
int set_disk_used_add(DEV_HANDLE hBurnDEV, int byte);
int set_burn_param_element(DEV_HANDLE hBurnDEV, DISK_ELEMENT element, unsigned int val);
unsigned int get_burn_param_element(DEV_HANDLE hBurnDEV, DISK_ELEMENT element);

/*State.c*/
int Burn_Call_CallBack(DEV_HANDLE hBurnDEV, DEV_TRAY_STAT tray_state, RUNNING_STATE running_state, RUNNING_INFO running_info);
void *StateThrFxn(void *args);
int set_burn_state(DEV_HANDLE hBurnDEV, BURN_STAT state);
int set_tray_state(DEV_HANDLE hBurnDEV, DEV_TRAY_STAT state);
int set_running_state(DEV_HANDLE hBurnDEV, RUNNING_STATE state);
int set_running_info(DEV_HANDLE hBurnDEV, RUNNING_INFO state);
int set_burning_file_name(DEV_HANDLE hBurnDEV, char *name);
int set_completed_file_name(DEV_HANDLE hBurnDEV, char *name);

/*SetParam.c*/
int set_disc_size_var(DEV_HANDLE hBurnDEV, BURN_BOOL has_disc, unsigned long discsize, unsigned long usedsize, unsigned long freesize);
RUNNING_INFO get_running_info(DEV_HANDLE hBurnDEV);
RUNNING_STATE get_running_state(DEV_HANDLE hBurnDEV);
int Burn_Do_Event_Callback(DEV_HANDLE hBurnDEV, BURN_RUN_STATE  *run_state);

/*Contrl.c*/
int	Burn_Ctrl_AddCompletedFile(DEV_HANDLE hBurnDEV, char *file_path);
int Get_Dir_Wait_List_Num(DEV_HANDLE hBurnDEV);

/*FileProc.c*/
int burn_local_file(DEV_HANDLE hBurnDEV);
int Start_Burn_Local_List(DEV_HANDLE hBurnDEV);
void set_test_flag_false(void);
//int Write_Dev_Buf_Data_To_File(DEV_HANDLE hBurnDEV, FILE_HANDLE hFile);

/*DiskProc.c*/
int set_disk_write_flag(DEV_HANDLE hBurnDEV, BURN_BOOL write_flag);
BURN_BOOL is_write_backup_file(DEV_HANDLE hBurnDEV);
BURN_BOOL is_use_disk(DEV_HANDLE hBurnDEV);
int Disk_Write_BackUpFile(DEV_HANDLE hBurnDEV, char *buf, int buf_size);
//int Disk_Close_BackUpFile(DEV_HANDLE hBurnDEV);

/*StreamProc.c*/
int Burn_Stream_To_File(DEV_HANDLE hBurnDEV, FILE_HANDLE hBurnFile);
//int Fill_Data_To_Dev_Buf(DEV_HANDLE hBurnDEV, char *buf_ptr, int len, int block);
#endif
