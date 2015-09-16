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
    B_RUNNING_OK                = 0 ,    // ��������
    ERR_TRAY_IS_OPEN            = 10,    // �����ǿ�����
    ERR_NO_HAVE_DISC            = 11,    // û�й���
    ERR_LOAD_DISC_FAILED        = 12,    // ���ع���ʧ��
    ERR_GET_DISC_INFO_FAILED    = 13,    // ��ȡ������Ϣʧ��
    ERR_DISC_CAN_NOT_WRITE      = 14,    // �����޷�д����
    ERR_DISC_FORMAT_FAILED      = 15,    // ���̸�ʽ��ʧ��
    ERR_DISC_HAS_NO_SPACE       = 16,    // ����û�пռ�
    ERR_WRITE_DISC_FAILED       = 16,    // д����ʧ��
    WARNING_DISC_WILL_NO_SPACE  = 17,    // ���̼���û�пռ�

    ERR_CREATE_FILE_FAILED      = 20,    // �����ϴ����ļ�ʧ��
    ERR_WRITE_FILE_FAILED       = 21,    // ������д�ļ�ʧ��
    ERR_CLOSE_FILE_FAILED       = 22,    // �����Ϲر��ļ�ʧ��
    ERR_CLOSE_DISC_FAILED       = 23,    // ���̷���ʧ��

    ERR_LOCAL_FILE_NO_FIND      = 30,    // �����ļ��޷��ҵ�
    ERR_LOCAL_FILE_READ_FAILED  = 31,    // �����ļ��޷���ȡ
    ERR_LOCAL_DIR_NO_FIND       = 32,    // ����Ŀ¼�޷��ҵ�
    ERR_DISK_HAS_NO_SPACE       = 33,    // Ӳ��û�пռ�
}RUNNING_INFO;

#endif
