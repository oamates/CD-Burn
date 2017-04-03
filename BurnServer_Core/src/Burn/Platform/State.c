#include <stdio.h>
#include <string.h>
#include <Burn_SDK.h>
// #include <RingBuffer.h>

#include "BurnSelfInclude.h"
#include <time.h>

//int Burn_Do_Event_Callback(DEV_HANDLE hBurnDEV, BURN_RUN_STATE_T  *run_state);
//int Stop_Burning_Disc(DEV_HANDLE hBurnDEV);
//int Stop_Disk_BackUp(DEV_HANDLE hBurnDEV);

#if 1
int	CBurnDevInfo::Burn_Get_TrayState(DEV_HANDLE hBurnDEV)
{
	BURN_RUN_STATE *b_run_state_ptr;
	b_run_state_ptr = get_run_state(hBurnDEV);
	if(b_run_state_ptr == NULL)
	{
		printf("[StateThread] Get b_run_state_ptr Failed\n");	
		return BURN_FAILURE;
	}

    return b_run_state_ptr->tray_state;
}


int CBurnDevInfo::Burn_Get_DevInfo(DEV_HANDLE hBurnDEV, BURN_DEV_INFO *pDevInfo)
{
    BURN_DEV_INFO *b_dev_info_ptr;

    b_dev_info_ptr = get_dev_info(hBurnDEV);
    if(b_dev_info_ptr == NULL)
    {
        printf("Get b_dev_info_ptr == NULL\n");
        return BURN_FAILURE;
    }
    memcpy(pDevInfo, b_dev_info_ptr, sizeof(BURN_DEV_INFO));

    return BURN_SUCCESS;
}

int CBurnDevInfo::Burn_Get_DiscInfo(DEV_HANDLE hBurnDEV, BURN_DISC_INFO *pDiscInfo)
{
    BURN_DISC_INFO *b_disc_info_ptr;

    b_disc_info_ptr = get_disc_info(hBurnDEV);
    if(b_disc_info_ptr == NULL)
    {
        printf("Get b_disc_info_ptr == NULL\n");
        return BURN_FAILURE;
    }
    memcpy(pDiscInfo, b_disc_info_ptr, sizeof(BURN_DISC_INFO));

    return BURN_SUCCESS;

}

int CBurnDevInfo::Burn_Get_DiskInfo(DEV_HANDLE hBurnDEV, BURN_DISK_INFO *pDiscInfo)
{
    BURN_DISK_INFO *b_disk_info_ptr;

    b_disk_info_ptr = get_disk_info(hBurnDEV);
    if(b_disk_info_ptr == NULL)
    {
        printf("Get b_disk_info_ptr == NULL\n");
        return BURN_FAILURE;
    }
    memcpy(pDiscInfo, b_disk_info_ptr, sizeof(BURN_DISK_INFO));

    return BURN_SUCCESS;
}

int CBurnDevInfo::Burn_Get_DiscCanWrite(DEV_HANDLE hBurnDEV)
{
    return BURN_SUCCESS;
}
#endif
#if 0
int Burn_Get_TotalWriteSize(DEV_HANDLE hBurnDEV, unsigned long long *pTotalSize)
{
    unsigned long long TotalSize; 
    int ret;

    switch(hBurnDEV->kernel)
    {
        case BURN_LINUX:
            ret = Xkd_DVDSDK_GetTotalWriteSize(hBurnDEV->hDVD, &TotalSize);
            if(ret == BURN_SUCCESS)
            {
                set_disc_info_element(hBurnDEV, DISCSIZE, TotalSize);
                return BURN_SUCCESS;
            }
            break;

        case BURN_WINDOWS:
            break;

        case BURN_EPSON:
            break;

        case BURN_PRIMERA:
            break;

        default:
            printf("Kernel Type [%d] Is Err", hBurnDEV->kernel);
            break;
    }

    return BURN_FAILURE;
}


int Burn_Get_FreeWriteSize(DEV_HANDLE hBurnDEV, unsigned long long *pFreeSize)
{
    unsigned long long FreeSize; 
    int ret;

    switch(hBurnDEV->kernel)
    {
        case BURN_LINUX:
            ret = Xkd_DVDSDK_GetFreeWriteSize(hBurnDEV->hDVD, unsigned long long &FreeSize);
            if(ret == BURN_SUCCESS)
            {
                set_disc_info_element(hBurnDEV, FREESIZE, TotalSize);
                return BURN_SUCCESS;
            }
            break;

        case BURN_WINDOWS:
            break;

        case BURN_EPSON:
            break;

        case BURN_PRIMERA:
            break;

        default:
            printf("Kernel Type [%d] Is Err", hBurnDEV->kernel);
            break;
    }

    return BURN_FAILURE;
}
#endif

