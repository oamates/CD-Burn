
#include "defaults.h"
#include "mkudffs.h"

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

PrimaryVolDesc::PrimaryVolDesc()
{
	default_pvd.descTag.tagIdent = constant_cpu_to_le16(TAG_IDENT_PVD);
	default_pvd.descTag.descVersion = constant_cpu_to_le16(2);
	default_pvd.descTag.tagSerialNum = 0;
	default_pvd.descTag.descCRC = constant_cpu_to_le16(sizeof(struct primaryVolDesc) - sizeof(tag));

	default_pvd.volDescSeqNum = constant_cpu_to_le32(0);
	default_pvd.primaryVolDescNum = constant_cpu_to_le32(0);
	memset(default_pvd.volIdent, 0, sizeof(default_pvd.volIdent));
	memcpy(default_pvd.volIdent, "\x08", sizeof(unsigned char));
	strcpy((char*)(default_pvd.volIdent+1), "LinuxUDF");
	default_pvd.volSeqNum = constant_cpu_to_le16(1);
	default_pvd.maxVolSeqNum = constant_cpu_to_le16(1);
	default_pvd.interchangeLvl = constant_cpu_to_le16(2);
	default_pvd.maxInterchangeLvl = constant_cpu_to_le16(2);
	default_pvd.charSetList = constant_cpu_to_le32(CS0);
	default_pvd.maxCharSetList = constant_cpu_to_le32(CS0);
	memset(default_pvd.volSetIdent, 0, sizeof(default_pvd.volSetIdent));
	memcpy(default_pvd.volIdent, "\x08", sizeof(unsigned char));
	strcpy((char*)(default_pvd.volIdent+1), "FFFFFFFFLinuxUDF");
	default_pvd.descCharSet.charSetType = UDF_CHAR_SET_TYPE;
	
	memset(default_pvd.descCharSet.charSetInfo, 0, sizeof(default_pvd.descCharSet.charSetInfo));
	memcpy(default_pvd.descCharSet.charSetInfo, UDF_CHAR_SET_INFO, strlen(UDF_CHAR_SET_INFO));
	
	default_pvd.explanatoryCharSet.charSetType = UDF_CHAR_SET_TYPE;
	memset(default_pvd.explanatoryCharSet.charSetInfo, 0, sizeof(default_pvd.explanatoryCharSet.charSetInfo));
	memcpy(default_pvd.explanatoryCharSet.charSetInfo, UDF_CHAR_SET_INFO, strlen(UDF_CHAR_SET_INFO));

	memset(default_pvd.appIdent.ident, 0, sizeof(default_pvd.appIdent.ident));
	strcpy((char*)(default_pvd.appIdent.ident), UDF_ID_APPLICATION);
	memset(default_pvd.appIdent.identSuffix, 0, sizeof(default_pvd.appIdent.identSuffix));
	
	memset(default_pvd.impIdent.ident, 0, sizeof(default_pvd.impIdent.ident));
	strcpy((char*)(default_pvd.impIdent.ident), UDF_ID_DEVELOPER);
	memset(default_pvd.impIdent.identSuffix, 0, sizeof(default_pvd.impIdent.identSuffix));

	default_pvd.flags = constant_cpu_to_le16(0);//PVD_FLAGS_VSID_COMMON),
}

PrimaryVolDesc::~PrimaryVolDesc()
{}

static PrimaryVolDesc default_pvd;

#if 0
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
#endif

class ImpUseVolDesc
{
public:
	ImpUseVolDesc();
	~ImpUseVolDesc();
public:
	impUseVolDesc default_iuvd;
};

