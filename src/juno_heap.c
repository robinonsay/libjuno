/*
    MIT License

    Copyright (c) 2025 Robin A. Onsay

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
*/

#include "juno/ds/array_api.h"
#include "juno/ds/heap_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/status.h"
#include <stddef.h>


/**
 * @brief Compute the left child index of iIndex.
 *
 * @return A result wrapping an optional index:
 *  - tStatus = SUCCESS and bIsSome = true when the left child is within current length.
 *  - tStatus = SUCCESS and bIsSome = false when the left child would be beyond zLength.
 *  - tStatus = ERR when the computed index would exceed zCapacity.
 */
static inline JUNO_DS_HEAP_INDEX_RESULT_T JunoDs_Heap_ChildGetLeft(JUNO_DS_HEAP_ROOT_T *ptHeap, size_t iIndex)
{
    JUNO_DS_HEAP_INDEX_RESULT_T tResult = {0};
    tResult.tStatus = JunoDs_Heap_Verify(ptHeap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Calculate the left index
    iIndex = 2 * iIndex + 1;
    // Check if the index is out of bounds or if the length exceeds capacity
    if(iIndex > ptHeap->tRoot.zCapacity || ptHeap->zLength > ptHeap->tRoot.zCapacity)
    {
        tResult.tStatus = JUNO_STATUS_OOB_ERROR;
        return tResult;
    }
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tOk= iIndex;
    return tResult;
}

/**
 * @brief Compute the right child index of iIndex.
 * @copydetails JunoDs_Heap_ChildGetLeft
 */
static inline JUNO_DS_HEAP_INDEX_RESULT_T JunoDs_Heap_ChildGetRight(JUNO_DS_HEAP_ROOT_T *ptHeap, size_t iIndex)
{
    JUNO_DS_HEAP_INDEX_RESULT_T tResult = {JUNO_STATUS_SUCCESS, 0};
    tResult.tStatus = JunoDs_Heap_Verify(ptHeap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Calculate the right child
    iIndex = 2 * iIndex + 2;
    // Check if this index is beyond the capacity or if the length is beyond the capacity
    if(iIndex > ptHeap->tRoot.zCapacity || ptHeap->zLength > ptHeap->tRoot.zCapacity)
    {
        tResult.tStatus = JUNO_STATUS_OOB_ERROR;
        return tResult;
    }
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tOk = iIndex;
    return tResult;
}

/**
 * @brief Compute the parent index of iIndex.
 * @copydetails JunoDs_Heap_ChildGetLeft
 */
static inline JUNO_DS_HEAP_INDEX_RESULT_T JunoDs_Heap_ChildGetParent(JUNO_DS_HEAP_ROOT_T *ptHeap, size_t iIndex)
{
    JUNO_DS_HEAP_INDEX_RESULT_T tResult = {0};
    tResult.tStatus = JunoDs_Heap_Verify(ptHeap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // calculate the parent
    iIndex = (iIndex - 1)/2;
    // Check if the index is OOB
    if(iIndex > ptHeap->tRoot.zCapacity || ptHeap->zLength > ptHeap->tRoot.zCapacity)
    {
        tResult.tStatus = JUNO_STATUS_OOB_ERROR;
        return tResult;
    }
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tOk = iIndex;
    return tResult;
}

JUNO_STATUS_T JunoDs_Heap_Update(JUNO_DS_HEAP_ROOT_T *ptHeap)
{
    JUNO_STATUS_T tStatus = {0};
    tStatus = JunoDs_Heap_Verify(ptHeap);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    if(ptHeap->zLength == 0)
    {
        tStatus = JUNO_STATUS_ERR;
        return tStatus;
    }
    JUNO_DS_HEAP_INDEX_RESULT_T tIndexResult = {0};
    size_t iCurrentIndex = ptHeap->zLength-1;
    // Get the parent of the current index
    if(iCurrentIndex == 0)
    {
        return tStatus;
    }
    const JUNO_DS_ARRAY_API_T *ptArrayApi = &ptHeap->ptApi->tRoot;
    JUNO_DS_ARRAY_ROOT_T *ptArray = &ptHeap->tRoot;
    // Assign to the parent index
    size_t iParentIndex = 0;
    for(size_t i = 0; i < ptHeap->zLength; ++i)
    {
        tIndexResult = JunoDs_Heap_ChildGetParent(ptHeap, iCurrentIndex);
        JUNO_ASSERT_SUCCESS(tIndexResult.tStatus, return tIndexResult.tStatus);
        if(JUNO_OK(tIndexResult) >= ptHeap->zLength)
        {
            return tStatus;
        }
        // Assign to the parent index
        iParentIndex = tIndexResult.tOk;
        JUNO_RESULT_POINTER_T tResultCurrent = ptArrayApi->GetAt(ptArray, iCurrentIndex);
        JUNO_ASSERT_OK(tResultCurrent, return tResultCurrent.tStatus);
        JUNO_RESULT_POINTER_T tResultParent = ptArrayApi->GetAt(ptArray, iParentIndex);
        JUNO_ASSERT_OK(tResultParent, return tResultParent.tStatus);
        // Assign to the parent metrix
        JUNO_DS_HEAP_COMPARE_RESULT_T tCompareResult = ptHeap->ptApi->Compare(ptHeap, JUNO_OK(tResultParent), JUNO_OK(tResultCurrent));
        JUNO_ASSERT_SUCCESS(tCompareResult.tStatus, return tCompareResult.tStatus);
        if(!tCompareResult.tOk)
        {
            tStatus = ptHeap->ptApi->Swap(ptHeap, JUNO_OK(tResultCurrent), JUNO_OK(tResultParent));
            JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
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
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    if(ptHeap->zLength <= 0)
    {
        tStatus = JUNO_STATUS_ERR;
        return tStatus;
    }
    size_t iRoot = iStart;
    size_t iCurrentIndex = iRoot;
    const JUNO_DS_ARRAY_API_T *ptArrayApi = &ptHeap->ptApi->tRoot;
    JUNO_DS_ARRAY_ROOT_T *ptArray = &ptHeap->tRoot;
    for(size_t i = 0; i < ptHeap->zLength; ++i)
    {
        iCurrentIndex = iRoot;
        JUNO_DS_HEAP_COMPARE_RESULT_T tCompareResult = {0};
        JUNO_DS_HEAP_INDEX_RESULT_T tIndexResult = JunoDs_Heap_ChildGetLeft(ptHeap, iCurrentIndex);
        JUNO_ASSERT_SUCCESS(tIndexResult.tStatus, return tIndexResult.tStatus);
        size_t iLeft = 0;
        bool bLeftSome = JUNO_OK(tIndexResult) < ptHeap->zLength;
        if(bLeftSome)
        {
            iLeft = tIndexResult.tOk;
        }
        tIndexResult = JunoDs_Heap_ChildGetRight(ptHeap, iCurrentIndex);
        JUNO_ASSERT_SUCCESS(tIndexResult.tStatus, return tIndexResult.tStatus);
        size_t iRight = 0;
        bool bRightSome = tIndexResult.tOk < ptHeap->zLength;
        JUNO_RESULT_POINTER_T tResultCurrent = ptArrayApi->GetAt(ptArray, iCurrentIndex);
        JUNO_ASSERT_OK(tResultCurrent, return tResultCurrent.tStatus);
        if(bRightSome)
        {
            iRight = tIndexResult.tOk;
        }
        if(bLeftSome)
        {
            tResultCurrent = ptArrayApi->GetAt(ptArray, iCurrentIndex);
            JUNO_ASSERT_OK(tResultCurrent, return tResultCurrent.tStatus);
            JUNO_RESULT_POINTER_T tResultLeft = ptArrayApi->GetAt(ptArray, iLeft);
            JUNO_ASSERT_OK(tResultLeft, return tResultLeft.tStatus);
            tCompareResult = ptHeap->ptApi->Compare(ptHeap, JUNO_OK(tResultCurrent), JUNO_OK(tResultLeft));
            JUNO_ASSERT_SUCCESS(tCompareResult.tStatus, return tCompareResult.tStatus);
            if(!tCompareResult.tOk)
            {
                iCurrentIndex = iLeft;
            }
        }
        if(bRightSome)
        {
            tResultCurrent = ptArrayApi->GetAt(ptArray, iCurrentIndex);
            JUNO_ASSERT_OK(tResultCurrent, return tResultCurrent.tStatus);
            JUNO_RESULT_POINTER_T tResulRight = ptArrayApi->GetAt(ptArray, iRight);
            JUNO_ASSERT_OK(tResulRight, return tResulRight.tStatus);
            tCompareResult = ptHeap->ptApi->Compare(ptHeap, JUNO_OK(tResultCurrent), JUNO_OK(tResulRight));
            JUNO_ASSERT_SUCCESS(tCompareResult.tStatus, return tCompareResult.tStatus);
            if(!tCompareResult.tOk)
            {
                iCurrentIndex = iRight;
            }
        }
        if(iCurrentIndex != iRoot)
        {
            tResultCurrent = ptArrayApi->GetAt(ptArray, iCurrentIndex);
            JUNO_ASSERT_OK(tResultCurrent, return tResultCurrent.tStatus);
            JUNO_RESULT_POINTER_T tResultRoot = ptArrayApi->GetAt(ptArray, iRoot);
            JUNO_ASSERT_OK(tResultRoot, return tResultRoot.tStatus);
            tStatus = ptHeap->ptApi->Swap(ptHeap, JUNO_OK(tResultCurrent), JUNO_OK(tResultRoot));
            JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
            iRoot = iCurrentIndex;
        }
        else
        {
            break;
        }
    }
    return tStatus;
}


/**
 * @brief Reserve a new slot at the end of the heap array and return its index.
 *
 * After a successful insert, you must write the element to the returned index
 * in your storage, then call JunoDs_Heap_Update(...) to bubble it up.
 *
 * @return A result where:
 *  - tStatus = SUCCESS and tOk = new index when there is capacity.
 *  - tStatus = ERR when zLength >= zCapacity (no more space).
 */
JUNO_STATUS_T JunoDs_Heap_Insert(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tValue)
{
    JUNO_STATUS_T tStatus = JunoDs_Heap_Verify(ptHeap);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    if(ptHeap->zLength >= ptHeap->tRoot.zCapacity)
    {
        tStatus = JUNO_STATUS_ERR;
        return tStatus;
    }
    tStatus = JunoMemory_PointerVerify(tValue);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = ptHeap->ptApi->tRoot.SetAt(&ptHeap->tRoot, tValue, ptHeap->zLength);
    ptHeap->zLength += 1;
    tStatus = JunoDs_Heap_Update(ptHeap);
    return tStatus;
}

/**
 * @brief Build a heap in-place from the current array contents.
 *
 * Set zLength to the number of initialized elements, then call Heapify.
 *
 * @return JUNO_STATUS_SUCCESS on success; JUNO_STATUS_ERR when zLength == 0 or
 *         if SiftDown reports an error.
 */
JUNO_STATUS_T JunoDs_Heap_Heapify(JUNO_DS_HEAP_ROOT_T *ptHeap)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = JunoDs_Heap_Verify(ptHeap);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    if(ptHeap->zLength == 0)
    {
        return JUNO_STATUS_ERR;
    }
    JUNO_DS_HEAP_INDEX_RESULT_T iIndexResult = JunoDs_Heap_ChildGetParent(ptHeap, ptHeap->zLength);
    JUNO_ASSERT_SUCCESS(iIndexResult.tStatus, return iIndexResult.tStatus);
    if(iIndexResult.tOk >= ptHeap->zLength)
    {
        return tStatus;
    }
    size_t iIndex = iIndexResult.tOk;
    for(size_t i = 0; i <= iIndex; ++i)
    {
        size_t iCurrentIndex = iIndex - i;
        tStatus = JunoDs_Heap_SiftDown(ptHeap, iCurrentIndex);
        JUNO_ASSERT_SUCCESS(tStatus, continue);
    }
    return tStatus;
}

/**
 * @brief Remove the root element and restore the heap property.
 *
 * Algorithm:
 *  - Swap the root and the last element.
 *  - Call Reset on the last index.
 *  - Decrement zLength and SiftDown from the root.
 *
 * Error propagation:
 *  - If Reset returns an error, the delete operation returns that error (it is
 *    not ignored).
 *
 * @return JUNO_STATUS_SUCCESS on success; JUNO_STATUS_ERR if zLength == 0 or
 *         if Swap/Reset/SiftDown report an error.
 */
JUNO_STATUS_T JunoDs_Heap_Pop(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tReturn)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;;
    tStatus = JunoDs_Heap_Verify(ptHeap);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    if(ptHeap->zLength <= 0)
    {
        tStatus = JUNO_STATUS_ERR;
        return tStatus;
    }
    tStatus = JunoMemory_PointerVerify(tReturn);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    const JUNO_DS_ARRAY_API_T *ptArrayApi = &ptHeap->ptApi->tRoot;
    JUNO_DS_ARRAY_ROOT_T *ptArray = &ptHeap->tRoot;
    JUNO_RESULT_POINTER_T tEnd = ptArrayApi->GetAt(ptArray, ptHeap->zLength-1);
    JUNO_ASSERT_OK(tEnd, return tEnd.tStatus);
    JUNO_RESULT_POINTER_T tStart = ptArrayApi->GetAt(ptArray, 0);
    JUNO_ASSERT_OK(tStart, return tStart.tStatus);
    tStatus = ptHeap->ptApi->Swap(ptHeap, JUNO_OK(tEnd), JUNO_OK(tStart));
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tReturn);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = tReturn.ptApi->Copy(tReturn, JUNO_OK(tEnd));
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = ptArrayApi->RemoveAt(ptArray, ptHeap->zLength - 1);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    ptHeap->zLength -= 1;
    if(ptHeap->zLength > 0)
    {
        tStatus = JunoDs_Heap_SiftDown(ptHeap, 0);
    }
    return tStatus;
}
