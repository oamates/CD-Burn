#include <stdafx.h>

#include "UUID.h"
extern "C"
{
#ifdef WIN32
#include <Rpc.h>
#pragma comment(lib,"Rpcrt4.lib")
#else
#include <uuid/uuid.h>
#endif
}


std::string UUID_GetUUID()
{
#ifdef WIN32
    UUID uuid;
    char *sUUID;
    std::string strUUID;

    if (UuidCreate(&uuid) == RPC_S_OK)
    {
        if (UuidToStringA(&uuid, (RPC_CSTR *)&sUUID) == RPC_S_OK)
        {
            strUUID = std::string(sUUID);
            RpcStringFreeA((RPC_CSTR *)&sUUID);
        }
    }

    return strUUID;
#else
    uuid_t uuid;
    uuid_generate_random ( uuid );
    char s[37];
    uuid_unparse ( uuid, s );
    return s;
#endif
}
