#ifndef ERROR_CODE_TABLE_H
#define ERROR_CODE_TABLE_H

#include	"public.h"
#include	"Version.h"

#define ERROR_CODE_SIZE		(256)

// product name
#define PRODUCT_NAME	"TaskModel"
// sub module name
#define SMCOMMON				"Common"
#define SMMANAGER				"Manager"
#define SMMEDIA                 "Media"
#define SMMODULE				"Module"
#define SMPROTOCOL              "Protocol"
#define SMSERVER				"Server"

#define COMMON_STRING		PRODUCT_NAME""VERSION_NO"_"SMCOMMON"_"
#define MANAGER_STRING		PRODUCT_NAME""VERSION_NO"_"SMMANAGER"_"
#define MEDIA_STRING		PRODUCT_NAME""VERSION_NO"_"SMMEDIA"_"
#define MODULE_STRING		PRODUCT_NAME""VERSION_NO"_"SMMODULE"_"
#define PROTOCOL_STRING		PRODUCT_NAME""VERSION_NO"_"SMPROTOCOL"_"
#define SERVER_STRING		PRODUCT_NAME""VERSION_NO"_"SMSERVER"_"

// TaskModel1.0.10.16_Common_20000
const char* GetErrorCodeString(int nErrorCode);

#endif //ERROR_CODE_TABLE_H

