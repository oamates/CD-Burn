#include <inttypes.h>

#ifndef _ECMA_167_H
#define _ECMA_167_H 1

/* Character set specification (ECMA 167r3 1/7.2.1) */
typedef struct
{
	uint8_t		charSetType;   		//字符集类型，默认CS0
	uint8_t		charSetInfo[63];    //字符集信息
} __attribute__ ((packed)) charspec;

/* Character Set Type (ECMA 167r3 1/7.2.1.1) */
#define CHARSPEC_TYPE_CS0		0x00	/* (1/7.2.2) */
#define CHARSPEC_TYPE_CS1		0x01	/* (1/7.2.3) */
#define CHARSPEC_TYPE_CS2		0x02	/* (1/7.2.4) */
#define CHARSPEC_TYPE_CS3		0x03	/* (1/7.2.5) */
#define CHARSPEC_TYPE_CS4		0x04	/* (1/7.2.6) */
#define CHARSPEC_TYPE_CS5		0x05	/* (1/7.2.7) */
#define CHARSPEC_TYPE_CS6		0x06	/* (1/7.2.8) */
#define CHARSPEC_TYPE_CS7		0x07	/* (1/7.2.9) */
#define CHARSPEC_TYPE_CS8		0x08	/* (1/7.2.10) */

typedef uint8_t		dstring;

/* Timestamp (ECMA 167r3 1/7.3) */
typedef struct
{
	uint16_t	typeAndTimezone;           //时区类型，必须定义正确的时区，才可以播放
	int16_t		year;
	uint8_t		month;
	uint8_t		day;
	uint8_t		hour;
	uint8_t		minute;
	uint8_t		second;
	uint8_t		centiseconds;
	uint8_t		hundredsOfMicroseconds;
	uint8_t		microseconds;
} __attribute__ ((packed)) timestamp;

/* Type and Time Zone (ECMA 167r3 1/7.3.1) */
#define TIMESTAMP_TYPE_MASK			0xF000
#define TIMESTAMP_TYPE_CUT			0x0000
#define TIMESTAMP_TYPE_LOCAL		0x1000
#define TIMESTAMP_TYPE_AGREEMENT	0x2000
#define TIMESTAMP_TIMEZONE_MASK		0x0FFF

/* Entity identifier 实体标识符 (ECMA 167r3 1/7.4) */  
typedef struct
{
	uint8_t		flags;                //标志，默认0x00
	uint8_t		ident[23];            //标识
	uint8_t		identSuffix[8];       //标识下标
} __attribute__ ((packed)) regid;     //注册ID

/* Flags (ECMA 167r3 1/7.4.1) */
#define ENTITYID_FLAGS_DIRTY		0x00
#define ENTITYID_FLAGS_PROTECTED	0x01

/* Volume Structure Descriptor 卷标结构描述符 (ECMA 167r3 2/9.1) */
#define VSD_STD_ID_LEN			5
struct volStructDesc
{
	uint8_t		structType;    				 //结构类型
	uint8_t		stdIdent[VSD_STD_ID_LEN];    //标准标识
	uint8_t		structVersion;				 //结构版本
	uint8_t		structData[2041];
} __attribute__ ((packed));



/* If we find NSR0x then call it udf:		   
	NSR01 for UDF 1.00		   
	NSR02 for UDF 1.50		   
	NSR03 for UDF 2.00 或 2.01 ,2,5,2,6
*/

/* Standard Identifier (EMCA 167r2 2/9.1.2) */
#define VSD_STD_ID_NSR01		"NSR01"	/* (3/9.1) */
#define VSD_STD_ID_NSR02		"NSR02"	/* (3/9.1) */

/* Standard Identifier (ECMA 167r3 2/9.1.2) */

#define VSD_STD_ID_BEA01		"BEA01"	/* 卷标描述符，起始卷标 (2/9.2) */
#define VSD_STD_ID_BOOT2		"BOOT2"	/* 卷标描述符，光盘启动 (2/9.4) */
#define VSD_STD_ID_CD001		"CD001"	/* (ECMA-119) 可能是 iso9660 或 udf */
#define VSD_STD_ID_CDW02		"CDW02"	/* 卷标描述符, 可读写的ISO或udf (ECMA-168) */
#define VSD_STD_ID_NSR03		"NSR03"	/* (3/9.1) */
#define VSD_STD_ID_TEA01		"TEA01"	/* 卷标描述符，终止卷标 (2/9.3) */

