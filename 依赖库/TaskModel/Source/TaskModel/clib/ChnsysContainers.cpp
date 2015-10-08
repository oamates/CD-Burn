#include "ChnsysContainers.h"
#include "ZOSArray.h"
#include "ZOSHeap.h"
#include "ZOSQueue.h"

typedef ZOSArray<void *> VoidArray;

OS_VOIDARRAY_HANDLE OS_VOIDARRAY_CreateInstance(CHNSYS_UINT nInitSize)
{
    VoidArray *pVoidArray = NEW VoidArray(nInitSize);
    if (pVoidArray != NULL)
    {
        return (OS_VOIDARRAY_HANDLE)pVoidArray;
    }

    return NULL;
}

VOID OS_VOIDARRAY_DestroyInstance(OS_VOIDARRAY_HANDLE hVOIDArray)
{
    if (hVOIDArray != NULL)
    {
        VoidArray *pVoidArray = (VoidArray *)hVOIDArray;
        SAFE_DELETE(pVoidArray);
    }
}

CHNSYS_INT OS_VOIDARRAY_Add(OS_VOIDARRAY_HANDLE hVOIDArray, VOID *pValue)
{
    if (hVOIDArray != NULL)
    {
        VoidArray *pVoidArray = (VoidArray *)hVOIDArray;
        return pVoidArray->Add(pValue);
    }

    return -1;
}

CHNSYS_INT OS_VOIDARRAY_Set(OS_VOIDARRAY_HANDLE hVOIDArray, CHNSYS_INT nIndex, VOID *pValue)
{
    if (hVOIDArray != NULL)
    {
        VoidArray *pVoidArray = (VoidArray *)hVOIDArray;
        return pVoidArray->Set(nIndex, pValue);
    }

    return -1;
}

CHNSYS_INT OS_VOIDARRAY_Insert(OS_VOIDARRAY_HANDLE hVOIDArray, CHNSYS_INT nIndex, VOID *pValue)
{
    if (hVOIDArray != NULL)
    {
        VoidArray *pVoidArray = (VoidArray *)hVOIDArray;
        return pVoidArray->Insert(nIndex, pValue);
    }

    return -1;
}

CHNSYS_INT OS_VOIDARRAY_Remove(OS_VOIDARRAY_HANDLE hVOIDArray, CHNSYS_INT nIndex)
{
    if (hVOIDArray != NULL)
    {
        VoidArray *pVoidArray = (VoidArray *)hVOIDArray;
        return pVoidArray->Remove(nIndex);
    }

    return -1;
}

CHNSYS_INT OS_VOIDARRAY_RemoveAll(OS_VOIDARRAY_HANDLE hVOIDArray)
{
    if (hVOIDArray != NULL)
    {
        VoidArray *pVoidArray = (VoidArray *)hVOIDArray;
        return pVoidArray->RemoveAll();
    }

    return -1;
}

CHNSYS_INT OS_VOIDARRAY_Count(OS_VOIDARRAY_HANDLE hVOIDArray)
{
    if (hVOIDArray != NULL)
    {
        VoidArray *pVoidArray = (VoidArray *)hVOIDArray;
        return pVoidArray->Count();
    }

    return -1;
}

CHNSYS_INT OS_VOIDARRAY_At(OS_VOIDARRAY_HANDLE hVOIDArray, CHNSYS_INT nIndex, VOID **ppValue)
{
    if (
        (hVOIDArray != NULL)
        && (ppValue != NULL)
        )
    {
        VoidArray *pVoidArray = (VoidArray *)hVOIDArray;
        *ppValue = (*pVoidArray)[nIndex];

        return 0;
    }
    return -1;
}

typedef ZOSArray<int> IntArray;

OS_INTARRAY_HANDLE OS_INTARRAY_CreateInstance(CHNSYS_UINT nInitSize)
{
    IntArray *pIntArray = NEW IntArray(nInitSize);
    if (pIntArray != NULL)
    {
        return (OS_INTARRAY_HANDLE)pIntArray;
    }

    return NULL;
}

VOID OS_INTARRAY_DestroyInstance(OS_INTARRAY_HANDLE hIntArray)
{
    if (hIntArray != NULL)
    {
        IntArray *pIntArray = (IntArray *)hIntArray;
        SAFE_DELETE(pIntArray);
    }
}

