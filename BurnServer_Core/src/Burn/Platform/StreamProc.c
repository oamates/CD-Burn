#include <stdio.h>
#include <string.h>
#include <Burn_SDK.h>

#include "BurnSelfInclude.h"

#define BUF_SIZE    1024*1024
#define WRITE_SIZE  32*1024


//BURN_DATA_T* get_burn_data(DEV_HANDLE hBurnDEV);


int Burn_Stream_To_File(DEV_HANDLE hBurnDEV, FILE_HANDLE hBurnFile)
{
    int   ret;
    char  buf[BUF_SIZE];

    while(1)
    {
        ret = Get_Data_Form_Buf(hBurnDEV->hBuf, buf, WRITE_SIZE, BURN_TRUE);
        if(ret == BURN_FAILURE)
        {
            printf("Get Stream Failed\n");
            usleep(10 * 1000);
            continue;
        }

        ret = Burn_Ctrl_WriteData(hBurnDEV, hBurnFile, buf, WRITE_SIZE);
        if(ret == BURN_FAILURE)
        {
            printf("Write Stream Data Failed\n");
            usleep(10 * 1000);
            continue;
        }
    }

    return BURN_SUCCESS;
}

int Fill_Data_To_Dev_Buf(DEV_HANDLE hBurnDEV, char *buf_ptr, int len, int block)
{
	BURN_DATA_T *data_ptr;

    data_ptr = get_burn_data(hBurnDEV);
    if(data_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    return Fill_Data_To_Buf(data_ptr->hBuf, buf_ptr, len, block);
//    return 0;
}
