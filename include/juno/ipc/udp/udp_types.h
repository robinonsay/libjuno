#ifndef JUNO_UDP_TYPES_H
#define JUNO_UDP_TYPES_H
#include "juno/status.h"
#include "juno/string/string_types.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JUNO_UDP_TAG JUNO_UDP_T;
typedef struct JUNO_UDP_OS_DATA_TAG JUNO_UDP_OS_DATA_T;

/**
    The Juno UDP Module
*/
struct JUNO_UDP_TAG
{
    /// The OS specific data
    JUNO_UDP_OS_DATA_T *_ptOsData;
    /// The IP Address
    JUNO_STRING_T _pcIpAddr;
    /// The Juno Failure Handler
    JUNO_FAILURE_HANDLER_T _pfcnFailureHdlr;
    /// The Juno User Data;
    JUNO_USER_DATA_T *_pvFailureUserData;
};

#ifdef __cplusplus
}
#endif
#endif