int Burn_Call_CallBack(DEV_HANDLE hBurnDEV, DEV_TRAY_STAT tray_state, RUNNING_STATE running_state, RUNNING_INFO running_info)
{
	printf("========== In Burn Call CallBack ===========\n");
#if 1
    BURN_RUN_STATE *b_running_ptr;
  
    b_running_ptr = get_run_state(hBurnDEV);
    if(b_running_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
   		return BURN_FAILURE;
    }

	b_running_ptr->tray_state    = tray_state;
	b_running_ptr->running_state = running_state;
	b_running_ptr->running_info  = running_info;

	Burn_Do_Event_Callback(hBurnDEV, b_running_ptr);
#endif

    return BURN_SUCCESS;
}

void *StateThrFxn(void *args)
{
    int ret=0;
    DEV_HANDLE hBurnDEV;
	BURN_DISC_INFO *b_disc_info_ptr;
	BURN_RUN_STATE *b_run_state_ptr;

    printf(" =============== State Thread Start ==============\n");

    hBurnDEV = (DEV_HANDLE)args;  
	
	if(hBurnDEV == NULL)
	{
		printf("[StateThread] hBurnDEV == NULL\n");	
		goto cleanup;
	}
	b_disc_info_ptr = get_disc_info(hBurnDEV);
	if(b_disc_info_ptr == NULL)
	{
		printf("[StateThread] Get b_disc_info_ptr Failed\n");	
		goto cleanup;
	}

    BURN_DISK_INFO* disk_info_ptr;

    disk_info_ptr = get_disk_info(hBurnDEV);
    if(disk_info_ptr == NULL)
    {
        printf("[StateThread] Get Disk Info Ptr Failed\n");
		goto cleanup;
    }

	b_run_state_ptr = get_run_state(hBurnDEV);
	if(b_run_state_ptr == NULL)
	{
		printf("[StateThread] Get b_run_state_ptr Failed\n");	
		goto cleanup;
	}

	set_burn_state(hBurnDEV, B_ALREAD);
	while((hBurnDEV->burn_state != B_STOP))
//	while(1)
	{
//        if(hBurnDEV->burn_state == B_STOP)
//        {
//            printf("BurnServer Is B_STOP, State Thread continue\n");
//			sleep(1);
//           continue;
//        }
        if(hBurnDEV->burn_state == B_CLOSED)
        {
            printf("BurnServer Is Closed, State Thread Return\n");
            break;
        }

		/********************** 打印BurnServer运行状态 ***********************/
		if(b_run_state_ptr->running_state == B_NORMALLY)
			printf("\nBurnServer Running OK... \n");	
		else	
			printf("\nBurnServer Running Abnormal... \n");	
			
		/********************** 打印光盘容量状态 ***********************/

		printf("~~~~~~~~~~~~~Dev[%d] Disc Used Size Is [%d] MB, ", hBurnDEV->dev_id, b_disc_info_ptr->usedsize);	
		printf("Free Size Is [%d] MB, ", b_disc_info_ptr->freesize);	
		printf("Alarm Size Is [%d] MB, ", b_disc_info_ptr->alarmsize);
        printf("Alarm Warning Size Is [%d] MB, ", b_disc_info_ptr->alarmwarningsize);
        printf("Buffer Size Is [%d] KB, ", ((&(hBurnDEV->data))->buf_size)/1024);
        printf("Free Size Is [%d] KB ", GetFreeSpaceLen((&(hBurnDEV->data))->hBuf)/1024);

        time_t tCurTime = time(NULL);
        char tmpbuf[80];
        strftime(tmpbuf,80,"Cur Time: %Y-%m-%d %H:%M:%S~~~~~~~~~~~~~\n",localtime(&tCurTime));
        printf(tmpbuf);
		if(b_disc_info_ptr->freesize <= b_disc_info_ptr->alarmsize)
		{
			printf("Dev[%d] Disc Free Size Is [%d] MB, Alarm Size Is [%d] MB, Alarm Warning Size Is [%d] MB...\n"
                , hBurnDEV->dev_id, b_disc_info_ptr->freesize, b_disc_info_ptr->alarmsize, b_disc_info_ptr->alarmwarningsize);	

			Burn_Call_CallBack(hBurnDEV, B_DEV_TRAY_CLOSE, B_ABNORMAL, ERR_DISC_HAS_NO_SPACE);
		}
        else
        {
            Burn_Call_CallBack(hBurnDEV, B_DEV_TRAY_CLOSE, B_NORMALLY, B_RUNNING_OK);
        }
        if(b_disc_info_ptr->freesize <= b_disc_info_ptr->alarmsize + b_disc_info_ptr->alarmwarningsize)
        {
            printf("Dev[%d] Disc Free Size Is [%d] MB, Alarm Size Is [%d] MB, Alarm Warning Size Is [%d] MB...\n"
                , hBurnDEV->dev_id, b_disc_info_ptr->freesize, b_disc_info_ptr->alarmsize, b_disc_info_ptr->alarmwarningsize);	

            Burn_Call_CallBack(hBurnDEV, B_DEV_TRAY_CLOSE, B_NORMALLY, WARNING_DISC_WILL_NO_SPACE);
        }

		if(disk_info_ptr->use_disk == BURN_TRUE)
		{
			if(disk_info_ptr->freesize <= disk_info_ptr->alarmsize)
			{
				printf("============= Dev[%d] Disk Free Size Is [%d] MB, Alarm Size Is [%d] MB, Alarm Warning Size Is [%d] MB =============\n"
                    , hBurnDEV->dev_id, disk_info_ptr->freesize, disk_info_ptr->alarmsize, b_disc_info_ptr->alarmwarningsize);	

				CDiskOpr::Stop_Disk_BackUp(hBurnDEV);	
			}	
		}

		sleep(3);	
	}

	return 0;

cleanup:
/*
   ret = Stop_Burning_Disc(hBurnDEV);
    if(ret == BURN_FAILURE)
    {
        printf("Stop Burnning Disc Failed\n");
        return 0;
    }
	*/
	return 0;
}

