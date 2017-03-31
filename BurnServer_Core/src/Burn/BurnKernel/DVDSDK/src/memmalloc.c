//memmalloc.c
#if defined(MEMORY_DEBUG)

#include "memmalloc.h"
#include "debug.h"

static int mallc_nums = 0;

static int mallc_ptr[1024 * 1024];

static int resetptr(int src, int dst)
{
	int i;
	for(i=0; i<mallc_nums; i++)
	{
		if(mallc_ptr[i] == src)
		{
			mallc_ptr[i] = dst;
			return 1;
		}
	}
	DPERROR(("not find ptr: 0x%08x\n", src));
	return 0;
}

void *memmalloc(int size)
{
	void *pPtr = malloc(size);
	LvNetDP_Printf("malloc buffer : 0x%08x, size:%d\n", (int)pPtr, size);
	mallc_ptr[mallc_nums++] = (int)pPtr;
	return pPtr;
}

void *memrealloc(void *memblock, size_t size)
{
	void *pPtr = realloc(memblock, size);
	LvNetDP_Printf("realloc buffer : 0x%08x - 0x%08x, size:%d\n", (int)memblock, (int)pPtr, size);
	if(!resetptr((int)memblock, (int)pPtr))
		mallc_ptr[mallc_nums++] = (int)pPtr;
	return pPtr;
}

void memfree(void *memblock)
{
	int i;
	int bFind = 0;
	for(i=0; i<mallc_nums; i++)
	{
		if(mallc_ptr[i] == (int)memblock)
		{
			mallc_ptr[i] = 0;
			bFind = 1;
			break;
		}
	}
	if(!bFind)
	{
		DPERROR(("free not find : 0x%08x\n", (int)memblock));
	}
	free(memblock);
}

void  memprintinfo()
{
	int i;
	DP(("malloc buffer nums : %d\n", mallc_nums));
	for(i=0; i<mallc_nums; i++)
	{
		if(mallc_ptr[i])
		{
			DPERROR(("not free buffer: 0x%08x\n", (int)mallc_ptr[i]));
		}
	}
}

#endif//MEMORY_DEBUG
