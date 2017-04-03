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
struct udf_extent *DVDRecUdf_next_extent(struct udf_extent *start_ext, enum udf_space_type type)
{
	while (start_ext != NULL && !(start_ext->space_type & type))
	{
		start_ext = start_ext->next;
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

	start_ext = disc->head;

	while (start_ext->next != NULL)
	{
		if (start_ext->start + start_ext->blocks > start)
			break;
		start_ext = start_ext->next;
	}
	return start_ext;
}

//设定盘区，用链表记录每个盘区的起始逻辑块和占用空间
struct udf_extent *DVDRecUdf_set_extent(void *hMem, struct udf_disc *disc, enum udf_space_type type, uint32_t start, uint32_t blocks)
{
	struct udf_extent *start_ext, *new_ext;

	start_ext = find_extent(disc, start);
	if (start == start_ext->start)
	{
		if (blocks == start_ext->blocks)              //block == 16
		{
			start_ext->space_type = type;

			return start_ext;
		}//         16               2297888
		else if (blocks < start_ext->blocks)
		{
			MEMMOCLINE;
			new_ext = MEMMALLOC(hMem, sizeof(struct udf_extent));
			memset(new_ext, 0, sizeof(struct udf_extent));
			new_ext->space_type = type;
			new_ext->start = start;
			new_ext->blocks = blocks;
			new_ext->head = new_ext->tail = NULL;
			new_ext->prev = start_ext->prev;
			if (new_ext->prev)
				new_ext->prev->next = new_ext;
			new_ext->next = start_ext;
			if (disc->head == start_ext)
				disc->head = new_ext;

			start_ext->start += blocks;
			start_ext->blocks -= blocks;
			start_ext->prev = new_ext;

			return new_ext;
		}
		else /* blocks > start_ext->blocks */
		{
			DPERROR(("trying to change type of multiple extents\n"));
			
			// 
			//exit(1);
			
			start_ext->space_type = type;

			return start_ext;
		}
	}
	else /* start > start_ext->start */
	{
		if (start + blocks == start_ext->start + start_ext->blocks)
		{
			MEMMOCLINE;
			new_ext = MEMMALLOC(hMem, sizeof(struct udf_extent));
			memset(new_ext, 0, sizeof(struct udf_extent));
			new_ext->space_type = type;
			new_ext->start = start;
			new_ext->blocks = blocks;
			new_ext->head = new_ext->tail = NULL;
			new_ext->prev = start_ext;
			new_ext->next = start_ext->next;
			if (new_ext->next)
				new_ext->next->prev = new_ext;
			if (disc->tail == start_ext)
				disc->tail = new_ext;

			start_ext->blocks -= blocks;
			start_ext->next = new_ext;

			return new_ext;
		}
		else if (start + blocks < start_ext->start + start_ext->blocks)
		{
			MEMMOCLINE;
			new_ext = MEMMALLOC(hMem, sizeof(struct udf_extent));
			memset(new_ext, 0, sizeof(struct udf_extent));
			new_ext->space_type = type;
			new_ext->start = start;
			new_ext->blocks = blocks;
			new_ext->head = new_ext->tail = NULL;
			new_ext->prev = start_ext;
			
			MEMMOCLINE;
			new_ext->next = MEMMALLOC(hMem, sizeof(struct udf_extent));
			memset(new_ext->next, 0, sizeof(struct udf_extent));
			new_ext->next->prev = new_ext;
			new_ext->next->space_type = start_ext->space_type;
			new_ext->next->start = start + blocks;
			new_ext->next->blocks = start_ext->blocks - blocks - start + start_ext->start;
			new_ext->next->head = new_ext->next->tail = NULL;
			new_ext->next->next = start_ext->next;
			if (new_ext->next->next)
				new_ext->next->next->prev = new_ext->next;
			if (disc->tail == start_ext)
				disc->tail = new_ext->next;

			start_ext->blocks = start - start_ext->start;
			start_ext->next = new_ext;

			return new_ext;
		}
		else /* start + blocks > start_ext->start + start_ext->blocks */
		{
			MEMMOCLINE;
			new_ext = MEMMALLOC(hMem, sizeof(struct udf_extent));
			memset(new_ext, 0, sizeof(struct udf_extent));
			new_ext->space_type = type;
			new_ext->start = start;
			new_ext->blocks = blocks;
			new_ext->head = new_ext->tail = NULL;
			new_ext->prev = start_ext;
			new_ext->next = start_ext->next;
			if (new_ext->next)
				new_ext->next->prev = new_ext;
			if (disc->tail == start_ext)
				disc->tail = new_ext;

			start_ext->blocks -= blocks;
			start_ext->next = new_ext;

			return new_ext;
		}
	}
}

//下一个描述符指针
struct udf_desc *DVDRecUdf_next_desc(struct udf_desc *start_desc, uint16_t ident)
{
	while (start_desc != NULL && start_desc->ident != ident)
		start_desc = start_desc->next;

