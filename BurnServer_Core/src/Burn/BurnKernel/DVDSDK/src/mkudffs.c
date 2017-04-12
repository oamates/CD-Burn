/*
 * mkudffs.c
 * DVD-R的文件系统数据结构
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include "memmalloc.h"

#include "debug.h"
#include "LibDVDSDK.h"
#include "drvcomm.h"

#include "mkudffs.h"
#include "defaults.h"
#include "UdffsTag.h"

#if 0
static char *udf_space_type_str[UDF_SPACE_TYPE_SIZE] = 
{
	"RESERVED",
	"VRS",
	"ANCHOR",
	"PVDS",
	"RVDS",
	"LVID",
	"FETMP",
	"FSD",
	"PSPACE",
	"USPACE",
	"IFO",
	"VOB",
};
#endif

//添加通用物理分区到逻辑卷，并在LVID中记录分区使用数量和
static void add_type1_partition(void *hMem, struct udf_disc *disc, uint16_t partitionNum)
{
	struct genericPartitionMap1 *pm;
	int mtl = le32_to_cpu(disc->udf_lvd[0]->mapTableLength);
	int npm = le32_to_cpu(disc->udf_lvd[0]->numPartitionMaps);

	disc->udf_lvd[0] = (struct logicalVolDesc*)MEMREALLOC(hMem, disc->udf_lvd[0],
		sizeof(struct logicalVolDesc) + mtl +
		sizeof(struct genericPartitionMap1));

	pm = (struct genericPartitionMap1 *)&disc->udf_lvd[0]->partitionMaps[mtl];
	mtl += sizeof(struct genericPartitionMap1);

	disc->udf_lvd[0]->mapTableLength = cpu_to_le32(mtl);
	disc->udf_lvd[0]->numPartitionMaps = cpu_to_le32(npm + 1);
	
	pm->partitionMapType = 1;
	pm->partitionMapLength = sizeof(struct genericPartitionMap1);
	pm->volSeqNum = cpu_to_le16(1);
	pm->partitionNum = cpu_to_le16(partitionNum);

	disc->udf_lvid->numOfPartitions = cpu_to_le32(npm + 1);
	disc->udf_lvid = (struct logicalVolIntegrityDesc*)MEMREALLOC(hMem, disc->udf_lvid,
		sizeof(struct logicalVolIntegrityDesc) +
		sizeof(uint32_t) * 2 * (npm + 1) +
		sizeof(struct logicalVolIntegrityDescImpUse));
	
	memmove(&disc->udf_lvid->impUse[sizeof(uint32_t) * 2 * (npm + 1)],
		&disc->udf_lvid->impUse[sizeof(uint32_t) * 2 * npm],
		sizeof(struct logicalVolIntegrityDescImpUse));
	
	memmove(&disc->udf_lvid->impUse[sizeof(uint32_t) * (npm + 1)],
		&disc->udf_lvid->impUse[sizeof(uint32_t) * npm],
		sizeof(uint32_t));
}

//设定PVDS
static void setup_pvd(void *hMem, struct udf_disc *disc, struct udf_extent *pvds
                  ,struct udf_extent *rvds, uint32_t offset,uint16_t DataMode)
{
	struct udf_desc *desc;
	int length = sizeof(struct primaryVolDesc);

	desc = DVDRecUdf_set_desc(hMem, disc, pvds, TAG_IDENT_PVD, offset, 0, NULL);
	desc->length = desc->data->length = length;
	desc->data->buffer = disc->udf_pvd[0];
	if(DataMode == UDFDATAMODE_DATA)
		disc->udf_pvd[0]->impIdent.identSuffix[0]= 5;
	disc->udf_pvd[0]->descTag = DVDRecUdf_query_tag(disc, pvds, desc, DataMode);

	desc = DVDRecUdf_set_desc(hMem, disc, rvds, TAG_IDENT_PVD, offset, length, NULL);
	memcpy(disc->udf_pvd[1] = (struct primaryVolDesc*)desc->data->buffer, disc->udf_pvd[0], length);
	disc->udf_pvd[1] = (struct primaryVolDesc *)desc->data->buffer;
	if(DataMode == UDFDATAMODE_DATA)
		disc->udf_pvd[1]->impIdent.identSuffix[0]= 5;
	disc->udf_pvd[1]->descTag = DVDRecUdf_query_tag(disc, rvds, desc, DataMode);

}

//设定IUVD
static void setup_iuvd(void *hMem, struct udf_disc *disc, struct udf_extent *pvds
                   ,struct udf_extent *rvds, uint32_t offset,uint16_t DataMode)
{
	struct udf_desc *desc;
	int length = sizeof(struct impUseVolDesc);

	((uint16_t *)disc->udf_iuvd[0]->impIdent.identSuffix)[0] = cpu_to_le16(disc->udf_rev);

	desc = DVDRecUdf_set_desc(hMem, disc, pvds, TAG_IDENT_IUVD, offset, 0, NULL);
	desc->length = desc->data->length = length;
	desc->data->buffer = disc->udf_iuvd[0];
	if(DataMode == UDFDATAMODE_DATA)
		disc->udf_iuvd[0]->impIdent.identSuffix[2] = 5;
	disc->udf_iuvd[0]->descTag = DVDRecUdf_query_tag(disc, pvds, desc, DataMode);
	
	desc = DVDRecUdf_set_desc(hMem, disc, rvds, TAG_IDENT_IUVD, offset, length, NULL);
	memcpy(disc->udf_iuvd[1] = (impUseVolDesc*)desc->data->buffer, disc->udf_iuvd[0], length);
	disc->udf_iuvd[1] = (struct impUseVolDesc *)desc->data->buffer;
	if(DataMode == UDFDATAMODE_DATA)
		disc->udf_iuvd[1]->impIdent.identSuffix[2] = 5;
	disc->udf_iuvd[1]->descTag = DVDRecUdf_query_tag(disc, rvds, desc, DataMode);

}

//设定分区pD
static void setup_pd(void *hMem, struct udf_disc *disc, struct udf_extent *pvds
                 ,struct udf_extent *rvds, uint32_t offset, uint16_t DataMode
                 ,udfinfo_t *pUdfInfo)
{
	struct udf_desc *desc;
	struct udf_extent *ext;
	int PDStart,PDBlocks;
	int length = sizeof(struct partitionDesc);

	ext = DVDRecUdf_next_extent(disc->head, FSD);
	PDStart = ext->start;
	PDBlocks = pUdfInfo->m_CdRwDiskinfo->udffile.writenext - pUdfInfo->m_CdRwDiskinfo->udffile.writestart;//disc->blocks - UDF_SYS_LEN - UDF_FSD_LEN - 512;
	PDBlocks += UDF_FS_LENGTH + PACKET_BLOCK_16 - UDF_SYS_LEN;

	disc->udf_pd[0]->partitionStartingLocation = cpu_to_le32(UDF_SYS_LEN);
	disc->udf_pd[0]->partitionLength = cpu_to_le32(PDBlocks);
	
	desc = DVDRecUdf_set_desc(hMem, disc, pvds, TAG_IDENT_PD, offset, 0, NULL);
	desc->length = desc->data->length = length;
	desc->data->buffer = disc->udf_pd[0];
	if(DataMode == UDFDATAMODE_DATA)
	{
		disc->udf_pd[0]->impIdent.identSuffix[0] = 5;
	}
	else
		disc->udf_pd[0]->partitionContents.flags = 2;
	disc->udf_pd[0]->descTag = DVDRecUdf_query_tag(disc, pvds, desc, DataMode);


	desc = DVDRecUdf_set_desc(hMem, disc, rvds, TAG_IDENT_PD, offset, length, NULL);
	memcpy(disc->udf_pd[1] = (struct partitionDesc*)desc->data->buffer, disc->udf_pd[0], length);
	disc->udf_pd[1] = (struct partitionDesc *)desc->data->buffer;
	if(DataMode == UDFDATAMODE_DATA)
	{
		disc->udf_pd[1]->impIdent.identSuffix[0] = 5;
	}
	else
		disc->udf_pd[1]->partitionContents.flags = 2;
	disc->udf_pd[1]->descTag = DVDRecUdf_query_tag(disc, rvds, desc, DataMode);
}

//设定LVD
static void setup_lvd(void *hMem, struct udf_disc *disc, struct udf_extent *pvds
                 ,struct udf_extent *rvds, struct udf_extent *lvid
                 ,uint32_t offset,uint16_t DataMode)
{
	struct udf_desc *desc;
	int length = sizeof(struct logicalVolDesc) + le32_to_cpu(disc->udf_lvd[0]->mapTableLength);

	//disc->udf_lvd[0]->integritySeqExt.extLength = cpu_to_le32(lvid->blocks * disc->blocksize);
	disc->udf_lvd[0]->integritySeqExt.extLength = cpu_to_le32(2 * disc->blocksize);
	disc->udf_lvd[0]->integritySeqExt.extLocation = cpu_to_le32(lvid->start);

	if(DataMode == UDFDATAMODE_DATA)
        ((long_ad *)disc->udf_lvd[0]->logicalVolContentsUse)->extLength = cpu_to_le32(CDROM_BLOCK);
	else
		((long_ad *)disc->udf_lvd[0]->logicalVolContentsUse)->extLength = cpu_to_le32(2*CDROM_BLOCK);
	((long_ad *)disc->udf_lvd[0]->logicalVolContentsUse)->extLocation.logicalBlockNum = cpu_to_le32(0);
	((long_ad *)disc->udf_lvd[0]->logicalVolContentsUse)->extLocation.partitionReferenceNum = cpu_to_le16(0);

	desc = DVDRecUdf_set_desc(hMem, disc, pvds, TAG_IDENT_LVD, offset, 0, NULL);
	desc->length = desc->data->length = length;
	desc->data->buffer = disc->udf_lvd[0];
	if(DataMode == UDFDATAMODE_DATA)
		disc->udf_lvd[0]->impIdent.identSuffix[0] = 5;
	
	disc->udf_lvd[0]->descTag = DVDRecUdf_query_tag(disc, pvds, desc, DataMode);


	desc = DVDRecUdf_set_desc(hMem, disc, rvds, TAG_IDENT_LVD, offset, length, NULL);
	memcpy(disc->udf_lvd[1] = (struct logicalVolDesc*)desc->data->buffer, disc->udf_lvd[0], length);
	
	if(DataMode == UDFDATAMODE_DATA)
		((long_ad *)disc->udf_lvd[1]->logicalVolContentsUse)->extLength = cpu_to_le32(CDROM_BLOCK);
	else
		((long_ad *)disc->udf_lvd[1]->logicalVolContentsUse)->extLength = cpu_to_le32(2*CDROM_BLOCK);
	((long_ad *)disc->udf_lvd[1]->logicalVolContentsUse)->extLocation.logicalBlockNum = cpu_to_le32(0);
	((long_ad *)disc->udf_lvd[1]->logicalVolContentsUse)->extLocation.partitionReferenceNum = cpu_to_le16(0);

	disc->udf_lvd[1] = (struct logicalVolDesc *)desc->data->buffer;
	if(DataMode == UDFDATAMODE_DATA)
		disc->udf_lvd[1]->impIdent.identSuffix[0] = 5;	
	disc->udf_lvd[1]->descTag = DVDRecUdf_query_tag(disc, rvds, desc, DataMode);

}

//设定USD，没有分配的空间，也可以不分配启用该描述符，对于提前封盘的情况下，没有分区的空间都算USD空间
static void setup_usd(void *hMem, struct udf_disc *disc, struct udf_extent *pvds
                  ,struct udf_extent *rvds, uint32_t offset,uint16_t DataMode)
{
	struct udf_desc *desc;
	int length = sizeof(struct unallocSpaceDesc);

	/*
	ext = LvDVDRecUdf_next_extent(disc->head, USPACE);
	while (ext)
	{
		length += sizeof(extent_ad);
		disc->udf_usd[0] = MEMREALLOC(hMem, disc->udf_usd[0], length);
		disc->udf_usd[0]->numAllocDescs = cpu_to_le32(le32_to_cpu(disc->udf_usd[0]->numAllocDescs)+1);
		disc->udf_usd[0]->allocDescs[count].extLength = cpu_to_le32(ext->blocks * disc->blocksize);
		disc->udf_usd[0]->allocDescs[count].extLocation = cpu_to_le32(ext->start);
		count ++;
		ext = LvDVDRecUdf_next_extent(ext->next, USPACE);
	}
	*/
	//disc->udf_usd[0] = MEMREALLOC(hMem, disc->udf_usd[0], length);
	disc->udf_usd[0]->numAllocDescs =  cpu_to_le32(0);//cpu_to_le32(le32_to_cpu(disc->udf_usd[0]->numAllocDescs)+1);
	//disc->udf_usd[0]->allocDescs[count].extLength = cpu_to_le32(0);//cpu_to_le32(ext->blocks * disc->blocksize);
	//disc->udf_usd[0]->allocDescs[count].extLocation = cpu_to_le32(0);//cpu_to_le32(ext->start);


	desc = DVDRecUdf_set_desc(hMem, disc, pvds, TAG_IDENT_USD, offset, 0/*length*/, NULL);
	desc->length = desc->data->length = length;
	desc->data->buffer = disc->udf_usd[0];
	disc->udf_usd[0]->descTag = DVDRecUdf_query_tag(disc, pvds, desc, DataMode);

	desc = DVDRecUdf_set_desc(hMem, disc, rvds, TAG_IDENT_USD, offset, length, NULL);
	memcpy(disc->udf_usd[1] = (struct unallocSpaceDesc *)desc->data->buffer, disc->udf_usd[0], length);
	disc->udf_usd[1] = (struct unallocSpaceDesc *)desc->data->buffer;
	//memcpy(disc->udf_usd[1] = desc->data->buffer, disc->udf_usd[0], length);
	disc->udf_usd[1]->descTag = DVDRecUdf_query_tag(disc, rvds, desc, DataMode);

}