/* Beginning Extended Area Descriptor (ECMA 167r3 2/9.2) */
struct beginningExtendedAreaDesc
{
	uint8_t		structType;
	uint8_t		stdIdent[VSD_STD_ID_LEN];
	uint8_t		structVersion;
	uint8_t		structData[2041];
} __attribute__ ((packed));

/* Terminating Extended Area Descriptor (ECMA 167r3 2/9.3) */
struct terminatingExtendedAreaDesc
{
	uint8_t		structType;
	uint8_t		stdIdent[VSD_STD_ID_LEN];
	uint8_t		structVersion;
	uint8_t		structData[2041];
} __attribute__ ((packed));

/* Boot Descriptor (ECMA 167r3 2/9.4) bytes-2056 */
struct bootDesc
{
	uint8_t		structType;
	uint8_t		stdIdent[VSD_STD_ID_LEN];
	uint8_t		structVersion;
	uint8_t		reserved1;
	regid		archType;
	regid		bootIdent;
	uint32_t	bootExtLocation;
	uint32_t	bootExtLength;
	uint64_t	loadAddress;
	uint64_t	startAddress;
	timestamp	descCreationDateAndTime;
	uint16_t	flags;
	uint8_t		reserved2[32];
	uint8_t		bootUse[1906];
} __attribute__ ((packed));

/* Flags (ECMA 167r3 2/9.4.12) */
#define BOOT_FLAGS_ERASE		0x01

/* Extent Descriptor (ECMA 167r3 3/7.1) */
typedef struct
{
	uint32_t	extLength;           //盘区长度
	uint32_t	extLocation;          //盘区位置
} __attribute__ ((packed)) extent_ad;

/* Descriptor Tag (ECMA 167r3 3/7.2) */
typedef struct
{ 
	uint16_t	tagIdent;           //标记类型
	uint16_t	descVersion;        //描述符版本
	uint8_t		tagChecksum;        //标记校验
	uint8_t		reserved;           //保留,0x00
	uint16_t	tagSerialNum;       //标记序列号
	uint16_t	descCRC;            //描述符CRC
	uint16_t	descCRCLength;      //描述符CRC长度，默认16
	uint32_t	tagLocation;        //标记块位置
} __attribute__ ((packed)) tag;   //描述符标记块

/* Tag Identifier (ECMA 167r3 3/7.2.1) */
#define TAG_IDENT_PVD			0x0001
#define TAG_IDENT_AVDP			0x0002
#define TAG_IDENT_VDP			0x0003
#define TAG_IDENT_IUVD			0x0004
#define TAG_IDENT_PD			0x0005
#define TAG_IDENT_LVD			0x0006
#define TAG_IDENT_USD			0x0007
#define TAG_IDENT_TD			0x0008
#define TAG_IDENT_LVID			0x0009

/* NSR Descriptor (ECMA 167r3 3/9.1) */
struct NSRDesc
{
	uint8_t		structType;
	uint8_t		stdIdent[VSD_STD_ID_LEN];
	uint8_t		structVersion;
	uint8_t		reserved;
	uint8_t		structData[2040];
} __attribute__ ((packed));
	
/* Primary Volume Descriptor (ECMA 167r3 3/10.1) */
struct primaryVolDesc                 //主卷标描述符,MVDS
{
	tag		descTag;                  //标记描述块
	uint32_t	volDescSeqNum;        //卷标描述序列号
	uint32_t	primaryVolDescNum;    //主卷标描述序列号
	dstring		volIdent[32];         //卷标标识,数组的最后一个字节为长度,即volIdent[31]位置存放该字符串长度
	uint16_t	volSeqNum;            //卷标序列号
	uint16_t	maxVolSeqNum;         //最大卷标序列号
	uint16_t	interchangeLvl;       //交互逻辑卷标
	uint16_t	maxInterchangeLvl;    //最大交互逻辑卷标
	uint32_t	charSetList;          //字符集列表
	uint32_t	maxCharSetList;       //最大字符集列表
	dstring		volSetIdent[128];     //卷标集标识,存放的时间，数组的最后一个字节为长度
	charspec	descCharSet;          //描述符字符集
	charspec	explanatoryCharSet;   //解释性字符集
	extent_ad	volAbstract;          //卷标抽象,使用扩展盘区结构
	extent_ad	volCopyright;         //卷标版权,使用扩展盘区结构
	regid		appIdent;             //应用标识，注册iD
	timestamp	recordingDateAndTime; //记录时间戳
	regid		impIdent;             //实施标识，注册ID
	uint8_t		impUse[64];           //实施使用
	uint32_t	predecessorVolDescSeqLocation;//前任卷标描述序列位置
	uint16_t	flags;                        //标志
	uint8_t		reserved[22];                 //保留
} __attribute__ ((packed));

