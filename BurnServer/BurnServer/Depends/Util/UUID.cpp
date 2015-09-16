#include "UUID.h"
extern "C"
{
#ifdef WIN32
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")
#else
#endif
}
#ifdef WIN32
#else
#include <uuid/uuid.h>
#endif

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
    uuid_generate_random(uuid);
    char szOut[100];
    memset(szOut, 0, 100);
    uuid_unparse(uuid, szOut);
    return std::string(szOut);
#endif
}


std::string UUID_GetSpecialUUID()
{
    std::string     strUUID;
    std::string     strSpecialUUID;

    strUUID = UUID_GetUUID();
    for (size_t i = 0; i < strUUID.length(); i ++)
    {
        if (strUUID.at(i) == '-')
        {
            strSpecialUUID.push_back('_');
        }
        else if (strUUID.at(i) >= 'a' && strUUID.at(i) <= 'z')
        {
            strSpecialUUID.push_back(strUUID.at(i)-32);
        }
        else
        {
            strSpecialUUID.push_back(strUUID.at(i));
        }
    }

    return strSpecialUUID;
}
