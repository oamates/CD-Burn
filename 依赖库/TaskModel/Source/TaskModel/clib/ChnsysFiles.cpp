#include "ChnsysFiles.h"
#include "ZOSFile.h"

OS_FILE_HANDLE OS_FILE_CreateInstance()
{
    ZOSFile *pOSFile = NEW ZOSFile;
    return (OS_FILE_HANDLE)pOSFile;
}

VOID OS_FILE_DestroyInstance(OS_FILE_HANDLE hHandle)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        SAFE_DELETE(pOSFile);
    }
}

CHNSYS_BOOL OS_FILE_IsOpen(OS_FILE_HANDLE hHandle)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->IsOpen();
    }

    return FALSE;
}

CHNSYS_BOOL OS_FILE_IsDir(OS_FILE_HANDLE hHandle)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->IsDir();
    }

    return FALSE;
}

CHNSYS_BOOL OS_FILE_IsReadOnly(OS_FILE_HANDLE hHandle)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->IsReadOnly();
    }

    return FALSE;
}

CHNSYS_BOOL OS_FILE_IsReadable(OS_FILE_HANDLE hHandle)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->IsReadable();
    }

    return FALSE;
}

CHNSYS_BOOL OS_FILE_IsWriteable(OS_FILE_HANDLE hHandle)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->IsWriteable();
    }

    return FALSE;
}

CHNSYS_BOOL OS_FILE_Open(OS_FILE_HANDLE hHandle, CONST CHNSYS_CHAR* sName, CHNSYS_BOOL bReadOnly, CHNSYS_BOOL bAppend)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->Open(sName, bReadOnly, bAppend);
    }

    return FALSE;
}

CHNSYS_BOOL OS_FILE_Create(OS_FILE_HANDLE hHandle, CONST CHNSYS_CHAR* sName, CHNSYS_BOOL bCreateNew)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->Create(sName, bCreateNew);
    }

    return FALSE;
}

CHNSYS_BOOL OS_FILE_Close(OS_FILE_HANDLE hHandle)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->Close();
    }

    return FALSE;
}

CHNSYS_INT OS_FILE_Read(OS_FILE_HANDLE hHandle, CHNSYS_CHAR* sdata, CHNSYS_INT ndata)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->Read(sdata, ndata);
    }

    return -1;
}

CHNSYS_INT OS_FILE_Write(OS_FILE_HANDLE hHandle, CONST CHNSYS_CHAR* sdata, CHNSYS_INT ndata)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->Write(sdata, ndata);
    }

    return -1;
}

CHNSYS_INT OS_FILE_Seek(OS_FILE_HANDLE hHandle, CHNSYS_UINT64 nPos)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->Seek(nPos);
    }

    return -1;
}

CHNSYS_INT OS_FILE_GetHandle(OS_FILE_HANDLE hHandle)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->GetHandle();
    }

    return -1;
}

CHNSYS_CHAR* OS_FILE_GetPath(OS_FILE_HANDLE hHandle)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->GetPath();
    }

    return NULL;
}

CHNSYS_UINT64 OS_FILE_GetPosition(OS_FILE_HANDLE hHandle)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->GetPosition();
    }

    return -1;
}

CHNSYS_UINT64 OS_FILE_SetPosition(OS_FILE_HANDLE hHandle, CHNSYS_UINT64 nPos)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->SetPosition(nPos);
    }

    return -1;
}

CHNSYS_UINT64 OS_FILE_GetSize(OS_FILE_HANDLE hHandle)
{
    if (hHandle != NULL)
    {
        ZOSFile *pOSFile = (ZOSFile *)hHandle;
        return pOSFile->GetSize();
    }

    return -1;
}
