#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Burn_SDK.h>
#include <Burn_Stream.h>
#include <Burn_State.h>

#include "StreamSelfInclude.h"

//int config_rtsp_url_and_ts_channel(RTSPTS_HANDLE *hStream, char *stream_url, int ts_channel);

//最大支持4个光驱设备
BURN_STREAM_T *stream_database[BURN_DEV_MAX_NUM];

BURN_STREAM_T *get_stream_database(DEV_HANDLE hBurnDEV)
{
    int i;

    for(i = 0; i < BURN_DEV_MAX_NUM; i++)
    {
        if(stream_database[i] != NULL)
        {
            if(stream_database[i]->hBurnDEV == hBurnDEV)
            {
//                printf("[get_stream_database] Find Dev [%d] Form DataBase[%d]\n", stream_database[i]->hBurnDEV->dev_id, i);
                return stream_database[i];       
            }
        }
    }

    return NULL;
}

RTSPTS_HANDLE *get_stream_handle(DEV_HANDLE hBurnDEV)
{
    BURN_STREAM_T *s_database_ptr;;

    s_database_ptr = get_stream_database(hBurnDEV);
    if(s_database_ptr == NULL)
    {
        printf("Get Stream DataBase Ptr Failed\n");
        return NULL;
    }

    return &(s_database_ptr->hStream);
}

STREAM_INFO_T *get_stream_info_ptr(DEV_HANDLE hBurnDEV)
{
    BURN_STREAM_T *s_database_ptr;;

    s_database_ptr = get_stream_database(hBurnDEV);
    if(s_database_ptr == NULL)
    {
        printf("Get Stream DataBase Ptr Failed\n");
        return NULL;
    }

    return (STREAM_INFO_T *)s_database_ptr->stream_info;
}


STREAM_INFO_T *get_stream_info(STREAM_INFO_T *s_info_ptr, int stream_id)
{
    int i;

    for(i = 0; i < STREAM_INFO_MAX_NUM; i++)
    {
        if((s_info_ptr + i) != NULL)
        {
            if((s_info_ptr + i)->stream_id == stream_id)
            {
                return (s_info_ptr + i);
            }
        }
    }
    return NULL;
}



DEV_HANDLE get_dev_handle(RTSPTS_HANDLE *hStream)
{
    int i;

    for(i = 0; i < BURN_DEV_MAX_NUM; i++)
    {
        if(stream_database[i] != NULL)
        {
            if(&(stream_database[i]->hStream) == hStream)
            {
                return stream_database[i]->hBurnDEV;       
            }
        }
    }

    return NULL;
}

void Print_Stream_List(DEV_HANDLE hBurnDEV)
{
    int i;
    STREAM_INFO_T *s_info_ptr;
    BURN_STREAM_T *s_database_ptr;;

    printf("==========================================================\n");
    s_database_ptr = get_stream_database(hBurnDEV);
    if(s_database_ptr == NULL)
    {
        printf("Get Stream DataBase Ptr Failed\n");
        return ;
    }

    for(i = 0; i < STREAM_INFO_MAX_NUM; i++)
    {
        s_info_ptr = s_database_ptr->stream_info[i];

        if(s_info_ptr != NULL)
        {
            printf("List[%d] Has Stream[%d], Url is [%s]\n", i, s_info_ptr->stream_id, s_info_ptr->url);
        }
        else
            printf("List[%d] Is NULL\n", i);
    }
    printf("==========================================================\n");
}

//创建媒体层数据库
int create_stream_database(DEV_HANDLE hBurnDEV)
{
    int i, j;

    for(i = 0; i < BURN_DEV_MAX_NUM; i++)
    {
        if(stream_database[i] == NULL)
        {
//			static BURN_STREAM_T burn_stream_TT;
//            stream_database[i] = &burn_stream_TT;
            stream_database[i] = (BURN_STREAM_T*) calloc(1, sizeof(BURN_STREAM_T));
            if(stream_database[i] == NULL)
            {
                printf("[Database,create_stream_database] Create Stream DataBase [%d] Failed\n", i);
                return BURN_FAILURE;
            }

            stream_database[i]->hBurnDEV = hBurnDEV;
            
            //初始化流信息数组
            for(j = 0; j < STREAM_INFO_MAX_NUM; j++)
            {
                stream_database[i]->stream_info[j] = NULL;
            }
            printf("[Database,create_stream_database] stream_database[%d] can be used!\n", i);
            return BURN_SUCCESS;       
        }
    }

    return BURN_FAILURE;
}


