#ifndef __STREAM_SELF_INCLUDE_H__
#define __STREAM_SELF_INCLUDE_H__

/*Database.c*/
BURN_STREAM_T *get_stream_database(DEV_HANDLE hBurnDEV);
RTSPTS_HANDLE *get_stream_handle(DEV_HANDLE hBurnDEV);
STREAM_INFO_T *get_stream_info_ptr(DEV_HANDLE hBurnDEV);
STREAM_INFO_T *get_stream_info(STREAM_INFO_T *s_info_ptr, int stream_id);
DEV_HANDLE get_dev_handle(RTSPTS_HANDLE *hStream);
int create_stream_database(DEV_HANDLE hBurnDEV);
int add_stream_to_list(DEV_HANDLE hBurnDEV, STREAM_INFO_T *stream);
int config_rtsp_and_ts_param(DEV_HANDLE hBurnDEV);

/*Stream.c*/
int config_rtsp_url_and_ts_channel(RTSPTS_HANDLE *hStream, char *stream_url, int ts_channel);
void Print_Stream_Struct(RTSPTS_HANDLE *hStream);
void initrtsp(RTSP_CLIENT_CONFIG *cfg,char* name);

#endif

