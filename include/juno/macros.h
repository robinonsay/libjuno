#ifndef JUNO_MACROS_H
#define JUNO_MACROS_H

#include "juno/status.h"
#include <stdint.h>

#define ASSERT_EXISTS(ptr) \
if(!ptr) \
{ \
    return JUNO_STATUS_NULLPTR_ERROR; \
}

#define ASSERT_SUCCESS(tStatus, failOp) if(tStatus != JUNO_STATUS_SUCCESS) \
{ \
    failOp; \
}


#endif // JUNO_MACROS_H
