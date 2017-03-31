/*
	��������μ�¼��¼��Ƶ��λ����Ϣ�����ڵ���ָ�
*/
#ifndef __UDF_FSTAG_H__
#define __UDF_FSTAG_H__

/**************���1************�����ļ�ϵͳ�����������ַ*****��288����**********************/
#define UDF_SYS_LEN     288   //ϵͳ�������ȣ�AVDP(256) + 16��λ��
#define UDF_VRS_ADDR    16    //udf�����ļ�ϵͳ����������ʼλ��:��17�������飬ռһ����
#define UDF_PVDS_ADDR   32    //PVDS��ʼ��ַ��ռһ����,PVDS = 32, IUVD = 33, PD = 34, LVD = 35 ,USD = 36 , TD = 37
#define UDF_RVDS_ADDR   48    //RVDS��ʼ��ַ��ռһ����,RVDS = 48, IUVD = 49, PD = 50, LVD = 51 ,USD = 52 , TD = 53
#define UDF_LVID_ADDR   64    //LVID��ʼ��ַ��ռһ����,LVID = 64, TD=65, ��80��255��11������Ϊ����
#define UDF_ADVP_ADDR   256   //udf��λ��ָ���ַ��256�鿪ʼ��ռһ��������288��������

#define UDF_PD_ADDR     UDF_PVDS_ADDR
#define UDF_UUID_ADDR   992

#define UDF_FSD_LEN     736   //FSDռ�ó���,��288��ʼΪFSD��ַ����1008λ�ã����������0����,��������ʵ����������м�ռ������㣬���Է�ȥ��������
#define UDF_FSD_ADDR    UDF_SYS_LEN   //272  //FSD��ʼ��ַ

/**************���2************�����ļ�ϵͳ�����������ַ*****��256����**********************/
#define UDF_FS_LENGTH    (UDF_FSD_ADDR + UDF_FSD_LEN )   //�ļ�ϵͳ�ܳ���,1024���������������64K��������λ�ã�����2048��4096�����ļ�������ʼλ�ô�1040��ʼ��


#endif//__UDF_FSTAG_H__
