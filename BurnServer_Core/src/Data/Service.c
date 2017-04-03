#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Burn_SDK.h>
#include <Burn_Stream.h>

#include "StreamSelfInclude.h"

//BURN_STREAM_T *get_stream_database(DEV_HANDLE hBurnDEV);

#if 0
BURN_MODE Get_Service_Mode(DEV_HANDLE hBurnDEV)
{
    BURN_STREAM_T *s_database_ptr;;

    s_database_ptr = get_stream_database(hBurnDEV);
    if(s_database_ptr == NULL)
    {
        printf("Get Stream DataBase Ptr Failed\n");
        return (BURN_MODE)BURN_FAILURE;
    }

    return s_database_ptr->burn_mode;
}
#endif

//设置业务模式，流刻录还是文件刻录
int Set_Stream_Service_Mode(DEV_HANDLE hBurnDEV)
{
    BURN_STREAM_T *s_database_ptr;;
    int ret;

    s_database_ptr = get_stream_database(hBurnDEV);
    if(s_database_ptr == NULL)
    {
        printf("Get Stream DataBase Ptr Failed\n");
        return BURN_FAILURE;
    }

	ret = CBurnDev::Burn_Set_Data_Interface(hBurnDEV, B_STREAM);
    if(ret != BURN_SUCCESS)
    {
        printf("Set Burn Mode Failed\n");
        return BURN_FAILURE;
    }

    s_database_ptr->burn_mode = MEDIAFILE_BURN_STREAM;

    printf("Set Service Mode Is Stream\n");
    return BURN_SUCCESS;
}

//设置刻录数据处理类型，本期只为TS
int Set_Stream_Proc_Type(MEDIAFILE_TYPE type)
{
    return BURN_SUCCESS;
}