//设定TD
static void setup_td(void *hMem, struct udf_disc *disc, struct udf_extent *pvds
                ,struct udf_extent *rvds, uint32_t offset,uint16_t DataMode)
{
	struct udf_desc *desc;
	int length = sizeof(struct terminatingDesc);

	desc = DVDRecUdf_set_desc(hMem, disc, pvds, TAG_IDENT_TD, offset, 0, NULL);
	desc->length = desc->data->length = length;
	desc->data->buffer = disc->udf_td[0];
	disc->udf_td[0]->descTag = DVDRecUdf_query_tag(disc, pvds, desc, DataMode);

	desc = DVDRecUdf_set_desc(hMem, disc, rvds, TAG_IDENT_TD, offset, length, NULL);
	memcpy(disc->udf_td[1] = (struct terminatingDesc *)desc->data->buffer, disc->udf_td[0], length);
	disc->udf_td[1] = (struct terminatingDesc *)desc->data->buffer;
	disc->udf_td[1]->descTag = DVDRecUdf_query_tag(disc, rvds, desc, DataMode);
}

//设定IVID
static void setup_lvid(void *hMem, struct udf_disc *disc, struct udf_extent *lvid
                  ,uint16_t DataMode)
{
	struct udf_desc *desc;
	struct udf_extent *ext;
	int length = sizeof(struct logicalVolIntegrityDesc) + le32_to_cpu(disc->udf_lvid->numOfPartitions) * sizeof(uint32_t) * 2 + le32_to_cpu(disc->udf_lvid->lengthOfImpUse);

	ext = DVDRecUdf_next_extent(disc->head, LVID);
	disc->udf_lvid->freeSpaceTable[0] = 0;//cpu_to_le32(ext->blocks); //统计物理分区空间的大小
	disc->udf_lvid->sizeTable[1] = 0;//cpu_to_le32(ext->blocks);  //统计可用空间大小
	((struct logicalVolHeaderDesc *)disc->udf_lvid->logicalVolContentsUse)->uniqueID = 32;
	
	if (disc->flags & FLAG_VAT) 
	{
		disc->udf_lvid->integrityType = cpu_to_le32(LVID_INTEGRITY_TYPE_OPEN);
	}
	else
	{
		disc->udf_lvid->integrityType = LVID_INTEGRITY_TYPE_CLOSE;
	}
	desc = DVDRecUdf_set_desc(hMem, disc, lvid, TAG_IDENT_LVID, 0, 0, NULL);
	desc->length = desc->data->length = length;
	desc->data->buffer = disc->udf_lvid;
	disc->udf_lvid->descTag = DVDRecUdf_query_tag(disc, lvid, desc, DataMode);

	desc = DVDRecUdf_set_desc(hMem, disc, lvid, TAG_IDENT_TD, 1, sizeof(struct terminatingDesc), NULL);
	disc->udf_td[2] = (struct terminatingDesc *)desc->data->buffer;
	disc->udf_td[2]->descTag = DVDRecUdf_query_tag(disc, lvid, desc, DataMode);

}

/**************************************************************************************************/

#if 0
//设定udf文件系统版本
static int udf_set_version(struct udf_disc *disc, int udf_rev)
{
	struct logicalVolIntegrityDescImpUse *lvidiu;

	if (disc->udf_rev == udf_rev)
	{
		return 0;
	}
	else if (udf_rev != 0x0102 &&
			udf_rev != 0x0150 &&
			udf_rev != 0x0200 &&
			udf_rev != 0x0201 &&
			udf_rev != 0x0250)
	{
		return 1;
	}
	else
	{
		disc->udf_rev = udf_rev;
	}

	if (disc->udf_rev < 0x0200)
	{
		disc->flags &= ~FLAG_EFE;
		strcpy((char *)disc->udf_pd[0]->partitionContents.ident, PD_PARTITION_CONTENTS_NSR02);
	}
	else
	{
		disc->flags |= FLAG_EFE;
		strcpy((char *)disc->udf_pd[0]->partitionContents.ident, PD_PARTITION_CONTENTS_NSR03);
	}

	((uint16_t *)disc->udf_fsd->domainIdent.identSuffix)[0] = cpu_to_le16(udf_rev); 
	((uint16_t *)disc->udf_lvd[0]->domainIdent.identSuffix)[0] = cpu_to_le16(udf_rev); 
	((uint16_t *)disc->udf_iuvd[0]->impIdent.identSuffix)[0] = le16_to_cpu(udf_rev); 
	lvidiu = query_lvidiu(disc);
	lvidiu->minUDFReadRev = le16_to_cpu(udf_rev);
	lvidiu->minUDFWriteRev = le16_to_cpu(udf_rev);
	lvidiu->maxUDFWriteRev = le16_to_cpu(udf_rev);
	//((uint16_t *)disc->udf_stable[0]->sparingIdent.identSuffix)[0] = le16_to_cpu(udf_rev);
	return 0;
}
#endif

/*
udf-disc-blocks=2297888
start=0, blocks=16, type=RESERVED //保留扇区，可以全部写0x00
start=16, blocks=16, type=VRS     //卷标注册序列,占一个包
start=32, blocks=16, type=PVDS    //主卷描述序列，占一个包，连续包括PVDS,IUVD,PD,USD,TD
start=48, blocks=16, type=RVDS    //保留卷描述序列，占一个包，连续包括PVDS,IUVD,PD,USD,TD 
start=64, blocks=16, type=LVID    //LVID新轨道开始，占一个包，连续包括,LVID,TD
start=80, blocks=176, type=USPACE  //连续空闲11个包，全部写0x00
start=256, blocks=32, type=ANCHOR  //定位点描述符，占用两个包
start=288, blocks=16, type=USPACE  //封闭轨道1，占用一个包

start=304, blocks=512, type=FSD    //文件集描述符，占用4个包，连续包括,FSD,ROOT_FE,ROOT_FID,VIDEO_TS_FE,VIDEO_TS_FID,VIDEO_TS.IFO_FE,.....
start=816, blocks=16, type=USPACE  //封闭轨道2，占用一个包

 
start=832, blocks=2296544, type=VOB  //视频数据，直到封盘，占据轨道3
start=2297376, blocks=256, type=ANCHOR 
start=2297632, blocks=256, type=USPACE 


*/

