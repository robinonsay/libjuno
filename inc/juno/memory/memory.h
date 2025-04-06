#ifndef JUNO_MEMORY_DIRECT_H
#define JUNO_MEMORY_DIRECT_H

#include "juno/memory/memory_types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes a memory block for allocation.
 *
 * Sets up a memory block with an associated free stack for managing fixed-size allocations.
 *
 * @param ptMemBlk Pointer to the memory block structure to initialize.
 * @param pvMemory Pointer to the contiguous memory used for allocations.
 * @param pvMemoryFreeStack Pointer to an array of pointers used as the free stack.
 * @param zTypeSize Size in bytes of each element in the block.
 * @param zLength Total number of possible allocations.
 * @param pfcnFailureHandler Callback function to handle failures.
 * @param pvUserData User data passed to the failure handler.
 *
 * @return JUNO_STATUS_T Status of the initialization.
 */
JUNO_STATUS_T Juno_MemoryBlkInit(
    JUNO_MEMORY_BLOCK_T *ptMemBlk,
    void *pvMemory,
    uint8_t **pvMemoryFreeStack,
    size_t zTypeSize,
    size_t zLength,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
);

/**
 * @brief Allocates a block of memory.
 *
 * Retrieves an available fixed-size memory block from the free stack or creates a new allocation if available.
 *
 * @param ptMemBlk Pointer to the memory block structure.
 * @param pvRetAddr Address where the pointer to the allocated block will be stored.
 *
 * @return JUNO_STATUS_T Status of the allocation attempt.
 */
JUNO_STATUS_T Juno_MemoryBlkGet(JUNO_MEMORY_BLOCK_T *ptMemBlk, void **pvRetAddr);

/**
 * @brief Frees a previously allocated block of memory.
 *
 * Returns a memory block back to the free stack so it can be reused.
 *
 * @param ptMemBlk Pointer to the memory block structure.
 * @param pvAddr Pointer to the allocated block to be freed.
 *
 * @return JUNO_STATUS_T Status of the free operation.
 */
JUNO_STATUS_T Juno_MemoryBlkPut(JUNO_MEMORY_BLOCK_T *ptMemBlk, void *pvAddr);

/**
 * @brief Generic memory allocation function.
 *
 * Allocates memory by using the appropriate allocation method based on the allocation type.
 *
 * @param ptMem Pointer to the memory allocation structure.
 * @param ptMemory Pointer to a memory descriptor where allocation details will be stored.
 *
 * @return JUNO_STATUS_T Status of the allocation.
 */
JUNO_STATUS_T Juno_MemoryGet(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory);

/**
 * @brief Generic memory free function.
 *
 * Releases memory back to the allocator and clears the memory descriptor.
 *
 * @param ptMem Pointer to the memory allocation structure.
 * @param ptMemory Pointer to the memory descriptor to free.
 *
 * @return JUNO_STATUS_T Status of the free operation.
 */
JUNO_STATUS_T Juno_MemoryPut(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory);

/**
 * @brief Fills a block of memory with a specific byte value.
 *
 * This function writes a given byte value to a memory block of a specified size. It first aligns the
 * destination pointer to a multiple of sizeof(size_t) and fills any leading unaligned bytes individually.
 * Once aligned, it fills the memory in blocks of size_t for efficiency, and finally handles any remaining bytes.
 *
 * @param pcMem Pointer to the memory block to be set.
 * @param iVal The value to be assigned to each byte.
 * @param zSize The total number of bytes to set in the memory block.
 */
inline void Juno_Memset(void *pcMem, const uint8_t iVal, size_t zSize)
{
    uint8_t *pData = (uint8_t *)pcMem;
    // Fill any initial bytes until p is aligned to sizeof(size_t)
    while(zSize > 0 && ((size_t)pData % sizeof(size_t)) != 0) {
        *pData = iVal;
        pData += 1;
        zSize -= 1;
    }
    if(zSize >= sizeof(size_t)) {
        // Create a word with every byte set to iVal
        size_t word = 0;
        for (size_t i = 0; i < sizeof(size_t); ++i) {
            word |= (size_t)iVal << (i * 8);
        }

        // Write in blocks of size_t
        size_t *pword = (void *)pData;
        size_t num_words = zSize / sizeof(size_t);
        for (size_t i = 0; i < num_words; ++i) {
            pword[i] = word;
        }

        // Advance pointer and adjust remaining size
        pData += num_words * sizeof(size_t);
        zSize -= num_words * sizeof(size_t);
    }
    // Set any remaining bytes one by one
    while(zSize--) {
        *pData = iVal;
        pData += 1;
    }
}

/**
 * @brief Copies memory from source to destination.
 *
 * This function transfers zSize bytes from the memory block pointed to by pSrc
 * to the memory block pointed to by pDest. It first copies byte-by-byte until both 
 * source and destination pointers are aligned to a multiple of sizeof(size_t), which
 * allows for a more efficient block copy using word-sized chunks. After the bulk copy,
 * any remaining bytes are copied individually.
 *
 * @param pDest Pointer to the destination memory block.
 * @param pSrc Pointer to the source memory block.
 * @param zSize The number of bytes to copy.
 */
inline void Juno_Memcpy(void *pDest, const void *pSrc, size_t zSize)
{
    uint8_t *pcDest = (uint8_t *)pDest;
    const uint8_t *pcSrc = (const uint8_t *)pSrc;

    // Copy bytes until both source and destination pointers are aligned
    while(zSize > 0 && !(((size_t)pcDest % sizeof(size_t)) == 0 && ((size_t)pcSrc % sizeof(size_t)) == 0))
    {
        *pcDest = *pcSrc;
        pcDest += 1;
        pcSrc += 1;
        zSize -= 1;
    }
    // Now both pointers are aligned (or n is too small for word copy)
    size_t numWords = zSize / sizeof(size_t);
    size_t remainder = zSize % sizeof(size_t);
    size_t *pzDestWord = (void *)pcDest;
    const size_t *pzSrcWord = (const void *)pcSrc;
    for (size_t i = 0; i < numWords; ++i) {
        pzDestWord[i] = pzSrcWord[i];
    }
    // Copy any remaining bytes
    pcDest = (uint8_t *)(pzDestWord + numWords);
    pcSrc = (const uint8_t *)(pzSrcWord + numWords);
    for (size_t i = 0; i < remainder; ++i) {
        pcDest[i] = pcSrc[i];
    }
}

#ifdef __cplusplus
}
#endif
#endif
