/*********************************************************************************
作者:passion
时间:2017-04-02
功能:定义UDF文件系统MPEG数据流刻录接口
**********************************************************************************/
#ifndef __UDF_FSCORE_H__
#define __UDF_FSCORE_H__

#include "LibDVDSDK.h"
#include "drvcomm.h"
#include "libudffs.h"

#define MAX_FILE_OR_DIR   256
#define	PAD(val, granularity)	(((val)+(granularity)-1)&~((granularity)-1))
#define MEM_BUFFER_SIZE		(1024 * 1024 * 3)	// 3M

//节点类型
enum NODETYPE
{
   NODETYPE_FILE = 0 ,  //文件
   NODETYPE_DIR  = 1    //目录
};

//udf刻录模式
enum UDFDATAMODE
{
	UDFDATAMODE_VIDEO = 0,  //视频模式
	UDFDATAMODE_DATA  = 1,  //数据模式
};

//光盘状态
enum DVDSTATUS 
{
   DVD_OPEN     = 0 ,   //打开
   DVD_CLOSED   = 1 ,   //关闭
   DVD_READ     = 2 ,   //读数据
   DVD_WRITE    = 3 ,   //写数据
   DVD_RESUME1  = 4 ,	//原盘恢复
   DVD_RESUME2  = 5 ,   //新盘恢复
   DVD_COPY     = 6     //光盘复制
};

typedef struct UDFINFO_T udfinfo_t;
typedef struct UDF_CMD_T udfcmd_t;


//文件目录节点
typedef struct tagFileDirNode
{	
    //节点类型
	int NodeType;

	//节点ID
	int NodeID;
	
	//节点名称
	char Name[256];

	//长度,文件长度或目录名长度
	uint64_t FileSize;

	//文件位置
	uint64_t FileLoca;

	//创建时间
	timestamp ts;

	//父节点
	struct tagFileDirNode * Parent;

	//兄弟节点
	struct tagFileDirNode * Next;
	struct tagFileDirNode * Prior;

	//子节点
	struct tagFileDirNode * Child;
	
}FILDIRNODE;


//文件目录树
typedef struct tagFileDirTree
{
	//节点总数
	int NodeCount;

	//总文件大小
	uint64_t TotalFileSize;

	//首节点
	FILDIRNODE * FirstNode;

	//尾部节点
	FILDIRNODE * LastNode;	
	
}FILEDIRTREE;

//光盘结构	
typedef struct{
	// 光盘类型,默认DVD-R,支持蓝光DVD
	//uint8_t  discType;           

	// 轨道总数
	//int trackCount;
	
    // 是否已经封盘
	//int bClosed;     

	// 介质类型
	//int nDiskType;
	
	// 光盘总容量
	uint64_t nDiskCapicity;

	// 剩余容量
	//int nDiskRemainCapicity;

	// 尾部AVDP位置，距离光盘总容量倒数512的位置，此后512扇区空间内容必须全部写入AVDP作为封盘数据
	//int nDVDFootAVDPAddr;

	// 分区起始地址,也就是FSD起始地址
	//int nPDStratAddr;

	// 分区长度,默认是从FSD到距离光盘尾部257位置
	//int nPDLength;

	// 分区已经刻录的容量
	//int nPDRecorded;

	//uint8_t			fpacket;			/* fixed/variable packets */
	//uint8_t			packet_size;		/* fixed packet size */
	//uint8_t			link_size;			/* link loss size */
	//uint8_t			write_type;			/* mode1 or mode2 */
	//uint8_t			border;				/* border/session */
	//uint8_t			speed;				/* writing speed */
	
	char            ExtenData[CDROM_BLOCK * PACKET_BLOCK_16];      /* 扩展数据 */   //OutUUIDString[32];				/* UUID */OutUUIDString[32];				/* UUID */
	int             ExtenDataLen;       /* 扩展数据长度 */
	
	// udf系统盘区 (0   ~ 63扇区)
	// VRS = 16, PVDS = 32( IUVD = 33, PD = 34, LVD = 35 ,USD = 36 , TD = 37 ), RVDS = 48( IUVD = 49, PD = 50, LVD = 51 ,USD = 52 , TD = 53)
	CDR_TRACK_T udfsys;             
	
	// udf文件盘区 (288 ~ 351扇区)
	// (为文件集和文件实体描述符),FSD=288,FE_ROOT=289,FID_ROOT=290,FE_VIDEO_TS=291,FID_VIDEO_TS=292,FE_VIDEO_VMGIFO=293,FE_VIDEO_VTSIFO=294
	CDR_TRACK_T udffile;            

	struct udf_disc	udf_disc;
}CDRWDISKINFO;

typedef struct{
	char logicalVolIdent[128];
	char volIdent[32];
	char volSetIdent[128];
	char LVInfoTitle[36];
	char LVInfoDataTime[36];
	char LVInfoEmail[36];
	char fileSetIdent[32];
	char copyrightFileIdent[32];
	char abstractFileIdent[32];
}DISC_VOLID_T;

