#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <RingBuffer.h>

#define DATA_POOL_SIZE   3*1024*1024
#define DATA_INFO_SIZE   500

// ������ϢԪ��
typedef enum{
	HASDISC_E = 0,	
    DISC_TYPE_E,
    DISC_MAXSPEED_E,
    DISC_MAXSIZE_E,
    DISC_USEDSIZE_E,
    DISC_FREESIZE_E,
    DISC_ALARMSIZE_E,
}DISC_ELEMENT;

// Ӳ����ϢԪ��
typedef enum{
	HASDISK_E = 0,	
    DISK_MAXSIZE_E,
    DISK_BACKUPSIZE_E,
    DISK_USEDSIZE_E,
    DISK_FREESIZE_E,
    DISK_ALARMSIZE_E,
}DISK_ELEMENT;


// ����״̬��ϢԪ��
typedef enum{
	TYAY_STATE_E = 0,	
    DISC_STATE_E,
    FILE_SIZE_E,
}RUN_ELEMENT;


// ��¼ҵ�����Ԫ��
typedef enum{
	CACHE_E,	
    MD5_BOOL_E,
    BURN_NUM_E,
    AUTOFORMAT_E,
}PARAM_ELEMENT;

/* �����������Ϣ*/
typedef struct{
    int   file_wait_num;                          // �ļ��ȴ����и���
    int   file_burn_num;                          // �ѿ�¼����ļ�����
    int   dir_wait_num;                           // Ŀ¼�ȴ����и���
    unsigned long buf_size;                       // ���ݻ�������С(Ĭ�� 10M)
	char *file_name;				              // ���ڿ�¼���ļ���
    char *file_wait_list[DATA_INFO_SIZE][2];      // �ȴ���¼���ļ�����[Ҫ��¼���ļ�·��][���ڹ����ϵ�·��]
    char *file_burn_list[DATA_INFO_SIZE];         // �Ѿ���¼���ļ�����
    char *dir_wait_list[DATA_INFO_SIZE][2];       // �ȴ���¼��Ŀ¼����[Ҫ��¼��Ŀ¼·��][���ڹ����ϵ�·��]
    char *data_buf;                               // ���ݻ����� 
    RingBuffer hBuf;                              // ���ݻ���������
}BURN_DATA_T;



#endif
