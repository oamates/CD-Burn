//memmalloc.h
#ifndef __MEMMALLOC_H__
#define __MEMMALLOC_H__
/*
 * 内存调试，查看分配内存与释放内存，查看是否有内存泄漏
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "LvDebugPrint.h"

#if defined(MEMORY_DEBUG)
void  *memmalloc(int size);
void  *memrealloc(void *memblock, size_t size);
void  memfree(void *memblock);
void  memprintinfo();

#define MEMMOCLINE	LvNetDP_Printf("malloc buffer %s, line : %d\n", __FILE__, __LINE__)
#define MEMMALLOC(_h, _size)		memmalloc(_size)
#define MEMREALLOC(_h, _mp, _size)	memrealloc(_mp, _size)
#define MEMFREE(_h, _mp)			memfree(_mp)

#elif defined(USE_MEMADMIN)
#include "UsememAdmin.h"
// memory admin

#define MEMMOCLINE
#define MEMMALLOC(_h, _size)		umemalloc(_h, NULL, _size)
#define MEMREALLOC(_h, _mp, _size)	umemrealloc(_h, NULL, _mp, _size)
#define MEMFREE(_h, _mp)			umemfree(_h, NULL, _mp)

#else // MEMMALLOC

#define MEMMOCLINE
#define MEMMALLOC(_h, _size)		malloc(_size)
#define MEMREALLOC(_h, _mp, _size)	realloc(_mp, _size)
#define MEMFREE(_h, _mp)			free(_mp)

#endif//MEMORY_DEBUG

#endif//__MEMMALLOC_H__
