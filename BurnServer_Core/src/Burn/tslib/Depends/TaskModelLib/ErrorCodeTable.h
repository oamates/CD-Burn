#ifndef ERROR_CODE_TABLE_H
#define ERROR_CODE_TABLE_H

#include	"public.h"
#include	"Version.h"

#define ERROR_CODE_SIZE		(256)

// product name
#define PRODUCT_NAME	"TaskModel"
// sub module name
#define COMMON				"Common"
#define MANAGER				"Manager"
#define MEDIA				"Media"
#define MODULE				"Module"
#define PROTOCOL			"Protocol"
#define SERVER				"Server"

#define COMMON_STRING		PRODUCT_NAME""VERSION_NO"_"COMMON"_"
#define MANAGER_STRING		PRODUCT_NAME""VERSION_NO"_"MANAGER"_"
#define MEDIA_STRING		PRODUCT_NAME""VERSION_NO"_"MEDIA"_"
#define MODULE_STRING		PRODUCT_NAME""VERSION_NO"_"MODULE"_"
#define PROTOCOL_STRING		PRODUCT_NAME""VERSION_NO"_"PROTOCOL"_"
#define SERVER_STRING		PRODUCT_NAME""VERSION_NO"_"SERVER"_"

// TaskModel1.0.10.16_Common_20000
const char* GetErrorCodeString(int nErrorCode);

#endif //ERROR_CODE_TABLE_H