/* Flags (ECMA 167r3 3/10.1.21) */
#define PVD_FLAGS_VSID_COMMON		0x0001

/* Anchor Volume Descriptor Pointer (ECMA 167r3 3/10.2) */
struct anchorVolDescPtr              	//定位点指针,AVDP
{ 
	tag		    descTag;             	 //标记描述块
	extent_ad	mainVolDescSeqExt;  	 //主卷标描述序列盘区，指定了PVDS的起始位置
	extent_ad	reserveVolDescSeqExt;	 //保留卷标描述序列盘区
	uint8_t	 	reserved[480];           //保留
} __attribute__ ((packed)); 

/* Volume Descriptor Pointer (ECMA 167r3 3/10.3) */
struct volDescPtr                        //卷标描述指针，VDP
{ 
	tag		descTag;                     //标记描述块
	uint32_t	volDescSeqNum;			 //卷标描述序列号
	extent_ad	nextVolDescSeqExt;       //下一个卷标描述序列盘区
	uint8_t		reserved[484];
} __attribute__ ((packed));

/* Implementation Use Volume Descriptor (ECMA 167r3 3/10.4) */
struct impUseVolDesc                     //实施使用的卷标描述
{
	tag		descTag;                     //标记描述块
	uint32_t	volDescSeqNum; 			 //卷标描述序列号
	regid		impIdent;                //实施标识
	uint8_t		impUse[460];             //实施使用数组
} __attribute__ ((packed));

//附属于impUseVolDesc.impUse[460];
struct LVInformation 
{
	charspec LVICharset;
	dstring LogicalVolumeIdentifier[128];
	dstring LVInfo1[36];
	dstring LVInfo2[36];
	dstring LVInfo3[36];
	regid	impIdent;
	uint8_t ImplementationUse[128];
} __attribute__ ((packed));


/* Partition Descriptor (ECMA 167r3 3/10.5) */
struct partitionDesc                    //分区描述，PD
{ 
	tag		descTag;						//标记描述块
	uint32_t	volDescSeqNum;		    	//卷标描述序列号
	uint16_t	partitionFlags;				//分区标记
	uint16_t	partitionNumber;			//分区号
	regid		partitionContents;      	//分区内容，指文件系统版本
	uint8_t		partitionContentsUse[128];  //分区内容使用
	uint32_t	accessType;                 //访问类型，PD_ACCESS_TYPE_WRITE_ONCE
	uint32_t	partitionStartingLocation;   //分区起始位置
	uint32_t	partitionLength;             //分区长度，也指定了分区结束位置
	regid		impIdent;					 //实施标识
	uint8_t		impUse[128];                 //实施使用
	uint8_t		reserved[156];               //保留
} __attribute__ ((packed));

/* Partition Flags (ECMA 167r3 3/10.5.3) */
#define PD_PARTITION_FLAGS_ALLOC	0x0001

/* Partition Contents (ECMA 167r2 3/10.5.3) */
#define PD_PARTITION_CONTENTS_NSR02	"+NSR02"

/* Partition Contents (ECMA 167r3 3/10.5.5) */
#define PD_PARTITION_CONTENTS_FDC01	"+FDC01"
#define PD_PARTITION_CONTENTS_CD001	"+CD001"
#define PD_PARTITION_CONTENTS_CDW02	"+CDW02"
#define PD_PARTITION_CONTENTS_NSR03	"+NSR03"

/* Access Type (ECMA 167r3 3/10.5.7) */
#define PD_ACCESS_TYPE_NONE			0x00000000
#define PD_ACCESS_TYPE_READ_ONLY	0x00000001
#define PD_ACCESS_TYPE_WRITE_ONCE	0x00000002
#define PD_ACCESS_TYPE_REWRITABLE	0x00000003
#define PD_ACCESS_TYPE_OVERWRITABLE	0x00000004

/* Logical Volume Descriptor (ECMA 167r3 3/10.6) */
struct logicalVolDesc               		//逻辑卷标描述，LVD
{
	tag		descTag;						//标记描述块
	uint32_t	volDescSeqNum;     			//卷标描述序列号
	charspec	descCharSet;             	//描述字符集
	dstring		logicalVolIdent[128];       //逻辑卷标标识
	uint32_t	logicalBlockSize;           //逻辑块字节，2048
	regid		domainIdent;                //域标识
	uint8_t		logicalVolContentsUse[16];  //逻辑卷标内容使用,
	uint32_t	mapTableLength;             //映射表长度
	uint32_t	numPartitionMaps;           //分区映射表数量
	regid		impIdent;                   //实施标识
	uint8_t		impUse[128];                //实施使用
	extent_ad	integritySeqExt;            //完整序列盘区
	uint8_t		partitionMaps[0];           //分区映射
} __attribute__ ((packed));