#if 0
//显示盘区占用空间信息
static void dump_space(struct udf_disc *disc)
{
	struct udf_extent *start_ext;
	int i;

	start_ext = disc->head;
	
	DPERROR(("udf-disc-blocks=%d\n", disc->blocks));

	while (start_ext != NULL)
	{
		DPERROR(("start=%d, blocks=%d, type=", start_ext->start, start_ext->blocks));
		for (i=0; i<UDF_SPACE_TYPE_SIZE; i++)
		{
			if (start_ext->space_type & (1<<i))
			{
				DPERROR(("%s ", udf_space_type_str[i]));
			}
		}
		DP(("\n"));
		start_ext = start_ext->next;
	}
}
#endif

#if 0
//设定pspace Space ，该结构在DVD-R上是不应该记录的
static int setup_space(struct udf_disc *disc, struct udf_extent *pspace, uint32_t offset)
{
	struct udf_desc *desc;
	struct partitionHeaderDesc *phd = (struct partitionHeaderDesc *)disc->udf_pd[0]->partitionContentsUse;
	int length = (((sizeof(struct spaceBitmapDesc) + pspace->blocks) >> (disc->blocksize_bits + 3)) + 1) << disc->blocksize_bits;

	if (disc->flags & FLAG_FREED_BITMAP)
	{
		phd->freedSpaceBitmap.extPosition = cpu_to_le32(offset);
		phd->freedSpaceBitmap.extLength = cpu_to_le32(length);
		disc->udf_lvid->freeSpaceTable[0] = cpu_to_le32(le32_to_cpu(disc->udf_lvid->freeSpaceTable[0]) - (length >> disc->blocksize_bits));
	}
	else if (disc->flags & FLAG_FREED_TABLE)
	{
		phd->freedSpaceTable.extPosition = cpu_to_le32(offset);
		if (disc->flags & FLAG_STRATEGY4096)
		{
			phd->freedSpaceTable.extLength = cpu_to_le32(disc->blocksize * 2);
			disc->udf_lvid->freeSpaceTable[0] = cpu_to_le32(le32_to_cpu(disc->udf_lvid->freeSpaceTable[0]) - 2);
		}
		else
		{
			phd->freedSpaceTable.extLength = cpu_to_le32(disc->blocksize);
			disc->udf_lvid->freeSpaceTable[0] = cpu_to_le32(le32_to_cpu(disc->udf_lvid->freeSpaceTable[0]) - 1);
		}
	}
	else if (disc->flags & FLAG_UNALLOC_BITMAP)
	{
		phd->unallocSpaceBitmap.extPosition = cpu_to_le32(offset);
		phd->unallocSpaceBitmap.extLength = cpu_to_le32(length);
		disc->udf_lvid->freeSpaceTable[0] = cpu_to_le32(le32_to_cpu(disc->udf_lvid->freeSpaceTable[0]) - (length >> disc->blocksize_bits));
	}
	else if (disc->flags & FLAG_UNALLOC_TABLE)
	{
		phd->unallocSpaceTable.extPosition = cpu_to_le32(offset);
		if (disc->flags & FLAG_STRATEGY4096)
		{
			phd->unallocSpaceTable.extLength = cpu_to_le32(disc->blocksize * 2);
			disc->udf_lvid->freeSpaceTable[0] = cpu_to_le32(le32_to_cpu(disc->udf_lvid->freeSpaceTable[0]) - 2);
		}
		else
		{
			phd->unallocSpaceTable.extLength = cpu_to_le32(disc->blocksize);
			disc->udf_lvid->freeSpaceTable[0] = cpu_to_le32(le32_to_cpu(disc->udf_lvid->freeSpaceTable[0]) - 1);
		}
	}

	if (disc->flags & FLAG_SPACE_BITMAP)
	{
		struct spaceBitmapDesc *sbd;
		int nBytes = (pspace->blocks+7)/8;

		length = sizeof(struct spaceBitmapDesc) + nBytes;
		desc = LvDVDRecUdf_set_desc(hMem, disc, pspace, TAG_IDENT_SBD, offset, length, NULL);
		sbd = (struct spaceBitmapDesc *)desc->data->buffer;
		sbd->numOfBits = cpu_to_le32(pspace->blocks);
		sbd->numOfBytes = cpu_to_le32(nBytes);
		memset(sbd->bitmap, 0xFF, sizeof(uint8_t) * nBytes);
		if (pspace->blocks%8)
			sbd->bitmap[nBytes-1] = 0xFF >> (8-(pspace->blocks%8));
		clear_bits(sbd->bitmap, offset, (length + disc->blocksize - 1) >> disc->blocksize_bits);
		sbd->descTag = udf_query_tag(disc, TAG_IDENT_SBD, 1, desc->offset, desc->data, sizeof(tag),0);
	}
	else if (disc->flags & FLAG_SPACE_TABLE)
	{
		struct unallocSpaceEntry *use;
		short_ad *sad;
		int max = (0x3FFFFFFF / disc->blocksize) * disc->blocksize;
		int pos = 0;
		long long rem;

		if (disc->flags & FLAG_STRATEGY4096)
			length = disc->blocksize * 2;
		else
			length = disc->blocksize;
		desc = LvDVDRecUdf_set_desc(hMem, disc, pspace, TAG_IDENT_USE, offset, disc->blocksize, NULL);
		use = (struct unallocSpaceEntry *)desc->data->buffer;
		use->lengthAllocDescs = cpu_to_le32(sizeof(short_ad));
		sad = (short_ad *)&use->allocDescs[0];
		rem = (long long)pspace->blocks * disc->blocksize - length;
		if (disc->blocksize - sizeof(struct unallocSpaceEntry) < (rem / max) * sizeof(short_ad))
		pos = offset + (length/disc->blocksize);
		DP(("pos=%d, rem=%lld\n", pos, rem));
		if (rem > 0x3FFFFFFF)
		{
			while (rem > max)
			{
				sad->extLength = cpu_to_le32(EXT_NOT_RECORDED_ALLOCATED | max);
				sad->extPosition = cpu_to_le32(pos);
				pos += max / disc->blocksize;
				sad ++;
				rem -= max;
				use->lengthAllocDescs = cpu_to_le32(le32_to_cpu(use->lengthAllocDescs) + sizeof(short_ad));
			}
		}
		sad->extLength = cpu_to_le32(EXT_NOT_RECORDED_ALLOCATED | rem);
		sad->extPosition = cpu_to_le32(pos);

		if (disc->flags & FLAG_STRATEGY4096)
		{
			use->icbTag.strategyType = cpu_to_le16(4096);
			use->icbTag.strategyParameter = cpu_to_le16(1);
			use->icbTag.numEntries = cpu_to_le16(2);
		}
		else
		{
			use->icbTag.strategyType = cpu_to_le16(4);
			use->icbTag.numEntries = cpu_to_le16(1);
		}
		use->icbTag.parentICBLocation.logicalBlockNum = cpu_to_le32(0);
		use->icbTag.parentICBLocation.partitionReferenceNum = cpu_to_le16(0);
		//use->icbTag.fileType = ICBTAG_FILE_TYPE_USE;
		//use->icbTag.flags = cpu_to_le16(ICBTAG_FLAG_AD_SHORT);
		use->descTag = udf_query_tag(disc, TAG_IDENT_USE, 1, desc->offset, desc->data, sizeof(struct unallocSpaceEntry) + le32_to_cpu(use->lengthAllocDescs),0);

		if (disc->flags & FLAG_STRATEGY4096)
		{
			struct udf_desc *tdesc;
			struct terminalEntry *te;

			if (disc->flags & FLAG_BLANK_TERMINAL)
			{
//				tdesc = LvDVDRecUdf_set_desc(hMem, disc, pspace, TAG_IDENT_IE, offset+1, sizeof(struct indirectEntry), NULL);
			}
			else
			{
				tdesc = LvDVDRecUdf_set_desc(hMem, disc, pspace, TAG_IDENT_TE, offset+1, sizeof(struct terminalEntry), NULL);
				te = (struct terminalEntry *)tdesc->data->buffer;
				te->icbTag.priorRecordedNumDirectEntries = cpu_to_le32(1);
				te->icbTag.strategyType = cpu_to_le16(4096);
				te->icbTag.strategyParameter = cpu_to_le16(1);
				te->icbTag.numEntries = cpu_to_le16(2);
				te->icbTag.parentICBLocation.logicalBlockNum = cpu_to_le32(desc->offset);
				te->icbTag.parentICBLocation.partitionReferenceNum = cpu_to_le16(0);
				te->icbTag.fileType = ICBTAG_FILE_TYPE_TE;
				te->descTag = LvDVDRecUdf_query_tag(disc, pspace, tdesc, 0);
			}
		}
	}

	return (length + disc->blocksize - 1) >> disc->blocksize_bits;
}
#endif

#if 0
//添加DVD文件
void addDVDFile(struct udf_disc * disc,struct udf_extent * ext, struct udf_desc * desc ,
				struct udf_desc * Parentdesc ,char * fileName, UdfDVDFileType ft, uint32_t start)
{
	DVDfileInfo_t * tmp = (DVDfileInfo_t *) MEMMALLOC(hMem, sizeof(DVDfileInfo_t));
	memset(tmp,0x00,sizeof(DVDfileInfo_t));
	if( !tmp ) return ;
	
	if( disc->DVDFilehead == NULL )
	{	    
		disc->DVDFilehead = disc->DVDFiletail = tmp;
		disc->DVDFilehead->prev = disc->DVDFiletail->next = disc->DVDFiletail->prev = disc->DVDFilehead->next = NULL;
	}
	else
	{
		
		disc->DVDFiletail->next = tmp;
		tmp->next = NULL;
		tmp->prev = disc->DVDFiletail;
		disc->DVDFiletail = tmp;		
	}
	
	tmp->Data = NULL;
	tmp->ext = ext;
	tmp->Parentdesc = Parentdesc;
	tmp->desc = desc;
	tmp->fe = (struct fileEntry *)desc->data->buffer;
	strcpy(tmp->fileName,fileName);
	tmp->fileSize = 0;
	tmp->FileType = ft;
	tmp->start = start;	
}

//查找DVD文件
DVDfileInfo_t * findDVDFile(struct udf_disc * disc,char * fileName)
{
	DVDfileInfo_t * tmp = disc->DVDFilehead;
	while( tmp )
	{
		if( !strcmp(fileName,tmp->fileName) ) return tmp;
		tmp = tmp->next;
	}		
	return NULL;
}

