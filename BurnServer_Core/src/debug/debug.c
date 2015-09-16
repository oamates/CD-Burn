#include <syslog.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include "common.h"
#include "debug.h"

struct _debug_flags
{
	int32_t	all;
	int32_t info;
	int32_t time;
	int32_t main;
	int32_t ses;
}debug_flags;

void DPRINT(s32 mode, s8 *filename, s32 line, s8 *format, ...)
{
	va_list args;

#if 1
	switch(mode)
	{
		case DMAIN:
			if(!(debug_flags.all == 1) && !(debug_flags.main == 1)) 
			{
				return;
			}
			break;
		case DSES:	
			if(!(debug_flags.all == 1) && !(debug_flags.ses == 1)) 
			{
				return;
			}
			break;
		default:
			return;
	}
#endif

	if(debug_flags.info == 1)
	{
		fprintf(stderr, "<%s:%d> ", filename, line);
	}	

	if(debug_flags.time == 1)
	{
		time_t sec = time(NULL);
		struct tm t = *localtime(&sec);	
		fprintf(stderr, "[%02d:%02d:%02d] ", t.tm_hour, t.tm_min, t.tm_sec);
	}	
		
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

void DEBUG_PRINTF(s32 level, s8 *filename, s32 line, s8 *format, ...)
{

	va_list args;
	time_t sec = time(NULL);
	struct tm t = *localtime(&sec);	

	fprintf(stderr, "%s:%d [%02d:%02d:%02d] %ld \t\t", filename, line, t.tm_hour, t.tm_min, t.tm_sec, syscall(SYS_gettid));

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

int set_debug_mode(int mode)
{
	switch(mode)
	{
		case DALL:
			debug_flags.all = 1;
			//debug_flags.all = TRUE;
			break;
		case DTIME:
			//debug_flags.time = TRUE;
			debug_flags.time = 1;
			break;
		case DINFO:
			debug_flags.info = 1;
			break;
		case DMAIN:
			debug_flags.main = 1;
			break;
		case DSES:
			debug_flags.ses = 1;
			break;
		case DTIMEOFF:
			debug_flags.time = 0;
			break;
		case DINFOOFF:
			debug_flags.info = 0;
			break;
		case DMAINOFF:
			debug_flags.main = 0;
			break;
		case DSESOFF:
			debug_flags.ses = 0;
			break;
		default:
			printf("No Support Mode For Debug\n");
	}

//	printf("all = %d, time = %d, info = %d, main = %d, ses = %d\n", debug_flags.all, debug_flags.time, debug_flags.info, debug_flags.main, debug_flags.ses);
	return 0;	
}

void debug_init(void)
{
	bzero(&debug_flags, sizeof(debug_flags));
}

void debug_clean(void)
{
	bzero(&debug_flags, sizeof(debug_flags));
}
