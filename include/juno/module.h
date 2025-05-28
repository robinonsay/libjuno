#ifndef JUNO_MODULE_H
#define JUNO_MODULE_H
#include "juno/status.h"
#include "juno/memory/memory_types.h"
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>


#define JUNO_FAILURE_HANDLER pfcnFailureHandler
#define JUNO_FAILURE_USERDATA pvFailureUserData

#define JUNO_MODULE_REF(refName)   refName##Ref
#define JUNO_MODULE_DECLARE(name) typedef struct name##_TAG name
#define JUNO_MODULE_GET_REF(name, ...)   name##_GetRef(__VA_ARGS__)
#define JUNO_MODULE_PUT_REF(name, ...)   name##_PutRef(__VA_ARGS__)
#define JUNO_MODULE(name, defs) \
struct name##_TAG \
{ \
    JUNO_MEMORY_T tMemory; \
    defs \
    JUNO_FAILURE_HANDLER_T JUNO_FAILURE_HANDLER; \
    JUNO_USER_DATA_T *JUNO_FAILURE_USERDATA; \
}; \
static inline name * JUNO_MODULE_GET_REF(name, name *ptModule){ \
    if(ptModule) \
    { \
        ptModule->tMemory.iRefCount += 1; \
    } \
    return ptModule; \
} \
static inline void JUNO_MODULE_PUT_REF(name, name *ptModule){ \
    if(ptModule && ptModule->tMemory.iRefCount > 1) \
    { \
        ptModule->tMemory.iRefCount -= 1; \
    } \
}\

#define JUNO_MODULE_FAIL(tStatus, ptModule, pcMessage) FAIL(tStatus, ptModule->JUNO_FAILURE_HANDLER, ptModule->JUNO_FAILURE_USERDATA, pcMessage)

#ifdef __cplusplus
}
#endif
#endif // JUNO_MODULE_H