ImpUseVolDesc::ImpUseVolDesc()
{
	default_iuvd.descTag.tagIdent = constant_cpu_to_le16(TAG_IDENT_IUVD);
	default_iuvd.descTag.descVersion = constant_cpu_to_le16(2);
	default_iuvd.descTag.tagSerialNum = 0;
	default_iuvd.descTag.descCRC = constant_cpu_to_le16(sizeof(struct impUseVolDesc) - sizeof(tag));
	default_iuvd.volDescSeqNum = constant_cpu_to_le32(1);
	memset(default_iuvd.impIdent.ident, 0, sizeof(default_iuvd.impIdent.ident));
	strcpy((char*)(default_iuvd.impIdent.ident), UDF_ID_LV_INFO);
	default_iuvd.impIdent.identSuffix[0] = 0x02;//0x50,
	default_iuvd.impIdent.identSuffix[1] = 0x01;
}

ImpUseVolDesc::~ImpUseVolDesc()
{}

static ImpUseVolDesc default_iuvd;

#if 0
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
#endif

class PartitionDesc
{
public:
	PartitionDesc();
	~PartitionDesc();
public:
	partitionDesc default_pd;
};

PartitionDesc::PartitionDesc()
{
	default_pd.descTag.tagIdent = constant_cpu_to_le16(TAG_IDENT_PD);
	default_pd.descTag.descVersion = constant_cpu_to_le16(2);
	default_pd.descTag.tagSerialNum = 0;
	default_pd.descTag.descCRC = constant_cpu_to_le16(sizeof(struct partitionDesc) - sizeof(tag));
	
	default_pd.volDescSeqNum = constant_cpu_to_le32(2);
	default_pd.partitionFlags = constant_cpu_to_le16(PD_PARTITION_FLAGS_ALLOC);
	memset((void*)&default_pd.partitionContents, 0, sizeof(default_pd.partitionContents));
	strcpy((char*)default_pd.partitionContents.ident, PD_PARTITION_CONTENTS_NSR02);   //udf1.02, udf1.5
	default_pd.accessType = constant_cpu_to_le32(PD_ACCESS_TYPE_READ_ONLY); //dvd-r
	
	memset(default_pd.impIdent.ident, 0, sizeof(default_pd.impIdent.ident));
	strcpy((char*)(default_pd.impIdent.ident), UDF_ID_DEVELOPER);
	memset(default_pd.impIdent.identSuffix, 0, sizeof(default_pd.impIdent.identSuffix));
}

PartitionDesc::~PartitionDesc(){};

static PartitionDesc default_pd;

#if 0
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
#endif

class LogicalVolDesc
{
public:
	LogicalVolDesc();
	~LogicalVolDesc();

public:
	logicalVolDesc default_lvd;
};

LogicalVolDesc::LogicalVolDesc()
{
	default_lvd.descTag.tagIdent = constant_cpu_to_le16(TAG_IDENT_LVD);
	default_lvd.descTag.descVersion = constant_cpu_to_le16(2);
	default_lvd.descTag.tagSerialNum = 0;
	default_lvd.descTag.descCRC = constant_cpu_to_le16(sizeof(struct logicalVolDesc) - sizeof(tag));
	default_lvd.volDescSeqNum = constant_cpu_to_le32(3);
	default_lvd.descCharSet.charSetType = UDF_CHAR_SET_TYPE;
	memset((void*)default_lvd.descCharSet.charSetInfo, 0, sizeof(default_lvd.descCharSet.charSetInfo));
	strcpy((char*)default_lvd.descCharSet.charSetInfo , UDF_CHAR_SET_INFO);

	memset(default_lvd.logicalVolIdent, 0, sizeof(default_lvd.logicalVolIdent));
	default_lvd.logicalVolIdent[0] = '\x08';
	strcpy((char*)(default_lvd.logicalVolIdent + 1), "UDF_1.5");
	
	default_lvd.logicalBlockSize = constant_cpu_to_le32(2048);
	
	
	memset(default_lvd.domainIdent.ident, 0, sizeof(default_lvd.domainIdent.ident));
	strcpy((char*)(default_lvd.domainIdent.ident), UDF_ID_COMPLIANT);

	memset(default_lvd.domainIdent.identSuffix, 0, sizeof(default_lvd.domainIdent.identSuffix));
	default_lvd.domainIdent.identSuffix[0] = 0x50;
	default_lvd.domainIdent.identSuffix[1] = 0x01;
	default_lvd.domainIdent.identSuffix[2] = 0x00;
	memset((void*)default_lvd.impIdent.ident, 0, sizeof(default_lvd.impIdent.ident));
	strcpy((char*)default_lvd.impIdent.ident, UDF_ID_DEVELOPER);
	memset((void*)(default_lvd.impIdent.identSuffix), 0, sizeof(default_lvd.impIdent.identSuffix));
}