	return start_desc;
}

//寻找描述符
struct udf_desc *DVDRecUdf_find_desc(struct udf_extent *ext, uint32_t offset)
{
	struct udf_desc *start_desc;

	start_desc = ext->head;

	while (start_desc->next != NULL)
	{
		if (start_desc->offset == offset)
			return start_desc;
		else if ((start_desc->offset) > offset)
		{	
			return start_desc->prev;
		}
		else
			start_desc = start_desc->next;
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
	new_desc = MEMMALLOC(hMem, sizeof(struct udf_desc));
	memset(new_desc, 0, sizeof(struct udf_desc));
	new_desc->ident = ident;
	new_desc->offset = offset;
	new_desc->length = length;
	if (data == NULL)
	{
		new_desc->data = DVDRecUdf_alloc_data(hMem, length);
	}
	else
	{
		new_desc->data = data;
	}
	if (ext->head == NULL)
	{
		ext->head = ext->tail = new_desc;
		new_desc->next = new_desc->prev = NULL;
	}
	else
	{
		start_desc = DVDRecUdf_find_desc(ext, offset);
		if (start_desc == NULL)
		{
			new_desc->next = ext->head;
			new_desc->prev = NULL;
			new_desc->next->prev = new_desc;
			ext->head = new_desc;
		}
		else
		{
			new_desc->next = start_desc->next;
			new_desc->prev = start_desc;
			if (start_desc->next)
				start_desc->next->prev = new_desc;
			else
			{
				ext->tail = new_desc;
				//new_desc->prev = start_desc;
				//new_desc->next = NULL;
			}
			start_desc->next = new_desc;
			
		}
	}

	return new_desc;
}

//为描述符追加数据
void DVDRecUdf_append_data(struct udf_desc *desc, struct udf_data *data)
{
	struct udf_data *ndata = desc->data;

	desc->length += data->length;

	while (ndata->next != NULL)
		ndata = ndata->next;

	ndata->next = data;
	data->prev = ndata;
}

void* DVDRecUdf_realloc(void *hMem,struct udf_desc *desc,unsigned long long size)
{
	desc->data->buffer = MEMREALLOC(hMem,desc->data->buffer,size);
	return desc->data->buffer;
}

//分配数据
struct udf_data *DVDRecUdf_alloc_data(void *hMem, int length)
{
	struct udf_data *data;

	MEMMOCLINE;
	data = MEMMALLOC(hMem, sizeof(struct udf_data));
	memset(data, 0, sizeof(struct udf_data));

	if (length)
	{
		MEMMOCLINE;
		data->buffer  = MEMMALLOC(hMem, length);
		memset(data->buffer, 0, length);
		data->bMalloc = 1;
	}
	else
	{
		data->buffer = NULL;
	}
	data->length = length;
	data->ilength = 0;

	return data;
}