CHNSYS_INT OS_INTARRAY_Add(OS_INTARRAY_HANDLE hIntArray, CHNSYS_INT nValue)
{
    if (hIntArray != NULL)
    {
        IntArray *pIntArray = (IntArray *)hIntArray;
        return pIntArray->Add(nValue);
    }

    return -1;
}

CHNSYS_INT OS_INTARRAY_Set(OS_INTARRAY_HANDLE hIntArray, CHNSYS_INT nIndex, CHNSYS_INT nValue)
{
    if (hIntArray != NULL)
    {
        IntArray *pIntArray = (IntArray *)hIntArray;
        return pIntArray->Set(nIndex, nValue);
    }

    return -1;
}

CHNSYS_INT OS_INTARRAY_Insert(OS_INTARRAY_HANDLE hIntArray, CHNSYS_INT nIndex, CHNSYS_INT nValue)
{
    if (hIntArray != NULL)
    {
        IntArray *pIntArray = (IntArray *)hIntArray;
        return pIntArray->Insert(nIndex, nValue);
    }

    return -1;
}

CHNSYS_INT OS_INTARRAY_Remove(OS_INTARRAY_HANDLE hIntArray, CHNSYS_INT nIndex)
{
    if (hIntArray != NULL)
    {
        IntArray *pIntArray = (IntArray *)hIntArray;
        return pIntArray->Remove(nIndex);
    }

    return -1;
}

CHNSYS_INT OS_INTARRAY_RemoveAll(OS_INTARRAY_HANDLE hIntArray)
{
    if (hIntArray != NULL)
    {
        IntArray *pIntArray = (IntArray *)hIntArray;
        return pIntArray->RemoveAll();
    }

    return -1;
}

CHNSYS_INT OS_INTARRAY_Count(OS_INTARRAY_HANDLE hIntArray)
{
    if (hIntArray != NULL)
    {
        IntArray *pIntArray = (IntArray *)hIntArray;
        return pIntArray->Count();
    }

    return -1;
}

CHNSYS_INT OS_INTARRAY_At(OS_INTARRAY_HANDLE hIntArray, CHNSYS_INT nIndex, CHNSYS_INT *pValue)
{
    if (
        (hIntArray != NULL)
        && (pValue != NULL)
        )
    {
        IntArray *pIntArray = (IntArray *)hIntArray;
        *pValue = (*pIntArray)[nIndex];

        return 0;
    }
    return -1;
}

OS_HEAPELEMENT_HANDLE OS_HEAPELEMENT_CreateInstance(CHNSYS_UINT64 nValue, VOID *pObject)
{
    ZOSHeapElement* pZOSHeapElement = NEW ZOSHeapElement(nValue,pObject);

    if (pZOSHeapElement != NULL)
    {
        return (OS_HEAPELEMENT_HANDLE)pZOSHeapElement;
    }

    return NULL;
}

VOID OS_HEAPELEMENT_DestroyInstance(OS_HEAPELEMENT_HANDLE hHeapElement)
{
    if (hHeapElement != NULL)
    {
        ZOSHeapElement *pZOSHeapElement = (ZOSHeapElement *)hHeapElement;
        SAFE_DELETE(pZOSHeapElement);
    }
}