LogicalVolDesc::~LogicalVolDesc()
{}

static LogicalVolDesc default_lvd;

#if 0
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
#endif

class UnallocSpaceDesc
{
public:
	UnallocSpaceDesc();
	~UnallocSpaceDesc(){};

public:
	unallocSpaceDesc default_usd;
};

UnallocSpaceDesc::UnallocSpaceDesc()
{
	default_usd.descTag.tagIdent = constant_cpu_to_le16(TAG_IDENT_USD);
	default_usd.descTag.descVersion = constant_cpu_to_le16(TAG_IDENT_USD);
	default_usd.descTag.tagSerialNum = 0;
	default_usd.descTag.descCRC = constant_cpu_to_le16(sizeof(struct unallocSpaceDesc) - sizeof(tag));
	default_usd.volDescSeqNum = constant_cpu_to_le32(4);
}

static UnallocSpaceDesc default_usd;


#if 0
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
#endif

class TerminatingDesc
{
public:
	TerminatingDesc();
	~TerminatingDesc(){};
public:
	terminatingDesc default_td;
};

TerminatingDesc::TerminatingDesc()
{
	default_td.descTag.tagIdent = constant_cpu_to_le16(TAG_IDENT_TD);
	default_td.descTag.descVersion = constant_cpu_to_le16(2);
	default_td.descTag.tagSerialNum = 0;
	default_td.descTag.descCRC = constant_cpu_to_le16(sizeof(struct terminatingDesc) - sizeof(tag));
}

static TerminatingDesc default_td;

#if 0
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
#endif 

class VolDescPtr
{
public:
	VolDescPtr();
	~VolDescPtr(){};

public:
	volDescPtr default_vdp;
};

VolDescPtr::VolDescPtr()
{
	default_vdp.descTag.tagIdent = constant_cpu_to_le16(TAG_IDENT_VDP);
	default_vdp.descTag.descVersion = constant_cpu_to_le16(2);
	default_vdp.descTag.tagSerialNum = 0;
	default_vdp.descTag.descCRC = constant_cpu_to_le16(sizeof(struct volDescPtr) - sizeof(tag));
	default_vdp.volDescSeqNum = constant_cpu_to_le32(3);
}

static VolDescPtr default_vdp;