/* Generic Partition Map (ECMA 167r3 3/10.7.1) */
struct genericPartitionMap
{
	uint8_t		partitionMapType;
	uint8_t		partitionMapLength;
	uint8_t		partitionMapping[0];
} __attribute__ ((packed));

/* Partition Map Type (ECMA 167r3 3/10.7.1.1) */
#define GP_PARTITION_MAP_TYPE_UNDEF	0x00
#define GP_PARTIITON_MAP_TYPE_1		0x01
#define GP_PARTITION_MAP_TYPE_2		0x02

/* Type 1 Partition Map (ECMA 167r3 3/10.7.2) */
struct genericPartitionMap1
{
	uint8_t		partitionMapType;
	uint8_t		partitionMapLength;
	uint16_t	volSeqNum;
	uint16_t	partitionNum;
} __attribute__ ((packed));

/* Type 2 Partition Map (ECMA 167r3 3/10.7.3) */
struct genericPartitionMap2
{
	uint8_t		partitionMapType;
	uint8_t		partitionMapLength; 
	uint8_t		partitionIdent[62];
} __attribute__ ((packed));

/* Unallocated Space Descriptor (ECMA 167r3 3/10.8) */
struct unallocSpaceDesc
{
	tag		descTag;
	uint32_t	volDescSeqNum;   //卷描述序列号
	uint32_t	numAllocDescs;   //分配描述符的数量
	extent_ad	allocDescs[0];   //分配描述的盘区
} __attribute__ ((packed));

/* Terminating Descriptor (ECMA 167r3 3/10.9) */
struct terminatingDesc                //终止描述,封盘标识，TD
{
	tag		descTag;                  //描述符标记块
	uint8_t		reserved[496];
} __attribute__ ((packed));

/* Logical Volume Integrity Descriptor (ECMA 167r3 3/10.10) */
struct logicalVolIntegrityDesc        //逻辑卷标完整性描述，LVID
{
	tag		descTag;				  //描述符标记块
	timestamp	recordingDateAndTime; //记录时间戳
	uint32_t	integrityType;        //完整性类型
	extent_ad	nextIntegrityExt;     //下一个完整的盘区
	uint8_t		logicalVolContentsUse[32];  //逻辑卷标内容使用
	uint32_t	numOfPartitions;            //分区数量
	uint32_t	lengthOfImpUse;             //实施使用长度,决定了impUse[0]的长度:struct logicalVolIntegrityDescImpUse
	uint32_t	freeSpaceTable[0];          //释放可用空间表，对于只读光盘这里可以是0
	uint32_t	sizeTable[0];               //字节表，映射了可用空间字节，1个位对应一个块，可以为0
	uint8_t		impUse[0];                  //实施使用，附加描述了分区内有多少目录，多少文件，已经读写版本等信息

} __attribute__ ((packed));

/* Integrity Type (ECMA 167r3 3/10.10.3) */
#define LVID_INTEGRITY_TYPE_OPEN	0x00000000
#define LVID_INTEGRITY_TYPE_CLOSE	0x00000001

/* Recorded Address (ECMA 167r3 4/7.1) */
typedef struct 
{
	uint32_t	logicalBlockNum;
	uint16_t	partitionReferenceNum;
} __attribute__ ((packed)) lb_addr;

/* Short Allocation Descriptor (ECMA 167r3 4/14.14.1) */
typedef struct
{
        uint32_t	extLength;
        uint32_t	extPosition;
} __attribute__ ((packed)) short_ad;

/* Long Allocation Descriptor (ECMA 167r3 4/14.14.2) */
typedef struct
{
	uint32_t	extLength;
	lb_addr		extLocation;
	uint8_t		impUse[6];
} __attribute__ ((packed)) long_ad;

/* Extended Allocation Descriptor (ECMA 167r3 4/14.14.3) */
typedef struct
{
	uint32_t	extLength;
	uint32_t	recordedLength;
	uint32_t	informationLength;
	lb_addr		extLocation;
} __attribute__ ((packed)) ext_ad;

/* Descriptor Tag (ECMA 167r3 4/7.2 - See 3/7.2) */

