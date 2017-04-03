#include <stdio.h>
#include <string.h>
#include <Burn_SDK.h>

#include "BurnSelfInclude.h"

//CALLBACK_T* get_callback_info(DEV_HANDLE hBurnDEV);

#if 0
int Burn_Set_WriteSpeed(DEV_HANDLE hBurnDEV, int speed)
{
    return set_disc_info_element(hBurnDEV, MAXSPEED, speed);
}



int Burn_Set_AutoFormat(DEV_HANDLE hBurnDEV, BURN_BOOL bopen)
{
    return set_burn_param_element(hBurnDEV, AUTOFORMAT, bopen);
}

int Burn_Set_BurnCache(DEV_HANDLE hBurnDEV, int cache_size)
{
    return set_burn_param_element(hBurnDEV, CACHE, cache_size);
}

int Burn_Set_AutoMD5(DEV_HANDLE hBurnDEV, int bopen)
{
    return set_burn_param_element(hBurnDEV, MD5_BOOL, bopen);
}

int Burn_Set_BurnNum(DEV_HANDLE hBurnDEV, int num)
{
    return set_burn_param_element(hBurnDEV, BURN_NUM, num);
}

int Burn_Set_PassWord(DEV_HANDLE hBurnDEV, char *password)
{
    printf("TODO\n");
    return BURN_SUCCESS;
}

int Burn_Set_DiscCover(DEV_HANDLE hBurnDEV, char *photo_path)
{
    printf("TODO\n");
    return BURN_SUCCESS;
}

int Burn_Set_PrintProfile(DEV_HANDLE hBurnDEV)
{
    printf("TODO\n");
    return BURN_SUCCESS;
}

#endif

/*设置平台无关层接收数据接口方式*/
int CBurnDev::Burn_Set_Data_Interface(DEV_HANDLE hBurnDEV, INTERFACE_TYPE interface1)
{
    if(hBurnDEV != NULL)
    {
        hBurnDEV->interface1 = interface1;
        return BURN_SUCCESS;
    }

    return BURN_FAILURE;
}