int add_stream_to_list(DEV_HANDLE hBurnDEV, STREAM_INFO_T *stream)
{
    int i;
    BURN_STREAM_T *s_database_ptr;;

    printf("[Database,add_stream_to_list] Url is [%s]\n", stream->url);

    s_database_ptr = get_stream_database(hBurnDEV);
    if(s_database_ptr == NULL)
    {
        printf("[Database,add_stream_to_list] Get Stream DataBase Ptr Failed\n");
        return BURN_FAILURE;
    }

    //预先检测一下数据库中是否已经包含该流的信息
#if 0
    STREAM_INFO_T *s_info_ptr;
    s_info_ptr = get_stream_info(s_database_ptr->stream_info, stream->stream_id);
    if(s_info_ptr != NULL)
    {
        printf("There Is Already Have The Stream Info\n");
        return BURN_FAILURE;
    }
#else
    for(i = 0; i < STREAM_INFO_MAX_NUM; i++)
    {
        if(s_database_ptr->stream_info[i] != NULL)
        {
            if(s_database_ptr->stream_info[i]->stream_id == stream->stream_id)
            {
                printf("[Database,add_stream_to_list] There Is Already Have The Stream Info\n");
                return BURN_FAILURE;
            }
        }
    }
#endif

    for(i = 0; i < STREAM_INFO_MAX_NUM; i++)
    {
//        s_info_ptr = s_database_ptr->stream_info[i];

        if(s_database_ptr->stream_info[i] == NULL)
        {
            s_database_ptr->stream_info[i] = (STREAM_INFO_T*) calloc(1, sizeof(STREAM_INFO_T));
            if(s_database_ptr->stream_info[i] == NULL)
            {
                printf("[Database,add_stream_to_list] Stream [%d] List Get Mem Failed\n", stream->stream_id);
                return BURN_FAILURE;
            }
            memcpy(s_database_ptr->stream_info[i], stream, sizeof(STREAM_INFO_T));
            printf("[Database,add_stream_to_list] List[%d] Add Stream[%d], Url is [%s]\n", i, s_database_ptr->stream_info[i]->stream_id, s_database_ptr->stream_info[i]->url);
            return BURN_SUCCESS;       
        }
    }

    return BURN_FAILURE;
}


int Add_Stream_To_Burn(DEV_HANDLE hBurnDEV, STREAM_INFO_T *stream)
{
    int ret;
    BURN_STREAM_T *s_database_ptr;;

    printf("[Database,Add_Stream_To_Burn] Url is [%s]\n", stream->url);
    s_database_ptr = get_stream_database(hBurnDEV);
    if(s_database_ptr == NULL)
    {
        printf("[Database,Add_Stream_To_Burn]Get Stream DataBase Ptr Failed\n");
        return BURN_FAILURE;
    }

    //判断设置的TS通道是否合法
    if((stream->ts_channel > 3) || (stream->ts_channel < 0))
    {
        printf("[Database,Add_Stream_To_Burn] Add Stream's Ts_Channel Is Invalid\n");
        return BURN_FAILURE;
    }

    //判断设置的TS通道是否已经被设置过
    printf("[Database,Add_Stream_To_Burn] s_database_ptr->ts_mark is [%d]\n", s_database_ptr->ts_mark);
    if(((s_database_ptr->ts_mark) & 1 << stream->ts_channel) != 0)
    {
        printf("[Database,Add_Stream_To_Burn] Stream Id[%d]'s Ts_Channel Is Already Set\n", stream->stream_id);
        return BURN_FAILURE;
    }

    ret = add_stream_to_list(hBurnDEV, stream);
    if(ret == BURN_SUCCESS)
    {
        //如果添加流信息成功，就将该流设置的TS通道也记录下来
        s_database_ptr->ts_mark |= 1 << stream->ts_channel;
    }

    return ret;
}

/*清空流列表*/
int clear_stream_database(DEV_HANDLE hBurnDEV)
{
    int i;
    int j;
    BURN_STREAM_T *s_database_ptr;;

    s_database_ptr = get_stream_database(hBurnDEV);
    if(s_database_ptr == NULL)
    {
        printf("[Database, clear_stream_database] Get Stream DataBase Ptr Failed\n");
        return BURN_FAILURE;
    }

    for(i = 0; i < BURN_DEV_MAX_NUM; i++)
    {
        if(stream_database[i] != NULL)
        {
            if(stream_database[i]->hBurnDEV == hBurnDEV)
            {
                printf("[Database, clear_stream_database] clear_stream_list\n");
                for(j = 0; j < STREAM_INFO_MAX_NUM; j++)
                {
                    if(stream_database[i]->stream_info[j] != NULL)
                    {
                        free(stream_database[i]->stream_info[j]);
                        stream_database[i]->stream_info[j] = NULL;
                    }
                }

                //如果清空流信息成功，就将整个结构清空掉
                free(stream_database[i]);
                stream_database[i] = NULL;

                //清空Ring_Buffer
                printf("[Database, clear_stream_database] Begin to release ring buffer\n");
                int nRet = RingBuffer_Release(hBurnDEV->data.hBuf);
                if (nRet == 0)
                {
                    printf("[Database, clear_stream_database] Release ring buffer success!\n");
                }
                else
                {
                    printf("[Database, clear_stream_database] Release ring buffer failed!\n");
                }
                
                hBurnDEV->data.hBuf = NULL;

                break;
            }
        }
    }

    return BURN_SUCCESS;       
}


