#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <RingBuffer.h>

#define DATA_POOL_SIZE   3*1024*1024
#define DATA_INFO_SIZE   500

// 光盘信息元素
typedef enum{
	HASDISC_E = 0,	
    DISC_TYPE_E,
    DISC_MAXSPEED_E,
    DISC_MAXSIZE_E,
    DISC_USEDSIZE_E,
    DISC_FREESIZE_E,
    DISC_ALARMSIZE_E,
}DISC_ELEMENT;

// 硬盘信息元素
typedef enum{
	HASDISK_E = 0,	
    DISK_MAXSIZE_E,
    DISK_BACKUPSIZE_E,
    DISK_USEDSIZE_E,
    DISK_FREESIZE_E,
    DISK_ALARMSIZE_E,
}DISK_ELEMENT;


// 运行状态信息元素
typedef enum{
	TYAY_STATE_E = 0,	
    DISC_STATE_E,
    FILE_SIZE_E,
}RUN_ELEMENT;


// 刻录业务参数元素
typedef enum{
	CACHE_E,	
    MD5_BOOL_E,
    BURN_NUM_E,
    AUTOFORMAT_E,
}PARAM_ELEMENT;

/* 数据类相关信息*/
typedef struct{
    int   file_wait_num;                          // 文件等待队列个数
    int   file_burn_num;                          // 已刻录完成文件个数
    int   dir_wait_num;                           // 目录等待队列个数
    unsigned long buf_size;                       // 数据缓冲区大小(默认 10M)
	char *file_name;				              // 正在刻录的文件名
    char *file_wait_list[DATA_INFO_SIZE][2];      // 等待刻录的文件队列[要刻录的文件路径][可在光盘上的路径]
    char *file_burn_list[DATA_INFO_SIZE];         // 已经刻录的文件队列
    char *dir_wait_list[DATA_INFO_SIZE][2];       // 等待刻录的目录队列[要刻录的目录路径][可在光盘上的路径]
    char *data_buf;                               // 数据缓冲区 
    RingBuffer hBuf;                              // 数据缓冲操作句柄
}BURN_DATA_T;



#endif
