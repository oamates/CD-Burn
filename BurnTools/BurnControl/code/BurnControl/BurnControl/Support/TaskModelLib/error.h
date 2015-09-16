///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	error		Header File
	Create		20100603		ZHAOTT
******************************************************************************/
#ifndef	ZERROR_H_
#define	ZERROR_H_
///////////////////////////////////////////////////////////////////////////////
//	|L |CODE							 |
//	|xx|xxxxxx xxxxxxxx xxxxxxxx xxxxxxxx|
//	L	:	00	Success;	01	Informational;	10	Warning;	11	Error
///////////////////////////////////////////////////////////////////////////////
//Error Code
///////////////////////////////////////////////////////////////////////////////
//no error
#define	DEFAULT_ERROR_OK					0x00000000	//ok
//error
#define	DEFAULT_ERROR_ERROR					0xC0000001	//unknown error
#define	DEFAULT_ERROR_MEMORY				0xC0000002	//memory error
#define	DEFAULT_ERROR_INVALID				0xC0000003	//parameters invalid
#define	DEFAULT_ERROR_TYPE					0xC0000004	//type error
//net error
#define	DEFAULT_ERROT_NET_CREATE			0xC0000011	//net create error
#define	DEFAULT_ERROT_NET_CONNECT			0xC0000012	//net connect error
#define	DEFAULT_ERROT_NET_BIND				0xC0000013	//net bind error
#define	DEFAULT_ERROT_NET_GET				0xC0000014	//net get option error
#define	DEFAULT_ERROT_NET_SET				0xC0000015	//net set option error
#define	DEFAULT_ERROT_NET_READ				0xC0000016	//net read error
#define	DEFAULT_ERROT_NET_WRITE				0xC0000017	//net write error
//module error
#define	DEFAULT_ERROR_ALREADYEXISTS			0xC0000101	//exists
#define	DEFAULT_ERROR_NOTSUPPORT			0xC0000102	//not support
//
#define	DEFAULT_ERROR_MODULE_NOTSUPPORT		0x80000201	//module not support
#define	DEFAULT_ERROR_MODULE_RECORDER_OPEN	0xC0000211	//module recorder open error
#define	DEFAULT_ERROR_MODULE_RECORDER_DATA	0xC0000212	//module recorder data error
#define DEFALUT_ERROR_MODULE_SOURCE_NODATA	0xC0000231	//module source no data
//command error
#define	DEFAULT_ERROR_COMMAND_NOTSUPPORT	0x80000301	//command not support
#define	DEFAULT_ERROR_COMMAND_UNKNOWN		0x80000302	//command unknown
//
#define DEFAULT_ERROR_CONFIGNAME_NOTSUPPORT	0x80000351	//
//protocol error
#define	DEFAULT_ERROR_PROTOCOL_NOTSUPPORT	0x80000401	//protocol not support
//admin error
#define	DEFAULT_ADMIN_READY					0x40000101	//admin ready
#define	DEFAULT_ADMIN_BUSY					0x40000102	//admin busy
#define	DEFAULT_ADMIN_UNKNOWN				0x40000103	//admin unknown
#define	DEFAULT_ADMIN_CLOSE					0x40000104	//admin close
//
#define	DEFAULT_STATUS						0x00000001	//status
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const char* GetServerError(int nErrorCode);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif	//ZERROR_H_
///////////////////////////////////////////////////////////////////////////////