#endif 

static int change_data(struct udf_desc *desc,int pos,int len)
{
	struct fileEntry *fe;
	unsigned int fidlen = 0;
	fe = (struct fileEntry *)desc->data->buffer;

	if ((le16_to_cpu(fe->icbTag.flags) & ICBTAG_FLAG_AD_MASK) == ICBTAG_FLAG_AD_SHORT)
	{
		short_ad *sad;

		fe = (struct fileEntry *)desc->data->buffer;
		fe->lengthAllocDescs = cpu_to_le32(sizeof(short_ad));

		sad = (short_ad *)(&fe->extendedAttr[0]);
		sad->extPosition = cpu_to_le32(pos);
		sad->extLength = cpu_to_le32(len);
	}
	else if ((le16_to_cpu(fe->icbTag.flags) & ICBTAG_FLAG_AD_MASK) == ICBTAG_FLAG_AD_LONG)
	{

		long_ad *lad;

		fe = (struct fileEntry *)desc->data->buffer;
		fe->lengthAllocDescs = cpu_to_le32(sizeof(long_ad));
		
		lad = (long_ad *)&fe->allocDescs[le32_to_cpu(fe->lengthExtendedAttr)];
		lad->extLocation.partitionReferenceNum = cpu_to_le16(0);
		lad->extLocation.logicalBlockNum = cpu_to_le32(pos);
		lad->extLength = cpu_to_le32(len);
		
	}
	if(len > 2048)                                         //add by yanming 10.12.24
		fidlen += ((len/2048)+(((len%2048) > 0) ? 1:0));
	fe->logicalBlocksRecorded = cpu_to_le32(fidlen);

	//fe->logicalBlocksRecorded = tmp->fileSize / 2048 + ( tmp->fileSize % 2048 == 0 ? 0 : 1);

	return 0;

}
//计算fid需要的大小
static int CalcNeedFidSize(FILDIRNODE * Dir,int DataMode)
{
	FILDIRNODE * tmpDir;
	int namelen = 0;
	int tmplen = 0;

	unsigned char UnicodeName[256];
	int buflen = 256;         //buf 长度
	int slen = 0;

	if(Dir == NULL)
		return 0;
	tmpDir = Dir;
	while(tmpDir)
	{
		//get unicode
		DVDRec_Utf8ToUnicode((unsigned char *)tmpDir->Name, UnicodeName, buflen, &slen);
		if(DataMode == UDFDATAMODE_DATA)
		{
			tmplen = slen;//strlen(tmpDir->Name); modify by yanming for utf
			//tmplen = LvDVDRecUdf_EncUnicodeStr(tmpDir->Name,(uint8_t *)tmpname,strlen(tmpDir->Name),strlen(tmpDir->Name)+2);
		}
		else
			tmplen = strlen(tmpDir->Name);	
		//tmplen = (2*count);
		//tmplen = 2*strlen(tmpname);
		//namelen += PAD((40+(2*strlen(tmpDir->Name))),4);
		namelen += PAD((40+tmplen),4);
		DP(("tmpDir->Name=%s\n",tmpDir->Name));
		//printf("namelen=%d,tmplen=%d\n",namelen,tmplen);
		tmpDir = tmpDir->Next;
	}
	namelen += 40; //第一个fid的长度为40
	//printf("1namelen=%d\n",namelen);
	return namelen;
}
//计算路径深度
static int DirDepth(FILDIRNODE * Directory,int DataMode)
{
	int num = 0;
	int fidlen = 0;
	FILDIRNODE * Dir = NULL;

	Dir = Directory;//->Child;
	if(Dir)
	{
		if(Dir->NodeType == NODETYPE_DIR)
		{
			num += 2;   //目录占用2个数据区
			fidlen = CalcNeedFidSize(Dir->Child,DataMode); //计算fid占的空间
			if(fidlen > 2048)
			{
				num += ((fidlen/2048)+(((fidlen%2048) > 0) ? 1:0));
				num--;		//多加一个
			}			
		}
		else if(Dir->NodeType == NODETYPE_FILE)
		{
			num += 1;
		}
		
		if(Dir->Next)
			num += DirDepth(Dir->Next,DataMode);
		if(Dir->Child)
			num += DirDepth(Dir->Child,DataMode);
	}
	return num;
}

