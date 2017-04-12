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
#include "libudffs.h"
#include "debug.h"

//下一个盘区的指针  找到fsd
struct udf_extent *DVDRecUdf_next_extent(struct udf_extent *start_ext, enum euUDF_SPACE_TYPE type)
{
	while (start_ext != NULL && !(start_ext->euSpaceType & type))
	{
		start_ext = start_ext->pNext;
		//usleep(2000);
	}

	return start_ext;
}

#if 0
//下一个盘区的大小
static uint32_t next_extent_size(struct udf_extent *start_ext, enum udf_space_type type, uint32_t blocks, uint32_t offset)
{
	uint32_t inc;
	start_ext = LvDVDRecUdf_next_extent(start_ext, type);
cont:
	while (start_ext != NULL && start_ext->blocks < blocks)
		start_ext = LvDVDRecUdf_next_extent(start_ext->next, type);

	if (start_ext->start % offset)
	{
		inc = offset - (start_ext->start % offset);
		if (start_ext->blocks - inc < blocks)
		{
			start_ext = LvDVDRecUdf_next_extent(start_ext->next, type);
			goto cont;
		}
	}
	else
		inc = 0;

	return start_ext->start + inc;
}
#endif

#if 0
//前一个盘区指针
static struct udf_extent *prev_extent(struct udf_extent *start_ext, enum udf_space_type type)
{
	while (start_ext != NULL && !(start_ext->space_type & type))
		start_ext = start_ext->prev;

	return start_ext;
}


//前一个盘区的大小
static uint32_t prev_extent_size(struct udf_extent *start_ext, enum udf_space_type type, uint32_t blocks, uint32_t offset)
{
	uint32_t dec;
	start_ext = prev_extent(start_ext, type);
cont:
	while (start_ext != NULL && start_ext->blocks < blocks)
		start_ext = prev_extent(start_ext->prev, type);

	if ((start_ext->start + start_ext->blocks) % offset)
	{
		dec = (start_ext->start + start_ext->blocks) % offset;
		if (start_ext->blocks - dec < blocks)
		{
			start_ext = LvDVDRecUdf_next_extent(start_ext->next, type);
			goto cont;
		}
	}
	else
		dec = 0;

	return start_ext->start + start_ext->blocks - dec - blocks;
}
#endif

//寻找盘区，按起始逻辑块
static struct udf_extent *find_extent(struct udf_disc *disc, uint32_t start)
{
	struct udf_extent *start_ext;

	start_ext = disc->pHead;

	while (start_ext->pNext != NULL)
	{
		if (start_ext->nStart + start_ext->nBlocks > start)
			break;
		start_ext = start_ext->pNext;
	}
	return start_ext;
}

//设定盘区，用链表记录每个盘区的起始逻辑块和占用空间
struct udf_extent *DVDRecUdf_set_extent(void *hMem, struct udf_disc *disc, enum euUDF_SPACE_TYPE type, uint32_t start, uint32_t blocks)
{
	struct udf_extent *start_ext, *new_ext;

	start_ext = find_extent(disc, start);
	if (start == start_ext->nStart)
	{
		if (blocks == start_ext->nBlocks)              //block == 16
		{
			start_ext->euSpaceType = type;

			return start_ext;
		}//         16               2297888
		else if (blocks < start_ext->nBlocks)
		{
			MEMMOCLINE;
			new_ext = (udf_extent*)MEMMALLOC(hMem, sizeof(struct udf_extent));
			memset(new_ext, 0, sizeof(struct udf_extent));
			new_ext->euSpaceType = type;
			new_ext->nStart = start;
			new_ext->nBlocks = blocks;
			new_ext->pHead = new_ext->pTail = NULL;
			new_ext->pPrev = start_ext->pPrev;
			if (new_ext->pPrev)
				new_ext->pPrev->pNext = new_ext;
			new_ext->pNext = start_ext;
			if (disc->pHead == start_ext)
				disc->pHead = new_ext;

			start_ext->nStart += blocks;
			start_ext->nBlocks -= blocks;
			start_ext->pPrev = new_ext;

			return new_ext;
		}
		else /* blocks > start_ext->blocks */
		{
			DPERROR(("trying to change type of multiple extents\n"));
			
			// 
			//exit(1);
			
			start_ext->euSpaceType = type;

			return start_ext;
		}
	}
	else /* start > start_ext->start */
	{
		if (start + blocks == start_ext->nStart + start_ext->nBlocks)
		{
			MEMMOCLINE;
			new_ext = (udf_extent*)MEMMALLOC(hMem, sizeof(struct udf_extent));
			memset(new_ext, 0, sizeof(struct udf_extent));
			new_ext->euSpaceType = type;
			new_ext->nStart = start;
			new_ext->nBlocks = blocks;
			new_ext->pHead = new_ext->pTail = NULL;
			new_ext->pPrev = start_ext;
			new_ext->pNext = start_ext->pNext;
			if (new_ext->pNext)
				new_ext->pNext->pPrev = new_ext;
			if (disc->pTail == start_ext)
				disc->pTail = new_ext;

			start_ext->nBlocks -= blocks;
			start_ext->pNext = new_ext;

			return new_ext;
		}
		else if (start + blocks < start_ext->nStart + start_ext->nBlocks)
		{
			MEMMOCLINE;
			new_ext = (udf_extent*)MEMMALLOC(hMem, sizeof(struct udf_extent));
			memset(new_ext, 0, sizeof(struct udf_extent));
			new_ext->euSpaceType = type;
			new_ext->nStart = start;
			new_ext->nBlocks = blocks;
			new_ext->pHead = new_ext->pTail = NULL;
			new_ext->pPrev = start_ext;
			
			MEMMOCLINE;
			new_ext->pNext = (udf_extent*)MEMMALLOC(hMem, sizeof(struct udf_extent));
			memset(new_ext->pNext, 0, sizeof(struct udf_extent));
			new_ext->pNext->pPrev = new_ext;
			new_ext->pNext->euSpaceType = start_ext->euSpaceType;
			new_ext->pNext->nStart = start + blocks;
			new_ext->pNext->nBlocks = start_ext->nBlocks - blocks - start + start_ext->nStart;
			new_ext->pNext->pHead = new_ext->pNext->pTail = NULL;
			new_ext->pNext->pNext = start_ext->pNext;
			if (new_ext->pNext->pNext)
				new_ext->pNext->pNext->pPrev = new_ext->pNext;
			if (disc->pTail == start_ext)
				disc->pTail = new_ext->pNext;

			start_ext->nBlocks = start - start_ext->nStart;
			start_ext->pNext = new_ext;

			return new_ext;
		}
		else /* start + blocks > start_ext->start + start_ext->blocks */
		{
			MEMMOCLINE;
			new_ext = (udf_extent*)MEMMALLOC(hMem, sizeof(struct udf_extent));
			memset(new_ext, 0, sizeof(struct udf_extent));
			new_ext->euSpaceType = type;
			new_ext->nStart = start;
			new_ext->nBlocks = blocks;
			new_ext->pHead = new_ext->pTail = NULL;
			new_ext->pPrev = start_ext;
			new_ext->pNext = start_ext->pNext;
			if (new_ext->pNext)
				new_ext->pNext->pPrev = new_ext;
			if (disc->pTail == start_ext)
				disc->pTail = new_ext;

			start_ext->nBlocks -= blocks;
			start_ext->pNext = new_ext;

			return new_ext;
		}
	}
}