/* Tag Identifier (ECMA 167r3 4/7.2.1) */
#define TAG_IDENT_FSD			0x0100
#define TAG_IDENT_FID			0x0101
#define TAG_IDENT_AED			0x0102
#define TAG_IDENT_IE			0x0103
#define TAG_IDENT_TE			0x0104
#define TAG_IDENT_FE			0x0105
#define TAG_IDENT_EAHD			0x0106
#define TAG_IDENT_USE			0x0107
#define TAG_IDENT_SBD			0x0108
#define TAG_IDENT_PIE			0x0109
#define TAG_IDENT_EFE			0x010A


struct filetag 
{ /* ISO 13346 4/7.2 */
uint16_t TagIdentifier;
uint16_t DescriptorVersion;
uint8_t TagChecksum;
uint32_t Reserved;
uint16_t TagSerialNumber ;
uint16_t DescriptorCRC;
uint16_t DescriptorCRCLength ;
uint32_t TagLocation;
}file_tag;




/* File Set Descriptor (ECMA 167r3 4/14.1) */
struct fileSetDesc
{
	tag		descTag; 					 //描述符标记
	timestamp	recordingDateAndTime;    //记录时间戳
	uint16_t	interchangeLvl;          //交互逻辑卷
	uint16_t	maxInterchangeLvl;       //最大交互逻辑卷
	uint32_t	charSetList;             //字符集列表
	uint32_t	maxCharSetList;          //最大字符集列表
	uint32_t	fileSetNum;              //文件集数量
	uint32_t	fileSetDescNum;          //文件集描述数量
	charspec	logicalVolIdentCharSet;  //逻辑卷识别字符集
	dstring		logicalVolIdent[128];    //逻辑卷标识
	charspec	fileSetCharSet;          //文件集字符集
	dstring		fileSetIdent[32];        //文件集标识
	dstring		copyrightFileIdent[32];  //文件标识版权
	dstring		abstractFileIdent[32];   //抽象文件标识
	long_ad		rootDirectoryICB;        //根目录信息块
	regid		domainIdent;             //域标识
	long_ad		nextExt;                 //下一个盘区
	long_ad		streamDirectoryICB;      //流目录信息块
	uint8_t		reserved[32];            //保留,0x00
} __attribute__ ((packed));

/* Partition Header Descriptor (ECMA 167r3 4/14.3) */
struct partitionHeaderDesc
{
	short_ad	unallocSpaceTable;
	short_ad	unallocSpaceBitmap;
	short_ad	partitionIntegrityTable;
	short_ad	freedSpaceTable;
	short_ad	freedSpaceBitmap;
	uint8_t		reserved[88];
} __attribute__ ((packed));

/* File Identifier Descriptor (ECMA 167r3 4/14.4) */
struct fileIdentDesc
{
	tag		descTag;                     //描述符标记
	uint16_t	fileVersionNum;          //文件版本
	uint8_t		fileCharacteristics;     //文件特征，隐藏，目录，删除，元数据
	uint8_t		lengthFileIdent;         //文件识别长度
	long_ad		icb;                     //对应的信息块
	uint16_t	lengthOfImpUse;          //使用的长度
	uint8_t		impUse[0];               //实际使用
	uint8_t		fileIdent[0];            //文件标识
	uint8_t		padding[0];              //填充
} __attribute__ ((packed));            

/* File Characteristics (ECMA 167r3 4/14.4.3) */
#define FID_FILE_CHAR_HIDDEN		0x01
#define FID_FILE_CHAR_DIRECTORY		0x02
#define FID_FILE_CHAR_DELETED		0x04
#define FID_FILE_CHAR_PARENT		0x08
#define FID_FILE_CHAR_METADATA		0x10

/* Allocation Ext Descriptor (ECMA 167r3 4/14.5) */
struct allocExtDesc
{
	tag		descTag;
	uint32_t	previousAllocExtLocation;
	uint32_t	lengthAllocDescs;
} __attribute__ ((packed));

/* ICB Tag (ECMA 167r3 4/14.6) */
typedef struct
{
	uint32_t	priorRecordedNumDirectEntries;   //前一个记录的号直接入口
	uint16_t	strategyType;                    //策略类型
	uint16_t	strategyParameter;               //策略参数
	uint16_t	numEntries;                      //编号入口
	uint8_t		reserved;                        //保留,0x00
	uint8_t		fileType;                        //文件类型
	lb_addr		parentICBLocation;               //父信息块位置
	uint16_t	flags;                           //标记
} __attribute__ ((packed)) icbtag;

