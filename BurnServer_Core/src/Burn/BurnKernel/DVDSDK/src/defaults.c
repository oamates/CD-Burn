
#include "mkudffs.h"
#include "defaults.h"

/* 
	VDS_SIZE, LVID_SIZE, STABLE_SIZE, SSPACE_SIZE, PSPACE_SIZE 
	分配了udf文件系统各个盘区和描述占用逻辑块大小

*/
/*
struct udf_sizing default_sizing[][UDF_ALLOC_TYPE_SIZE] =
{
	{
		{	1,	0,	1,	16	},
		{	1,	0,	1,	1	},
		{	1,	0,	1,	0	},
		{	1,	0,	1,	0	},
		{	1,	0,	1,	0	},
	},
	{
		{	1,	0,	1,	255	},
		{	1,	1,	50,	10	},
		{	1,	0,	1,	0	},
		{	1,	0,	1,	0	},
		{	1,	0,	1,	0	},
	},
	{
		{	32,	0,	1,	32	},
		{	32,	0,	1,	32	},
		{	32,	0,	1,	32	},
		{	32,	0,	1,	1024	},
		{	32,	0,	1,	0	},
	},
	{
		{	1,	0,	1,	16	},
		{	1,	0,	1,	1	},
		{	1,	0,	1,	0	},
		{	1,	0,	1,	0	},
		{	1,	0,	1,	0	},
	}
};
*/

static struct primaryVolDesc default_pvd =
{
	descTag :
	{
		tagIdent : constant_cpu_to_le16(TAG_IDENT_PVD),
		descVersion : constant_cpu_to_le16(2),
		tagSerialNum : 0,
		descCRC : constant_cpu_to_le16(sizeof(struct primaryVolDesc) - sizeof(tag)),
	},
	volDescSeqNum : constant_cpu_to_le32(0),
	primaryVolDescNum : constant_cpu_to_le32(0),
	volIdent : "\x08" "LinuxUDF",
	volSeqNum : constant_cpu_to_le16(1),
	maxVolSeqNum : constant_cpu_to_le16(1),
	interchangeLvl : constant_cpu_to_le16(2),
	maxInterchangeLvl : constant_cpu_to_le16(2),
	charSetList : constant_cpu_to_le32(CS0),
	maxCharSetList : constant_cpu_to_le32(CS0),
	//volSetIdent : "\x08" "FFFFFFFFLinuxUDF",
	//volSetIdent : "FFFFFFFFLinuxUDF",
	//volSetIdent[31] : '\x08',
	memset(volSetIdent, "FFFFFFFFLinuxUDF", strlen("FFFFFFFFLinuxUDF"));
	memcpy(volSetI.dent[31], "\x08", sizeof(char));
	descCharSet :
	{
		charSetType : UDF_CHAR_SET_TYPE,
		charSetInfo : UDF_CHAR_SET_INFO,
	},
	explanatoryCharSet :
	{
		charSetType : UDF_CHAR_SET_TYPE,
		charSetInfo : UDF_CHAR_SET_INFO,
	},
	appIdent :
	{
		ident : UDF_ID_APPLICATION,
		identSuffix :
		{
			//UDF_OS_CLASS_UNIX,
			//UDF_OS_ID_LINUX,
			//UDF_OS_CLASS_WIN9X
		},
	},
	impIdent :
	{
		ident : UDF_ID_DEVELOPER,
		identSuffix :
		{
			//UDF_OS_CLASS_UNIX,
			//UDF_OS_ID_LINUX,
			//UDF_OS_CLASS_WIN9X
		},
	},
	flags : constant_cpu_to_le16(0),//PVD_FLAGS_VSID_COMMON),
};


static struct impUseVolDesc default_iuvd =
{
	descTag :
	{
		tagIdent : constant_cpu_to_le16(TAG_IDENT_IUVD),
		descVersion : constant_cpu_to_le16(2),
		tagSerialNum : 0,
		descCRC : constant_cpu_to_le16(sizeof(struct impUseVolDesc) - sizeof(tag)),
	},
	volDescSeqNum : constant_cpu_to_le32(1),
	impIdent :
	{
		ident : UDF_ID_LV_INFO,
		identSuffix :
		{
			0x02,//0x50,
			0x01,
			//UDF_OS_CLASS_UNIX,
			//UDF_OS_ID_LINUX,
			//UDF_OS_CLASS_WIN9X
		},
	},
};