#if 0
static struct impUseVolDescImpUse default_iuvdiu =
{
	LVICharset :
	{
		charSetType : UDF_CHAR_SET_TYPE,
		charSetInfo : UDF_CHAR_SET_INFO
	},
	logicalVolIdent : "\x08  xkd_UDF_1.5",
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
#endif

class ImpUseVolDescImpUse
{
public:
	ImpUseVolDescImpUse();
	~ImpUseVolDescImpUse(){};
public:
	impUseVolDescImpUse default_iuvdiu;
};

ImpUseVolDescImpUse::ImpUseVolDescImpUse()
{
	int nStrLen = 0;
	default_iuvdiu.LVICharset.charSetType = UDF_CHAR_SET_TYPE;
	memset((void*)default_iuvdiu.LVICharset.charSetInfo, 0, sizeof(default_iuvdiu.LVICharset.charSetInfo));
	strcpy((char*)default_iuvdiu.LVICharset.charSetInfo, UDF_CHAR_SET_INFO);
	
	memset(default_iuvdiu.logicalVolIdent, 0, sizeof(default_iuvdiu.logicalVolIdent));
	default_iuvdiu.logicalVolIdent[0] = '\x08';
	strcpy((char*)(default_iuvdiu.logicalVolIdent+1), "UDF_1.5");
	
	memset((void*)default_iuvdiu.LVInfo1, 0, sizeof(default_iuvdiu.LVInfo1));
	default_iuvdiu.LVInfo1[0] = '\x08';
	strcpy((char*)(default_iuvdiu.LVInfo1 + 1), "Linux mkudffs ");
	nStrLen = strlen("Linux mkudffs ");
	strcpy((char*)(default_iuvdiu.LVInfo1 + 1 + nStrLen), MKUDFFS_VERSION);
	
	memset((void*)default_iuvdiu.LVInfo2, 0, sizeof(default_iuvdiu.LVInfo2));
	default_iuvdiu.LVInfo2[0] = '\x08';
	strcpy((char*)(default_iuvdiu.LVInfo2 + 1), "Linux UDF ");
	nStrLen = strlen("Linux UDF ");
	strcpy((char*)(default_iuvdiu.LVInfo2 + 1 + nStrLen), UDFFS_VERSION);
	nStrLen += strlen(UDFFS_VERSION);
	strcpy((char*)(default_iuvdiu.LVInfo2 + 1 + nStrLen), " (");
	nStrLen += strlen(" (");
	strcpy((char*)(default_iuvdiu.LVInfo2 + 1 + nStrLen), ")");

	memset((void*)default_iuvdiu.LVInfo3, 0, sizeof(default_iuvdiu.LVInfo3));
	default_iuvdiu.LVInfo3[0] = '\x08';
	strcpy((char*)(default_iuvdiu.LVInfo3 + 1), EMAIL_STRING);
	
	memset((void*)default_iuvdiu.impIdent.ident, 0, sizeof(default_iuvdiu.impIdent.ident));
	strcpy((char*)default_iuvdiu.impIdent.ident, UDF_ID_DEVELOPER);

	memset((void*)default_iuvdiu.impIdent.identSuffix, 0, sizeof(default_iuvdiu.impIdent.identSuffix));
}

static ImpUseVolDescImpUse default_iuvdiu;

#if 0
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
#endif 

class LogicalVolIntegrityDesc
{
public:
	LogicalVolIntegrityDesc();
	~LogicalVolIntegrityDesc(){};
public:
	logicalVolIntegrityDesc default_lvid;
};

LogicalVolIntegrityDesc::LogicalVolIntegrityDesc()
{
	default_lvid.descTag.tagIdent = constant_cpu_to_le16(TAG_IDENT_LVID);
	default_lvid.descTag.descVersion = constant_cpu_to_le16(2);
	default_lvid.descTag.tagSerialNum = 0;
	default_lvid.descTag.descCRC = constant_cpu_to_le16(sizeof(struct logicalVolIntegrityDesc) - sizeof(tag));
	
	default_lvid.integrityType = constant_cpu_to_le32(LVID_INTEGRITY_TYPE_CLOSE);
	default_lvid.lengthOfImpUse = constant_cpu_to_le32(sizeof(struct logicalVolIntegrityDescImpUse));
}

static LogicalVolIntegrityDesc default_lvid;


#if 0
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
#endif

// class LogicalVolIntegrityDescImpUse
// {
// public:
// 	LogicalVolIntegrityDescImpUse();
// 	~LogicalVolIntegrityDescImpUse(){};
// public:
// 	logicalVolIntegrityDescImpUse default_lvidiu;
// };

LogicalVolIntegrityDescImpUse::LogicalVolIntegrityDescImpUse()
{
	memset((void*)default_lvidiu.impIdent.ident, 0, sizeof(default_lvidiu.impIdent.ident));
	strcpy((char*)default_lvidiu.impIdent.ident, UDF_ID_DEVELOPER);
	memset((void*)default_lvidiu.impIdent.identSuffix, 0, sizeof(default_lvidiu.impIdent.identSuffix));
	
	default_lvidiu.minUDFReadRev = constant_cpu_to_le16(0x0150);
	default_lvidiu.minUDFWriteRev = constant_cpu_to_le16(0x0150);
	default_lvidiu.maxUDFWriteRev = constant_cpu_to_le16(0x0150);
}

static LogicalVolIntegrityDescImpUse default_lvidiu;

#if 0
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
#endif

//对DVD-R无用
class SparingTable
{
public:
	SparingTable();
	~SparingTable(){};

public:
	sparingTable default_stable;
};

SparingTable::SparingTable()
{
	default_stable.descTag.tagIdent = constant_cpu_to_le16(0);
	default_stable.descTag.descVersion = constant_cpu_to_le16(2);
	default_stable.descTag.tagSerialNum = 0;
	default_stable.descTag.descCRC = constant_cpu_to_le16(sizeof(struct sparingTable) - sizeof(tag));

	default_stable.sparingIdent.flags = 0;

	memset((void*)default_stable.sparingIdent.ident, 0, sizeof(default_stable.sparingIdent.ident));
	strcpy((char*)default_stable.sparingIdent.ident, UDF_ID_SPARING);
	memset((void*)default_stable.sparingIdent.identSuffix, 0, sizeof(default_stable.sparingIdent));

	default_stable.reallocationTableLen = constant_cpu_to_le16(0);
	default_stable.sequenceNum = constant_cpu_to_le32(0);
}

static SparingTable default_stable;

#if 0
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
#endif

class SparablePartitionMap
{
public:
	SparablePartitionMap();
	~SparablePartitionMap(){};
public:
	sparablePartitionMap default_sparmap;
};

SparablePartitionMap::SparablePartitionMap()
{
	default_sparmap.partitionMapType = 2;
	default_sparmap.partitionMapLength = sizeof(struct sparablePartitionMap);
	default_sparmap.partIdent.flags = 0;
	memset((void*)default_sparmap.partIdent.ident, 0, sizeof(default_sparmap.partIdent.ident));
	strcpy((char*)default_sparmap.partIdent.ident, UDF_ID_SPARABLE);
	memset((void*)default_sparmap.partIdent.identSuffix, 0, sizeof(default_sparmap.partIdent.identSuffix));
	default_sparmap.volSeqNum = constant_cpu_to_le16(1);
	default_sparmap.packetLength = constant_cpu_to_le16(32);
}

static SparablePartitionMap default_sparmap;

#if 0
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
#endif

class VirtualAllocationTable15
{
public:
	VirtualAllocationTable15();
	~VirtualAllocationTable15(){};
public:
	virtualAllocationTable15 default_vat15;
};

VirtualAllocationTable15::VirtualAllocationTable15()
{
	default_vat15.vatIdent.flags = 0;
	memset((void*)default_vat15.vatIdent.ident, 0, sizeof(default_vat15.vatIdent.ident));
	strcpy((char*)default_vat15.vatIdent.ident, UDF_ID_ALLOC);
	
	memset((void*)default_vat15.vatIdent.identSuffix, 0, sizeof(default_vat15.vatIdent.identSuffix));
	default_vat15.vatIdent.identSuffix[0] = 0x50;
	default_vat15.vatIdent.identSuffix[0] = 0x01;
	default_vat15.vatIdent.identSuffix[0] = UDF_OS_CLASS_UNIX;
	default_vat15.vatIdent.identSuffix[0] = UDF_OS_ID_LINUX;
	default_vat15.vatIdent.identSuffix[0] = UDF_OS_CLASS_WIN9X;

	default_vat15.previousVATICBLoc = constant_cpu_to_le32(0xFFFFFFFF);
}

static VirtualAllocationTable15 default_vat15;

#if 0
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
#endif

class VirtualAllocationTable20
{
public:
	VirtualAllocationTable20();
	~VirtualAllocationTable20(){};
public:
	virtualAllocationTable20 default_vat20;
};

VirtualAllocationTable20::VirtualAllocationTable20()
{
	default_vat20.lengthHeader = constant_cpu_to_le16(136);
	default_vat20.lengthImpUse = constant_cpu_to_le16(0);
	memset((void*)default_vat20.logicalVolIdent, 0, sizeof(default_vat20.logicalVolIdent));
	default_vat20.logicalVolIdent[0] = '\x08';
	strcpy((char*)(default_vat20.logicalVolIdent + 1), "LinuxUDF");
	default_vat20.previousVATICBLoc = constant_cpu_to_le32(0xFFFFFFFF);
	default_vat20.minUDFReadRev = constant_cpu_to_le16(0x0150);
	default_vat20.minUDFWriteRev = constant_cpu_to_le16(0x0150);
	default_vat20.maxUDFWriteRev = constant_cpu_to_le16(0x0150);
}

static VirtualAllocationTable20 default_vat20;

#if 0
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
#endif

class VirtualPartitionMap
{
public:
	VirtualPartitionMap();
	~VirtualPartitionMap(){};
public:
	virtualPartitionMap default_virtmap;
};

VirtualPartitionMap::VirtualPartitionMap()
{
	default_virtmap.partitionMapType = 2;
	default_virtmap.partitionMapLength = sizeof(struct virtualPartitionMap);
	default_virtmap.partIdent.flags = 0;
	memset((void*)default_virtmap.partIdent.ident, 0, sizeof(default_virtmap.partIdent.ident));
	strcpy((char*)default_virtmap.partIdent.ident, UDF_ID_VIRTUAL);
	memset((void*)default_virtmap.partIdent.identSuffix, 0, sizeof(default_virtmap.partIdent.identSuffix));
	default_virtmap.partIdent.identSuffix[0] = UDF_OS_ID_LINUX;	
	default_virtmap.volSeqNum = constant_cpu_to_le16(1);
}

static VirtualPartitionMap default_virtmap;

#if 0
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
#endif

class  FileSetDesc
{
public:
	FileSetDesc();
	~FileSetDesc(){};

public:
	fileSetDesc default_fsd;
};

FileSetDesc::FileSetDesc()
{
	default_fsd.descTag.tagIdent = constant_cpu_to_le16(TAG_IDENT_FSD);
	default_fsd.descTag.descVersion = constant_cpu_to_le16(2);
	default_fsd.descTag.tagSerialNum = 0;
	default_fsd.descTag.descCRC = constant_cpu_to_le16(sizeof(struct fileSetDesc) - sizeof(tag));

	default_fsd.interchangeLvl = constant_cpu_to_le16(3);
	default_fsd.maxInterchangeLvl = constant_cpu_to_le16(3);
	default_fsd.charSetList = constant_cpu_to_le32(CS0);
	default_fsd.maxCharSetList = constant_cpu_to_le32(CS0);

	default_fsd.logicalVolIdentCharSet.charSetType = UDF_CHAR_SET_TYPE;
	memset((void*)default_fsd.logicalVolIdentCharSet.charSetInfo, 0, sizeof(default_fsd.logicalVolIdentCharSet.charSetInfo));
	strcpy((char*)default_fsd.logicalVolIdentCharSet.charSetInfo, UDF_CHAR_SET_INFO);

	memset(default_fsd.logicalVolIdent, 0, sizeof(default_fsd.logicalVolIdent));
	default_fsd.logicalVolIdent[0] = '\x08';
	strcpy((char*)(default_fsd.logicalVolIdent + 1), "UDF_1.5");

	default_fsd.fileSetCharSet.charSetType = UDF_CHAR_SET_TYPE;
	memset((void*)default_fsd.fileSetCharSet.charSetInfo, 0, sizeof(default_fsd.fileSetCharSet.charSetInfo));
	strcpy((char*)default_fsd.fileSetCharSet.charSetInfo, UDF_CHAR_SET_INFO);

	memset((void*)default_fsd.fileSetIdent, 0, sizeof(default_fsd.fileSetIdent));
	default_fsd.fileSetIdent[0] = '\x08';
	strcpy((char*)(default_fsd.fileSetIdent + 1), "UDF_1.5");

	memset((void*)default_fsd.copyrightFileIdent, 0, sizeof(default_fsd.copyrightFileIdent));
	default_fsd.copyrightFileIdent[0] = '\x08';
	strcpy((char*)(default_fsd.copyrightFileIdent + 1), "Copyright");

	memset((void*)default_fsd.abstractFileIdent, 0, sizeof(default_fsd.abstractFileIdent));
	default_fsd.abstractFileIdent[0] = '\x08';
	strcpy((char*)(default_fsd.abstractFileIdent + 1), "Abstract");

	memset((void*)default_fsd.domainIdent.ident, 0, sizeof(default_fsd.domainIdent.ident));
	strcpy((char*)(default_fsd.domainIdent.ident), UDF_ID_COMPLIANT);

	memset((void*)default_fsd.domainIdent.identSuffix, 0, sizeof(default_fsd.domainIdent.identSuffix));

	default_fsd.domainIdent.identSuffix[0] = 0x50;
	default_fsd.domainIdent.identSuffix[0] = 0x01;
	default_fsd.domainIdent.identSuffix[0] = 0x00;
}

static FileSetDesc default_fsd;

#if 0
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
#endif

FileEntry::FileEntry()
{
	default_fe.descTag.tagIdent = constant_cpu_to_le16(TAG_IDENT_FE);
	default_fe.descTag.descVersion = constant_cpu_to_le16(2);
	default_fe.descTag.tagSerialNum = 0;
	default_fe.descTag.descCRC = constant_cpu_to_le16(sizeof(struct fileEntry) - sizeof(tag));
	
	default_fe.icbTag.strategyType = constant_cpu_to_le16(4);
	default_fe.icbTag.strategyParameter = constant_cpu_to_le16(0);
	default_fe.icbTag.numEntries = constant_cpu_to_le16(1);
	default_fe.icbTag.fileType = ICBTAG_FILE_TYPE_REGULAR;
	default_fe.icbTag.flags = constant_cpu_to_le16(ICBTAG_FLAG_AD_LONG);

	default_fe.permissions = constant_cpu_to_le32(FE_PERM_O_EXEC | FE_PERM_O_READ | FE_PERM_G_EXEC | FE_PERM_G_READ | FE_PERM_U_EXEC | FE_PERM_U_READ);
	default_fe.fileLinkCount = constant_cpu_to_le16(0);
	default_fe.informationLength = constant_cpu_to_le64(0);
	default_fe.logicalBlocksRecorded = constant_cpu_to_le64(0);
	
	memset((void*)default_fe.impIdent.ident, 0, sizeof(default_fe.impIdent.ident));
	strcpy((char*)default_fe.impIdent.ident, UDF_ID_DEVELOPER);
	
	memset((void*)default_fe.impIdent.identSuffix, 0, sizeof(default_fe.impIdent.identSuffix));
}

static FileEntry default_fe;

#if 0
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
#endif

class ExtendedFileEntry
{
public:
	ExtendedFileEntry();
	~ExtendedFileEntry(){};
public:
	extendedFileEntry default_efe;
};

ExtendedFileEntry::ExtendedFileEntry()
{
	default_efe.descTag.tagIdent = constant_cpu_to_le16(TAG_IDENT_EFE);
	default_efe.descTag.descVersion = constant_cpu_to_le16(2);
	default_efe.descTag.tagSerialNum = 0;
	default_efe.descTag.descCRC = constant_cpu_to_le16(sizeof(struct extendedFileEntry) - sizeof(tag));

	default_efe.icbTag.strategyType = constant_cpu_to_le16(4);
	default_efe.icbTag.strategyParameter = constant_cpu_to_le16(0);
	default_efe.icbTag.numEntries = constant_cpu_to_le16(1);
	default_efe.icbTag.fileType = 0;
	default_efe.icbTag.flags = constant_cpu_to_le16(ICBTAG_FLAG_AD_IN_ICB);

	default_efe.permissions = constant_cpu_to_le32(FE_PERM_U_DELETE | FE_PERM_U_CHATTR | FE_PERM_U_READ | FE_PERM_U_WRITE | FE_PERM_U_EXEC | FE_PERM_G_READ | FE_PERM_G_EXEC | FE_PERM_O_READ | FE_PERM_O_EXEC);
	default_efe.fileLinkCount = constant_cpu_to_le16(0);
	default_efe.informationLength = constant_cpu_to_le64(0);
	default_efe.objectSize = constant_cpu_to_le64(0);
	default_efe.logicalBlocksRecorded = constant_cpu_to_le64(0);

	memset((void*)default_efe.impIdent.ident, 0, sizeof(default_efe.impIdent.ident));
	strcpy((char*)default_efe.impIdent.ident, UDF_ID_DEVELOPER);

	memset((void*)default_efe.impIdent.identSuffix, 0, sizeof(default_efe.impIdent.identSuffix));
	default_efe.impIdent.identSuffix[0] = UDF_OS_ID_LINUX;
}

static ExtendedFileEntry default_efe;

UDF_DefaultInfo::UDF_DefaultInfo()
{
	pDefault_pvd = NULL;
	pDefault_lvd = NULL;
	pDefault_vdp = NULL;
	pDefault_iuvdiu = NULL;
	pDefault_iuvd = NULL;
	pDefault_pd = NULL;
	pDefault_usd = NULL;
	pDefault_td = NULL;
	pDefault_lvid = NULL;
	pDefault_lvidiu = NULL;
	pDefault_stable = NULL;
	pDefault_sparmap = NULL;
	pDefault_vat15 = NULL;
	pDefault_vat20 = NULL;
	pDefault_virtmap = NULL;
	pDefault_fsd = NULL;
	pDefault_fe = NULL;
	pDefault_efe = NULL;
}

UDF_DefaultInfo::UDF_DefaultInfo(PrimaryVolDesc* pPVD,
	LogicalVolDesc* pLVD, VolDescPtr* pVDP,
	ImpUseVolDescImpUse* pIUVDIU, ImpUseVolDesc* pIUVD,
	PartitionDesc* pPD, UnallocSpaceDesc* pUSD,
	TerminatingDesc* pTD, LogicalVolIntegrityDesc* pLVID,
	LogicalVolIntegrityDescImpUse* pLVIDIU, SparingTable* pST,
	SparablePartitionMap* pSPM, VirtualAllocationTable15* pVAT15,
	VirtualAllocationTable20* pVAT20, VirtualPartitionMap* pVPM,
	FileSetDesc* pFSD, FileEntry* pFE, ExtendedFileEntry* pEFE)
{
	pDefault_pvd = pPVD;
	pDefault_lvd = pLVD;
	pDefault_vdp = pVDP;
	pDefault_iuvdiu = pIUVDIU;
	pDefault_iuvd = pIUVD;
	pDefault_pd = pPD;
	pDefault_usd = pUSD;
	pDefault_td = pTD;
	pDefault_lvid = pLVID;
	pDefault_lvidiu = pLVIDIU;
	pDefault_stable = pST;
	pDefault_sparmap = pSPM;
	pDefault_vat15 = pVAT15;
	pDefault_vat20 = pVAT20;
	pDefault_virtmap = pVPM;
	pDefault_fsd = pFSD;
	pDefault_fe = pFE;
	pDefault_efe = pEFE;
}

UDF_DefaultInfo UDF_Defaults(&default_pvd, &default_lvd, &default_vdp,
	&default_iuvdiu, &default_iuvd, &default_pd, &default_usd, &default_td,
	&default_lvid, &default_lvidiu, &default_stable, &default_sparmap, &default_vat15,
	&default_vat20, &default_virtmap, &default_fsd, &default_fe, &default_efe
	);