#ifndef __BURN_STATE_H__
#define __BURN_STATE_H__
#include <Burn_SDK.h>

typedef enum
{
    B_DEV_TRAY_CLOSE,
    B_DEV_TRAY_OPEN,
}DEV_TRAY_STAT;

typedef enum
{
    B_ALREAD = 0,
    B_START,
    B_STOP,
	B_CLOSED,
}BURN_STAT;

typedef enum
{
    B_NORMALLY = 0,
    B_ABNORMAL,
}RUNNING_STATE;

typedef enum
{
    B_RUNNING_OK                = 0 ,    // 运行正常
    ERR_TRAY_IS_OPEN            = 10,    // 托盘是开启的
    ERR_NO_HAVE_DISC            = 11,    // 没有光盘
    ERR_LOAD_DISC_FAILED        = 12,    // 加载光盘失败
    ERR_GET_DISC_INFO_FAILED    = 13,    // 获取光盘信息失败
    ERR_DISC_CAN_NOT_WRITE      = 14,    // 光盘无法写操作
    ERR_DISC_FORMAT_FAILED      = 15,    // 光盘格式化失败
    ERR_DISC_HAS_NO_SPACE       = 16,    // 光盘没有空间
    ERR_WRITE_DISC_FAILED       = 16,    // 写光盘失败
    WARNING_DISC_WILL_NO_SPACE  = 17,    // 光盘即将没有空间

    ERR_CREATE_FILE_FAILED      = 20,    // 光盘上创建文件失败
    ERR_WRITE_FILE_FAILED       = 21,    // 光盘上写文件失败
    ERR_CLOSE_FILE_FAILED       = 22,    // 光盘上关闭文件失败
    ERR_CLOSE_DISC_FAILED       = 23,    // 光盘封盘失败

    ERR_LOCAL_FILE_NO_FIND      = 30,    // 本地文件无法找到
    ERR_LOCAL_FILE_READ_FAILED  = 31,    // 本地文件无法读取
    ERR_LOCAL_DIR_NO_FIND       = 32,    // 本地目录无法找到
    ERR_DISK_HAS_NO_SPACE       = 33,    // 硬盘没有空间
}RUNNING_INFO;

#endif