static struct partitionDesc default_pd =
{
	descTag :
	{
		tagIdent : constant_cpu_to_le16(TAG_IDENT_PD),
		descVersion : constant_cpu_to_le16(2),
		tagSerialNum : 0,
		descCRC : constant_cpu_to_le16(sizeof(struct partitionDesc) - sizeof(tag)),
	},
	volDescSeqNum : constant_cpu_to_le32(2),
	partitionFlags : constant_cpu_to_le16(PD_PARTITION_FLAGS_ALLOC),
	partitionContents :
	{
		//flags : 2,
		ident : PD_PARTITION_CONTENTS_NSR02,   //udf1.02, udf1.5
	},
	accessType : constant_cpu_to_le32(PD_ACCESS_TYPE_READ_ONLY), //dvd-r
	impIdent :
	{
		ident : UDF_ID_DEVELOPER,
		identSuffix :
		{
			//UDF_OS_CLASS_UNIX,
			//UDF_OS_ID_LINUX,
			//UDF_OS_CLASS_WIN9X
		},
	},
};

static struct logicalVolDesc default_lvd =
{
	descTag :
	{
		tagIdent : constant_cpu_to_le16(TAG_IDENT_LVD),
		descVersion : constant_cpu_to_le16(2),
		tagSerialNum : 0,
		descCRC : constant_cpu_to_le16(sizeof(struct logicalVolDesc) - sizeof(tag)),
	},
	volDescSeqNum : constant_cpu_to_le32(3),
	descCharSet :
	{
		charSetType : UDF_CHAR_SET_TYPE,
		charSetInfo : UDF_CHAR_SET_INFO,
	},
	logicalVolIdent : "\x08" "xkd_UDF_1.5",
	logicalBlockSize : constant_cpu_to_le32(2048),
	domainIdent :
	{
		ident : UDF_ID_COMPLIANT,
		identSuffix :
		{
			0x50,
			0x01,
			0x00,
		}
	},
	impIdent :
	{
		ident : UDF_ID_DEVELOPER,
		identSuffix :
		{
			//UDF_OS_CLASS_UNIX,
			//UDF_OS_ID_LINUX,
			//UDF_OS_CLASS_WIN9X
		},
	}
};

static struct unallocSpaceDesc default_usd =
{
	descTag :
	{
		tagIdent : constant_cpu_to_le16(TAG_IDENT_USD),
		descVersion : constant_cpu_to_le16(2),
		tagSerialNum : 0,
		descCRC : constant_cpu_to_le16(sizeof(struct unallocSpaceDesc) - sizeof(tag)),
	},
	volDescSeqNum : constant_cpu_to_le32(4),
};



static struct terminatingDesc default_td =
{
	descTag :
	{
		tagIdent : constant_cpu_to_le16(TAG_IDENT_TD),
		descVersion : constant_cpu_to_le16(2),
		tagSerialNum : 0,
		descCRC : constant_cpu_to_le16(sizeof(struct terminatingDesc) - sizeof(tag)),
	},
};


static struct volDescPtr default_vdp =
{
	descTag :
	{
		tagIdent : constant_cpu_to_le16(TAG_IDENT_VDP),
		descVersion : constant_cpu_to_le16(2),
		tagSerialNum : 0,
		descCRC : constant_cpu_to_le16(sizeof(struct volDescPtr) - sizeof(tag)),
	},
	volDescSeqNum : constant_cpu_to_le32(3),
};

static struct impUseVolDescImpUse default_iuvdiu =
{
	LVICharset :
	{
		charSetType : UDF_CHAR_SET_TYPE,
		charSetInfo : UDF_CHAR_SET_INFO
	},
	logicalVolIdent : "\x08" "xkd_UDF_1.5",
	LVInfo1 : "\x08" "Linux mkudffs " MKUDFFS_VERSION,
	LVInfo2 : "\x08" "Linux UDF " UDFFS_VERSION " (" UDFFS_DATE ")",
	LVInfo3 : "\x08" EMAIL_STRING,
	impIdent :
	{
		ident : UDF_ID_DEVELOPER,
		identSuffix :
		{
			//UDF_OS_CLASS_UNIX,
			//UDF_OS_ID_LINUX,
			//UDF_OS_CLASS_WIN9X
		},
	},
};
	

static struct logicalVolIntegrityDesc default_lvid =
{
	descTag :
	{
		tagIdent : constant_cpu_to_le16(TAG_IDENT_LVID),
		descVersion : constant_cpu_to_le16(2),
		tagSerialNum : 0,
		descCRC : constant_cpu_to_le16(sizeof(struct logicalVolIntegrityDesc) - sizeof(tag)),
	},
	integrityType : constant_cpu_to_le32(LVID_INTEGRITY_TYPE_CLOSE),
	lengthOfImpUse : constant_cpu_to_le32(sizeof(struct logicalVolIntegrityDescImpUse)),
		
};