/* Strategy Type (ECMA 167r3 4/14.6.2) */
#define ICBTAG_STRATEGY_TYPE_UNDEF	0x0000
#define ICBTAG_STRATEGY_TYPE_1		0x0001
#define ICBTAG_STRATEGY_TYPE_2		0x0002
#define ICBTAG_STRATEGY_TYPE_3		0x0003
#define ICBTAG_STRATEGY_TYPE_4		0x0004

/* File Type (ECMA 167r3 4/14.6.6) */
#define ICBTAG_FILE_TYPE_UNDEF		0x00
#define ICBTAG_FILE_TYPE_USE		0x01
#define ICBTAG_FILE_TYPE_PIE		0x02
#define ICBTAG_FILE_TYPE_IE			0x03
#define ICBTAG_FILE_TYPE_DIRECTORY	0x04
#define ICBTAG_FILE_TYPE_REGULAR	0x05
#define ICBTAG_FILE_TYPE_BLOCK		0x06
#define ICBTAG_FILE_TYPE_CHAR		0x07
#define ICBTAG_FILE_TYPE_EA			0x08
#define ICBTAG_FILE_TYPE_FIFO		0x09
#define ICBTAG_FILE_TYPE_SOCKET		0x0A
#define ICBTAG_FILE_TYPE_TE			0x0B
#define ICBTAG_FILE_TYPE_SYMLINK	0x0C
#define ICBTAG_FILE_TYPE_STREAMDIR	0x0D

/* Flags (ECMA 167r3 4/14.6.8) */
#define ICBTAG_FLAG_AD_MASK			0x0007
#define ICBTAG_FLAG_AD_SHORT		0x0000
#define ICBTAG_FLAG_AD_LONG			0x0001
#define ICBTAG_FLAG_AD_EXTENDED		0x0002
#define ICBTAG_FLAG_AD_IN_ICB		0x0003
#define ICBTAG_FLAG_SORTED			0x0008
#define ICBTAG_FLAG_NONRELOCATABLE	0x0010
#define ICBTAG_FLAG_ARCHIVE			0x0020
#define ICBTAG_FLAG_SETUID			0x0040
#define ICBTAG_FLAG_SETGID			0x0080
#define ICBTAG_FLAG_STICKY			0x0100
#define ICBTAG_FLAG_CONTIGUOUS		0x0200
#define ICBTAG_FLAG_SYSTEM			0x0400
#define ICBTAG_FLAG_TRANSFORMED		0x0800
#define ICBTAG_FLAG_MULTIVERSIONS	0x1000
#define ICBTAG_FLAG_STREAM			0x2000

/* Indirect Entry (ECMA 167r3 4/14.7) */
struct indirectEntry                //间接入口
{                                   
	tag		descTag;                //描述符标记
	icbtag		icbTag;             //信息块
	long_ad		indirectICB;        //间接信息块
} __attribute__ ((packed));

/* Terminal Entry (ECMA 167r3 4/14.8) */
struct terminalEntry               //结束入口
{ 
	tag		descTag;               //描述符标记
	icbtag		icbTag;            //信息块
} __attribute__ ((packed));

/* File Entry (ECMA 167r3 4/14.9) */
struct fileEntry                   //文件入口结构，总长度不超过2048
{
	tag		descTag;               		//描述符标记
	icbtag		icbTag;            		//信息块
	uint32_t	uid;               		//uid
	uint32_t	gid;               		//gid
	uint32_t	permissions;       		//权限
	uint16_t	fileLinkCount;     		//文件链接数量
	uint8_t		recordFormat;      		//记录格式
	uint8_t		recordDisplayAttr;  	//记录显示属性
	uint32_t	recordLength;       	//记录长度
	uint64_t	informationLength;   	//信息长度
	uint64_t	logicalBlocksRecorded;  //已经记录的逻辑块数量
	timestamp	accessTime;             //访问时间
	timestamp	modificationTime;       //修改时间
	timestamp	attrTime;               //属性时间
	uint32_t	checkpoint;             //检查点
	long_ad		extendedAttrICB;        //扩展属性信息块
	regid		impIdent;               //执行标识
	uint64_t	uniqueID;               //唯一编码
	uint32_t	lengthExtendedAttr;     //扩展属性长度
	uint32_t	lengthAllocDescs;       //描述符分配长度
	uint8_t		extendedAttr[0];        //扩展属性
	uint8_t		allocDescs[0];     		//分配描述符
	//short_ad	allocDescs;          	//分配描述符
} __attribute__ ((packed));

