#ifndef _CHNSYSTYPES_H_
#define _CHNSYSTYPES_H_

//

typedef int                 CHNSYS_BOOL;
typedef char                CHNSYS_CHAR;
typedef unsigned char       CHNSYS_UCHAR;
typedef unsigned char       CHNSYS_BYTE;
typedef unsigned short      CHNSYS_WORD;
typedef unsigned int        CHNSYS_DWORD;
typedef float               CHNSYS_FLOAT;
typedef double              CHNSYS_DOUBLE;
typedef signed int          CHNSYS_INT;
typedef signed char         CHNSYS_INT8;
typedef signed short        CHNSYS_INT16;
typedef signed int          CHNSYS_INT32;
typedef long                CHNSYS_LONG;
typedef unsigned int        CHNSYS_UINT;
typedef unsigned char       CHNSYS_UINT8;
typedef unsigned short      CHNSYS_UINT16;
typedef unsigned int        CHNSYS_UINT32;

#if _WIN32_
typedef __int64             CHNSYS_INT64;
typedef unsigned __int64    CHNSYS_UINT64;
#else
typedef signed long long    CHNSYS_INT64;
typedef unsigned long long  CHNSYS_UINT64;
#endif //_WIN32_

//CONST
#ifndef CONST
#define CONST   const
#endif//CONST

//VOID
#ifndef	VOID
typedef void    VOID;
#endif//VOID

//FALSE
#ifndef FALSE
#define FALSE 0
#endif//FALSE

//TRUE
#ifndef TRUE
#define TRUE 1
#endif//TRUE

#endif //_CHNSYS_TYPES_H_