int CBurnDevInfo::Get_Burn_Info(DEV_HANDLE hBurnDEV, BURN_INFO *burn_info)
{
    BURN_INFO *burn_info_ptr;

    burn_info_ptr = get_burn_database(hBurnDEV);
    if(burn_info_ptr == NULL)
    {
        printf("dev[%d] get burn_info_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }
	
	memcpy(burn_info, burn_info_ptr, sizeof(BURN_INFO));
		
	return BURN_SUCCESS;
}

int CBurnDevInfo::Get_Burn_File_State(DEV_HANDLE hBurnDEV, BURN_FILE  *burn_file_info)
{
    BURN_FILE *burn_file_info_ptr;

	burn_file_info_ptr = get_burn_file_state(hBurnDEV);
    if(burn_file_info_ptr == NULL)
    {
        printf("dev[%d] get burn_file_info_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

	memcpy(burn_file_info, burn_file_info_ptr, sizeof(BURN_FILE));
		
	return BURN_SUCCESS;
}

int set_burn_state(DEV_HANDLE hBurnDEV, BURN_STAT state)
{
    if(hBurnDEV == NULL)
    {
        printf("dev[%d] get hBurnDEV is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

	switch(state)
	{
		case B_ALREAD:
			printf("dev[%d] Set BurnServer State To [Already]\n ", hBurnDEV->dev_id);
			break;
			
		case B_START:
			printf("dev[%d] Set BurnServer State To [Start]\n ", hBurnDEV->dev_id);
			break;
			
		case B_STOP:
			printf("dev[%d] Set BurnServer State To [Stop]\n ", hBurnDEV->dev_id);
			break;
			
		case B_CLOSED:
			printf("dev[%d] Set BurnServer State To [Closed]\n ", hBurnDEV->dev_id);
			break;

		default:
			printf("dev[%d] Set BurnServer State Is No Support\n ", hBurnDEV->dev_id);
			return BURN_FAILURE;
	}

//	if(hBurnDEV->burn_state != B_CLOSED)
	    hBurnDEV->burn_state = state;

    return BURN_SUCCESS;
}

BURN_STAT CBurnDevInfo::Get_Running_Burn_State(DEV_HANDLE hBurnDEV)
{
    if(hBurnDEV == NULL)
    {
        printf("dev[%d] get hBurnDEV is NULL\n", hBurnDEV->dev_id);
        return (BURN_STAT)BURN_FAILURE;
    }

    return hBurnDEV->burn_state;
}

int set_tray_state(DEV_HANDLE hBurnDEV, DEV_TRAY_STAT state)
{
    BURN_RUN_STATE *b_run_ptr;
    
    b_run_ptr = get_run_state(hBurnDEV);
    if(b_run_ptr == NULL)
    {
        printf("dev[%d] get b_run_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    b_run_ptr->tray_state = state;

    return BURN_SUCCESS;
}

DEV_TRAY_STAT CBurnDevInfo::Get_Running_Tray_State(DEV_HANDLE hBurnDEV)
{
    BURN_RUN_STATE *b_run_ptr;
    
    b_run_ptr = get_run_state(hBurnDEV);
    if(b_run_ptr == NULL)
    {
        printf("dev[%d] get b_run_ptr is NULL\n", hBurnDEV->dev_id);
        return (DEV_TRAY_STAT)BURN_FAILURE;
    }

    return b_run_ptr->tray_state;
}


int set_running_state(DEV_HANDLE hBurnDEV, RUNNING_STATE state)
{
    BURN_RUN_STATE *b_run_ptr;
    
    b_run_ptr = get_run_state(hBurnDEV);
    if(b_run_ptr == NULL)
    {
        printf("dev[%d] get b_run_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    b_run_ptr->running_state = state;

    return BURN_SUCCESS;
}

RUNNING_STATE CBurnDevInfo::Get_Running_State(DEV_HANDLE hBurnDEV)
{
    BURN_RUN_STATE *b_run_ptr;
    
    b_run_ptr = get_run_state(hBurnDEV);
    if(b_run_ptr == NULL)
    {
        printf("dev[%d] get b_run_ptr is NULL\n", hBurnDEV->dev_id);
        return (RUNNING_STATE)BURN_FAILURE;
    }

    return b_run_ptr->running_state;
}


int set_running_info(DEV_HANDLE hBurnDEV, RUNNING_INFO state)
{
    BURN_RUN_STATE *b_run_ptr;
    
    b_run_ptr = get_run_state(hBurnDEV);
    if(b_run_ptr == NULL)
    {
        printf("dev[%d] get b_run_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    b_run_ptr->running_info = state;

    return BURN_SUCCESS;
}

RUNNING_INFO CBurnDevInfo::Get_Running_info(DEV_HANDLE hBurnDEV)
{
    BURN_RUN_STATE *b_run_ptr;
    
    b_run_ptr = get_run_state(hBurnDEV);
    if(b_run_ptr == NULL)
    {
        printf("dev[%d] get b_run_ptr is NULL\n", hBurnDEV->dev_id);
        return (RUNNING_INFO)BURN_FAILURE;
    }

    return b_run_ptr->running_info;
}


int set_burning_file_name(DEV_HANDLE hBurnDEV, char *name)
{
    BURN_FILE *b_file_ptr;
    
    b_file_ptr = get_burn_file_state(hBurnDEV);
    if(b_file_ptr == NULL)
    {
        printf("dev[%d] get b_file_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    b_file_ptr->burning_file_name = name;

    return BURN_SUCCESS;
}

char* CBurnDevInfo::Get_Burning_File_Name(DEV_HANDLE hBurnDEV)
{
    BURN_FILE *b_file_ptr;
    
    b_file_ptr = get_burn_file_state(hBurnDEV);
    if(b_file_ptr == NULL)
    {
        printf("dev[%d] get b_file_ptr is NULL\n", hBurnDEV->dev_id);
        return NULL;
    }

    return b_file_ptr->burning_file_name;
}


int set_completed_file_name(DEV_HANDLE hBurnDEV, char *name)
{
    BURN_FILE *b_file_ptr;
    
    b_file_ptr = get_burn_file_state(hBurnDEV);
    if(b_file_ptr == NULL)
    {
        printf("dev[%d] get b_file_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    b_file_ptr->completed_file_name = name;

    return BURN_SUCCESS;
}

char* CBurnDevInfo::Get_Completed_File_Name(DEV_HANDLE hBurnDEV)
{
    BURN_FILE *b_file_ptr;
    
    b_file_ptr = get_burn_file_state(hBurnDEV);
    if(b_file_ptr == NULL)
    {
        printf("dev[%d] get b_file_ptr is NULL\n", hBurnDEV->dev_id);
        return NULL;
    }

    return b_file_ptr->completed_file_name;
}