static int RecursiveTree(void *hMem, struct udf_disc *disc,struct udf_extent *pspace,
	            FILDIRNODE * Directory,uint32_t offset,
	            udfinfo_t *pUdfInfo, struct udf_desc *desc)
{
	uint32_t FileType,FileChar,suboffset,ChildNum,ChildFid,i,j,num,Num_Fid,Cumulative;
	uint8_t DirCount;
	//uint8_t tempunicode[256];
	struct udf_desc *fe_desc;
	struct fileEntry *fentry[MAX_FILE_OR_DIR];
	struct udf_desc *fe[MAX_FILE_OR_DIR];//目录文件数不大于256
	FILDIRNODE *SubDir[MAX_FILE_OR_DIR];
	FILDIRNODE * Dir;
	//int flag = 1;
	int Depth = 0;
	int SecNum = 0;
	int NeedFidLen = 0;
	unsigned char UnicodeName[256];
	int buflen = 256;
	int slen = 0;

	if(Directory == NULL) return 0;
	Dir = Directory;
	suboffset = offset;
	for(i = 0;i < MAX_FILE_OR_DIR;i++)
	{
		SubDir[i] = NULL;
		fe[i] = NULL;
		fentry[i] = NULL;
	}
	num = i = 0;
	Num_Fid = ChildNum = 0;

	//DirCount = pUdfInfo->udf_cmd->getdircount(pUdfInfo->m_FileDirTree);
	DirCount = pUdfInfo->udfCmd.GetDirCount(pUdfInfo->m_FileDirTree);
	//查找目录或文件并创建FE
	do{
		if(Dir->NodeType == NODETYPE_DIR)
		{
			FileType = ICBTAG_FILE_TYPE_DIRECTORY;  //目录
			FileChar = FID_FILE_CHAR_DIRECTORY;
		}
		else
		{
			FileType = ICBTAG_FILE_TYPE_REGULAR;  //文件
			FileChar = FID_FILE_CHAR_METADATA;
		}
		if(Dir->FileSize < FILE1G)
		{
			fe_desc = DVDRecUdf_udf_create(hMem, disc, pspace,suboffset, desc,DirCount, //Directory->Name,suboffset, desc,DirCount, 
			       		&Dir->ts,FileType, 0, pUdfInfo->m_DataMode, 0);				
		}
		else
		{
			fe_desc = DVDRecUdf_udf_create(hMem, disc, pspace,suboffset, desc,DirCount, //Directory->Name,suboffset, desc,DirCount, 
			       		&Dir->ts,FileType, 0, pUdfInfo->m_DataMode,((Dir->FileSize/BIGFILERECORD)+(Dir->FileSize%BIGFILERECORD == 0?0:1))*16);	
		}
		
		fentry[i] = (struct fileEntry *)fe_desc->data->buffer;
		fe[i] = fe_desc;		
		//如果是文件则设定文件的位置及大小
		if(FileType == ICBTAG_FILE_TYPE_REGULAR)		 //如果是文件计算文件位置及tag crc
		{
			fentry[i]->informationLength = cpu_to_le64(Dir->FileSize);
			fentry[i]->logicalBlocksRecorded = Dir->FileSize / CDROM_BLOCK + ( Dir->FileSize % CDROM_BLOCK == 0 ? 0 : 1);
			fentry[i]->logicalBlocksRecorded = cpu_to_le64(fentry[i]->logicalBlocksRecorded);

            DP(("+++++++++[RecursiveTree] File name = %s,File  location = %ld,File size =%ld++++++++++\n",Dir->Name,Dir->FileLoca,Dir->FileSize));
			if(Dir->FileSize < FILE1G)
			{
				if( (le16_to_cpu(fentry[i]->icbTag.flags) & ICBTAG_FLAG_AD_MASK) == ICBTAG_FLAG_AD_SHORT )
				{	
					fentry[i]->lengthAllocDescs = cpu_to_le32(sizeof(short_ad));
					((short_ad *)&fentry[i]->allocDescs[0])->extLength = cpu_to_le32((unsigned int)Dir->FileSize);
					((short_ad *)&fentry[i]->allocDescs[0])->extPosition = cpu_to_le32((unsigned int)Dir->FileLoca);//cpu_to_le32(pUdfInfo->m_CurrentFileLocation);	
				}
				else if( (le16_to_cpu(fentry[i]->icbTag.flags) & ICBTAG_FLAG_AD_MASK) == ICBTAG_FLAG_AD_LONG )
				{
					fentry[i]->lengthAllocDescs = cpu_to_le32(sizeof(long_ad));
					((long_ad *)&fentry[i]->allocDescs[0])->extLocation.logicalBlockNum = cpu_to_le32((unsigned int)Dir->FileLoca);//cpu_to_le32(pUdfInfo->m_CurrentFileLocation);
					((long_ad *)&fentry[i]->allocDescs[0])->extLocation.partitionReferenceNum = cpu_to_le16(0);
					((long_ad *)&fentry[i]->allocDescs[0])->extLength = cpu_to_le32((unsigned int)Dir->FileSize);
				}
			}
			else
			{
				fentry[i]->lengthAllocDescs = cpu_to_le32(0x50);
				for(j = 0;j < (Dir->FileSize/BIGFILERECORD);j++)
				{
					(((long_ad *)&fentry[i]->extendedAttr[0])+j)->extLocation.logicalBlockNum = cpu_to_le32(Dir->FileLoca+(j*BIGFILENUM));//cpu_to_le32(pUdfInfo->m_CurrentFileLocation+(j*BIGFILENUM));
					(((long_ad *)&fentry[i]->extendedAttr[0])+j)->extLocation.partitionReferenceNum = cpu_to_le16(0);
					(((long_ad *)&fentry[i]->extendedAttr[0])+j)->extLength = cpu_to_le32((unsigned int)BIGFILERECORD);
				}
				if((Dir->FileSize%BIGFILERECORD) == 0?0:1)
				{
					(((long_ad *)&fentry[i]->extendedAttr[0])+j)->extLocation.logicalBlockNum = cpu_to_le32(Dir->FileLoca+(j*BIGFILENUM));//cpu_to_le32(pUdfInfo->m_CurrentFileLocation+(j*BIGFILENUM));
					(((long_ad *)&fentry[i]->extendedAttr[0])+j)->extLocation.partitionReferenceNum = cpu_to_le16(0);
					(((long_ad *)&fentry[i]->extendedAttr[0])+j)->extLength = cpu_to_le32((unsigned int)(Dir->FileSize%BIGFILERECORD));
				}
			}
			
			//pUdfInfo->m_CurrentFileLocation += fentry[i]->logicalBlocksRecorded; 	
			//if(pUdfInfo->m_CurrentFileLocation % PACKET_BLOCK_16)
			//	pUdfInfo->m_CurrentFileLocation = PACKET_BLOCK_16 * ((pUdfInfo->m_CurrentFileLocation / PACKET_BLOCK_16) + 1);
		}

		fentry[i]->descTag = DVDRecUdf_query_tag(disc, pspace, fe_desc, pUdfInfo->m_DataMode);
		//记录当前目录是的子目录或文件
		SubDir[i] = Dir;
		i++;
		suboffset++;
	}while((Dir = Dir->Next));
	
	//查找当前目录下子目录或文件个数
	for(i = 0;i < MAX_FILE_OR_DIR;i++)
	{
		if((SubDir[i] == NULL)||(fe[i] == NULL))
			break;
		
		if(SubDir[i] != NULL)
		{
			if(SubDir[i]->NodeType == NODETYPE_FILE)
				continue;                           //文件不需要FID
		}
		Num_Fid++;
	}

	ChildNum += Num_Fid;
	ChildFid = suboffset;
	//创建子目录下的文件或目录的FID
	for(i = 0;i < MAX_FILE_OR_DIR;i++)
	{
		if((SubDir[i] == NULL)||(fe[i] == NULL))
			break;
		
		if(SubDir[i] != NULL)
		{
			if(SubDir[i]->NodeType == NODETYPE_FILE)
				continue;                           //文件不需要FID
		}
			
		//Dir = SubDir[i];
		Cumulative = PAD(sizeof(struct fileIdentDesc), 4);	//数据和视频的长度不同		
    	SecNum = DVDRecUdf_insert_fid(hMem, disc, pspace, fe[i], NULL,suboffset, fentry[i]->descTag.tagLocation,//Directory->NodeID,
				   FID_FILE_CHAR_DIRECTORY | FID_FILE_CHAR_PARENT,pUdfInfo->m_DataMode,Cumulative);
		if(SecNum == -1)
			return suboffset;

		if(SubDir[i]->Prior)					//modify by yanming 10.12.27
		{
			Depth += DirDepth(SubDir[i]->Prior->Child,pUdfInfo->m_DataMode);
			ChildNum = Num_Fid + Depth;// 多文件时fid占用的空间会大于1个2048
		}		
		else
		{
			ChildFid = suboffset;
			for(j = 0;j < MAX_FILE_OR_DIR;j++)
			{
				if(SubDir[j] == NULL)
					break;				
				if(SubDir[j]->NodeType == NODETYPE_FILE)
					continue;       				
				NeedFidLen = CalcNeedFidSize(SubDir[j]->Child,pUdfInfo->m_DataMode);
				if(NeedFidLen > 2048)
				{
					ChildFid += ((NeedFidLen/2048)+(((NeedFidLen%2048) > 0) ? 1:0));
					ChildFid--;         //多加一个
				}				
			}			
		}
		if(SubDir[i]->Child != NULL)
		{
			Dir = SubDir[i]->Child;
#if 0			
			ChildFid = tmpChild;
			if(flag)
			{
				for(j = 0;j < MAX_FILE_OR_DIR;j++)
				{
					flag = 0;
					if(SubDir[j] == NULL)
						break;				
					if(SubDir[j]->NodeType == NODETYPE_FILE)
						continue;       				
					//TmpDir = SubDir[i]->Child;
					NeedFidLen = CalcNeedFidSize(SubDir[j]->Child,pUdfInfo->m_DataMode);
					if(NeedFidLen > 2048)
					{
						ChildFid += ((NeedFidLen/2048)+(((NeedFidLen%2048) > 0) ? 1:0));
						ChildFid--;         //多加一个
					}				
				}			
			}
#endif			
#if 0			
			NeedFidLen = CalcNeedFidSize(Dir,pUdfInfo->m_DataMode);
			ChildFid = tmpChild;
			if(NeedFidLen > 2048)
			{
				ChildFid += ((NeedFidLen/2048)+(((NeedFidLen%2048) > 0) ? 1:0));
				ChildFid--;         //多加一个
			}
#endif			
			do{
				if(Dir->NodeType == NODETYPE_DIR)
					FileType = FID_FILE_CHAR_DIRECTORY;  //目录
				else
					FileType = 0;//文件

				//get unicode
				DVDRec_Utf8ToUnicode((unsigned char *)Dir->Name, UnicodeName, buflen, &slen);

				if(UDFDATAMODE_VIDEO == pUdfInfo->m_DataMode)
					Cumulative = PAD(sizeof(struct fileIdentDesc) + strlen((char *)Dir->Name), 4);  //根据目录的unicode的数据长度获得一个文件数据块的长度(例如:VIDEO_TS.VOB)
				else
				{
					Cumulative = PAD(sizeof(struct fileIdentDesc) + slen, 4);  //modify by yanming for utf-8			
					//Cumulative = PAD(sizeof(struct fileIdentDesc) + LvDVDRecUdf_EncUnicodeStr((uint8_t *)Dir->Name,
					//				  tempunicode,strlen((char *)Dir->Name),strlen((char *)Dir->Name)+2) + 1,4);	//数据和视频的长度不同		
				}

				SecNum = DVDRecUdf_insert_fid(hMem, disc,pspace,fe[i],
							(uint8_t *)Dir->Name,suboffset,ChildFid+(ChildNum++),FileType,pUdfInfo->m_DataMode,Cumulative);//目录为2
				if(SecNum == -1)
					return -1;

				//if(Directory->Next)											//modify by yanming 10.12.21					
				//	ChildNum++;							
			}
			while((Dir = Dir->Next));
		}

		change_data(fe[i],suboffset,(unsigned int)fentry[i]->informationLength); 
		fentry[i]->descTag = DVDRecUdf_query_tag(disc, pspace, fe[i], pUdfInfo->m_DataMode);
		//ChildNum++;
		if(SecNum)
			suboffset += SecNum;
		else
			suboffset++;
	}

#if 0
	//子目录下的fid生成
	for(i = 0;i < num;i++)
	{
		int fid;
		FILDIRNODE * Dir;
		
		fid = 0;
		Dir = Fid[i].RecordFid[fid];

		do{
			if(Dir->NodeType == NODETYPE_DIR)
				FileType = FID_FILE_CHAR_DIRECTORY;  //目录
			else
				FileType = 0;//文件
			LvDVDRecUdf_insert_fid(disc,pspace,fe[Fid[i].SubDirNum],Dir->Name,Fid[i].offset,Fid[i].offset+ChildNum,FileType,pUdfInfo->m_DataMode,Cumulative);//目录为2
			ChildNum++;
		}
		while(Dir = Dir->Next);
		change_data(fe[Fid[i].SubDirNum],suboffset,fentry[Fid[i].SubDirNum]->informationLength); 
		fentry[Fid[i].SubDirNum]->descTag = LvDVDRecUdf_query_tag(disc, pspace, fe[Fid[i].SubDirNum], pUdfInfo->m_DataMode);		
	}
#endif

	for(i = 0;i < MAX_FILE_OR_DIR;i++)
	{
		//查找是否还有子目录
		if(SubDir[i] != NULL)
		{
			if(SubDir[i]->Child != NULL)
				Dir = SubDir[i]->Child;
			else
				continue;
		}
		else
			continue;

		//有则递归子目录下的
		//if((Directory->NodeType != NODETYPE_DIR)&&(Directory != NULL))
		if(Dir != NULL)            //modify by yanming 10.12.21
		{
			suboffset = RecursiveTree(hMem, disc,pspace,Dir,suboffset,pUdfInfo,fe[i]);
			if(suboffset == -1)
				return -1;
		}
		else
		{
			break;
		}
	}

	return suboffset;
}

#if 0
//设定分区盘区
static void setup_partition(struct udf_disc *disc,uint16_t DataMode)
{
	//struct udf_extent *vat, *pspace;

	//pspace = LvDVDRecUdf_next_extent(disc->head, FSD);
	//setup_fileset(disc, pspace,DataMode);
	//setup_root(disc, pspace,DataMode);
}
#endif

