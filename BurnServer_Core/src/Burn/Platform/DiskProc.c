#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Burn_SDK.h>

#include "BurnSelfInclude.h"

#define _FILE_OFFSET_BITS 64


//int set_disk_used_add(DEV_HANDLE hBurnDEV, int byte);

int CDiskOpr::Set_Disk_Param(DEV_HANDLE hBurnDEV, BURN_BOOL use_dev, int backup_size, int alarm_size, char *disk_path)
{
    BURN_DISK_INFO *disk_info_ptr;

    disk_info_ptr = get_disk_info(hBurnDEV);
    if(disk_info_ptr == NULL)
    {
        printf("[%s][%d] Get Disk Info Ptr Failed\n", __FILE__, __LINE__);
        return BURN_FAILURE;
    }

    if(disk_path == NULL)
    {
        printf("disk_path == NULL\n");
        return BURN_FAILURE;
    }
    else
    {
        disk_info_ptr->backup_path = disk_path;
    }

    disk_info_ptr->use_disk   = use_dev;
    disk_info_ptr->write_flag = BURN_FLASE;
    disk_info_ptr->backupsize = backup_size;
    disk_info_ptr->usedsize   = 0;
    disk_info_ptr->freesize   = backup_size;
    disk_info_ptr->alarmsize  = alarm_size;

    return BURN_SUCCESS;
}

int set_disk_write_flag(DEV_HANDLE hBurnDEV, BURN_BOOL write_flag)
{
    BURN_DISK_INFO *disk_info_ptr;

    disk_info_ptr = get_disk_info(hBurnDEV);
    if(disk_info_ptr == NULL)
    {
        printf("[%s][%d] Get Disk Info Ptr Failed\n", __FILE__, __LINE__);
        return BURN_FAILURE;
    }

    disk_info_ptr->write_flag = write_flag;

	return BURN_SUCCESS;
}

BURN_BOOL is_write_backup_file(DEV_HANDLE hBurnDEV)
{
    BURN_DISK_INFO *disk_info_ptr;

    disk_info_ptr = get_disk_info(hBurnDEV);
    if(disk_info_ptr == NULL)
    {
        printf("[%s][%d] Get Disk Info Ptr Failed\n", __FILE__, __LINE__);
        return BURN_FLASE;
    }

	return 	disk_info_ptr->write_flag;
}

BURN_BOOL is_use_disk(DEV_HANDLE hBurnDEV)
{
    BURN_DISK_INFO* disk_info_ptr;

    disk_info_ptr = get_disk_info(hBurnDEV);
    if(disk_info_ptr == NULL)
    {
        printf("[%s][%d] Get Disk Info Ptr Failed\n", __FILE__, __LINE__);
        return BURN_FLASE;
    }

    return disk_info_ptr->use_disk;
}

int CDiskOpr::Disk_Create_BackUpFile(DEV_HANDLE hBurnDEV, const char *file_name)
{
    char file_path[1000];
    BURN_DISK_INFO* disk_info_ptr;

    if(is_use_disk(hBurnDEV) == BURN_FLASE)
    {
		printf("Disk Is Set No Used ...\n");
        return BURN_FAILURE;
    }

    disk_info_ptr = get_disk_info(hBurnDEV);
    if(disk_info_ptr == NULL)
    {
        printf("[%s][%d] Get Disk Info Ptr Failed\n", __FILE__, __LINE__);
        return BURN_FAILURE;
    }

    sprintf(file_path, "%s/%s", disk_info_ptr->backup_path, file_name);
    printf("Create BackUp File Is %s\n", file_path);
    
    hBurnDEV->fd = fopen(file_path, "w+");
	if(hBurnDEV->fd == NULL)
    {
        printf("Create BackUp File Failed\n");
		return BURN_FAILURE;
    }

	return BURN_SUCCESS;	
}


int Disk_Write_BackUpFile(DEV_HANDLE hBurnDEV, char *buf, int buf_size)
{
	if(is_write_backup_file(hBurnDEV) == BURN_FLASE)
		return 0;

    if((hBurnDEV->fd == NULL) || (is_use_disk(hBurnDEV) == BURN_FLASE))
    {
		printf("[Disk_Write_BackUpFile] Param Error\n");
        return 0;
    }

    fwrite(buf, buf_size, 1, hBurnDEV->fd);

	set_disk_used_add(hBurnDEV, buf_size);

	return BURN_SUCCESS;
}

int CDiskOpr::Disk_Read_BackUpFile(DEV_HANDLE hBurnDEV, char *buf, int buf_size)
{
    if((hBurnDEV->fd == NULL) || (is_use_disk(hBurnDEV) == BURN_FLASE))
    {
		printf("[Disk_Read_BackUpFile] Param Error\n");
        return 0;
    }

    return fread(buf, buf_size, 1, hBurnDEV->fd);
}

int CDiskOpr::Disk_Close_BackUpFile(DEV_HANDLE hBurnDEV)
{
    if((hBurnDEV->fd == NULL) || (is_use_disk(hBurnDEV) == BURN_FLASE))
    {
		printf("[Disk_Close_BackUpFile] Param Error\n");
        return 0;
    }

    fclose(hBurnDEV->fd);
	hBurnDEV->fd = NULL;

	return BURN_SUCCESS;
}

int CDiskOpr::Start_Disk_BackUp(DEV_HANDLE hBurnDEV)
{
    if((hBurnDEV->fd == NULL) || (is_use_disk(hBurnDEV) == BURN_FLASE) || (hBurnDEV == NULL))
    {
		printf("[Start_Stream_BackUp] Param Error\n");
        return 0;
    }

	return set_disk_write_flag(hBurnDEV, BURN_TRUE);
}

int CDiskOpr::Stop_Disk_BackUp(DEV_HANDLE hBurnDEV)
{
	set_disk_write_flag(hBurnDEV, BURN_FLASE);

	return Disk_Close_BackUpFile(hBurnDEV);
}