//下一个描述符指针
struct udf_desc *DVDRecUdf_next_desc(struct udf_desc *start_desc, uint16_t ident)
{
	while (start_desc != NULL && start_desc->ident != ident)
		start_desc = start_desc->pNext;

	return start_desc;
}

//寻找描述符
struct udf_desc *DVDRecUdf_find_desc(struct udf_extent *ext, uint32_t offset)
{
	struct udf_desc *start_desc;

	start_desc = ext->pHead;

	while (start_desc->pNext != NULL)
	{
		if (start_desc->offset == offset)
			return start_desc;
		else if ((start_desc->offset) > offset)
		{	
			return start_desc->pPrev;
		}
		else
			start_desc = start_desc->pNext;
	}
	return start_desc;
}

//设定描述符，在指定位置，分配描述符数据块
struct udf_desc *DVDRecUdf_set_desc(void *hMem, struct udf_disc *disc, struct udf_extent *ext, 
	                      uint16_t ident, uint32_t offset, uint32_t length, 
	                      struct udf_data *data)
{
	struct udf_desc *start_desc, *new_desc;

	MEMMOCLINE;
	new_desc = (udf_desc*)MEMMALLOC(hMem, sizeof(struct udf_desc));
	memset(new_desc, 0, sizeof(struct udf_desc));
	new_desc->ident = ident;
	new_desc->offset = offset;
	new_desc->length = length;
	if (data == NULL)
	{
		new_desc->pData = DVDRecUdf_alloc_data(hMem, length);
	}
	else
	{
		new_desc->pData = data;
	}
	if (ext->pHead == NULL)
	{
		ext->pHead = ext->pTail = new_desc;
		new_desc->pNext = new_desc->pPrev = NULL;
	}
	else
	{
		start_desc = DVDRecUdf_find_desc(ext, offset);
		if (start_desc == NULL)
		{
			new_desc->pNext = ext->pHead;
			new_desc->pPrev = NULL;
			new_desc->pNext->pPrev = new_desc;
			ext->pHead = new_desc;
		}
		else
		{
			new_desc->pNext = start_desc->pNext;
			new_desc->pPrev = start_desc;
			if (start_desc->pNext)
				start_desc->pNext->pPrev = new_desc;
			else
			{
				ext->pTail = new_desc;
				//new_desc->prev = start_desc;
				//new_desc->next = NULL;
			}
			start_desc->pNext = new_desc;
			
		}
	}

	return new_desc;
}

//为描述符追加数据
void DVDRecUdf_append_data(struct udf_desc *desc, struct udf_data *data)
{
	struct udf_data *ndata = desc->pData;

	desc->length += data->length;

	while (ndata->pNext != NULL)
		ndata = ndata->pNext;

	ndata->pNext = data;
	data->pPrev = ndata;
}

void* DVDRecUdf_realloc(void *hMem,struct udf_desc *desc,unsigned long long size)
{
	desc->pData->pBuffer = MEMREALLOC(hMem,desc->pData->pBuffer,size);
	return desc->pData->pBuffer;
}

//分配数据
struct udf_data *DVDRecUdf_alloc_data(void *hMem, int length)
{
	struct udf_data *data;

	MEMMOCLINE;
	data = (udf_data *)MEMMALLOC(hMem, sizeof(struct udf_data));
	memset(data, 0, sizeof(struct udf_data));

	if (length)
	{
		MEMMOCLINE;
		data->pBuffer  = MEMMALLOC(hMem, length);
		memset(data->pBuffer, 0, length);
		data->bMalloc = 1;
	}
	else
	{
		data->pBuffer = NULL;
	}
	data->length = length;
	data->ilength = 0;

	return data;
}
