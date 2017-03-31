///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	public		Header File
	Create		20100603		ZHAOTT
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_PUBLIC_H_
#define	_PUBLIC_H_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef	_WIN32_
///////////////////////////////////////////////////////////////////////////////
//Warnings
#ifndef     _CRT_SECURE_NO_WARNINGS
#define		_CRT_SECURE_NO_WARNINGS
#endif //_CRT_SECURE_NO_WARNINGS
#ifndef     _CRT_NONSTDC_NO_WARNINGS
#define		_CRT_NONSTDC_NO_WARNINGS
#endif //_CRT_NONSTDC_NO_WARNINGS
///////////////////////////////////////////////////////////////////////////////
//POSIX errorcodes
#define EADDRINUSE  98 //Address already in use.
#define EADDRNOTAVAIL 99 //Address not available
#define ECONNRESET  104 //Connection reset by peer.
#define ENOBUFS   105 //No buffer space available.
#define ENOTCONN  107 //Socket is not connected
#define ETIMEDOUT  110 //Connection timed out.
#define ECONNREFUSED 111 //Connection refused.
#define EINPROGRESS  115 //Connection already in progress

///////////////////////////////////////////////////////////////////////////////
//Assert
#ifdef	_DEBUG
#define	_ASSERT_	1
#endif	//_DEBUG
///////////////////////////////////////////////////////////////////////////////
#endif	//_WIN32_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef	_MINGW_
///////////////////////////////////////////////////////////////////////////////
//POSIX errorcodes
#define	ENOTCONN		1002
#define	EADDRINUSE		1004
#define	EINPROGRESS		1007
#define	ENOBUFS			1008
#define	EADDRNOTAVAIL	1009
///////////////////////////////////////////////////////////////////////////////
#endif	//__MINGW__
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef	_LINUX_
///////////////////////////////////////////////////////////////////////////////
//Large file
#ifndef	_LARGEFILE_SOURCE
#define	_LARGEFILE_SOURCE
#endif	//_LARGEFILE_SOURCE
#ifndef	_LARGEFILE64_SOURCE
#define	_LARGEFILE64_SOURCE
#endif	//_LARGEFILE64_SOURCE
#ifndef	_FILE_OFFSET_BITS
#define	_FILE_OFFSET_BITS		64
#endif	//_FILE_OFFSET_BITS
///////////////////////////////////////////////////////////////////////////////
#endif	//_LINUX_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//Includes
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//Const defines
typedef int					BOOL;
typedef char				CHAR;
typedef unsigned char		UCHAR;
typedef unsigned char		BYTE;
typedef unsigned short		WORD;
#if _WIN32_
typedef unsigned long		DWORD;
#else
typedef unsigned int		DWORD;
#endif
typedef float				FLOAT;
typedef double				DOUBLE;
typedef signed int			INT;
typedef signed char			INT8;
typedef signed short		INT16;
typedef signed int			INT32;
typedef unsigned int		UINT;
typedef unsigned char		UINT8;
typedef unsigned short		UINT16;
typedef unsigned int		UINT32;
///////////////////////////////////////////////////////////////////////////////
#if	_WIN32_
typedef __int64				INT64;
typedef unsigned __int64    UINT64;
#define	F_NUM_64			"I64"
#else
#include <limits.h>
typedef signed long long	INT64;
typedef unsigned long long  UINT64;
#define	F_NUM_64			"ll"
#endif	//_WIN32_
///////////////////////////////////////////////////////////////////////////////
#undef	SINT16_MAX
#define	SINT16_MAX			SHRT_MAX
#undef	UINT16_MAX
#define	UINT16_MAX			USHRT_MAX
#undef	SINT32_MAX
#define	SINT32_MAX			LONG_MAX
#undef	UINT32_MAX
#define	UINT32_MAX			ULONG_MAX
#undef	SINT64_MAX
#define	SINT64_MAX			LLONG_MAX
#undef	UINT64_MAX
#define	UINT64_MAX			ULLONG_MAX
typedef	UINT64				QWORD;
typedef	float				FLOAT32;
typedef	double				FLOAT64;
typedef	long				LONG;
typedef	unsigned long		ULONG;
///////////////////////////////////////////////////////////////////////////////
//VOID
#ifndef	VOID
typedef void	            VOID;
#endif	//VOID
typedef void				*PVOID;
typedef void	            *LPVOID;
///////////////////////////////////////////////////////////////////////////////
//NULL
#ifndef NULL
#ifdef __cplusplus
#define NULL				0
#else
#define NULL				((void *)0)
#endif
#endif
///////////////////////////////////////////////////////////////////////////////
//FALSE
#ifndef FALSE
#define FALSE				0
#endif
///////////////////////////////////////////////////////////////////////////////
//TRUE
#ifndef TRUE
#define TRUE				1
#endif
///////////////////////////////////////////////////////////////////////////////
//MIN MAX ABS
#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef	ABS
#define	ABS(a)		((a) < 0 ? -(a) : (a))
#endif
///////////////////////////////////////////////////////////////////////////////
//CONST
#ifndef	CONST
#define CONST	const
#endif
///////////////////////////////////////////////////////////////////////////////
//STATIC
#ifndef	STATIC
#define STATIC	static
#endif
///////////////////////////////////////////////////////////////////////////////
//INLINE
#ifndef	INLINE
#define INLINE	inline
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//File Handle
#define	INVALID_FILE_HANDLE			-1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if	defined(_WIN32_)
#define	S_ISDIR(f)					(f&_S_IFDIR)
#define	S_IRWXU						(_S_IWRITE|_S_IREAD|_S_IEXEC)
#define	S_IRWXG						0
#define	S_IRWXO						0
#elif defined(_MINGW_)
#define	S_IRWXG						0
#define	S_IRWXO						0
#else
#define	O_BINARY					0
#endif	//_WIN32_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef	_WIN32_
#define	snprintf					_snprintf
#define	vsnprintf					_vsnprintf
#define	strcasecmp					stricmp
#define	strncasecmp					strnicmp
#define	fstat64						_fstat64
#define	stat64						_stat64
#define	lseek64						_lseeki64
#else
#ifndef	__USE_LARGEFILE64
#define	lseek64						lseek
#define	__NO_MINGW_LFS				1
#endif	//__USE_LARGEFILE64
#endif	//_WIN32_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define	MAX_KEY_LABEL			256
#define	MAX_FILE_NAME			256
#ifndef MAX_FILE_PATH
#define	MAX_FILE_PATH			1024
#endif//
#define	MAX_URI_PATH			1024
#define	MAX_MESSAGE_TITLE		1024
#define	MAX_MESSAGE_DATA		4096
#define MAX_IP_LENGTH			16
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//Seconds(s)
#define	DEFAULT_NET_MIN_TIMEOUT		1		//1s
#define	DEFAULT_NET_MAX_TIMEOUT		9		//10s
//Milliseconds(ms)
#define	DEFAULT_NET_MIN_MTIMEOUT	1		//1ms
#define	DEFAULT_NET_MAX_MTIMEOUT	999		//1000ms=1s
//Microsecond(us)
#define	DEFAULT_NET_MIN_UTIMEOUT	999		//1000us=1ms
#define	DEFAULT_NET_MAX_UTIMEOUT	999999	//1000000us=1s
//
#define	DEFAULT_PROCESS_TIMEOUT		30		//30s
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define LOG_SET_PATH(l)		if(pLog)			{pLog->SetLogPath(l);}
#define LOG_SET_LEVEL(l)	if(pLog)			{pLog->SetLogLevel(l);}
#define LOG_ERROR(s)		if(pLog)			{pLog->LOGError		s;}
#define LOG_WARNING(s)		if(pLog)			{pLog->LOGWarning	s;}
#define LOG_INFO(s)			if(pLog)			{pLog->LOGInfo		s;}
#define LOG_DEBUG(s)		if(pLog)			{pLog->LOGDebug		s;}
#define	DEBUG_OUT(s)		if(pLog)			{pLog->Logout		s;}
#define	MESSAGE_OUT(s)		if(pLog)			{pLog->MessageOut	s;}
#define	PRINT_OUT(s)							{printf				s;}
#define	TITLE_OUT(n,s)		if(n&COMMAND_DEBUG)	{printf				s;}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define	COUNT_OF(a)			(sizeof(a)/sizeof(a[0]))
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define	COMMAND_SERVICE_START		(1)
#define	COMMAND_SERVICE_STOP		(1<<1)
#define	COMMAND_SERVICE_PAUSE		(1<<2)
#define	COMMAND_SERVICE_RESUME		(1<<3)
#define	COMMAND_SERVICE_INSTALL		(1<<4)
#define	COMMAND_SERVICE_UNINSTALL	(1<<5)
#define	COMMAND_RUN					(1<<6)
#define	COMMAND_DEBUG				(1<<7)
#define	COMMAND_VERSION				(1<<8)
#define	COMMAND_HELP				(1<<9)
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define	SAFE_DELETE(p)				if(p!=NULL){delete(p);(p)=NULL;}
#define	SAFE_DELETE_ARRAY(a)		if(a!=NULL){delete[](a);(a)=NULL;}
#define	SAFE_RELEASE(p)				if(p!=NULL){p->Release();(p)=NULL;}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define _DEBUG_
#endif//_DEBUG
#ifdef _DEBUG_
#if	defined(_WIN32_)
#ifndef	_MINGW_
#define	MEMORY_LEAK_CHECKER_BEGIN	ZOSMemory::CheckBegin();{
#define	MEMORY_LEAK_CHECKER_END		}ZOSMemory::CheckEnd();
#else
#define	MEMORY_LEAK_CHECKER_BEGIN
#define	MEMORY_LEAK_CHECKER_END
#endif	//_MINGW_
#else
#define	MEMORY_LEAK_CHECKER_BEGIN
#define	MEMORY_LEAK_CHECKER_END
#endif	//_WIN32_
#else
#define	MEMORY_LEAK_CHECKER_BEGIN
#define	MEMORY_LEAK_CHECKER_END
#endif	//_DEBUG_
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "error.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif	//_PUBLIC_H_
///////////////////////////////////////////////////////////////////////////////