// 初始化光盘数据结构
void LvDVDUdf_udf_init_disc(void *hMem, struct udf_disc *disc, uint64_t DisckBlocks, DISC_VOLID_T *pDiscVol)
{
	timestamp	ts;
	struct timeval	tv;
	struct tm 	*tm;
	int		altzone;
	unsigned char unicodename[256];
	int buflen = 256;
	int slen = 0;
	
	memset((void *)disc, 0x00, sizeof(disc));

	disc->blocksize = 2048;
	disc->packetSize = PACKET_BLOCK_16;
	disc->blocksize_bits = 11;
	disc->udf_rev = le16_to_cpu(UDF_Defaults.default_lvidiu->default_lvidiu.minUDFReadRev);  //确定udf版本，默认1.02
	disc->flags = FLAG_UTF8 | FLAG_CLOSED;
	
	//if (disc->udf_rev >= 0x0200)
	//	disc->flags |= FLAG_EFE;  //如果udf版本高于2.0，那么采用扩展文件入口

    //开始计算本地时区
	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);
	altzone = timezone - 3600;
	if (daylight)
		ts.typeAndTimezone = cpu_to_le16(((-altzone/60) & 0x0FFF) | 0x1000);
	else
		ts.typeAndTimezone = cpu_to_le16(((-timezone/60) & 0x0FFF) | 0x1000);
	ts.year = cpu_to_le16(1900 + tm->tm_year);
	ts.month = 1 + tm->tm_mon;
	ts.day = tm->tm_mday;
	ts.hour = tm->tm_hour;
	ts.minute = tm->tm_min;
	ts.second = tm->tm_sec;
	ts.centiseconds = tv.tv_usec / 10000;
	ts.hundredsOfMicroseconds = (tv.tv_usec - ts.centiseconds * 10000) / 100;
	ts.microseconds = tv.tv_usec - ts.centiseconds * 10000 - ts.hundredsOfMicroseconds * 100;

	//分配,初始化主卷标描述符PVD---0
	MEMMOCLINE;
	disc->udf_pvd[0] = (primaryVolDesc*)MEMMALLOC(hMem, sizeof(struct primaryVolDesc));
	memcpy((void *)disc->udf_pvd[0], UDF_Defaults.default_pvd, sizeof(struct primaryVolDesc));
	memcpy((void *)&disc->udf_pvd[0]->recordingDateAndTime, &ts, sizeof(timestamp));
	sprintf((char *)&disc->udf_pvd[0]->volSetIdent[1], "%08lx%s",
	        mktime(tm), &disc->udf_pvd[0]->volSetIdent[9]);
	disc->udf_pvd[0]->volSetIdent[127] = strlen((char *)disc->udf_pvd[0]->volSetIdent);
	//modify by yanming for utf-8
	DVDRec_Utf8ToUnicode((unsigned char * )pDiscVol->volIdent, unicodename, buflen, &slen);
	memcpy(disc->udf_pvd[0]->volIdent,unicodename,32);
	//LvDVDRecUdf_EncUnicodeStr(pDiscVol->volIdent, disc->udf_pvd[0]->volIdent, strlen(pDiscVol->volIdent), 32);	
	
    //分配初始化实施使用描述符LUVD---1
	MEMMOCLINE;
	disc->udf_iuvd[0] = (impUseVolDesc *)MEMMALLOC(hMem, sizeof(struct impUseVolDesc) + sizeof(struct impUseVolDescImpUse));
	memcpy(disc->udf_iuvd[0], UDF_Defaults.default_iuvd, sizeof(struct impUseVolDesc));
	memcpy(query_iuvdiu(disc), UDF_Defaults.default_iuvdiu, sizeof(struct impUseVolDescImpUse));
	//modify by yanming for utf-8
	DVDRec_Utf8ToUnicode((unsigned char * )pDiscVol->logicalVolIdent, unicodename, buflen, &slen);
	memcpy(query_iuvdiu(disc)->logicalVolIdent,unicodename,128);
	//LvDVDRecUdf_EncUnicodeStr(pDiscVol->logicalVolIdent, query_iuvdiu(disc)->logicalVolIdent, strlen(pDiscVol->logicalVolIdent),128);
	DVDRec_Utf8ToUnicode((unsigned char * )pDiscVol->LVInfoTitle, unicodename, buflen, &slen);
	memcpy(query_iuvdiu(disc)->LVInfo1,unicodename,36);	
	//LvDVDRecUdf_EncUnicodeStr(pDiscVol->LVInfoTitle,query_iuvdiu(disc)->LVInfo1,strlen(pDiscVol->LVInfoTitle),36);
	DVDRec_Utf8ToUnicode((unsigned char * )pDiscVol->LVInfoDataTime, unicodename, buflen, &slen);
	memcpy(query_iuvdiu(disc)->LVInfo2,unicodename,36);		
	//LvDVDRecUdf_EncUnicodeStr(pDiscVol->LVInfoDataTime,query_iuvdiu(disc)->LVInfo2,strlen(pDiscVol->LVInfoDataTime),36);
	DVDRec_Utf8ToUnicode((unsigned char * )pDiscVol->LVInfoEmail, unicodename, buflen, &slen);
	memcpy(query_iuvdiu(disc)->LVInfo3,unicodename,36);		
	//LvDVDRecUdf_EncUnicodeStr(pDiscVol->LVInfoEmail,query_iuvdiu(disc)->LVInfo3,strlen(pDiscVol->LVInfoEmail),36);
	//query_iuvdiu(disc)->logicalVolIdent[127] = strlen(query_iuvdiu(disc)->logicalVolIdent);
	//query_iuvdiu(disc)->LVInfo1[35] = strlen(query_iuvdiu(disc)->LVInfo1);
	//query_iuvdiu(disc)->LVInfo2[35] = strlen(query_iuvdiu(disc)->LVInfo2);
	//query_iuvdiu(disc)->LVInfo3[35] = strlen(query_iuvdiu(disc)->LVInfo3);
	
    //分配，初始化逻辑卷标描述LVD---2
	MEMMOCLINE;
	disc->udf_lvd[0] = (logicalVolDesc *)MEMMALLOC(hMem, sizeof(struct logicalVolDesc));
	memcpy(disc->udf_lvd[0], UDF_Defaults.default_lvd, sizeof(struct logicalVolDesc));
	//disc->udf_lvd[0]->logicalVolIdent[127] = strlen(disc->udf_lvd[0]->logicalVolIdent);
	//modify by yanming for utf-8
	DVDRec_Utf8ToUnicode((unsigned char * )pDiscVol->logicalVolIdent, unicodename, buflen, &slen);
	memcpy(disc->udf_lvd[0]->logicalVolIdent,unicodename,128);		
	//LvDVDRecUdf_EncUnicodeStr(pDiscVol->logicalVolIdent, disc->udf_lvd[0]->logicalVolIdent, strlen(pDiscVol->logicalVolIdent),128);
	disc->udf_lvd[0]->mapTableLength   = 0;
	disc->udf_lvd[0]->numPartitionMaps = 0;

    //分配初始化分区描述PD---3
	MEMMOCLINE;
	disc->udf_pd[0] = (struct partitionDesc *)MEMMALLOC(hMem, sizeof(struct partitionDesc));
	memcpy(disc->udf_pd[0], UDF_Defaults.default_pd, sizeof(struct partitionDesc));

    //分配初始化空白空间描述USD---4
	MEMMOCLINE;
	disc->udf_usd[0] = (struct unallocSpaceDesc *)MEMMALLOC(hMem, sizeof(struct unallocSpaceDesc));
	memcpy(disc->udf_usd[0], UDF_Defaults.default_usd, sizeof(struct unallocSpaceDesc));

    //分配初始化结束描述符TD
	MEMMOCLINE;
	disc->udf_td[0] = (struct terminatingDesc *)MEMMALLOC(hMem, sizeof(struct terminatingDesc));
	memcpy(disc->udf_td[0], UDF_Defaults.default_td, sizeof(struct terminatingDesc));

    //分配初始化逻辑卷标完整性描述LVID
	MEMMOCLINE;
	disc->udf_lvid = (struct logicalVolIntegrityDesc *)MEMMALLOC(hMem, sizeof(struct logicalVolIntegrityDesc) + sizeof(struct logicalVolIntegrityDescImpUse));
	memcpy(disc->udf_lvid, UDF_Defaults.default_lvid, sizeof(struct logicalVolIntegrityDesc));
	memcpy(&disc->udf_lvid->recordingDateAndTime, &ts, sizeof(timestamp));
	memcpy(query_lvidiu(disc), UDF_Defaults.default_lvidiu, sizeof(struct logicalVolIntegrityDescImpUse));

	//disc->udf_stable[0] = MEMMALLOC(hMem, sizeof(struct sparingTable));
	//memcpy(disc->udf_stable[0], &default_stable, sizeof(struct sparingTable));

	//disc->vat = MEMMALLOC(hMem, disc->blocksize);
	//disc->vat_entries = 0;

    //分配初始化文件集描述符FSD
	MEMMOCLINE;
	disc->udf_fsd = (struct fileSetDesc *)MEMMALLOC(hMem, sizeof(struct fileSetDesc));
	memcpy(disc->udf_fsd, UDF_Defaults.default_fsd, sizeof(struct fileSetDesc));
	memcpy(&disc->udf_fsd->recordingDateAndTime, &ts, sizeof(timestamp));
	//modify by yanming for utf-8
	DVDRec_Utf8ToUnicode((unsigned char * )pDiscVol->logicalVolIdent, unicodename, buflen, &slen);
	memcpy(disc->udf_fsd->logicalVolIdent,unicodename,128);			
	//LvDVDRecUdf_EncUnicodeStr(pDiscVol->logicalVolIdent, disc->udf_fsd->logicalVolIdent, strlen(pDiscVol->logicalVolIdent),128);
	DVDRec_Utf8ToUnicode((unsigned char * )pDiscVol->fileSetIdent, unicodename, buflen, &slen);
	memcpy(disc->udf_fsd->fileSetIdent,unicodename,32);		
	//LvDVDRecUdf_EncUnicodeStr(pDiscVol->fileSetIdent, disc->udf_fsd->fileSetIdent,strlen(pDiscVol->fileSetIdent),32);
	DVDRec_Utf8ToUnicode((unsigned char * )pDiscVol->copyrightFileIdent, unicodename, buflen, &slen);
	memcpy(disc->udf_fsd->copyrightFileIdent,unicodename,32);		
	//LvDVDRecUdf_EncUnicodeStr(pDiscVol->copyrightFileIdent, disc->udf_fsd->copyrightFileIdent, strlen(pDiscVol->copyrightFileIdent),32);
	DVDRec_Utf8ToUnicode((unsigned char * )pDiscVol->abstractFileIdent, unicodename, buflen, &slen);
	memcpy(disc->udf_fsd->abstractFileIdent,unicodename,32);	
	//LvDVDRecUdf_EncUnicodeStr(pDiscVol->abstractFileIdent, disc->udf_fsd->abstractFileIdent, strlen(pDiscVol->abstractFileIdent),32);
	//disc->udf_fsd->logicalVolIdent[127] = strlen(disc->udf_fsd->logicalVolIdent);
	//disc->udf_fsd->fileSetIdent[31] = strlen(disc->udf_fsd->fileSetIdent);
	//disc->udf_fsd->copyrightFileIdent[31] = strlen(disc->udf_fsd->copyrightFileIdent);
	//disc->udf_fsd->abstractFileIdent[31] = strlen(disc->udf_fsd->abstractFileIdent);

	//添加物理分区
	add_type1_partition(hMem, disc, 0);

    //分配初始化扩展盘区，环形结构
	MEMMOCLINE;
	disc->head = (struct udf_extent *)MEMMALLOC(hMem, sizeof(struct udf_extent));
	memset(disc->head, 0, sizeof(struct udf_extent));
	disc->tail = disc->head;

    //初始化盘区
	disc->head->space_type = USPACE;
	disc->head->start  = 0;	
	DP(("udf_init_disc DisckBlocks=%lld\n",DisckBlocks));
	disc->blocks = DisckBlocks;	 //默认光驱扇区块数量，可以通过光盘信息获取
	disc->head->blocks = disc->blocks; 
	disc->head->next = NULL;
	disc->head->prev = NULL;
}

