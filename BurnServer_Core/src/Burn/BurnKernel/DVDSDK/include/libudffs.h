#ifndef __LIBUDFFS_H__
#define __LIBUDFFS_H__

#include "ecma_167.h"
#include "osta_udf.h"
#include "udf_endian.h"

#define FLAG_FREED_BITMAP		0x00000001
#define FLAG_FREED_TABLE		0x00000002
#define FLAG_UNALLOC_BITMAP		0x00000004
#define FLAG_UNALLOC_TABLE		0x00000008
#define FLAG_SPACE_BITMAP		(FLAG_FREED_BITMAP | FLAG_UNALLOC_BITMAP)
#define FLAG_SPACE_TABLE		(FLAG_FREED_TABLE  | FLAG_UNALLOC_TABLE)
#define FLAG_SPACE			    (FLAG_SPACE_BITMAP | FLAG_SPACE_TABLE)

//EFE = Extended File Entry

#define FLAG_EFE			0x00000010   

#define FLAG_UNICODE8		0x00000020
#define FLAG_UNICODE16		0x00000040
#define FLAG_UTF8			0x00000080
#define FLAG_CHARSET		(FLAG_UNICODE8 | FLAG_UNICODE16 | FLAG_UTF8)

#define FLAG_STRATEGY4096	0x00000100
#define FLAG_BLANK_TERMINAL	0x00000200

#define FLAG_BRIDGE			0x00000400
#define FLAG_CLOSED			0x00000800
#define FLAG_VAT			0x00001000

#define DEFAULE_UNIQUEID    21

struct udf_extent;
struct udf_desc;
struct udf_data;

//udf光盘空间类型
enum udf_space_type
{
	RESERVED	= 0x0001,	/* Reserved Space */
	VRS			= 0x0002,	/* Volume Recognition Sequence */
	ANCHOR		= 0x0004,	/* Anchor */
	PVDS		= 0x0008,	/* Primary Volume Descriptor Sequence */
	RVDS		= 0x0010,	/* Reserved Volume Descriptor Sequence */
	LVID		= 0x0020,	/* Logical Volume Identifier */
	FETMP		= 0x0040,	/* 临时文件区 */
	FSD  		= 0x0080,	/* 文件描述集，文件实体，IFO文件 */
	PSPACE		= 0x0100,	/* Partition Space */
	USPACE		= 0x0200,	/* Unallocated Space */
	IFO			= 0x0400,	/* IFO 数据文件区*/
	VOB			= 0x0800,	/* VOB 数据文件区*/	
	UDF_SPACE_TYPE_SIZE = 12,
};

//udf对齐
struct udf_sizing
{
	uint32_t	align;
	uint32_t	numSize;
	uint32_t	denomSize;
	uint32_t	minSize;
};

//udf分配类型
enum udf_alloc_type
{
	VDS_SIZE,           //卷标描述序列
	LVID_SIZE,          //逻辑卷标完整性描述
	STABLE_SIZE,        
	SSPACE_SIZE,
	PSPACE_SIZE,
	UDF_ALLOC_TYPE_SIZE,
};

//文件类型
typedef enum
{
	DVDVIDEO_VMGIFO = 0, //视频管理信息文件
	DVDVIDEO_VTSIFO = 1, //视频序列信息文件
	DVDVIDEO_BUPIFO = 2, //备份信息
	DVDVIDEO_VOB 	= 3, //视频数据
	DVDVIDEO_DATA   = 4,  //纯数据文件
	DVDVIDEO_LAST
}UdfDVDFileType;


typedef struct DVDfileInfo
{
	char * fileName[256];           //文件名
	UdfDVDFileType 	FileType;       //是否IFO配置文件，不是则为VOB文件
	struct udf_extent * ext;        //盘区
	struct udf_desc * Parentdesc;   //父目录描述符
	struct udf_desc * desc;         //描述符
	struct fileEntry * fe;    		//文件描述符指针	
	uint64_t fileSize;        		//文件大小
	uint64_t start;                 //起始扇区，相对PD而言
	uint8_t *Data;                  //文件数据	
	struct DVDfileInfo * next;      //下一个
    struct DVDfileInfo * prev;      //前一个
}DVDfileInfo_t ;

//udf盘面
struct udf_disc
{
	uint16_t			udf_rev;              //udf版本
	uint16_t			blocksize;            //块大小，2048
	uint32_t            blocks;               //块总数
	uint16_t            packetSize;           //包大小, 默认16
	uint8_t				blocksize_bits;       //块字节位数，11
	uint32_t			flags;                //标记

	struct udf_sizing	sizing[UDF_ALLOC_TYPE_SIZE]; 

