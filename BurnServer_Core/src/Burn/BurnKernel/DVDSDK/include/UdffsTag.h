/*
	定义了如何记录刻录视频的位置信息，便于掉电恢复
*/
#ifndef __UDF_FSTAG_H__
#define __UDF_FSTAG_H__

/**************轨道1************定义文件系统卷标描述符地址*****共288扇区**********************/
#define UDF_SYS_LEN     288   //系统盘区长度，AVDP(256) + 16的位置
#define UDF_VRS_ADDR    16    //udf光盘文件系统，引导区起始位置:第17个引导块，占一个包
#define UDF_PVDS_ADDR   32    //PVDS起始地址，占一个包,PVDS = 32, IUVD = 33, PD = 34, LVD = 35 ,USD = 36 , TD = 37
#define UDF_RVDS_ADDR   48    //RVDS起始地址，占一个包,RVDS = 48, IUVD = 49, PD = 50, LVD = 51 ,USD = 52 , TD = 53
#define UDF_LVID_ADDR   64    //LVID起始地址，占一个包,LVID = 64, TD=65, 从80到255共11个包都为空闲
#define UDF_ADVP_ADDR   256   //udf定位点指针地址，256块开始，占一个包，到288扇区结束

#define UDF_PD_ADDR     UDF_PVDS_ADDR
#define UDF_UUID_ADDR   992

#define UDF_FSD_LEN     736   //FSD占用长度,从288开始为FSD地址，到1008位置，不够的填充0数据,这个可以适当调整，把中间空间调整大点，可以放去其他数据
#define UDF_FSD_ADDR    UDF_SYS_LEN   //272  //FSD开始地址

/**************轨道2************定义文件系统卷标描述符地址*****共256扇区**********************/
#define UDF_FS_LENGTH    (UDF_FSD_ADDR + UDF_FSD_LEN )   //文件系统总长度,1024扇区（建议调整到64K的整数倍位置，例如2048或，4096），文件数据起始位置从1040开始，


#endif//__UDF_FSTAG_H__