void LvDVDUdf_split_space(void *hMem, struct udf_disc *disc)
{
	uint32_t blocks = disc->head->blocks;
	uint32_t start, size;
	int i, j;

    //一号轨道数据
	start = UDF_VRS_ADDR;
	DVDRecUdf_set_extent(hMem, disc, RESERVED, 0, disc->packetSize); //设定保留区16扇区，16个块
	
	DVDRecUdf_set_extent(hMem, disc, VRS, start, disc->packetSize); //设定VRS盘区，16个块

	start += disc->packetSize;
	
	DVDRecUdf_set_extent(hMem, disc, PVDS, start, disc->packetSize);	

	start += disc->packetSize;
	DVDRecUdf_set_extent(hMem, disc, RVDS, start, disc->packetSize);		

	start += disc->packetSize;
	DVDRecUdf_set_extent(hMem, disc, LVID, start, disc->packetSize ); //设定临时文件区

	start = UDF_ADVP_ADDR;
	DVDRecUdf_set_extent(hMem, disc, ANCHOR, UDF_ADVP_ADDR, disc->packetSize * 2); //设定AVDP盘区，两个包

	//二号轨道数据
    start = UDF_FSD_ADDR;
	DVDRecUdf_set_extent(hMem, disc, FSD, start,UDF_FSD_LEN); 

	start = UDF_FS_LENGTH + 16; //最新起点,1040
	
	size = blocks - start - 512 ;   //空白块
	DVDRecUdf_set_extent(hMem, disc, VOB, start,size - disc->packetSize * 16); //全部分给分区描述符

	if (disc->flags & FLAG_CLOSED)
		DVDRecUdf_set_extent(hMem, disc, ANCHOR, blocks - 512 - disc->packetSize * 16 , 512); //设定封盘区,AVDP位置N - 512 开始，连续512个AVDP, 
	
	for (i=0; i<disc->udf_lvd[0]->numPartitionMaps; i++)
	{
		//DP(("disc->udf_lvid : %d, freetabl:%d\n", (int)disc->udf_lvid, (int)disc->udf_lvid->freeSpaceTable[i] ));
		if (i == 1)
		{
			disc->udf_lvid->freeSpaceTable[i] = cpu_to_le32(0xFFFFFFFF);
		}
		else
		{
			disc->udf_lvid->freeSpaceTable[i] = cpu_to_le32(size);
		}
	}
	for (j=0; j<disc->udf_lvd[0]->numPartitionMaps; j++)
	{
		if (j == 1)
			disc->udf_lvid->sizeTable[i+j] = cpu_to_le32(0xFFFFFFFF);
		else
			disc->udf_lvid->sizeTable[i+j] = cpu_to_le32(size);
	}

}

//设定文件描述集,在分区盘区
int LvDVDUdf_setup_fileset(void *hMem, struct udf_disc *disc, struct udf_extent *pspace, uint16_t DataMode)
{
	uint32_t offset = 0;
	struct udf_desc *desc;
	int length = sizeof(struct fileSetDesc);

	offset = 0; //udf_alloc_blocks(disc, pspace, offset, 1);

	((uint16_t *)disc->udf_fsd->domainIdent.identSuffix)[0] = cpu_to_le16(disc->udf_rev);
	
	desc = DVDRecUdf_set_desc(hMem, disc, pspace, TAG_IDENT_FSD, offset, 0, NULL);
	
	desc->length = desc->data->length = length;
	
	desc->data->buffer = disc->udf_fsd;
	
	disc->udf_fsd->descTag = DVDRecUdf_query_tag(disc, pspace, desc, DataMode);

	return (length + disc->blocksize - 1) >> disc->blocksize_bits;  //实现块对齐
}

//设定根文件系统
int LvDVDUdf_setup_root(void *hMem, struct udf_disc *disc, struct udf_extent *pspace, udfinfo_t *pUdfInfo)
{
	uint32_t offset,FileType,ChildNum,FidLen;
	uint32_t SecNum = 0;
	uint8_t DirCount;
	//uint8_t tempunicode[256];
	uint32_t Cumulative;
	uint32_t ret = 0;
	uint32_t ChildFid = 0;
	struct udf_desc *desc, *fsd_desc;
	struct fileEntry *fentry;
	FILEDIRTREE * FileDirTree;
	FILDIRNODE *Directory;
	unsigned char UnicodeName[256];
	int buflen = 256;
	int slen = 0;

	FileType = 0;
	ChildNum = 0;
	FileDirTree = pUdfInfo->m_FileDirTree;

	DP(("setup_root 1\n"));
	//DirCount = pUdfInfo->udf_cmd->getdircount(pUdfInfo->m_FileDirTree);
	DirCount = pUdfInfo->udfCmd.GetDirCount(pUdfInfo->m_FileDirTree);

	if(pUdfInfo->m_DataMode == UDFDATAMODE_VIDEO)
		offset = 2;
	else
		offset = 1;
	DP(("setup_root 2\n"));
	if (disc->flags & FLAG_STRATEGY4096)
		disc->udf_fsd->rootDirectoryICB.extLength = cpu_to_le32(disc->blocksize * 2);
	else
		disc->udf_fsd->rootDirectoryICB.extLength = cpu_to_le32(disc->blocksize);
	
	DP(("setup_root 3\n"));
	if(pUdfInfo->m_DataMode == UDFDATAMODE_DATA)
		disc->udf_fsd->rootDirectoryICB.extLocation.logicalBlockNum = cpu_to_le32(1);//cpu_to_le32(offset);yan
	else
		disc->udf_fsd->rootDirectoryICB.extLocation.logicalBlockNum = cpu_to_le32(2);//cpu_to_le32(offset);yan
	
	DP(("setup_root 4\n"));
	disc->udf_fsd->rootDirectoryICB.extLocation.partitionReferenceNum = cpu_to_le16(0);
	fsd_desc = DVDRecUdf_next_desc(pspace->head, TAG_IDENT_FSD);
	disc->udf_fsd->descTag = DVDRecUdf_query_tag(disc, pspace, fsd_desc, pUdfInfo->m_DataMode);
	
	DP(("setup_root 5\n"));
	Directory = FileDirTree->FirstNode->Child; //modify by yanming for get file time

	//首先建立根目录
	desc = DVDRecUdf_udf_create(hMem, disc, pspace, offset, NULL,//"",offset,NULL,
		               DirCount, &Directory->ts, ICBTAG_FILE_TYPE_DIRECTORY, 0, pUdfInfo->m_DataMode, 0);
	
	fentry = (struct fileEntry *)desc->data->buffer;
	
	Cumulative = PAD(sizeof(struct fileIdentDesc), 4);	//数据和视频的长度不同		
	
	DP(("setup_root 6\n"));
	//根据树结构确定根FID内容
	SecNum = DVDRecUdf_insert_fid(hMem, disc, pspace, desc, NULL, offset+1, offset,
			   FID_FILE_CHAR_DIRECTORY | FID_FILE_CHAR_PARENT, pUdfInfo->m_DataMode, Cumulative);
	if(SecNum == -1)
		return 0;	
	offset++;
	
	if((CalcNeedFidSize(Directory,pUdfInfo->m_DataMode)) > 2048)
	{
		ChildFid += (((CalcNeedFidSize(Directory,pUdfInfo->m_DataMode))/2048)+
					((((CalcNeedFidSize(Directory,pUdfInfo->m_DataMode))%2048) > 0) ? 1:0));
		ChildFid--; 		//多加一个
	}	

	while(Directory)
	{
		if(Directory->NodeType == NODETYPE_DIR)
			FileType = FID_FILE_CHAR_DIRECTORY;  //目录
		else
			FileType = 0;//文件
		ChildNum++;

		//get unicode
		DVDRec_Utf8ToUnicode((unsigned char *)Directory->Name, UnicodeName, buflen, &slen);
		DP(("setup_root 7\n"));
		if(UDFDATAMODE_VIDEO == pUdfInfo->m_DataMode)
			Cumulative = PAD(sizeof(struct fileIdentDesc) + strlen((char *)Directory->Name), 4);  //根据目录的unicode的数据长度获得一个文件数据块的长度(例如:VIDEO_TS.VOB)
		else
		{
			DP(("setup_root 8\n"));
			FidLen = slen;//strlen((char *)Directory->Name);//modify by yanming for utf-8
			//FidLen = LvDVDRecUdf_EncUnicodeStr((uint8_t *)Directory->Name,
			//                  tempunicode, strlen((char *)Directory->Name), strlen((char *)Directory->Name)+2) + 1;
			Cumulative = PAD(sizeof(struct fileIdentDesc) + FidLen,4);	//4//4字节对齐		
		}

		DP(("setup_root 9\n"));
		SecNum = DVDRecUdf_insert_fid(hMem, disc, pspace, desc, 
					(uint8_t *)Directory->Name, offset, (offset + ChildNum + ChildFid), 
					FileType, pUdfInfo->m_DataMode, Cumulative);//目录为2
		if(SecNum == -1)
			return 0;			
		Directory = Directory->Next;
	};
	
	DP(("setup_root 10\n"));
	//根据类型填写根目录FE的扩展数据(数据位置 长度)
	change_data(desc,offset,fentry->informationLength);	

	fentry->descTag = DVDRecUdf_query_tag(disc, pspace, desc, pUdfInfo->m_DataMode);
	
	DP(("setup_root 11\n"));
	if(SecNum)
		offset += SecNum;
	else
		offset++;	
	//目录递归调用处理子目录文件
	ret = RecursiveTree(hMem, disc, pspace, FileDirTree->FirstNode->Child, offset, pUdfInfo, desc);
	if(ret == -1)
		return -1;
	return 0;
}

