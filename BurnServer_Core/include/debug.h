#ifndef __DEBUG__H__
#define __DEBUG__H__


#define DEBUG_LEVEL  4
#define DEBUG_TRACE  1

#define DEBUG_NORMAL 3
#define DEBUG_ERR   10

#define DEBUG 1

#define SESSION 8
#define SESSION2 8
#define UTILS    DEBUG_NORMAL
#define UTILS2    DEBUG_NORMAL
#define MAIN    DEBUG_NORMAL
#define MAIN2    DEBUG_NORMAL

#include <assert.h>
#define ASSERT(exp) assert((exp))

#include "common.h"

volatile int32_t mix_search_flags;
volatile int32_t mix_getdata_flags;
volatile int32_t mix_version_flags;
volatile int32_t mix_search_data[10];
void write_mix_xml(u8*buf_ptr,u16 size);
void write_version_to_xml(u8*buf_ptr,u16 size);


enum
{
	DALL,
	DTIME,
	DINFO,
	DMAIN,
	DSES,
	DTIMEOFF,
	DINFOOFF,
	DMAINOFF,
	DSESOFF
};

void debug_init(void);
void debug_clean(void);
int set_debug_mode(int mode);
void DPRINT(s32 mode, s8 *filename, s32 line, s8 *format, ...);

void DEBUG_PRINTF(s32 level, s8 *filename, s32 line, s8 *format, ...);

#define DEP(mode,format, ...)   		\
	do { \
			DPRINT(mode, __FILE__, __LINE__, format, ##__VA_ARGS__);  \
	}while(0)

#if 0
#if (defined(DEBUG))
#define DPRINTF(level,format, ...)   \
	do { \
		if(level <= DEBUG_LEVEL)  \
			break;  \
		DEBUG_PRINTF(level, __FILE__, __LINE__, format, ##__VA_ARGS__);  \
		if (level == DEBUG_TRACE) usleep(20*1000); \
		if(level == DEBUG_ERR)  { \
			usleep(20*1000); \
			ASSERT(0); \
		} \
	}while(0)
#else		
#define DPRINTF(level,format, ...)   		\
	do { \
		if(level == DEBUG_ERR) { \
			DEBUG_PRINTF(level, __FILE__, __LINE__, format, ##__VA_ARGS__);  \
			usleep(20*1000); \
			ASSERT(0); \
			} \
	}while(0)
									
#endif

#define DEP_MAIN(format, ...) \
do{ \
	DEP(DMAIN, format, ##__VA_ARGS__); \
}while(0)

#endif
#endif