//默认采用UDF-1.02高度兼容版本
static struct logicalVolIntegrityDescImpUse default_lvidiu =
{
	impIdent :
	{
		ident : UDF_ID_DEVELOPER,
		identSuffix :
		{
			//UDF_OS_CLASS_UNIX,
			//UDF_OS_ID_LINUX,
			//UDF_OS_CLASS_WIN9X
		},
	},
	//minUDFReadRev : constant_cpu_to_le16(0x0001),
	//minUDFWriteRev : constant_cpu_to_le16(0x0000),
	//maxUDFWriteRev : constant_cpu_to_le16(0x0250),
	
	minUDFReadRev : constant_cpu_to_le16(0x0150),
	minUDFWriteRev : constant_cpu_to_le16(0x0150),
	maxUDFWriteRev : constant_cpu_to_le16(0x0150),
};

//对DVD-R无用
static struct sparingTable default_stable =
{
	descTag :
	{
		tagIdent : constant_cpu_to_le16(0),
		descVersion : constant_cpu_to_le16(2),
		tagSerialNum : 0,
		descCRC : constant_cpu_to_le16(sizeof(struct sparingTable) - sizeof(tag)),
	},
	sparingIdent :
	{
		flags : 0,
		ident : UDF_ID_SPARING,
		identSuffix :
		{
			//0x50,
			//0x01,
			//UDF_OS_CLASS_UNIX,
			//UDF_OS_ID_LINUX,
			//UDF_OS_CLASS_WIN9X
		},
	},
	reallocationTableLen : constant_cpu_to_le16(0),
	sequenceNum : constant_cpu_to_le32(0)
};

//对DVD-R无用
static struct sparablePartitionMap default_sparmap =
{
	partitionMapType : 2,
	partitionMapLength : sizeof(struct sparablePartitionMap),
	partIdent :
	{
		flags : 0,
		ident : UDF_ID_SPARABLE,
		identSuffix :
		{
			//0x50,
			//0x01,
			//UDF_OS_CLASS_UNIX,
			//UDF_OS_ID_LINUX,
			//UDF_OS_CLASS_WIN9X
		},
	},
	volSeqNum : constant_cpu_to_le16(1),
	packetLength : constant_cpu_to_le16(32)
};


static struct virtualAllocationTable15 default_vat15 =
{
	vatIdent :
	{
		flags : 0,
		ident : UDF_ID_ALLOC,
		identSuffix :
		{
			0x50,
			0x01,
			UDF_OS_CLASS_UNIX,
			UDF_OS_ID_LINUX,
			UDF_OS_CLASS_WIN9X
		},
	},
	previousVATICBLoc : constant_cpu_to_le32(0xFFFFFFFF)
};

static struct virtualAllocationTable20 default_vat20 =
{
	lengthHeader : constant_cpu_to_le16(136),
	lengthImpUse : constant_cpu_to_le16(0),
	logicalVolIdent : "\x08" "LinuxUDF",
	previousVATICBLoc : constant_cpu_to_le32(0xFFFFFFFF),
	minUDFReadRev : constant_cpu_to_le16(0x0150),
	minUDFWriteRev : constant_cpu_to_le16(0x0150),
	maxUDFWriteRev : constant_cpu_to_le16(0x0150)
};

static struct virtualPartitionMap default_virtmap =
{
	partitionMapType : 2,
	partitionMapLength : sizeof(struct virtualPartitionMap),
	partIdent :
	{
		flags : 0,
		ident : UDF_ID_VIRTUAL,
		identSuffix :
		{
			//0x50,
			//0x01,
			//UDF_OS_CLASS_UNIX,
			UDF_OS_ID_LINUX,
			//UDF_OS_CLASS_WIN9X
		},
	},
	volSeqNum : constant_cpu_to_le16(1)
};

