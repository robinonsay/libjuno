/**
   Copyright 2025 Robin A. Onsay

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef JUNO_MEMORY_API_H
#define JUNO_MEMORY_API_H
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "juno/memory/memory_types.h"

typedef struct JUNO_MEMORY_API_TAG JUNO_MEMORY_API_T;

/// @brief API for generic memory allocation operations.
/// 
/// This structure holds pointers to functions that implement operations for
/// generic memory allocation including allocation, update, and free.
struct JUNO_MEMORY_API_TAG
{
    /// @brief Allocates memory using the specified memory allocation method.
    /// 
    /// @param ptMem Pointer to the memory allocation structure.
    /// @param pvRetAddr Pointer to a memory descriptor where allocation details will be stored.
    /// @param zSize Size of the memory block to allocate in bytes.
    /// @return JUNO_STATUS_T Status of the allocation operation.
    JUNO_STATUS_T (*Get)(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *pvRetAddr, size_t zSize);

    /// @brief Updates an existing memory allocation to a new size.
    /// 
    /// @param ptMem Pointer to the memory allocation structure.
    /// @param ptMemory Pointer to the memory descriptor to update.
    /// @param zNewSize The new size for the memory block.
    /// @return JUNO_STATUS_T Status of the update operation.
    JUNO_STATUS_T (*Update)(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory, size_t zNewSize);

    /// @brief Frees an allocated memory block.
    /// 
    /// @param ptMem Pointer to the memory allocation structure.
    /// @param pvAddr Pointer to the memory block to free.
    /// @return JUNO_STATUS_T Status of the free operation.
    JUNO_STATUS_T (*Put)(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *pvAddr);
};

#ifdef __cplusplus
}
#endif
#endif