/* Permissions (ECMA 167r3 4/14.9.5) */
#define FE_PERM_O_EXEC			0x00000001U
#define FE_PERM_O_WRITE			0x00000002U
#define FE_PERM_O_READ			0x00000004U
#define FE_PERM_O_CHATTR		0x00000008U
#define FE_PERM_O_DELETE		0x00000010U
#define FE_PERM_G_EXEC			0x00000020U
#define FE_PERM_G_WRITE			0x00000040U
#define FE_PERM_G_READ			0x00000080U
#define FE_PERM_G_CHATTR		0x00000100U
#define FE_PERM_G_DELETE		0x00000200U
#define FE_PERM_U_EXEC			0x00000400U
#define FE_PERM_U_WRITE			0x00000800U
#define FE_PERM_U_READ			0x00001000U
#define FE_PERM_U_CHATTR		0x00002000U
#define FE_PERM_U_DELETE		0x00004000U

/* Record Format (ECMA 167r3 4/14.9.7) */
#define FE_RECORD_FMT_UNDEF				0x00
#define FE_RECORD_FMT_FIXED_PAD			0x01
#define FE_RECORD_FMT_FIXED				0x02
#define FE_RECORD_FMT_VARIABLE8			0x03
#define FE_RECORD_FMT_VARIABLE16		0x04
#define FE_RECORD_FMT_VARIABLE16_MSB	0x05
#define FE_RECORD_FMT_VARIABLE32		0x06
#define FE_RECORD_FMT_PRINT				0x07
#define FE_RECORD_FMT_LF				0x08
#define FE_RECORD_FMT_CR				0x09
#define FE_RECORD_FMT_CRLF				0x0A
#define FE_RECORD_FMT_LFCR				0x0B

/* Record Display Attributes (ECMA 167r3 4/14.9.8) */
#define FE_RECORD_DISPLAY_ATTR_UNDEF	0x00
#define FE_RECORD_DISPLAY_ATTR_1		0x01
#define FE_RECORD_DISPLAY_ATTR_2		0x02
#define FE_RECORD_DISPLAY_ATTR_3		0x03

/* Extended Attribute Header Descriptor (ECMA 167r3 4/14.10.1) */
struct extendedAttrHeaderDesc
{
	tag		descTag;
	uint32_t	impAttrLocation;
	uint32_t	appAttrLocation;
} __attribute__ ((packed));

/* Generic Format (ECMA 167r3 4/14.10.2) */
struct genericFormat
{
	uint32_t	attrType;
	uint8_t		attrSubtype;
	uint8_t		reserved[3];
	uint32_t	attrLength;
	uint8_t		attrData[0];
} __attribute__ ((packed));

/* Character Set Information (ECMA 167r3 4/14.10.3) */
struct charSetInfo
{
	uint32_t	attrType;
	uint8_t		attrSubtype;
	uint8_t		reserved[3];
	uint32_t	attrLength;
	uint32_t	escapeSeqLength;
	uint8_t		charSetType;
	uint8_t		escapeSeq[0];
} __attribute__ ((packed));

/* Alternate Permissions (ECMA 167r3 4/14.10.4) */
struct altPerms
{
	uint32_t	attrType;
	uint8_t		attrSubtype;
	uint8_t		reserved[3];
	uint32_t	attrLength;
	uint16_t	ownerIdent;
	uint16_t	groupIdent;
	uint16_t	permission;
} __attribute__ ((packed));

/* File Times Extended Attribute (ECMA 167r3 4/14.10.5) */
struct fileTimesExtAttr
{
	uint32_t	attrType;
	uint8_t		attrSubtype;
	uint8_t		reserved[3];
	uint32_t	attrLength;
	uint32_t	dataLength;
	uint32_t	fileTimeExistence;
	uint8_t		fileTimes;
} __attribute__ ((packed));

/* FileTimeExistence (ECMA 167r3 4/14.10.5.6) */
#define FTE_CREATION			0x00000001
#define FTE_DELETION			0x00000004
#define FTE_EFFECTIVE			0x00000008
#define FTE_BACKUP				0x00000002

/* Information Times Extended Attribute (ECMA 167r3 4/14.10.6) */
struct infoTimesExtAttr
{
	uint32_t	attrType;
	uint8_t		attrSubtype;
	uint8_t		reserved[3];
	uint32_t	attrLength;
	uint32_t	dataLength;
	uint32_t	infoTimeExistence;
	uint8_t		infoTimes[0];
} __attribute__ ((packed));