static struct fileSetDesc default_fsd =
{
	descTag :
	{
		tagIdent : constant_cpu_to_le16(TAG_IDENT_FSD),
		descVersion : constant_cpu_to_le16(2),
		tagSerialNum : 0,
		descCRC : constant_cpu_to_le16(sizeof(struct fileSetDesc) - sizeof(tag)),
	},
	interchangeLvl : constant_cpu_to_le16(3),
	maxInterchangeLvl : constant_cpu_to_le16(3),
	charSetList : constant_cpu_to_le32(CS0),
	maxCharSetList : constant_cpu_to_le32(CS0),
	logicalVolIdentCharSet :
	{
		charSetType : UDF_CHAR_SET_TYPE,
		charSetInfo : UDF_CHAR_SET_INFO,
	},
	logicalVolIdent : "\x08" "xkd_UDF_1.5",
	fileSetCharSet :
	{
		charSetType : UDF_CHAR_SET_TYPE,
		charSetInfo : UDF_CHAR_SET_INFO,
	},
	fileSetIdent : "\x08" "xkd_UDF_1.5",
	copyrightFileIdent : "\x08" "xkd_Copyright",
	abstractFileIdent : "\x08" "Abstract",
	domainIdent :
	{
		ident : UDF_ID_COMPLIANT,
		identSuffix :
		{
			0x50,
			0x01,
			0x00,
		},
	}
};

static struct fileEntry default_fe =
{
	descTag :
	{
		tagIdent : constant_cpu_to_le16(TAG_IDENT_FE),
		descVersion : constant_cpu_to_le16(2),
		tagSerialNum : 0,
		descCRC : constant_cpu_to_le16(sizeof(struct fileEntry) - sizeof(tag)),
	},
	icbTag :
	{
		strategyType : constant_cpu_to_le16(4),
		strategyParameter : constant_cpu_to_le16(0),
		numEntries : constant_cpu_to_le16(1),
		fileType : ICBTAG_FILE_TYPE_REGULAR,
		flags : constant_cpu_to_le16(ICBTAG_FLAG_AD_LONG),
	},		
	permissions : constant_cpu_to_le32(FE_PERM_O_EXEC | FE_PERM_O_READ | FE_PERM_G_EXEC | FE_PERM_G_READ |	FE_PERM_U_EXEC | FE_PERM_U_READ),
	fileLinkCount : constant_cpu_to_le16(0),
	informationLength : constant_cpu_to_le64(0),
	logicalBlocksRecorded : constant_cpu_to_le64(0),
	impIdent :
	{
		ident : UDF_ID_DEVELOPER,
		identSuffix :
		{
			//UDF_OS_CLASS_UNIX,
			//UDF_OS_ID_LINUX,
			//UDF_OS_CLASS_WIN9X
		},
	},
};

static struct extendedFileEntry default_efe =
{
	descTag :
	{
		tagIdent : constant_cpu_to_le16(TAG_IDENT_EFE),
		descVersion : constant_cpu_to_le16(2),
		tagSerialNum : 0,
		descCRC : constant_cpu_to_le16(sizeof(struct extendedFileEntry) - sizeof(tag)),
	},
	icbTag :
	{
		strategyType : constant_cpu_to_le16(4),
		strategyParameter : constant_cpu_to_le16(0),
		numEntries : constant_cpu_to_le16(1),
		fileType : 0,
		flags : constant_cpu_to_le16(ICBTAG_FLAG_AD_IN_ICB),
	},
	permissions : constant_cpu_to_le32(FE_PERM_U_DELETE|FE_PERM_U_CHATTR|FE_PERM_U_READ|FE_PERM_U_WRITE|FE_PERM_U_EXEC|FE_PERM_G_READ|FE_PERM_G_EXEC|FE_PERM_O_READ|FE_PERM_O_EXEC),
	fileLinkCount : constant_cpu_to_le16(0),
	informationLength : constant_cpu_to_le64(0),
	objectSize : constant_cpu_to_le64(0),
	logicalBlocksRecorded : constant_cpu_to_le64(0),
	impIdent :
	{
		ident : UDF_ID_DEVELOPER,
		identSuffix :
		{
			//UDF_OS_CLASS_UNIX,
			UDF_OS_ID_LINUX,
			//UDF_OS_CLASS_WIN9X
		},
	},
};

LVUDF_DEFAULTSINFO_T LvUDF_Defaults = {
	.default_pvd     = &default_pvd,
	.default_lvd     = &default_lvd,
	.default_vdp     = &default_vdp,
	.default_iuvdiu  = &default_iuvdiu,
	.default_iuvd    = &default_iuvd,
	.default_pd      = &default_pd,
	.default_usd     = &default_usd,
	.default_td      = &default_td,
	.default_lvid    = &default_lvid,
	.default_lvidiu  = &default_lvidiu,
	.default_stable  = &default_stable,
	.default_sparmap = &default_sparmap,
	.default_vat15   = &default_vat15,
	.default_vat20   = &default_vat20,
	.default_virtmap = &default_virtmap,
	.default_fsd     = &default_fsd,
	.default_fe      = &default_fe,
	.default_efe     = &default_efe,
};