	struct volStructDesc			*udf_vrs[3];    //VRS
	struct anchorVolDescPtr			*udf_anchor[3]; //AVDP
	struct primaryVolDesc			*udf_pvd[2];    //PVD
	struct logicalVolDesc			*udf_lvd[2];    //LVD
	struct partitionDesc			*udf_pd[2];     //PDS,分区描述
	struct unallocSpaceDesc			*udf_usd[2];    //USD,数据区，DVD-R可以不用
	struct impUseVolDesc			*udf_iuvd[2];   //IVDS，DvD-R无用
	struct terminatingDesc			*udf_td[3];     //TDS，光盘封闭描述包
	struct logicalVolIntegrityDesc	*udf_lvid;      //LVID  逻辑卷标完整性描述包
	struct fileSetDesc				*udf_fsd;       //FSD,文件集描述包

	struct udf_extent				*head;
	struct udf_extent				*tail;
};

//udf盘区
struct udf_extent
{
	enum udf_space_type		space_type;
	uint32_t				start;
	uint32_t				blocks;
	struct udf_desc			*head;
	struct udf_desc			*tail;

	struct udf_extent		*next;
	struct udf_extent		*prev;
};

//udf描述符
struct udf_desc
{
	uint16_t			ident;
	uint32_t			offset;            //链表中的位置
	uint64_t			length;
	struct udf_data		*data;

	struct udf_desc		*next;
	struct udf_desc		*prev;
};

//udf数据
struct udf_data
{
	uint64_t			length;
	uint32_t            ilength;
	int				    bMalloc;	// 自己分配
	void				*buffer;
	struct udf_data		*next;
	struct udf_data		*prev;
};

/* crc.c */
uint16_t DVDRecUdf_crc(uint8_t *, uint32_t, uint16_t);

/* extent.c */
struct udf_extent *DVDRecUdf_next_extent(struct udf_extent *, enum udf_space_type);
//uint32_t next_extent_size(struct udf_extent *, enum udf_space_type, uint32_t, uint32_t);
//struct udf_extent *prev_extent(struct udf_extent *, enum udf_space_type);
//uint32_t prev_extent_size(struct udf_extent *, enum udf_space_type, uint32_t, uint32_t);
//struct udf_extent *find_extent(struct udf_disc *, uint32_t);
struct udf_extent *DVDRecUdf_set_extent(void *hMem, struct udf_disc *, enum udf_space_type, uint32_t,uint32_t);
struct udf_desc *DVDRecUdf_next_desc(struct udf_desc *, uint16_t);
struct udf_desc *DVDRecUdf_find_desc(struct udf_extent *, uint32_t);
struct udf_desc *DVDRecUdf_set_desc(void *hMem, struct udf_disc *, struct udf_extent *, uint16_t, uint32_t, uint32_t, struct udf_data *);
void DVDRecUdf_append_data(struct udf_desc *, struct udf_data *);
void* DVDRecUdf_realloc(void *hMem,struct udf_desc *desc,unsigned long long size);//add by passion 17.4.23
struct udf_data *DVDRecUdf_alloc_data(void *hMem, int);


/* file.c */
tag DVDRecUdf_query_tag(struct udf_disc *, struct udf_extent *, struct udf_desc *, uint16_t);
tag DVDRecUdf_udf_query_tag(struct udf_disc *, uint16_t, uint16_t, uint32_t, struct udf_data *, uint16_t,uint16_t jump);
struct udf_desc *DVDRecUdf_udf_create(void *hMem, struct udf_disc *, struct udf_extent *, uint32_t, 
										struct udf_desc *, uint8_t, timestamp *ts, uint8_t, uint16_t,uint16_t,uint32_t);
//void insert_data(struct udf_disc *disc, struct udf_extent *pspace, struct udf_desc *desc, struct udf_data *data);
int DVDRecUdf_insert_fid(void *hMem, struct udf_disc *, struct udf_extent *, struct udf_desc *, uint8_t *, uint8_t ,int ,uint8_t,uint16_t,uint32_t);
int DVDRec_Utf8ToUnicode(unsigned char * pSrc, unsigned char * pDst, int buflen, int * slen);//utf-8 转 unicode
//int LvDVDRecUdf_encode_utf8(char *, char *, char *, int);
//int LvDVDRecUdf_EncUnicodeStr(uint8_t *instr,uint8_t * outstr,int inlen,int outMaxlen);
int DVDRecUdf_insert_desc(struct udf_desc *desc);

static inline void clear_bits(uint8_t *bitmap, uint32_t offset, uint64_t length)
{
	for (;length>0;length--)
	{
		bitmap[(length+offset-1)/8] &= ~(1 << ((offset+length-1)%8));
	}
}

static inline struct impUseVolDescImpUse *query_iuvdiu(struct udf_disc *disc)
{
	return (struct impUseVolDescImpUse *)disc->udf_iuvd[0]->impUse;
}

static inline struct logicalVolIntegrityDescImpUse *query_lvidiu(struct udf_disc *disc)
{
	return (struct logicalVolIntegrityDescImpUse *)&(disc->udf_lvid->impUse[le32_to_cpu(disc->udf_lvd[0]->numPartitionMaps) * 2 * sizeof(uint32_t)]);
}

#endif//__LIBUDFFS_H__