/* Device Specification (ECMA 167r3 4/14.10.7) */
struct deviceSpec
{
	uint32_t	attrType;
	uint8_t		attrSubtype;
	uint8_t		reserved[3];
	uint32_t	attrLength;
	uint32_t	impUseLength;
	uint32_t	majorDeviceIdent;
	uint32_t	minorDeviceIdent;
	uint8_t		impUse[0];
} __attribute__ ((packed));

/* Implementation Use Extended Attr (ECMA 167r3 4/14.10.8) */
struct impUseExtAttr
{
	uint32_t	attrType;
	uint8_t		attrSubtype;
	uint8_t		reserved[3];
	uint32_t	attrLength;
	uint32_t	impUseLength;
	regid		impIdent;
	uint8_t		impUse[0];
} __attribute__ ((packed));

/* Application Use Extended Attribute (ECMA 167r3 4/14.10.9) */
struct appUseExtAttr
{
	uint32_t	attrType;
	uint8_t		attrSubtype;
	uint8_t		reserved[3];
	uint32_t	attrLength;
	uint32_t	appUseLength;
	regid		appIdent;
	uint8_t		appUse[0];
} __attribute__ ((packed));

#define EXTATTR_CHAR_SET		1
#define EXTATTR_ALT_PERMS		3
#define EXTATTR_FILE_TIMES		5
#define EXTATTR_INFO_TIMES		6
#define EXTATTR_DEV_SPEC		12
#define EXTATTR_IMP_USE			2048
#define EXTATTR_APP_USE			65536


/* Unallocated Space Entry (ECMA 167r3 4/14.11) */
struct unallocSpaceEntry
{
	tag		descTag;
	icbtag		icbTag;
	uint32_t	lengthAllocDescs;
	uint8_t		allocDescs[0];
} __attribute__ ((packed));

/* Space Bitmap Descriptor (ECMA 167r3 4/14.12) */
struct spaceBitmapDesc
{
	tag		descTag;
	uint32_t	numOfBits;
	uint32_t	numOfBytes;
	uint8_t		bitmap[0];
} __attribute__ ((packed));

/* Partition Integrity Entry (ECMA 167r3 4/14.13) */
struct partitionIntegrityEntry
{
	tag		descTag;
	icbtag		icbTag;
	timestamp	recordingDateAndTime;
	uint8_t		integrityType;
	uint8_t		reserved[175];
	regid		impIdent;
	uint8_t		impUse[256];
} __attribute__ ((packed));

/* Short Allocation Descriptor (ECMA 167r3 4/14.14.1) */

/* Extent Length (ECMA 167r3 4/14.14.1.1) */
#define EXT_RECORDED_ALLOCATED			0x00000000
#define EXT_NOT_RECORDED_ALLOCATED		0x40000000
#define EXT_NOT_RECORDED_NOT_ALLOCATED	0x80000000
#define EXT_NEXT_EXTENT_ALLOCDECS		0xC0000000

/* Long Allocation Descriptor (ECMA 167r3 4/14.14.2) */

/* Extended Allocation Descriptor (ECMA 167r3 4/14.14.3) */

/* Logical Volume Header Descriptor (ECMA 167r3 4/14.15) */
struct logicalVolHeaderDesc
{
	uint64_t	uniqueID;
	uint8_t		reserved[24];
} __attribute__ ((packed));

/* Path Component (ECMA 167r3 4/14.16.1) */
struct pathComponent
{
	uint8_t		componentType;
	uint8_t		lengthComponentIdent;
	uint16_t	componentFileVersionNum;
	dstring		componentIdent[0];
} __attribute__ ((packed));

/* File Entry (ECMA 167r3 4/14.17) */
struct extendedFileEntry
{
	tag		descTag;
	icbtag		icbTag;
	uint32_t	uid;
	uint32_t	gid;
	uint32_t	permissions;
	uint16_t	fileLinkCount;
	uint8_t		recordFormat;
	uint8_t		recordDisplayAttr;
	uint32_t	recordLength;
	uint64_t	informationLength;
	uint64_t	objectSize;
	uint64_t	logicalBlocksRecorded;
	timestamp	accessTime;
	timestamp	modificationTime;
	timestamp	createTime;
	timestamp	attrTime;
	uint32_t	checkpoint;
	uint32_t	reserved;
	long_ad		extendedAttrICB;
	long_ad		streamDirectoryICB;
	regid		impIdent;
	uint64_t	uniqueID;
	uint32_t	lengthExtendedAttr;
	uint32_t	lengthAllocDescs;
	uint8_t		extendedAttr[0];
	uint8_t		allocDescs[0];
} __attribute__ ((packed));

#endif /* _ECMA_167_H */
