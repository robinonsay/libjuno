#ifndef JUNO_TYPES_H
#define JUNO_TYPES_H
#include "module.h"
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct JUNO_RESULT_BOOL_TAG JUNO_MODULE_RESULT(bool) JUNO_RESULT_BOOL_T;
typedef struct JUNO_RESULT_UINT32_TAG JUNO_MODULE_RESULT(uint32_t) JUNO_RESULT_UINT32_T;

#ifdef __cplusplus
}
#endif
#endif