//设定VRS盘区，其第一个扇区要在16位置后即，17个扇区位置,可确定该光盘是否UDF和其版本号
void LvDVDUdf_setup_vrs(void *hMem, struct udf_disc *disc)
{
	struct udf_extent *ext;
	struct udf_desc *desc;

	if (!(ext = DVDRecUdf_next_extent(disc->head, VRS))) return;
	
	//设定第一个VRS结构描述，在16+1 位置
	desc = DVDRecUdf_set_desc(hMem, disc, ext, 0x00, 0, sizeof(struct volStructDesc), NULL);//申请空间并加入到链表中
	disc->udf_vrs[0] = (struct volStructDesc *)desc->data->buffer;
	disc->udf_vrs[0]->structType    = 0x00;
	disc->udf_vrs[0]->structVersion = 0x01;
	memcpy(disc->udf_vrs[0]->stdIdent, VSD_STD_ID_BEA01, VSD_STD_ID_LEN);

	//设定第二个VRS结构描述，在16+2 位置
	if (disc->blocksize >= 2048)
		desc = DVDRecUdf_set_desc(hMem, disc, ext, 0x00, 1, sizeof(struct volStructDesc), NULL);
	else
		desc = DVDRecUdf_set_desc(hMem, disc, ext, 0x00, 2048 / disc->blocksize, sizeof(struct volStructDesc), NULL);
	disc->udf_vrs[1] = (struct volStructDesc *)desc->data->buffer;
	disc->udf_vrs[1]->structType    = 0x00;
	disc->udf_vrs[1]->structVersion = 0x01;
	memcpy(disc->udf_vrs[1]->stdIdent, disc->udf_rev >= 0x0200 ? VSD_STD_ID_NSR03 : VSD_STD_ID_NSR02, VSD_STD_ID_LEN);

	//设定第三个VRS结构描述，在16+3 位置
	if (disc->blocksize >= 2048)
	{
		desc = DVDRecUdf_set_desc(hMem, disc, ext, 0x00, 2, sizeof(struct volStructDesc), NULL);
	}
	else
	{
		desc = DVDRecUdf_set_desc(hMem, disc, ext, 0x00, 4096 / disc->blocksize, sizeof(struct volStructDesc), NULL);
	}
	disc->udf_vrs[2] = (struct volStructDesc *)desc->data->buffer;
	disc->udf_vrs[2]->structType    = 0x00;
	disc->udf_vrs[2]->structVersion = 0x01;
	memcpy(disc->udf_vrs[2]->stdIdent, VSD_STD_ID_TEA01, VSD_STD_ID_LEN);
}

//设定AVDP盘区，在256，N-256 或N -1 位置
void LvDVDUdf_setup_anchor(void *hMem, struct udf_disc *disc,int DataMode)
{
	struct udf_extent *ext;
	uint32_t mloc, rloc, mlen, rlen;
	int i = 0;

	ext = DVDRecUdf_next_extent(disc->head, PVDS);
	mloc = ext->start;
	mlen = ext->blocks << disc->blocksize_bits;

	ext = DVDRecUdf_next_extent(disc->head, RVDS);
	rloc = ext->start;
	rlen = ext->blocks << disc->blocksize_bits;

	ext = DVDRecUdf_next_extent(disc->head, ANCHOR);
	do
	{
		MEMMOCLINE;
		ext->head = ext->tail = (struct udf_desc*)MEMMALLOC(hMem, sizeof(struct udf_desc));
		memset(ext->tail, 0, sizeof(struct udf_desc));

		MEMMOCLINE;
		ext->head->data = (struct udf_data*)MEMMALLOC(hMem, sizeof(struct udf_data));
		memset(ext->head->data, 0, sizeof(struct udf_data));
		ext->head->data->next = ext->head->data->prev = NULL;
		ext->head->ident = TAG_IDENT_AVDP;
		ext->head->offset = 0;
		ext->head->length = ext->head->data->length = sizeof(struct anchorVolDescPtr);

		MEMMOCLINE;
		ext->head->data->buffer = MEMMALLOC(hMem, sizeof(struct anchorVolDescPtr));
		disc->udf_anchor[i] = (struct anchorVolDescPtr*)ext->head->data->buffer;
		memset(disc->udf_anchor[i], 0, sizeof(struct anchorVolDescPtr));
		ext->head->next = ext->head->prev = NULL;
		disc->udf_anchor[i]->mainVolDescSeqExt.extLocation = cpu_to_le32(mloc);
		disc->udf_anchor[i]->mainVolDescSeqExt.extLength = cpu_to_le32(mlen);
		disc->udf_anchor[i]->reserveVolDescSeqExt.extLocation = cpu_to_le32(rloc);
		disc->udf_anchor[i]->reserveVolDescSeqExt.extLength = cpu_to_le32(rlen);
		disc->udf_anchor[i]->descTag = DVDRecUdf_query_tag(disc, ext, ext->head, DataMode);
		ext = DVDRecUdf_next_extent(ext->next, ANCHOR);
	} while (i++, ext != NULL);
}

//设定VDS,主卷描述序列包括PVD,IUVD,LVD,PD,USD,TD
void LvDVDUdf_setup_vds(void *hMem, struct udf_disc *disc,uint16_t DataMode, udfinfo_t *pUdfInfo)
{
	struct udf_extent *pvds, *rvds, *lvid;

	pvds = DVDRecUdf_next_extent(disc->head, PVDS); //主卷描述序列，包括PVD,IUVD,LVD,PD,USD,TD

	rvds = DVDRecUdf_next_extent(disc->head, RVDS);
	lvid = DVDRecUdf_next_extent(disc->head, LVID);
	setup_pvd(hMem, disc, pvds, rvds, 0,DataMode);
	setup_iuvd(hMem, disc, pvds, rvds, 1,DataMode);
	setup_pd(hMem, disc, pvds, rvds, 2,DataMode, pUdfInfo);
	setup_lvd(hMem, disc, pvds, rvds, lvid, 3,DataMode);
	setup_usd(hMem, disc, pvds, rvds, 4,DataMode);
	setup_td(hMem, disc, pvds, rvds, 5,DataMode);
	setup_lvid(hMem, disc, lvid,DataMode);
}

static void Freeudf_data(void *hMem, struct udf_data *pData)
{
	struct udf_data *pTmp;
	while(pData)
	{
		if(pData->buffer)
		{
			if(pData->bMalloc)
			{
				MEMFREE(hMem, pData->buffer);
			}
		}
		pTmp = pData->next;
		MEMFREE(hMem, pData);
		pData = pTmp;
	}
}

static void Freeudf_desc(void *hMem, struct udf_desc *pdesc)
{
	struct udf_desc *pTmp;
	while(pdesc)
	{
		Freeudf_data(hMem, pdesc->data);
		pTmp = pdesc->next;
		MEMFREE(hMem, pdesc);
		pdesc = pTmp;
	}
}

static void FreeUdf_extent(void *hMem, struct udf_extent *pExt)
{
	struct udf_extent *pTmp;
	while(pExt)
	{
		//printf("FreeUdf_extent 1\n");
		Freeudf_desc(hMem, pExt->head);
		pTmp = pExt->next;
		MEMFREE(hMem, pExt);
		pExt = pTmp;
	}
	//printf("FreeUdf_extent OK\n");
}

void LvDVDUdf_freeUdfdisc(void *hMem, struct udf_disc * disc)
{
	int i;
	struct udf_extent *ext;
	if(!hMem || !disc )
		return;
	
	//printf("LvDVDUdf_freeUdfdisc 0\n");
	ext = disc->head;
	
	//printf("LvDVDUdf_freeUdfdisc 1\n");
	FreeUdf_extent(hMem, ext);

	for(i=0; i< 3; i++)
	{
		//printf("LvDVDUdf_freeUdfdisc 2\n");
		if( disc->udf_anchor[i] ) MEMFREE(hMem, disc->udf_anchor[i]);
		disc->udf_anchor[i] = NULL;
	}
	
	//printf("LvDVDUdf_freeUdfdisc 3\n");
	if( disc->udf_pvd[0] ) MEMFREE(hMem, disc->udf_pvd[0]); disc->udf_pvd[0] = NULL;
	
	if( disc->udf_lvd[0] ) MEMFREE(hMem, disc->udf_lvd[0]); disc->udf_lvd[0] = NULL;
	
	if( disc->udf_pd[0] ) MEMFREE(hMem, disc->udf_pd[0]); disc->udf_pd[0] = NULL;
	
	if( disc->udf_usd[0] ) MEMFREE(hMem, disc->udf_usd[0]); disc->udf_usd[0] = NULL;

	if( disc->udf_iuvd[0] ) MEMFREE(hMem, disc->udf_iuvd[0]); disc->udf_iuvd[0] = NULL;

	if( disc->udf_td[0] ) MEMFREE(hMem, disc->udf_td[0]); disc->udf_td[0] = NULL;

	if( disc->udf_lvid ) MEMFREE(hMem, disc->udf_lvid); disc->udf_lvid = NULL;

	if( disc->udf_fsd )  MEMFREE(hMem, disc->udf_fsd);  disc->udf_fsd  = NULL;
	//printf("LvDVDUdf_freeUdfdisc OK\n");
}

//为FE插入描述符，指定FE分配描述的字段
int LvDVDUdf_insert_desc(void *hMem, struct udf_desc *desc)
{
	struct fileEntry *fe;
	fe = (struct fileEntry *)desc->data->buffer;

	if (le32_to_cpu(fe->lengthAllocDescs) == 0)
	{
		if ((le16_to_cpu(fe->icbTag.flags) & ICBTAG_FLAG_AD_MASK) == ICBTAG_FLAG_AD_SHORT)
		{
			desc->length += sizeof(short_ad);
			desc->data->length += sizeof(short_ad);
			desc->data->buffer = MEMREALLOC(hMem, desc->data->buffer, desc->length);
		}
		else if ((le16_to_cpu(fe->icbTag.flags) & ICBTAG_FLAG_AD_MASK) == ICBTAG_FLAG_AD_LONG)
		{
			desc->length += sizeof(long_ad);
			desc->data->length += sizeof(long_ad);		
			desc->data->buffer = MEMREALLOC(hMem, desc->data->buffer, desc->length);
		}
	}

	return 0;
}