CHNSYS_BOOL OS_HEAPELEMENT_GetValue(OS_HEAPELEMENT_HANDLE hHeapElement, CHNSYS_UINT64 *pValue)
{
    if (hHeapElement != NULL)
    {
        ZOSHeapElement* pZOSHeapElement = (ZOSHeapElement*)hHeapElement;
        *pValue = pZOSHeapElement->GetValue();

        return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_HEAPELEMENT_SetValue(OS_HEAPELEMENT_HANDLE hHeapElement, CHNSYS_UINT64 nValue)
{
    if(hHeapElement != NULL)
    {
        ZOSHeapElement* pZOSHeapElement = (ZOSHeapElement*)hHeapElement;
        pZOSHeapElement->SetValue(nValue);

        return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_HEAPELEMENT_GetObject(OS_HEAPELEMENT_HANDLE hHeapElement, VOID **ppObject)
{
    if (hHeapElement != NULL)
    {
        ZOSHeapElement* pZOSHeapElement = (ZOSHeapElement*)hHeapElement;
        *ppObject = pZOSHeapElement->GetObject();

        return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_HEAPELEMENT_SetObject(OS_HEAPELEMENT_HANDLE hHeapElement, VOID *pObject)
{
    if (hHeapElement != NULL)
    {
        ZOSHeapElement* pZOSHeapElement = (ZOSHeapElement*)hHeapElement;
        pZOSHeapElement->SetObject(pObject);

        return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_HEAPELEMENT_GetHeap(OS_HEAPELEMENT_HANDLE hHeapElement, OS_HEAP_HANDLE *phHeap)
{
    if (hHeapElement != NULL)
    {
        ZOSHeapElement* pZOSHeapElement = (ZOSHeapElement*)hHeapElement;
        *phHeap =  pZOSHeapElement->GetHeap();

        return TRUE;
    }

    return FALSE;
}

OS_HEAP_HANDLE OS_HEAP_CreateInstance(CHNSYS_UINT nInitSize)
{
    ZOSHeap* pZOSHeap = NEW ZOSHeap("CLibHeap",nInitSize);

    if (pZOSHeap != NULL)
    {
        return (OS_HEAP_HANDLE)pZOSHeap;
    }
    return NULL;
}

VOID OS_HEAP_DestroyInstance(OS_HEAP_HANDLE hHeap)
{
    if (hHeap != NULL)
    {
        ZOSHeap* pZOSHeap = (ZOSHeap*)hHeap;
        SAFE_DELETE(pZOSHeap);
    }
}

CHNSYS_BOOL OS_HEAP_Insert(OS_HEAP_HANDLE hHeap,OS_HEAPELEMENT_HANDLE hHeapElement)
{
    if (
        (hHeap != NULL)
        && (hHeapElement != NULL)
        )
    {
        ZOSHeap* pZOSHeap = (ZOSHeap*)hHeap;
        pZOSHeap->Insert((ZOSHeapElement*)hHeapElement);

        return TRUE;
    }
    return FALSE;
}   

CHNSYS_BOOL OS_HEAP_ExtractMin(OS_HEAP_HANDLE hHeap, OS_HEAPELEMENT_HANDLE *phHeapElement)
{
    if (hHeap != NULL)
    {
        ZOSHeap* pZOSHeap = (ZOSHeap*)hHeap;
        *phHeapElement = pZOSHeap->ExtractMin();
        
        return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_HEAP_PeekMin(OS_HEAP_HANDLE hHeap, OS_HEAPELEMENT_HANDLE *phHeapElement)
{
    if (hHeap != NULL)
    {
        ZOSHeap* pZOSHeap = (ZOSHeap*)hHeap;
        *phHeapElement = pZOSHeap->PeekMin();

        return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_HEAP_Remove(OS_HEAP_HANDLE hHeap, OS_HEAPELEMENT_HANDLE hHeapElement)
{
    if (
        (hHeap != NULL)
        && (hHeapElement != NULL)
        )
    {
        ZOSHeap* pZOSHeap = (ZOSHeap*)hHeap;
        pZOSHeap->Remove((ZOSHeapElement*)hHeapElement);

        return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_HEAP_RemoveAll(OS_HEAP_HANDLE hHeap)
{
    if (hHeap != NULL)
    {
        ZOSHeap* pZOSHeap = (ZOSHeap*)hHeap;
        pZOSHeap->RemoveAll();

        return TRUE;
    }

    return FALSE;
}

CHNSYS_INT OS_HEAP_Count(OS_HEAP_HANDLE hHeap)
{
    if (hHeap != NULL)
    {
        ZOSHeap* pZOSHeap = (ZOSHeap*)hHeap;
        return pZOSHeap->HeapCount();
    }

    return -1;
}

OS_QUEUEELEMENT_HANDLE OS_QUEUEELEMENT_CreateInstance(VOID *pObject)
{
    ZOSQueueElement* pZOSQueueElement = NEW ZOSQueueElement(pObject);

    if (pZOSQueueElement != NULL)
    {
        return (OS_QUEUEELEMENT_HANDLE)pZOSQueueElement;
    }

    return NULL;
}

VOID OS_QUEUEELEMENT_DestroyInstance(OS_QUEUEELEMENT_HANDLE hQueueElement)
{
    if(hQueueElement != NULL)
    {
        ZOSQueueElement* pZOSQueueElement = (ZOSQueueElement*)hQueueElement;
        SAFE_DELETE(pZOSQueueElement);
    }
}

CHNSYS_BOOL OS_QUEUEELEMENT_GetObject(OS_QUEUEELEMENT_HANDLE hQueueElement, VOID **ppObject)
{
    if (hQueueElement != NULL)
    {
        ZOSQueueElement* pZOSQueueElement = (ZOSQueueElement*)hQueueElement;
        *ppObject = pZOSQueueElement->GetObject();

        return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_QUEUEELEMENT_SetObject(OS_QUEUEELEMENT_HANDLE hQueueElement, VOID *pObject)
{
    if (hQueueElement != NULL)
    {
        ZOSQueueElement* pZOSQueueElement = (ZOSQueueElement*)hQueueElement;
        pZOSQueueElement->SetObject(pObject);

        return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_QUEUEELEMENT_GetQueue(OS_QUEUEELEMENT_HANDLE hQueueElement, OS_QUEUE_HANDLE *phQueue)
{
    if (hQueueElement != NULL)
    {
        ZOSQueueElement* pZOSQueueElement = (ZOSQueueElement*)hQueueElement;
        *phQueue = (OS_QUEUE_HANDLE*)pZOSQueueElement->GetQueue();

        return TRUE;
    }

    return FALSE;
}

OS_QUEUE_HANDLE OS_QUEUE_CreateInstance()
{
    ZOSQueue* pZOSQueue = NEW ZOSQueue;

    if (pZOSQueue != NULL)
    {
        return (OS_QUEUE_HANDLE)pZOSQueue;
    }

    return NULL;
}

VOID OS_QUEUE_DestroyInstance(OS_QUEUE_HANDLE hQueue)
{
    if (hQueue != NULL)
    {
        ZOSQueue* pZOSQueue = (ZOSQueue*)hQueue;
        SAFE_DELETE(pZOSQueue);
    }
}

CHNSYS_BOOL OS_QUEUE_Push(OS_QUEUE_HANDLE hQueue, OS_QUEUEELEMENT_HANDLE hQueueElement)
{
    if (hQueue != NULL)
    {
         ZOSQueue* pZOSQueue = (ZOSQueue*)hQueue;
         pZOSQueue->Push((ZOSQueueElement*)hQueueElement);

         return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_QUEUE_Pop(OS_QUEUE_HANDLE hQueue, OS_QUEUEELEMENT_HANDLE *phQueueElement)
{
    if (hQueue != NULL)
    {
        ZOSQueue* pZOSQueue = (ZOSQueue*)hQueue;
        *phQueueElement = (ZOSQueue*)pZOSQueue->Pop();

        return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_QUEUE_GetHead(OS_QUEUE_HANDLE hQueue, OS_QUEUEELEMENT_HANDLE *phQueueElement)
{
    if (hQueue != NULL)
    {
        ZOSQueue* pZOSQueue = (ZOSQueue*)hQueue;
        *phQueueElement = (OS_QUEUEELEMENT_HANDLE*)pZOSQueue->GetHead();

        return TRUE;
    }

    return FALSE;
}

CHNSYS_BOOL OS_QUEUE_GetTail(OS_QUEUE_HANDLE hQueue, OS_QUEUEELEMENT_HANDLE *phQueueElement)
{
    if (hQueue != NULL)
    {
        ZOSQueue* pZOSQueue = (ZOSQueue*)hQueue;
        *phQueueElement = (OS_QUEUEELEMENT_HANDLE*)pZOSQueue->GetTail();
        return TRUE;
    }

    return FALSE;
}

CHNSYS_INT OS_QUEUE_Count(OS_QUEUE_HANDLE hQueue)
{
    if (hQueue != NULL)
    {
        ZOSQueue* pZOSQueue = (ZOSQueue*)hQueue;
        return pZOSQueue->GetLength();
    }

    return -1;
}
