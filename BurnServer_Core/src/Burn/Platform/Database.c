#include <stdio.h>
#include <stdlib.h>
#include <Burn_SDK.h>
// #include <RingBuffer.h>
//#include <Platform.h>

#include "BurnSelfInclude.h"

#define DEV_MAX_NUM    6


static BURN_INFO_T* dev_info[DEV_MAX_NUM] = {NULL, NULL, NULL, NULL, NULL, NULL};


BURN_DATA_T* get_burn_data(DEV_HANDLE hBurnDEV)
{
    return &(hBurnDEV->data);
}

int create_dev_database(DEV_HANDLE hBurnDEV)
{
    int i;

    for(i = 0; i < DEV_MAX_NUM; i++)
    {
        if(dev_info[i] == NULL)
        {
            dev_info[i] = (BURN_INFO_T*) calloc(1, sizeof(BURN_INFO_T));
            dev_info[i]->dev_id = i;
            hBurnDEV->data.buf_size = 10 * 1024 * 1024;
            hBurnDEV->dev_id = i;
            return BURN_SUCCESS;
        }
    }

    return BURN_FAILURE;
}

int Create_Burn_Environment(DEV_HANDLE hBurnDEV)
{
    BURN_DATA_T *data_ptr;
	int          ret;

    data_ptr = get_burn_data(hBurnDEV);
    if(data_ptr == NULL)
    {
        printf("[Database, Create_Burn_Environment] dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    if (data_ptr->hBuf == NULL)
    {
        printf("[Database, Create_Burn_Environment] data_ptr->hBuf is NULL\n");
    }

    ret = Ring_Buffer_Init(&(data_ptr->hBuf), data_ptr->buf_size);
	if(ret == 0)
		return BURN_SUCCESS;
	else
		return BURN_FAILURE;
}

int remove_dev_database(DEV_HANDLE hBurnDEV)
{
    int i=0;

    if((hBurnDEV->dev_id < 0) || (hBurnDEV->dev_id > DEV_MAX_NUM))
    {
        printf("dev_id = %d is err\n");
        return BURN_FAILURE;
    }

    free(dev_info[i]);

    return BURN_SUCCESS;
}

BURN_INFO_T* get_burn_database(DEV_HANDLE hBurnDEV)
{
    return dev_info[hBurnDEV->dev_id];
}


BURN_DEV_INFO_T* get_dev_info(DEV_HANDLE hBurnDEV)
{
    BURN_INFO_T *burn_info_ptr;

    burn_info_ptr = get_burn_database(hBurnDEV);
    if(burn_info_ptr == NULL)
    {
        printf("dev[%d] get burn_info_ptr is NULL\n", hBurnDEV->dev_id);
        return NULL;
    }

    return &(burn_info_ptr->dev_info);
}


BURN_DISC_INFO_T* get_disc_info(DEV_HANDLE hBurnDEV)
{
    BURN_INFO_T *burn_info_ptr;

    burn_info_ptr = get_burn_database(hBurnDEV);
    if(burn_info_ptr == NULL)
    {
        printf("dev[%d] get burn_info_ptr is NULL\n", hBurnDEV->dev_id);
        return NULL;
    }

    return &(burn_info_ptr->disc_info);
}


BURN_DISK_INFO_T* get_disk_info(DEV_HANDLE hBurnDEV)
{
    BURN_INFO_T *burn_info_ptr;

    burn_info_ptr = get_burn_database(hBurnDEV);
    if(burn_info_ptr == NULL)
    {
        printf("dev[%d] get burn_info_ptr is NULL\n", hBurnDEV->dev_id);
        return NULL;
    }

    return &(burn_info_ptr->disk_info);
}


BURN_RUN_STATE_T* get_run_state(DEV_HANDLE hBurnDEV)
{
    BURN_INFO_T *b_info_ptr;

    b_info_ptr = get_burn_database(hBurnDEV);
    if(b_info_ptr == NULL)
    {
        printf("dev[%d] get b_info_ptr is NULL\n", hBurnDEV->dev_id);
        return NULL;
    }

    return &(b_info_ptr->run_state);
}

#if 0
BURN_FILE_T* get_burn_file_info(DEV_HANDLE hBurnDEV)
{
    BURN_INFO_T *b_info_ptr;

    b_info_ptr = get_burn_database(hBurnDEV);
    if(b_info_ptr == NULL)
    {
        printf("dev[%d] get b_info_ptr is NULL\n", hBurnDEV->dev_id);
        return NULL;
    }

    return &(b_info_ptr->burn_file);
}
#endif

BURN_PARAM_T* get_burn_param(DEV_HANDLE hBurnDEV)
{
    BURN_INFO_T *b_info_ptr;

    b_info_ptr = get_burn_database(hBurnDEV);
    if(b_info_ptr == NULL)
    {
        printf("dev[%d] get b_info_ptr is NULL\n", hBurnDEV->dev_id);
        return NULL;
    }

    return &(b_info_ptr->burn_param);
}

BURN_FILE_T* get_burn_file_state(DEV_HANDLE hBurnDEV)
{
    BURN_INFO_T *b_info_ptr;

    b_info_ptr = get_burn_database(hBurnDEV);
    if(b_info_ptr == NULL)
    {
        printf("dev[%d] get b_info_ptr is NULL\n", hBurnDEV->dev_id);
        return NULL;
    }

    return &(b_info_ptr->burn_file);
}

CALLBACK_T* get_callback_info(DEV_HANDLE hBurnDEV)
{
    BURN_INFO_T *burn_info_ptr;

    printf("in get_callback_info\n");
    burn_info_ptr = get_burn_database(hBurnDEV);
    printf("out get_callback_info\n");
    if(burn_info_ptr == NULL)
    {
        printf("dev[%d] get burn_info_ptr is NULL\n", hBurnDEV->dev_id);
        return NULL;
    }

    return &(burn_info_ptr->event);
}

int set_disc_used_add(DEV_HANDLE hBurnDEV, int byte)
{
    BURN_DISC_INFO_T *disc_info_ptr;

    disc_info_ptr = get_disc_info(hBurnDEV);
    if(disc_info_ptr == NULL)
    {
        printf("Get Disc Info Ptr Failed\n");
        return BURN_FAILURE;
    }

    disc_info_ptr->usedsize += byte;
    disc_info_ptr->freesize -= byte;

    return BURN_SUCCESS;
}

int set_disk_used_add(DEV_HANDLE hBurnDEV, int byte)
{
    BURN_DISK_INFO_T *disk_info_ptr;

    disk_info_ptr = get_disk_info(hBurnDEV);
    if(disk_info_ptr == NULL)
    {
        printf("[%s][%d] Get Disk Info Ptr Failed\n", __FILE__, __LINE__);
        return BURN_FAILURE;
    }

    disk_info_ptr->usedsize += byte;
    disk_info_ptr->freesize -= byte;

    return BURN_SUCCESS;
}