/*对硬盘信息结构体中的指定元素进行设置*/
int CBurnDev::Burn_Set_Disk(DEV_HANDLE hBurnDEV, BURN_BOOL use_disk, char *backup_path, unsigned long backupsize, unsigned long alarmsize)
{
    BURN_DISK_INFO *b_disk_ptr;
    
    b_disk_ptr = get_disk_info(hBurnDEV);
    if(b_disk_ptr == NULL)
    {
        printf("dev[%d] get b_disk_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    b_disk_ptr->use_disk     = use_disk;
    b_disk_ptr->backupsize   = backupsize;
    b_disk_ptr->usedsize     = 0;
    b_disk_ptr->freesize     = backupsize;
    b_disk_ptr->alarmsize    = alarmsize;
    b_disk_ptr->backup_path  = backup_path;

    return BURN_SUCCESS;
}

/*对光盘信息结构体中的指定元素进行设置*/
int CBurnDev::Burn_Set_Disc(DEV_HANDLE hBurnDEV, int type, int maxspeed, unsigned long alarmsize, unsigned long alarmwarningsize)
{
    BURN_DISC_INFO *b_disc_ptr;
    
    b_disc_ptr = get_disc_info(hBurnDEV);
    if(b_disc_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    b_disc_ptr->type             = type;
    b_disc_ptr->maxspeed         = maxspeed;
    b_disc_ptr->alarmsize        = alarmsize;
    b_disc_ptr->alarmwarningsize = alarmwarningsize;
    
    return BURN_SUCCESS;
}


/*设置光盘使用大小情况*/
int set_disc_size_var(DEV_HANDLE hBurnDEV, BURN_BOOL has_disc, unsigned long discsize, unsigned long usedsize, unsigned long freesize)
{
    BURN_DISC_INFO *b_disc_ptr;
    
    b_disc_ptr = get_disc_info(hBurnDEV);
    if(b_disc_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    b_disc_ptr->has_disc = has_disc;
    b_disc_ptr->discsize = discsize;
    b_disc_ptr->usedsize = usedsize;
    b_disc_ptr->freesize = freesize;
    
    return BURN_SUCCESS;
}


#if 0
int set_burning_file_name(DEV_HANDLE hBurnDEV, char *burning_file_name)
{
    BURN_FILE_T *b_file_ptr;
    
    b_file_ptr = get_burn_file_state(hBurnDEV);
    if(b_file_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    b_file_ptr->burning_file_name = burning_file_name;
    
    return BURN_SUCCESS;
}


int set_completed_file_name(DEV_HANDLE hBurnDEV, char *completed_file_name)
{
    BURN_FILE_T *b_file_ptr;
    
    b_file_ptr = get_burn_file_state(hBurnDEV);
    if(b_file_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    b_file_ptr->completed_file_name = completed_file_name;
    
    return BURN_SUCCESS;
}
#endif
#if 0
int set_running_state(DEV_HANDLE hBurnDEV, RUNNING_STATE state)
{
    BURN_RUN_STATE_T *b_running_ptr;
    
    b_running_ptr = get_run_state(hBurnDEV);
    if(b_running_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    b_running_ptr->running_state = state;
    
    return BURN_SUCCESS;
}
#endif

#if 1
RUNNING_INFO get_running_info(DEV_HANDLE hBurnDEV)
{
    BURN_RUN_STATE *b_running_ptr;
    
    b_running_ptr = get_run_state(hBurnDEV);
    if(b_running_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return (RUNNING_INFO)B_ABNORMAL;
    }

    return b_running_ptr->running_info;
}
#endif

RUNNING_STATE get_running_state(DEV_HANDLE hBurnDEV)
{
    BURN_RUN_STATE *b_running_ptr;
    
    b_running_ptr = get_run_state(hBurnDEV);
    if(b_running_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return B_ABNORMAL;
    }

    return b_running_ptr->running_state;
}

#if 0
int set_running_info(DEV_HANDLE hBurnDEV, RUNNING_INFO info)
{
    BURN_RUN_STATE_T *b_running_ptr;
    
    b_running_ptr = get_run_state(hBurnDEV);
    if(b_running_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    b_running_ptr->running_info = info;
    
    return BURN_SUCCESS;
}
#endif

int CBurnDev::Set_Burn_Buffer_Size(DEV_HANDLE hBurnDEV, unsigned long buf_size)
{
    BURN_DATA_T *data_ptr;

    data_ptr = get_burn_data(hBurnDEV);
    if(data_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    data_ptr->buf_size = buf_size;

    return BURN_SUCCESS;
}


int CBurnDev::Burn_Set_DiscAlarmSize(DEV_HANDLE hBurnDEV, int alarm_size)
{
	BURN_DISC_INFO *b_disc_info_ptr;;

	b_disc_info_ptr = get_disc_info(hBurnDEV);
	if(b_disc_info_ptr != NULL)
	{
		//为报警设置500M默认配置
		printf("Set Disc Alarm Size Is [%d]M... \n", alarm_size);
		b_disc_info_ptr->alarmsize = alarm_size;	

		return BURN_SUCCESS;
	}		

	b_disc_info_ptr->alarmsize = 0;	

	return BURN_FAILURE;
}

int CBurnDev::Burn_Set_DiscAlarmWarningSize(DEV_HANDLE hBurnDEV, int alarm_warning_size)
{
    BURN_DISC_INFO *b_disc_info_ptr;;

    b_disc_info_ptr = get_disc_info(hBurnDEV);
    if(b_disc_info_ptr != NULL)
    {
        //为预警设置0M默认配置
        printf("Set Disc Alarm Warning Size Is [%d]M... \n", alarm_warning_size);
        b_disc_info_ptr->alarmwarningsize = alarm_warning_size;	

        return BURN_SUCCESS;
    }		

    b_disc_info_ptr->alarmwarningsize = 0;	

    return BURN_FAILURE;
}

int CBurnDev::Burn_Set_Event_Callback(DEV_HANDLE hBurnDEV, CB_EVENTS important_events, void *val)
{
	CALLBACK_T *b_callback_ptr;

	b_callback_ptr = get_callback_info(hBurnDEV);
    if(b_callback_ptr == NULL)
    {
        printf("dev[%d] get b_callback_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }	

	b_callback_ptr->val = val;
	b_callback_ptr->important_events = important_events;

	return BURN_SUCCESS;
}

int Burn_Do_Event_Callback(DEV_HANDLE hBurnDEV, BURN_RUN_STATE  *run_state)
{
	CALLBACK_T *b_callback_ptr;

	printf("=========== In Burn Do Event Callback ===========\n");

    printf("in get_callback_inf: %p\n",(void*)hBurnDEV);
	b_callback_ptr = get_callback_info(hBurnDEV);
    printf("Out get_callback_inf\n");
    if(b_callback_ptr == NULL)
    {
        printf("dev[%d] get b_callback_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }	
	
	if(b_callback_ptr->important_events != NULL)
		return b_callback_ptr->important_events(hBurnDEV, run_state, b_callback_ptr->val);	

	return BURN_FAILURE;	
}

int CBurnDev::Burn_Set_DiscName(DEV_HANDLE hBurnDEV, char *name)
{
	BURN_DISC_INFO *b_disc_info_ptr;;

#ifdef WIN32
	SetDiscVolumeLabel(hBurnDEV->hDVD, name);
#endif	
	b_disc_info_ptr = get_disc_info(hBurnDEV);
	if(b_disc_info_ptr != NULL)
	{
		printf("Set Disc Name Is [%s]... \n", name);
		strcpy(b_disc_info_ptr->disc_name, name);
		return BURN_SUCCESS;
	}			

	return BURN_FAILURE;
}