class CUdfCmd
{
public:
	CUdfCmd(){};
	~CUdfCmd(){};

public:
	// 添加目录或文件
	FILDIRNODE *(*addnode)(void *hMem, FILEDIRTREE *FileDirTree, FILDIRNODE *Parent, char *szName, uint64_t filesize, int NodeType);

	// 通过名称查找节点
	FILDIRNODE *(*findnodebyname)(FILEDIRTREE *FileDirTree, char *szName);

	// 通过节点ID
	FILDIRNODE *(*findnodebyid)(FILEDIRTREE *FileDirTree, int nodeid);

	// 判断目录下名称是否重复
	FILDIRNODE *(*GetNodeInDir)(FILDIRNODE *pDirNode, char *szName);

	// 获取目录数量
	int(*getdircount)(FILEDIRTREE *FileDirTree);

	// 获取文件数量
	int(*getfilecount)(FILEDIRTREE *FileDirTree);

	// 初始化文件系统,初始化卷标和UDF文件系统描述符
	int(*InitUdfFs)(udfinfo_t *pUdfInfo, DISC_VOLID_T *pDiscVol);

	// 写流数据到光盘文件, length可以为任意大小
	int(*WriteStream)(udfinfo_t *pUdfInfo, FILDIRNODE *FileNode, uint8_t *Data, int length);

	// 填充空数据到光盘最后, length可以为任意大小
	int(*WriteEmptyStream)(udfinfo_t *pUdfInfo, uint8_t *Data, int length);

	// 封盘光盘
	int(*CloseDisc)(udfinfo_t *pUdfInfo);

	// 在文件轨道写入空数据，需在格式化之后调用, 防止流刻录开始时写轨道的停滞，造成视频丢帧
	int(*WriteFileTrackEmdpy)(udfinfo_t *pUdfInfo, uint32_t emptydize);

	// UDF文件系统测试
	int(*udffstest)(udfinfo_t *pUdfInfo);
};

// UDF文件系统结构
struct UDFINFO_T{
	// 设备句柄
	int fd;
	
	//数据模式
	int m_DataMode;

	// 当前文件位置
	//uint32_t m_CurrentFileLocation;	
	
	void *m_hMem;
	uint8_t *m_pMemBuffer;

	// 临时使用的Buffer : 32 * 2048
	uint8_t	pPackBuff[PACKET32_SIZE];

	// 保留扇区Buffer : 32 * 2048, 用于存放UUID等信息
	//uint8_t	pUUIDBuff[PACKET32_SIZE];

	//光盘信息结构
	CDRWDISKINFO *m_CdRwDiskinfo;
	
	//文件目录树
	FILEDIRTREE *m_FileDirTree;
	
	// UDF cmd
	udfcmd_t *udf_cmd;
	
	// mmc cmd
	struct CDR_CMD_T *cdr_cmd;

	//
	CUdfCmd	udfCmd;
	//class CCdrcmd
	CCDRCmd cdrCmd;
};

//struct UDF_CMD_T{
//	// 添加目录或文件
//	FILDIRNODE *(*addnode)(void *hMem, FILEDIRTREE *FileDirTree, FILDIRNODE *Parent, char *szName, uint64_t filesize, int NodeType);
//
//	// 通过名称查找节点
//	FILDIRNODE *(*findnodebyname)(FILEDIRTREE *FileDirTree, char *szName);
//
//	// 通过节点ID
//	FILDIRNODE *(*findnodebyid)(FILEDIRTREE *FileDirTree, int nodeid);
//
//	// 判断目录下名称是否重复
//	FILDIRNODE *(*GetNodeInDir)(FILDIRNODE *pDirNode,  char *szName);
//
//	// 获取目录数量
//	int (*getdircount)(FILEDIRTREE *FileDirTree);
//
//	// 获取文件数量
//	int (*getfilecount)(FILEDIRTREE *FileDirTree);
//
//	// 初始化文件系统,初始化卷标和UDF文件系统描述符
//	int (*InitUdfFs)(udfinfo_t *pUdfInfo, DISC_VOLID_T *pDiscVol);
//
//	// 写流数据到光盘文件, length可以为任意大小
//	int (*WriteStream)(udfinfo_t *pUdfInfo, FILDIRNODE *FileNode, uint8_t *Data, int length);
//
//	// 填充空数据到光盘最后, length可以为任意大小
//	int (*WriteEmptyStream)(udfinfo_t *pUdfInfo, uint8_t *Data, int length);
//
//	// 封盘光盘
//	int (*CloseDisc)(udfinfo_t *pUdfInfo);
//
//	// 在文件轨道写入空数据，需在格式化之后调用, 防止流刻录开始时写轨道的停滞，造成视频丢帧
//	int (*WriteFileTrackEmdpy)(udfinfo_t *pUdfInfo, uint32_t emptydize);
//
//	// UDF文件系统测试
//	int (*udffstest)(udfinfo_t *pUdfInfo);
//};

// 创建UDF
udfinfo_t * DVDRec_UdfCreate(int fd, uint16_t DataMode);

// 销毁UDF
int DVDRec_UdfFree(udfinfo_t *pUdfInfo);

// 释放UDF文件系统目录
int DVDRec_UdfTreeFree(udfinfo_t *pUdfInfo);

#endif//__UDF_FSCORE_H__

