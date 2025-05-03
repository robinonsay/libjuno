# LibJuno

LibJuno is a lightweight C99 library designed specifically for embedded systems. It focuses on providing essential functionalities like memory management, data structures, string operations, and more without dynamic memory allocation. LibJuno optimizes for memory safety, determinism, and efficiency in constrained environments.

## Library Architecture

LibJuno is structured around several key modules, each providing specialized functionality while maintaining a consistent API pattern:

- **Memory Management**: Provides block-based allocation, deallocation, and memory tracking
- **String Operations**: Handles string initialization, manipulation, concatenation, and cleanup
- **CRC**: Offers various CRC calculation algorithms (ARC, BINHEX, CCITT, etc.)
- **Hash**: Implements hash table functionality with configurable hash functions
- **Map**: Provides key-value pair storage and retrieval
- **Table**: Implements structured data storage with optional persistence

## General Design Principles

LibJuno follows several core design principles:

1. **No Dynamic Memory Allocation**: All memory is pre-allocated and managed using block allocators, ensuring deterministic behavior.
2. **Minimal Dependencies**: The library minimizes dependencies on the standard C library, enhancing portability.
3. **Consistent API Structure**: Each module provides both direct function calls and a structured API interface.
4. **Error Handling**: Comprehensive error codes and optional failure handlers for robust error management.
5. **Reference Counting**: Memory objects use reference counting to prevent premature deallocation of shared resources.

## Common Patterns

### API Structure

Each module follows a consistent pattern:

```c
// Direct function call
JUNO_STATUS_T Juno_ModuleFunction(...);

// API-based access
const JUNO_MODULE_API_T* Juno_ModuleApi(void);
// Then use ptModuleApi->Function(...)
```

### Error Handling

LibJuno uses a standardized error handling approach:

```c
JUNO_STATUS_T tStatus = Juno_ModuleFunction(...);
if (tStatus != JUNO_STATUS_SUCCESS) {
    // Handle error
}
```

Modules also support optional failure handlers that can be registered during initialization:

```c
void MyFailureHandler(JUNO_STATUS_T tStatus, const char* pcMessage, JUNO_USER_DATA_T* pvUserData) {
    // Custom error handling
}

// Then during initialization
Juno_ModuleInit(..., MyFailureHandler, pvUserData);
```

### Memory Management

Memory in LibJuno is typically managed through pre-allocated blocks:

```c
// Declare memory block
JUNO_MEMORY_BLOCK(myMemory, MyType, 10);
JUNO_MEMORY_BLOCK_METADATA(myMetadata, 10);

// Initialize block allocator
JUNO_MEMORY_BLOCK_T tMemBlock;
Juno_MemoryBlkInit(&tMemBlock, myMemory, myMetadata, sizeof(MyType), 10, NULL, NULL);
```

### Reference Counting

Memory objects support reference counting to manage shared resources:

```c
// Get a reference to memory
JUNO_MEMORY_T* ptMemRef = Juno_MemoryGetRef(ptMemory);

// Release reference when done
Juno_MemoryPutRef(ptMemRef);
```

## Module Documentation

For detailed documentation on individual modules, please refer to:

- [Memory Module](memory/README.md)
- [String Module (Beta)](string/README.md)
- [CRC Module (Beta)](crc/README.md)
- [Hash Module (Beta)](hash/README.md)
- [Map Module (Beta)](map/README.md)
- [Table Module (Beta)](table/README.md)
