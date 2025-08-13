#include "juno/ds/heap_api.h"
#include "juno/macros.h"
#include "juno/status.h"

JUNO_STATUS_T JunoDs_Heap_Update(JUNO_DS_HEAP_ROOT_T *ptHeap)
{
    JUNO_STATUS_T tStatus = {0};
    tStatus = JunoDs_Heap_Verify(ptHeap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    if(ptHeap->zLength == 0)
    {
        tStatus = JUNO_STATUS_ERR;
        return tStatus;
    }
    JUNO_DS_HEAP_INDEX_OPTION_RESULT_T tIndexResult = {0};
    size_t iCurrentIndex = ptHeap->zLength-1;
    // Get the parent of the current index
    if(iCurrentIndex == 0)
    {
        return tStatus;
    }
    // Assign to the parent index
    size_t iParentIndex = 0;
    for(size_t i = 0; i < ptHeap->zLength; ++i)
    {
        tIndexResult = JunoDs_Heap_ChildGetParent(ptHeap, iCurrentIndex);
        ASSERT_SUCCESS(tIndexResult.tStatus, return tIndexResult.tStatus);
        if(!tIndexResult.tSuccess.bIsSome)
        {
            return tStatus;
        }
        // Assign to the parent index
        iParentIndex = tIndexResult.tSuccess.tSome;
        // Assign to the parent metrix
        JUNO_DS_HEAP_COMPARE_RESULT_T tCompareResult = ptHeap->ptApi->Compare(ptHeap, iParentIndex, iCurrentIndex);
        ASSERT_SUCCESS(tCompareResult.tStatus, return tCompareResult.tStatus);
        if(!tCompareResult.tSuccess)
        {
            tStatus = ptHeap->ptApi->Swap(ptHeap, iCurrentIndex, iParentIndex);
            ASSERT_SUCCESS(tStatus, return tStatus);
        }
        if(iParentIndex == 0)
        {
            break;
        }
        iCurrentIndex = iParentIndex;
    }
    return tStatus;
}

JUNO_STATUS_T JunoDs_Heap_SiftDown(JUNO_DS_HEAP_ROOT_T *ptHeap, size_t iStart)
{
    JUNO_STATUS_T tStatus = {0};
    tStatus = JunoDs_Heap_Verify(ptHeap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    if(ptHeap->zLength <= 0)
    {
        tStatus = JUNO_STATUS_ERR;
        return tStatus;
    }
    size_t iRoot = iStart;
    size_t iCurrentIndex = iRoot;
    for(size_t i = 0; i < ptHeap->zLength; ++i)
    {
        iCurrentIndex = iRoot;
        JUNO_DS_HEAP_COMPARE_RESULT_T tCompareResult = {0};
        JUNO_DS_HEAP_INDEX_OPTION_RESULT_T tIndexResult = JunoDs_Heap_ChildGetLeft(ptHeap, iCurrentIndex);
        ASSERT_SUCCESS(tIndexResult.tStatus, return tIndexResult.tStatus);
        size_t iLeft = 0;
        bool bLeftSome = tIndexResult.tSuccess.bIsSome;
        if(tIndexResult.tSuccess.bIsSome)
        {
            iLeft = tIndexResult.tSuccess.tSome;
        }
        tIndexResult = JunoDs_Heap_ChildGetRight(ptHeap, iCurrentIndex);
        ASSERT_SUCCESS(tIndexResult.tStatus, return tIndexResult.tStatus);
        size_t iRight = 0;
        bool bRightSome = tIndexResult.tSuccess.bIsSome;
        if(tIndexResult.tSuccess.bIsSome)
        {
            iRight = tIndexResult.tSuccess.tSome;
        }
        if(bLeftSome)
        {
            tCompareResult = ptHeap->ptApi->Compare(ptHeap, iCurrentIndex, iLeft);
            ASSERT_SUCCESS(tCompareResult.tStatus, return tCompareResult.tStatus);
            if(!tCompareResult.tSuccess)
            {
                iCurrentIndex = iLeft;
            }
        }
        if(bRightSome)
        {
            tCompareResult = ptHeap->ptApi->Compare(ptHeap, iCurrentIndex, iRight);
            ASSERT_SUCCESS(tCompareResult.tStatus, return tCompareResult.tStatus);
            if(!tCompareResult.tSuccess)
            {
                iCurrentIndex = iRight;
            }
        }
        if(iCurrentIndex != iRoot)
        {
            tStatus = ptHeap->ptApi->Swap(ptHeap, iRoot, iCurrentIndex);
            ASSERT_SUCCESS(tStatus, return tStatus);
            iRoot = iCurrentIndex;
        }
        else
        {
            break;
        }
    }
    return tStatus;
}
