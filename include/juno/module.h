#ifndef JUNO_MODULE_H
#define JUNO_MODULE_H

#include "juno/status.h"
#include <stdint.h>

#define JUNO_MODULE_DECLARE(name)   typedef struct name##_TAG name;

#define JUNO_FAILURE_HANDLER    _pfcnFailureHandler
#define JUNO_FAILURE_USER_DATA    _pvFailurUserData
#define JUNO_MODULE_EMPTY
#define JUNO_MODULE(name, members) \
struct name##_TAG \
{ \
    members \
    JUNO_FAILURE_HANDLER_T JUNO_FAILURE_HANDLER; \
    JUNO_USER_DATA_T *JUNO_FAILURE_USER_DATA; \
}

#define JUNO_MODULE_SUPER   tSuper
#define JUNO_MODULE_DERIVE(name, base, members) \
struct name##_TAG \
{ \
    base JUNO_MODULE_SUPER; \
    members \
}

#endif // JUNO_MODULE_H