int Clear_Burn_Stream_List(DEV_HANDLE hBurnDEV)
{
    int ret;
    BURN_STREAM_T *s_database_ptr;;

    printf("[Database, Clear_Burn_Stream_List] Begin!\n");
    s_database_ptr = get_stream_database(hBurnDEV);
    if(s_database_ptr == NULL)
    {
        printf("[Database, Clear_Burn_Stream_List] Get Stream DataBase Ptr Failed\n");
        return BURN_FAILURE;
    }

    ret = clear_stream_database(hBurnDEV);
    return ret;
}

int config_rtsp_and_ts_param(DEV_HANDLE hBurnDEV)
{
    int i, ret;
    RTSPTS_HANDLE *hStream;

#if 0
    STREAM_INFO_T *s_info_ptr;
    s_info_ptr = get_stream_info_ptr(hBurnDEV);
    if(s_info_ptr == NULL)
    {
        printf("Get Stream Info Ptr Failed\n");
        return BURN_FAILURE;
    }
#else
    BURN_STREAM_T *s_database_ptr;;

    s_database_ptr = get_stream_database(hBurnDEV);
    if(s_database_ptr == NULL)
    {
        printf("Get Stream DataBase Ptr Failed\n");
        return BURN_FAILURE;
    }
#endif
    hStream = get_stream_handle(hBurnDEV);
    if(hStream == NULL)
    {
        printf("Get Stream Handle Failed\n");
        return BURN_FAILURE;            
    }

    for(i = 0; i < STREAM_INFO_MAX_NUM; i++)
    {
        if(s_database_ptr->stream_info[i] != NULL)
        {
//            printf("[config_rtsp_and_ts_param] DataBase[%d] Has Url [%s], Ts [%d]\n", i, s_database_ptr->stream_info[i]->url, s_database_ptr->stream_info[i]->ts_channel);
            //本期只配置URL和TS通道，其他采用默认配置
            ret = config_rtsp_url_and_ts_channel(hStream, s_database_ptr->stream_info[i]->url, s_database_ptr->stream_info[i]->ts_channel);
            if(ret == BURN_FAILURE)
            {
                printf("Config RTSP URL And Ts Channel Failed\n");
            }
        }
    }

    return BURN_SUCCESS;
}

static char rt_set_filename[1000];
static char rt_used_filename[1000];
static int  filename_cnt = 0;
int CBurnData::Burn_Set_RT_File_Name(char *filename)
{
    if(filename == NULL)
    {
        printf("Set RT File Name Is NULL\n");
        return BURN_FAILURE;
    }

    bzero(rt_set_filename, sizeof(rt_set_filename));
    bzero(rt_used_filename, sizeof(rt_used_filename));

    strcpy(rt_set_filename, filename);
    strcpy(rt_used_filename, filename);

	printf("Burn_Set_RT_File_Name suucessed, rt_used_filename = %s, rt_set_filename = %s\n", rt_used_filename, rt_set_filename);
    return BURN_SUCCESS;
}

int CBurnData::Set_RT_File_Name_Update(void)
{
    bzero(rt_used_filename, sizeof(rt_used_filename));
    
    sprintf(rt_used_filename, "%s%d", rt_set_filename, filename_cnt++);

	printf("Set_RT_File_Name_Update, rt_used_filename = %s, rt_set_filename = %s, filename_cnt = %d\n", rt_used_filename, rt_set_filename, filename_cnt);
    
    return BURN_SUCCESS;
}

void CBurnData::Burn_Get_RT_File_Name(char *filename)
{
    if(strlen(rt_used_filename) <= 2)
	{
		printf("strlen(rt_used_filename) <= 2\n");
        strcpy(filename, "test.ts");
		CBurnData::Burn_Set_RT_File_Name("test.ts");
	}
    else
	{
        strcpy(filename, rt_used_filename);
	}

	printf("Burn_Get_RT_File_Name ,filename = %s\n", filename);
}

